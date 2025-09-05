#pragma once

#include "types.hpp"

struct AppInfo {
    u64 permMemorySize, tempMemorySize;
};

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

struct SystemInfo {
    // System
    cstr  processorArchitecture;
    u32   numberOfProcessors;
    u32   pageSize;
    void* minAppAddress;
    void* maxAppAddress;
    u32   allocationGranularity;

    f64 cpuFreq;

    // Memory
    u64 totalPhys;
    u64 availPhys;
    u64 totalVirtual;
    u64 availVirtual;

    // OS
    u32 majorVersion;
    u32 minorVersion;
    u32 buildNumber;
    u32 platformId;

    SystemInfo();

    static SystemInfo const& Get() {
        static const SystemInfo info;
        return info;
    }

    void Check(AppInfo& info) const {
        if ((info.permMemorySize + info.tempMemorySize) > this->availPhys) {
            FATAL("Insufficient available RAM: app needs at least %llu MBs, got %llu MBs",
                  (info.permMemorySize + info.tempMemorySize) / (1024 * 1024),
                  availPhys / (1024 * 1024));
        };

        return;
    }

    void Print() const {
        INFO("System Information");
        printf("\t> Platform: \t\t\tWindows %s\n", processorArchitecture);
        printf("\t> Version: \t\t\t%u.%u.%u\n", majorVersion, minorVersion, buildNumber);
        printf("\t> Processor Count: \t\t%u\n", numberOfProcessors);
        printf("\t> CPU Frequency: \t\t%.2f GHz\n", cpuFreq);
        printf("\t> Page Size: \t\t\t%u bytes\n", pageSize);

        INFO("Memory Information");
        printf("\t> Total Physical Memory: \t%llu MB\n", totalPhys / (1024 * 1024));
        printf("\t> Available Physical Memory: \t%llu MB\n", availPhys / (1024 * 1024));
        printf("\t> Total Virtual Memory: \t%llu MB\n", totalVirtual / (1024 * 1024));
        printf("\t> Available Virtual Memory: \t%llu MB\n", availVirtual / (1024 * 1024));
    }
};

}; // namespace OS