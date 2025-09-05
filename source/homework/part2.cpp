#define ENABLE_PROFILER 1

#include "os.cpp"

#include "graphics.hpp"
#include "haversine.hpp"

i32 main(i32 argc, cstr argv[]) {
    OS::InitializeMetrics();
    Arena::Perm(MB(1024));
    Rand::Init();
    Profiler::New("Haversine Sum");

    int pairCount = 0;
    if (argc >= 2) {
        pairCount = atoi(argv[1]);
        if (pairCount < 0) pairCount = 0;
    }

    if (pairCount == 0) pairCount = 100000;

    GenerateHaversineJson(pairCount, "input.json");

    auto file = ReadEntireFile("input.json");
    auto pr   = ParseHaversineJson(file, pairCount);

    printf("Sum: %.2f\n", SumHaversines(pr));

    return 0;
}