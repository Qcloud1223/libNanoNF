//

#include "Heap.h"
#include "HeapImpl.h"
#include <assert.h>
#include <string.h>

Heap *CreateHeap(Raw raw, Size userSize) {
    assert(userSize > sizeof(Heap) + sizeof(Chuck) * 3);
    Heap *heap = (Heap *) raw;
    for (int i = 0; i < 64; i += 1) {
        heap->exact[i] = heap->sorted[i] = NULL;
    }
    Chuck *firstChuck =
            (Chuck *) ((Bytes) raw + sizeof(Heap) - sizeof(uint32_t));
    firstChuck->lowerUsed = 1;
    Size initialSize =
            (userSize - sizeof(Heap) - sizeof(Chuck) - sizeof(uint32_t))
                    >> 3u << 3u;
    SetSize(firstChuck, initialSize);
    SetUsed(firstChuck, 0);
    SetPrev(firstChuck, NULL);
    SetNext(firstChuck, NULL);
    Chuck *endingOverhead = GetHigher(firstChuck);
    SetSize(endingOverhead, sizeof(Chuck));
    SetUsed(endingOverhead, 1);
    SetPrev(endingOverhead, endingOverhead);
    SetNext(endingOverhead, endingOverhead);
    heap->head = heap->last = NULL;
    MoveIn(heap, firstChuck);
    return heap;
}

static void AllocateInChuck(Heap *heap, Chuck *chuck, Size size) {
    MoveOut(heap, chuck);
    Chuck *remain = SplitIfWorthy(chuck, size);
    if (remain) {
        if (!GetUsed(GetHigher(remain))) {
            MoveOut(heap, GetHigher(remain));
            SetHigher(remain, GetHigher(GetHigher(remain)));
        }
        MoveIn(heap, remain);
    }
}

Raw AllocateObject(Heap *heap, Size userSize) {
    Size size = (userSize + OVERHEAD_OF_USED_CHUCK + 0x07) >> 3u << 3u;
    Chuck *selected = FindSmallestFitInHeap(heap, size);
    if (!selected) {
        return NULL;
    }

    assert(!GetUsed(selected));
    AllocateInChuck(heap, selected, size);
    SetUsed(selected, 1);
    return ToObject(selected);
}

void ReleaseObject(Heap *heap, Raw object) {
    Chuck *chuck = ToChuck(object);
    SetUsed(chuck, 0);
    SetHigher(chuck, GetHigher(chuck));
    if (!chuck->lowerUsed) {
        MoveOut(heap, GetLower(chuck));
        SetHigher(GetLower(chuck), GetHigher(chuck));
        chuck = GetLower(chuck);
        assert(chuck->lowerUsed);
    }
    if (!GetUsed(GetHigher(chuck))) {
        MoveOut(heap, GetHigher(chuck));
        SetHigher(chuck, GetHigher(GetHigher(chuck)));
        assert(GetUsed(GetHigher(chuck)));
    }
    MoveIn(heap, chuck);
}

Raw ResizeObject(Heap *heap, Raw object, Size userSize) {
    Chuck *chuck = ToChuck(object);
    Size size = (userSize + OVERHEAD_OF_USED_CHUCK + 0x07) >> 3u << 3u;
    if (size <= GetSize(chuck)) {
        return object;
    }
    if (!GetUsed(GetHigher(chuck)) &&
        size <= GetSize(chuck) + GetSize(GetHigher(chuck))) {
        AllocateInChuck(heap, GetHigher(chuck), size - GetSize(chuck));
        SetHigher(chuck, GetHigher(GetHigher(chuck)));
        return object;
    }
    Raw copied = AllocateObject(heap, userSize);
    if (!copied) {
        return NULL;
    }
    memcpy(copied, object, GetSize(chuck) - OVERHEAD_OF_USED_CHUCK);
    ReleaseObject(heap, object);
    return copied;
}