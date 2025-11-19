#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "../src/mymalloc.h"
#include "testing.h"

/**
 * This test tries to call `my_free` with a memory address that wasn't allocated
 * by `my_malloc`.
 *
 * Reason(s) you might be failing this test:
 * - The `my_free` function doesn't correctly handle the case where it is given
 *   an invalid memory address.
 */

int main(void) {
  int *random_ptr = malloc(sizeof(int));
  *random_ptr = 23450;
  my_free(random_ptr);
  free(random_ptr);
  return 0;
}
