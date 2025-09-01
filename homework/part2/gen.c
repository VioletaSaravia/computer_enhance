#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdio.h>

#define ENABLE_PROFILER
#include "profiler.c"

// TODO: format-checking attribute
#define GEN(...)  fprintf(gen, __VA_ARGS__)
#define TO_STR(a) #a

typedef struct {
    const char* name;
    const char* file;
} ArrayTypeDef;

typedef enum { TD_F32, TD_I32, TD_V2, TD_V3, TD_V4, TD_COLOR, TD_U64, TD_BLOCK, AT_COUNT } ArrayTypes;

static ArrayTypeDef ArrayTypeDefs[AT_COUNT] = {
    [TD_F32]   = {.name = "f32", .file = "types.h"},
    [TD_I32]   = {.name = "i32", .file = "types.h"},
    [TD_V2]    = {.name = "v2", .file = "types.h"},
    [TD_V3]    = {.name = "v3", .file = "types.h"},
    [TD_V4]    = {.name = "v4", .file = "types.h"},
    [TD_COLOR] = {.name = "Color", .file = "types.h"},
    [TD_U64]   = {.name = "u64", .file = "types.h"},
    [TD_BLOCK] = {.name = "Block", .file = "profiler.h"},
};

typedef enum { V2, V3, V4, V_COUNT } VectorTypes;

typedef struct {
    const char* name;
    const char* type;
    int         dimensions;
} VectorDef;

static VectorDef VectorDefs[V_COUNT] = {
    [V2] = {.name = "v2", .type = "f32", .dimensions = 2},
    [V3] = {.name = "v3", .type = "f32", .dimensions = 3},
    [V4] = {.name = "v4", .type = "f32", .dimensions = 4},
};

static const char* DimNames[4] = {"x", "y", "z", "w"};

typedef enum {
    VO_ADD,
    VO_SUB,
    VO_DOT,
    VO_CROSS,
    VO_DIV,
    VO_MULT,
    VO_SCALAR_ADD,
    VO_SCALAR_SUB,
    VO_SCALAR_MULT,
    VO_SCALAR_DIV,
    VO_DIST,
    VO_COUNT
} VectorOp;

typedef struct {
    const char* name;
    const char* ret;
    const char* params;
    const char* code;
    bool        mut;
} OpDef;

typedef enum {
    // AO_NEW,
    AO_PUSH,
    AO_POP,
    AO_LAST,
    AO_LAST_MUT,
    AO_GET,
    AO_GET_MUT,
    AO_COUNT
} ArrayOps;

static OpDef ArrayOpDefs[AO_COUNT] = {
    // [AO_NEW] = {.name = "New", .params = "u64 size", .code = "return (@T@Array){.data =
    // malloc(sizeof(@T@) * cap), .len = 0, .cap = cap};"},
    [AO_PUSH]     = {.name   = "Push",
                     .ret    = NULL,
                     .params = "%s val",
                     .code   = "\tif (array->len >= array->cap) return;\n\tarray->data[array->len++] = val;",
                     .mut    = true},
    [AO_POP]      = {.name   = "Pop",
                     .ret    = "%s",
                     .params = NULL,
                     .code   = "\tassert(array->len > 0);\n\treturn array->data[--array->len];",
                     .mut    = true},
    [AO_LAST]     = {.name   = "Last",
                     .ret    = "%s",
                     .params = NULL,
                     .code   = "\tassert(array.len > 0);\n\treturn array.data[array.len - 1];"},
    [AO_LAST_MUT] = {.name   = "LastMut",
                     .ret    = "%s*",
                     .params = NULL,
                     .code   = "\tassert(array.len > 0);\n\treturn &array.data[array.len - 1];"},
    [AO_GET]      = {.name   = "Get",
                     .ret    = "%s",
                     .params = "u64 id",
                     .code   = "\tassert(array.len < id);\n\treturn array.data[id];"},
    [AO_GET_MUT]  = {.name   = "GetMut",
                     .ret    = "%s*",
                     .params = "u64 id",
                     .code   = "\tassert(array.len < id);\n\treturn &array.data[id];"},
};

