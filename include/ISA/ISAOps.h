#include "Op.h"
#include <iostream>

#ifndef ISA_OP_H
#define ISA_OP_H

class GlobalToLocalMemCopyOp : public Op {
private:
  SliceOperand srcSlice;
  SliceOperand dstSlice;

public:
  GlobalToLocalMemCopyOp(ID coreNum, SliceOperand srcSlice,
                         SliceOperand dstSlice)
      : Op(GLOBAL_TO_LOCAL_MEM_COPY, coreNum), srcSlice(srcSlice),
        dstSlice(dstSlice) {}

  void dump() const override {
    // Implementation of dump for GlobalToLocalMemCopyOp
    std::cout << "\nGlobalToLocalMemCopyOp" << std::endl;
    std::cout << "\tSrc Global Memory" << std::endl;
    srcSlice.print("\t  ");

    std::cout << "\tDst Core ID: " << getCoreNum() << std::endl;
    std::cout << "\tLocal Memory " << std::endl;

    dstSlice.print("\t  ");
  }

  SliceOperand &getSrcSlice() { return srcSlice; }

  SliceOperand &getDstSlice() { return dstSlice; }

  ~GlobalToLocalMemCopyOp() = default;
};

class LocalToGlobalMemCopyOp : public Op {
private:
  SliceOperand srcSlice;
  SliceOperand dstSlice;

public:
  LocalToGlobalMemCopyOp(ID coreNum, SliceOperand srcSlice,
                         SliceOperand dstSlice)
      : Op(LOCAL_TO_GLOBAL_MEM_COPY, coreNum), srcSlice(srcSlice),
        dstSlice(dstSlice) {}

  void dump() const override {
    // Implementation of dump for GlobalToLocalMemCopyOp
    std::cout << "\nLocalToGlobalMemCopyOp" << std::endl;
    std::cout << "\tSrc Core ID: " << getCoreNum() << std::endl;
    std::cout << "\tSrc Local Memory" << std::endl;
    srcSlice.print("\t  ");

    std::cout << "\tDsr Global Memory " << std::endl;
    dstSlice.print("\t  ");
  }

  SliceOperand &getSrcSlice() { return srcSlice; }

  SliceOperand &getDstSlice() { return dstSlice; }

  ~LocalToGlobalMemCopyOp() = default;
};

class MatmulOp : public Op {
private:
  ID mmUnitNum;
  SliceOperand sliceA;
  SliceOperand sliceB;
  SliceOperand sliceC;
  BoolOperand accumulate;

public:
  MatmulOp(ID coreNum, ID mmUnitNum, SliceOperand sliceA, SliceOperand sliceB,
           SliceOperand sliceC, BoolOperand accumulate)
      : Op(MATMUL, coreNum), mmUnitNum(mmUnitNum), sliceA(sliceA),
        sliceB(sliceB), sliceC(sliceC), accumulate(accumulate) {}

  int getMMUnitNum() const { return mmUnitNum; }

  void dump() const override {
    // Implementation of dump for GlobalToLocalMemCopyOp
    std::cout << "\nMatmulOp" << std::endl;
    std::cout << "\tCore ID: " << getCoreNum() << std::endl;
    std::cout << "\tMM Unit Num: " << getMMUnitNum() << std::endl;

    std::cout << "\tSlice A" << std::endl;
    sliceA.print("\t  ");
    std::cout << "\tSlice B" << std::endl;
    sliceB.print("\t  ");
    std::cout << "\tSlice C" << std::endl;
    sliceC.print("\t  ");

    std::cout << "\tAccumulate" << std::endl;
    accumulate.print("\t  ");
  }

  SliceOperand &getSliceA() { return sliceA; }

  SliceOperand &getSliceB() { return sliceB; }

  SliceOperand &getSliceC() { return sliceC; }

  bool getAccumulate() const { return accumulate.asBool(); }

  ~MatmulOp() = default;
};

#endif // ISA_OP_H
