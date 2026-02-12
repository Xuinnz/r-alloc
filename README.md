# Custom Memory Manager with N-API Bridge

This project is a **learning project** aimed at understanding how memory management works by implementing a simple custom memory manager. The project uses **N-API (Node.js API)** to bridge the memory management functionality to JavaScript.

## Features

The memory manager provides the following core functions:

### `init_heap()`
Initializes the heap. This function sets up the memory pool that will be managed using `mmap`, ensuring efficient and direct memory management.

### `r_alloc(size)`
Allocates a block of memory of the specified size from the heap. The allocated memory is padded to the nearest 8-byte boundary for alignment. Returns a pointer or identifier for the allocated memory.

### `r_free(ptr)`
Frees the memory block associated with the given pointer or identifier, making it available for future allocations.

### `r_defrag()`
Merges all neighboring free memory blocks to reduce fragmentation and optimize memory usage.

## Purpose

The goal of this project is to:

- Gain a deeper understanding of how memory allocation and deallocation work at a low level.
- Learn how to implement a custom memory manager.
- Explore the use of **N-API** to create a bridge between C/C++ and JavaScript.

## How It Works

- The memory manager operates on a pre-allocated heap using `mmap`, providing direct control over memory allocation and deallocation.
- The N-API bridge allows JavaScript code to interact with the memory manager, enabling dynamic memory allocation and deallocation.

## Usage

1. **Initialize the heap**  
   Call `init_heap()` to set up the memory pool.

2. **Allocate memory**  
   Use `r_alloc(size)` to allocate memory blocks. The memory is padded to the nearest 8-byte boundary.

3. **Free memory**  
   Call `r_free(ptr)` to release memory blocks when they are no longer needed.

4. **Defragment memory**  
   Use `r_defrag()` to merge neighboring free memory blocks and reduce fragmentation.

## How to Run

1. **Rebuild the project using `node-gyp`**:
   ```bash
   node-gyp rebuild
   ```

2. **Test the memory manager**:
   ```bash
   node test.js
   ```

## Updates
### February 7, 2026
### Added Arena-Based Memory Allocation

To optimize memory allocation, the project added an **arena-based memory management**. This approach reduces fragmentation and improves allocation speed by pre-allocating memory chunks for specific lifetimes.

#### Arenas

The memory manager defines three arenas, each tailored for a specific lifetime:

- **Transient Arena**: For short-lived allocations.
- **Intermediate Arena**: For medium-lived allocations.
- **Persistent Arena**: For long-lived allocations.

#### Functions

- **`init_arenas()`**  
  Initializes the three arenas (`transient`, `intermediate`, and `persistent`) with a fixed chunk size.

- **`r_arena(size, lifetime)`**  
  Allocates memory from the specified arena based on the `lifetime` parameter. Lifetimes can be:
  - `LIFETIME_TRANSIENT`
  - `LIFETIME_INTERMEDIATE`
  - `LIFETIME_PERSISTENT`

- **`r_reset(lifetime)`**  
  Resets the specified arena with bump pointer, making all its memory available for reuse.

#### Benefits

- Faster allocation and deallocation due to the use of bump pointers.
- Reduced fragmentation by grouping allocations with similar lifetimes.
- Simplified memory management for temporary and persistent data.

### February 10, 2026
### Dynamic Arena Initialization and Enhanced Memory Management

#### Changes
- **`create_arena()`**: Refactored from static initialization to dynamic initialization. This allows for more flexible and efficient memory management.
- **`r_reset()`**: Updated to handle transient arenas more effectively by resetting the bump pointer and clearing the size.
- **`r_destroy()`**: Enhanced to properly free both the arena base and the arena structure itself, ensuring no memory leaks.

#### Results

Performance benchmarks comparing V8, Base Allocator, and Arena Allocator:

```
[1] Testing V8 (Native JavaScript)...
[2] Testing Base Allocator (rAlloc/rFree)...
[3] Testing Arena Allocator (Factory)...
┌─────────────────┬───────────┬─────────┬─────────────────┐
│     (index)     │ Time (ms) │ Req/Sec │ Mem Growth (MB) │
├─────────────────┼───────────┼─────────┼─────────────────┤
│    V8 Engine    │ '105.70'  │ '94604' │     '7.63'      │
│ Base Allocator  │ '223.04'  │ '44834' │     '0.13'      │
│ Arena Allocator │ '120.22'  │ '83181' │     '0.00'      │
└─────────────────┴───────────┴─────────┴─────────────────┘

>>> VERDICT <<<
Arena is 1.86x FASTER than Base Allocator
Arena is 0.88x FASTER than V8 Node.js Engine
Efficiency: Arena Memory Growth is minimal (No leaks detected).
```

### February 12, 2026
### Implemented Slab Allocator and Enhanced Testing

#### Changes
- **Slab Allocator**: Added `slab_cache` and `slab_slot` for efficient memory allocation. These structures optimize memory usage by grouping allocations of similar sizes and reducing fragmentation.
- **Heap Size Fix**: Corrected heap size calculation in `allocator.c` to ensure proper memory allocation and avoid overflows.

#### Testing
- **Backend Simulation**:
  ```
  === STARTING BACKEND SIMULATION ===

  Target: 100000 requests
  Time:        182.88 ms
  Throughput:  546802 req/sec
  Peak Users:  65
  Final Heap:  53.95 MB
  ```
- **Express Web Server**:
  ```
  Running 10s test @ http://localhost:3000/arena

  100 connections

  ┌─────────┬───────┬───────┬───────┬───────┬──────────┬─────────┬────────┐
  │ Stat    │ 2.5%  │ 50%   │ 97.5% │ 99%   │ Avg      │ Stdev   │ Max    │
  ├─────────┼───────┼───────┼───────┼───────┼──────────┼─────────┼────────┤
  │ Latency │ 42 ms │ 46 ms │ 61 ms │ 69 ms │ 47.72 ms │ 6.76 ms │ 125 ms │
  └─────────┴───────┴───────┴───────┴───────┴──────────┴─────────┴────────┘

  ┌───────────┬────────┬────────┬────────┬────────┬─────────┬─────────┬────────┐
  │ Stat      │ 1%     │ 2.5%   │ 50%    │ 97.5%  │ Avg     │ Stdev   │ Min    │
  ├───────────┼────────┼────────┼────────┼────────┼─────────┼─────────┼────────┤
  │ Req/Sec   │ 1,784  │ 1,784  │ 2,089  │ 2,199  │ 2,072.9 │ 110.16  │ 1,784  │
  ├───────────┼────────┼────────┼────────┼────────┼─────────┼─────────┼────────┤
  │ Bytes/Sec │ 432 kB │ 432 kB │ 506 kB │ 532 kB │ 502 kB  │ 26.6 kB │ 432 kB │
  └───────────┴────────┴────────┴────────┴────────┴─────────┴─────────┴────────┘
  ```

- **Observations**:
  - **Throughput**: 2,072 req/sec, approximately 16% slower on raw throughput compared to V8 due to the Bridge Tax.
  - **Stability**: Standard deviation of 6.76 ms, making it 32% more stable than V8.
  - **Worst-Case Performance**: Maximum latency of 125 ms, which is ~30% faster than V8's worst-case scenario.

## Disclaimer

This project is for **educational purposes only** and is not intended for production use. It is designed to help developers understand the basics of memory management and the interaction between C/C++ and JavaScript through N-API.