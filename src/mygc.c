#include "mygc.h"

static void *start_of_stack = NULL;

// Word alignment
const size_t kAlignment = sizeof(size_t);
// Minimum allocation size (1 word)
const size_t kMinAllocationSize = kAlignment;
// Size of meta-data per Block
const size_t kMetadataSize = sizeof(Block);
// Maximum allocation size (512 MB)
const size_t kMaxAllocationSize = (128ull << 20) - kMetadataSize;
// Memory size that is mmapped (256 MB)
const size_t kMemorySize = (64ull << 20);

// Call this function in your test code (at the start of main)
void set_start_of_stack(void *start_addr) {
  start_of_stack = start_addr;
}

void *my_malloc(size_t size) {
  return NULL;
}

void my_free(void *ptr) {
  return; 
}

void *get_end_of_stack() {
  return __builtin_frame_address(1);
}

__attribute__((no_sanitize_address)) // ASan dislikes manual stack unrolling
void my_gc() {
  void *end_of_stack = get_end_of_stack();

  // TODO
}
