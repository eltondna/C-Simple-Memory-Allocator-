#include "mymalloc.h"

// Word alignment
const size_t kAlignment = sizeof(size_t);
// Minimum allocation size (1 word)
const size_t kMinAllocationSize = kAlignment;
// Size of unallocated meta-data per free Block
const size_t kMetadataSize = sizeof(Block);
// Size of allocated meta-data per allocated Block
const size_t kAllocMetadataSize = sizeof(Tag_t);
// Maximum allocation size (512 MB)
const size_t kMaxAllocationSize = (512ull << 20) - kMetadataSize;
// Memory size that is mmapped (256 MB)
const size_t kMemorySize = (256ull << 20);

/*  Notes
    Constant-time Coelesce
*/

// 1. Free block list 
Block * freeList    = NULL;
// 2. mmap region
Arena * mmap_arena  = NULL;

static void memoryAllocation(size_t size);
static void removeNode(Block* b);
static void insert_bound_tag(Block * node);
static Block * Left_Coalesce(Block * node);
static Block * Right_Coalesce(Block * node);

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

  // ! We need to ensure kMetadataSize + Minallocation size since 
  //   we need the next and prev later when we free the block

  size_t user_request_size  = size + (kAllocMetadataSize << 1);
  size_t minimum_alloc_size = kMinAllocationSize + kMetadataSize;
  size_t required_size      = user_request_size > minimum_alloc_size ? user_request_size : minimum_alloc_size;

  // ! Linear Time to find best fit
  while (node){
      if (block_size(node) >= required_size){
          if (best){
              if (block_size(best) > block_size(node)) 
                  best = node;
          }else best = node;
      }
      node = node->next;
  }
  // ! 1. Find Large Enough Blocks
  if (best){
    if (block_size(best) > required_size){
          // ! Leftover > Minimum Size (Split)
          size_t leftover = block_size(best) - required_size;
          if (leftover >= (minimum_alloc_size)){
              removeNode(best);
            
              // ! Leftover block (New Block: insert tags)
              Block * nBlock    = (Block *)((char *)best + required_size);
              nBlock->size      = leftover;
              nBlock->next      = nBlock->prev = NULL;
              CLEAR_ALLOC_BIT(nBlock);
              insert_bound_tag(nBlock);
              
              if (freeList){
                 nBlock->next   = freeList;
                 freeList->prev = nBlock;
              }
              freeList = nBlock;
              
              // ! Allocated Block 
              best->size = required_size;
              SET_ALLOC_BIT(best);
              insert_bound_tag(best);

              return (void *)((char *)(best) + kAllocMetadataSize);
          }
          // ! LeftOver < Minimum Size (Allocate all)
          else{
              removeNode(best);
              SET_ALLOC_BIT(best);
              insert_bound_tag(best);
              return (void *)((char *)(best) + kAllocMetadataSize);
          }
      }
      // ! Best Size == Required Size (Perfect)
      else{
        removeNode(best);
        SET_ALLOC_BIT(best);
        insert_bound_tag(best);
        return (void *)((char *)(best) + kAllocMetadataSize);
      }
    }

  // ! 2. No match Blocks -> reallocation
  if (required_size < (kMemorySize - kMetadataSize))
      memoryAllocation(kMemorySize);
  else if (required_size < (kMaxAllocationSize - kMetadataSize))
      memoryAllocation(kMaxAllocationSize);
  else 
      return NULL;
  return searchBlock(size);
}

