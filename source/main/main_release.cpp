#include "app.hpp"
#include "types.hpp"

i32 main() {
    Init();
    while (!ShouldClose()) Update();
    Shutdown();
}