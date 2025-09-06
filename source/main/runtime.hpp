#include "lib/os.hpp"

extern "C" {
// Forces use of the discrete NVIDIA GPU
__declspec(dllexport) u32 NvOptimusEnablement = 1;

// Forces use of the discrete AMD GPU
__declspec(dllexport) i32 AmdPowerXpressRequestHighPerformance = 1;
}