// Massive TODO (2020.12.17 by sgdxbc): 
// The integration of proxy functions introduces a major refactor in plan
// Currently, ProxyRecord[] just get inserted in the end of arguments and 
// pass down to relocating function all the way, which breaks the modularity
// Rethinking its role is necessary, because this is a new feature only provided
// by NoMalloc, not the original dlopen

/* include the function headers */
#include "NFlink.h"
#include "Loader.h"
#include <stdint.h>

extern void *NFopen(const char *file, int mode, void *addr, ProxyRecord *records);
extern void *NFsym(void *l, const char *s);
extern uint64_t NFusage(void *l);
extern uint64_t NFusage_worker(const char *name, int mode);