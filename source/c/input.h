#pragma once

#include "lib/types.h"

typedef enum InputState {
    INPUTNONE    = 0,
    JUSTPRESSED  = 0b0001,
    PRESSED      = 0b0010,
    INPUTDOWN    = 0b0011,
    RELEASED     = 0b0100,
    JUSTRELEASED = 0b1000,
    INPUTUP      = 0b1100,
} InputState;

typedef enum Key {
    KEY_F1        = SDL_SCANCODE_F1,
    KEY_F2        = SDL_SCANCODE_F2,
    KEY_F3        = SDL_SCANCODE_F3,
    KEY_F4        = SDL_SCANCODE_F4,
    KEY_F5        = SDL_SCANCODE_F5,
    KEY_F6        = SDL_SCANCODE_F6,
    KEY_F7        = SDL_SCANCODE_F7,
    KEY_F8        = SDL_SCANCODE_F8,
    KEY_F9        = SDL_SCANCODE_F9,
    KEY_F10       = SDL_SCANCODE_F10,
    KEY_F12       = SDL_SCANCODE_F12,
    KEY_Q         = SDL_SCANCODE_Q,
    KEY_W         = SDL_SCANCODE_W,
    KEY_E         = SDL_SCANCODE_E,
    KEY_R         = SDL_SCANCODE_R,
    KEY_T         = SDL_SCANCODE_T,
    KEY_Y         = SDL_SCANCODE_Y,
    KEY_U         = SDL_SCANCODE_U,
    KEY_I         = SDL_SCANCODE_I,
    KEY_O         = SDL_SCANCODE_O,
    KEY_P         = SDL_SCANCODE_P,
    KEY_A         = SDL_SCANCODE_A,
    KEY_S         = SDL_SCANCODE_S,
    KEY_D         = SDL_SCANCODE_D,
    KEY_F         = SDL_SCANCODE_F,
    KEY_G         = SDL_SCANCODE_G,
    KEY_H         = SDL_SCANCODE_H,
    KEY_J         = SDL_SCANCODE_J,
    KEY_K         = SDL_SCANCODE_K,
    KEY_L         = SDL_SCANCODE_L,
    KEY_SEMICOLON = SDL_SCANCODE_SEMICOLON,
    KEY_Z         = SDL_SCANCODE_Z,
    KEY_X         = SDL_SCANCODE_X,
    KEY_C         = SDL_SCANCODE_C,
    KEY_V         = SDL_SCANCODE_V,
    KEY_B         = SDL_SCANCODE_B,
    KEY_N         = SDL_SCANCODE_N,
    KEY_M         = SDL_SCANCODE_M,
    KEY_SPACE     = SDL_SCANCODE_SPACE,
    KEY_ESCAPE    = SDL_SCANCODE_ESCAPE,
    KEY_ENTER     = SDL_SCANCODE_RETURN,
    KEY_UP        = SDL_SCANCODE_UP,
    KEY_DOWN      = SDL_SCANCODE_DOWN,
    KEY_LEFT      = SDL_SCANCODE_LEFT,
    KEY_RIGHT     = SDL_SCANCODE_RIGHT,
    KEY_LCTRL     = SDL_SCANCODE_LCTRL,
    KEY_LSHIFT    = SDL_SCANCODE_LSHIFT,
    KEY_LALT      = SDL_SCANCODE_LALT,
    KEY_LGUI      = SDL_SCANCODE_LGUI,
    KEY_RCTRL     = SDL_SCANCODE_RCTRL,
    KEY_RSHIFT    = SDL_SCANCODE_RSHIFT,
    KEY_RALT      = SDL_SCANCODE_RALT,
    KEY_RGUI      = SDL_SCANCODE_RGUI,
    KEY_COUNT     = SDL_SCANCODE_COUNT
} Key;

typedef enum Button {
    BTN_NONE   = 0,
    BTN_LEFT   = SDL_BUTTON_LEFT,
    BTN_MIDDLE = SDL_BUTTON_MIDDLE,
    BTN_RIGHT  = SDL_BUTTON_RIGHT,
    BTN_X1     = SDL_BUTTON_X1,
    BTN_X2     = SDL_BUTTON_X2,
    BTN_COUNT,
} Button;

