#include "../src/mymalloc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

#define CHECK_NULL(x)                                                 \
  do {                                                                \
    if (x == NULL) {                                                  \
      fprintf(stderr, "my_malloc returned NULL. Aborting program\n"); \
      exit(1);                                                        \
    }                                                                 \
  } while (0)

static inline void **mallocing_loop(void **array, size_t size, size_t count) {
  for (size_t i = 0; i < count; i++) {
    void *v = my_malloc(size);
    CHECK_NULL(v);
    if (array != NULL)
      array[i] = v;
  }
  return array;
}

static inline void *mallocing(size_t size) {
  void *ptr = my_malloc(size);
  CHECK_NULL(ptr);
  return ptr;
}

static inline void freeing_loop(void **array, size_t count) {
  for (size_t i = 0; i < count; i++) {
    my_free(array[i]);
  }
}

static inline void freeing(void *ptr) {
  my_free(ptr);
}
