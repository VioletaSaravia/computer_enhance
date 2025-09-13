#pragma once

#define GEN(...)

#include <SDL3/SDL.h>

#define COL_RESET "\033[0m"
#define COL_INFO  "\033[32m"         // Green
#define COL_WARN  "\033[33m"         // Yellow
#define COL_ERROR "\033[31m"         // Red
#define COL_FATAL "\033[41m\033[97m" // White on Red background

typedef const char* cstr;

#define LIST_VAR "\n\t> "

#define INFO(msg, ...)                                                                             \
    SDL_Log(COL_INFO "[INFO]" COL_RESET "  [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define WARN(msg, ...)                                                                             \
    SDL_Log(COL_WARN "[WARN]" COL_RESET "  [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define ERR(msg, ...)                                                                              \
    SDL_Log(COL_ERROR "[ERROR]" COL_RESET " [%s] " msg "\n", __func__, ##__VA_ARGS__)
#define FATAL(msg, ...)                                                                            \
    do {                                                                                           \
        SDL_Log(COL_FATAL "[FATAL]" COL_RESET " [%s:%d] " msg "\n",                                \
                __func__,                                                                          \
                __LINE__,                                                                          \
                ##__VA_ARGS__);                                                                    \
        abort();                                                                                   \
    } while (0);

void SDLInfo() {
    INFO("%s", SDL_GetError());
}

void SDLWarn() {
    WARN("%s", SDL_GetError());
}

void SDLError() {
    ERR("%s", SDL_GetError());
}

void SDLFatal() {
    FATAL("%s", SDL_GetError());
}

#define global   static
#define persist  static
#define internal static

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float_t  f32;
typedef double_t f64;

// Strip name from variable in macro.
// @example StripName("Mem->foo.bar") == "bar"
// @example StripName("Mem->foo") == "foo"
cstr StripName(cstr var) {
    cstr strippedName = var;
    u64  len          = strlen(var);
    for (int i = len - 1; i >= 0; i--) {
        if (var[i] == '>' || var[i] == '.') {
            strippedName = &var[i + 1];
            break;
        }
    }

    return strippedName;
}

bool IsZero(f32 value) {
    f32 epsilon = 0.001f;
    return fabs(value) < epsilon;
}

u32 RandInit(u32 seed) {
    SDL_srand(seed);
    INFO("Initialized random seed:\t%llu", seed);
    return seed;
}

typedef struct v2 {
    f32 x, y;
} v2;

typedef struct v3 {
    f32 x, y, z;
} v3;

typedef struct v4 {
    f32 x, y, z, w;
} v4;

inline v2 v2Add(v2 a, v2 b) {
    return (v2){a.x + b.x, a.y + b.y};
}

inline v2 v2Sub(v2 a, v2 b) {
    return (v2){a.x - b.x, a.y - b.y};
}

inline v2 v2Scale(v2 a, f32 s) {
    return (v2){a.x * s, a.y * s};
}

inline v3 v3Add(v3 a, v3 b) {
    return (v3){a.x + b.x, a.y + b.y, a.z + b.z};
}

inline v3 v3Sub(v3 a, v3 b) {
    return (v3){a.x - b.x, a.y - b.y, a.z - b.z};
}

inline v3 v3Scale(v3 a, f32 s) {
    return (v3){a.x * s, a.y * s, a.z * s};
}

inline v4 v4Add(v4 a, v4 b) {
    return (v4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

inline v4 v4Sub(v4 a, v4 b) {
    return (v4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

inline v4 v4Scale(v4 a, f32 s) {
    return (v4){a.x * s, a.y * s, a.z * s, a.w * s};
}

inline f32 v2LenSq(v2 v) {
    return v.x * v.x + v.y * v.y;
}

inline f32 v2Len(v2 v) {
    return sqrt(v2LenSq(v));
}

inline f32 v3LenSq(v3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline f32 v3Len(v3 v) {
    return sqrt(v3LenSq(v));
}

inline f32 v4LenSq(v4 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline f32 v4Len(v4 v) {
    return sqrt(v4LenSq(v));
}

inline v3 v3Cross(v3 a, v3 b) {
    return (v3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

#if defined(__GNUC__) || defined(__clang__)
#define DEFER(func) __attribute__((cleanup(func)))
#else
#define DEFER(func)
#endif

#define PI  3.14159265358979323846
#define TAU PI * 2

#define KB(val) val * 1024
#define MB(val) KB(val) * 1024
#define GB(val) MB(val) * 1024
#define TB(val) GB(val) * 1024