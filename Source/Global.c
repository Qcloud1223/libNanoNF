//

#include "Global.h"

static Heap *globalCurrent;

void SetCurrentHeap(Heap *heap)
{
    globalCurrent = heap;
}

Raw GlobalMalloc(Size size)
{
    return AllocateObject(globalCurrent, size);
}

Raw GlobalRealloc(Raw object, Size size)
{
    return ResizeObject(globalCurrent, object, size);
}

Raw GlobalCalloc(Size count, Size size)
{
    return AllocateZeroedObjectList(globalCurrent, count, size);
}

void GlobalFree(Raw object)
{
    return ReleaseObject(globalCurrent, object);
}