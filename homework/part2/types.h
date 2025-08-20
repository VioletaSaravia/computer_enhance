#pragma once
#include <stdint.h>

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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef const char *cstr;

#if defined(_MSC_VER)
    #define DEFER(func)
#elif defined(__GNUC__) || defined(__clang__)
    #define DEFER(func) __attribute__((cleanup(func)))
#else
    #define DEFER(func)
#endif