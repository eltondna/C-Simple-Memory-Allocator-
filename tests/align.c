#include "testing.h"
#include <assert.h>
#include <stdbool.h>

/**
 * This test checks that the memory you allocate is properly aligned.
 *
 * Reason(s) you might be failing this test:
 *   - Your my_malloc function returns addresses that aren't a multiple of
 *     kAlignment.
 **/

bool is_word_aligned(void *ptr) {
  return (((size_t)ptr) & (sizeof(size_t) - 1)) == 0;
}

int main(void) {
  void *ptr = mallocing(1);
  assert(is_word_aligned(ptr));
  void *ptr2 = mallocing(1);
  assert(is_word_aligned(ptr2));
  return 0;
}
