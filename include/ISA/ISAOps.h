#include "Op.h"

#ifndef ISA_OP_H
#define ISA_OP_H

class GlobalToLocalMemCopyOp : public Op {
  private:     
    Operand srcSlice;
    Operand dstSlice;
 
  public:
    GlobalToLocalMemCopyOp(ID coreNum, Operand srcSlice, Operand dstSlice)
        : Op(GLOBAL_TO_LOCAL_MEM_COPY, coreNum), srcSlice(srcSlice), dstSlice(dstSlice) {}
};

class LocalToGlobalMemCopyOp : public Op {
  private:     
     Operand srcSlice;
     Operand dstSlice;    
    
  public:
    LocalToGlobalMemCopyOp(ID coreNum, Operand srcSlice, Operand dstSlice)
          : Op(LOCAL_TO_GLOBAL_MEM_COPY, coreNum), srcSlice(srcSlice), dstSlice(dstSlice) {}
};


class MatmulOp : public Op {
  private:   
    ID mmUnitNum;  
    Operand sliceA;
    Operand sliceB;
    Operand sliceC;
    Operand accumulate;  
  
  public:
    MatmulOp(ID coreNum, ID mmUnitNum, Operand sliceA, Operand sliceB, Operand sliceC, Operand accumulate)  
          : Op(MATMUL, coreNum), mmUnitNum(mmUnitNum), sliceA(sliceA), sliceB(sliceB), sliceC(sliceC), accumulate(accumulate) {}
  
    int getMMUnitNum() const {
        return mmUnitNum;
    }
};

#endif // ISA_OP_H

