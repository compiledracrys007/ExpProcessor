#include "Simulator/Simulator.h"
#include "Target/EPU/Asm/EPUOps.h"
#include <memory>

#ifndef EPUSIMULATOR_H
#define EPUSIMULATOR_H

class EPUSimulator : public Simulator {
private:
  void executeGlobalToLocalMemCopy(GlobalToLocalMemCopyOp *op);

  void executeLocalToGlobalMemCopy(LocalToGlobalMemCopyOp *op);

  void executeMatmul(MatmulOp *op);

public:
  EPUSimulator(const Processor &proc) : Simulator(proc) {}

  ~EPUSimulator() = default;

  void execute(Op *inst);

  void dispatchParallelInstructions(const std::vector<Op *> &insts);

  void simulateInstructions(
      const std::vector<std::unique_ptr<Op>> &instructions) override;
};

#endif // EPUSIMULATOR_H