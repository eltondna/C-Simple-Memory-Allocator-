#include "metadata_reduction.h"

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

/* Notes
    This is the metadata-reduction + constant coelesce
*/

// 1. Free block list 
Block * freeList    = NULL;
// 2. mmap region
Arena * mmap_arena  = NULL;

static void memoryAllocation(size_t size);
static void removeNode(Block* b);
static void replaceNode(Block * o_node, Block* n_node);

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

  // ! Include the Metadata size
  size_t required_size = size + kMetadataSize;

  // ! Linear Time to find best fit
  while (node){
      if (node->size >= required_size){
          if (best){
              if (best->size > node->size) 
                  best = node;
          }else best = node;
      }
      node = node->next;
  }
  // ! 1. Find Large Enough Blocks
  if (best){
    if (best->size > required_size){
          // ! Leftover > Minimum Size (Split)
          size_t leftover = best->size - required_size;
          if (leftover >= (kMinAllocationSize + kMetadataSize)){
              Block * nBlock    = (Block *)((char *)best + required_size);
              nBlock->size      = leftover;
              SET_ALLOC_BIT(nBlock);
              best->size        = required_size;

              // ！ replace node
              replaceNode(best,nBlock);
              SET_ALLOC_BIT(best);
              return (void *)((char *)(best) + kMetadataSize);
          }
          // ! LeftOver < Minimum Size (Allocated)
          else{
              removeNode(best);
              SET_ALLOC_BIT(best);
              return (void *)((char *)(best) + kMetadataSize);
          }
      }
      // ! Best Size == Required Size (Perfect)
      else{
        removeNode(best);
        SET_ALLOC_BIT(best);
        return (void *)((char *)(best) + kMetadataSize);
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
      CLEAR_ALLOC_BIT(freeregion);
      freeregion->prev       = NULL;
      freeregion->next       = NULL;

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
      // ! 3. 512 MB
      else alloc_size = (kMaxAllocationSize << 1);
      memoryAllocation(alloc_size);
  }
  return searchBlock(target_size);
}


void coalesce(){
    Arena * arena = mmap_arena;
    while (arena){
        Block * blk = (Block *)((char *)arena + sizeof(Arena));
        while (blk->size != 0){
            Block * r_blk = (Block *)((char *)blk + blk->size);
            if (r_blk->size == 0) 
                break;
            if (is_free(blk) && is_free(r_blk)){
                removeNode(blk);
                removeNode(r_blk);

                blk->size += r_blk->size;
                blk->next = freeList;
                blk->prev = NULL;
                CLEAR_ALLOC_BIT(blk);

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

    Block * m_data = (Block *)((char *)ptr - kMetadataSize);
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
  return ((size_t) block & 1) > 0;
}

/* Returns the size of the given block */
size_t block_size(Block *block) {
  return block->size;
}

/* Returns the first block in memory (excluding fenceposts) */
Block *get_start_block(void) {
    if (!mmap_arena) return NULL;
    return (Block *)((char *)mmap_arena + sizeof(Arena));
}

/* Returns the next block in memory */
Block *get_next_block(Block *block) {
    if (!block) return NULL;

    Block * next_block = (Block *) ((char *) block + block->size);
    
    if (next_block->size == 0){
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

/* Given a ptr assumed to be returned from a previous call to `alloc`,
   return a pointer to the start of the metadata block. */
Block *ptr_to_block(void *ptr) {
  return ADD_BYTES(ptr, -((ssize_t) kMetadataSize));
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

static void replaceNode(Block * o_node, Block* n_node){
  if (!o_node || !n_node) return;

  if (o_node->prev) {
      o_node->prev->next = n_node;
      n_node->prev       = o_node->prev;
  }
  if (o_node->next) {
      o_node->next->prev = n_node;
      n_node->next       = o_node->next;
  } 

  if (o_node == freeList){
      freeList       = n_node;
      freeList->prev = NULL;
  }
  o_node->prev = o_node->next = NULL;
}