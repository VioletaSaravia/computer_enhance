#define ENABLE_PROFILER

#include "types.h"
#include "haversine.cpp"

i32 main(i32 argc, cstr *argv)
{
    Arena::Init(1024 * 1024 * 1024);

    i32 count = atoi(argc > 1 ? argv[1] : "1000000");
    GenerateHaversineJson(count, "input.json");

    Array<u8> file = {};
    REPETITION_PROFILE("ReadEntireFile", 500);
    file = ReadEntireFile("input.json");
    REPETITION_BANDWIDTH(file.cap);
    Arena::Clear();
    REPETITION_END();

    REPETITION_PROFILE("ParseHaversineJson", 500);
    auto pr = ParseHaversineJson(file, count);
    REPETITION_BANDWIDTH(file.cap);
    Arena::Clear();
    REPETITION_END();

    return 0;
}