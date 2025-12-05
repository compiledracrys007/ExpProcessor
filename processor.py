class MatmulUnit:
    def __init__(self, id, tile_m, tile_n, tile_k):
        self.id = id
        self.tile_m = tile_m
        self.tile_n = tile_n
        self.tile_k = tile_k

    def get_info(self):
        return f"Matmul Unit ID: {self.id}, with tile sizes M: {self.tile_m}, N: {self.tile_n}, K: {self.tile_k}"

class ComputeCore:
    def __init__(self, id, local_memory, matmul_units):
        self.id = id
        self.local_memory = local_memory
        self.matmul_units = matmul_units

    def get_info(self):
        return f"Compute Unit ID: {self.id} with Local Memory: {self.local_memory} bytes"

class Processor:
    def __init__(self, name, global_memory, compute_cores):
        self.name = name
        self.global_memory = global_memory
        self.compute_cores = compute_cores

    
    def get_device_info(self):
        info = f"Device: {self.name}\n"
        info += f"Global Memory: {self.global_memory} bytes\n"
        info += f"Number of Compute Cores: {len(self.compute_cores)}\n"
        for core in self.compute_cores:
            info += f"  {core.get_info()}\n"
            for mm_unit in core.matmul_units:
                info += f"    {mm_unit.get_info()}\n"
        return info
