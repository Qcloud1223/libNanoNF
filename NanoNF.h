/* include the function headers */
#include "NFlink.h"

extern void *NFopen(const char* file, int mode, void *addr);
extern void *NFsym(void *l, const char *s);
extern uint64_t NFusage(void *l);