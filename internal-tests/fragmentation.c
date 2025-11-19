#include "internal-tests.h"
#include <stdlib.h> /* Defines rand, srand */
#include <time.h>   /* Defines time */

/** Starting code for writing tests that measure memory fragmentation.
 *  Note that the CI will not run this test intentionally. 
 */

// You can modify these values to be larger or smaller as needed
// By default they are quite small to help you test your code.
#define REPTS 1000
#define NUM_PTRS 100
#define MAX_ALLOC_SIZE 4096

char *ptrs[NUM_PTRS];

/* Returns a random number between min and max (inclusive) */
int random_in_range(int min, int max) {
  return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

/* Performs REPTS number of calls to my_malloc/my_free. */
void random_allocations() {
  for (int i = 0; i < REPTS; i++) {
    int idx = random_in_range(0, NUM_PTRS-1);
    if (ptrs[idx] == NULL) {
      size_t random_size = (size_t) random_in_range(0, MAX_ALLOC_SIZE);
      ptrs[idx] = my_malloc(random_size);
    } else {
      my_free(ptrs[idx]);
      ptrs[idx] = NULL;
    }
  }
}

/* Usage: passing an unsigned integer as the first argument will use that value
 * to seed the pRNG. This will allow you to re-run the same sequence of calls to
 * my_malloc and my_free for the purposes of debugging or measuring 
 * fragmentation. 
 * If a seed is not given to the program it will use the current time instead.
 */
int main(int argc, char const *argv[]) {
  unsigned int seed; 
  if (argc < 2) {
    seed = (unsigned int) time(NULL); 
  } else {
    sscanf(argv[1], "%u", &seed); 
  }
  fprintf(stderr, "Running fragmentation test with random seed: %u\n", seed);
  srand(seed);
  random_allocations(); 

  /* TODO: put your code to measure and report memory fragmentation here */

  return 0;
}
