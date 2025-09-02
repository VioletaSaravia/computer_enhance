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

#include <assert.h>
#include <concepts>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

template <typename T, typename E = cstr> struct Result {
    bool ok;
    union {
        T value;
        E error;
    };

    ~Result() {
        if (ok) {
            value.~T();
        } else {
            error.~E();
        }
    }

    constexpr operator T() {
        if (ok)
            return value;
        else
            abort();
    }
    constexpr operator T&() {
        if (ok)
            return value;
        else
            abort();
    }

    static constexpr Result<T, E> Ok(T value) { return Result{.ok = true, .value = value}; }
    static constexpr Result<T, E> Err(E error) { return Result{.ok = false, .error = error}; }
};

#define global   static
#define persist  static
#define internal static

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float_t;
using f64 = double_t;

template <typename T>
concept Float = std::floating_point<T>;

template <Float T> constexpr bool IsZero(T value, T epsilon = 0.001f) {
    return fabs(value) < epsilon;
}

typedef const char* cstr;

struct v2 {
    f32 x, y;

    f32 LenSq() { return x * x + y * y; }
    f32 Len() { return sqrt(this->LenSq()); }

    f32 Cross(v2 b) { return x * b.y - y * b.x; }

    constexpr v2 operator+() const { return *this; }
    constexpr v2 operator-() const { return {-x, -y}; }

    constexpr v2 operator+(const v2& rhs) const { return {x + rhs.x, y + rhs.y}; }
    constexpr v2 operator-(const v2& rhs) const { return {x - rhs.x, y - rhs.y}; }
    constexpr v2 operator*(const v2& rhs) const { return {x * rhs.x, y * rhs.y}; }
    constexpr v2 operator/(const v2& rhs) const { return {x / rhs.x, y / rhs.y}; }

    constexpr v2 operator+(Float auto s) const { return {x + s, y + s}; }
    constexpr v2 operator-(Float auto s) const { return {x - s, y - s}; }
    constexpr v2 operator*(Float auto s) const { return {x * s, y * s}; }
    constexpr v2 operator/(Float auto s) const { return {x / s, y / s}; }

