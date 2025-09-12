#pragma once

#include "lib/types.hpp"

#include "glad/glad.c"

#include "imgui-1.92.2b/imgui.h"

#include "imgui-1.92.2b/backends/imgui_impl_sdl3.h"

#include "imgui-1.92.2b/backends/imgui_impl_opengl3.h"

namespace GL {

void Clear(v4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Begin() {
}

void End() {
}

typedef struct DrawCmd {
    u32 shader, texture;
} DrawCmd;

typedef struct GraphicsCtx {
    v4      clearColor;
    DrawCmd drawQueue[64];
} GraphicsCtx;

} // namespace GL