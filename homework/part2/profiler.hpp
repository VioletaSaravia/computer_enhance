#pragma once

#include <stdio.h>

#include "containers.hpp"
#include "os.hpp"
#include "types.hpp"

struct Block {
    cstr label, file;
    i32  line;

    u64 iterations;
    u64 from, timeEx, timeInc;

    u64 bytesProcessed;
};

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 64
#endif

struct Profiler {
    struct BlockFlag {
        Profiler* parent;
        ~BlockFlag() { parent->EndBlock(); }
    };

    cstr name;
    bool ended;
    u64  start;

    StackArray<Block, MAX_BLOCKS> blocks;
    StackArray<u64, MAX_BLOCKS>   queue;

    static Profiler  _Profiler;
    static bool      Initialized; // Prevents destructor from being called on init <.<
    static Profiler& Get() { return Profiler::_Profiler; }

    static void New(cstr name = "") {
        Profiler::_Profiler = Profiler{
            .name   = name,
            .ended  = false,
            .start  = OS::ReadTimer(),
            .blocks = {},
            .queue  = {},
        };
    }

    void BeginBlock(u64 id, cstr label = "", cstr file = "", i32 line = 0, u64 bytesProcessed = 0) {
        if (id >= blocks.cap) {
            return;
        }

        Block* m    = &blocks[id];
        u64    time = OS::ReadTimer();

        if (queue.len > 0) {
            Block* prev = &blocks[queue.Last()];
            prev->timeEx += time - prev->from;
            prev->timeInc += time - prev->from;
        }

        m->from  = time;
        m->label = label;
        m->file  = file;
        m->line  = line;
        m->bytesProcessed += bytesProcessed;

        queue.Push(id);

        m->iterations++;

        return;
    }

    void AddBytes(u64 bytes) { blocks[queue.Last()].bytesProcessed += bytes; }

    BlockFlag
    BeginScopeBlock(i32 id, cstr label, cstr file = "", i32 line = 0, u64 bytesProcessed = 0) {
        BeginBlock(id, label, file, line, bytesProcessed);
        return BlockFlag{.parent = this};
    }

    void EndBlock() {
        u64 now = OS::ReadTimer();

        Block* m = &blocks[queue.Pop()];
        m->timeEx += now - m->from;
        m->timeInc += now - m->from;

        if (queue.len > 0) {
            // TODO: rodata in [] so this check is unnecesary
            Block* prev = &blocks[queue.Last()];
            prev->from  = now;
            prev->timeInc += now - m->from;
        }
    }

    void End() {
        if (ended) return;

        ended       = true;
        Initialized = false;

        f64 totalTime = f64(OS::ReadTimer() - start) / f64(OS::GetTimerFreq());
        
        INFO("Finished profiler %s in %.6f seconds", name, totalTime);
        printf(" %-24s \t| %-25s \t| %-25s \t| %-12s\n",
               "Name[n]",
               "Time (Ex)",
               "Time (Inc)",
               "Bandwidth");
        printf("-----------------------------------------------------------------------------------"
               "--------------------"
               "--------\n");

        for (u64 i = 1; i < blocks.cap; i++) {
            auto next = blocks[i];
            if (next.iterations == 0) continue;

            f64 nextTimeEx  = (f64(next.timeEx) / f64(OS::GetTimerFreq()));
            f64 nextTimeInc = (f64(next.timeInc) / f64(OS::GetTimerFreq()));
            if (next.bytesProcessed == 0) {
                printf(" %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t|\n",
                       next.label,
                       next.iterations,
                       nextTimeEx,
                       (nextTimeEx / totalTime) * 100,
                       nextTimeInc,
                       (nextTimeInc / totalTime) * 100);
            } else {
                printf(
                    " %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t| %.3f GB/s\n",
                    next.label,
                    next.iterations,
                    nextTimeEx,
                    (nextTimeEx / totalTime) * 100,
                    nextTimeInc,
                    (nextTimeInc / totalTime) * 100,
                    f64(next.bytesProcessed) / nextTimeEx / 1024.0 / 1024.0 / 1024.0);
            }
        }
    }

    ~Profiler() {
        if (!Initialized) {
            Initialized = true;
            return;
        }
        if (start != 0) End();
    }
};

Profiler Profiler::_Profiler   = {};
bool     Profiler::Initialized = false;

typedef struct {
    u64 time, bytes, pageFaults;
} RepBlock;

extern "C" {
i32 ByTime(const void* from, const void* to) {
    return ((RepBlock*)(from))->time - ((RepBlock*)(to))->time;
}

i32 ByBytes(const void* from, const void* to) {
    return ((RepBlock*)(from))->bytes - ((RepBlock*)(to))->bytes;
}

i32 ByPageFaults(const void* from, const void* to) {
    return ((RepBlock*)(from))->pageFaults - ((RepBlock*)(to))->pageFaults;
}
}

