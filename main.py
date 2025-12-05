from utils import *

GLOBAL_MEMORY = 1024 * 1024 * 1024 # 1 GB in bytes
NUMBER_OF_CORES = 4
LOCAL_MEMORY_PER_CORE = 512 * 1024 # 512 KB in bytes
NUMBER_OF_MM_UNITS_PER_CORE = 4
TILE_M = 32
TILE_N = 32
TILE_K = 32


if __name__ == "__main__":
    target = create_target("ipu", GLOBAL_MEMORY, NUMBER_OF_CORES, LOCAL_MEMORY_PER_CORE, NUMBER_OF_MM_UNITS_PER_CORE, (TILE_M, TILE_N, TILE_K))
    print(target.get_device_info())
    


