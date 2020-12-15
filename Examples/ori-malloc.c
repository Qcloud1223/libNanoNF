#include "Loader.h"
#include <stdlib.h>

int main()
{
    Library *lib_1 = CreateLibrary("lib_1.so");
    lib_1->address = malloc(lib_1->size);
    ProxyFunction proxy[] = {{NULL, NULL}};
    LoadLibrary(lib_1, proxy);
    void (*p1)() = GetFunction(lib_1, "p1");
    p1();

        return 0;
}
