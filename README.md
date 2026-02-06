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

## Disclaimer

This project is for **educational purposes only** and is not intended for production use. It is designed to help developers understand the basics of memory management and the interaction between C/C++ and JavaScript through N-API.