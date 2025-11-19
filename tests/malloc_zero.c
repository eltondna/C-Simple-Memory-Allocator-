#include "../src/mymalloc.h"
#include "testing.h"

/**
 * This test checks what happens when `my_malloc` is called with a size of 0.
 *
 * Reason(s) you might be failing this test:
 * - `my_malloc` does not correctly handle the case where it receives a request
 *   for 0 bytes.
 */

int main(void) {
  void *ptr = my_malloc(0);
  void *ptr2 = mallocing(8);
  freeing(ptr2);
  if (ptr != NULL) {
    fprintf(stderr, "Expected NULL for an allocation of 0 bytes\n");
    exit(1);
  }
  return 0;
}
