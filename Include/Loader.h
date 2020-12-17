#ifndef NOMALLOC_INCLUDE_LOADER_H
#define NOMALLOC_INCLUDE_LOADER_H

#include <stddef.h>

typedef size_t Size;
typedef void *Raw;

typedef struct
{
    const char *file;
    Size size;
    Raw address;
    Raw internal;
} Library;

typedef struct
{
    const char *name;
    void *pointer;
} ProxyRecord;

Library *CreateLibrary(const char *file);

int LoadLibrary(Library *library, ProxyRecord records[]);

void *GetFunction(Library *library, const char *name);

void ReleaseLibrary(Library *library);

#endif