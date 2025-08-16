#include <stdint.h>
#include <Windows.h>
#include <stdio.h>

typedef struct Allocator Allocator;

typedef enum
{
    ALLOCATOR_NONE,
    ALLOCATOR_ARENA,
} AllocatorType;

struct Allocator
{
    AllocatorType type;
    uint8_t *data;
    uint64_t len, cap;
};

void *Alloc(Allocator *self, uint64_t count)
{
    void *result = 0;

    switch (self->type)
    {
    case ALLOCATOR_NONE:
        break;

    case ALLOCATOR_ARENA:
        if (self->len + count > self->cap)
        {
            printf("[WARNING] Arena full\n");
            abort();
        }

        result = self->data + self->len;
        self->len += count;
        break;

    default:
        break;
    }

    return result;
}

void Free(Allocator *self)
{
    switch (self->type)
    {
    case ALLOCATOR_NONE:
        return;

    case ALLOCATOR_ARENA:
        self->len = 0;
        return;

    default:
        break;
    }
}

Allocator NewArena(uint64_t size)
{
    return (Allocator){
        .type = ALLOCATOR_ARENA,
        .len = 0,
        .data = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE),
        .cap = size,
    };
}