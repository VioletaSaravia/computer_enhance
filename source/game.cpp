#include "lib/engine.hpp"

struct Game::Data {
    f32 someNum;
};

Game::Settings Game::Setup() {
    return Game::Settings{
        .name       = "Test",
        .resolution = {640, 480},
        .glVersion  = {4, 6},
        .permMemory = MB(512),
        .tempMemory = MB(32),
    };
}

void Game::Init(Game::Data* data) {
    *data = {
        .someNum = TWEAK(data->someNum, 0, 1),
    };
    
}

void Game::Update(Game::Data* data) {
    auto k = GetKey(Key::F);
    if (k == InputState::JustPressed) {
        INFO("JustPressed");
    };
    if (k == InputState::JustReleased) {
        INFO("JustReleased");
    }
    if (IsDoubleClicked(Key::F)) {
        INFO("Double Click");
    }
}