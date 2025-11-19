#include "internal-tests.h"

/* This is a very simple internal test to check whether an allocated Block is 
 * correctly marked.
 * If you are failing this test, it is possible you are not correctly marking a
 * block as allocated. 
 * If you have implemented the metadata reduction optimisation in a way that 
 * changes how the allocation status is stored, check the implementation of 
 * `is_free` is correct as well. 
 */

int main(int argc, char const *argv[]) {
  unsigned long *ptr = my_malloc(sizeof(unsigned long)); 
  if (ptr == NULL) {
    ILOG("my_malloc unexpectedly returned NULL.\n");
    return 1;
  } 

  if (is_free(ptr_to_block(ptr))) {
    ILOG("Block returned from my_malloc was not correctly set at allocated.\n");
    return 1;
  }
}
