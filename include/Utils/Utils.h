#include "Parser/Parser.h"
#include "Processor/Processor.h"
#include "Simulator/Simulator.h"
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifndef UTILS_H
#define UTILS_H

Processor createTarget(std::string name, size_t global_memory,
                       int number_of_compute_cores,
                       size_t local_memory_per_core,
                       int number_of_mm_units_per_core,
                       std::array<int, 3> mm_tile_sizes);

std::unique_ptr<Parser> getTargetParser(const Processor &processor);

std::unique_ptr<Simulator> getTargetSimulator(const Processor &processor);

Processor createEPUTarget();

#endif // UTILS_H