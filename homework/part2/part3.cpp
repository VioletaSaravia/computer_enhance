#define ENABLE_PROFILER 1

#include "os.cpp"

#include "haversine.hpp"

i32 main(i32 argc, cstr* argv) {
    OS::InitializeMetrics();
    Arena::Perm(MB(1024));
    Rand::Init();

    i32 count = atoi(argc > 1 ? argv[1] : "200000");
    GenerateHaversineJson(count, "input.json");

    u8* testData = (u8*)OS::Alloc(count);
    REPETITION_PROFILE("WriteToBytes w/ raw ptr", 100);
    {
        for (size_t i = 0; i < count; i++) {
            testData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(count);
    }
    REPETITION_END();

    auto someData = Array<u8>::New(count);
    REPETITION_PROFILE("WriteToBytes w/ Array<u8>", 100);
    {
        for (size_t i = 0; i < someData.cap; i++) {
            someData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(someData.cap);
    }
    REPETITION_END();

    Array<u8> file;
    REPETITION_PROFILE("ReadEntireFile", 100);
    {
        if (ENABLE_PROFILER) Arena::Perm().Clear();
        file = ReadEntireFile("input.json");

        REPETITION_BANDWIDTH(file.cap);
    }
    REPETITION_END();

    // REPETITION_PROFILE("ParseHaversineJson", 100);
    // {
    //     auto _ = ParseHaversineJson(file, count);

    //     REPETITION_BANDWIDTH(file.cap);
    //     if (ENABLE_PROFILER) Arena::Perm().Clear();
    // }
    // REPETITION_END();

    return 0;
}