# Target Description - EPU 4-core MatMul Accelerator ISA

**Version:** 1.0

**Overview**

This document describes the target architecture and instruction set for a small tiled matrix‑multiply accelerator with: 4 cores, each core containing 4 independent 32×32 matmul units and one Local Memory (512 KiB). There is a single Global Memory (1 GiB) accessible to all cores. The ISA exposes explicit copy instructions between Global and Local memory and a `matmul` instruction that operates on 32×32 float tiles held inside Local Memory.

---

## Table of Contents

1. Architecture summary
2. Addressing & memory model
3. Data types & tile size
4. Slice grammar and semantics
5. Instruction set (syntax, semantics, latency/throughput notes)
6. Examples

---

## 1. Architecture summary

* **Cores:** 4 (core IDs 0..3)
* **Matmul units per core:** 4 (matmul unit IDs 0..3). Each matmul unit executes an independent 32×32 single-precision (float32) matrix multiply: C := (A × B) or C += (A × B) when `accumulator=True`.
* **Global Memory:** 1 GiB, byte addressable, unified, accessible by all cores via `cp_global_to_local` / `cp_local_to_global` instructions.
* **Local Memory per core:** 512 KiB (private to the core). Local memory is byte addressable and must contain the tiles used by `matmul`.
* **Instruction model:** Very small explicit micro‑ISA with explicit DMA-like copy instructions and compute instructions that reference local memory slices by base & 2D slice ranges.

---

## 2. Addressing & memory model

* **Global addresses:** 64‑bit byte addresses (implementation may choose 32‑bit for simulation). Global memory holds large matrices, constants, buffers.
* **Local addresses:** Offsets into the core's local memory (base 0 for each core). Local addresses are 32‑bit offsets counted in bytes.
* **Endianness:** Little‑endian (default for float32 layout). Specify otherwise if a target differs.
* **Access granularity:** Copies transfer contiguous blocks of memory described by 2D slices. The system should ensure proper alignment when required by DMA engine.

---

## 3. Data types & tile size

* **Primary compute type:** `float32` (32‑bit IEEE 754). Future extensions may support `float16`, `bfloat16` or integer types.
* **Matmul tile size:** 32 × 32. Each tile requires 32×32×4 = 4096 bytes.
* **Accumulator semantics:** `matmul` supports `accumulator=True` which performs C := C + A×B (reads existing C tile from local memory before adding). `accumulator=False` stores result into C (overwriting previous contents).

---

## 4. Slice grammar and semantics

A **slice** identifies a 2D logical subarray inside a flat memory region. It contains:

* `base` — base byte address (global) or base offset in local memory.
* `dim1` (columns) — `start:end:stride` form, where `start` inclusive, `end` exclusive, `stride` in elements.
* `dim0` (rows) — `start:end:stride` form, contiguous dimension (fastest varying) — note: the spec below treats Dim0 as contiguous dimension.

**Notation (assembly-friendly)**

```
<base, [dim0_start:dim0_end:dim0_stride], [dim1_start:dim1_end:dim1_stride]>
```

Example for a full 32×32 tile at global base `A`:

```
<A, [0:32:1], [0:32:1]>
```

**Semantics**

* The number of rows = `ceil((dim0_end - dim0_start)/dim0_stride)` and number of columns = `ceil((dim1_end - dim1_start)/dim1_stride)`.
* The total number of elements = rows × columns. For a valid `matmul` tile, these must be exactly 32 × 32.
* Memory layout inside a slice uses row-major ordering (dim1 is inner/contiguous if that matches user expectation—see example and assembler convention). The slice representation allows regular strided copies.
* `stride` is expressed in *elements* (not bytes). Element size is determined by the data type (e.g., float32 = 4 bytes).

---

## 5. Instruction set

All instructions are atomic from the ISA viewpoint (no sub-instruction visibility). The assembler/scheduler must ensure correctness for overlapping copies and compute when necessary.

### 5.1 cp_global_to_local

**Syntax**

```
cp_global_to_local <global_src_slice>, core=<core_id>, <local_dst_slice>
```

**Semantics**

* Perform a 2D copy from `global_src_slice` (Global Memory) to `local_dst_slice` (Local memory of `core_id`).
* The source and destination element counts must match.
* Data type implied by source (float32 for current ISA).
* This is a DMA-style copy; the latency depends on size and memory subsystem. The instruction completes only when the copy is committed to local memory and visible to subsequent `matmul` instructions on the target core.

**Notes**

* Implementations may allow overlapping source slices for different cores but must ensure coherence.
* Stride support allows tiling and submatrix extraction without host-side packing.

