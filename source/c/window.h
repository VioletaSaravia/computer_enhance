#pragma once

#include "core/opengl.h"

#include "lib/containers.h"
#include "lib/game.h"
#include "lib/os.h"

typedef struct WindowCtx {
    SDL_Window* window;
    v2          initialResolution;

    SDL_GLContext gl;
    i32           glMajorVersion;
    i32           glMinorVersion;

} WindowCtx;

void FreeWindow(WindowCtx* ctx) {
    SDL_GL_DestroyContext(ctx->gl);
    SDL_DestroyWindow(ctx->window);
}

WindowCtx NewWindow(GameSettings settings) {
    WindowCtx result = {.initialResolution = settings.resolution,
                        .glMajorVersion    = (i32)(settings.glVersion.x),
                        .glMinorVersion    = (i32)(settings.glVersion.y)};
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, result.glMajorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, result.glMinorVersion);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    f32 mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    result.window = SDL_CreateWindow(settings.name,
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
        FreeWindow(&result);
        SDL_Quit();
        return (WindowCtx){0};
    }

    return result;
}
