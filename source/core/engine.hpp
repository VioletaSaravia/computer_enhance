#pragma once

#include "core/game.hpp"
#include "core/input.hpp"
#include "core/opengl.hpp"
#include "core/os.hpp"
#include "core/window.hpp"

#include "lib/containers.hpp"

struct Memory {
    Arena       perm;
    Arena       temp;
    SystemInfo  info;
    Metrics     metrics;
    InputCtx    input;
    GraphicsCtx gfx;
    WindowCtx   window;
    Data*       data;
};

global Memory* Mem;

Metrics& Metrics::Get() {
    return Mem->metrics;
}

InputCtx& Input() {
    return Mem->input;
}

Arena& Perm() {
    return Mem->perm;
}

Arena& Temp() {
    return Mem->temp;
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

#ifndef PERM_ARENA_SIZE
#define PERM_ARENA_SIZE MB(512)
#endif

#ifndef TEMP_ARENA_SIZE
#define TEMP_ARENA_SIZE MB(32)
#endif

EXPORT void EngineInit() {
    auto  settings = Setup();
    Arena perm(PERM_ARENA_SIZE);
    Arena temp(TEMP_ARENA_SIZE);

    Mem = perm.Alloc<Memory>();
    if (!Mem) SDLFatal();

    *Mem = {
        .perm    = perm,
        .temp    = temp,
        .info    = SystemInfo::Init(),
        .metrics = Metrics::New(),
        .input =
            {
                .randomSeed       = Rand::Init(),
                .doubleClickSpeed = 0.5f,
            },
        .gfx    = {.clearColor = {0.4, 0, 0.6, 1}},
        .window = WindowCtx::Init(settings),
        .data   = (Data*)perm.Alloc(settings.memory),
    };

    ImguiInit(Mem->window);
    Mem->info.GetAndPrintGPUInfo();
    Init(Mem->data);
}

void UpdateEvents() {
    using enum InputState;

    for (auto& i : Mem->input.keys) {
        switch (i.state) {
        case None:
        case JustReleased: {
            i = {
                .state = Released,
                .time  = Delta(),
            };
            break;
        }

        case JustPressed: {
            i = {
                .state = Pressed,
                .time  = Delta(),
            };
            break;
        }

        case Pressed:
        case Released: {
            i.time += Delta();
            break;
        }
        default: break;
        }
    }

    for (auto& i : Mem->input.buttons) {
        switch (i.state) {
        case None:
        case JustReleased: {
            i = {
                .state = Released,
                .time  = Delta(),
            };
            break;
        }

        case JustPressed: {
            i = {
                .state = Pressed,
                .time  = Delta(),
            };
            break;
        }

        case Pressed:
        case Released: {
            i.time += Delta();
            break;
        }
        default: break;
        }
    }

    for (auto& pad : Mem->input.pads) {
        for (auto& i : pad) {
            switch (i.state) {
            case None:
            case JustReleased: {
                i = {
                    .state = Released,
                    .time  = Delta(),
                };
                break;
            }

            case JustPressed: {
                i = {
                    .state = Pressed,
                    .time  = Delta(),
                };
                break;
            }

            case Pressed:
            case Released: {
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
            *cur     = *cur == JustPressed || *cur == Pressed ? Pressed : JustPressed;
            break;
        }

        case SDL_EVENT_KEY_UP: {
            Mem->input.keys[event.key.scancode].state = JustReleased;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            u32  id  = GamepadFromJoystick(event.gbutton.which);
            auto cur = &Mem->input.pads[id][event.gbutton.button].state;
            *cur     = *cur == Down || *cur == Pressed ? Pressed : JustPressed;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            u32 id = GamepadFromJoystick(event.gbutton.which);
            Mem->input.pads[id][event.gbutton.button].state = JustReleased;
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
            *cur     = *cur == JustPressed || *cur == Pressed ? Pressed : JustPressed;
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            Mem->input.buttons[event.button.button].state = JustReleased;
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

    Update(Mem->data);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    static bool show_demo_window = true;
    if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

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
    return sizeof(Memory) + Mem->perm.cap + Mem->temp.cap;
}

EXPORT void HotReloaded(void* memory) {
    Mem = (Memory*)memory;
}

EXPORT bool ForceReload() {
    return false;
}

EXPORT bool ForceRestart() {
    return false;
}