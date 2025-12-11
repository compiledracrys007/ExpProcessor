#include "Target/EPU/CodeGen/EPUCodeGen.h"
#include "Utils/Utils.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

static void emitActivationCopy(int coreId, std::string &asmStr) {
  auto inst = "cp_global_to_local <1, 0:32:1, 0:32:1>, " +
              std::to_string(coreId) + ", <0, 0:32:1, 0:32:1>";
  asmStr = asmStr + "\n" + inst;
}

static void emitWeightCopy(int coreId, int numOfColTiles,
                           int freeLocalMemOffset, int tileN,
                           std::string &asmStr) {
  std::string localMemOffset = std::to_string(freeLocalMemOffset);
  auto srcDim1Start = std::to_string(coreId * numOfColTiles * tileN);
  auto srcDim1End = std::to_string((coreId * numOfColTiles * tileN) +
                                   (numOfColTiles * tileN));
  auto srcDim1Stride = std::to_string(1);

  auto dim0Slice = "0:32:1";
  auto srcDim1Slice = srcDim1Start + ":" + srcDim1End + ":" + srcDim1Stride;

  auto dstDim1Start = std::to_string(0);
  auto dstDim1End = std::to_string((numOfColTiles * tileN));
  auto dstDim1Stride = std::to_string(1);
  auto dstDim1Slice = dstDim1Start + ":" + dstDim1End + ":" + dstDim1Stride;

  auto srcMemLocation = "<" + std::to_string(2) /*handle_name*/ + ", " +
                        dim0Slice + ", " + srcDim1Slice + ">";

  auto dstMemLocation =
      "<" + localMemOffset + ", " + dim0Slice + ", " + dstDim1Slice + ">";

  auto inst = "cp_global_to_local " + srcMemLocation + ", " +
              std::to_string(coreId) + ", " + dstMemLocation;

  asmStr = asmStr + "\n" + inst;
}

void emitMatmul(int coreId, int mmUnitId, int activationOffset,
                int weightOffset, int freeLocalMemOffset, std::string &asmStr) {
  std::string localMemOffset = std::to_string(freeLocalMemOffset);

  auto sliceA = "<" + std::to_string(activationOffset) + ", 0:32:1, 0:32:1>";
  auto sliceB = "<" + std::to_string(weightOffset) + ", 0:32:1, 0:32:1>";
  auto sliceC = "<" + std::to_string(freeLocalMemOffset) + ", 0:32:1, 0:32:1>";

  auto inst = "matmul " + std::to_string(coreId) + ", " +
              std::to_string(mmUnitId) + ", " + sliceA + ", " + sliceB + ", " +
              sliceC + ", accumulator=False";
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

  assert(M == tileM && K == tileM &&
         "Support codegen for M > TILE_M && K > TILE_K");

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
    for (int i = 0;
         i < (numOfColTiles < numOfCores ? numOfColTiles : numOfCores); i++) {
      // copy Activation[:, :] to local memory
      // copy Weight[:, i * colTilesPerCore:
      //                i * colTilesPerCore  + colTilesPerCore]
      // matmul
      // copy output to out[:, i * colTilesPerCore:
      //                       i * colTilesPerCore  + colTilesPerCore]
      auto availableLocalMemOffset = 0;
      auto activationOffset = 0;
      emitActivationCopy(i, asmStr);
      availableLocalMemOffset += bytesPerActivationTile;
      auto weightOfffset = availableLocalMemOffset;

      emitWeightCopy(i, colTilesPerCore, availableLocalMemOffset, tileN,
                     asmStr);

      availableLocalMemOffset += (bytesPerWeightTile * colTilesPerCore);

      // distribute across different matmul units (round robin)
      for (int j = 0; j < colTilesPerCore; j++) {
        emitMatmul(i, j % mmUnitsPerCore, activationOffset, weightOfffset,
                   availableLocalMemOffset, asmStr);
      }

      emitLocalToGlobalCopy(i, colTilesPerCore, availableLocalMemOffset, tileN,
                            asmStr);
    }

    return asmStr;

  } else {
    assert("Can't fit memory, need to change codegen logic\n");
  }

  assert(false && "unhandled case");
  return "";
}
