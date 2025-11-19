/* Benchmark malloc and free functions.
   Copyright (C) 2019-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include "../tests/testing.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <time.h>

/* Benchmark the malloc/free performance of a varying number of blocks of a
   given size. */

#define NUM_ITERS 300
#define NUM_ALLOCS 4
#define MAX_ALLOCS 200

typedef struct {
  size_t iters;
  size_t size;
  int n;
} malloc_args;

static void do_benchmark(malloc_args *args, char **arr) {
  size_t iters = args->iters;
  size_t size = args->size;
  int n = args->n;

  for (int j = 0; j < iters; j++) {
    for (int i = 0; i < n; i++) {
      arr[i] = mallocing(size);
      for (int g = 0; g < size; g++) {
        arr[i][g] = (char)g;
      }
    }

    // free half in fifo order
    for (int i = 0; i < n / 2; i++) {
      freeing(arr[i]);
    }

    // and the other half in lifo order
    for (int i = n - 1; i >= n / 2; i--) {
      freeing(arr[i]);
    }
  }

}

static malloc_args tests[3][NUM_ALLOCS];
static int allocs[NUM_ALLOCS] = {25, 100, 400, MAX_ALLOCS};

void bench(unsigned long size) {
  size_t iters = NUM_ITERS;
  char **arr = (char **)mallocing(MAX_ALLOCS * sizeof(void *));
  for (int t = 0; t < 3; t++)
    for (int i = 0; i < NUM_ALLOCS; i++) {
      tests[t][i].n = allocs[i];
      tests[t][i].size = size;
      tests[t][i].iters = iters / allocs[i];

      /* Do a quick warmup run.  */
      if (t == 0)
        do_benchmark(&tests[0][i], arr);
    }
  /* Run benchmark single threaded in main_arena.  */
  for (int i = 0; i < NUM_ALLOCS; i++)
    do_benchmark(&tests[0][i], arr);
  freeing(arr);
}

static void usage(const char *name) {
  fprintf(stderr, "%s: <alloc_size>\n", name);
  exit(1);
}

int main(int argc, char **argv) {
  clock_t start_t = clock();
  long size = 16;
  if (argc == 2)
    size = strtol(argv[1], NULL, 0);

  if (argc > 2 || size <= 0)
    usage(argv[0]);

  for (int i = 0; i < 100; i++) {
    bench(size);
    bench(2 * size);
    bench(4 * size);
    bench(6 * size);
    bench(8 * size);
    bench(10 * size);
    bench(12 * size);
    bench(14 * size);
    bench(16 * size);
  }
  clock_t end_t = clock();
  double time_taken = (double)(end_t - start_t) / CLOCKS_PER_SEC;
  printf("%f\n", time_taken);
  return 0;
}
