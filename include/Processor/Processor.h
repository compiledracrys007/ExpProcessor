#include <iostream>
#include <sstream> // Used for efficient string building
#include <string>
#include <vector>

#ifndef PROCESSOR_H
#define PROCESSOR_H

// 1. MatmulUnit Class
class MatmulUnit {
  int id;
  int tile_m;
  int tile_n;
  int tile_k;

public:
  // Constructor using member initializer list
  MatmulUnit(int id, int tile_m, int tile_n, int tile_k)
      : id(id), tile_m(tile_m), tile_n(tile_n), tile_k(tile_k) {}

  // get_info method marked 'const' as it doesn't modify the object
  std::string get_info() const {
    std::stringstream ss;
    ss << "Matmul Unit ID: " << id << ", with tile sizes M: " << tile_m
       << ", N: " << tile_n << ", K: " << tile_k;
    return ss.str();
  }
};

// 2. ComputeCore Class
class ComputeCore {
  int id;
  size_t local_memory; // size_t is best for memory sizes
  std::vector<MatmulUnit> matmul_units;

public:
  // Constructor
  ComputeCore(int id, size_t local_memory,
              const std::vector<MatmulUnit> &matmul_units)
      : id(id), local_memory(local_memory), matmul_units(matmul_units) {}

  std::string get_info() const {
    std::stringstream ss;
    ss << "Compute Unit ID: " << id << " with Local Memory: " << local_memory
       << " bytes";
    return ss.str();
  }

  const std::vector<MatmulUnit> &getMatmulUnits() const { return matmul_units; }

  int getLocalMemory() const { return local_memory; }
};

// 3. Processor Class
class Processor {
public:
  std::string name;
  size_t global_memory;
  std::vector<ComputeCore> compute_cores;

  // Constructor
  Processor(std::string name, size_t global_memory,
            const std::vector<ComputeCore> &compute_cores)
      : name(name), global_memory(global_memory), compute_cores(compute_cores) {
  }

  size_t getGlobalMemory() const { return global_memory; }

  int getNumberOfCores() const { return compute_cores.size(); }

  int getLocalMemoryPerCore() const {
    if (!compute_cores.empty()) {
      return compute_cores[0].getLocalMemory();
    }
    return 0; // or throw an exception if preferred
  }

  std::string getDeviceName() const { return name; }

  std::string get_device_info() const {
    std::stringstream ss;
    ss << "Device: " << name << "\n";
    ss << "Global Memory: " << global_memory << " bytes\n";
    ss << "Number of Compute Cores: " << compute_cores.size() << "\n";

    for (const auto &core : compute_cores) {
      ss << "  " << core.get_info() << "\n";
      for (const auto &mm_unit : core.getMatmulUnits()) {
        ss << "    " << mm_unit.get_info() << "\n";
      }
    }
    return ss.str();
  }
};

#endif // PROCESSOR_H