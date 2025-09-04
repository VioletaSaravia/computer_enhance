#pragma once

#include "profiler.hpp"

Array<u8> ReadEntireFile(cstr path) {
    // THE BEST WAY TO AVOID ERRORS IS NOT TO HAVE THEM
    // ANTI-MODERN C++ BLOG POST
    PROFILE_FUNCTION();

    auto f = fopen(path, "rb");
    if (!f) return {};

    if (fseek(f, 0, SEEK_END) != 0) return {};

    u64 cap = ftell(f);
    if (cap == 0) return {}; // == ?
    PROFILE_ADD_BANDWIDTH(cap);

    auto result = Array<u8>::New(cap);
    if (!result.data) return {};
    result.len = result.cap;

    rewind(f);
    u64 rd = fread((void*)(result.data.ToConstPtr()), 1, (u64)result.cap, f);
    if (rd != (u64)result.cap) return {};

    return result;
}