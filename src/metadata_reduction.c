#include "mymalloc.h"

// Word alignment
const size_t kAlignment = sizeof(size_t);
// Minimum allocation size (1 word)
const size_t kMinAllocationSize = kAlignment;
// Size of unallocated meta-data per free Block
const size_t kMetadataSize = sizeof(Block);
// Size of allocated meta-data per allocated Block
const size_t kAllocMetadataSize = sizeof(size_t);
// Maximum allocation size (512 MB)
const size_t kMaxAllocationSize = (512ull << 20) - kMetadataSize;
// Memory size that is mmapped (256 MB)
const size_t kMemorySize = (256ull << 20);

/* Notes
    Metadata-reduction : 
    1. Use last bit of size field to indicate allocated or not
    2. The next and prev pointer is free to user
*/

// 1. Free block list 
Block * freeList    = NULL;
// 2. mmap region
Arena * mmap_arena  = NULL;

static void memoryAllocation(size_t size);
static void removeNode(Block* b);

// ! Multiple of 256 MB
size_t memAlign(size_t chunk, size_t alignment){
  return (chunk + alignment - 1) & ~(alignment - 1);
}

// ! Traverse free list (Best fit)
void * searchBlock(size_t size){
  if (freeList == NULL)
      return NULL;
  Block * node  = freeList;

  // ! 1. Traverse free list and find best fit blocks
  Block * best = NULL;

  // ! We need to ensure kMetadataSize + Minallocation size since we need the next and prev later
  //   when we free the block

  size_t user_request_size  = size + kAllocMetadataSize;
  size_t minimum_alloc_size = kMinAllocationSize + kMetadataSize;
  size_t required_size      = user_request_size > minimum_alloc_size ? user_request_size : minimum_alloc_size;

  // ! Linear Time to find best fit
  while (node){
      if (GET_SIZE(node) >= required_size){
          if (best){
              if (GET_SIZE(best) > GET_SIZE(node)) 
                  best = node;
          }else best = node;
      }
      node = node->next;
  }
  // ! 1. Find Large Enough Blocks
  if (best){
    if (GET_SIZE(best) > required_size){
          // ! Leftover > Minimum Size (Split)
          size_t leftover = GET_SIZE(best) - required_size;
          if (leftover >= (kMinAllocationSize + kMetadataSize)){
              removeNode(best);

              Block * nBlock    = (Block *)((char *)best + required_size);
              nBlock->size      = leftover;
              nBlock->next      = nBlock->prev = NULL;
              CLEAR_ALLOC_BIT(nBlock);

              if (freeList){
                 nBlock->next   = freeList;
                 freeList->prev = nBlock;
              }
              freeList = nBlock;

              best->size = required_size;
              SET_ALLOC_BIT(best);
              return (void *)((char *)(best) + kAllocMetadataSize);
          }
          // ! LeftOver < Minimum Size (Allocated)
          else{
              removeNode(best);
              SET_ALLOC_BIT(best);
              return (void *)((char *)(best) + kAllocMetadataSize);
          }
      }
      // ! Best Size == Required Size (Perfect)
      else{
        removeNode(best);
        SET_ALLOC_BIT(best);
        return (void *)((char *)(best) + kAllocMetadataSize);
      }
    }

  // ! 2. No match Blocks -> reallocation
  if (size < (kMemorySize - kMetadataSize))
      memoryAllocation(kMemorySize);
  else if (size < (kMaxAllocationSize - kMetadataSize))
      memoryAllocation(kMaxAllocationSize);
  else 
      memoryAllocation((kMaxAllocationSize << 1));
  return searchBlock(size);
}


