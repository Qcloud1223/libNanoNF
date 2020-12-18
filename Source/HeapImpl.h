//
#if defined NOMALLOC_SOURCE_HEAP_IMPL_H
#error "Implementation header should not be included from multiple sources."
#endif
#define NOMALLOC_SOURCE_HEAP_IMPL_H

#include <stdint.h>
#include <assert.h>

typedef struct ChuckLayout {
    uint32_t lowerSize;
    uint8_t lowerUsed: 1;
    uint32_t size: 31;
    struct ChuckLayout *prev;
    struct ChuckLayout *next;
} Chuck;

/*
`prev` and `next`, and `lowerSize` in the higher layout are overwritten in used 
chuck; `lowerUsed` and `size` are overhead of used chuck, totally sized 4 bytes 
for now

notice that a chuck is at least as large as sizeof(Chuck) which provides 20 
bytes available spaces even if user does not require that much, which introduces 
some more implicit overhead

`size` and `lowerSize` are not available size, but total chuck size with 
overhead; the user-required size is added with overhead before any processing, 
so every size variable inside implementation counts overhead
total chuck size is always multiple of 8 bytes, so the two fields are encoded 
without the 3 least significant bits
*/
static const Size OVERHEAD_OF_USED_CHUCK = sizeof(uint32_t);
static const Size OBJECT_OFFSET = sizeof(uint32_t) * 2;

typedef uint8_t *Bytes;

static Raw ToObject(Chuck *chuck) {
    return (Raw) chuck + OBJECT_OFFSET;
}

static Chuck *GetHigher(Chuck *chuck);

static void SetHigher(Chuck *chuck, Chuck *higher);

static Chuck *ToChuck(Raw object) {
    Chuck *chuck = object - OBJECT_OFFSET;
    // Not applicable to realloc
    // SetHigher(chuck, GetHigher(chuck)); // restore back link for higher chuck
    return chuck;
}

static Size GetSize(Chuck *chuck) {
    return (Size) chuck->size << 3u;
}

static void SetSize(Chuck *chuck, Size size) {
    assert(!(size & 0x07u));
    chuck->size = size >> 3u;
    // GetHigher(chuck) does not work before setting chuck->size
    GetHigher(chuck)->lowerSize = size >> 3u;
}

static uint8_t GetUsed(Chuck *chuck) {
    // assert: there is a higher chuck
    return GetHigher(chuck)->lowerUsed;
}

static void SetUsed(Chuck *chuck, uint8_t used) {
    // assert: there is a higher chuck
    GetHigher(chuck)->lowerUsed = used;
}

// GetPrev & GetNext is trivial so no helper

static void SetPrev(Chuck *chuck, Chuck *prev) {
    chuck->prev = prev;
    if (prev) {
        prev->next = chuck;
    }
}

static void SetNext(Chuck *chuck, Chuck *next) {
    chuck->next = next;
    if (next) {
        next->prev = chuck;
    }
}

static Chuck *GetLower(Chuck *chuck) {
    assert(!chuck->lowerUsed); // lowerSize is not taken
    return (Chuck *) ((Bytes) chuck - (chuck->lowerSize << 3u));
}

static Chuck *GetHigher(Chuck *chuck) {
    return (Chuck *) ((Bytes) chuck + GetSize(chuck));
}

static void SetLower(Chuck *chuck, Chuck *lower) {
    assert(!chuck->lowerUsed); // lowerSize is not taken
    SetSize(lower, (Bytes) chuck - (Bytes) lower);
}

static void SetHigher(Chuck *chuck, Chuck *higher) {
    // huge mistake here
    // we are just about to set lowerSize to another place
    // assert(!GetUsed(chuck)); // lowerSize of higher chuck is not taken
    SetSize(chuck, (Bytes) higher - (Bytes) chuck);
}

static Chuck *FindSmallestFit(Chuck *start, Size size) {
    for (Chuck *current = start; current; current = current->next) {
        assert(!GetUsed(current));
        if (GetSize(current) >= size) {
            return current;
        }
    }
    return NULL;
}

static void Insert(Chuck *start, Chuck *chuck) {
    Chuck *current;
    for (current = start; GetSize(chuck) >= GetSize(current);
         current = current->next) {
        if (!current->next) {
            SetPrev(chuck, current);
            SetNext(chuck, NULL);
            return;
        }
    }
    Chuck *prev = current->prev, *next = current;
    SetPrev(chuck, prev);
    SetNext(chuck, next);
}

static Chuck *SplitIfWorthy(Chuck *chuck, Size size) {
    assert(!GetUsed(chuck));
    assert(size <= GetSize(chuck));
    if (size < sizeof(Chuck)) {
        size = sizeof(Chuck);
    }
    size = (size + 0x07u) >> 3u << 3u;
    // split to get a chuck whose size equals to sizeof(Chuck) is still worthy
    // it could store sizeof(Chuck) - OVERHEAD_OF_USED_CHUCK bytes data
    if (GetSize(chuck) < size + sizeof(Chuck)) {
        return NULL;
    }
    Chuck *lower = chuck, *higher = GetHigher(chuck);
    SetSize(lower, size);
    Chuck *avail = GetHigher(lower);
    // SetUsed(avail, 0);
    SetUsed(lower, 0);
    // a little worry that compiler is not clever enough to eliminate this
    SetLower(avail, lower);
    SetHigher(avail, higher);
    return avail;
}

