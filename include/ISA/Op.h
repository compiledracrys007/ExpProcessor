#include <iostream>
#include <vector>

#ifndef OP_H
#define OP_H

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

  int getStart() const { return start; }

  int getEnd() const { return end; }

  int getStride() const { return stride; }
};

class SliceOperand {
private:
  int baseAddress;
  Dim dim1;
  Dim dim0;

public:
  SliceOperand(int baseAddress, Dim dim1, Dim dim0)
      : baseAddress(baseAddress), dim1(dim1), dim0(dim0) {}

  void print(const std::string &indent = "") const {
    std::cout << indent << "Base Address: " << baseAddress << std::endl;
    std::cout << indent << "Dim1:" << std::endl;
    dim1.print(indent + " ");
    dim0.print(indent + " ");
  }

  int getBaseAddress() const { return baseAddress; }

  Dim getDim1() const { return dim1; }

  Dim getDim0() const { return dim0; }
};

class BoolOperand {
private:
  bool value;

public:
  BoolOperand(bool value) : value(value) {}

  void print(const std::string &indent = "") const {
    std::cout << indent << "Bool Value: " << (value ? "true" : "false")
              << std::endl;
  }

  bool asBool() const { return value; }
};

class Op {
private:
  int opCode;
  ID coreId;

public:
  Op(int opCode, ID coreId) : opCode(opCode), coreId(coreId) {}

  int getCoreNum() const { return coreId; }

  ~Op() = default;

  virtual void dump() const = 0;
};

#endif // OP_H
