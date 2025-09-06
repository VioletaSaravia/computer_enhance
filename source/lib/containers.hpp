#pragma once

// #include "lib/os.hpp"
#include "lib/types.hpp"

static constexpr int TEMP_MEMORY_SIZE = MB(32);
static constexpr int PERM_MEMORY_SIZE = MB(1024);

struct Arena : ISingleton {
    u8*       data;
    u32       gen;
    u64       len;
    const u64 cap;

    Arena(u64 size) : data{(u8*)SDL_malloc(size)}, gen{1}, len{0}, cap{size} {}

    static Arena& Perm(u64 initSize = PERM_MEMORY_SIZE) {
        static Arena arena(initSize);
        return arena;
    }

    static Arena& Temp(u64 initSize = TEMP_MEMORY_SIZE) {
        static Arena arena(initSize);
        return arena;
    }

    template <typename T> struct Handle {
        u32 gen, idx;

        Handle() : gen{0}, idx{0} {}
        Handle(T* ptr) : gen{Arena::Perm().gen}, idx{u32((u8*)ptr - (u8*)Arena::Perm().data)} {}

        T*       ToPtr() { return (T*)(&Arena::Perm().data[this->idx]); }
        const T* ToConstPtr() const { return (const T*)(&Arena::Perm().data[this->idx]); }
        T&       operator*() { return *ToPtr(); }
        T*       operator->() { return ToPtr(); }

        // TODO MultiHandle<T> ?
        T& operator[](size_t index) { return *(ToPtr() + index); }
        operator T*() { return ToPtr(); }
        operator const T*() const { return (const T*)(&Arena::Perm().data[this->idx]); }

        bool NotNull() const { return bool(this); }
    };

    template <typename T> Handle<T> Alloc(u64 count = 1) {
        if (len + sizeof(T) * count > cap) {
            WARN("Permanent memory full");
            return nullptr;
        }

        T* result = (T*)&data[len];
        len += sizeof(T) * count;
        return Handle(result);
    }

    struct ArenaScope {
        Arena* arena;
        u64    mark;

        ~ArenaScope() { arena->len = mark; }
    };

    ArenaScope Scope() { return ArenaScope{.arena = this, .mark = this->len}; }

    void Clear() { len = 0; }
};

template <typename T> using Handle = Arena::Handle<T>;

template <typename T> struct Array {
    inline static T Empty = {};

    Handle<T> data;
    u64       len;
    u64       cap;

    Handle<Array<T>> next;

    Array<T>() { WARN("Empty array initialized"); }
    Array<T>(Handle<T> _data, u64 _len, u64 _cap) : data{_data}, len{_len}, cap{_cap} {}

    static Array<T> New(u64 size) { return Array<T>(Arena::Perm().Alloc<T>(size), 0, size); }

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
        INFO("Resizing array");
        next  = Arena::Perm().Alloc<Array<T>>();
        *next = Array<T>::New(cap);
    }

    T& Pop() {
        if (next && next->len > 0) {
            return next->Pop();
        }

        if (len == 0) {
            SDL_Log("[WARNING] Array length exceeded\n");
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
            SDL_Log("[WARNING] Array is null\n");
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

// TODO fails because const strings are not in arena :/
struct String : Array<u8> {
    cstr Cstr() { return (cstr)(this->data.ToConstPtr()); }
    // String(cstr str) : Array<u8>{.data = (u8*)(str), .len = strlen(str), .cap = len} {}
};

// TODO empty case in [0] or...?
template <typename T, unsigned int N> struct StackArray {
    T   data[N];
    u64 len;
    u64 cap = N;

    void Push(T& element) {
        if (len >= N) {
            SDL_Log("[WARNING] StackArray length exceeded\n");
            return;
        }

        data[len] = element;
        len++;
    }

    T& Pop() {
        if (len == 0) {
            SDL_Log("[WARNING] StackArray is empty\n");
            return data[0];
        }

        len -= 1;
        return data[len];
    }

    T& Last() {
        if (len == 0) {
            SDL_Log("[WARNING] StackArray is empty\n");
            return data[0];
        }

        return data[len - 1];
    }

    T& operator[](u64 id) {
        if (id >= cap) {
            SDL_Log("[WARNING] StackArray is empty\n");
            return data[0];
        }

        return data[id];
    }
};