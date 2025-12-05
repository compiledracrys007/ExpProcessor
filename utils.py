from processor import *

def create_target(name, global_memory, 
                 number_of_compute_cores,
                 local_memory_per_core,
                 number_of_mm_units_per_core,
                 mm_tile_sizes):
    assert len(mm_tile_sizes) == 3, "mm_tile_sizes must be a tuple of (tile_m, tile_n, tile_k)"
    mm_units = []
    for i in range(number_of_mm_units_per_core):
        mm_unit = MatmulUnit(i, *mm_tile_sizes)
        mm_units.append(mm_unit)

    compute_cores = []
    for i in range(number_of_compute_cores):
        core = ComputeCore(i, local_memory_per_core, mm_units)
        compute_cores.append(core)

    
    target = Processor(name, global_memory, compute_cores)
    return target