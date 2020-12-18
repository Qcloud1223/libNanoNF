#include <stdio.h>
#include <stdlib.h>

void p1()
{
    printf("Hello from lib1.so!\n");
    void *p = malloc(1024);
    printf("explicitly allocate a memory at: %p\n", p);
    free(p);
}
