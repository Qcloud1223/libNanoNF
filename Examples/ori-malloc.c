#include "Loader.h"
#include "Heap.h"
#include "Global.h"
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <termio.h>

// https://stackoverflow.com/a/16361724
char getch(void)
{
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    // printf("%c\n", buf);
    return buf;
}

// heap size: 20MB
#define HEAP_SIZE (64u << 20u)

int main()
{
    Library *lib_1 = CreateLibrary("./lib_1.so");
    lib_1->address = memalign(getpagesize(), lib_1->size + HEAP_SIZE);
    printf("lib_1.so loading address: %p\n", lib_1->address);

    printf("create heap for lib_1 at %p\n", lib_1->address + lib_1->size);
    Heap *libHeap = CreateHeap(lib_1->address + lib_1->size, HEAP_SIZE);

    ProxyRecord records[] = {
        {"malloc", GlobalMalloc},
        {"realloc", GlobalRealloc},
        {"calloc", GlobalCalloc},
        {"free", GlobalFree},
        {"gibbish", NULL}};
    LoadLibrary(lib_1, records);

    SetCurrentHeap(libHeap);
    void (*p1)() = GetFunction(lib_1, "p1");
    p1();

    printf("press any key to exit...");
    getch();
    printf("\n");

    free(lib_1->address);
    ReleaseLibrary(lib_1);
    return 0;
}
