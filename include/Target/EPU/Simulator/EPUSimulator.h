#include "Simulator/Simulator.h"

#ifndef EPUSIMULATOR_H
#define EPUSIMULATOR_H

class EPUSimulator : public Simulator {
private:
  void executeGlobalToLocalMemCopy(GlobalToLocalMemCopyOp *op);

  void executeLocalToGlobalMemCopy(LocalToGlobalMemCopyOp *op);

  void executeMatmul(MatmulOp *op);

public:
  EPUSimulator(const Processor &proc) : Simulator(proc) {}

  void simulateInstructions(
      const std::vector<std::unique_ptr<Op>> &instructions) override;
};

#endif // EPUSIMULATOR_H