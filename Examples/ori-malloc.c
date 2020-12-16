#include "Loader.h"
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

int main()
{
    Library *lib_1 = CreateLibrary("lib_1.so");
    lib_1->address = memalign(getpagesize(), lib_1->size);
    printf("load lib_1.so to %p\n", lib_1->address);
    ProxyFunction proxy[] = {{NULL, NULL}};
    LoadLibrary(lib_1, proxy);
    void (*p1)() = GetFunction(lib_1, "p1");
    p1();

    printf("press any key to exit...");
    getch();
    printf("\n");
    ReleaseLibrary(lib_1);
    return 0;
}
