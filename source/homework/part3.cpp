#define ENABLE_PROFILER 1

#include "lib/engine.hpp"
#include "lib/haversine.hpp"

extern "C" void MOVAllBytesASM(u64 Count, u8* Data);
extern "C" void NOPAllBytesASM(u64 Count);
extern "C" void CMPAllBytesASM(u64 Count);
extern "C" void DECAllBytesASM(u64 Count);

extern "C" void NOP3x1AllBytes(u64 Count, u8* Data);
extern "C" void NOP1x3AllBytes(u64 Count, u8* Data);
extern "C" void NOP1x9AllBytes(u64 Count, u8* Data);

i32 Part3(i32 argc, cstr argv[]) {
    auto info          = OS::SystemInfo::Init();
    PlaceholderMetrics = OS::Metrics::Init();
    auto seed          = Rand::Init();

    i32 count   = SDL_atoi(argc > 1 ? argv[1] : "200000");
    i32 repeats = SDL_atoi(argc > 2 ? argv[2] : "50");
    GenerateHaversineJson(count, "input.json");

    // {
    //     auto ptr = Arena::Perm().data;
    //     REPETITION_PROFILE("WriteToBytes Arena ptr", 2);
    //     for (size_t i = 0; i < Arena::Perm().cap; i++) {
    //         ptr[i] = u8(0xCD);
    //     }
    //     REPETITION_BANDWIDTH(Arena::Perm().cap);
    //     REPETITION_END();
    // }

    // {
    //     REPETITION_PROFILE("WriteToBytes Arena::Perm()", 2);
    //     for (size_t i = 0; i < Arena::Perm().cap; i++) {
    //         Arena::Perm().data[i] = u8(0xCD);
    //     }
    //     REPETITION_BANDWIDTH(Arena::Perm().cap);
    //     REPETITION_END();
    // }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("MOVAllBytesASM", repeats);

        MOVAllBytesASM(count, testData);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        SDL_free(testData);
    }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("NOP3x1AllBytes", repeats);

        NOP3x1AllBytes(count, testData);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        SDL_free(testData);
    }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("NOP1x3AllBytes", repeats);

        NOP1x3AllBytes(count, testData);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        SDL_free(testData);
    }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("NOP1x9AllBytes", repeats);

        NOP1x9AllBytes(count, testData);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
        SDL_free(testData);
    }

    {
        REPETITION_PROFILE("NOPAllBytesASM", repeats);

        NOPAllBytesASM(count);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
    }

    {
        REPETITION_PROFILE("CMPAllBytesASM", repeats);

        CMPAllBytesASM(count);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
    }

    {
        REPETITION_PROFILE("DECAllBytesASM", repeats);

        DECAllBytesASM(count);

        REPETITION_BANDWIDTH(count);
        REPETITION_END();
    }

    {
        u8* testData = (u8*)SDL_malloc(count);
        REPETITION_PROFILE("WriteToBytes w/ raw ptr", repeats);

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

        REPETITION_PROFILE("WriteToBytes w/ Array<u8>", repeats);
        for (size_t i = 0; i < someData.cap; i++) {
            someData[i] = u8(i);
        }

        REPETITION_BANDWIDTH(someData.cap);
        REPETITION_END();
    }

    {
        auto scope    = Arena::Perm().Scope();
        auto someData = Array<u8>::New(count);
        someData.len  = someData.cap;

        REPETITION_PROFILE("WriteToBytes w/ Array<u8> iterator", repeats);
        int j = 0;
        for (auto& i : someData) {
            i = u8(j);
            j++;
        }

        REPETITION_BANDWIDTH(someData.cap);
        REPETITION_END();
    }

    {
        auto scope    = Arena::Perm().Scope();
        auto someData = Array<u8>::New(count);
        someData.len  = someData.cap;

        REPETITION_PROFILE("WriteToBytes w/ Array<u8> iterator 0xCD", repeats);
        for (auto& i : someData) {
            i = 0xCD;
        }

        REPETITION_BANDWIDTH(someData.cap);
        REPETITION_END();
    }

    {
        auto scope = Arena::Perm().Scope();

        Array<u8> file;
        REPETITION_PROFILE("ReadEntireFile", repeats);
        {
            auto scope2 = Arena::Perm().Scope(); // careful
            file        = ReadEntireFile("input.json");

            REPETITION_BANDWIDTH(file.cap);
        }
        REPETITION_END();

        Array<HaversinePair> parsed;
        REPETITION_PROFILE("ParseHaversineJson", 5);
        {
            auto scope2 = Arena::Perm().Scope(); // careful
            parsed      = ParseHaversineJson(file, count);

            REPETITION_BANDWIDTH(file.cap);
        }
        REPETITION_END();

        REPETITION_PROFILE("SumHaversines", 10);
        {
            f64 sum = SumHaversines(parsed);

            REPETITION_BANDWIDTH(parsed.len * sizeof(HaversinePair));
        }
        REPETITION_END();
    }

    return 0;
}