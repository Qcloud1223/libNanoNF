#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void p1()
{
    printf("Hello from lib1.so!\n");
    void *p = malloc(1024);
    printf("explicitly allocate a memory at: %p\n", p);
    void *cp = calloc(1, 1024);
    printf("calloc succeed\n");
    memcpy(p, cp, 100);
    printf("memcpy succeed\n");
    free(p);
}
