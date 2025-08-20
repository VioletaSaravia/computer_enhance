#include <cstdio>

#include "types.h"

template <typename T>
struct Array
{
    T *data;
    u64 len, cap;

    void Push(T &element)
    {
        if (len >= cap)
        {
            cap *= 2;
            data = realloc(data, sizeof(T) * cap);
        }

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
            abort();

        len -= 1;
        return data[len];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
            abort();
        return data[id];
    }
};

template <typename T, unsigned int N>
struct FixedArray
{
    T data[N];
    u64 len, cap = N;

    void Push(T &element)
    {
        if (len >= N)
            return;

        data[len] = element;
        len++;
    }

    T &Pop()
    {
        if (len == 0)
            abort();

        len -= 1;
        return data[len];
    }

    T &Last()
    {
        if (len == 0)
            abort();

        return data[len - 1];
    }

    T &operator[](u64 id)
    {
        if (id >= cap)
        {
            abort();
        }

        return data[id];
    }
};

typedef Array<char> String;

template <int N>
using FixedString = FixedArray<char, N>;

u64 GetOSTimerFreq(void)
{
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}

u64 ReadOSTimer(void)
{
    LARGE_INTEGER Value;
    QueryPerformanceCounter(&Value);
    return Value.QuadPart;
}

inline u64 ReadCPUTimer(void)
{
    // NOTE(casey): If you were on ARM, you would need to replace __rdtsc
    // with one of their performance counter read instructions, depending
    // on which ones are available on your platform.

    return __rdtsc();
}

u64 EstimateCPUTimerFreq(void)
{
    u64 MillisecondsToWait = 100;
    u64 OSFreq = GetOSTimerFreq();

    u64 CPUStart = ReadCPUTimer();
    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime)
    {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }

    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;

    u64 CPUFreq = 0;
    if (OSElapsed)
    {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

    return CPUFreq;
};

struct Profiler
{
    struct Measurement
    {
        cstr label, file;
        i32 line;

        u64 iterations;
        u64 from, timeEx, timeInc;

        u64 bytesProcessed;
    };

    struct BlockFlag
    {
        Profiler *parent;
        ~BlockFlag()
        {
            parent->EndBlock();
        }
    };

    cstr name;
    bool ended;
    u64 start;
    bool printFile;

    FixedArray<Measurement, 64> measurements;
    FixedArray<u64, 64> queue;

    static Profiler _Profiler;
    static bool IHateCpp; // Prevents destructor from being called on init <.<
    static Profiler &Get()
    {
        return Profiler::_Profiler;
    }
    static void New(cstr name = "", bool printFile = false)
    {
        Profiler::_Profiler = Profiler{
            .name = name,
            .ended = false,
            .start = ReadOSTimer(),
            .printFile = printFile,
            .measurements = {},
            .queue = {},
        };
    }

    void BeginBlock(u64 id, cstr label = "", cstr file = "", i32 line = 0, u64 bytesProcessed = 0)
    {
        if (id >= measurements.cap)
        {
            return;
        }

        Measurement *m = &measurements[id];
        u64 time = ReadOSTimer();

        if (queue.len > 0)
        {
            Measurement *prev = &measurements[queue.Last()];
            prev->timeEx += time - prev->from;
            prev->timeInc += time - prev->from;
        }

        m->from = time;
        m->label = label;
        m->file = file;
        m->line = line;
        m->bytesProcessed += bytesProcessed;

        queue.Push(id);

        m->iterations++;

        return;
    }

    void AddBytes(u64 bytes)
    {
        measurements[queue.Last()].bytesProcessed += bytes;
    }

    BlockFlag BeginScopeBlock(i32 id, cstr label, cstr file = "", i32 line = 0, u64 bytesProcessed = 0)
    {
        BeginBlock(id, label, file, line, bytesProcessed);
        return BlockFlag{.parent = this};
    }

    void EndBlock()
    {
        u64 now = ReadOSTimer();

        Measurement *m = &measurements[queue.Pop()];
        m->timeEx += now - m->from;
        m->timeInc += now - m->from;

        if (queue.len > 0)
        {
            // TODO rodata in [] so this check is unnecesary
            Measurement *prev = &measurements[queue.Last()];
            prev->from = now;
            prev->timeInc += now - m->from;
        }
    }

    void End()
    {
        if (!ended)
        {
            ended = true;
            IHateCpp = false;
        }
        else
        {
            return;
        }

        f64 totalTime = f64(ReadOSTimer() - start) / f64(GetOSTimerFreq());
        printf("[INFO] Finished profiler %s in %.6f seconds\n", name, totalTime);
        printf(" %-24s \t| %-25s \t| %-25s \t| %-12s\n", "Name[n]", "Time (Ex)", "Time (Inc)", "Bandwidth");
        printf("---------------------------------------------------------------------------------------------------------------\n");

        for (u64 i = 1; i < measurements.cap; i++)
        {
            auto next = measurements[i];
            if (next.iterations == 0)
                continue;

            f64 nextTimeEx = (f64(next.timeEx) / f64(GetOSTimerFreq()));
            f64 nextTimeInc = (f64(next.timeInc) / f64(GetOSTimerFreq()));
            if (next.bytesProcessed == 0)
            {
                printf(" %-20s [%lld] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t|\n",
                       next.label, next.iterations,
                       nextTimeEx, (nextTimeEx / totalTime) * 100,
                       nextTimeInc, (nextTimeInc / totalTime) * 100);
            }
            else
            {
                printf(" %-20s [%lld] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t| %.3f MB/s\n",
                       next.label, next.iterations,
                       nextTimeEx, (nextTimeEx / totalTime) * 100,
                       nextTimeInc, (nextTimeInc / totalTime) * 100,
                       f64(next.bytesProcessed) / nextTimeEx / 1024.0 / 1024.0);
            }
        }
    }

    ~Profiler()
    {
        if (!IHateCpp)
        {
            IHateCpp = true;
            return;
        }
        if (start != 0)
            End();
    }
};

Profiler Profiler::_Profiler{};
bool Profiler::IHateCpp{};

// #define DISABLE_PROFILER

#ifndef DISABLE_PROFILER

#define PROFILER_NEW(name) Profiler::New(name)
#define PROFILER_END() Profiler::Get().End()
#define PROFILE_BLOCK_BEGIN(name) Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_ADD_BANDWIDTH(bytes) Profiler::Get().AddBytes(bytes)
#define PROFILE_BLOCK_END() Profiler::Get().EndBlock()
#define PROFILE_SCOPE(name) auto _profilerFlag = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_FUNCTION() auto _profilerFlag = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, __func__, __FILE__, __LINE__)
#define PROFILE(name, code)                                                \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__); \
    code;                                                                  \
    Profiler::Get().EndBlock();

#else

#define PROFILER_NEW(name)
#define PROFILER_END()
#define PROFILE_BLOCK_BEGIN(name)
#define PROFILE_ADD_BANDWIDTH(bytes)
#define PROFILE_BLOCK_END()
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#define PROFILE(name, code) code

#endif
