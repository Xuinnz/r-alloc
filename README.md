# Custom Memory Manager with N-API Bridge

This project is a **learning project** aimed at understanding how memory management works by implementing a simple custom memory manager. The project uses **N-API (Node.js API)** to bridge the memory management functionality to JavaScript.

## Features

The memory manager provides the following core functions:

### `init_heap(size)`
Initializes a heap of the specified size. This function sets up the memory pool that will be managed.

### `r_alloc(size)`
Allocates a block of memory of the specified size from the heap. Returns a pointer or identifier for the allocated memory.

### `r_free(ptr)`
Frees the memory block associated with the given pointer or identifier, making it available for future allocations.

## Purpose

The goal of this project is to:

- Gain a deeper understanding of how memory allocation and deallocation work at a low level.
- Learn how to implement a custom memory manager.
- Explore the use of **N-API** to create a bridge between C/C++ and JavaScript.

## How It Works

- The memory manager operates on a pre-allocated heap, simulating how memory is managed in low-level systems.
- The N-API bridge allows JavaScript code to interact with the memory manager, enabling dynamic memory allocation and deallocation.

## Usage

1. **Initialize the heap**  
   Call `init_heap(size)` to set up the memory pool.

2. **Allocate memory**  
   Use `r_alloc(size)` to allocate memory blocks.

3. **Free memory**  
   Call `r_free(ptr)` to release memory blocks when they are no longer needed.