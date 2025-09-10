#pragma once

#include "lib/types.hpp"

namespace Game {

struct Settings {
    cstr name;
    v2   resolution;
    v2   glVersion;
    u64  permMemory, tempMemory;
};

Settings Setup();

struct Data;

void Init(Data*);

void Update(Data*);

}; // namespace Game