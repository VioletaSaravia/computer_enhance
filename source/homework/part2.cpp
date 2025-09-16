#define ENABLE_PROFILER 1

#include "lib/os.cpp"

#include "lib/graphics.hpp"
#include "lib/haversine.hpp"

i32 Part2(i32 argc, cstr argv[]) {
    SystemInfo::Init();
    Metrics::Init();
    Rand::Init();

    Profiler::New("Haversine Sum");

    int pairCount = 0;
    if (argc >= 2) {
        pairCount = SDL_atoi(argv[1]);
        if (pairCount < 0) pairCount = 0;
    }

    if (pairCount == 0) pairCount = 100000;

    GenerateHaversineJson(pairCount, "input.json");

    auto file = ReadEntireFile("input.json");
    auto pr   = ParseHaversineJson(file, pairCount);

    printf("Sum: %.2f\n", SumHaversines(pr));

    return 0;
}