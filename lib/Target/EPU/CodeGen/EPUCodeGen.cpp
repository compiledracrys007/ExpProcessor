#include "Target/EPU/CodeGen/EPUCodeGen.h"
#include "Utils/Utils.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

static void emitActivationCopy(int coreId, const std::string &kSlice,
                               std::string &asmStr) {
  auto inst = "cp_global_to_local <1, 0:32:1, " + kSlice + ">, " +
              std::to_string(coreId) + ", <0, 0:32:1, 0:32:1>";
  asmStr += "\n" + inst;
}

static void emitWeightCopy(int coreId, int numOfColTiles,
                           int freeLocalMemOffset, const std::string &kSlice,
                           int tileN, std::string &asmStr) {
  std::string localMemOffset = std::to_string(freeLocalMemOffset);

  // source slice on the K‑dimension (first dimension)
  auto srcKSlice = kSlice;

  // source slice on the N‑dimension (second dimension)
  auto srcDim1Start = std::to_string(coreId * numOfColTiles * tileN);
  auto srcDim1End = std::to_string((coreId * numOfColTiles * tileN) +
                                   (numOfColTiles * tileN));
  auto srcDim1Stride = "1";
  auto srcDim1Slice = srcDim1Start + ":" + srcDim1End + ":" + srcDim1Stride;

  // destination slice (always starts at 0)
  std::string dstDim1Start = "0";
  auto dstDim1End = std::to_string(numOfColTiles * tileN);
  auto dstDim1Stride = "1";
  auto dstDim1Slice = dstDim1Start + ":" + dstDim1End + ":" + dstDim1Stride;
  auto dim0Slice = "0:32:1";

  auto srcMemLocation = "<2, " + srcKSlice + ", " + srcDim1Slice + ">";
  auto dstMemLocation =
      "<" + localMemOffset + ", " + dim0Slice + ", " + dstDim1Slice + ">";

  auto inst = "cp_global_to_local " + srcMemLocation + ", " +
              std::to_string(coreId) + ", " + dstMemLocation;
  asmStr += "\n" + inst;
}

void emitMatmul(int coreId, int mmUnitId, int activationOffset,
                int weightOffset, int freeLocalMemOffset, bool accumulator,
                std::string &asmStr) {
  std::string localMemOffset = std::to_string(freeLocalMemOffset);
  std::string accStr = accumulator ? "True" : "False";
  auto sliceA = "<" + std::to_string(activationOffset) + ", 0:32:1, 0:32:1>";
  auto sliceB = "<" + std::to_string(weightOffset) + ", 0:32:1, 0:32:1>";
  auto sliceC = "<" + std::to_string(freeLocalMemOffset) + ", 0:32:1, 0:32:1>";

  auto inst = "matmul " + std::to_string(coreId) + ", " +
              std::to_string(mmUnitId) + ", " + sliceA + ", " + sliceB + ", " +
              sliceC + ", accumulator=" + accStr;
  asmStr = asmStr + "\n" + inst;
}

static void emitLocalToGlobalCopy(int coreId, int numOfColTiles,
                                  int outputOffset, int tileN,
                                  std::string &asmStr) {
  std::string localMemOffset = std::to_string(outputOffset);
  auto dstDim1Start = std::to_string(coreId * numOfColTiles * tileN);
  auto dstDim1End = std::to_string((coreId * numOfColTiles * tileN) +
                                   (numOfColTiles * tileN));
  auto dstDim1Stride = std::to_string(1);

  auto dim0Slice = "0:32:1";
  auto dstDim1Slice = dstDim1Start + ":" + dstDim1End + ":" + dstDim1Stride;

  auto srcDim1Start = std::to_string(0);
  auto srcDim1End = std::to_string((numOfColTiles * tileN));
  auto srcDim1Stride = std::to_string(1);
  auto srcDim1Slice = srcDim1Start + ":" + srcDim1End + ":" + srcDim1Stride;

  auto dstMemLocation = "<" + std::to_string(3) /*handle_name*/ + ", " +
                        dim0Slice + ", " + dstDim1Slice + ">";

  auto srcMemLocation =
      "<" + localMemOffset + ", " + dim0Slice + ", " + srcDim1Slice + ">";

  auto inst = "cp_local_to_global " + std::to_string(coreId) + ", " +
              srcMemLocation + ", " + dstMemLocation;
  asmStr = asmStr + "\n" + inst;
}

