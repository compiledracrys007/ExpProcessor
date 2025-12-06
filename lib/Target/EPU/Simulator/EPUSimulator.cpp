#include "Target/EPU/Simulator/EPUSimulator.h"
#include "ISA/Op.h"
#include "Simulator/Simulator.h"
#include "Target/EPU/Asm/EPUOps.h"
#include <exception>
#include <iostream>
#include <stdexcept>

void EPUSimulator::executeGlobalToLocalMemCopy(GlobalToLocalMemCopyOp *op) {
  auto src = op->getSrcSlice();
  auto dst = op->getDstSlice();
  int coreId = op->getCoreNum();

  // -----------------------------
  // Resolve base addresses
  // -----------------------------
  int handleId = src.getBaseAddress();
  if (this->inputHandleToMemoryLocMap.count(handleId) == 0) {
    std::cerr << "ERROR: Unknown global handle " << handleId << "\n";
    return;
  }

  uint8_t *handleBase =
      getGlobalMemoryBaseAddress() + inputHandleToMemoryLocMap[handleId];

  uint8_t *localBase = getLocalMemoryBaseAddress(coreId) + dst.getBaseAddress();

  // -----------------------------
  // Extract dims
  // -----------------------------
  const Dim &s1 = src.getDim1(); // src row dimension
  const Dim &s0 = src.getDim0(); // src col dimension

  const Dim &d1 = dst.getDim1(); // dst row dimension
  const Dim &d0 = dst.getDim0(); // dst col dimension

  // -----------------------------
  // Validate shapes match
  // -----------------------------
  int rows_src = (s1.getEnd() - s1.getStart()) / s1.getStride();
  int cols_src = (s0.getEnd() - s0.getStart()) / s0.getStride();

  int rows_dst = (d1.getEnd() - d1.getStart()) / d1.getStride();
  int cols_dst = (d0.getEnd() - d0.getStart()) / d0.getStride();

  if (rows_src != rows_dst || cols_src != cols_dst) {
    std::cerr << "ERROR: mismatched source/destination slice shapes!\n";
    return;
  }

  int rows = rows_src;
  int cols = cols_src;

  // -----------------------------
  // *** REAL COPY WITH STRIDES ***
  // -----------------------------
  // Assumption that element type is always float , but this
  // needs to be enhanced to accept any dtype.
  const size_t elemSize = sizeof(float);

  int srcRowStrideBytes = (s0.getEnd() - s0.getStart()) * elemSize;
  int dstRowStrideBytes = (d0.getEnd() - d0.getStart()) * elemSize;

  uint8_t *srcPtr = handleBase;
  uint8_t *dstPtr = localBase;

  for (int r = 0; r < rows; ++r) {
    int srcRowIndex = s1.getStart() + r * s1.getStride();
    int dstRowIndex = d1.getStart() + r * d1.getStride();

    for (int c = 0; c < cols; ++c) {
      int srcColIndex = s0.getStart() + c * s0.getStride();
      int dstColIndex = d0.getStart() + c * d0.getStride();

      size_t srcOffset = (srcRowIndex * (s0.getEnd())) + srcColIndex;
      size_t dstOffset = (dstRowIndex * (d0.getEnd())) + dstColIndex;

      // convert to bytes
      float *srcElement = reinterpret_cast<float *>(handleBase) + srcOffset;
      float *dstElement = reinterpret_cast<float *>(localBase) + dstOffset;

      *dstElement = *srcElement; // actual scalar copy
    }
  }
}

