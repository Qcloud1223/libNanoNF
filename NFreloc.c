/* Do relocation for a loaded elf
 * Thanks to the info in l_info, we now know many extra information, like the address of synbol table
 * 
 * Note that l_info is just a index for fast, or random access
 * In limited, user version of link_map, l_ld exists. This contains enough info for symbol lookup
 * even we are not dynamic linker.
 * 
 * Simply breaking the relocation into 2 types: mess/do not mess with the GOT
 * Former one is called relative relocation: though variables and functions inside a so is access via rip
 *      other libraries will search its symbol table 
 * Latter one is more familiar: We first find the map containing the symbol, find its actual address, 
 *      and fill the GOT 
 * 
 * readelf --relocs can show the relocation entries
*/

#include "headers/NFlink.h"
#include <elf.h>
#include <link.h> //we have to search link_map here

/* a uniform structure for .data and .text relocation */
struct uniReloc
{
    Elf64_Addr start;
    Elf64_Addr size;
    Elf64_Xword nrelative; //count of relative relocs, omitted in .text
    int lazy; //lazy reloc, omitted in .data
}ranges[2] = {{0, 0, 0, 0},{0, 0, 0, 0}};

static int lookup_linkmap(struct link_map *l, const char *name)
{
    /* search the symbol table of given link_map to find the occurrence of the symbol
        return 1 upon success and 0 otherwise */
    Elf64_Dyn *dyn = l->l_ld;
    while(dyn->d_tag != DT_STRTAB)
        ++dyn;
    /* string table is in a flatten char array */
    const char *strtab = (void *)dyn->d_un.d_ptr;
    while(dyn->d_tag != DT_SYMTAB)
        ++dyn;
    /* dyn is now at the symbol table entry */
    Elf64_Sym *sym = (void *)dyn->d_un.d_ptr;

        

}

static void do_reloc(struct NF_link_map *l, struct uniReloc *ur)
{
    Elf64_Rela *r = (void *)ur->start;  
    Elf64_Rela *r_end = r + ur->nrelative;
    Elf64_Rela *end = (void *)(ur->start + ur->size);//the end of .rel.dyn
    if (ur->nrelative)
    {
        /* do relative reloc here */

        /* start points to the beginning of .rel.dyn, and the thing in memory should be parsed as Rela entries */

        for(Elf64_Rela *it = r; it < r_end; it++)
        {
            Elf64_Addr *tmp = (void *)(l->l_addr + it->r_offset); //get the address we need to fill
            *tmp = l->l_addr + it->r_addend; //fill in the blank with rebased address
        }
    }


}

void NFreloc(struct NF_link_map *l)
{
    /* set up range[0] for relative reloc */
    if(l->l_info[DT_RELA])
    {
        ranges[0].start = l->l_info[DT_RELA]->d_un.d_ptr;
        ranges[0].size = l->l_info[DT_RELASZ]->d_un.d_val;
        ranges[0].nrelative = l->l_info[34]->d_un.d_val; //relacount is now at 34
    }

    /* do actucal reloc here using ranges set up */
    for(int i = 0; i < 2; ++i)
        do_reloc(l, &ranges[i]);
}