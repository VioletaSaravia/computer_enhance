#define ENABLE_PROFILER

#include "types.h"
#include "haversine.cpp"

i32 main(i32 argc, cstr *argv)
{
    Arena::Init(1024 * 1024 * 1024);

    PROFILER_NEW("Haversine Sum");
    int pairCount = 0;
    if (argc >= 2)
    {
        pairCount = atoi(argv[1]);
        if (pairCount < 0)
            pairCount = 0;
    }

    if (pairCount == 0)
    {
        pairCount = 25000;
    }
    srand(123456789u);

    GenerateHaversineJson(pairCount, "input.json");

    Array<u8> file = ReadEntireFile("input.json");
    if (!file.data)
    {
        fprintf(stderr, "[ERROR] Failed to read input.json\n");
        return 1;
    };
    auto pr = ParseHaversineJson(file, pairCount);

    if (pr.len == 0)
    {
        fprintf(stderr, "parse failed\n");
        return 1;
    }

    printf("Sum: %.2f\n", SumHaversines(pr.data, pr.len));

    return 0;
}