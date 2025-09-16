#include "core/engine.hpp"

struct Data {
    f32 someNum;
};

Settings Setup() {
    return Settings{
        .name       = "Test",
        .resolution = {1280, 720},
        .glVersion  = {4, 6},
        .memory     = sizeof(Data),
    };
}

void Init(Data* data) {
    *data = {};
}

void Update(Data* data) {
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