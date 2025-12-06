#include "ISA/ISAOps.h"
#include "Target/Processor.h"
#include <cstdint>

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

  uint8_t *getGlobalMemoryBaseAddress() const { return memory; }

  uint8_t *getLocalMemoryBaseAddress(int coreId) const {
    if (coreId < 0 || coreId >= numberOfCores) {
      return nullptr; // or throw an exception
    }
    return memory + globalMemorySize + (coreId * localMemoryPerCore);
  }

  void
  simulateInstructions(const std::vector<std::unique_ptr<Op>> &instructions) {
    for (auto &inst : instructions) {
      if (auto *mm = dynamic_cast<MatmulOp *>(inst.get())) {
        std::cout << "Found MatmulOp, MM unit = " << mm->getMMUnitNum() << "\n";
      } else if (auto *gtl =
                     dynamic_cast<GlobalToLocalMemCopyOp *>(inst.get())) {
        std::cout << "Found GlobalToLocalMemCopyOp\n";
      } else if (auto *ltg =
                     dynamic_cast<LocalToGlobalMemCopyOp *>(inst.get())) {
        std::cout << "Found LocalToGlobalMemCopyOp\n";
      }
    }
  }
};

#endif // SIMULATOR_H
