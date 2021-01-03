// OneBoxOneFuncOnce.c
// Run a function inside a box for one time.
#include <stdio.h>
#include <malloc.h>
#include <termio.h>
#include <unistd.h>
#include "Box.h"

typedef void (*Runner)();

void SystemPause(void);

int main(int argc, char *argv[], char **env)
{
    if (argc < 3)
    {
        printf("usage: %s [path to library] [function name]\n", argv[0]);
        return 0;
    }
    const char *file = argv[1], *function = argv[2];

    // create a box with 64MB heap for library
    Box *libraryBox = CreateBox(file, 64u << 20u);
    // make sure there are sufficient usable memory at the location
    // that the box will be deployed, the memory must be aligned to page
    // the memory region could be acquired in any way, even from stack,
    // from global variable or from OS's shared memory
    // for demostration the memory is simply `malloc`ed
    Address boxLocation = memalign(getpagesize(), GetBoxSize(libraryBox));
    printf("[driver] the box is at %p\n", boxLocation);
    // deploy the box into the location
    if (DeployBox(libraryBox, boxLocation, argc, argv, env))
    {
        fprintf(stderr, "DeployBox: fail\n");
        return 1;
    }
    // call the function
    // place function arguments in the second pair of bracket if there's any
    RunBox(libraryBox, Runner, function)();

    // pause to let user check process memory layout
    SystemPause();

    ReleaseBox(libraryBox);
    free(boxLocation);
    return 0;
}

// https://stackoverflow.com/a/16361724
void SystemPause(void)
{
    printf("press any key to exit...");
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
    printf("\n");
}
