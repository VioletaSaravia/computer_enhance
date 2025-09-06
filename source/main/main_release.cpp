#include "lib/os.cpp"

#include "lib/engine.hpp"
#include "main/runtime.hpp"

#include "game.cpp"

i32 main(i32 argc, cstr argv[]) {
    Init();
    while (!ShouldClose()) Update();
    Shutdown();
}