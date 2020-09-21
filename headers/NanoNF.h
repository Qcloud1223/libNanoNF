/* include the function headers */
#include "NFlink.h"

extern void *NFopen(const char* file, int mode, void *addr);
extern void *NFsym(struct NF_link_map *l, const char *s);