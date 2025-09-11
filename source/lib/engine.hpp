#pragma once

#include "core/opengl.hpp"
#include "core/window.hpp"

#include "lib/containers.hpp"
#include "lib/os.hpp"

#include "game.hpp"

enum class InputState : u8 {
    None         = 0,
    JustPressed  = 0b0001,
    Pressed      = 0b0010,
    Down         = 0b0011,
    Released     = 0b0100,
    JustReleased = 0b1000,
    Up           = 0b1100,
};

enum class Key : u32 {
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
};

enum class Pad : i8 {
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
};

enum class Controllers { Controller0, Controller1, Controller2, Controller3, COUNT };

struct Editor {
    bool                 hide;
    StackArray<f32*, 64> tweakables;

    void ShowAndUpdate() {
        ImGui::Begin("Parameters", &hide);
        if (ImGui::Button("Close")) hide = true;

        for (auto i : tweakables) {
            ImGui::DragFloat("var", i);
        }

        ImGui::End();
    }
};

struct InputInfo {
    InputState state;
    f32        time; // held time or released time
};

struct InputCtx {
    u32       randomSeed;
    f32       delta;
    f64       prevTime;
    bool      quit;
    InputInfo keys[(u32)(Key::COUNT)];
    InputInfo pads[(u32)(Controllers::COUNT)][(u32)(Pad::COUNT)];
};

struct GameMemory {
    OS::SystemInfo  info;
    OS::Metrics     metrics;
    InputCtx        input;
    Game::Data*     data;
    GL::GraphicsCtx gfx;
    Arena           perm;
    Arena           temp;
    WindowCtx       window;
    Editor          editor;
};

global GameMemory* Mem;

Arena& Arena::Perm() {
    return Mem->perm;
}

Arena& Arena::Temp() {
    return Mem->temp;
}

f32 Tweak(f32* val, f32 from, f32 to) {
    Mem->editor.tweakables.Push(val);
    return *val;
}

EXPORT void EngineInit() {
    auto settings = Game::Setup();
    Mem = (GameMemory*)SDL_malloc(sizeof(GameMemory) + settings.permMemory + settings.tempMemory);
    if (!Mem) SDLFatal();

    *Mem = {
        .info    = OS::SystemInfo::Init(),
        .metrics = OS::Metrics::Init(),
        .input =
            {
                .randomSeed = Rand::Init(),
            },
        .data   = (Game::Data*)((u8*)(Mem) + sizeof(GameMemory)),
        .gfx    = {.clearColor = {0.4, 0, 0.6, 1}},
        .perm   = Arena(settings.permMemory),
        .temp   = Arena(settings.tempMemory),
        .window = WindowCtx::Init(settings),
    };
    ImguiInit(Mem->window);

    INFO("GPU Information" LIST_VAR "Name: \t\t\t%s" LIST_VAR "Vendor: \t\t\t%s" LIST_VAR
         "OpenGL version: \t\t%s",
         glGetString(GL_RENDERER),
         glGetString(GL_VENDOR),
         glGetString(GL_VERSION));

    Game::Init(Mem->data);
}

f32 Delta() {
    return Mem->input.delta;
}

void UpdateEvents() {
    // TODO Double check logic
    for (auto& i : Mem->input.keys) {
        switch (i.state) {
        case InputState::None:
        case InputState::JustReleased: {
            i.state = InputState::Released;
            i.time  = 0.0f;
            break;
        }

        case InputState::JustPressed: {
            i.state = InputState::Pressed;
            i.time  = 0.0f;
            break;
        }

        case InputState::Pressed:
        case InputState::Released: {
            i.time += Delta();
            break;
        }
        default: break;
        }
    }

    for (auto& pad : Mem->input.pads) {
        for (auto& i : pad) {
            switch (i.state) {
            case InputState::None:
            case InputState::JustReleased: {
                i.state = InputState::Released;
                i.time  = 0.0f;
                break;
            }

            case InputState::JustPressed: {
                i.state = InputState::Pressed;
                i.time = 0.0f;
                break;
            }

            case InputState::Pressed:
            case InputState::Released: {
                i.time += Delta();
                break;
            }
            default: break;
            }
        }
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        switch (event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            if (event.window.windowID == SDL_GetWindowID(Mem->window.window))
                Mem->input.quit = true;
            break;
        }

        case SDL_EVENT_QUIT: {
            Mem->input.quit = true;
            break;
        }

        case SDL_EVENT_GAMEPAD_ADDED: {
            break;
        }

        case SDL_EVENT_GAMEPAD_REMOVED: {
            break;
        }

        case SDL_EVENT_KEY_DOWN: {
            Mem->input.keys[event.key.scancode].state = InputState::JustPressed; // WRONG ?
            break;
        }

        case SDL_EVENT_KEY_UP: {
            Mem->input.keys[event.key.scancode].state = InputState::JustReleased;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            Mem->input.pads[0][event.gbutton.button].state = InputState::JustPressed;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            Mem->input.pads[0][event.gbutton.button].state = InputState::JustReleased;
            break;
        }

        default: break;
        }
    }
}

EXPORT void EngineUpdate() {
    UpdateEvents();

    if (SDL_GetWindowFlags(Mem->window.window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10);
        return;
    }

    Game::Update(Mem->data);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    static bool show_demo_window = true;
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

    Mem->editor.ShowAndUpdate();

    ImGui::Render();

    GL::Begin();
    glViewport(0, 0, Mem->window.initialResolution.x, Mem->window.initialResolution.y);
    GL::Clear(Mem->gfx.clearColor);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(Mem->window.window);
    GL::End();

    // Uncapped timestep
    f64 timeCur         = (f64)(SDL_GetPerformanceCounter()) / (f64)(SDL_GetPerformanceFrequency());
    Mem->input.delta    = (f32)(timeCur - Mem->input.prevTime);
    Mem->input.prevTime = timeCur;
}

EXPORT void EngineShutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Mem->window.Destroy();
    SDL_Quit();
}

EXPORT bool ShouldClose() {
    return Mem->input.quit;
}

EXPORT void* GetMemory() {
    return (void*)Mem;
}

EXPORT u64 MemorySize() {
    return sizeof(GameMemory) + Mem->perm.cap + Mem->temp.cap;
}

EXPORT void HotReloaded(void* memory) {
    Mem = (GameMemory*)memory;
}

EXPORT bool ForceReload() {
    return false;
}

EXPORT bool ForceRestart() {
    return false;
}