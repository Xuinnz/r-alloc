#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdint.h>

void init_heap();
void* r_alloc(uint32_t size);
void r_free(void* ptr);
void r_defrag();

#endif