#include <stdbool.h>
#include <stdio.h>

#include "containers.gen.c"
#include "profiler.h"

#ifdef _WIN32

u64 GetOSTimerFreq(void) {
    LARGE_INTEGER Freq = {0};

    bool ok = QueryPerformanceFrequency(&Freq);
    if (!ok) printf("[ERROR] Couldn't obtain OS timer frequency\n");

    return ok ? Freq.QuadPart : 0;
}

u64 ReadOSTimer(void) {
    LARGE_INTEGER Value = {0};

    bool ok = QueryPerformanceCounter(&Value);
    if (!ok) printf("[ERROR] Couldn't obtain OS timer\n");

    return ok ? Value.QuadPart : 0;
}

#else

#include <time.h>

u64 GetOSTimerFreq() {
    return 1000000000ULL;
}

u64 ReadOSTimer() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * GetOSTimerFreq() + ts.tv_nsec;
}

#endif

inline u64 ReadCPUTimer(void) {
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(_M_IX86)
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
    u64 OSFreq = GetOSTimerFreq();

    u64 CPUStart = ReadCPUTimer();
    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime) {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;

    u64 CPUFreq = 0;
    if (OSElapsed) {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

    return CPUFreq;
};

typedef struct {
    bool ended;
    u64 start;

    BlockArray blocks;
    u64Array queue;
} Profiler;

global Profiler Prof;

typedef struct {
    bool active;
} BlockFlag;

BlockFlag BlockBegin(u64 id, cstr label, cstr file, i32 line) {
    if (id >= Prof.blocks.cap) {
        return (BlockFlag){0};
    }

    Block* m = GetMut(Prof.blocks, id);
    u64 time = ReadOSTimer();

    if (Prof.queue.len > 0) {
        Block* prev = GetMut(Prof.blocks, Last(Prof.queue));
        prev->timeEx += time - prev->from;
        prev->timeInc += time - prev->from;
    }

    m->from = time;
    m->label = label;
    m->file = file;
    m->line = line;

    Push(Prof.queue, id);

    m->iterations++;

    return (BlockFlag){true};
}

void BlockAddBytes(u64 bytes) {
    GetMut(Prof.blocks, Last(Prof.queue))->bytesProcessed += bytes;
}

void BlockEnd() {
    u64 now = ReadOSTimer();

    Block* m = GetMut(Prof.blocks, Pop(Prof.queue));
    m->timeEx += now - m->from;
    m->timeInc += now - m->from;

    if (Prof.queue.len > 0) {
        Block* prev = GetMut(Prof.blocks, Last(Prof.queue));
        prev->from = now;
        prev->timeInc += now - m->from;
    }
}

void BlockFlagEnd(BlockFlag* flag) {
    if (!flag || !flag->active) return;
    u64 now = ReadOSTimer();

    Block* m = GetMut(Prof.blocks, Pop(Prof.queue));
    m->timeEx += now - m->from;
    m->timeInc += now - m->from;

    if (Prof.queue.len > 0) {
        Block* prev = GetMut(Prof.blocks, Last(Prof.queue));
        prev->from = now;
        prev->timeInc += now - m->from;
    }
}

void ProfilerEnd() {
    if (Prof.ended) return;

    Prof.ended = true;

    f64 totalTime = (f64)(ReadOSTimer() - Prof.start) / (f64)(GetOSTimerFreq());
    printf("[INFO] Finished profiler in %.6f seconds\n", totalTime);
    printf(" %-24s \t| %-25s \t| %-25s \t| %-12s\n", "Name[n]", "Time (Ex)", "Time (Inc)", "Bandwidth");
    printf("---------------------------------------------------------------------------------------------------------------\n");

    for (u64 i = 1; i < Prof.blocks.cap; i++) {
        Block next = Get(Prof.blocks, i);
        if (next.iterations == 0)
            continue;

        f64 nextTimeEx = ((f64)(next.timeEx) / (f64)(GetOSTimerFreq()));
        f64 nextTimeInc = ((f64)(next.timeInc) / (f64)(GetOSTimerFreq()));
        if (next.bytesProcessed == 0) {
            printf(" %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t|\n",
                   next.label, next.iterations,
                   nextTimeEx, (nextTimeEx / totalTime) * 100,
                   nextTimeInc, (nextTimeInc / totalTime) * 100);
        } else {
            printf(" %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t| %.3f GB/s\n",
                   next.label, next.iterations,
                   nextTimeEx, (nextTimeEx / totalTime) * 100,
                   nextTimeInc, (nextTimeInc / totalTime) * 100,
                   (f64)(next.bytesProcessed) / nextTimeEx / 1024.0 / 1024.0 / 1024.0);
        }
    }
}

void ProfilerStart() {
    BlockArray blocks = BlockArrayNew(128);
    u64Array queue = u64ArrayNew(128);
    memset(blocks.data, 0, sizeof(Block) * 128);
    memset(queue.data, 0, sizeof(u64) * 128);
    Prof = (Profiler){
        .start = ReadOSTimer(),
        .blocks = blocks,
        .queue = queue,
    };
}

#define ENABLE_PROFILER

#ifdef ENABLE_PROFILER
#define PROFILER_NEW() ProfilerStart()
#define PROFILER_END() ProfilerEnd()
#define PROFILE_BLOCK_BEGIN(name) BlockBegin(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_ADD_BANDWIDTH(bytes) BlockAddBytes(bytes)
#define PROFILE_BLOCK_END() BlockEnd()

// TODO(violeta): _blockFlag doesn't allow nesting scopes!!!
#define PROFILE_SCOPE(name) \
    DEFER(BlockFlagEnd)     \
    BlockFlag _blockFlag = BlockBegin(__COUNTER__ + 1, name, __FILE__, __LINE__);

// TODO(violeta): Does optimization clear away deferFlag?
#define PROFILE_FUNCTION() \
    DEFER(BlockFlagEnd)    \
    BlockFlag _blockFlag = BlockBegin(__COUNTER__ + 1, __func__, __FILE__, __LINE__);

#define PROFILE(name, code)                                \
    BlockBegin(__COUNTER__ + 1, name, __FILE__, __LINE__); \
    code;                                                  \
    BlockEnd();

// #define REPETITION_PROFILE(name, count)                        \
//     do {                                                       \
//         auto _profiler = RepProfiler::New(name, count); \
//         while (_profiler.repeats < _profiler.maxRepeats) {     \
//             _profiler.BeginRep();

// #define REPETITION_BANDWIDTH(bytes) _profiler.AddBytes(bytes)

// #define REPETITION_END() \
//     _profiler.EndRep();  \
//     }                    \
//     }                    \
//     while (0);

#else

#define PROFILER_NEW(...)
#define PROFILER_END(...)
#define PROFILE_BLOCK_BEGIN(...)
#define PROFILE_ADD_BANDWIDTH(...)
#define PROFILE_BLOCK_END(...)
#define PROFILE_SCOPE(...)
#define PROFILE_FUNCTION(...)
#define PROFILE(name, code) code

#define REPETITION_PROFILE(...)
#define REPETITION_BANDWIDTH(...)
#define REPETITION_END(...)

#endif

// #sort(time, bytes)
typedef struct {
    u64 time;
    u64 bytes;
} RepBlock;

i32 ByTime(const void* from, const void* to) {
    return ((RepBlock*)(from))->time - ((RepBlock*)(to))->time;
}

i32 ByBytes(const void* from, const void* to) {
    return ((RepBlock*)(from))->bytes - ((RepBlock*)(to))->bytes;
}

typedef struct {
    cstr name;

    RepBlock min, max, avg, current;
    RepBlock* all;
    u64 repeats, maxRepeats;
} RepProfiler;

RepProfiler RepProfilerNew(cstr name, u64 maxRepeats) {
    return (RepProfiler){
        .name = name,
        .all = (RepBlock*)malloc(sizeof(RepBlock) * maxRepeats),
        .repeats = 0,
        .maxRepeats = maxRepeats != 0 ? maxRepeats : 1000,
    };
}

void RepBegin(RepProfiler* this) {
    this->current.time = ReadOSTimer();
}

void RepAddBytes(RepProfiler* this, u64 bytes) {
    this->current.bytes += bytes;
}

void RepEnd(RepProfiler* this) {
    this->current.time = ReadOSTimer() - this->current.time;

    if (this->current.time < this->min.time || this->min.time == 0) {
        this->min = this->current;
    }

    if (this->current.time > this->max.time || this->max.time == 0) {
        this->max = this->current;
    }

    this->all[this->repeats] = this->current;
    this->repeats++;
    this->current = (RepBlock){0};
}

void RepProfilerEnd(RepProfiler* this) {
    printf("[INFO] Finished profiler %s after %llu repeats.\n", this->name, this->repeats);

    f64 minTime = (f64)(this->min.time) / (f64)(GetOSTimerFreq());
    printf("\t> Min: \t%.4f ms\t%.4f GB/s\n", minTime * 1000.0, (f64)(this->min.bytes) / minTime / 1024.0 / 1024.0 / 1024.0);
    f64 maxTime = (f64)(this->max.time) / (f64)(GetOSTimerFreq());
    printf("\t> Max: \t%.4f ms\t%.4f GB/s\n", maxTime * 1000.0, (f64)(this->max.bytes) / maxTime / 1024.0 / 1024.0 / 1024.0);

    for (u64 i = 0; i < this->repeats; i++) {
        this->avg.time += this->all[i].time;
        this->avg.bytes += this->all[i].bytes;
    }
    f64 avgTime = (f64)(this->avg.time) / (f64)(this->repeats);
    f64 avgBytes = (f64)(this->avg.bytes) / (f64)(this->repeats);

    avgTime /= (f64)(GetOSTimerFreq());
    printf("\t> Avg: \t%.4f ms\t%.4f GB/s\n", avgTime * 1000.0, (f64)(avgBytes) / avgTime / 1024.0 / 1024.0 / 1024.0);

    qsort(this->all, this->repeats, sizeof(RepBlock), ByTime);
    f64 meanTime = this->all[this->repeats / 2].time / (f64)(GetOSTimerFreq());

    // qsort(all, repeats, sizeof(RepBlock), ByBytes);
    f64 meanBytes = this->all[this->repeats / 2].bytes;
    printf("\t> Mean:\t%.4f ms\t%.4f GB/s\n", meanTime * 1000.0, (f64)(meanBytes) / meanTime / 1024.0 / 1024.0 / 1024.0);

    printf("\n");
    free(this->all);
}