#pragma once

#include "lib/types.hpp"

typedef enum class InputState : u8 {
    None         = 0,
    JustPressed  = 0b0001,
    Pressed      = 0b0010,
    Down         = 0b0011,
    Released     = 0b0100,
    JustReleased = 0b1000,
    Up           = 0b1100,
} InputState;

typedef enum class Key : u32 {
    F1        = SDL_SCANCODE_F1,
    F2        = SDL_SCANCODE_F2,
    F3        = SDL_SCANCODE_F3,
    F4        = SDL_SCANCODE_F4,
    F5        = SDL_SCANCODE_F5,
    F6        = SDL_SCANCODE_F6,
    F7        = SDL_SCANCODE_F7,
    F8        = SDL_SCANCODE_F8,
    F9        = SDL_SCANCODE_F9,
    F10       = SDL_SCANCODE_F10,
    F12       = SDL_SCANCODE_F12,
    Q         = SDL_SCANCODE_Q,
    W         = SDL_SCANCODE_W,
    E         = SDL_SCANCODE_E,
    R         = SDL_SCANCODE_R,
    T         = SDL_SCANCODE_T,
    Y         = SDL_SCANCODE_Y,
    U         = SDL_SCANCODE_U,
    I         = SDL_SCANCODE_I,
    O         = SDL_SCANCODE_O,
    P         = SDL_SCANCODE_P,
    A         = SDL_SCANCODE_A,
    S         = SDL_SCANCODE_S,
    D         = SDL_SCANCODE_D,
    F         = SDL_SCANCODE_F,
    G         = SDL_SCANCODE_G,
    H         = SDL_SCANCODE_H,
    J         = SDL_SCANCODE_J,
    K         = SDL_SCANCODE_K,
    L         = SDL_SCANCODE_L,
    Semicolon = SDL_SCANCODE_SEMICOLON,
    Z         = SDL_SCANCODE_Z,
    X         = SDL_SCANCODE_X,
    C         = SDL_SCANCODE_C,
    V         = SDL_SCANCODE_V,
    B         = SDL_SCANCODE_B,
    N         = SDL_SCANCODE_N,
    M         = SDL_SCANCODE_M,
    Space     = SDL_SCANCODE_SPACE,
    Escape    = SDL_SCANCODE_ESCAPE,
    Enter     = SDL_SCANCODE_RETURN,
    Up        = SDL_SCANCODE_UP,
    Down      = SDL_SCANCODE_DOWN,
    Left      = SDL_SCANCODE_LEFT,
    Right     = SDL_SCANCODE_RIGHT,
    LCtrl     = SDL_SCANCODE_LCTRL,
    LShift    = SDL_SCANCODE_LSHIFT,
    LAlt      = SDL_SCANCODE_LALT,
    LGui      = SDL_SCANCODE_LGUI,
    RCtrl     = SDL_SCANCODE_RCTRL,
    RShift    = SDL_SCANCODE_RSHIFT,
    RAlt      = SDL_SCANCODE_RALT,
    RGui      = SDL_SCANCODE_RGUI,
    COUNT     = SDL_SCANCODE_COUNT
} Key;

typedef enum class Button : u8 {
    None   = 0,
    Left   = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right  = SDL_BUTTON_RIGHT,
    X1     = SDL_BUTTON_X1,
    X2     = SDL_BUTTON_X2,
    COUNT,
} Button;

typedef enum class Pad : i8 {
    None  = SDL_GAMEPAD_BUTTON_INVALID,
    South = SDL_GAMEPAD_BUTTON_SOUTH,
    North = SDL_GAMEPAD_BUTTON_NORTH,
    West  = SDL_GAMEPAD_BUTTON_WEST,
    East  = SDL_GAMEPAD_BUTTON_EAST,
    L1    = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,
    L3    = SDL_GAMEPAD_BUTTON_LEFT_STICK,
    R1    = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    R3    = SDL_GAMEPAD_BUTTON_RIGHT_STICK,
    Start = SDL_GAMEPAD_BUTTON_START,
    Back  = SDL_GAMEPAD_BUTTON_BACK,
    Pad   = SDL_GAMEPAD_BUTTON_TOUCHPAD,
    Up    = SDL_GAMEPAD_BUTTON_DPAD_UP,
    Down  = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    Left  = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    Right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    COUNT = SDL_GAMEPAD_BUTTON_COUNT
} Pad;

typedef enum class PadAxis : i8 {
    None         = SDL_GAMEPAD_AXIS_INVALID,
    LeftX        = SDL_GAMEPAD_AXIS_LEFTX,
    LeftY        = SDL_GAMEPAD_AXIS_LEFTY,
    RightX       = SDL_GAMEPAD_AXIS_RIGHTX,
    RightY       = SDL_GAMEPAD_AXIS_RIGHTY,
    TriggerLeft  = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    TriggerRight = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    COUNT        = SDL_GAMEPAD_AXIS_COUNT
} PadAxis;

typedef enum class Controllers {
    Controller0,
    Controller1,
    Controller2,
    Controller3,
    COUNT
} Controllers;

typedef struct InputInfo {
    InputState state;
    f32        time; // held time or released time
} InputInfo;

typedef struct AnalogInfo {
    i16 value;
} AnalogInfo;

typedef struct InputCtx {
    u32  randomSeed;
    f32  delta;
    f64  prevTime;
    bool quit;

    // KB&M
    InputInfo keys[(u32)(Key::COUNT)];
    InputInfo buttons[(u32)(Button::COUNT)];
    v2        mousePos, mouseDelta, wheel;

    // Gamepads
    u32       padIDs[(u32)(Controllers::COUNT)];
    InputInfo pads[(u32)(Controllers::COUNT)][(u32)(Pad::COUNT)];
    i16       axes[(u32)(Controllers::COUNT)][(u32)(PadAxis::COUNT)];
} InputCtx;