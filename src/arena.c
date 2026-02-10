#include "arena.h"
#include "allocator.h"
#include <stdio.h>

arena_t* create_arena(size_t size, lifetime_t policy){
    arena_t* new_arena = (arena_t*)r_alloc(sizeof(arena_t));
    new_arena -> base = r_alloc(size);
    new_arena -> current = new_arena -> base;
    new_arena -> size = 0;
    new_arena -> capacity = size;
    new_arena-> policy = policy;

    return new_arena;
}

void* r_arena(arena_t* arena, size_t size){
    size = (size + 7) & ~7;

    if(arena -> policy == LIFETIME_TRANSIENT){
        if ((char*)arena -> current + size <= (char*)arena -> base + arena -> capacity){
            void* ptr = arena -> current;
            arena -> current = (char*)arena -> current + size;
            arena -> size += size;
            return ptr;
        }
    }
    else{
        if ((char*)arena -> current + size <= (char*)arena -> base + arena -> capacity){
            void* ptr = arena -> current;
            arena -> current = (char*)arena -> current + size;
            arena -> size += size;
            return ptr;
        }
    }

    printf("Arena Overflow! Capacity: %lu\n", arena->capacity);
    return NULL;
}

void r_reset(arena_t* arena){
    if (arena && arena -> policy == LIFETIME_TRANSIENT){
        arena -> current = arena -> base;
        arena -> size = 0;
        printf("Arena Reset (Transient)");
    }
    else {
        printf("Warning: r_reset only for transient lifetimes");
    }
}

void r_destroy(arena_t* arena){
    if(arena){
        r_free(arena -> base);
        r_free(arena);
    }
}