void EPUSimulator::executeLocalToGlobalMemCopy(LocalToGlobalMemCopyOp *op) {
  auto &src = op->getSrcSlice(); // source is LOCAL memory
  auto &dst = op->getDstSlice(); // destination is GLOBAL memory
  int coreId = op->getCoreNum();

  // ------------------------------------------------------------
  // Resolve local memory base
  // ------------------------------------------------------------
  uint8_t *localBase = getLocalMemoryBaseAddress(coreId) + src.getBaseAddress();

  // ------------------------------------------------------------
  // Resolve destination global handle
  // ------------------------------------------------------------
  int handleId = dst.getBaseAddress();
  if (outputHandleToMemoryLocMap.count(handleId) == 0) {
    std::cerr << "ERROR: Unknown global output handle " << handleId << "\n";
    return;
  }

  uint8_t *globalBase =
      getGlobalMemoryBaseAddress() + outputHandleToMemoryLocMap[handleId];

  // ------------------------------------------------------------
  // Dim extraction
  // ------------------------------------------------------------
  const Dim &s1 = src.getDim1(); // rows
  const Dim &s0 = src.getDim0(); // cols

  const Dim &d1 = dst.getDim1();
  const Dim &d0 = dst.getDim0();

  // Slice sizes (number of elements)
  int rows_src = (s1.getEnd() - s1.getStart()) / s1.getStride();
  int cols_src = (s0.getEnd() - s0.getStart()) / s0.getStride();

  int rows_dst = (d1.getEnd() - d1.getStart()) / d1.getStride();
  int cols_dst = (d0.getEnd() - d0.getStart()) / d0.getStride();

  if (rows_src != rows_dst || cols_src != cols_dst) {
    std::cerr << "ERROR: Local→Global slice shape mismatch!\n";
    return;
  }

  int rows = rows_src;
  int cols = cols_src;

  // ------------------------------------------------------------
  // Assume element type = float
  // TODO: attach dtype information to SliceOperand
  // ------------------------------------------------------------
  const size_t elemSize = sizeof(float);

  // Full row lengths for correct offset calculation
  int srcFullCols = s0.getEnd(); // local memory slice full width
  int dstFullCols = d0.getEnd(); // global memory slice full width

  // ------------------------------------------------------------
  // PER-ELEMENT STRIDED COPY (correct stride semantics)
  // ------------------------------------------------------------
  for (int r = 0; r < rows; ++r) {
    int srcRowIndex = s1.getStart() + r * s1.getStride();
    int dstRowIndex = d1.getStart() + r * d1.getStride();

    for (int c = 0; c < cols; ++c) {
      int srcColIndex = s0.getStart() + c * s0.getStride();
      int dstColIndex = d0.getStart() + c * d0.getStride();

      // Logical offsets
      size_t srcOffset = srcRowIndex * srcFullCols + srcColIndex;
      size_t dstOffset = dstRowIndex * dstFullCols + dstColIndex;

      float *srcElement = reinterpret_cast<float *>(localBase) + srcOffset;
      float *dstElement = reinterpret_cast<float *>(globalBase) + dstOffset;

      *dstElement = *srcElement;
    }
  }
}

