#pragma once

#include "lib/containers.hpp"
#include "lib/os.hpp"

#include "core/opengl.hpp"
#include "core/window.hpp"

#include "game.hpp"

global AppInfo App = {.permMemorySize = MB(512), .tempMemorySize = MB(32)};

struct GameMemory {
    OS::SystemInfo  info;
    OS::Metrics     metrics;
    WindowCtx       window;
    GL::GraphicsCtx gfx;

    u32  randomSeed;
    bool quit;

    Arena       temp;
    Arena       perm;
    Game::Data* data;
};

global GameMemory* Mem;

EXPORT void Init() {
    auto info = Game::Setup();
    // SDL_SetMemoryFunctions(NULL, NULL, NULL, NULL);
    Mem = (GameMemory*)SDL_malloc(sizeof(GameMemory) + info.permMemory + info.tempMemory);

    Mem->info       = OS::SystemInfo::Init();
    Mem->metrics    = OS::Metrics::Init();
    Mem->randomSeed = Rand::Init();
    Mem->data       = (Game::Data*)((u8*)(Mem) + sizeof(GameMemory));
    Mem->gfx        = {.clearColor = {0.4, 0, 0.6, 1}};
    // Mem->perm = Arena(info.permMemory); // TODO
    // Mem->temp = Arena(info.tempMemory); // TODO
    Arena::Perm(info.permMemory);
    Arena::Temp(info.tempMemory);
    Mem->window = WindowCtx::Init({640, 480}, 4, 6);
    ImguiInit(Mem->window);

    Game::Init(Mem->data);
}

void HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        switch (event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            if (event.window.windowID == SDL_GetWindowID(Mem->window.window)) Mem->quit = true;
            break;
        }
        case SDL_EVENT_QUIT: {
            Mem->quit = true;
            break;
        }

        default: break;
        }
    }
}

EXPORT void Update() {
    HandleEvents();

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

    static bool show = true;
    if (show) {

        ImGui::Begin("Another Window", &show);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) show = false;
        ImGui::End();
    }

    ImGui::Render();

    GL::Begin();
    glViewport(0, 0, Mem->window.initialResolution.x, Mem->window.initialResolution.y);
    GL::Clear(Mem->gfx.clearColor);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(Mem->window.window);
    GL::End();
}

EXPORT void Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    Mem->window.Destroy();
    SDL_Quit();
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