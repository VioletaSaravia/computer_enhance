#if defined(_WIN32)
#include "lib/os_win32.cpp"
#elif defined(__linux__)
#include "lib/os_linux.cpp"
#elif defined(__EMSCRIPTEN__)
#include "lib/os_wasm.cpp"
#else
#error "Unsupported platform"
#endif