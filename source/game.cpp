#include "lib/engine.hpp"

struct Game::Data {
    f32 someNum = Tweak(&someNum, 0.0f, 64.0f); // MACRO
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
    *data = {};
    View(&Mem->input.mousePos, {}, {640, 480});
    View(&Mem->input.mouseDelta, {-100, -100}, {100, 100});
    View(&Mem->input.wheel, {-1, -1}, {1, 1});
}

void Game::Update(Game::Data* data) {
    auto k = GetKey(Key::F);
    if (k == InputState::JustPressed) {
        INFO("BLABERS");
    };
}