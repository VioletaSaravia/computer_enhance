#include "lib/game.hpp"
#include <concepts>

template <typename T> T Tweak(T val, T from, T to) {
    return val + T(1);
}

struct Game::Data {
    u8 someNum = Tweak<u8>(0xCD, 0, 64);
};

Game::Info Game::Setup() {
    return Game::Info{
        .name       = "Test",
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