#pragma once

#ifdef _WIN32
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#define _CRT_SECURE_NO_WARNINGS 1
#include <Windows.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#else

#include <stdlib.h>
#include <string.h>

#endif
#include <math.h>
#include <stdint.h>
#include <stdio.h>

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

struct v2 {
    f32 x, y;
};

struct v3 {
    f32 x, y, z;
};

#if defined(__GNUC__) || defined(__clang__)
#define DEFER(func) __attribute__((cleanup(func)))
#else
#define DEFER(func)
#endif

#define PI 3.14159265358979323846

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

#define PERM_MEMORY_SIZE MB(64)
#define TEMP_MEMORY_SIZE MB(8)

struct Arena {
    u8* data;
    u32 gen;
    u64 len, cap;

    static Arena& Perm(u64 initSize = PERM_MEMORY_SIZE) {
        static Arena arena(initSize);
        return arena;
    }

    static Arena& Temp(u64 initSize = TEMP_MEMORY_SIZE) {
        static Arena arena(initSize);
        return arena;
    }

    template <typename T> struct Handle {
        u32 gen, id;

        Handle() : gen{0}, id{0} {}
        Handle(T* ptr) : gen{Arena::Perm().gen}, id{u32((u8*)ptr - (u8*)Arena::Perm().data)} {}

        T* ToPtr() { return (T*)(&Arena::Perm().data[this->id]); }
        T& operator*() { return *ToPtr(); }
        T* operator->() { return ToPtr(); }
        T& operator[](size_t index) { return *(ToPtr() + index); }

        operator T*() { return ToPtr(); }
        operator const T*() const { return (const T*)(&Arena::Perm().data[this->id]); }

        operator bool() const { return gen != 0; }
        bool operator!() const { return gen == 0; }
        bool NotNull() const { return bool(this); }
    };

    Arena(u64 size) : data{(u8*)malloc(size)}, gen{1}, len{0}, cap{size} {}

    ~Arena()                       = default;
    Arena(const Arena&)            = delete;
    Arena(Arena&&)                 = delete;
    Arena& operator=(const Arena&) = delete;
    Arena& operator=(Arena&&)      = delete;

    template <typename T> Handle<T> Alloc(u64 count = 1) {
        if (len + sizeof(T) * count > cap) return nullptr;

        T* result = (T*)&data[len];
        len += sizeof(T) * count;
        return Handle(result);
    }

    void Clear() { len = 0; }
};

template <typename T> using Handle = Arena::Handle<T>;

template <typename T> struct Array {
    inline static T Empty = {};

    Handle<T> data;
    u64       len, cap;

    Handle<Array<T>> next; // for reallocing?

    static Array<T> New(u64 size) {
        // assert size > 0

        return Array<T>{
            .data = Arena::Perm().Alloc<T>(size),
            .len  = 0,
            .cap  = size,
            .next = nullptr,
        };
    }

    void Push(T& element) {
        if (len >= cap) {
            if (next.NotNull()) {
                next->Push(element);
                return;
            }

            Resize();
            Push(element);
            return;
        }

        data[len] = element;
        len++;
    }

    void Push(T const& element) {
        if (len >= cap) {
            if (next.NotNull()) {
                next->Push(element);
                return;
            }

            Resize();
            Push(element);
            return;
        }

        data[len] = element;
        len++;
    }

    void Resize() {
        printf("[INFO] Resizing array.\n");
        next  = Arena::Perm().Alloc<Array<T>>();
        *next = Array<T>::New(cap);
    }

    T& Pop() {
        if (next && next->len > 0) {
            return next->Pop();
        }

        if (len == 0) {
            printf("[WARNING] Array length exceeded\n");
            return Array<T>::Empty;
        }

        len -= 1;
        return data[len];
    }

    T& Last() {
        if (next && next->len > 0) {
            return next->Last();
        }

        if (len == 0) {
            printf("[WARNING] Array is null\n");
            return Array<T>::Empty;
        }

        return data[len - 1];
    }

    T& operator[](u64 id) {
        if (id >= cap) {
            return next ? (*next)[id - cap] : Array<T>::Empty;
        }
        return data[id];
    }
};

typedef Array<u8> String;

template <typename T, unsigned int N> struct FixedArray {
    T   data[N];
    u64 len;
    u64 cap = N;

    void Push(T& element) {
        if (len >= N) {
            printf("[WARNING] FixedArray length exceeded\n");
            return;
        }

        data[len] = element;
        len++;
    }

    T& Pop() {
        if (len == 0) {
            printf("[WARNING] FixedArray is empty\n");
            return data[0];
        }

        len -= 1;
        return data[len];
    }

    T& Last() {
        if (len == 0) {
            printf("[WARNING] FixedArray is empty\n");
            return data[0];
        }

        return data[len - 1];
    }

    T& operator[](u64 id) {
        if (id >= cap) {
            printf("[WARNING] FixedArray is empty\n");
            return data[0];
        }

        return data[id];
    }
};