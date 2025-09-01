#pragma once

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _WIN32
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#else

#include <stdlib.h>
#include <string.h>

#endif

#include "meta.h"

#define global   static
#define persist  static
#define internal static

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;
typedef float       f32;
typedef double      f64;
typedef const char* cstr;

META(ArrayType)
typedef union {
    struct {
        f32 x, y;
    };
    struct {
        f32 w, h;
    };
    f32 coords[2];
} v2;

v2 v2Add(v2 a, v2 b) {
    return (v2){a.x + b.x, a.y + b.y};
}

v2 v2Sub(v2 a, v2 b) {
    return (v2){a.x - b.x, a.y - b.y};
}

f32 v2Dot(v2 a, v2 b) {
    return a.x * b.x + a.y * b.y;
}

f32 v2Cross(v2 a, v2 b) {
    return a.x * b.y - a.y * b.x;
}

v2 v2ScalarAdd(v2 a, f32 b) {
    return (v2){a.x + b, a.y + b};
}

v2 v2ScalarSub(v2 a, f32 b) {
    return (v2){a.x - b, a.y - b};
}

v2 v2ScalarMult(v2 a, f32 b) {
    return (v2){a.x * b, a.y * b};
}

v2 v2ScalarDiv(v2 a, f32 b) {
    return (v2){a.x / b, a.y / b};
}

f32 v2LenSq(v2 v) {
    return v.x * v.x + v.y * v.y;
}

f32 v2Len(v2 v) {
    return sqrt(v2LenSq(v));
}

META(ArrayType)
typedef union {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    f32 coords[3];
} v3;

v3 v3Add(v3 a, v3 b) {
    return (v3){a.x + b.x, a.y + b.y, a.z + b.z};
}

v3 v3Sub(v3 a, v3 b) {
    return (v3){a.x - b.x, a.y - b.y, a.z - b.z};
}