/*
(real) Layout of heap
                   /----1st chuck-----\ /-2nd chuck--\
+-----------------+--------+-+-+----+--+----+-+-+--+--+----
|HeapLayout fields|lu(=1):s|p|n|####|ls|lu:s|p|n|##|ls|....
+-----------------+--------+-+-+----+--+----+-+-+--+--+----
               ^           *        ^       *      ^

     /---last chuck---\ /-----ending overhead-----\
----+----+-+-+------+--+-----------+-+-+--+--------+
....|lu:s|p|n|######|ls|lu:s(=3<<3)|p|n|ls|lu(=1):s|
----+----+-+-+------+--+-----------+-+-+--+--------+
 ^       *          ^                  ^

lu:s lowerUsed & size, (p)rev, (n)ext, ls lowerSize, #s are not used 
^s are actually position of pointers of `Chuck`s, *s are object pointers for 
user
the first and last ChuckLayout structs (not conceptual chucks) are not complete
*/

struct HeapLayout {
    Chuck *head, *last;
    // a slot of sorted bins points to the chuck whose prev chuck is not large
    // enough for this slot, or NULL if that chuck is too large for this slot
    Chuck *exact[64], *sorted[64];
};

static uint8_t MatchExact(Size size) {
    if (size < sizeof(Chuck)) {
        size = sizeof(Chuck);
    }
    assert(!(size & 0x07u));
    // size in [24, 536)
    assert(((size - sizeof(Chuck)) >> 3u) < 64);
    return (size - sizeof(Chuck)) >> 3u;
}

static const Size NOT_EXACT_MIN = (64u << 3u) + sizeof(Chuck);

static uint8_t MatchSorted(Size size) {
    assert(size >= NOT_EXACT_MIN);
#ifndef __GNUC__
#error "GCC builtin is used."
#endif
    unsigned mostSigBitIndex =
            (unsigned) sizeof(Size) * 8 - __builtin_clzl(size) - 1;
    assert(mostSigBitIndex >= 9);
    if (((mostSigBitIndex - 9) << 2u) >= 64) {
        return 63;
    }
    return ((mostSigBitIndex - 9) << 2u) |
           ((size >> (mostSigBitIndex - 2u)) & 0x03u);
}

static Chuck *FindSmallestFitInHeap(Heap *heap, Size size) {
    Chuck *start = NULL;
    if (size < NOT_EXACT_MIN) {
        for (int i = MatchExact(size); i < 64; i += 1) {
            if (heap->exact[i]) {
                start = heap->exact[i];
                break;
            }
        }
    }
    if (!start) {
        for (int i = MatchSorted(size > NOT_EXACT_MIN ? size : NOT_EXACT_MIN);
             i < 64; i += 1) {
            if (heap->sorted[i]) {
                start = heap->sorted[i];
                break;
            }
        }
    }
    return FindSmallestFit(start, size);
}

static void MoveIn(Heap *heap, Chuck *chuck) {
    if (heap->head) {
        Chuck *fit = FindSmallestFitInHeap(heap, GetSize(chuck));
        Insert(fit ? fit : heap->last, chuck);
    } else {
        SetPrev(chuck, NULL);
        SetNext(chuck, NULL);
    }

    if (!heap->head) {
        assert(!heap->last);
        heap->head = heap->last = chuck;
    } else {
        if (chuck == heap->head->prev) {
            heap->head = chuck;
        }
        if (chuck == heap->last->next) {
            heap->last = chuck;
        }
    }

    if (GetSize(chuck) < NOT_EXACT_MIN) {
        uint8_t slot = MatchExact(GetSize(chuck));
        if (!heap->exact[slot]) {
            heap->exact[slot] = chuck;
        }
    } else {
        uint8_t slot = MatchSorted(GetSize(chuck));
        if (!heap->sorted[slot] ||
            GetSize(chuck) < GetSize(heap->sorted[slot])) {
            heap->sorted[slot] = chuck;
        }
    }
}

static void MoveOut(Heap *heap, Chuck *chuck) {
    if (chuck->prev) {
        SetNext(chuck->prev, chuck->next);
    } else if (chuck->next) {
        SetPrev(chuck->next, chuck->prev);
    }

    if (heap->head == heap->last) {
        assert(chuck == heap->head);
        heap->head = heap->last = NULL;
    } else {
        if (chuck == heap->head) {
            heap->head = chuck->next;
        }
        if (chuck == heap->last) {
            heap->last = chuck->prev;
        }
    }

    if (GetSize(chuck) < NOT_EXACT_MIN) {
        uint8_t slot = MatchExact(GetSize(chuck));
        if (heap->exact[slot] == chuck) {
            if (!chuck->next || GetSize(chuck->next) != GetSize(chuck)) {
                heap->exact[slot] = NULL;
            } else {
                heap->exact[slot] = chuck->next;
            }
        }
    } else {
        uint8_t slot = MatchSorted(GetSize(chuck));
        if (heap->sorted[slot] == chuck) {
            if (!chuck->next || MatchSorted(GetSize((chuck->next))) != slot) {
                heap->sorted[slot] = NULL;
            } else {
                heap->sorted[slot] = chuck->next;
            }
        }
    }
}
