#include "types.h"
#include "haversine.cpp"

i32 main(i32 argc, cstr *argv)
{
    GenerateHaversineJson(50000, "input.json");

    // REPETITION_PROFILE("ReadEntireFile outer w/ malloc", 5000);
    Array<u8> file = ReadEntireFile("input.json");
    // REPETITION_BANDWIDTH(file.cap);
    // REPETITION_END();
}