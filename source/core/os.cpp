#if defined(_WIN32)
#include "core/os_win32.cpp"
#elif defined(__linux__)
#include "core/os_linux.cpp"
#elif defined(__EMSCRIPTEN__)
#include "core/os_wasm.cpp"
#else
#error "Unsupported platform"
#endif