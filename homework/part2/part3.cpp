#define ENABLE_PROFILER 1

#include "haversine.hpp"

i32 main(i32 argc, cstr* argv) {
    i32 count = atoi(argc > 1 ? argv[1] : "100000");
    GenerateHaversineJson(count, "input.json");

    Result<Array<u8>, ReadFileError> file = {};
    REPETITION_PROFILE("ReadEntireFile", 500);
    file = ReadEntireFile("input.json");
    REPETITION_BANDWIDTH(file.value.cap);
    if (ENABLE_PROFILER) Arena::Perm().Clear();
    REPETITION_END();

    REPETITION_PROFILE("ParseHaversineJson", 500);
    auto _ = ParseHaversineJson(file, count);
    REPETITION_BANDWIDTH(file.value.cap);
    if (ENABLE_PROFILER) Arena::Perm().Clear();
    REPETITION_END();

    return 0;
}