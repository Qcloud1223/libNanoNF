#ifndef NOMALLOC_INCLUDE_GLOBAL_H
#define NOMALLOC_INCLUDE_GLOBAL_H

#include "Heap.h"

void SetCurrentHeap(Heap *heap);

Raw GlobalMalloc(Size size);

Raw GlobalRealloc(Raw object, Size size);

Raw GlobalCalloc(Size count, Size size);

void GlobalFree(Raw object);

#endif