struct RepetitionProfiler {
    cstr name;

    RepBlock  min, max, avg, current;
    RepBlock* all;
    u64       repeats, maxRepeats;

    static RepetitionProfiler New(cstr name, u64 maxRepeats = 1000) {
        return RepetitionProfiler{
            .name       = name,
            .min        = {},
            .max        = {},
            .avg        = {},
            .current    = {},
            .all        = (RepBlock*)malloc(sizeof(RepBlock) * maxRepeats),
            .repeats    = 0,
            .maxRepeats = maxRepeats,
        };
    }

    void BeginRep() {
        this->current.time       = OS::ReadTimer();
        this->current.pageFaults = OS::ReadPageFaultCount();
    }

    void AddBytes(u64 bytes) { this->current.bytes += bytes; }

    void EndRep() {
        this->current.time       = OS::ReadTimer() - this->current.time;
        this->current.pageFaults = OS::ReadPageFaultCount() - this->current.pageFaults;

        if (this->current.time < this->min.time || this->min.time == 0) {
            this->min = this->current;
        }

        if (this->current.time > this->max.time || this->max.time == 0) {
            this->max = this->current;
        }

        this->all[this->repeats] = this->current;
        this->repeats++;
        this->current = {};
    }

    ~RepetitionProfiler() {
        INFO("Finished profiler %s after %llu repeats.", this->name, this->repeats);

        // MIN
        f64 minTime = f64(this->min.time) / f64(OS::GetTimerFreq());
        printf("\t> Min: \t%.3f ms\t%.3f GB/s\t%llu pf\n",
               minTime * 1000.0,
               f64(this->min.bytes) / minTime / 1024.0 / 1024.0 / 1024.0,
               this->min.pageFaults);

        // MAX
        f64 maxTime = f64(this->max.time) / f64(OS::GetTimerFreq());
        printf("\t> Max: \t%.3f ms\t%.3f GB/s\t%llu pf\n",
               maxTime * 1000.0,
               f64(this->max.bytes) / maxTime / 1024.0 / 1024.0 / 1024.0,
               this->max.pageFaults);

        // AVERAGE
        for (u64 i = 0; i < repeats; i++) {
            this->avg.time += all[i].time;
            this->avg.bytes += all[i].bytes;
            this->avg.pageFaults += all[i].pageFaults;
        }
        f64 avgBytes  = f64(avg.bytes) / f64(repeats);
        f64 avgFaults = f64(avg.pageFaults) / f64(repeats);
        f64 avgTime   = f64(avg.time) / f64(repeats);
        avgTime /= f64(OS::GetTimerFreq());

        printf("\t> Avg: \t%.3f ms\t%.3f GB/s\t%.2f pf\n",
               avgTime * 1000.0,
               f64(avgBytes) / avgTime / 1024.0 / 1024.0 / 1024.0,
               avgFaults);

        // MEAN
        qsort(all, repeats, sizeof(RepBlock), ByTime);
        f64 meanTime = all[repeats / 2].time / f64(OS::GetTimerFreq());

        // qsort(all, repeats, sizeof(RepBlock), ByBytes);
        f64 meanBytes = all[repeats / 2].bytes;

        qsort(all, repeats, sizeof(RepBlock), ByPageFaults);
        u64 meanPageFaults = all[repeats / 2].pageFaults;
        printf("\t> Mean:\t%.3f ms\t%.3f GB/s\t%llu pf\n",
               meanTime * 1000.0,
               f64(meanBytes) / meanTime / 1024.0 / 1024.0 / 1024.0,
               meanPageFaults);

        printf("\n");
        free(all);
    }
};

#ifdef ENABLE_PROFILER

#define PROFILER_NEW(name) Profiler::New(name)
#define PROFILER_END()     Profiler::Get().End()
#define PROFILE_BLOCK_BEGIN(name)                                                                  \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_ADD_BANDWIDTH(bytes) Profiler::Get().AddBytes(bytes)
#define PROFILE_BLOCK_END()          Profiler::Get().EndBlock()
#define PROFILE_SCOPE(name)                                                                        \
    auto _profilerFlag = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_FUNCTION()                                                                         \
    auto _profilerFlag =                                                                           \
        Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, __func__, __FILE__, __LINE__)
#define PROFILE(name, code)                                                                        \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__);                         \
    code;                                                                                          \
    Profiler::Get().EndBlock();

#define REPETITION_PROFILE(name, count)                                                            \
    do {                                                                                           \
        auto _profiler = RepetitionProfiler::New(name, count);                                     \
        while (_profiler.repeats < _profiler.maxRepeats) {                                         \
            _profiler.BeginRep();

#define REPETITION_BANDWIDTH(bytes) _profiler.AddBytes(bytes)

#define REPETITION_END()                                                                           \
    _profiler.EndRep();                                                                            \
    }                                                                                              \
    }                                                                                              \
    while (0);

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
