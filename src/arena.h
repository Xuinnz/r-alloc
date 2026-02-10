#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LIFETIME_TRANSIENT,
    LIFETIME_INTERMEDIATE,
    LIFETIME_PERSISTENT
} lifetime_t;

typedef struct arena_t {
    void* base;          // start of memory region
    void* current;       // current bump pointer
    size_t size;         // current usage (offset)
    size_t capacity;     // max size
    lifetime_t policy;   // the strategy this arena uses
} arena_t;

arena_t* create_arena(size_t size, lifetime_t policy);
void* r_arena(arena_t* arena, size_t size);
void r_reset(arena_t* arena);
void r_destroy(arena_t* arena);

#endif