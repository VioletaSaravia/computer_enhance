#define ENABLE_PROFILER 1

#include "haversine.hpp"

global const u32 seed = 123456789u;

i32 main(i32 argc, cstr* argv) {
    InitializeOSMetrics();
    Arena::Perm(MB(1024));
    srand(seed);

    i32 count = atoi(argc > 1 ? argv[1] : "200000");
    GenerateHaversineJson(count, "input.json");

    Array<u8> file;
    REPETITION_PROFILE("ReadEntireFile", 500);
    file = ReadEntireFile("input.json");
    REPETITION_BANDWIDTH(file.cap);
    if (ENABLE_PROFILER) Arena::Perm().Clear();
    REPETITION_END();

    REPETITION_PROFILE("ParseHaversineJson", 500);
    auto _ = ParseHaversineJson(file, count);
    REPETITION_BANDWIDTH(file.cap);
    if (ENABLE_PROFILER) Arena::Perm().Clear();
    REPETITION_END();

    return 0;
}