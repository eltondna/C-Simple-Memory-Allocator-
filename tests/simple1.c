#include "testing.h"

/**
 * This is a very basic test of the memory allocator that requests enough memory
 * to store a single `int`.
 */

int main(void) {
  int *mem1 = (int *)mallocing(sizeof(int));
  *mem1 = 10;
  return *mem1 - *mem1;
}
