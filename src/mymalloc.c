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

Block * memoryList = NULL;

// ! Multiple of 256 MB
size_t memAlign(size_t chunk, size_t alignment){
  return (chunk + alignment - 1) & ~(alignment - 1);
}

// ! Traverse free list (Best fit)
// ! Minimum Size : kMetadataSize
void * searchBlock(size_t size){
  if (memoryList == NULL)
      return NULL;
  Block * node  = memoryList;
  // ! 1. Traverse free list and find best fit blocks
  Block * best = NULL;

  // ! Linear Time to find best fit
  while (node){
      if (node->size >= size){
          if (best){
              if (best->size > node->size) 
                  best = node;
          }else best = node;
      }
      node = node->next;
  }

  // ! 1. Find Large Enough Blocks
  if (best){
    if (best->size > size){
          // ! Cond 1 : Leftover > Minimum Size (Split)
          size_t leftover = best->size - size;
          if (leftover >= (kMinAllocationSize + kMetadataSize)){
              // ! 1. Copy best node info to the leftover node 
              Block * nBlock = ((char *)best + size);
              nBlock->next = best->next;
              nBlock->prev = best->prev;
              nBlock->allocated = false;
              nBlock->size = best->size - size;

              // ! 2. Amend Neighbor reference
              if (best->next) best->next->prev = nBlock;
              if (best->prev) best->prev->next = nBlock;

              // ! 3. Set metadata of allocated node
              best->allocated = true;
              best->size      = size;
              best->prev = best->next = NULL;

              // ! If header Node, adjust 
              if (best == memoryList) {
                  memoryList = nBlock;
                  nBlock->prev = NULL;
              }
              return (void *)((char *)(best) + kMetadataSize);
          }
          // ! Cond 2 : LeftOver < Minimum Size (Allocated)
          else{
              // ! Amend Neighbor Reference
              if (best->next) best->next->prev = best->prev;
              if (best->prev) best->prev->next = best->next;

              best->allocated = true;
              best->prev = best->next = NULL;

              // ! If header Node, adjust 
              if (best == memoryList) {
                  memoryList = best->next;
                  memoryList->prev = NULL;
              }
              return (void *)((char *)(best) + kMetadataSize);
          }
      }
      // ! Cond 3 : Best Size == Required Size (Perfect)
      else{
        // ! Amend Neighbor Reference
        if (best->next) best->next->prev = best->prev;
        if (best->prev) best->prev->next = best->next;

        // ! Set metadata of allocated node
        best->allocated = true;
        best->prev = best->next = NULL;

        // ! If header Node, adjust 
        if (best == memoryList) {
            memoryList = best->next;
            memoryList->prev = NULL;
        }
        return (void *)((char *)(best) + kMetadataSize);
      }
    }

  // ! 2. Cant find large enough block -> allocate + search again
  // ! Either 256MB or 512MB
  if (size <= kMemorySize){
      memoryAllocation(kMemorySize);
  }else {
      memoryAllocation(kMaxAllocationSize);
  }
  return searchBlock(size);
}


// ! Internal function to mmap
static void memoryAllocation(size_t size){
      Block * n_alloc     = (Block *) mmap(NULL, kMemorySize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      n_alloc->next       = memoryList->prev = NULL;
      n_alloc->size       = size;
      n_alloc->allocated  = false;
      if (memoryList == NULL){
          memoryList = n_alloc;
      }else{
        n_alloc->next = memoryList;
        memoryList = n_alloc;
      }
}

void *my_malloc(size_t size) {
  if (size <= 0 || size < kMinAllocationSize) 
      return NULL;

  size_t target_size = memAlign(size + kMetadataSize, kAlignment);
  if (target_size > kMaxAllocationSize)
      return NULL;
  if (!memoryList){
      // ! 1st mmap
      size_t alloc_size = target_size > kMemorySize ? kMaxAllocationSize : kMemorySize;
      memoryAllocation(alloc_size);
  }
  return searchBlock(target_size);
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
