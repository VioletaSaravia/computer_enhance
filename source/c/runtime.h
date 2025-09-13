#pragma once

// Forces use of the discrete NVIDIA GPU
__declspec(dllexport) unsigned int NvOptimusEnablement = 1;

// Forces use of the discrete AMD GPU
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;