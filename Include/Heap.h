#ifndef NOMALLOC_INCLUDE_HEAP_H
#define NOMALLOC_INCLUDE_HEAP_H

#include <stddef.h>
#include <string.h>

typedef void *Raw;
typedef size_t Size;
typedef struct HeapLayout Heap;

Heap *CreateHeap(Raw raw, Size Size);

Raw AllocateObject(Heap *heap, Size Size);

Raw ResizeObject(Heap *heap, Raw object, Size size);

void ReleaseObject(Heap *heap, Raw object);

static Raw AllocateZeroedObjectList(Heap *heap, Size count, Size size)
{
    Raw allocated = AllocateObject(heap, count * size);
    if (allocated)
    {
        memset(allocated, 0x00, count * size);
    }
    return allocated;
}

#endif