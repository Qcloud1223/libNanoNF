//

#include "Loader.h"
#include "Loader/NanoNF.h"
#include <stdlib.h>

Library *CreateLibrary(const char *file)
{
    Library *lib = malloc(sizeof(Library));
    if (!lib)
    {
        return NULL;
    }
    lib->file = file;
    lib->size = NFusage_worker(file, 0);
    lib->address = NULL;
    lib->internal = NULL;
    return lib;
}

int LoadLibrary(Library *lib, const ProxyRecord records[])
{
    lib->internal = NFopen(lib->file, 0, lib->address, records);
    return lib->internal == NULL;
}

void *GetFunction(Library *lib, const char *name)
{
    return NFsym(lib->internal, name);
}

void ReleaseLibrary(Library *lib)
{
    // todo: internally close
    free(lib);
}