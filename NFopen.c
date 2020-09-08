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

#include "headers/NFlink.h"

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

    /* search and load and map, space is allocated here on heap */
    struct NF_link_map *new;
    args->new = new = NF_map(file, mode, addr);

    /* relocating */
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