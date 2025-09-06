#pragma once

#include "lib/containers.hpp"
#include "lib/os.hpp"

#include "graphics/opengl.hpp"

#include "game.hpp"

struct WindowCtx {
    SDL_Window* window;
    v2          initialResolution;

    SDL_GLContext gl;
    i32           glMajorVersion;
    i32           glMinorVersion;

    static WindowCtx Init(v2 resolution, i32 maj, i32 min) {
        WindowCtx result = {
            .initialResolution = resolution, .glMajorVersion = maj, .glMinorVersion = min};
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, maj);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, min);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        result.window = SDL_CreateWindow("Game",
                                         (i32)result.initialResolution.x,
                                         (i32)result.initialResolution.y,
                                         SDL_WINDOW_OPENGL);

        result.gl = SDL_GL_CreateContext(result.window);

        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
            FATAL("Failed to initialize GLAD\n");
            SDL_GL_DestroyContext(result.gl);
            SDL_DestroyWindow(result.window);
            SDL_Quit();
            return {};
        }

        return result;
    }

    void Destroy() {
        SDL_GL_DestroyContext(gl);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

global AppInfo App = {.permMemorySize = MB(512), .tempMemorySize = MB(32)};

struct DrawCmd {
    u32 shader, texture;
};

struct GraphicsCtx {
    v4      clearColor;
    DrawCmd drawQueue[64];
};

struct GameMemory {
    OS::SystemInfo info;
    OSMetrics      metrics;
    WindowCtx      window;
    GraphicsCtx    gfx;

    u32  randomSeed;
    bool quit;

    Arena       temp;
    Arena       perm;
    Game::Data* data;
};

global GameMemory* Mem;

EXPORT void Init() {
    // SDL_SetMemoryFunctions(NULL, NULL, NULL, NULL);
    Mem = (GameMemory*)SDL_malloc(sizeof(GameMemory) + MB(256) + MB(32)); // TODO

    Mem->info       = OS::SystemInfo::Init();
    Mem->metrics    = OSMetrics::Init();
    Mem->randomSeed = Rand::Init();
    Mem->data       = (Game::Data*)((u8*)(Mem) + sizeof(GameMemory));
    Mem->gfx        = {.clearColor = {0.4, 0, 0.6, 1}};
    // Mem->game = Arena(MB(1)); // TODO
    // Mem->temp = Arena(MB(1)); // TODO
    Arena::Perm(App.permMemorySize);
    Arena::Temp(App.tempMemorySize);
    Mem->window = WindowCtx::Init({640, 480}, 4, 6);

    Game::Init(Mem->data);
}

void HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            Mem->quit = true;
        }
    }
}

EXPORT void Update() {
    HandleEvents();

    Game::Update(Mem->data);

    GL::Clear(Mem->gfx.clearColor);
    GL::Begin();
    SDL_GL_SwapWindow(Mem->window.window);
    GL::End();
}

EXPORT void Shutdown() {
    Mem->window.Destroy();
    SDL_free(Mem);
}

EXPORT bool ShouldClose() {
    return Mem->quit;
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