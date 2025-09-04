#define ENABLE_PROFILER 1

#include "os.cpp"

#include "haversine.hpp"

// TODO
// COUNT CYCLES IN LOOP ITERATION

i32 main(i32 argc, cstr argv[]) {
    OS::InitializeMetrics();
    Arena::Perm(MB(1024));
    Rand::Init();
    INFO("CPU Freq: %.2f Ghz", f64(OS::EstimateCPUTimerFreq()) / 1000.0 / 1000.0 / 1000.0);

    i32 count = atoi(argc > 1 ? argv[1] : "200000");
    GenerateHaversineJson(count, "input.json");

    {
        auto ptr = Arena::Perm().data;
        REPETITION_PROFILE("WriteToBytes Arena", 3);
        for (size_t i = 0; i < Arena::Perm().cap; i++) {
            ptr[i] = u8(0xCD);
        }
        REPETITION_BANDWIDTH(Arena::Perm().cap);
        REPETITION_END();
    }

    {
        u8* testData = (u8*)OS::Alloc(count);
        REPETITION_PROFILE("WriteToBytes w/ raw ptr", 30);

        for (size_t i = 0; i < count; i++) {
            testData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        OS::Free(testData);
    }

    {
        auto scope    = Arena::Perm().Scope();
        auto someData = Array<u8>::New(count);

        REPETITION_PROFILE("WriteToBytes w/ Array<u8>", 30);
        for (size_t i = 0; i < someData.cap; i++) {
            someData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(someData.cap);
        REPETITION_END();
    }

    {
        auto      scope = Arena::Perm().Scope();
        Array<u8> file;

        REPETITION_PROFILE("ReadEntireFile", 30);
        if (ENABLE_PROFILER) Arena::Perm().Clear();
        file = ReadEntireFile("input.json");

        REPETITION_BANDWIDTH(file.cap);
        REPETITION_END();
    }
    // 3.29 * 1000^3
    // 3.83 * 1024^3

    // REPETITION_PROFILE("ParseHaversineJson", 10);
    // {
    //     // u64  len    = Arena::Perm().len;
    //     auto parsed = ParseHaversineJson(file, count);
    //     REPETITION_BANDWIDTH(file.cap);
    //     Arena::Perm().len = parsed.data.idx;
    // }
    // REPETITION_END();

    return 0;
}