#pragma once

#include "lib/containers.hpp"
#include "lib/os.hpp"

#include "core/opengl.hpp"

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

        f32 mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        result.window = SDL_CreateWindow("Game",
                                         (i32)(result.initialResolution.x * mainScale),
                                         (i32)(result.initialResolution.y * mainScale),
                                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        result.gl = SDL_GL_CreateContext(result.window);

        SDL_GL_MakeCurrent(result.window, result.gl);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        SDL_SetWindowPosition(result.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(result.window);

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
    }
};

void ImguiInit(const WindowCtx& w) {
    f32 mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    // IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(
        mainScale); // Bake a fixed style scale. (until we have a solution for dynamic style
                    // scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = mainScale;

    ImGui_ImplSDL3_InitForOpenGL(w.window, w.gl);
    ImGui_ImplOpenGL3_Init("#version 460");
}