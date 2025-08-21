#include "types.h"
#include "haversine.cpp"

i32 main(i32 argc, cstr *argv)
{
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
    ParseResult pr = {};
    pr = ParseHaversineJson(file, pairCount);

    if (!pr.success)
    {
        fprintf(stderr, "parse failed\n");
        free(pr.result.data);
        return 1;
    }

    SumHaversines(pr.result.data, pr.result.len);

    free(pr.result.data);

    return 0;
}