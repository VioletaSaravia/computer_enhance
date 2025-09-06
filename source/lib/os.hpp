#pragma once

#include "lib/types.hpp"

#define EXPORT

struct AppInfo {
    u64 permMemorySize, tempMemorySize;
};

struct OSMetrics {
    bool   Initialized;
    void* ProcessHandle;

    static OSMetrics Init();
};

namespace OS {

static u64 ReadPageFaultCount();

u64 ReadCPUTimer(void);

u64 EstimateCPUTimerFreq(void) {
    u64 MillisecondsToWait = 100;
    u64 OSFreq             = SDL_GetPerformanceFrequency();

    u64 CPUStart   = ReadCPUTimer();
    u64 OSStart    = SDL_GetPerformanceCounter();
    u64 OSEnd      = 0;
    u64 OSElapsed  = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime) {
        OSEnd     = SDL_GetPerformanceCounter();
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
    cstr processorArchitecture;
    u32  numberOfProcessors;
    u32  pageSize;
    u32  allocationGranularity;

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

    static SystemInfo Init();

    // TODO move to engine
    void Check(const AppInfo& info) const {
        if ((info.permMemorySize + info.tempMemorySize) > this->availPhys) {
            FATAL("Insufficient available RAM: app needs at least %llu MBs, got %llu MBs",
                  (info.permMemorySize + info.tempMemorySize) / (1024 * 1024),
                  availPhys / (1024 * 1024));
        };

        return;
    }

    void Print() const {
        INFO("System Information");
        SDL_Log("\t> Platform: \t\t\tWindows %s\n", processorArchitecture);
        SDL_Log("\t> Version: \t\t\t%u.%u.%u\n", majorVersion, minorVersion, buildNumber);
        SDL_Log("\t> Processor Count: \t\t%u\n", numberOfProcessors);
        SDL_Log("\t> CPU Frequency: \t\t%.2f GHz\n", cpuFreq);
        SDL_Log("\t> Page Size: \t\t\t%u bytes\n", pageSize);

        INFO("Memory Information");
        SDL_Log("\t> Total Physical Memory: \t%llu MB\n", totalPhys / (1024 * 1024));
        SDL_Log("\t> Available Physical Memory: \t%llu MB\n", availPhys / (1024 * 1024));
        SDL_Log("\t> Total Virtual Memory: \t%llu MB\n", totalVirtual / (1024 * 1024));
        SDL_Log("\t> Available Virtual Memory: \t%llu MB\n", availVirtual / (1024 * 1024));
    }
};

}; // namespace OS