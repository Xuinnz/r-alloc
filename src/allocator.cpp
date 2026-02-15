// src/allocator.cpp
#include "allocator.h"
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <time.h>

#define HEAP_SIZE (64 * 1024 * 1024)

// --- PROFILING STATE ---
struct AllocationMeta {
    uint64_t start_time;
    uint32_t site_id;
    size_t size;
};

// The Shadow Map: Ptr -> Metadata
std::map<void*, AllocationMeta> shadow_map;
FILE *log_file = NULL;

// Helper: Get nanoseconds
uint64_t get_nanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// --- ALLOCATOR STATE ---
struct Block {
    size_t size;
    int free;
    struct Block *next;
};

Block *free_list = NULL;
void *heap_start = NULL;

void init_heap() {
    // 1. Setup Heap
    heap_start = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    free_list = (Block*)heap_start;
    free_list->size = HEAP_SIZE - sizeof(Block);
    free_list->free = 1;
    free_list->next = NULL;

    // 2. Setup Logging (If Profiling)
    if (PROFILING_MODE) {
        log_file = fopen("training_data.csv", "w");
        if (log_file) {
            fprintf(log_file, "site_id,size,lifespan_ns\n");
            printf("--- PROFILING MODE ENABLED: Logging to training_data.csv ---\n");
        }
    }
}

void* r_alloc(size_t size, uint32_t site_id) {
    Block *current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            // Split logic (Simplified for brevity)
            if (current->size > size + sizeof(Block) + 32) {
                Block *new_block = (Block*)((char*)current + sizeof(Block) + size);
                new_block->size = current->size - size - sizeof(Block);
                new_block->free = 1;
                new_block->next = current->next;
                current->size = size;
                current->next = new_block;
            }
            current->free = 0;
            
            void* ptr = (void*)((char*)current + sizeof(Block));

            // --- PROFILING: Record Birth ---
            if (PROFILING_MODE) {
                AllocationMeta meta;
                meta.start_time = get_nanos();
                meta.site_id = site_id;
                meta.size = size;
                shadow_map[ptr] = meta;
            }

            return ptr;
        }
        current = current->next;
    }
    return NULL;
}

void r_free(void* ptr) {
    if (!ptr) return;

    // --- PROFILING: Record Death ---
    if (PROFILING_MODE) {
        if (shadow_map.count(ptr)) {
            AllocationMeta meta = shadow_map[ptr];
            uint64_t lifespan = get_nanos() - meta.start_time;
            
            // Log to CSV: site_id, size, lifespan
            if (log_file) {
                fprintf(log_file, "%u,%lu,%lu\n", meta.site_id, meta.size, lifespan);
                // Flush occasionally to ensure data is written if we crash
                // fflush(log_file); 
            }
            shadow_map.erase(ptr);
        }
    }

    Block *block = (Block*)((char*)ptr - sizeof(Block));
    block->free = 1;
    // (Coalescing logic omitted for brevity, keep your existing logic here)
}