#pragma once

#include "types.hpp"

namespace OS {

u8*  Alloc(u64 size);
bool Free(void* ptr);

u64 GetTimerFreq();
u64 ReadTimer();

struct OSMetrics;

static u64  ReadPageFaultCount(void);
static void InitializeMetrics(void);

inline u64 ReadCPUTimer(void) {
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64) ||           \
    defined(__i386__) || defined(_M_IX86)
    return __rdtsc();

#elif defined(__aarch64__)
    // ARMv8 (AArch64): use CNTVCT_EL0
    uint64_t cnt;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(cnt));
    return cnt;

#elif defined(__arm__)
    // ARMv7-A: use PMCCNTR (if enabled)
    uint32_t cc;
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cc));
    return (uint64_t)cc;

#else
#error "Unsupported architecture"
#endif
}

u64 EstimateCPUTimerFreq(void) {
    u64 MillisecondsToWait = 100;
    u64 OSFreq             = GetTimerFreq();

    u64 CPUStart   = ReadCPUTimer();
    u64 OSStart    = ReadTimer();
    u64 OSEnd      = 0;
    u64 OSElapsed  = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime) {
        OSEnd     = ReadTimer();
        OSElapsed = OSEnd - OSStart;
    }

    u64 CPUEnd     = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;

    u64 CPUFreq = 0;
    if (OSElapsed) {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

    return CPUFreq;
};

}; // namespace OS