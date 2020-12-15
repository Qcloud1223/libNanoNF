#include "Heap.h"
#include "HeapImpl.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// low level chuck operations test
void ChuckOp()
{
    Raw *plane = malloc(sizeof(uint8_t) * 128);
    Chuck *ck1 = (Chuck *)((Bytes)plane - sizeof(uint32_t)),
          *ck2 = (Chuck *)((Bytes)plane + 40 - sizeof(uint32_t)),
          *ck3 = (Chuck *)((Bytes)plane + 72 - sizeof(uint32_t)),
          *ck3_post = (Chuck *)((Bytes)plane + 104 - sizeof(uint32_t));

    SetLower(ck2, ck1);
    assert(GetLower(ck2) == ck1);
    assert(GetHigher(ck1) == ck2);
    SetHigher(ck2, ck3);
    assert(GetHigher(ck2) == ck3);
    assert(GetLower(ck3) == ck2);
    SetHigher(ck3, ck3_post);

    ck1->lowerUsed = 1;
    SetUsed(ck1, 0);
    SetUsed(ck2, 0);
    SetUsed(ck3, 0);

    assert(GetSize(ck1) - GetSize(ck2) == 8);
    assert(GetSize(ck3) == GetSize(ck2));

    SetPrev(ck2, NULL);
    assert(ck2->prev == NULL);
    SetNext(ck2, ck1);
    assert(ck2->next == ck1);
    assert(ck1->prev == ck2);
    SetNext(ck1, NULL);
    Insert(ck2, ck3);
    assert(ck2->next == ck3);
    assert(ck3->next == ck1);
    assert(FindSmallestFit(ck2, 0) == ck2);
    assert(FindSmallestFit(ck2, GetSize(ck2)) == ck2);
    assert(FindSmallestFit(ck2, GetSize(ck1)) == ck1);

    free(plane);
}

void MatchSlot()
{
    uint8_t prev = 0;
    assert(MatchExact(sizeof(Chuck)) == 0);
    for (Size size = sizeof(Chuck); size < NOT_EXACT_MIN; size += 8)
    {
        uint8_t slot = MatchExact(size);
        assert(slot + 1 > prev);
        prev = slot + 1;
    }
    assert(prev == 63 + 1);

    prev = 0;
    assert(MatchSorted(NOT_EXACT_MIN) == 0);
    for (Size size = NOT_EXACT_MIN; size < 2ul << 30u; size += 8)
    {
        uint8_t slot = MatchSorted(size);
        assert(slot >= prev);
        prev = slot;
    }
    assert(prev == 63);
}

void HeapOp()
{
    Raw raw = malloc(sizeof(uint8_t) * 4096);
    Heap *heap = CreateHeap(raw, 4096);

    assert(heap->head == heap->last);
    assert(!GetUsed(heap->head));
    assert(!heap->head->prev);
    assert(!heap->head->next);
    assert(GetSize(heap->head) > 0);
    uint8_t slot = MatchSorted(GetSize(heap->head));
    for (int i = 0; i < 64; i += 1)
    {
        assert(!heap->exact[i]);
        if (i != slot)
        {
            assert(!heap->sorted[i]);
        }
        else
        {
            assert(heap->sorted[i] == heap->head);
        }
    }

    Chuck *ck = heap->head;
    MoveOut(heap, ck);
    assert(!heap->head);
    assert(!heap->last);
    MoveIn(heap, ck);
    assert(heap->head == ck);
    assert(heap->last == ck);

    MoveOut(heap, ck);
    Chuck *ck1 = ck;
    Chuck *ck2 = SplitIfWorthy(ck1, 128);
    Chuck *ck3 = SplitIfWorthy(ck2, 64);
    SplitIfWorthy(ck3, 256);
    MoveIn(heap, ck1);
    MoveIn(heap, ck3);
    assert(heap->head == ck1);
    assert(ck1->prev == NULL);
    assert(ck1->next == ck3);
    assert(ck3->next == NULL);
    assert(heap->last == ck3);
    assert(FindSmallestFitInHeap(heap, 0) == ck1);
    for (Size size = 0; size <= GetSize(ck3); size += 8)
    {
        if (size <= GetSize(ck1))
        {
            assert(FindSmallestFitInHeap(heap, size) == ck1);
        }
        else
        {
            assert(FindSmallestFitInHeap(heap, size) == ck3);
        }
    }
    assert(FindSmallestFitInHeap(heap, GetSize(ck3) + 8) == NULL);

    MoveIn(heap, ck2);
    assert(heap->head == ck2);
    assert(ck2->next == ck1);
    MoveOut(heap, ck1);
    assert(ck2->next == ck3);
    MoveOut(heap, ck3);
    assert(ck2->next == NULL);
    assert(heap->last == ck2);

    free(raw);
}