f32 v3Dot(v3 a, v3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

v3 v3Cross(v3 a, v3 b) {
    return (v3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

v3 v3ScalarAdd(v3 a, f32 b) {
    return (v3){a.x + b, a.y + b, a.z + b};
}

v3 v3ScalarSub(v3 a, f32 b) {
    return (v3){a.x - b, a.y - b, a.z - b};
}

v3 v3ScalarMult(v3 a, f32 b) {
    return (v3){a.x * b, a.y * b, a.z * b};
}

v3 v3ScalarDiv(v3 a, f32 b) {
    return (v3){a.x / b, a.y / b, a.z / b};
}

f32 v3LenSq(v3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

f32 v3Len(v3 v) {
    return sqrt(v3LenSq(v));
}

META(ArrayType)
typedef union {
    struct {
        f32 x, y, z, w;
    };
    struct {
        f32 r, g, b, a;
    };
    f32 coords[4];
} v4;

v4 v4Add(v4 a, v4 b) {
    return (v4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

v4 v4Sub(v4 a, v4 b) {
    return (v4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

f32 v4Dot(v4 a, v4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

v4 v4ScalarAdd(v4 a, f32 b) {
    return (v4){a.x + b, a.y + b, a.z + b, a.w + b};
}

v4 v4ScalarSub(v4 a, f32 b) {
    return (v4){a.x - b, a.y - b, a.z - b, a.w - b};
}

v4 v4ScalarMult(v4 a, f32 b) {
    return (v4){a.x * b, a.y * b, a.z * b, a.w * b};
}

v4 v4ScalarDiv(v4 a, f32 b) {
    return (v4){a.x / b, a.y / b, a.z / b, a.w / b};
}

f32 v4LenSq(v4 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

f32 v4Len(v4 v) {
    return sqrt(v4LenSq(v));
}

void StubVectorOp(u8 _, ...) {
    printf("[ERROR] Vector operator called on invalid types\n");
}

#define V2(x, y)                                                                                                       \
    (v2) {                                                                                                             \
        x, y                                                                                                           \
    }
#define V3(x, y, z)                                                                                                    \
    (v3) {                                                                                                             \
        x, y, z                                                                                                        \
    }
#define V4(x, y, z, w)                                                                                                 \
    (v4) {                                                                                                             \
        x, y, z, w                                                                                                     \
    }

#define Add(a, b)                                                                                                      \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), v2: v2Add, f32: v2ScalarAdd, default: StubVectorOp),                                         \
        v3: _Generic((b), v3: v3Add, f32: v3ScalarAdd, default: StubVectorOp),                                         \
        v4: _Generic((b), v4: v4Add, f32: v4ScalarAdd, default: StubVectorOp),                                         \
        default: StubVectorOp)(a, b)

#define Sub(a, b)                                                                                                      \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), v2: v2Sub, f32: v2ScalarSub, default: StubVectorOp),                                         \
        v3: _Generic((b), v3: v3Sub, f32: v3ScalarSub, default: StubVectorOp),                                         \
        v4: _Generic((b), v4: v4Sub, f32: v4ScalarSub, default: StubVectorOp),                                         \
        default: StubVectorOp)(a, b)

#define Dot(a, b)                                                                                                      \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), v2: v2Dot, default: StubVectorOp),                                                           \
        v3: _Generic((b), v3: v3Dot, default: StubVectorOp),                                                           \
        v4: _Generic((b), v4: v4Dot, default: StubVectorOp),                                                           \
        default: StubVectorOp)(a, b)

#define Mult(a, b)                                                                                                     \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), f32: v2ScalarMult, default: StubVectorOp),                                                   \
        v3: _Generic((b), f32: v3ScalarMult, default: StubVectorOp),                                                   \
        v4: _Generic((b), f32: v4ScalarMult, default: StubVectorOp),                                                   \
        default: StubVectorOp)(a, b)

#define Div(a, b)                                                                                                      \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), f32: v2ScalarDiv, default: StubVectorOp),                                                    \
        v3: _Generic((b), f32: v3ScalarDiv, default: StubVectorOp),                                                    \
        v4: _Generic((b), f32: v4ScalarDiv, default: StubVectorOp),                                                    \
        default: StubVectorOp)(a, b)

#define Cross(a, b)                                                                                                    \
    _Generic((a),                                                                                                      \
        v2: _Generic((b), v2: v2Cross, default: StubVectorOp),                                                         \
        v3: _Generic((b), v3: v3Cross, default: StubVectorOp),                                                         \
        default: StubVectorOp)(a, b)

#define Len(v)   _Generic((v), v2: v2Len, v3: v3Len, v4: v4Len, default: StubVectorOp)(v)
#define LenSq(v) _Generic((v), v2: v2LenSq, v3: v3LenSq, v4: v4LenSq, default: StubVectorOp)(v)

typedef union {
    struct {
        u8 r, g, b, a;
    };
    u8 coords[4];
} Color;

// NOTE(violeta): Provides color palette support on vscode :O
#define rgba(r, g, b, a)                                                                                               \
    (Color) {                                                                                                          \
        r, g, b, (u8)(a * 255.0)                                                                                       \
    }

inline v4 ToV4(Color c) {
    return (v4){
        (f32)c.r / 255,
        (f32)c.g / 255,
        (f32)c.b / 255,
        (f32)c.a / 255,
    };
}

#if defined(__GNUC__) || defined(__clang__)
#define DEFER(func) __attribute__((cleanup(func)))
#else
#define DEFER(func)
#endif

#define PI  3.14159265358979323846
#define TAU 6.28318530717958647692

inline double Deg2Rad(double deg) {
    return deg * (PI / 180.0);
}
inline double Rad2Deg(double rad) {
    return rad / (PI / 180.0);
}

#define KB(bytes) bytes * 1024
#define MB(bytes) KB(bytes) * 1024
#define GB(bytes) MB(bytes) * 1024
#define TB(bytes) GB(bytes) * 1024

typedef struct {
    u8* data;
    u64 len, cap;
} Arena;

Arena ArenaNew(u64 cap) {
    return (Arena){
        .data = malloc(cap),
        .len  = 0,
        .cap  = cap,
    };
}

u8* ArenaAlloc(Arena* arena, u64 count) {
    if (arena->len + count > arena->cap) return NULL;

    u8* result = &arena->data[arena->len];
    arena->len += count;
    return result;
}

void ArenaClear(Arena* arena) {
    arena->len = 0;
}

typedef struct {
    u32 a, b;
} Foo;

typedef struct {
    FooNode* next;
    Foo      data;
} FooNode;

typedef struct {
    FooNode* data;
    FooNode* free;
    u64      len, cap;
} FooPool;

FooNode* FooPoolPush(FooPool* pool, Foo value) {
    FooNode* result = pool->free;

    if (result) {
        pool->free = pool->free->next;
        *result    = (FooNode){
               .data = value,
               .next = NULL,
        };
    } else {
        if (pool->len >= pool->cap) return NULL;

        result                  = &pool->data[pool->len];
        pool->data[pool->len++] = (FooNode){
            .data = value,
            .next = NULL,
        };
    }

    return result;
}

void FooPoolPop(FooPool* pool, FooNode* value) {
    value->next = pool->free;
    pool->free  = value;
}