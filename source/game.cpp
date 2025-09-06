#include "lib/game.hpp"

struct Game::Data {
    u8 num;
};

void Game::Init(Game::Data* data) {
    *data = {.num = 0xCD};
}

void Game::Update(Game::Data* data) {
    INFO("%d", data->num);
}