void EPUSimulator::executeMatmul(MatmulOp *op) {
  int coreId = op->getCoreNum();
  int mmUnit = op->getMMUnitNum();
  bool accumulate = op->getAccumulate(); // however you represent this

  // -----------------------------
  // Resolve slices
  // -----------------------------
  auto &A = op->getSliceA();
  auto &B = op->getSliceB();
  auto &C = op->getSliceC();

  // -----------------------------
  // Resolve local memory bases
  // -----------------------------
  uint8_t *coreLocalBase = getLocalMemoryBaseAddress(coreId);

  float *A_base = reinterpret_cast<float *>(coreLocalBase + A.getBaseAddress());
  float *B_base = reinterpret_cast<float *>(coreLocalBase + B.getBaseAddress());
  float *C_base = reinterpret_cast<float *>(coreLocalBase + C.getBaseAddress());

  // -----------------------------
  // Extract slice dims
  // -----------------------------
  const Dim &A_r = A.getDim1();
  const Dim &A_c = A.getDim0();
  const Dim &B_r = B.getDim1();
  const Dim &B_c = B.getDim0();
  const Dim &C_r = C.getDim1();
  const Dim &C_c = C.getDim0();

  // Matrix sizes
  int M = (A_r.getEnd() - A_r.getStart()) / A_r.getStride();  // rows of A
  int K = (A_c.getEnd() - A_c.getStart()) / A_c.getStride();  // cols of A
  int K2 = (B_r.getEnd() - B_r.getStart()) / B_r.getStride(); // rows of B
  int N = (B_c.getEnd() - B_c.getStart()) / B_c.getStride();  // cols of B

  // Sanity check
  if (K != K2) {
    std::cerr << "ERROR: Matmul dimension mismatch: A.cols != B.rows\n";
    return;
  }

  int M2 = (C_r.getEnd() - C_r.getStart()) / C_r.getStride();
  int N2 = (C_c.getEnd() - C_c.getStart()) / C_c.getStride();

  if (M != M2 || N != N2) {
    std::cerr << "ERROR: Matmul output slice shape mismatch\n";
    return;
  }

  // Full width of each row in underlying memory
  int A_fullCols = A_c.getEnd();
  int B_fullCols = B_c.getEnd();
  int C_fullCols = C_c.getEnd();

  // -----------------------------
  // Perform Matmul: C = A * B
  // -----------------------------
  for (int m = 0; m < M; ++m) {
    for (int n = 0; n < N; ++n) {
      // compute C(m,n) initial value
      float sum = accumulate ? 0.0f : 0.0f;

      if (!accumulate) {
        sum = 0.0f;
      } else {
        int Cm = C_r.getStart() + m * C_r.getStride();
        int Cn = C_c.getStart() + n * C_c.getStride();
        size_t C_offset = Cm * C_fullCols + Cn;
        sum = C_base[C_offset]; // accumulate existing value
      }

      // compute dot product for this element
      for (int k = 0; k < K; ++k) {
        int Ar = A_r.getStart() + m * A_r.getStride();
        int Ac = A_c.getStart() + k * A_c.getStride();
        int Br = B_r.getStart() + k * B_r.getStride();
        int Bc = B_c.getStart() + n * B_c.getStride();

        size_t A_offset = Ar * A_fullCols + Ac;
        size_t B_offset = Br * B_fullCols + Bc;

        sum += A_base[A_offset] * B_base[B_offset];
      }

      // write back C(m,n)
      int Cm = C_r.getStart() + m * C_r.getStride();
      int Cn = C_c.getStart() + n * C_c.getStride();
      size_t C_offset = Cm * C_fullCols + Cn;
      C_base[C_offset] = sum;
    }
  }
}

void EPUSimulator::execute(const std::unique_ptr<Op> &inst) {
  if (auto *mm = dynamic_cast<MatmulOp *>(inst.get())) {
    executeMatmul(mm);
  } else if (auto *gtl = dynamic_cast<GlobalToLocalMemCopyOp *>(inst.get())) {
    executeGlobalToLocalMemCopy(gtl);
  } else if (auto *ltg = dynamic_cast<LocalToGlobalMemCopyOp *>(inst.get())) {
    executeLocalToGlobalMemCopy(ltg);
  } else {
    throw std::runtime_error("Unhandled op");
  }
}

void EPUSimulator::simulateInstructions(
    const std::vector<std::unique_ptr<Op>> &instructions) {
  std::cout << "Starting simulation for target = " << processor.getDeviceName()
            << "\n";

  std::cout << "\nTarget Info:\n" << processor.get_device_info() << "\n";

  std::vector<Op *> parallelInstsToDispatch;
  bool fillToParallelDispatcher = false;
  for (auto &inst : instructions) {
    if (auto startParallel = dynamic_cast<StartParallelOp *>(inst.get())) {
      fillToParallelDispatcher = true;
    } else if (auto endParallel = dynamic_cast<EndParallelOp *>(inst.get())) {
      fillToParallelDispatcher = false;

      if (!parallelInstsToDispatch.empty()) {
        // fill code to dispatch parallelly
      }
    } else {
      if (fillToParallelDispatcher)
        parallelInstsToDispatch.push_back(inst.get());
      else
        execute(inst);
    }
  }
}