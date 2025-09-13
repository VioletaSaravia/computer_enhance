#pragma once

#include "lib/types.h"

typedef struct GameSettings {
    cstr name;
    v2   resolution;
    v2   glVersion;
    u64  permMemory, tempMemory;
} GameSettings;

GameSettings GameSetup();

typedef struct GameData GameData;

void GameInit(GameData*);

void GameUpdate(GameData*);