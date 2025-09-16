#pragma once

#include "core/types.hpp"

struct Settings {
    cstr name;
    v2   resolution;
    v2   glVersion;
    u64  memory;
};

Settings Setup();

struct Data;

void Init(Data*);

void Update(Data*);