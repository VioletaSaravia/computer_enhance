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
    REPETITION_PROFILE("WriteToBytes w/ raw ptr", 30);
    {
        for (size_t i = 0; i < count; i++) {
            testData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(count);
    }
    REPETITION_END();
    OS::Free(testData);

    auto someData = Array<u8>::New(count);
    REPETITION_PROFILE("WriteToBytes w/ Array<u8>", 30);
    {
        for (size_t i = 0; i < someData.cap; i++) {
            someData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(someData.cap);
    }
    REPETITION_END();
    Arena::Perm().Clear();

    Array<u8> file;
    REPETITION_PROFILE("ReadEntireFile", 30);
    {
        if (ENABLE_PROFILER) Arena::Perm().Clear();
        file = ReadEntireFile("input.json");

        REPETITION_BANDWIDTH(file.cap);
    }
    REPETITION_END();
    INFO("FAULTS: %llu", OS::ReadPageFaultCount());

    REPETITION_PROFILE("ParseHaversineJson", 10);
    {
        auto parsed = ParseHaversineJson(file, count);
        REPETITION_BANDWIDTH(file.cap);
        Arena::Perm().len -= parsed.len * sizeof(HaversinePair);
    }
    REPETITION_END();
    INFO("FAULTS: %llu", OS::ReadPageFaultCount());

    return 0;
}