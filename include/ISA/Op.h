#include <vector>
#ifndef OP_H
#define OP_H

enum OpCode {
    GLOBAL_TO_LOCAL_MEM_COPY,
    LOCAL_TO_GLOBAL_MEM_COPY,
    MATMUL
};  

class Operand {
  public:
    Operand() = default;
};

using ID = int;

class SliceOperand: public Operand {
  private:
    int baseAddress;
    int dim0End;
    int dim0Start;
    int dim0Stride;
    int dim1End;
    int dim1Start;
    int dim1Stride;  
  
  public:
    SliceOperand(int baseAddress, int dim1Start, int dim1End, int dim1Stride, 
                 int dim0Start, int dim0End, int dim0Stride)
                : Operand(), baseAddress(baseAddress),
                  dim0End(dim0End), dim0Start(dim0Start), dim0Stride(dim0Stride),
                  dim1End(dim1End), dim1Start(dim1Start), dim1Stride(dim1Stride) {}
};

class BoolOperand : public Operand {
  private:
    bool value;

  public:
    BoolOperand(bool value) : Operand(), value(value) {}
};


class Op {
  private:
    OpCode opCode;
    ID coreId;
 
  public:
    Op(OpCode opCode, ID coreId) : opCode(opCode), coreId(coreId) {}

    int getCoreNum() const {
        return coreId;
    }
};

#endif // OP_H
