#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include "Global.h"
#include "Heap.h"

void *malloc(size_t size) {
    // exit(14);
    return GlobalMalloc(size);
}

void *realloc(void *object, size_t size) {
    return GlobalRealloc(object, size);
}

void *calloc(size_t count, size_t size) {
    return GlobalCalloc(count, size);
}

void free(void *object) {
    GlobalFree(object);
}

extern void InitializeFunction(void);

int main() {
    void *heap = memalign(getpagesize(), 1u << 30u);
    SetCurrentHeap(CreateHeap(heap, 1u << 30u));
    InitializeFunction();
    return 0;
}