// ! Internal function to mmap
static void memoryAllocation(size_t size){
      
      Arena * region         = (Arena *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      Block * freeregion     = (Block *) ((char *) region + sizeof(Arena));
      Block * endfence       = (Block *) ((char *) region + size - kMetadataSize);

      // ! 1. mmap_arena 
      region->size         = size;
      if (mmap_arena == NULL){
          mmap_arena       = region;
          mmap_arena->next = NULL;
      }else{
        region->next       = mmap_arena;
        mmap_arena         = region;
      }

      // ! 2. End fence
      endfence->prev         = NULL;
      endfence->next         = NULL;
      endfence->size         = 0;
      SET_ALLOC_BIT(endfence);

      // 3. Free region
      freeregion->size       = size - (kMetadataSize) - sizeof(Arena);
      freeregion->prev       = NULL;
      freeregion->next       = NULL;
      CLEAR_ALLOC_BIT(freeregion);

      if (freeList == NULL){
          freeList = freeregion;
      }else{
          freeregion->next = freeList;
          freeList->prev   = freeregion;
          freeList         = freeregion;
      }
}


void *my_malloc(size_t size) {
  if (size == 0)
      return NULL;

  if (size < kMinAllocationSize)
    size = kMinAllocationSize;
  size_t target_size = memAlign(size, kAlignment);

  if (target_size > kMaxAllocationSize)
      return NULL;
  if (!freeList){
      size_t alloc_size = 0;
      // ! 1. < 256MB
      if (target_size < (kMemorySize - kMetadataSize))
          alloc_size = kMemorySize;
      // ! 2. < 512 MB
      else if (target_size < (kMaxAllocationSize - kMetadataSize))
          alloc_size = kMaxAllocationSize;
      // ! 3. 1 GB
      else alloc_size = (kMaxAllocationSize << 1);
      memoryAllocation(alloc_size);
  }
  return searchBlock(target_size);
}


void coalesce(){
    Arena * arena = mmap_arena;
    while (arena){
        Block * blk = (Block *)((char *)arena + sizeof(Arena));
        while (GET_SIZE(blk) != 0){
            Block * r_blk = (Block *)((char *)blk + GET_SIZE(blk));
            if (GET_SIZE(r_blk) == 0) 
                break;
            if (is_free(blk) && is_free(r_blk)){
                removeNode(blk);
                removeNode(r_blk);

                blk->size += GET_SIZE(r_blk);
                blk->next = freeList;
                blk->prev = NULL;
                if (freeList) 
                    freeList->prev = blk;
                freeList  = blk;
                continue;
            }
            blk = r_blk;
        }
        arena = arena->next;
    }
}

void my_free(void *ptr) {
    if (!ptr) 
        return;
    if (((size_t) ptr) & (kAlignment -1))
        return;
    if (mmap_arena == NULL)
        return;

    Block * m_data = (Block *)((char *)ptr - kAllocMetadataSize);
    if (is_free(m_data))
        return;

    // ! 1. Insert the free block into head of the free list
    CLEAR_ALLOC_BIT(m_data);

    m_data->prev      = NULL;
    m_data->next      = NULL;

    // ! 2. Check free list empty
    if (freeList){
        freeList->prev   = m_data;
        m_data->next     = freeList;
    }
    freeList = m_data;
    
    // ! 3. Linear Coelasce
    coalesce();
    return;
}


/** These are helper functions you are required to implement for internal testing
 *  purposes. Depending on the optimisations you implement, you will need to
 *  update these functions yourself.
 **/

/* Returns 1 if the given block is free, 0 if not. */
int is_free(Block *block) {
  return (block->size & 1) == 0;
}

/* Returns the size of the given block */
size_t block_size(Block *block) {
  return GET_SIZE(block);
}

/* Returns the first block in memory (excluding fenceposts) */
Block *get_start_block(void) {
    if (!mmap_arena) return NULL;
    return (Block *)((char *)mmap_arena + sizeof(Arena));
}

/* Returns the next block in memory */
Block *get_next_block(Block *block) {
    if (!block) return NULL;

    Block * next_block = (Block *) ((char *) block + GET_SIZE(block));
    
    if (GET_SIZE(next_block) == 0){
        Arena * arena = mmap_arena;
        while (arena){
            size_t a_start = (size_t)arena;
            size_t a_end   = a_start + arena->size;
            size_t block_addr = (size_t) block;
            if (block_addr >= a_start && block_addr < a_end){
                Arena * next_arena = arena->next;
                if (next_arena) 
                    return (Block *) ((char *) next_arena + sizeof(Arena));
                return NULL;
            }
            arena = arena->next;
        }
    }
    return next_block;
}

/* Given a ptr assumed to be returned from a previous call to `malloc`,
   return a pointer to the start of the metadata block. */
Block *ptr_to_block(void *ptr) {
  return ADD_BYTES(ptr, -((ssize_t) kAllocMetadataSize));
}

static void removeNode(Block* b) {
    if (b->prev) b->prev->next = b->next;
    if (b->next) b->next->prev = b->prev;
    if (b == freeList) {
        freeList = b->next;
        if (freeList) freeList->prev = NULL;
    }
    b->next = b->prev = NULL;
}