void ObjectFullyUsable()
{
    Raw raw = malloc(sizeof(uint8_t) * (64u << 20u));
    Heap *heap = CreateHeap(raw, 64u << 20u);

    Raw objects[1024];
    for (int i = 0; i < 1024; i += 1)
    {
        objects[i] = AllocateObject(heap, (i + 1) * 32);
        assert(objects[i] != NULL);
        memset(objects[i], 0xcc, (i + 1) * 32);
    }

    free(raw);
}

void ReleaseNoLoss()
{
    Raw raw = malloc(sizeof(uint8_t) * (64u << 20u));
    Heap *heap = CreateHeap(raw, 64u << 20u);
    Size originalSize = GetSize(heap->head);

    Raw objects[1024];
    for (int i = 0; i < 1024; i += 1)
    {
        objects[i] = AllocateObject(heap, (i + 1) * 32);
    }
    for (int i = 1023; i >= 0; i -= 1)
    {
        ReleaseObject(heap, objects[i]);
    }
    assert(GetSize(heap->head) == originalSize);

    for (int i = 0; i < 1024; i += 1)
    {
        objects[i] = AllocateObject(heap, (i + 1) * 32);
    }
    for (int i = 0; i < 1024; i += 1)
    {
        ReleaseObject(heap, objects[i]);
    }
    assert(GetSize(heap->head) == originalSize);

    free(raw);
}

void AssertConsistency(Heap *heap, Chuck *lowest, Chuck *highest)
{
    (void)heap, (void)lowest, (void)highest;
#ifdef NOMALLOC_TEST_RESTRICT
    Size prevSize = 0;
    uint32_t inListCount = 0;
    for (Chuck *current = heap->head; current; current = current->next)
    {
        Size size = GetSize(current);
        assert(size >= prevSize);
        prevSize = size;

        assert(current->lowerUsed);
        assert(GetUsed(GetHigher(current)));
        inListCount += 1;
    }
    uint32_t totalFreeCount = 0;
    for (Chuck *current = lowest; current != highest; current = GetHigher(current))
    {
        assert(current->lowerUsed || GetUsed(current));
        if (!GetUsed(current))
        {
            assert(GetLower(GetHigher(current)) == current);
            totalFreeCount += 1;
        }
    }
    assert(inListCount == totalFreeCount);
#endif
}

void Random()
{
    // no set seed for reproducible
    Raw raw = malloc(sizeof(uint8_t) * (64u << 20u));
    Heap *heap = CreateHeap(raw, 64u << 20u);
    Size originalSize = GetSize(heap->head);
    Chuck *lowest = heap->head, *highest = GetHigher(GetHigher(heap->head));

    Raw objects[16384];
    memset(objects, 0, sizeof(Raw) * 16384);
    for (int i = 0; i < 100000000; i += 1)
    {
        long index = random() % 16384;
        if (objects[index])
        {
            ReleaseObject(heap, objects[index]);
            objects[index] = NULL;
        }
        else
        {
            objects[index] = AllocateObject(heap, random() % (32u << 10u));
        }
        AssertConsistency(heap, lowest, highest);
    } // no assertion, I will be glad if it will not break itself
    for (int i = 0; i < 16384; i += 1)
    {
        if (objects[i])
        {
            ReleaseObject(heap, objects[i]);
        }
    }
    assert(heap->head == heap->last);
    assert(GetSize(heap->head) == originalSize);

    free(raw);
}

void Misc()
{
    Raw raw[8192];
    Heap *heap = CreateHeap(raw, 8192);
    assert(AllocateObject(heap, 16384) == NULL);
    uint32_t *array = AllocateZeroedObjectList(heap, 128, sizeof(uint32_t));
    for (int i = 0; i < 128; i += 1)
    {
        assert(array[i] == 0);
    }
    assert(ResizeObject(heap, array, 0) == array);
    assert(ResizeObject(heap, array, 256 * sizeof(uint32_t)) == array);
    AllocateObject(heap, 0);
    Raw *resized = ResizeObject(heap, array, 512 * sizeof(uint32_t));
    assert(resized);
    assert(resized != (Raw)array);
}

#define RunTest(testName)        \
    printf("* %s\n", #testName); \
    testName()

int main(void)
{
    RunTest(ChuckOp);
    RunTest(MatchSlot);
    RunTest(HeapOp);
    RunTest(ObjectFullyUsable);
    RunTest(ReleaseNoLoss);
    RunTest(Random);
    RunTest(Misc);
    return 0;
}