std::string generateMatmulISAForEPU(int M, int N, int K) {
  // Basic validation
  if (M <= 0 || K <= 0 || N <= 0) {
    std::cerr << "Error: All dimensions must be positive integers.\n";
    return "";
  }

  auto epuTarget = createEPUTarget();

  auto numOfCores = epuTarget.getNumberOfCores();
  auto localMemPerCore = epuTarget.getLocalMemoryPerCore();
  auto mmUnitsPerCore = epuTarget.getMMUnitsPerCore();

  auto mmTiles = epuTarget.getMMUnitTiles();

  auto tileM = std::get<0>(mmTiles);
  auto tileK = std::get<1>(mmTiles);
  auto tileN = std::get<2>(mmTiles);

  assert(M % tileM == 0 && N % tileN == 0 && K % tileK == 0 &&
         "Support perfect tiles only for now");

  assert(M == tileM && "Support codegen for M > TILE_M");

  // Simple column splittting matmul algo
  auto numOfColTiles = N / tileN;

  auto bytesPerActivationTile =
      tileM * tileK * (32 /* since 32-bit float */ / 8);

  auto bytesPerWeightTile = tileK * tileN * (32 /* since 32-bit float */ / 8);

  assert(((numOfColTiles % numOfCores == 0) || (numOfColTiles < numOfCores)) &&
         "Support codegen for imperfect splits among cores");

  auto colTilesPerCore =
      numOfColTiles < numOfCores ? 1 : numOfColTiles / numOfCores;

  if (colTilesPerCore * bytesPerWeightTile <
      (localMemPerCore - bytesPerActivationTile)) {

    std::string asmStr = "";
    int kTiles = K / tileK;
    for (int k = 0; k < kTiles; ++k) {
      asmStr += "\nstart_parallel";
      for (int i = 0;
           i < (numOfColTiles < numOfCores ? numOfColTiles : numOfCores); i++) {
        int availableLocalMemOffset = 0;
        auto activationOffset = availableLocalMemOffset;
        availableLocalMemOffset += bytesPerActivationTile;
        auto weightOfffset = availableLocalMemOffset;
        availableLocalMemOffset += (bytesPerWeightTile * colTilesPerCore);
        int actStart = k * tileK;
        int actEnd = actStart + tileK;
        std::string actSlice =
            std::to_string(actStart) + ":" + std::to_string(actEnd) + ":1";
        emitActivationCopy(i, actSlice, asmStr);
      }
      asmStr += "\nend_parallel";

      asmStr += "\nstart_parallel";
      for (int i = 0;
           i < (numOfColTiles < numOfCores ? numOfColTiles : numOfCores); i++) {
        int availableLocalMemOffset = 0;
        auto activationOffset = availableLocalMemOffset;
        availableLocalMemOffset += bytesPerActivationTile;
        auto weightOfffset = availableLocalMemOffset;
        availableLocalMemOffset += (bytesPerWeightTile * colTilesPerCore);
        int wtStart = k * tileK;
        int wtEnd = wtStart + tileK;
        std::string wtSlice =
            std::to_string(wtStart) + ":" + std::to_string(wtEnd) + ":1";
        emitWeightCopy(i, colTilesPerCore, weightOfffset, wtSlice, tileN,
                       asmStr);
      }
      asmStr += "\nend_parallel";

      asmStr += "\nstart_parallel";
      for (int i = 0;
           i < (numOfColTiles < numOfCores ? numOfColTiles : numOfCores); i++) {
        int availableLocalMemOffset = 0;
        auto activationOffset = availableLocalMemOffset;
        availableLocalMemOffset += bytesPerActivationTile;
        auto weightOfffset = availableLocalMemOffset;
        availableLocalMemOffset += (bytesPerWeightTile * colTilesPerCore);
        for (int j = 0; j < colTilesPerCore; j++) {
          bool acc = (k != 0);
          emitMatmul(i, j % mmUnitsPerCore, activationOffset, weightOfffset,
                     availableLocalMemOffset, acc, asmStr);
        }
      }
      asmStr += "\nend_parallel";
    }

    asmStr += "\nstart_parallel";
    for (int i = 0;
         i < (numOfColTiles < numOfCores ? numOfColTiles : numOfCores); i++) {
      int availableLocalMemOffset = 0;
      auto activationOffset = availableLocalMemOffset;
      availableLocalMemOffset += bytesPerActivationTile;
      auto weightOfffset = availableLocalMemOffset;
      availableLocalMemOffset += (bytesPerWeightTile * colTilesPerCore);
      emitLocalToGlobalCopy(i, colTilesPerCore, availableLocalMemOffset, tileN,
                            asmStr);
    }
    asmStr += "\nend_parallel";

    std::cout<<"asmStr : "<<asmStr<<"\n";

    return asmStr;

  } else {
    assert("Can't fit memory, need to change codegen logic\n");
  }

  assert(false && "unhandled case");
  return "";
}
