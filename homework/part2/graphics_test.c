#include "graphics.c"

int main() {
    Rect rect = {.pos = {2, 3}, .size = {10, 15}};
    DrawRect(rect, .color = rgba(200, 120, 120, 1), .rounding = 0.25);

    Circle c = {.center = {5, 5}, .radius = 10};
    DrawCircle(c, .line = 0.1);
}