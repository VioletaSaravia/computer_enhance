#pragma once

#include "lib/types.h"

#include ".\parser.h"

typedef struct FieldArray {
Field*  data;
u64 len, cap;
} FieldArray;

void FieldPush(FieldArray* array, Field value) {
    if (array->len >= array->cap) {
        WARN("Array full");
        return;
    }
    array->data[array->len] = value;
    array->len += 1;
}

#include ".\parser.h"

typedef struct StructDefArray {
StructDef*  data;
u64 len, cap;
} StructDefArray;

void StructDefPush(StructDefArray* array, StructDef value) {
    if (array->len >= array->cap) {
        WARN("Array full");
        return;
    }
    array->data[array->len] = value;
    array->len += 1;
}

