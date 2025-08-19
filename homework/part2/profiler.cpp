#include <cstdio>
#include <cstdint>
#include <Windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef const char *cstr;

template <typename T>
struct Maybe
{
    bool nil;
    T val;

    T Unwrap()
    {
        if (nil)
            abort();
        else
            return val;
    }
};

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

    T &operator[](u64 id) // TODO return type
    {
        if (id >= len)
        {
            abort();
        }

        return data[id];
    }
};

typedef Array<char> String;

template <int N>
using FixedString = FixedArray<char, N>;

static u64 GetOSTimerFreq(void)
{
    LARGE_INTEGER Freq;
    QueryPerformanceFrequency(&Freq);
    return Freq.QuadPart;
}

static u64 ReadOSTimer(void)
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

static u64 EstimateCPUTimerFreq(void)
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
}

struct Measurement
{
    cstr label;
    u64 iterations;
    u64 from, soFar;
};

struct Profiler
{
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

    FixedArray<Measurement, 64> measurements;
    FixedArray<i32, 64> queue;

    static Profiler _Profiler;
    static bool IHateCpp;
    static Profiler &Get()
    {
        return Profiler::_Profiler;
    }
    static void New(cstr name = "")
    {
        Profiler::_Profiler = Profiler{
            .name = name,
            .start = ReadOSTimer(),
        };
    }

    void BeginBlock(i32 id, cstr label = "")
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
            prev->soFar += time - prev->from;
        }

        m->from = time;
        m->label = label;

        queue.Push(id);

        m->iterations++;

        return;
    }

    BlockFlag BeginScopeBlock(i32 id, cstr label)
    {
        BeginBlock(id, label);
        return BlockFlag{.parent = this};
    }

    void EndBlock()
    {
        u64 now = ReadOSTimer();

        Measurement *m = &measurements[queue.Pop()];
        m->soFar += now - m->from;

        if (queue.len > 0)
        {
            Measurement *prev = &measurements[queue.Last()];
            prev->from = now;
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

        printf("[INFO] Finished profiler %s in %.6f seconds\n", name, f64(ReadOSTimer() - start) / f64(GetOSTimerFreq()));
        for (u64 i = 1; i < measurements.cap; i++)
        {
            auto next = measurements[i];
            if (next.iterations == 0)
                continue;

            printf("\t> %lld) %s [%lld]: %.6f secs total\n", i, next.label, next.iterations, (f64(next.soFar) / f64(GetOSTimerFreq())));
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

#define PROFILE_BLOCK_BEGIN(name) Profiler::Get().BeginBlock(__COUNTER__ + 1, name)
#define PROFILE_BLOCK_END() Profiler::Get().EndBlock()
#define PROFILE_SCOPE(name) auto __flag##__COUNTER__##__ = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, name)
#define PROFILE_FUNCTION() auto __profiler_flag##__COUNTER__##__ = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, __func__)

#define PROFILE(name, code)                            \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name); \
    code;                                              \
    Profiler::Get().EndBlock();\