// ! Internal function to mmap
static void memoryAllocation(size_t size){
      
      Arena * region         = (Arena *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
      Block * startfence     = (Block *) ((char *) region + sizeof(Arena));
      Block * endfence       = (Block *) ((char *) region + size - kMetadataSize);
      Block * freeregion     = (Block *) ((char *) startfence + kMetadataSize);

      // ! 1. mmap_arena 
      region->size         = size;
      if (mmap_arena == NULL){
          mmap_arena         = region;
          mmap_arena->next   = NULL;
      }else{
        region->next         = mmap_arena;
        mmap_arena           = region;
      }
      // ! 2. Start fence
      startfence->prev       = NULL;
      startfence->next       = NULL;
      startfence->size       = kMetadataSize;
      SET_ALLOC_BIT(startfence);
      insert_bound_tag(startfence);


      // ! 3. End fence
      endfence->prev         = NULL;
      endfence->next         = NULL;
      endfence->size         = kMetadataSize;
      SET_ALLOC_BIT(endfence);
      insert_bound_tag(endfence);


      // 4. Free region
      freeregion->size       = size - (kMetadataSize << 1) - sizeof(Arena);
      freeregion->prev       = NULL;
      freeregion->next       = NULL;
      CLEAR_ALLOC_BIT(freeregion);
      insert_bound_tag(freeregion);

      if (freeList != NULL){
          freeregion->next = freeList;
          freeList->prev   = freeregion;
      }
      freeList             = freeregion; 
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

// ! O(1) coalesce
void coalesce(Block * node){
    if (block_size(node) <= kMetadataSize)
        return;
    Arena * arena = mmap_arena;
    if (arena == NULL){
        printf("[Error]: Arena is NULL. This should not happen.\n");
    }

    char * n_addr = (char *) node;
    int flag      = 0;

    while (arena){
        char* start_addr = (char*) arena;
        char* end_addr   = ((char*) arena) + arena->size;
        if (n_addr > start_addr && n_addr < end_addr){
            flag = 1;
            break;
        }
        arena = arena->next;
    }
    if (!flag) {
        printf("[Coalesce]: Should be an Invalid address\n");
        return;
    }
    
    Block * n = Left_Coalesce(node);
    if (n == NULL){
        if (freeList){
            node->next = freeList;
            freeList->prev = node;
        }
        freeList = node;
        Right_Coalesce(node);
    }
    else Right_Coalesce(n);
    return;
}

Block * Left_Coalesce(Block * node){
    if (block_size(node) <= kMetadataSize)
        return NULL;
    if (!is_free(node)){
        printf("[Left Coalesce]: There should be some error, node should already be freed\n");
        return NULL;
    }
    Tag_t * L_Blk_Tag   = (Tag_t *)((char *) node - sizeof(Tag_t));
    size_t  L_Blk_Size  = block_size((Block *) L_Blk_Tag);
    Block * L_Blk       = (Block *)((char *) node - L_Blk_Size);
    // ! Fence Block
    if (block_size(L_Blk) <= kMetadataSize)
        return NULL;

    if (!is_free(L_Blk))
        return NULL;

    removeNode(node);
    removeNode(L_Blk);
    node->next         = node->prev = NULL;
    L_Blk->next        = L_Blk->prev = NULL;
    L_Blk->size        += node->size;
    insert_bound_tag(L_Blk);
    if (freeList){
        L_Blk->next    = freeList;
        freeList->prev = L_Blk;
    }
    freeList           = L_Blk;
    return L_Blk;
}

Block * Right_Coalesce(Block * node){
    if (node->size <= kMetadataSize)
        return node;
    if (!is_free(node)){
        printf("[Right coalesce]: There should be some error, node should already be freed\n");
        return node;
    }
    Block* R_Blk = (Block *) ((char *)node + node->size);
    if (block_size(R_Blk) <= kMetadataSize)
        return node;

    if (is_free(R_Blk)){
        removeNode(R_Blk);
        removeNode(node);
        node->next  = node->prev  = NULL;
        R_Blk->next = R_Blk->prev = NULL;
        node->size  += R_Blk->size;
        insert_bound_tag(node);
        if (freeList){
            node->next = freeList;
            freeList->prev = node;
        }
        freeList = node;
    }
    return node;
}

void my_free(void *ptr) {
    if (!ptr) 
        return;
    if (((size_t) ptr) & (kAlignment -1))
        return;
    if (mmap_arena == NULL)
        return;

    Block * m_data = (Block *)((char *) ptr - kAllocMetadataSize);
    if (is_free(m_data))
        return;

    if (block_size(m_data) <= kMetadataSize)
        return;
    m_data->next = m_data->prev = NULL;
    CLEAR_ALLOC_BIT(m_data);
    insert_bound_tag(m_data);
    // ! 3. Linear Coelasce
    coalesce(m_data);
 
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
    return (Block *)((char *)mmap_arena + sizeof(Arena) + kMetadataSize);
}

/* Returns the next block in memory */
Block *get_next_block(Block *block) {
    if (!block) return NULL;

    Block * next_block = (Block *) ((char *) block + block_size(block));
    
    if (GET_SIZE(next_block) <= kMetadataSize){
        Arena * arena = mmap_arena;
        while (arena){
            size_t a_start = (size_t)arena;
            size_t a_end   = a_start + arena->size;
            size_t block_addr = (size_t) block;
            if (block_addr >= a_start && block_addr < a_end){
                Arena * next_arena = arena->next;
                if (next_arena) 
                    return (Block *) ((char *) next_arena + sizeof(Arena) + kMetadataSize);
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
    if (!b) return;

    if (b->prev) b->prev->next = b->next;
    if (b->next) b->next->prev = b->prev;
    if (b == freeList) {
        freeList = b->next;
        if (freeList) freeList->prev = NULL;
    }
    b->next = b->prev = NULL;
}

static void insert_bound_tag(Block * node){
    size_t size    = block_size(node);
    Tag_t * Header = (Tag_t *) node;
    *Header = node->size;

    Tag_t * Footer = (Tag_t *)((char *) node + size - sizeof(Tag_t));
    *Footer = node->size;
}