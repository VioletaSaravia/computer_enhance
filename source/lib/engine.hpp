#pragma once

#include "core/opengl.hpp"
#include "core/window.hpp"

#include "lib/containers.hpp"
#include "lib/os.hpp"

#include "game.hpp"

struct Editor {
    StackArray<f32*, 64> tweakables;

    void Show() {
        for (auto i : tweakables) {
            ImGui::DragFloat("var", i);
        }
    }
};

struct GameMemory {
    OS::SystemInfo  info;
    OS::Metrics     metrics;
    WindowCtx       window;
    GL::GraphicsCtx gfx;

    u32    randomSeed;
    bool   quit;
    Editor editor;

    Arena       temp;
    Arena       perm;
    Game::Data* data;
};

global GameMemory* Mem;

f32 Tweak(f32* val, f32 from, f32 to) {
    Mem->editor.tweakables.Push(val);
    return *val;
}

Arena& Arena::Perm() {
    return Mem->perm;
}

Arena& Arena::Temp() {
    return Mem->temp;
}

void SDLError() {
    cstr err = SDL_GetError();
    ERR("%s", err);
}

void SDLFatal() {
    cstr err = SDL_GetError();
    FATAL("%s", err);
}

EXPORT void Init() {
    auto settings = Game::Setup();
    Mem = (GameMemory*)SDL_malloc(sizeof(GameMemory) + settings.permMemory + settings.tempMemory);
    if (!Mem) SDLFatal();

    Mem->info       = OS::SystemInfo::Init();
    Mem->metrics    = OS::Metrics::Init();
    Mem->randomSeed = Rand::Init();
    Mem->data       = (Game::Data*)((u8*)(Mem) + sizeof(GameMemory));
    Mem->gfx        = {.clearColor = {0.4, 0, 0.6, 1}};
    Mem->perm       = Arena(settings.permMemory);
    Mem->temp       = Arena(settings.tempMemory);
    Mem->window     = WindowCtx::Init(settings);
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

    if (SDL_GetWindowFlags(Mem->window.window) & SDL_WINDOW_MINIMIZED) {
        SDL_Delay(10);
        return;
    }
}

EXPORT void Update() {
    HandleEvents();

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

        // TODO
        Mem->editor.Show();

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