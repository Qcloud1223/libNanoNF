/* NFopen: a minimum runnable example for loading a shared object into a specific location
 * 
 * The libc version of dlopen sacrifices too much simplicity for compatibility and extra functions, making it too hard to understand
 * e.g., auditing interface which involves namespace issues, static dlopen using hook(which is instead in libc.a, dl-open:768), statically linked libdl, etc.
 * Multi-threading is also an issue, too. See ldsodefs.h:367
 * Note that libc is meant to run on nearly all machines running different architectures, but NFopen is not
 * 
 * A dynamic loading procedure should at least contain:
 * find and open the ELF; dealing with every segment; symbol relocation
 * It should also contain:
 * loading dependencies; error handling; 
 * 
 * In libc version implementation, the majority of information is stored in struct link_map, in space calloc'd in _dl_new_object
 * Refer to include/link.h to see the full struct
 * Especially, l_map_start and l_map_end specify the allocation start and end, which is vital to usage recording
 * To correctly unmap a so, you must use the two members above
 * This shows that extra info during loading is necessary, and libc's hiding technique is clever
 * 
 * Author: Yihan Dang
 * Email: qcloud1014 at gmail.com
 */

#include "../NFlink.h"
#include <stdlib.h>
#include <stdio.h>

//extern struct NF_link_map *NF_map(const char *file, int mode, void *addr);
extern struct NF_link_map *NF_map(struct NF_list *nl, int mode, void *addr);
extern void NFreloc(struct NF_link_map *l);
extern void map_deps(struct NF_link_map *l);

struct NFopen_args
{
    /* parameters */
    const char* file;
    int mode;
    void *addr;

    /* return value */
    void *new; //point to the base address of the new link map
};

/* A worker function is needed because sometimes we want to dlopen w/o error handling(but it's enabled by default) 
 * some internal dlopen might call worker directly, e.g. -ldl also dynamically loaded the lib
 * but dlerror cannot be used in this case
 */
static void NFopen_worker(void *a)
{
    struct NFopen_args *args = a;
    const char *file = args->file;
    int mode  = args->mode;
    void *addr = args->addr;

    if(head == NULL)
    {
        printf("Fatal error: NFusage not called before NFopen!");
        //maybe directly exit here?
    }
    /* search and load and map, space is allocated here on heap */
    struct NF_link_map *new = NF_map(head, mode, addr);
    args->new = new;

    /* the mapping of dependencies is now controled by the list, so no need for map_deps */
    struct NF_list *tmp = head->next;
    while(tmp)
    {
        //add interactive querying for address here
        int tmp_mode;
        printf("shared library:%s needs an open mode:", tmp->map->l_name);
        scanf("%d", &tmp_mode);
        Elf64_Addr tmp_addr = 0x0;
        NF_map(tmp, tmp_mode, tmp_addr);
        tmp = tmp->next;
    }

    /* filling in dependency info, like search list */
    //map_deps(new);

    /* relocating */
    /* I don't think a relocation is needed when so access its internal funcs and vars
     * Using l_addr + their offset at ELF will resolve it, because we are dynamically loading
     * we don't even need to know the address of the symbols until a explicit NFsym is called
     * Upon call, NFsym check match the dynamic symbol table, and return an address
     * 
     * But external symbols certainly need relocation, for we don't know the address of other so at runtime
     * The dynamic loader would probably do the job: go through all the link map, find the dependency, and use the base address
     * of the dep as the l_addr of that symbol. In this case the main program may exist as a link map
     * because of the --rdynamic option at linking
     * In this case, we need to traverse the link_map like ld.so, and update the GOT
     * It would make a difference whether the deps are mapped as an NF or not, in which case we check both link(only 2!)
     * 
     * After all, I'm doing a dynamic loading, and don't need the symbols to be all settled when I enter the program
     */
    NFreloc(new);
}

/* mode can contain a bunch of options like symbol relocation style, whether to load, etc. see more at bits/dlfcn.h
 * so it's better got passed to every func on call chain
 */

void *NFopen(const char* file, int mode, void *addr)
{
    /* no need for a __NFopen because no static link would be used */

    /* TODO: do some mode checking before actual job */
    struct NFopen_args a;
    a.file = file;
    a.mode = mode;
    a.addr = addr;

    NFopen_worker(&a);

    return a.new;  //the base address of the link_map
}