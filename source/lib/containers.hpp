#pragma once

#include "core/types.hpp"

struct Arena {
    u8* data;
    u32 gen;
    u64 len, cap;

    Arena(u64 size) : data{(u8*)SDL_malloc(size)}, gen{1}, len{0}, cap{size} {
        for (size_t i = 0; i < cap; i++) {
            data[i] = 0xCD;
        }
    }

    template <typename T = u8> T* Alloc(u64 count = 1) {
        if (len + sizeof(T) * count > cap) {
            WARN("Permanent memory full");
            return nullptr;
        }

        T* result = (T*)&data[len];
        len += sizeof(T) * count;
        return result;
    }

    struct ArenaScope {
        Arena* arena;
        u64    mark;

        ~ArenaScope() { arena->len = mark; }
    };

    ArenaScope Scope() { return ArenaScope{.arena = this, .mark = this->len}; }
};

Arena& Perm();
Arena& Temp();

template <typename T> struct Array {
    inline static T Empty = {};

    T*  data;
    u64 len;
    u64 cap;

    Array<T>() { WARN("Empty array initialized"); }
    Array<T>(T* _data, u64 _len, u64 _cap) : data{_data}, len{_len}, cap{_cap} {}

    ~Array<T>() = default;

    static Array<T> New(u64 size, Arena& arena = Perm()) {
        return Array<T>(arena.Alloc<T>(size), 0, size);
    }

    void Push(T& element) {
        if (len >= cap) {
            WARN("Array length exceeded");
            return;
        }

        data[len] = element;
        len++;
    }

    void Push(T const& element) {
        if (len >= cap) {
            return;
        }

        data[len] = element;
        len++;
    }

    T& Pop() {
        if (len == 0) {
            WARN("Array empty");
            return Array<T>::Empty;
        }

        len -= 1;
        return data[len];
    }

    T& Last() {
        if (len == 0) {
            WARN("Array empty");
            return Array<T>::Empty;
        }

        return data[len - 1];
    }

    T& operator[](u64 id) {
        if (id >= cap) {
            WARN("Array length exceeded");
            return Array<T>::Empty;
        }
        return data[id];
    }

    // -------- Iterator support --------
    T* begin() { return data; }
    T* end() { return data + len; }

    const T* begin() const { return data; }
    const T* end() const { return data + len; }
};

struct String : Array<u8> {
    operator cstr() { return (cstr)(this->data); }
    String(cstr str) : Array<u8>((u8*)(str), strlen(str), len) {}
};

template <typename T> struct DynArray : Array<T> {
    Arena*       arena;
    DynArray<T>* next;

    DynArray(Arena& _arena = Perm()) : Array<T>{}, arena{&_arena}, next{nullptr} {
        WARN("Empty array initialized");
    }

    DynArray(T* _data, u64 _len, u64 _cap, Arena& _arena)
        : Array<T>{_data, _len, _cap, _arena, nullptr}, arena{&_arena}, next{nullptr} {}

    ~DynArray<T>()                                   = default;
    DynArray<T>& operator=(DynArray<T> const& array) = default;

    static DynArray<T> New(u64 size, Arena& arena = Perm()) {
        return DynArray<T>(arena.Alloc<T>(size), 0, size, arena);
    }

    void Push(T& element) {
        if (this->len >= this->cap) {
            if (next) {
                next->Push(element);
                return;
            }

            Resize();
            Push(element);
            return;
        }

        this->data[this->len] = element;
        this->len++;
    }

    void Push(T const& element) {
        if (this->len >= this->cap) {
            if (next) {
                next->Push(element);
                return;
            }

            Resize();
            Push(element);
            return;
        }

        this->data[this->len] = element;
        this->len++;
    }

    void Resize() {
        INFO("Resizing array");
        next  = arena->Alloc<DynArray<T>>();
        *next = DynArray<T>::New(this->cap, *arena);
    }

    T& Pop() {
        if (next && next->len > 0) {
            return next->Pop();
        }

        if (this->len == 0) {
            SDL_Log("[WARNING] DynArray length exceeded\n");
            return DynArray<T>::Empty;
        }

        this->len -= 1;
        return this->data[this->len];
    }

    T& Last() {
        if (next && next->len > 0) {
            return next->Last();
        }

        if (this->len == 0) {
            SDL_Log("[WARNING] DynArray is null\n");
            return DynArray<T>::Empty;
        }

        return this->data[this->len - 1];
    }

    T& operator[](u64 id) {
        if (id >= this->cap) {
            return next ? (*next)[id - this->cap] : DynArray<T>::Empty;
        }
        return this->data[id];
    }

    // -------- Iterator support --------
    T* begin() { return this->data; }
    T* end() { return this->data + this->len; }

    const T* begin() const { return this->data; }
    const T* end() const { return this->data + this->len; }
};

template <typename T, unsigned int N> struct StackArray {
    T   data[N];
    u64 len;
    u64 cap = N;

    void Push(T& element) {
        if (len >= N) {
            WARN("StackArray length exceeded");
            return;
        }

        data[len] = element;
        len++;
    }

    T& Pop() {
        if (len == 0) {
            WARN("StackArray is empty");
            return data[0];
        }

        len -= 1;
        return data[len];
    }

    T& Last() {
        if (len == 0) {
            WARN("StackArray is empty");
            return data[0];
        }

        return data[len - 1];
    }

    T& operator[](u64 id) {
        if (id >= cap) {
            WARN("StackArray is empty");
            return data[0];
        }

        return data[id];
    }

    // -------- Iterator support --------
    T* begin() { return data; }
    T* end() { return data + len; }

    const T* begin() const { return data; }
    const T* end() const { return data + len; }
};