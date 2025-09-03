#pragma once

#include "types.hpp"

struct Color {
    u8 r, g, b, a;

    static StackArray<Color, 32> Palette;
    constexpr void SetPalette(cstr path){
    }

    constexpr operator v4() const {
        return v4{static_cast<f32>(r) / 255.0f,
                  static_cast<f32>(g) / 255.0f,
                  static_cast<f32>(b) / 255.0f,
                  static_cast<f32>(a) / 255.0f};
    }
};

// NOTE(violeta): This is to provide color palette support on vscode :O
constexpr Color rgba(u8 r, u8 g, u8 b, f32 a) {
    return Color{r, g, b, u8(a * 255.0f)};
}

static constexpr Color Black   = rgba(0, 0, 0, 1);
static constexpr Color White   = rgba(255, 255, 255, 1);
static constexpr Color Violeta = rgba(154, 60, 146, 1);

enum Shape2DPivot {
    SP_TOP_LEFT,
    SP_TOP_RIGHT,
    SP_TOP_CENTER,
    SP_BOT_LEFT,
    SP_BOT_RIGHT,
    SP_BOT_CENTER,
    SP_CENTER_LEFT,
    SP_CENTER_RIGHT,
    SP_CENTER,

    SP_COUNT,
};

struct Point2D : v2 {
    void Draw(f32 radius, Color color = Black, f32 line = {}) {
        printf("TODO\n");
        return;
    }
};

using Point = Point2D;

struct Point3D : v3 {
    void Draw(f32 radius, Color color = Black, f32 line = {}) {
        printf("TODO\n");
        return;
    }
};

struct Circle;

union Rect {
    struct {
        v2 pos, size;
    };
    struct {
        f32 x, y, w, h;
    };

    bool Intersects(Rect b) { return !(x + w < b.x || x > b.x + b.w || y + h < b.y || y > b.y + b.h); }
    bool Intersects(Circle b);

    void DrawRect(Color color = {}, Shape2DPivot pivot = {}, f32 rounding = {}, f32 rot = {}, f32 line = {}) {
        printf("TODO\n");
        return;
    }
    void DrawLine(Color color = {}, f32 thickness = {}) {
        printf("TODO\n");
        return;
    }
};

struct Circle {
    v2  center;
    f32 radius;

    bool Intersects(Rect b);

    bool Intersects(Circle b) {
        f32 dx = center.x - b.center.x;
        f32 dy = center.y - b.center.y;
        f32 r  = radius + b.radius;
        return (dx * dx + dy * dy) <= r * r;
    }

    void Draw(Color color = Black, f32 line = {}) {
        printf("TODO\n");
        return;
    }
};

bool Rect::Intersects(Circle b) {
    f32 closestX = fmaxf(x, fminf(b.center.x, x + w));
    f32 closestY = fmaxf(y, fminf(b.center.y, y + h));
    f32 dx       = b.center.x - closestX;
    f32 dy       = b.center.y - closestY;
    return (dx * dx + dy * dy) <= b.radius * b.radius;
}

bool Circle::Intersects(Rect b) {
    return b.Intersects(*this);
}

struct Arc {
    v2  center;
    Rad angle;
};

struct Poly {
    v2* points;
    f32 count;
};

enum Shape2DType {
    SHAPE2D_NONE,
    SHAPE2D_POINT,
    SHAPE2D_LINE,
    SHAPE2D_RECT,
    SHAPE2D_CIRCLE,
    SHAPE2D_POLY,
};

struct Shape2D {
    Shape2DType type;

    union {
        Point  point;
        Rect   line;
        Rect   rect;
        Circle circle;
        Poly   poly;
    };

    void Draw(Color color) {
        switch (type) {
        case SHAPE2D_NONE: return;
        case SHAPE2D_POINT: point.Draw(10, color = color); break;
        case SHAPE2D_LINE: line.DrawLine(color = color); break;
        case SHAPE2D_RECT: rect.DrawRect(color = color); break;
        case SHAPE2D_CIRCLE: circle.Draw(color = color); break;
        case SHAPE2D_POLY: {
            for (size_t i = 0; i < poly.count - 1; i++) {
                Rect{poly.points[i], poly.points[i + 1]}.DrawLine(color = color);
            }
            break;
        }
        }
    }
};

