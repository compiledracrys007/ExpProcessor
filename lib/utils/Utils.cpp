#include "Utils/Utils.h"
#include "Target/EPU/Parser/EPUAsmParser.h"
#include "Target/EPU/Simulator/EPUSimulator.h"

Processor createTarget(std::string name, size_t global_memory,
                       int number_of_compute_cores,
                       size_t local_memory_per_core,
                       int number_of_mm_units_per_core,
                       std::array<int, 3> mm_tile_sizes) {

  std::vector<MatmulUnit> mm_units;
  mm_units.reserve(
      number_of_mm_units_per_core); // Optimization: Reserve memory upfront

  for (int i = 0; i < number_of_mm_units_per_core; ++i) {
    // Unpack the array manually: [0]=M, [1]=N, [2]=K
    mm_units.emplace_back(i, mm_tile_sizes[0], mm_tile_sizes[1],
                          mm_tile_sizes[2]);
  }

  std::vector<ComputeCore> compute_cores;
  compute_cores.reserve(number_of_compute_cores);

  for (int i = 0; i < number_of_compute_cores; ++i) {
    compute_cores.emplace_back(i, local_memory_per_core, mm_units);
  }

  return Processor(name, global_memory, compute_cores);
}

std::unique_ptr<Parser> getTargetParser(const Processor &processor) {
  // Depending on the processor name, return the appropriate parser
  if (processor.getDeviceName() == "epu") {
    return std::make_unique<EPUAsmParser>(processor);
  }
  // Add more target parsers as needed

  throw std::runtime_error("Unsupported processor type for parser.");
}

std::unique_ptr<Simulator> getTargetSimulator(const Processor &processor) {
  // Depending on the processor name, return the appropriate simulator
  if (processor.getDeviceName() == "epu") {
    return std::make_unique<EPUSimulator>(processor);
  }
  // Add more target simulators as needed

  throw std::runtime_error("Unsupported processor type for simulator.");
}

Processor createEPUTarget() {
  constexpr size_t GLOBAL_MEMORY = 1024ULL * 1024 * 1024; // 1 GB
  constexpr int NUMBER_OF_CORES = 4;
  constexpr size_t LOCAL_MEMORY_PER_CORE = 512 * 1024; // 512 KB
  constexpr int NUMBER_OF_MM_UNITS_PER_CORE = 4;
  constexpr int TILE_M = 32;
  constexpr int TILE_N = 32;
  constexpr int TILE_K = 32;

  std::array<int, 3> tile_config = {TILE_M, TILE_N, TILE_K};

  auto target =
      createTarget("epu", GLOBAL_MEMORY, NUMBER_OF_CORES, LOCAL_MEMORY_PER_CORE,
                   NUMBER_OF_MM_UNITS_PER_CORE, tile_config);

  return target;
}