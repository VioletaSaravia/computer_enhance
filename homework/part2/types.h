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

#define PI 3.14159265358979323846

inline double Deg2Rad(double deg) { return deg * (PI / 180.0); }
inline double Rad2Deg(double rad) { return rad / (PI / 180.0); }


template <typename T>
struct Array
{
    T *data;
    u64 len, cap;

    ~Array()
    {
        if (this->data)
            free(this->data);
    }

    void Push(T &element)
    {
        if (len >= cap)
        {
            cap *= 2;
            data = realloc(data, sizeof(T) * cap);
        }

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
            abort();

        len -= 1;
        return data[len];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
            abort();
        return data[id];
    }
};

template <typename T, unsigned int N>
struct FixedArray
{
    T data[N];
    u64 len, cap = N;

    void Push(T &element)
    {
        if (len >= N)
            return;

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
            abort();

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (len == 0)
            abort();

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            abort();
        }

        return data[id];
    }
};

typedef Array<char> String;