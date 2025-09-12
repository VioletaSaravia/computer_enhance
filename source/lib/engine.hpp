#pragma once

#include "core/opengl.hpp"
#include "core/window.hpp"

#include "lib/containers.hpp"
#include "lib/game.hpp"
#include "lib/input.hpp"
#include "lib/os.hpp"

typedef struct Editor {
    bool                 show;
    StackArray<f32*, 32> tweakables;
    StackArray<v2*, 32>  views;

    void ShowAndUpdate();
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

InputState GetKey(Key key) {
    return Mem->input.keys[(u32)(key)].state;
}

InputState GetButton(Button button) {
    return Mem->input.buttons[(u32)(button)].state;
}

v2 GetMousePos() {
    return Mem->input.mousePos;
}

v2 GetMouseDelta() {
    return Mem->input.mouseDelta;
}

v2 GetWheel() {
    return Mem->input.wheel;
}

InputState GetPad(Pad pad, u32 controller = 0) {
    return Mem->input.pads[controller][(u32)(pad)].state;
}

i16 GetAxis(PadAxis axis, u32 controller = 0) {
    return Mem->input.axes[controller][(u32)(axis)];
}

void Editor::ShowAndUpdate() {
    if (GetKey(Key::F1) == InputState::JustPressed) Mem->editor.show ^= true;
    if (!show) return;

    ImGui::Begin("Debug", &show);

    for (auto i : tweakables) {
        ImGui::InputFloat("var", i);
    }

    for (auto i : views) {
        ImGui::Text("v2: %g, %g", i->x, i->y);
    }

    ImGui::End();
}

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

v2 View(v2* val, v2 from, v2 to) {
    Mem->editor.views.Push(val);
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

void UpdateEvents() {
    for (auto& i : Mem->input.keys) {
        switch (i.state) {
        case InputState::None:
        case InputState::JustReleased: {
            i.state = InputState::Released;
            i.time  = Delta();
            break;
        }

        case InputState::JustPressed: {
            i.state = InputState::Pressed;
            i.time  = Delta();
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
            i.state = InputState::Released;
            i.time  = Delta();
            break;
        }

        case InputState::JustPressed: {
            i.state = InputState::Pressed;
            i.time  = Delta();
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
                i.time  = Delta();
                break;
            }

            case InputState::JustPressed: {
                i.state = InputState::Pressed;
                i.time  = Delta();
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