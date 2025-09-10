#pragma once

#include "lib/types.hpp"

namespace Game {

struct Info {
    cstr name;
    u64 permMemory;
    u64 tempMemory;
};

Info Setup();

struct Data;

void Init(Data*);

void Update(Data*);

}; // namespace Game