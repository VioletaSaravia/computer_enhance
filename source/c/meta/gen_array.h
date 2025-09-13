#pragma once

#include "parser.h"

typedef int T;

typedef struct TArray {
    T*  data;
    u64 len, cap;
} TArray;

void TPush(TArray* array, T value) {
    if (array->len >= array->cap) {
        WARN("Array full");
        return;
    }

    array->data[array->len] = value;
    array->len += 1;
}

typedef struct Array {
    u8* data;
    u64 len, size, cap;
} Array;

void Push(Array* array, void* val) {
    if (array->len >= array->cap) {
        WARN("Array full");
        return;
    }

    memcpy(&array->data[array->len * array->size], val, array->size);
}

#define PUSH(t, array, val)

#define W(n, ...) fprintf(file, n "\n", ##__VA_ARGS__)

void WriteArray(FILE* file, const StructDef* s) {
    W("#include \"%s\"\n", s->file);
    W("typedef struct %sArray {", s->name);
    W("%s*  data;", s->name);
    W("u64 len, cap;");
    W("} %sArray;\n", s->name);
    W("void %sPush(%sArray* array, %s value) {", s->name, s->name, s->name);
    W("    if (array->len >= array->cap) {");
    W("        WARN(\"Array full\");");
    W("        return;");
    W("    }");
    W("    array->data[array->len] = value;");
    W("    array->len += 1;");
    W("}\n");
}

void WriteGenerics(FILE* file, StructDef structs[MAX_STRUCTS]) {
}