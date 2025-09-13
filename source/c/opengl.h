#pragma once

#include "lib/types.h"

#include "glad/glad.c"

void GL_Clear(v4 color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL_Begin() {
}

void GL_End() {
}

typedef struct DrawCmd {
    u32 shader, texture;
} DrawCmd;

typedef struct GraphicsCtx {
    v4      clearColor;
    DrawCmd drawQueue[64];
} GraphicsCtx;