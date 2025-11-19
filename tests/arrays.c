#include "testing.h"

/**
 * This test checks that previously allocated blocks aren't being overwritten by
 * subsequent calls to `my_malloc`.
 *
 * Reason(s) you may be failing this test:
 * - Accidentally allocating previously allocated memory
 */

#define LEN 10
#define SUM_N(n) ((n) * ((n) - 1) / 2)

int main(void) {
  int actual_sum = 0, expected_sum = SUM_N(LEN);

  int *array1 = mallocing(sizeof(int) * LEN);

  for (int i = 0; i < LEN; i++)
    array1[i] = i;

  int *array2 = mallocing(sizeof(int) * LEN);

  for (int i = 0; i < LEN; i++)
    array2[i] = -1;

  for (int i = 0; i < LEN; i++)
    actual_sum += array1[i];

  return expected_sum - actual_sum;
}