    constexpr v2& operator+=(const v2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    constexpr v2& operator-=(const v2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    constexpr v2& operator*=(const v2& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }
    constexpr v2& operator/=(const v2& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    constexpr v2& operator+=(Float auto s) {
        x += s;
        y += s;
        return *this;
    }
    constexpr v2& operator-=(Float auto s) {
        x -= s;
        y -= s;
        return *this;
    }
    constexpr v2& operator*=(Float auto s) {
        x *= s;
        y *= s;
        return *this;
    }
    constexpr v2& operator/=(Float auto s) {
        x /= s;
        y /= s;
        return *this;
    }

    constexpr friend v2 operator*(v2 v, float s) { return {v.x * s, v.y * s}; }
    constexpr friend v2 operator*(float s, v2 v) { return {v.x * s, v.y * s}; }
};

struct v3 {
    f32 x, y, z;

    f32 LenSq() { return x * x + y * y + z * z; }
    f32 Len() { return sqrt(this->LenSq()); }

    v3 Cross(v3 b) { return v3{y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x}; }

    constexpr v3 operator+() const { return *this; }
    constexpr v3 operator-() const { return {-x, -y, -z}; }

    constexpr v3 operator+(const v3& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    constexpr v3 operator-(const v3& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    constexpr v3 operator*(const v3& rhs) const { return {x * rhs.x, y * rhs.y, z * rhs.z}; }
    constexpr v3 operator/(const v3& rhs) const { return {x / rhs.x, y / rhs.y, z / rhs.z}; }

    constexpr v3 operator+(Float auto s) const { return {x + s, y + s, z + s}; }
    constexpr v3 operator-(Float auto s) const { return {x - s, y - s, z - s}; }
    constexpr v3 operator*(Float auto s) const { return {x * s, y * s, z * s}; }
    constexpr v3 operator/(Float auto s) const { return {x / s, y / s, z / s}; }

    constexpr v3& operator+=(const v3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    constexpr v3& operator-=(const v3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    constexpr v3& operator*=(const v3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    constexpr v3& operator/=(const v3& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    constexpr v3& operator+=(Float auto s) {
        x += s;
        y += s;
        z += s;
        return *this;
    }
    constexpr v3& operator-=(Float auto s) {
        x -= s;
        y -= s;
        z -= s;
        return *this;
    }
    constexpr v3& operator*=(Float auto s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    constexpr v3& operator/=(Float auto s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    constexpr friend v3 operator*(v3 v, float s) { return {v.x * s, v.y * s, v.z * s}; }
    constexpr friend v3 operator*(float s, v3 v) { return {v.x * s, v.y * s, v.z * s}; }
};

struct v4 {
    f32 x, y, z, w;

    f32 LenSq() { return x * x + y * y + z * z + w * w; }
    f32 Len() { return sqrt(this->LenSq()); }

    constexpr v4 operator+() const { return *this; }
    constexpr v4 operator-() const { return {-x, -y, -z, -w}; }

    constexpr v4 operator+(const v4& rhs) const { return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w}; }
    constexpr v4 operator-(const v4& rhs) const { return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
    constexpr v4 operator*(const v4& rhs) const { return {x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w}; }
    constexpr v4 operator/(const v4& rhs) const { return {x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w}; }

    constexpr v4 operator+(Float auto s) const { return {x + s, y + s, z + s, w + s}; }
    constexpr v4 operator-(Float auto s) const { return {x - s, y - s, z - s, w - s}; }
    constexpr v4 operator*(Float auto s) const { return {x * s, y * s, z * s, w * s}; }
    constexpr v4 operator/(Float auto s) const { return {x / s, y / s, z / s, w / s}; }

    constexpr v4& operator+=(const v4& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    constexpr v4& operator-=(const v4& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    constexpr v4& operator*=(const v4& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }
    constexpr v4& operator/=(const v4& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    constexpr v4& operator+=(Float auto s) {
        x += s;
        y += s;
        z += s;
        w += s;
        return *this;
    }
    constexpr v4& operator-=(Float auto s) {
        x -= s;
        y -= s;
        z -= s;
        w -= s;
        return *this;
    }
    constexpr v4& operator*=(Float auto s) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    constexpr v4& operator/=(Float auto s) {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    constexpr friend v4 operator*(v4 v, float s) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }
    constexpr friend v4 operator*(float s, v4 v) { return {v.x * s, v.y * s, v.z * s, v.w * s}; }
};

template <typename T>
concept Scalable = requires(T t, f32 s) {
    { t * s } -> std::same_as<T>;
    { s * t } -> std::same_as<T>;
};

template <Scalable T> struct Damped {
    T y, yd;

    Damped(T initial = {}) : y{initial}, yd{0} {}

    operator T() { return y; }
};

using f32d = Damped<f32>;
using v2d  = Damped<v2>;
using v3d  = Damped<v3>;
using v4d  = Damped<v4>;

#if defined(__GNUC__) || defined(__clang__)
#define DEFER(func) __attribute__((cleanup(func)))
#else
#define DEFER(func)
#endif

static constexpr Float auto PI  = 3.14159265358979323846;
static constexpr Float auto TAU = PI * 2;

struct Deg;

struct Rad {
    f32 value;

    constexpr Rad(f32 v = 0) : value(v) {}
    constexpr Rad(Deg deg); // defined after Deg
    constexpr Rad& operator=(f32 rhs) {
        value = rhs;
        return *this;
    }

    constexpr explicit operator f32() const { return value; }

    constexpr Rad operator+() const { return *this; }
    constexpr Rad operator-() const { return Rad{-value}; }

    constexpr Rad operator+(Rad rhs) const { return Rad{value + rhs.value}; }
    constexpr Rad operator-(Rad rhs) const { return Rad{value - rhs.value}; }
    constexpr Rad operator*(f32 s) const { return Rad{value * s}; }
    constexpr Rad operator/(f32 s) const { return Rad{value / s}; }

    constexpr Rad& operator+=(Rad rhs) {
        value += rhs.value;
        return *this;
    }
    constexpr Rad& operator-=(Rad rhs) {
        value -= rhs.value;
        return *this;
    }
    constexpr Rad& operator*=(f32 s) {
        value *= s;
        return *this;
    }
    constexpr Rad& operator/=(f32 s) {
        value /= s;
        return *this;
    }

    constexpr bool operator<(Rad rhs) const { return value < rhs.value; }
    constexpr bool operator<=(Rad rhs) const { return value <= rhs.value; }
    constexpr bool operator>(Rad rhs) const { return value > rhs.value; }
    constexpr bool operator>=(Rad rhs) const { return value >= rhs.value; }
};

struct Deg {
    f32 value;

    constexpr Deg(f32 v = 0) : value(v) {}
    constexpr Deg(Rad rad) : value(rad.value * (180.0f / PI)) {}
    constexpr Deg operator=(f32 rhs) {
        value = rhs;
        return *this;
    }

    constexpr Deg operator+() const { return *this; }
    constexpr Deg operator-() const { return Deg{-value}; }

    constexpr Deg operator+(Deg rhs) const { return Deg{value + rhs.value}; }
    constexpr Deg operator-(Deg rhs) const { return Deg{value - rhs.value}; }
    constexpr Deg operator*(f32 s) const { return Deg{value * s}; }
    constexpr Deg operator/(f32 s) const { return Deg{value / s}; }

    constexpr Deg& operator+=(Deg rhs) {
        value += rhs.value;
        return *this;
    }
    constexpr Deg& operator-=(Deg rhs) {
        value -= rhs.value;
        return *this;
    }
    constexpr Deg& operator*=(f32 s) {
        value *= s;
        return *this;
    }
    constexpr Deg& operator/=(f32 s) {
        value /= s;
        return *this;
    }

    constexpr bool operator<(Deg rhs) const { return value < rhs.value; }
    constexpr bool operator<=(Deg rhs) const { return value <= rhs.value; }
    constexpr bool operator>(Deg rhs) const { return value > rhs.value; }
    constexpr bool operator>=(Deg rhs) const { return value >= rhs.value; }
};

inline constexpr Rad::Rad(Deg deg) : value{deg.value / (180.0f / static_cast<f32>(PI))} {
}

static Deg bla = 32.0f;
static Rad ble = bla;

template <typename T>
concept Integral = std::integral<T>;

constexpr auto KB(Integral auto val) {
    return val * 1024;
}

constexpr auto MB(Integral auto val) {
    return KB(val) * 1024;
}

constexpr auto GB(Integral auto val) {
    return MB(val) * 1024;
}

constexpr auto TB(Integral auto val) {
    return GB(val) * 1024;
}

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

    Handle<Array<T>> next;

    static Array<T> New(u64 size) {
        assert(size > 0);

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