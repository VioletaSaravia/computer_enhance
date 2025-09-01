#include <math.h>

#define ENABLE_PROFILER
#include "profiler.c"

void blabers() {
    PROFILE_FUNCTION();

    printf("blabers!!!\n");
}

i32 main(i32 argc, cstr* argv) {
    PROFILER_NEW();

    blabers();

    {
        PROFILE_SCOPE("Vector ops");
        v2  some     = {1, 1};
        f32 tripleIt = 3;

        v2 tripleSome = Mult(some, tripleIt);
        v2 plusOne    = {1, 1};

        v2 plused = Add(tripleSome, plusOne);

        f32 betterPlusOne = 1;
        v2  morePlused    = Add(plused, betterPlusOne);

        v2  theLimit         = (v2){3, 1};
        f32 crossedAndPlused = Cross(morePlused, Add(morePlused, V2(3, 1)));

        printf("v2(%.2f %.2f) cross: %.2f\n", morePlused.x, morePlused.y, crossedAndPlused);
    }

    PROFILER_END();
}