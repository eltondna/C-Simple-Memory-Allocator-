#ifndef MYMALLOC_HEADER
#define MYMALLOC_HEADER

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>

#ifdef ENABLE_LOG
#define LOG(...) fprintf(stderr, "[malloc] " __VA_ARGS__);
#else
#define LOG(...)
#endif

#define N_LISTS 25

#define ADD_BYTES(ptr, n) ((void *) (((char *) (ptr)) + (n)))


/** This is the Block struct, which contains all metadata needed for your 
 *  explicit free list. You are allowed to modify this struct (and will need to 
 *  for certain optimisations) as long as you don't move the definition from 
 *  this file. **/
typedef struct Block Block;

struct Block {
  // Size of the block, including meta-data size.
  size_t size;
  // Next and Prev blocks
  Block *next;
  Block *prev;
  // Is the block allocated or not?
  bool allocated;
};

typedef struct Arena{
  size_t size;
  struct Arena * next;
} Arena;


// Word alignment
extern const size_t kAlignment;
// Minimum allocation size (1 word)
extern const size_t kMinAllocationSize;
// Size of meta-data per Block
extern const size_t kMetadataSize;
// Maximum allocation size (512 MB)
extern const size_t kMaxAllocationSize;
// Memory size that is mmapped (256 MB)
extern const size_t kMemorySize;

#define alloc   alloc_impl
#define release release_impl

void *alloc_impl(size_t size);
void release_impl(void *p);

/* Helper functions you are required to implement for internal testing. */

int is_free(Block *block);
size_t block_size(Block *block);

Block *get_start_block(void); 
Block *get_next_block(Block *block);

Block *ptr_to_block(void *ptr);

#endif