void GenArray(ArrayTypeDef td, FILE* gen) {
    PROFILE_FUNCTION();
    GEN("#include \"%s\"\n\n", td.file);

    GEN("typedef struct {\n");
    GEN("\t%s *data;\n", td.name);
    GEN("\tu64 len, cap;\n");
    GEN("} %sArray;\n\n", td.name);

    GEN("%sArray %sArrayNew(u64 cap) {\n", td.name, td.name);
    GEN("\treturn (%sArray){\n", td.name);
    GEN("\t\t.data = malloc(sizeof(%s) * cap),\n", td.name);
    GEN("\t\t.len = 0,\n");
    GEN("\t\t.cap = cap\n");
    GEN("\t};\n");
    GEN("}\n\n");

    for (size_t i = 0; i < AO_COUNT; i++) {
        PROFILE_SCOPE("GenArrayOp");
        OpDef op = ArrayOpDefs[i];

        if (op.ret) {
            GEN(op.ret, td.name);
        } else {
            GEN("void");
        }

        GEN(" %sArray%s(%sArray%s array", td.name, op.name, td.name, op.mut ? "*" : "");
        if (op.params) {
            GEN(", ");
            GEN(op.params, td.name);
        }
        GEN(") {\n");

        GEN("%s", op.code);
        GEN("\n}\n\n");
    }
}

void GenArrayOverloads(FILE* gen) {
    PROFILE_FUNCTION();
    GEN("void StubArrayOp(void* array, ...) {\n");
    GEN("\tprintf(\"[ERROR] Array operator called on non-array type\\n\");\n");
    GEN("}\n\n");

    GEN("#define ArrayNew(type, cap) type##ArrayNew(cap)\n\n");

    GEN("#define Push(array, new) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayPush, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(&array, new)\n\n");

    GEN("#define Pop(array) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayPop, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(&array)\n\n");

    GEN("#define Last(array) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayLast, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(array)\n\n");

    GEN("#define LastMut(array) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayLastMut, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(array)\n\n");

    GEN("#define Get(array, id) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayGet, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(array, id)\n\n");

    GEN("#define GetMut(array, id) _Generic((array), \\\n");
    for (size_t i = 0; i < AT_COUNT; i++) {
        ArrayTypeDef td = ArrayTypeDefs[i];
        GEN("\t%sArray: %sArrayGetMut, \\\n", td.name, td.name);
    }
    GEN("\tdefault: StubArrayOp)(array, id)\n\n");
}

typedef enum { BT_u8, BT_u16, BT_u32, BT_u64, BT_i8, BT_i16, BT_i32, BT_i64, BT_f32, BT_f64, BT_COUNT } BaseTypes;

static ArrayTypeDef BTypedefs[BT_COUNT] = {
    [BT_u8]  = {.name = "u8", .file = "types.h"},
    [BT_u16] = {.name = "u16", .file = "types.h"},
    [BT_u32] = {.name = "u32", .file = "types.h"},
    [BT_u64] = {.name = "u64", .file = "types.h"},
    [BT_i8]  = {.name = "i8", .file = "types.h"},
    [BT_i16] = {.name = "i16", .file = "types.h"},
    [BT_i32] = {.name = "i32", .file = "types.h"},
    [BT_i64] = {.name = "i64", .file = "types.h"},
    [BT_f32] = {.name = "f32", .file = "types.h"},
    [BT_f64] = {.name = "f64", .file = "types.h"},
};

void GenSortFunction(ArrayTypeDef td, FILE* gen) {
    PROFILE_FUNCTION();
    GEN("#include \"%s\"\n", td.file);
    GEN("inline i32 %sSort(const void* from, const void* to) {\n", td.name);
    GEN("\treturn ((%s*)(from)) - ((%s*)(to));\n", td.name, td.name);
    GEN("}\n\n");
}

void GenHeader(FILE* gen) {
    GEN("/*\n");
    GEN("\tAUTOGENERATED CODE\n");
    GEN("\tDO NOT EDIT\n");
    GEN("*/\n\n");
}

int main() {
    PROFILER_NEW();
    FILE* gen = NULL;

    PROFILE("Creating file", gen = fopen("containers.gen.c", "w"); if (gen == NULL) return -1;);
    GenHeader(gen);

    for (size_t i = 0; i < AT_COUNT; i++) {
        GenArray(ArrayTypeDefs[i], gen);
    }
    GenArrayOverloads(gen);

    PROFILE("Closing file", fclose(gen););

    PROFILE("Creating file", gen = fopen("sort.gen.h", "w"); if (gen == NULL) return -1;);

    GenHeader(gen);
    for (size_t i = 0; i < BT_COUNT; i++) {
        GenSortFunction(BTypedefs[i], gen);
    }
    PROFILE("Closing file", fclose(gen););

    PROFILER_END();
    return 0;
}