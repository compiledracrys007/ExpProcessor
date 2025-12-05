#include <iostream>
#include <array>
#include "Utils.h"


constexpr size_t GLOBAL_MEMORY = 1024ULL * 1024 * 1024; // 1 GB
constexpr int NUMBER_OF_CORES = 4;
constexpr size_t LOCAL_MEMORY_PER_CORE = 512 * 1024;    // 512 KB
constexpr int NUMBER_OF_MM_UNITS_PER_CORE = 4;
constexpr int TILE_M = 32;
constexpr int TILE_N = 32;
constexpr int TILE_K = 32;

int main() {
    std::array<int, 3> tile_config = {TILE_M, TILE_N, TILE_K};

    Processor target = create_target(
        "ipu",
        GLOBAL_MEMORY,
        NUMBER_OF_CORES,
        LOCAL_MEMORY_PER_CORE,
        NUMBER_OF_MM_UNITS_PER_CORE,
        tile_config
    );

    // 3. Print device info
    std::cout << target.get_device_info() << std::endl;

    return 0;
}