### 5.2 cp_local_to_global

**Syntax**

```
cp_local_to_global core=<core_id>, <local_src_slice>, <global_dst_slice>
```

**Semantics**

* Copy from `local_src_slice` (local memory on `core_id`) to `global_dst_slice` (global memory). Source and destination element counts must match.
* Completes when written to global memory and visible to other masters.

### 5.3 matmul

**Syntax**

```
matmul core=<core_id>, matmul_unit=<unit_id>, <local_slice_A> <local_slice_B> <local_slice_C>, accumulator=(True|False)
```

**Semantics**

* Read A and B tiles from `local_slice_A` and `local_slice_B` inside the local memory of `core_id`.
* Multiply A and B as 32×32 float matrices and write result into `local_slice_C`.
* If `accumulator=True`, perform `local_slice_C := local_slice_C + (A×B)` (read-modify-write). If `False`, write result (overwrite) into `local_slice_C`.
* The `matmul_unit` parameter selects one of the 4 independent 32×32 compute engines in the core.
* The instruction is synchronous from the ISA point of view and must be fenced w.r.t. local memory accesses: subsequent instructions that read the `local_slice_C` must wait until `matmul` completes.

**Constraints**

* A, B, C tiles must each be valid 32×32 tiles (exact sizes). The assembler should enforce or pad appropriately.
* Matmul units are fully independent. Concurrent `matmul` instructions may execute on different units in the same core, provided they access disjoint local memory slices (or semantics for overlapping writes must be explicitly managed by the assembler using `accumulator` semantics).

---

## 6. Examples

**Single tile multiply (pseudocode assembly)**

```
# Copy A (32x32) to core 0 local offset 0
cp_global_to_local <A, [0:32:1], [0:32:1]>, core=0, <0, [0:32:1], [0:32:1]>
# Copy B (32x32) to core 0 local offset 4096
cp_global_to_local <B, [0:32:1], [0:32:1]>, core=0, <4096, [0:32:1], [0:32:1]>
# Compute: unit 0 computes C = A x B and writes to local offset 8192
matmul core=0, matmul_unit=0, <0, [0:32:1], [0:32:1]>, <4096, [0:32:1], [0:32:1]>, <8192, [0:32:1], [0:32:1]>, accumulator=False
# Copy result back to Global
cp_local_to_global core=0, <8192, [0:32:1], [0:32:1]>, <C, [0:32:1], [0:32:1]>
```

---

## Change log

* 1.0 — Initial description covering core features, instruction syntax and examples.

---

## 2.0 — Added parallel execution block ISA (`start_parallel`, `end_parallel`)

Version 2.0 introduces two new structural ISA pseudo-operations:

- `start_parallel`
- `end_parallel`

### Purpose

These operations define **parallel execution regions** in the program.  
Any instructions appearing between `start_parallel` and `end_parallel` are considered **independent parallel tasks** that the hardware or simulator may dispatch to different cores or threads.

This extension allows the assembler/scheduler to express parallelism **explicitly**, instead of relying solely on driver-side scheduling or implicit analysis.

---

### Semantics

#### `start_parallel`
- Marks the beginning of a **parallel region**.
- All **top-level instructions** inside this region are eligible for **concurrent dispatch**.
- Example: four independent `cp_global_to_local` instructions or four independent `matmul` instructions can be executed in parallel across the 4 cores.

#### `end_parallel`
- Marks the end of the region and acts as an **implicit join/fence**.
- All parallel operations must complete before the next instruction **outside** the region begins execution.
- No ordering is guaranteed within the parallel block unless implied by:
  - explicit data dependencies,
  - overlapping local-memory slices,
  - accumulator semantics.

#### Additional rules
- **Nested parallel regions are not allowed** (unsupported in version 2.0).
- Multiple disjoint parallel regions **may appear sequentially**.

---

### Rationale

Before version 2.0, parallelism was inferred indirectly by:
- Scheduling separate instructions to different cores, or
- Relying on the simulator/runtime to detect independent operations.

This was error-prone. The new constructs provide:

- **Precise visibility** into which operations may legally execute in parallel.
- **Simpler simulation runtimes**: they only spawn worker threads for the defined parallel region.
- **Deterministic synchronization**: `end_parallel` always acts as a join point.

---

### Example usage

```asm
start_parallel
  cp_global_to_local <A0_tile>, core=0, <dst0>
  cp_global_to_local <A1_tile>, core=1, <dst1>
  cp_global_to_local <A2_tile>, core=2, <dst2>
  cp_global_to_local <A3_tile>, core=3, <dst3>
end_parallel