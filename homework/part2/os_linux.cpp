#include "os.hpp"

#include <stdio.h>
#include <time.h>

u64 GetTimerFreq() {
    return 1000000000ULL;
}

u64 ReadTimer() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * GetTimerFreq() + ts.tv_nsec;
}