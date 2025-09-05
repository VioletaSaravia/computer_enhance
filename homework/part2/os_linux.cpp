#include "os.hpp"

#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

namespace OS {

u64 GetTimerFreq() {
    return 1000000000ULL;
}

u64 ReadTimer() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * GetTimerFreq() + ts.tv_nsec;
}

SystemInfo::SystemInfo() {
    // System
    struct utsname uts;
    if (uname(&uts) == 0) {
        processorArchitecture = strdup(uts.machine); // e.g., "x86_64"
    } else {
        processorArchitecture = "unknown";
    }

    numberOfProcessors = (u32)sysconf(_SC_NPROCESSORS_ONLN);
    pageSize           = (u32)sysconf(_SC_PAGESIZE);

    // Linux doesn’t expose min/max app address like Win32
    minAppAddress         = (void*)0x0;
    maxAppAddress         = (void*)~0ULL;
    allocationGranularity = pageSize;

    // CPU frequency
    cpuFreq = 0.0;
    FILE* f = fopen("/proc/cpuinfo", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "cpu MHz", 7) == 0) {
                double mhz = 0.0;
                if (sscanf(line, "cpu MHz\t: %lf", &mhz) == 1) {
                    cpuFreq = mhz / 1000.0; // convert MHz → GHz
                    break;
                }
            }
        }
        fclose(f);
    }

    // Memory
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        totalPhys    = (u64)si.totalram * si.mem_unit;
        availPhys    = (u64)si.freeram * si.mem_unit;
        totalVirtual = (u64)(si.totalram + si.totalswap) * si.mem_unit;
        availVirtual = (u64)(si.freeram + si.freeswap) * si.mem_unit;
    } else {
        totalPhys = availPhys = totalVirtual = availVirtual = 0;
    }

    // OS version
    if (uname(&uts) == 0) {
        // uts.sysname = "Linux", uts.release = kernel version string
        // Just parse major.minor.build from uts.release if possible
        u32 maj = 0, min = 0, build = 0;
        sscanf(uts.release, "%u.%u.%u", &maj, &min, &build);
        majorVersion = maj;
        minorVersion = min;
        buildNumber  = build;
        platformId   = 0; // no direct equivalent
    } else {
        majorVersion = minorVersion = buildNumber = platformId = 0;
    }
}

} // namespace OS