bool Intersects2D(Shape2D a, Shape2D b) {
    switch (a.type) {
    case SHAPE2D_NONE: return false;
    case SHAPE2D_POINT: return false;
    case SHAPE2D_LINE: return false;

    case SHAPE2D_RECT:
        switch (b.type) {
        case SHAPE2D_NONE: return false;
        case SHAPE2D_POINT: return false;
        case SHAPE2D_LINE: return false;
        case SHAPE2D_RECT: return a.rect.Intersects(b.rect);
        case SHAPE2D_CIRCLE: return a.rect.Intersects(b.circle);
        case SHAPE2D_POLY: return false;
        }

    case SHAPE2D_CIRCLE:
        switch (b.type) {
        case SHAPE2D_NONE: return false;
        case SHAPE2D_POINT: return false;
        case SHAPE2D_LINE: return false;
        case SHAPE2D_RECT: return a.circle.Intersects(b.rect);
        case SHAPE2D_CIRCLE: return a.circle.Intersects(b.circle);
        case SHAPE2D_POLY: return false;
        }

    case SHAPE2D_POLY: return false;
    }

    return false;
}

struct Box {
    v3 pos, size;
};

struct Sphere {
    v3  pos;
    f32 radius;
};

struct Plane {
    v3 pos, normal;
};

struct Capsule {
    v3  pos;
    f32 radius, halfHeight;
};

struct Mesh {
    Handle<v3>  verts;
    Handle<f32> ids;
    f32         count;
};

enum Shape3DType {
    SHAPE3D_NONE,
    SHAPE3D_POINT,
    SHAPE3D_CUBE,
    SHAPE3D_SPHERE,
    SHAPE3D_PLANE,
    SHAPE3D_CAPSULE,
    SHAPE3D_MESH
};

struct Shape3D {
    Shape3DType type;

    union {
        v3      point;
        Box     cube;
        Sphere  sphere;
        Plane   plane;
        Capsule capsule;
        Mesh    mesh;
    };
};

inline bool IntersectsCubeCube(Box a, Box b) {
    return !(a.pos.x + a.size.x < b.pos.x || a.pos.x > b.pos.x + b.size.x || a.pos.y + a.size.y < b.pos.y ||
             a.pos.y > b.pos.y + b.size.y || a.pos.z + a.size.z < b.pos.z || a.pos.z > b.pos.z + b.size.z);
}

inline bool IntersectsCubeSphere(Box a, Sphere b) {
    v3 to      = a.pos + a.size;
    v3 closest = {
        fmaxf(a.pos.x, fminf(b.pos.x, to.x)),
        fmaxf(a.pos.y, fminf(b.pos.y, to.y)),
        fmaxf(a.pos.z, fminf(b.pos.z, to.z)),
    };
    v3  d  = b.pos - closest;
    f32 r2 = b.radius * b.radius;
    return d.Len() <= r2;
}

inline bool IntersectsSphereCube(Sphere a, Box b) {
    return IntersectsCubeSphere(b, a);
}

inline bool IntersectsSphereSphere(Sphere a, Sphere b) {
    v3  d = a.pos - b.pos;
    f32 r = a.radius + b.radius;
    return d.LenSq() <= r * r; // Sq?
}

inline bool IntersectsPlaneCube(Plane a, Box b) {
    v3 corners[8];
    for (int i = 0; i < 8; i++) {
        corners[i].x = b.pos.x + ((i & 1) ? b.size.x : 0);
        corners[i].y = b.pos.y + ((i & 2) ? b.size.y : 0);
        corners[i].z = b.pos.z + ((i & 4) ? b.size.z : 0);
    }
    bool pos = false, neg = false;
    for (int i = 0; i < 8; i++) {
        f32 d = (corners[i].x - a.pos.x) * a.normal.x + (corners[i].y - a.pos.y) * a.normal.y +
                (corners[i].z - a.pos.z) * a.normal.z;
        if (d >= 0) pos = true;
        if (d <= 0) neg = true;
    }
    return pos && neg;
}

