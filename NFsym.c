/* Search and return a symbol inside a NF_link_map
 * May turn to hash table in the future
 * But now, for simplicity, I just search it linearly
 */

#include "headers/NFlink.h"
#include <elf.h>
#include <stdlib.h> //for NULL
#include <string.h>

void *NFsym(struct NF_link_map *l, const char *s)
{
    Elf64_Sym *symtab = (void *)l->l_info[DT_SYMTAB]->d_un.d_ptr;
    const char *strtab = (void *)l->l_info[DT_STRTAB]->d_un.d_ptr;

    Elf64_Sym *curr_sym = symtab;
    //strtab is at higher address, so this can serve as a end point
    while((void *)curr_sym < (void *)strtab)
    {
        Elf64_Word name = curr_sym->st_name;
        if(!strcmp(strtab + name, s))
        {
            //found it
            //FIXME: you may want to check if the symbol is valid. E.g., is it UND?
            return (void *)curr_sym->st_value;
        }
        curr_sym++;
    }
    return NULL; //find nothing
}