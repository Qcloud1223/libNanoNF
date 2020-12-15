#include "../NanoNF.h"
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

//note that you may want to test your own malloc
//this variable is hard-coded and is designed to clobber the malloc in shared libraries
Elf64_Addr REAL_MALLOC = 0x0;

int main()
{
    void * p = malloc;
    REAL_MALLOC = (Elf64_Addr) p; // bind the custom malloc to the libc version
    Elf64_Addr usage = NFusage_worker("lib1.so", 0);
    void *handle = NFopen("lib1.so", 0, (void *)0x555555888000);

    void (*p1)() = NFsym(handle, "p1");
    p1();

    int sleeper;
    scanf("%d", &sleeper); //use proc maps to check memory map
    
    return 0;
}