inline bool IntersectsPlaneSphere(Plane a, Sphere b) {
    f32 d = (b.pos.x - a.pos.x) * a.normal.x + (b.pos.y - a.pos.y) * a.normal.y + (b.pos.z - a.pos.z) * a.normal.z;
    return fabs(d) <= b.radius;
}

inline bool IntersectsSpherePlane(Sphere a, Plane b) {
    return IntersectsPlaneSphere(b, a);
}

inline bool IntersectsCubePlane(Box a, Plane b) {
    return IntersectsPlaneCube(b, a);
}

inline bool IntersectsPlanePlane(Plane a, Plane b) {
    v3 cross = a.normal.Cross(b.normal);
    return !(IsZero(cross.x) && IsZero(cross.y) && IsZero(cross.z));
}

// helper: squared distance between point and AABB (cube)
inline f32 pointAABBDistSq(v3 p, Box c) {
    f32 dx = fmax(fmax(c.pos.x - p.x, 0), p.x - (c.pos.x + c.size.x));
    f32 dy = fmax(fmax(c.pos.y - p.y, 0), p.y - (c.pos.y + c.size.y));
    f32 dz = fmax(fmax(c.pos.z - p.z, 0), p.z - (c.pos.z + c.size.z));
    return dx * dx + dy * dy + dz * dz;
}

// helper: squared distance between point and vertical capsule
inline f32 pointCapsuleDistSq(v3 p, Capsule c) {
    f32 y  = fmax(-c.halfHeight, fmin(p.y - c.pos.y, c.halfHeight)); // clamp y to segment
    f32 dx = p.x - c.pos.x;
    f32 dy = (p.y - c.pos.y - y);
    f32 dz = p.z - c.pos.z;
    return dx * dx + dy * dy + dz * dz;
}

inline bool IntersectsCubeCapsule(Box a, Capsule b) {
    return pointAABBDistSq(b.pos, a) <= b.radius * b.radius;
}
inline bool IntersectsSphereCapsule(Sphere a, Capsule b) {
    f32 distSq = pointCapsuleDistSq(a.pos, b);
    f32 r      = a.radius + b.radius;
    return distSq <= r * r;
}
inline bool IntersectsPlaneCapsule(Plane a, Capsule b) {
    v3  top    = {b.pos.x, b.pos.y + b.halfHeight, b.pos.z};
    v3  bottom = {b.pos.x, b.pos.y - b.halfHeight, b.pos.z};
    f32 dTop   = (top.x - a.pos.x) * a.normal.x + (top.y - a.pos.y) * a.normal.y + (top.z - a.pos.z) * a.normal.z;
    f32 dBottom =
        (bottom.x - a.pos.x) * a.normal.x + (bottom.y - a.pos.y) * a.normal.y + (bottom.z - a.pos.z) * a.normal.z;
    return !(dTop > b.radius && dBottom > b.radius) && !(dTop < -b.radius && dBottom < -b.radius);
}
inline bool IntersectsCapsuleCapsule(Capsule a, Capsule b) {
    // check horizontal distance (x,z) and vertical overlap (y)
    f32 dx = a.pos.x - b.pos.x;
    f32 dz = a.pos.z - b.pos.z;
    f32 r  = a.radius + b.radius;
    if (dx * dx + dz * dz > r * r) return false;
    f32 aMinY = a.pos.y - a.halfHeight, aMaxY = a.pos.y + a.halfHeight;
    f32 bMinY = b.pos.y - b.halfHeight, bMaxY = b.pos.y + b.halfHeight;
    return aMaxY >= bMinY && bMaxY >= aMinY;
}
inline bool IntersectsCapsuleCube(Capsule b, Box a) {
    return IntersectsCubeCapsule(a, b);
}
inline bool IntersectsCapsuleSphere(Capsule b, Sphere a) {
    return IntersectsSphereCapsule(a, b);
}
inline bool IntersectsCapsulePlane(Capsule b, Plane a) {
    return IntersectsPlaneCapsule(a, b);
}

