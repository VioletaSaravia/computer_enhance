#define ENABLE_PROFILER 1

#include "graphics.hpp"
#include "haversine.hpp"

i32 main(i32 argc, cstr* argv) {
    PROFILER_NEW("Haversine Sum");
    int pairCount = 0;
    if (argc >= 2) {
        pairCount = atoi(argv[1]);
        if (pairCount < 0) pairCount = 0;
    }

    if (pairCount == 0) {
        pairCount = 25000;
    }
    srand(123456789u);

    GenerateHaversineJson(pairCount, "input.json");

    auto file = ReadEntireFile("input.json");
    if (!file.ok) return 1;

    auto pr = ParseHaversineJson(file, pairCount);

    if (pr.len == 0) {
        fprintf(stderr, "parse failed\n");
        return 1;
    }

    printf("Sum: %.2f\n", SumHaversines(pr));

    return 0;
}