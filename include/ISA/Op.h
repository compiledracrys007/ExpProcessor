#include <iostream>
#include <vector>

#ifndef OP_H
#define OP_H

enum OpCode { GLOBAL_TO_LOCAL_MEM_COPY, LOCAL_TO_GLOBAL_MEM_COPY, MATMUL };

class Operand {
public:
  Operand() = default;
};

using ID = int;

class Dim {
private:
  int start;
  int end;
  int stride;

public:
  Dim(int start, int end, int stride)
      : start(start), end(end), stride(stride) {}

  void print(const std::string &indent = "") const {
    std::cout << indent << "Start: " << start << ", End: " << end
              << ", Stride: " << stride << std::endl;
  }
};

class SliceOperand : public Operand {
private:
  int baseAddress;
  Dim dim1;
  Dim dim0;

public:
  SliceOperand(int baseAddress, Dim dim1, Dim dim0)
      : Operand(), baseAddress(baseAddress), dim1(dim1), dim0(dim0) {}

  void print(const std::string &indent = "") const {
    std::cout << indent << "Base Address: " << baseAddress << std::endl;
    std::cout << indent << "Dim1:" << std::endl;
    dim1.print(indent + " ");
    dim0.print(indent + " ");
  }
};

class BoolOperand : public Operand {
private:
  bool value;

public:
  BoolOperand(bool value) : Operand(), value(value) {}

  void print(const std::string &indent = "") const {
    std::cout << indent << "Bool Value: " << (value ? "true" : "false")
              << std::endl;
  }
};

class Op {
private:
  OpCode opCode;
  ID coreId;

public:
  Op(OpCode opCode, ID coreId) : opCode(opCode), coreId(coreId) {}

  int getCoreNum() const { return coreId; }

  ~Op() = default;

  virtual void dump() const = 0;
};

#endif // OP_H
