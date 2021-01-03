//

#include <stdlib.h>
#include "Box.h"
#include "Heap.h"
#include "Global.h"
#include "Loader.h"

struct BoxLayout
{
    Library *library;
    Heap *heap;
    Size heapSize;
};

const ProxyRecord BOX_RECORDS[] = {
    {"malloc", GlobalMalloc},
    {"realloc", GlobalRealloc},
    {"calloc", GlobalCalloc},
    {"free", GlobalFree},
    {NULL, NULL}};

Box *CreateBox(const char *file, Size heapSize)
{
    Box *box = malloc(sizeof(Box));
    if (!box)
    {
        return NULL;
    }
    box->library = CreateLibrary(file);
    if (!box->library)
    {
        free(box);
        return NULL;
    }
    box->heap = NULL;
    box->heapSize = heapSize;
    return box;
}

Size GetBoxSize(const Box *box)
{
    return box->library->size + box->heapSize;
}

int DeployBox(Box *box, const Address address, int argc, char *argv[], char **env)
{
    box->library->address = address;
    if (LoadLibrary(box->library, BOX_RECORDS, argc, argv, env))
    {
        return 1;
    }
    box->heap = CreateHeap(address + box->library->size, box->heapSize);
    return box->heap == NULL;
}

void PreExecute(const Box *box)
{
    SetCurrentHeap(box->heap);
}

void *GetExecutedFunction(const Box *box, const char *function)
{
    void *tmp = GetFunction(box->library, function);    
    return tmp;
}

void ReleaseBox(Box *box)
{
    ReleaseLibrary(box->library);
    free(box);
}
