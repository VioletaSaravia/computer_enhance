#pragma once

#include "core/opengl.h"
#include "core/window.h"

#include "lib/containers.h"
#include "lib/game.h"
#include "lib/input.h"
#include "lib/os.h"

typedef struct GameMemory {
    SystemInfo  info;
    Metrics     metrics;
    InputCtx    input;
    GameData*   data;
    GraphicsCtx gfx;
    Arena       perm;
    Arena       temp;
    WindowCtx   window;
} GameMemory;

global GameMemory* Mem;

InputCtx* Input() {
    return &Mem->input;
}

f32 Delta() {
    return Mem->input.delta;
}

#define LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

u32 GamepadFromJoystick(u32 id) {
    for (int i = 0; i < LEN(Mem->input.padIDs); i++) {
        if (id == i) return i;
    }

    ERR("Gamepad with joystick id %d not found", id);
    return 0;
}

EXPORT void EngineInit() {
    GameSettings settings = GameSetup();
    Mem = (GameMemory*)SDL_malloc(sizeof(GameMemory) + settings.permMemory + settings.tempMemory);
    if (!Mem) SDLFatal();

    *Mem = (GameMemory){
        .info    = NewSystemInfo(),
        .metrics = NewMetrics(),
        .input =
            {
                .randomSeed       = RandInit(1234),
                .doubleClickSpeed = 0.5f,
            },
        .data   = (GameData*)((u8*)(Mem) + sizeof(GameMemory)),
        .gfx    = {.clearColor = {0.4, 0, 0.6, 1}},
        .perm   = NewArena(settings.permMemory),
        .temp   = NewArena(settings.tempMemory),
        .window = NewWindow(settings),
    };

    GetAndPrintGPUInfo(&Mem->info);

    GameInit(Mem->data);
}

void UpdateEvents() {
    for (int i = 0; i < LEN(Mem->input.keys); i++) {
        InputInfo* key = &Mem->input.keys[i];
        switch (key->state) {
        case INPUTNONE:
        case JUSTRELEASED: {
            *key = (InputInfo){
                .state = RELEASED,
                .time  = Delta(),
            };
            break;
        }

        case JUSTPRESSED: {
            *key = (InputInfo){
                .state = PRESSED,
                .time  = Delta(),
            };
            break;
        }

        case PRESSED:
        case RELEASED: {
            key->time += Delta();
            break;
        }
        default: break;
        }
    }

    for (int i = 0; i < LEN(Mem->input.buttons); i++) {
        InputInfo* btn = &Mem->input.buttons[i];
        switch (btn->state) {
        case INPUTNONE:
        case JUSTRELEASED: {
            *btn = (InputInfo){
                .state = RELEASED,
                .time  = Delta(),
            };
            break;
        }

        case JUSTPRESSED: {
            *btn = (InputInfo){
                .state = PRESSED,
                .time  = Delta(),
            };
            break;
        }

        case PRESSED:
        case RELEASED: {
            btn->time += Delta();
            break;
        }
        default: break;
        }
    }

    for (int i = 0; i < LEN(Mem->input.pads); i++) {
        for (int j = 0; j < LEN(Mem->input.pads[i]); j++) {
            InputInfo* pad = &Mem->input.pads[i][j];
            switch (pad->state) {
            case INPUTNONE:
            case JUSTRELEASED: {
                *pad = (InputInfo){
                    .state = RELEASED,
                    .time  = Delta(),
                };
                break;
            }

            case JUSTPRESSED: {
                *pad = (InputInfo){
                    .state = PRESSED,
                    .time  = Delta(),
                };
                break;
            }

            case PRESSED:
            case RELEASED: {
                pad->time += Delta();
                break;
            }
            default: break;
            }
        }
    }

    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
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
            for (int i = 0; i < LEN(Mem->input.padIDs); i++) {
                u32* id = &Mem->input.padIDs[i];
                if (*id == 0) {
                    *id = event.gdevice.which;
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
            for (int i = 0; i < LEN(Mem->input.padIDs); i++) {
                u32* id = &Mem->input.padIDs[i];
                if (*id == event.gdevice.which) {
                    *id = 0;
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
            InputState* cur = &Mem->input.keys[event.key.scancode].state;
            *cur            = *cur == JUSTPRESSED || *cur == PRESSED ? PRESSED : JUSTPRESSED;
            break;
        }

        case SDL_EVENT_KEY_UP: {
            Mem->input.keys[event.key.scancode].state = JUSTRELEASED;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            u32         id  = GamepadFromJoystick(event.gbutton.which);
            InputState* cur = &Mem->input.pads[id][event.gbutton.button].state;
            *cur            = *cur == JUSTPRESSED || *cur == PRESSED ? PRESSED : JUSTPRESSED;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            u32 id = GamepadFromJoystick(event.gbutton.which);
            Mem->input.pads[id][event.gbutton.button].state = JUSTRELEASED;
            break;
        }

        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            u32 id                                = GamepadFromJoystick(event.gaxis.which);
            Mem->input.axes[id][event.gaxis.axis] = event.gaxis.value;
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            Mem->input.mousePos   = (v2){event.motion.x, event.motion.y};
            Mem->input.mouseDelta = (v2){event.motion.xrel, event.motion.yrel};
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            InputState* cur = &Mem->input.buttons[event.button.button].state;
            *cur     = *cur == JUSTPRESSED || *cur == PRESSED ? PRESSED : JUSTPRESSED;
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            Mem->input.buttons[event.button.button].state = JUSTRELEASED;
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            Mem->input.wheel = (v2){event.wheel.x, event.wheel.y};
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

    GameUpdate(Mem->data);

    GL_Begin();
    {
        glViewport(0, 0, Mem->window.initialResolution.x, Mem->window.initialResolution.y);
        GL_Clear(Mem->gfx.clearColor);
        SDL_GL_SwapWindow(Mem->window.window);
    }
    GL_End();

    // Uncapped timestep
    f64 timeCur         = (f64)(SDL_GetPerformanceCounter()) / (f64)(SDL_GetPerformanceFrequency());
    Mem->input.delta    = (f32)(timeCur - Mem->input.prevTime);
    Mem->input.prevTime = timeCur;
}

EXPORT void EngineShutdown() {
    FreeWindow(&Mem->window);
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