#define ENABLE_PROFILER

#include "haversine.cpp"
#include "types.hpp"

i32 main(i32 argc, cstr* argv) {
    i32 count = atoi(argc > 1 ? argv[1] : "1000000");
    GenerateHaversineJson(count, "input.json");

    Array<u8> file = {};
    REPETITION_PROFILE("ReadEntireFile", 500);
    file = ReadEntireFile("input.json");
    REPETITION_BANDWIDTH(file.cap);
    Arena::Perm().Clear();
    REPETITION_END();

    REPETITION_PROFILE("ParseHaversineJson", 500);
    auto _ = ParseHaversineJson(file, count);
    REPETITION_BANDWIDTH(file.cap);
    Arena::Perm().Clear();
    REPETITION_END();

    return 0;
}