bool Intersects3D(Shape3D a, Shape3D b) {
    switch (a.type) {
    case SHAPE3D_NONE: return false;
    case SHAPE3D_POINT: return false;

    case SHAPE3D_CUBE:
        switch (b.type) {
        case SHAPE3D_NONE: return false;
        case SHAPE3D_POINT: return false;
        case SHAPE3D_CUBE: return IntersectsCubeCube(a.cube, b.cube);
        case SHAPE3D_SPHERE: return IntersectsCubeSphere(a.cube, b.sphere);
        case SHAPE3D_PLANE: return IntersectsCubePlane(a.cube, b.plane);
        case SHAPE3D_CAPSULE: return IntersectsCubeCapsule(a.cube, b.capsule);
        case SHAPE3D_MESH: return false;
        }

    case SHAPE3D_SPHERE:
        switch (b.type) {
        case SHAPE3D_NONE: return false;
        case SHAPE3D_POINT: return false;
        case SHAPE3D_CUBE: return IntersectsSphereCube(a.sphere, b.cube);
        case SHAPE3D_SPHERE: return IntersectsSphereSphere(a.sphere, b.sphere);
        case SHAPE3D_PLANE: return IntersectsSpherePlane(a.sphere, b.plane);
        case SHAPE3D_CAPSULE: return IntersectsSphereCapsule(a.sphere, b.capsule);
        case SHAPE3D_MESH: return false;
        }

    case SHAPE3D_PLANE:
        switch (b.type) {
        case SHAPE3D_NONE: return false;
        case SHAPE3D_POINT: return false;
        case SHAPE3D_CUBE: return IntersectsPlaneCube(a.plane, b.cube);
        case SHAPE3D_SPHERE: return IntersectsPlaneSphere(a.plane, b.sphere);
        case SHAPE3D_PLANE: return IntersectsPlanePlane(a.plane, b.plane);
        case SHAPE3D_CAPSULE: return IntersectsPlaneCapsule(a.plane, b.capsule);
        case SHAPE3D_MESH: return false;
        }

    case SHAPE3D_CAPSULE:
        switch (b.type) {
        case SHAPE3D_NONE: return false;
        case SHAPE3D_POINT: return false;
        case SHAPE3D_CUBE: return IntersectsCapsuleCube(a.capsule, b.cube);
        case SHAPE3D_SPHERE: return IntersectsCapsuleSphere(a.capsule, b.sphere);
        case SHAPE3D_PLANE: return IntersectsCapsulePlane(a.capsule, b.plane);
        case SHAPE3D_CAPSULE: return IntersectsCapsuleCapsule(a.capsule, b.capsule);
        case SHAPE3D_MESH: return false;
        }

    case SHAPE3D_MESH: return false;
    }

    return false;
}

enum Shape3DPivot {
    SP3_TOP_LEFT_CENTER,
    SP3_TOP_CENTER_CENTER,
    SP3_TOP_RIGHT_CENTER,
    SP3_CENTER_LEFT_CENTER,
    SP3_CENTER_CENTER,
    SP3_CENTER_RIGHT_CENTER,
    SP3_BOT_LEFT_CENTER,
    SP3_BOT_CENTER_CENTER,
    SP3_BOT_RIGHT_CENTER,

    SP3_TOP_LEFT_FRONT,
    SP3_TOP_CENTER_FRONT,
    SP3_TOP_RIGHT_FRONT,
    SP3_CENTER_LEFT_FRONT,
    SP3_CENTER_FRONT_FRONT,
    SP3_CENTER_RIGHT_FRONT,
    SP3_BOT_LEFT_FRONT,
    SP3_BOT_CENTER_FRONT,
    SP3_BOT_RIGHT_FRONT,

    SP3_TOP_LEFT_BACK,
    SP3_TOP_CENTER_BACK,
    SP3_TOP_RIGHT_BACK,
    SP3_CENTER_LEFT_BACK,
    SP3_CENTER_BACK,
    SP3_CENTER_RIGHT_BACK,
    SP3_BOT_LEFT_BACK,
    SP3_BOT_CENTER_BACK,
    SP3_BOT_RIGHT_BACK,

    SP3_COUNT
};

void DrawCube(Box c, Color color = Black, Shape3DPivot pivot = {}, f32 thickness = {}, f32 rot = {}, f32 line = {}) {
    printf("TODO\n");
    return;
}