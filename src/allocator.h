// src/allocator.h
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

// 1 training mode
// 0 for prod mode
#define PROFILING_MODE 1 

void init_heap();
void* r_alloc(size_t size, uint32_t site_id);
void r_free(void* ptr);

void flush_profiling_data();

#endif