#include "internal-tests.h"

/** This test will check that you have implement block splitting correctly. 
 *  It makes a single allocation and counts the number of blocks and the number
 *  of free blocks. It expects the former to be at least two, and the latter to
 *  be at least one.
 *  
 *  If you have failed this test, it is likely you haven't implemented block 
 *  splitting correctly. 
 */

/* Returns the number of free blocks. */
size_t count_free_blocks(void) {
  size_t num_free = 0;
  Block *curr = get_start_block();
  Block *next = NULL;
  while (curr) {
    if (is_free(curr)) 
      num_free += 1;
    next = get_next_block(curr);
    // This will check whether get_next_block returned an identical ptr to the 
    // one it was given. If this occurs, it is most likely due to an internal
    // error in your allocator. Possible causes include an error in the function 
    // `get_next_block` or a block that somehow has size 0.
    if (curr == next) {
      ILOG("get_next_block for %p returned identical block\n", curr);
      exit(1);
    } 
    curr = next;
  }
  return num_free;
}

/* Returns the number of both free and allocated blocks. */
size_t count_all_blocks(void) {
  size_t total = 0;
  Block *curr = get_start_block();
  Block *next = NULL;
  while (curr) {
    total += 1;
    next = get_next_block(curr);
    // This will check whether get_next_block returned an identical ptr to the 
    // one it was given. If this occurs, it is most likely due to an internal
    // error in your allocator. Possible causes include an error in the function 
    // `get_next_block` or a block that somehow has size 0.
    if (curr == next) {
      ILOG("get_next_block for %p returned identical block\n", curr);
      exit(1);
    } 
    curr = next;
  }
  return total;
}

int main(int argc, char const *argv[]) {
  size_t *ptr =  my_malloc(sizeof(size_t));
  size_t all_blocks  = count_all_blocks();
  size_t free_blocks = count_free_blocks(); 
  if (all_blocks < 2) {
    ILOG("Expected at least 2 blocks, got %lu instead\n", all_blocks);
    return 1;
  } else if (free_blocks < 1) {
    ILOG("Expected at least 1 free block, got %lu instead\n", free_blocks);
    return 1;
  }
  return 0;
}
