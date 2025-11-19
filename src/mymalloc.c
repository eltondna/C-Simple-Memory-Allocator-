#include "mymalloc.h"

// Word alignment
const size_t kAlignment = sizeof(size_t);
// Minimum allocation size (1 word)
const size_t kMinAllocationSize = kAlignment;
// Size of meta-data per Block
const size_t kMetadataSize = sizeof(Block);
// Maximum allocation size (512 MB)
const size_t kMaxAllocationSize = (512ull << 20) - kMetadataSize;
// Memory size that is mmapped (256 MB)
const size_t kMemorySize = (256ull << 20);

void *my_malloc(size_t size) {
  return NULL;
}

void my_free(void *ptr) {
  return;
}

/** These are helper functions you are required to implement for internal testing
 *  purposes. Depending on the optimisations you implement, you will need to
 *  update these functions yourself.
 **/

/* Returns 1 if the given block is free, 0 if not. */
int is_free(Block *block) {
  return !block->allocated;
}

/* Returns the size of the given block */
size_t block_size(Block *block) {
  return block->size;
}

/* Returns the first block in memory (excluding fenceposts) */
Block *get_start_block(void) {
  return NULL;
}

/* Returns the next block in memory */
Block *get_next_block(Block *block) {
  return NULL;
}

/* Given a ptr assumed to be returned from a previous call to `my_malloc`,
   return a pointer to the start of the metadata block. */
Block *ptr_to_block(void *ptr) {
  return ADD_BYTES(ptr, -((ssize_t) kMetadataSize));
}