typedef enum Pad {
    PAD_NONE  = SDL_GAMEPAD_BUTTON_INVALID,
    PAD_SOUTH = SDL_GAMEPAD_BUTTON_SOUTH,
    PAD_NORTH = SDL_GAMEPAD_BUTTON_NORTH,
    PAD_WEST  = SDL_GAMEPAD_BUTTON_WEST,
    PAD_EAST  = SDL_GAMEPAD_BUTTON_EAST,
    PAD_L1    = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    PAD_L3    = SDL_GAMEPAD_BUTTON_LEFT_STICK,
    PAD_R1    = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    PAD_R3    = SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    PAD_START = SDL_GAMEPAD_BUTTON_START,
    PAD_BACK  = SDL_GAMEPAD_BUTTON_BACK,
    PAD_PAD   = SDL_GAMEPAD_BUTTON_TOUCHPAD,
    PAD_UP    = SDL_GAMEPAD_BUTTON_DPAD_UP,
    PAD_DOWN  = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    PAD_LEFT  = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    PAD_RIGHT = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    PAD_COUNT = SDL_GAMEPAD_BUTTON_COUNT
} Pad;

typedef enum PadAxis {
    AXIS_NONE         = SDL_GAMEPAD_AXIS_INVALID,
    AXIS_LEFTX        = SDL_GAMEPAD_AXIS_LEFTX,
    AXIS_LEFTY        = SDL_GAMEPAD_AXIS_LEFTY,
    AXIS_RIGHTX       = SDL_GAMEPAD_AXIS_RIGHTX,
    AXIS_RIGHTY       = SDL_GAMEPAD_AXIS_RIGHTY,
    AXIS_TRIGGERLEFT  = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    AXIS_TRIGGERRIGHT = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    AXIS_COUNT        = SDL_GAMEPAD_AXIS_COUNT
} PadAxis;

typedef enum Gamepad { PAD0, PAD1, PAD2, PAD3, GAMEPAD_COUNT } Gamepad;

typedef struct InputInfo {
    InputState state;
    f32        time; // time up or down
} InputInfo;

typedef struct AnalogInfo {
    i16 value;
} AnalogInfo;

typedef struct InputCtx {
    u32  randomSeed;
    f32  delta;
    f64  prevTime;
    bool quit;

    f32 doubleClickSpeed;

    // KB&M
    InputInfo keys[KEY_COUNT];
    InputInfo buttons[BTN_COUNT];
    v2        mousePos, mouseDelta, wheel;

    // Gamepads
    u32       padIDs[GAMEPAD_COUNT];
    InputInfo pads[GAMEPAD_COUNT][PAD_COUNT];
    i16       axes[GAMEPAD_COUNT][AXIS_COUNT];
} InputCtx;

InputCtx* Input();

InputState GetKey(Key key) {
    return Input()->keys[key].state;
}

bool IsKeyDoubleClicked(Key key, f32 speed) {
    if (speed == 0) speed = Input()->doubleClickSpeed;
    InputInfo info = Input()->keys[key];
    return info.state == JUSTPRESSED && info.time <= speed;
}

bool IsBtnDoubleClicked(Button btn, f32 speed) {
    InputInfo info = Input()->buttons[btn];
    return info.state == JUSTPRESSED && info.time <= speed;
}

bool IsPadDoubleClicked(Pad pad, Gamepad gamepad, f32 speed) {
    InputInfo info = Input()->pads[gamepad][pad];
    return info.state == JUSTPRESSED && info.time <= speed;
}

inline InputState GetButton(Button button) {
    return Input()->buttons[button].state;
}

inline v2 GetMousePos() {
    return Input()->mousePos;
}

inline v2 GetMouseDelta() {
    return Input()->mouseDelta;
}

inline v2 GetWheel() {
    return Input()->wheel;
}

inline InputState GetPad(Pad pad, u32 controller) {
    return Input()->pads[controller][pad].state;
}

inline i16 GetAxis(PadAxis axis, u32 controller) {
    return Input()->axes[controller][axis];
}