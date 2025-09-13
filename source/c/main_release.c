#include "lib/os.c"

#include "lib/engine.h"
#include "main/runtime.h"

#include "game.c"

i32 main(i32 argc, cstr argv[]) {
    EngineInit();
    while (!ShouldClose()) EngineUpdate();
    EngineShutdown();
}