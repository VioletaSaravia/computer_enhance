#pragma once

#include "types.h"

typedef struct {
    cstr label, file;
    i32 line;

    u64 iterations;
    u64 from, timeEx, timeInc;

    u64 bytesProcessed;
} Block;

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 64
#endif