#if defined(_WIN32)
#include "lib/os_win32.c"
#elif defined(__linux__)
#include "lib/os_linux.c"
#elif defined(__EMSCRIPTEN__)
#include "lib/os_wasm.c"
#else
#error "Unsupported platform"
#endif