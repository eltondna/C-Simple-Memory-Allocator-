#include "internal-tests.h"
#include <string.h>

/** This test checks that the size of a block is at least as big as the 
 *  requested size + kMetadataSize. 
 * 
 *  If you are failing this test it may be because you have either allocated a 
 *  block that is too small, or incorrectly updated the metadata. 
 */

int check_expected_size(void *malloced_ptr, size_t size) {
  Block *block = ptr_to_block(malloced_ptr);
  size_t actual_size = block_size(block);
  size_t expected_size = size + kMetadataSize;
  if (actual_size < (expected_size)) {
    ILOG("check_expected_size error: "
         "expected at least %lu for my_malloc(%lu), got %lu instead\n",
         expected_size, size, actual_size);
    return 0;
  }
  return 1;
}

int main(int argc, char const *argv[]) {
  size_t buff_size = sizeof(char) * 21;
  char *buff = my_malloc(buff_size);
  if (buff == NULL) {
    ILOG("Expected my_malloc to return non-NULL value.\n");
    exit(1);
  }

  memset(buff, 0xFF, buff_size); 

  if (!check_expected_size(buff, buff_size)) {
    ILOG("check_expected_size failed after setting memory " 
         "- Block metadata may have been overwritten.\n");
    return 1;
  }
  
  return 0;
}
