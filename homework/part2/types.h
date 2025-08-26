#pragma once
#include <stdint.h>
#include <stdio.h>

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

struct Arena;
static Arena *Perm;
static Arena *Temp;

struct Arena
{
    static constexpr u64 DEFAULT_PERM_ARENA_SIZE = 1024 * 1024 * 64;
    static constexpr u64 DEFAULT_TEMP_ARENA_SIZE = 1024 * 1024 * 64;
    u8 *data;
    u64 len, cap;

    Arena(u64 size) : data{(u8 *)malloc(size)}, len{0}, cap{size} {}

    static void Init(u64 permSize = DEFAULT_PERM_ARENA_SIZE, u64 tempSize = DEFAULT_TEMP_ARENA_SIZE)
    {
        Perm = new Arena(permSize);
        Temp = new Arena(tempSize);
    }

    ~Arena() = delete;
    Arena(const Arena &) = delete;
    Arena(Arena &&) = delete;
    Arena &operator=(const Arena &) = delete;
    Arena &operator=(Arena &&) = delete;

    template <typename T>
    T *Alloc(u64 count)
    {
        if (len + sizeof(T) * count > cap)
            return nullptr;

        T *result = (T *)&data[len];
        len += sizeof(T) * count;
        return result;
    }

    template <typename T>
    static T *PermAlloc(u64 count = 1)
    {
        return Perm->Alloc<T>(count);
    }

    template <typename T>
    static T *TempAlloc(u64 count = 1)
    {
        return Temp->Alloc<T>(count);
    }

    static void Clear()
    {
        Perm->len = 0;
        Temp->len = 0;
    }
};

template <typename T>
struct Array
{
    inline static T Empty = {};

    T *data;
    u64 len, cap;

    Array<T> *next; // for reallocing?

    static Array<T> New(u64 size)
    {
        // assert size > 0

        return Array<T>{
            .data = Arena::PermAlloc<T>(size),
            .len = 0,
            .cap = size,
            .next = nullptr,
        };
    }

    void Push(T &element)
    {
        if (len >= cap)
        {
            if (next)
            {
                next->Push(element);
                return;
            }

            Expand();
            Push(element);
            return;
        }

        data[len] = element;
        len++;
    }

    void Push(T const &element)
    {
        if (len >= cap)
        {
            if (next)
            {
                next->Push(element);
                return;
            }

            Expand();
            Push(element);
            return;
        }

        data[len] = element;
        len++;
    }

    void Expand()
    {
        printf("[INFO] Expanding array.\n");
        next = Arena::PermAlloc<Array<T>>();
        *next = Array<T>::New(cap);
    }

    T &Pop()
    {
        if (next && next->len > 0)
        {
            return next->Pop();
        }

        if (len == 0)
        {
            printf("[WARNING] Array length exceeded\n");
            return Array<T>::Empty;
        }

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (next && next->len > 0)
        {
            return next->Last();
        }

        if (len == 0)
        {
            printf("[WARNING] Array is null\n");
            return Array<T>::Empty;
        }

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            return next ? (*next)[id - cap] : Array<T>::Empty;
        }
        return data[id];
    }
};

typedef Array<u8> String;

struct StringBuilder
{
    inline static u8 Empty = {};

    u8 *data;
    u64 len, cap;

    static StringBuilder New(u64 size)
    {
        return StringBuilder{
            .data = Arena::PermAlloc<u8>(size),
            .len = 0,
            .cap = size,
        };
    }

    void Expand()
    {
        printf("[INFO] Expanding array.\n");
        // next = Arena::PermAlloc<Array<T>>();
        // *next = Array<T>::New(cap);
    }

    void Push(const u8 &element)
    {
        if (len >= cap)
        {
            Expand();
            Push(element);
            return;
        }

        data[len] = element;
        len++;
    }

    void Push(cstr &str)
    {
        for (u64 i = 0; str[i] != '\0'; i++)
        {
            Push(u8(str[i]));
        }
    }

    cstr ToCstr()
    {
        Push('\0');
        return cstr(data);
    }

    String ToString()
    {
        return String{
            .data = data,
            .len = len,
            .cap = cap,
        };
    }
};

template <typename T, unsigned int N>
struct FixedArray
{
    inline static T Empty = {};

    T data[N];
    u64 len, cap = N;

    void Push(T &element)
    {
        if (len >= N)
        {
            printf("[WARNING] Array length exceeded\n");
            return;
        }

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
        {
            printf("[WARNING] Array length exceeded\n");
            return FixedArray<T, N>::Empty;
        }

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (len == 0)
        {
            printf("[WARNING] Array length exceeded\n");
            return FixedArray<T, N>::Empty;
        }

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            printf("[WARNING] Array length exceeded\n");
            return FixedArray<T, N>::Empty;
        }

        return data[id];
    }
};
