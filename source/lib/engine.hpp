#pragma once

#include "core/opengl.hpp"
#include "core/window.hpp"

#include "lib/containers.hpp"
#include "lib/game.hpp"
#include "lib/input.hpp"
#include "lib/os.hpp"

template <typename T> struct NamedPtr {
    cstr name;
    T*   ptr;
};

enum class TweakableType {
    None,
    f32,
    v2,
    v3,
};

struct Tweakable {
    TweakableType t;
    cstr          name;
    union {
        f32* float32;
        v2*  vec2;
        v3*  vec3;
    };
};

typedef struct Editor {
    bool                      show;
    StackArray<Tweakable, 32> tweakables;

    void ShowAndUpdate() {
        if (GetKey(Key::F1) == InputState::JustPressed) show ^= true;
        if (!show) return;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(200, 1080), ImGuiCond_Always);

        ImGui::Begin("Debug", &show, flags);

        for (auto i : tweakables) {
            switch (i.t) {
            case TweakableType::f32: ImGui::InputFloat(i.name, i.float32); break;
            case TweakableType::v2: ImGui::InputFloat2(i.name, (f32*)(i.vec2)); break;
            case TweakableType::v3: ImGui::InputFloat3(i.name, (f32*)(i.vec3)); break;
            default: break;
            }
        }

        ImGui::End();
    }
} Editor;

typedef struct GameMemory {
    OS::SystemInfo  info;
    OS::Metrics     metrics;
    InputCtx        input;
    Game::Data*     data;
    GL::GraphicsCtx gfx;
    Arena           perm;
    Arena           temp;
    WindowCtx       window;
    Editor          editor;
} GameMemory;

global GameMemory* Mem;

InputCtx* Input() {
    return &Mem->input;
}

Arena* Arena::Perm() {
    return &Mem->perm;
}

Arena* Arena::Temp() {
    return &Mem->temp;
}

#define TWEAK(var, from, to) Tweak(&var, #var, from, to)
f32 Tweak(f32* val, cstr name, f32 from, f32 to) {
    cstr strippedName = StripName(name);

    Tweakable store = {.t = TweakableType::f32, .name = strippedName, .float32 = val};
    Mem->editor.tweakables.Push(store);
    return *val;
}

v2 Tweak(v2* val, cstr name, v2 from, v2 to) {
    cstr strippedName = StripName(name);

    Tweakable store = {.t = TweakableType::v2, .name = strippedName, .vec2 = val};
    Mem->editor.tweakables.Push(store);
    return *val;
}

v3 Tweak(v3* val, cstr name, v3 from, v3 to) {
    cstr strippedName = StripName(name);

    Tweakable store = {.t = TweakableType::v3, .name = strippedName, .vec3 = val};
    Mem->editor.tweakables.Push(store);
    return *val;
}

f32 Delta() {
    return Mem->input.delta;
}

u32 GamepadFromJoystick(u32 id) {
    for (auto i : Mem->input.padIDs) {
        if (id == i) return i;
    }

    ERR("Gamepad with joystick id %d not found", id);
    return 0;
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
                .randomSeed       = Rand::Init(),
                .doubleClickSpeed = 0.5f,
            },
        .data   = (Game::Data*)((u8*)(Mem) + sizeof(GameMemory)),
        .gfx    = {.clearColor = {0.4, 0, 0.6, 1}},
        .perm   = Arena(settings.permMemory),
        .temp   = Arena(settings.tempMemory),
        .window = WindowCtx::Init(settings),
    };

    ImguiInit(Mem->window);
    Mem->info.GetAndPrintGPUInfo();

    Game::Init(Mem->data);
}

void UpdateEvents() {
    for (auto& i : Mem->input.keys) {
        switch (i.state) {
        case InputState::None:
        case InputState::JustReleased: {
            i = {
                .state = InputState::Released,
                .time  = Delta(),
            };
            break;
        }

        case InputState::JustPressed: {
            i = {
                .state = InputState::Pressed,
                .time  = Delta(),
            };
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

    for (auto& i : Mem->input.buttons) {
        switch (i.state) {
        case InputState::None:
        case InputState::JustReleased: {
            i = {
                .state = InputState::Released,
                .time  = Delta(),
            };
            break;
        }

        case InputState::JustPressed: {
            i = {
                .state = InputState::Pressed,
                .time  = Delta(),
            };
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
                i = {
                    .state = InputState::Released,
                    .time  = Delta(),
                };
                break;
            }

            case InputState::JustPressed: {
                i = {
                    .state = InputState::Pressed,
                    .time  = Delta(),
                };
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

    SDL_Event event = {};
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
            for (auto& i : Mem->input.padIDs) {
                if (i == 0) {
                    i = event.gdevice.which;
                    goto added;
                }
            }

            WARN("Gamepad limit reached");
            break;

        added:
            INFO("Gamepad added");
            break;
        }

        case SDL_EVENT_GAMEPAD_REMOVED: {
            for (auto& i : Mem->input.padIDs) {
                if (i == event.gdevice.which) {
                    i = 0;
                    goto removed;
                }
            }

            WARN("Couldn't remove gamepad");
            break;

        removed:
            INFO("Gamepad removed");
            break;
        }

        case SDL_EVENT_KEY_DOWN: {
            auto cur = &Mem->input.keys[event.key.scancode].state;
            *cur     = *cur == InputState::JustPressed || *cur == InputState::Pressed
                           ? InputState::Pressed
                           : InputState::JustPressed;
            break;
        }

        case SDL_EVENT_KEY_UP: {
            Mem->input.keys[event.key.scancode].state = InputState::JustReleased;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            u32  id  = GamepadFromJoystick(event.gbutton.which);
            auto cur = &Mem->input.pads[id][event.gbutton.button].state;
            *cur     = *cur == InputState::JustPressed || *cur == InputState::Pressed
                           ? InputState::Pressed
                           : InputState::JustPressed;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            u32 id = GamepadFromJoystick(event.gbutton.which);
            Mem->input.pads[id][event.gbutton.button].state = InputState::JustReleased;
            break;
        }

        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            u32 id                                = GamepadFromJoystick(event.gaxis.which);
            Mem->input.axes[id][event.gaxis.axis] = event.gaxis.value;
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            Mem->input.mousePos   = v2{event.motion.x, event.motion.y};
            Mem->input.mouseDelta = v2{event.motion.xrel, event.motion.yrel};
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            auto cur = &Mem->input.buttons[event.button.button].state;
            *cur     = *cur == InputState::JustPressed || *cur == InputState::Pressed
                           ? InputState::Pressed
                           : InputState::JustPressed;
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            Mem->input.buttons[event.button.button].state = InputState::JustReleased;
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            Mem->input.wheel = v2{event.wheel.x, event.wheel.y};
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