#include "arena.h"
#include "allocator.h"
#include <stdio.h>
#include <string.h>

// Helper functions
int get_slab_index(size_t size) {
    if (size <= 32) return 0;
    if (size <= 64) return 1;
    if (size <= 128) return 2;
    if (size <= 256) return 3;
    if (size <= 512) return 4;
    if (size <= 1024) return 5;
    if (size <= 2048) return 6;
    if (size <= 4096) return 7;
    return -1; // Too big for slab
}

size_t get_class_size(int index){
    return 32 << index;
}

void init_slab_cache(arena_t* arena){
    for (int i = 0 ; i < SLAB_CLASS_COUNT; i++){
        arena->slab_cache.free_lists[i] = NULL;
    }
}

arena_t* create_arena(size_t size, lifetime_t policy){
    arena_t* new_arena = (arena_t*)r_alloc(sizeof(arena_t), 0); 
    if (!new_arena) return NULL; // Safety check
    new_arena->base = r_alloc(size, 0);
    new_arena->current = new_arena->base;
    new_arena->size = 0;
    new_arena->capacity = size;
    new_arena->policy = policy;

    if(policy == LIFETIME_INTERMEDIATE){
        init_slab_cache(new_arena);
    }

    return new_arena;
}

void* r_arena(arena_t* arena, size_t size, uint32_t site_id){
    // Align everything to 8 bytes minimum for safety
    size = (size + 7) & ~7;

    // STRATEGY 1: TRANSIENT (Bump Pointer)
    if(arena->policy == LIFETIME_TRANSIENT){
        if ((char*)arena->current + size <= (char*)arena->base + arena->capacity){
            void* ptr = arena->current;
            arena->current = (char*)arena->current + size;
            arena->size += size;
            return ptr;
        }
    }
    // STRATEGY 2: INTERMEDIATE (Slab Allocator)
    else if (arena->policy == LIFETIME_INTERMEDIATE){ 
        int index = get_slab_index(size);
        if(index == -1){
            return NULL;
        }
        
        slab_slot_t* free_node = arena->slab_cache.free_lists[index];

        if(free_node){
            arena->slab_cache.free_lists[index] = free_node->next;
            return (void*)free_node;
        } else {
            size_t class_size = get_class_size(index);
            int items_to_carve = 64; 
            size_t chunk_needed = class_size * items_to_carve;

            if ((char*)arena->current + chunk_needed > (char*)arena->base + arena->capacity) {
                 return NULL;
            }

            char* block_start = (char*)arena->current;
            arena->current = block_start + chunk_needed;
            arena->size += chunk_needed;

            for (int i = 1; i < items_to_carve; i++) {
                slab_slot_t* node = (slab_slot_t*)(block_start + (i * class_size));
                node->next = arena->slab_cache.free_lists[index];
                arena->slab_cache.free_lists[index] = node;
            }
            return (void*)block_start;
        }
    }
    // FALLBACK / PERSISTENT
    else {
        if ((char*)arena->current + size <= (char*)arena->base + arena->capacity){
            void* ptr = arena->current;
            arena->current = (char*)arena->current + size;
            arena->size += size;
            return ptr;
        }
    }
    return NULL;
}

void r_arena_free(arena_t* arena, void* ptr, size_t size) {
    if (!ptr) return;

    if (arena->policy != LIFETIME_INTERMEDIATE) {
        return; 
    }

    int index = get_slab_index(size);
    if (index == -1) return;

    slab_slot_t* node = (slab_slot_t*)ptr;

    // Push back to list
    node->next = arena->slab_cache.free_lists[index];
    arena->slab_cache.free_lists[index] = node;
}

void r_reset(arena_t* arena){
    if (arena && arena->policy == LIFETIME_TRANSIENT){
        arena->current = arena->base;
        arena->size = 0;
    }
    else if (arena && arena->policy == LIFETIME_INTERMEDIATE){
        init_slab_cache(arena);
        arena->current = arena->base;
        arena->size = 0;
    }
}

void r_destroy(arena_t* arena){
    if(arena){
        r_free(arena->base);
        r_free(arena);
    }
}