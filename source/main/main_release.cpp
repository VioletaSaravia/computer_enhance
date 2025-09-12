#include "lib/os.cpp"

#include "lib/engine.hpp"
#include "main/runtime.hpp"

#include "game.cpp"

#include "homework/part3.cpp"

i32 main(i32 argc, cstr argv[]) {
    // Part3(argc, argv);
    EngineInit();
    while (!ShouldClose()) EngineUpdate();
    EngineShutdown();
}