#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LIFETIME_TRANSIENT,
    LIFETIME_INTERMEDIATE,
    LIFETIME_PERSISTENT
} lifetime_t;

struct arena_t{
    void* base;
    void *current;
    size_t size;
};

void init_arenas();
void* r_arena(size_t size, lifetime_t lifetime);
void r_reset(lifetime_t lifetime);

#endif