#include "Processor/Processor.h"
#include "Target/EPU/EPUOps.h"
#include <cstdint>
#include <map>

#ifndef SIMULATOR_H
#define SIMULATOR_H

class Simulator {
private:
  Processor processor;
  int globalMemorySize;
  int localMemoryPerCore;
  int numberOfCores;

  size_t totalMemorySize;
  uint8_t *memory; // raw byte-addressable memory

  int nextFreeGlobalMemoryOffset = 0;
  std::map<int, int> inputHandleToMemoryLocMap;
  std::map<int, int> outputHandleToMemoryLocMap;

  uint8_t *getGlobalMemoryBaseAddress() const { return memory; }

  uint8_t *getLocalMemoryBaseAddress(int coreId) const {
    if (coreId < 0 || coreId >= numberOfCores) {
      return nullptr; // or throw an exception
    }
    return memory + globalMemorySize + (coreId * localMemoryPerCore);
  }

  void executeGlobalToLocalMemCopy(GlobalToLocalMemCopyOp *op);

  void executeLocalToGlobalMemCopy(LocalToGlobalMemCopyOp *op);

  void executeMatmul(MatmulOp *op);

public:
  Simulator(const Processor &proc) : processor(proc) {
    globalMemorySize = proc.getGlobalMemory();
    numberOfCores = proc.getNumberOfCores();
    if (numberOfCores > 0) {
      localMemoryPerCore = proc.getLocalMemoryPerCore();
    } else {
      localMemoryPerCore = 0;
    }

    totalMemorySize = globalMemorySize + (localMemoryPerCore * numberOfCores);

    memory = new uint8_t[totalMemorySize];
  }

  ~Simulator() { delete[] memory; }

  void
  simulateInstructions(const std::vector<std::unique_ptr<Op>> &instructions);

  void registerInputHandle(int handleId, const void *rawData, size_t numBytes);

  void registerOutputHandle(int handleId, size_t numBytes);

  void retrieveOutputData(int handleId, void *outputBuffer, size_t numBytes);
};

#endif // SIMULATOR_H
