#include "lib/engine.h"

struct GameData {
    f32 someNum;
};

GameSettings GameSetup() {
    return (GameSettings){
        .name       = "Test",
        .resolution = {640, 480},
        .glVersion  = {4, 6},
        .permMemory = MB(512),
        .tempMemory = MB(32),
    };
}

void GameInit(GameData* data) {
    *data = (GameData){
        .someNum = 0.0f,
    };
}

void GameUpdate(GameData* data) {
    auto k = GetKey(KEY_F);
    if (k == JUSTPRESSED) {
        INFO("JustPressed");
    };
    if (k == JUSTRELEASED) {
        INFO("JustReleased");
    }
    if (IsKeyDoubleClicked(KEY_F, 0)) {
        INFO("Double Click");
    }
}