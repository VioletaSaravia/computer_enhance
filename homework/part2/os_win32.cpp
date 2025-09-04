#include "os.hpp"

#include <Psapi.h>
#include <Windows.h>

namespace OS {

u8* Alloc(u64 size) {
    return (u8*)VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
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

}; // namespace OS
