#ifndef INTERNALTESTS_HEADER
#define INTERNALTESTS_HEADER

#include "../src/mymalloc.h"
#include <assert.h>
#include <stdlib.h>

/* Internal test logging - these won't be conditionally disabled like "LOG" is.
   It will also include the file name and line number to help you see where your
   internal tests are failing.
*/
#define ILOG(...)                                    \
  do {                                               \
    fprintf(stderr, "[" __FILE__ ":%d] ", __LINE__); \
    fprintf(stderr, __VA_ARGS__);                    \
  } while (0)

size_t count_free_blocks(void);
size_t count_all_blocks(void);


#endif