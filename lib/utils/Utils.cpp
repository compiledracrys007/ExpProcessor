#include "Utils/Utils.h"

Processor create_target(std::string name, 
                        size_t global_memory, 
                        int number_of_compute_cores,
                        size_t local_memory_per_core, 
                        int number_of_mm_units_per_core,
                        std::array<int, 3> mm_tile_sizes) {

    std::vector<MatmulUnit> mm_units;
    mm_units.reserve(number_of_mm_units_per_core); // Optimization: Reserve memory upfront

    for (int i = 0; i < number_of_mm_units_per_core; ++i) {
        // Unpack the array manually: [0]=M, [1]=N, [2]=K
        mm_units.emplace_back(i, mm_tile_sizes[0], mm_tile_sizes[1], mm_tile_sizes[2]);
    }

    std::vector<ComputeCore> compute_cores;
    compute_cores.reserve(number_of_compute_cores);

    for (int i = 0; i < number_of_compute_cores; ++i) {
        compute_cores.emplace_back(i, local_memory_per_core, mm_units);
    }

    return Processor(name, global_memory, compute_cores);
}
