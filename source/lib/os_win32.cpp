#include "os.hpp"

// #define WIN32_LEAN_AND_MEAN

#include <Psapi.h>
#include <Windows.h>

namespace OS {

void PrintError(cstr call) {
    DWORD err = GetLastError();
    if (err == 0) {
        printf("No error.");
        return;
    }

    LPSTR msg = NULL;
    DWORD len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL,
                               err,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPSTR)&msg,
                               0,
                               NULL);

    if (len) {
        ERR("[%s] %lu: %s", call, err, msg);
        LocalFree(msg);
    } else {
        ERR(" [%s] %lu (could not format message)", call, err);
    }
}

u8* Alloc(u64 size) {
    u8* result = (u8*)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!result) PrintError("VirtualAlloc");
    return result;
}

bool Free(void* ptr) {
    return VirtualFree(ptr, 0, MEM_RELEASE);
}

u64 GetTimerFreq() {
    LARGE_INTEGER Freq = {};

    bool ok = QueryPerformanceFrequency(&Freq);
    if (!ok) printf("[ERROR] Couldn't obtain OS timer frequency\n");

    return ok ? Freq.QuadPart : 0;
}

u64 ReadTimer() {
    LARGE_INTEGER Value = {};

    bool ok = QueryPerformanceCounter(&Value);
    if (!ok) printf("[ERROR] Couldn't obtain OS timer\n");

    return ok ? Value.QuadPart : 0;
}

struct OSMetrics : ISingleton {
    bool   Initialized;
    HANDLE ProcessHandle;

    static OSMetrics& Get() {
        static OSMetrics metrics;
        return metrics;
    }
};

u64 ReadPageFaultCount(void) {
    PROCESS_MEMORY_COUNTERS_EX MemoryCounters = {};

    MemoryCounters.cb = sizeof(MemoryCounters);
    GetProcessMemoryInfo(OSMetrics::Get().ProcessHandle,
                         (PROCESS_MEMORY_COUNTERS*)&MemoryCounters,
                         sizeof(MemoryCounters));

    u64 result = MemoryCounters.PageFaultCount;
    return result;
}

void InitializeMetrics(void) {
    if (!OSMetrics::Get().Initialized) {
        OSMetrics::Get().Initialized = true;
        OSMetrics::Get().ProcessHandle =
            OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    }
}

SystemInfo::SystemInfo() {
    SYSTEM_INFO     sysInfo;
    MEMORYSTATUSEX  memInfo;
    OSVERSIONINFOEX osInfo;

    // System
    GetSystemInfo(&sysInfo);
    // processorArchitecture = sysInfo.wProcessorArchitecture;
    numberOfProcessors    = sysInfo.dwNumberOfProcessors;
    pageSize              = sysInfo.dwPageSize;
    minAppAddress         = sysInfo.lpMinimumApplicationAddress;
    maxAppAddress         = sysInfo.lpMaximumApplicationAddress;
    allocationGranularity = sysInfo.dwAllocationGranularity;

    processorArchitecture = "Unknown";
    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: processorArchitecture = "x64 (AMD/Intel)"; break;
    case PROCESSOR_ARCHITECTURE_INTEL: processorArchitecture = "x86"; break;
    case PROCESSOR_ARCHITECTURE_ARM: processorArchitecture = "ARM"; break;
    case PROCESSOR_ARCHITECTURE_ARM64: processorArchitecture = "ARM64"; break;
    }

    cpuFreq = f64(OS::EstimateCPUTimerFreq()) / 1000.0 / 1000.0 / 1000.0;

    // Memory
    memInfo.dwLength = sizeof(memInfo);
    if (GlobalMemoryStatusEx(&memInfo)) {
        totalPhys    = memInfo.ullTotalPhys;
        availPhys    = memInfo.ullAvailPhys;
        totalVirtual = memInfo.ullTotalVirtual;
        availVirtual = memInfo.ullAvailVirtual;
    } else {
        totalPhys = availPhys = totalVirtual = availVirtual = 0;
    }

    // OS
    ZeroMemory(&osInfo, sizeof(osInfo));
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    if (GetVersionEx((OSVERSIONINFO*)&osInfo)) {
        majorVersion = osInfo.dwMajorVersion;
        minorVersion = osInfo.dwMinorVersion;
        buildNumber  = osInfo.dwBuildNumber;
        platformId   = osInfo.dwPlatformId;
    } else {
        majorVersion = minorVersion = buildNumber = platformId = 0;
    }
}

}; // namespace OS
