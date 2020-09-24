/* return the memory usage of a shared object */
#include "../NFlink.h"

uint64_t NFusage(void *ll)
{
    /* do simple math, this is filled when NFopening */
    struct NF_link_map *l = ll;
    return l->l_map_end - l->l_map_start;
}
