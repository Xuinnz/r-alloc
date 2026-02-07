#include "arena.h"
#include "allocator.h"
#include <stdio.h>

struct arena_t transient_arena;
struct arena_t intermediate_arena;
struct arena_t persistent_arena;

#define ARENA_CHUNK_SIZE 1024

//r_alloc for every instance of arena
void init_single_arena(struct arena_t* arena, size_t size){
    arena -> base = r_alloc(size);
    arena -> current = arena -> base;
    arena -> size = size;
}

void init_arenas(){
    init_heap();

    init_single_arena(&transient_arena, ARENA_CHUNK_SIZE);
    init_single_arena(&intermediate_arena, ARENA_CHUNK_SIZE);
    init_single_arena(&persistent_arena, ARENA_CHUNK_SIZE);

    printf("3 Arenas Initialized");
}

void* r_arena(size_t size, lifetime_t lifetime){
    struct arena_t* target = NULL;

    switch(lifetime){
        case LIFETIME_TRANSIENT:
            target = &transient_arena;
            break;
        case LIFETIME_INTERMEDIATE:
            target = &intermediate_arena;
            break;
        case LIFETIME_PERSISTENT:
            target = &persistent_arena;
            break;
    }
    //padding
    size = (size + 7) & ~7;
    if((char*) target -> current + size <= (char*)target->base + target->size){
        void* ptr = target -> current;
        target -> current = (char*)target -> current + size;
        return ptr;
    }
    printf("Arena Overflow! Lifetime: %d", lifetime);
    return NULL;
}

void r_reset(lifetime_t lifetime){
    struct arena_t* target = NULL;
    switch(lifetime){
        case LIFETIME_TRANSIENT:
            target = &transient_arena;
            break;
        case LIFETIME_INTERMEDIATE:
            target = &intermediate_arena;
            break;
        case LIFETIME_PERSISTENT:
            target = &persistent_arena;
            break;
    }
    if (target){
        target -> current = target -> base;
        printf("Arena Reset: Lifetime %d\n", lifetime);
    }
}


