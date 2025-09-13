#pragma once

#include "lib/types.h"

#define EXPORT

typedef struct Metrics {
    bool  Initialized;
    void* ProcessHandle;
} Metrics;

Metrics NewMetrics();
u64     ReadPageFaultCount(const Metrics*);

u64 ReadCPUTimer();

u64 EstimateCPUTimerFreq() {
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

typedef struct SystemInfo {
    // System
    cstr processorArchitecture;
    u32  numberOfProcessors, pageSize, allocationGranularity;
    f64  cpuFreq;

    // Memory
    u64 totalPhys, availPhys, totalVirtual, availVirtual;

    // OS
    u32 majorVersion, minorVersion, buildNumber, platformId;

    // GPU
    cstr gpuName, gpuVendor, glVersion;
} SystemInfo;

SystemInfo NewSystemInfo();

void PrintSystemInfo(const SystemInfo* info) {
    INFO("System Information");
    SDL_Log("\t> Platform: \t\t\tWindows %s\n", info->processorArchitecture);
    SDL_Log(
        "\t> Version: \t\t\t%u.%u.%u\n", info->majorVersion, info->minorVersion, info->buildNumber);
    SDL_Log("\t> Processor Count: \t\t%u\n", info->numberOfProcessors);
    SDL_Log("\t> CPU Frequency: \t\t%.2f GHz\n", info->cpuFreq);
    SDL_Log("\t> Page Size: \t\t\t%u bytes\n", info->pageSize);

    INFO("Memory Information");
    SDL_Log("\t> Total Physical Memory: \t%llu MB\n", info->totalPhys / (1024 * 1024));
    SDL_Log("\t> Available Physical Memory: \t%llu MB\n", info->availPhys / (1024 * 1024));
    SDL_Log("\t> Total Virtual Memory: \t%llu MB\n", info->totalVirtual / (1024 * 1024));
    SDL_Log("\t> Available Virtual Memory: \t%llu MB\n", info->availVirtual / (1024 * 1024));
}

void GetAndPrintGPUInfo(SystemInfo* info) {
    info->gpuName   = (cstr)glGetString(GL_RENDERER);
    info->gpuVendor = (cstr)glGetString(GL_VENDOR);
    info->glVersion = (cstr)glGetString(GL_VERSION);

    INFO("GPU Information" LIST_VAR "Name: \t\t\t%s" LIST_VAR "Vendor: \t\t\t%s" LIST_VAR
         "OpenGL version: \t\t%s",
         info->gpuName,
         info->gpuVendor,
         info->glVersion);
}