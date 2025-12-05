#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <cassert> 
#include "Processor.h"

#ifndef UTILS_H
#define UTILS_H

Processor create_target(std::string name, 
                        size_t global_memory, 
                        int number_of_compute_cores,
                        size_t local_memory_per_core, 
                        int number_of_mm_units_per_core,
                        std::array<int, 3> mm_tile_sizes); 
#endif // UTILS_H