#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LIFETIME_TRANSIENT,
    LIFETIME_INTERMEDIATE,
    LIFETIME_PERSISTENT
} lifetime_t;

#define SLAB_CLASS_COUNT 8
#define SLAB_MIN_SIZE 32

typedef struct slab_slot_t{
    struct slab_slot_t*  next;
} slab_slot_t;

typedef struct {
    slab_slot_t* free_lists[SLAB_CLASS_COUNT];
} slab_cache_t;

typedef struct arena_t {
    void* base;          // start of memory region
    void* current;       // current bump pointer
    size_t size;         // current usage (offset)
    size_t capacity;     // max size
    lifetime_t policy;   // the strategy this arena uses
    slab_cache_t slab_cache; //if policy == intermediate
} arena_t;

arena_t* create_arena(size_t size, lifetime_t policy);
void* r_arena(arena_t* arena, size_t size);
void r_reset(arena_t* arena);
void r_destroy(arena_t* arena);

void r_arena_free(arena_t* arena, void* ptr, size_t size);

#endif