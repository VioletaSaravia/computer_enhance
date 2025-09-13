#pragma once

#include "lib/types.h"

typedef struct Arena {
    u8* data;
    u64 len, cap;
} Arena;

Arena NewArena(u64 size) {
    return (Arena){
        .data = SDL_malloc(size),
        .len  = 0,
        .cap  = size,
    };
}

void* Alloc(Arena* arena, u64 size) {
    arena->len += size;
    return arena->len < arena->cap ? (void*)&arena->data[arena->len] : NULL;
}

#define SCOPE(arena) DEFER(EndScope) ArenaScope _scope = Scope(arena)
typedef struct ArenaScope {
    Arena* arena;
    u64    mark;
} ArenaScope;

ArenaScope Scope(Arena* arena) {
    return (ArenaScope){.arena = arena, .mark = arena->len};
}

void EndScope(ArenaScope scope) {
    scope.arena->len = scope.mark;
}

typedef struct Array {
    u8* data;
    u64 len;
} Array;