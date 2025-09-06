#define ENABLE_PROFILER 1

#include "lib/haversine.hpp"
#include "lib/os.cpp"
#include "engine.hpp"

AppInfo Part3 = {
    .permMemorySize = MB(512),
    .tempMemorySize = TEMP_MEMORY_SIZE,
};

i32 main(i32 argc, cstr argv[]) {
    InitSystem(Part3);
    InitOSMetrics();
    InitRandomSeed();
    WindowCtx window = InitWindow({640, 480}, 4, 6);

    i32 count = SDL_atoi(argc > 1 ? argv[1] : "200000");
    GenerateHaversineJson(count, "input.json");

    {
        auto ptr = Arena::Perm().data;
        REPETITION_PROFILE("WriteToBytes Arena ptr", 2);
        for (size_t i = 0; i < Arena::Perm().cap; i++) {
            ptr[i] = u8(0xCD);
        }
        REPETITION_BANDWIDTH(Arena::Perm().cap);
        REPETITION_END();
    }

    {
        REPETITION_PROFILE("WriteToBytes Arena::Perm()", 2);
        for (size_t i = 0; i < Arena::Perm().cap; i++) {
            Arena::Perm().data[i] = u8(0xCD);
        }
        REPETITION_BANDWIDTH(Arena::Perm().cap);
        REPETITION_END();
    }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("WriteToBytes w/ raw ptr", 30);

        for (size_t i = 0; i < count; i++) {
            testData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        SDL_free(testData);
    }

    {
        auto scope    = Arena::Perm().Scope();
        auto someData = Array<u8>::New(count);

        REPETITION_PROFILE("WriteToBytes w/ Array<u8>", 30);
        for (size_t i = 0; i < someData.cap; i++) {
            someData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(someData.cap);
        REPETITION_END();
    }

    {
        auto      scope = Arena::Perm().Scope();
        Array<u8> file;

        REPETITION_PROFILE("ReadEntireFile", 30);
        if (ENABLE_PROFILER) Arena::Perm().Clear();
        file = ReadEntireFile("input.json");

        REPETITION_BANDWIDTH(file.cap);
        REPETITION_END();
    }

    // REPETITION_PROFILE("ParseHaversineJson", 3);
    // {
    //     // u64  len    = Arena::Perm().len;
    //     auto parsed = ParseHaversineJson(file, count);
    //     REPETITION_BANDWIDTH(file.cap);
    //     Arena::Perm().len = parsed.data.idx;
    // }
    // REPETITION_END();

    return 0;
}