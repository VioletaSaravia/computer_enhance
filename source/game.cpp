#include "lib/game.hpp"
#include <concepts>

struct Game::Data {
    f32 someNum = Tweak(&someNum, 0.0f, 64.0f);
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
}

void Game::Update(Game::Data* data) {
    INFO("%d", data->someNum);
}