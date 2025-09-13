#pragma once

#include <stdio.h>

#include "lib/containers.h"
#include "lib/types.h"

Array ReadEntireFile(cstr path, Arena* arena) {
    if (!arena) return {0};
    Array result = {0};

    FILE* f = fopen(path, "rb");
    if (!f) return {};

    if (fseek(f, 0, SEEK_END) != 0) return {};

    u64 cap = ftell(f);
    if (cap == 0) return {0};

    result = {
        .data = (u8*)Alloc(arena, cap),
        .len  = cap,
    };
    if (!result.data) return {0};

    rewind(f);
    u64 rd = fread((void*)(result.data), 1, (u64)result.len, f);
    if (rd != (u64)result.len) return {0};

    return result;
}

bool WriteToFile(Array data, cstr path) {
    auto file = fopen(path, "wb");
    if (!file) return false;

    u64 written = fwrite(data.data, sizeof(u8), data.len, file);

    fclose(file);
    return written == data.len;
}

bool CopyEntireFile(cstr from, cstr to, Arena* arena) {
    Array file = ReadEntireFile(from, arena);
    return WriteToFile(file, to);
}