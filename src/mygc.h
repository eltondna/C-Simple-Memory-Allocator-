#ifndef MYGC_HEADER
#define MYGC_HEADER

#include "mymalloc.h"
#include <stddef.h>

void set_start_of_stack(void *start_addr);
void *get_end_of_stack(void);
void my_gc(void);

#endif
