#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define TRUE 1
#define FALSE 0 
#define ALIGN 8 //Size of an adress. 8 bytes.
#define HEAD  (sizeof(struct head))
#define ARENA (64*1024) // The size of the heap.

#define MIN(size) (((size)>(8))?(size):(8)) // The minimum size of a block. It can not be smaller than 8 bytes.
#define LIMIT(size) (MIN(0) + HEAD + size) // The smallest block we will split i.e. 40 bytes. It is 40 + 24 bytes large(block + header) which can be divided into two eight size blocks((24+8)+(24+8)) 
#define MAGIC(memory) ((struct head*)memory - 1) // Retreiving the head one head block (24 bytes) behind the block.
#define HIDE(block) (void*)((struct head* )block + 1) // Hiding the head behind the block of data.


struct head {
  uint16_t bfree;  
  uint16_t bsize;  
  uint16_t free;    
  uint16_t size;   
  struct head *next;   
  struct head *prev;   
};

struct head *arena = NULL;  
struct head *flist;

struct head *after(struct head *block) {
  return (struct head*)((char*)(block) + HEAD + block->size); 
}

struct head *before(struct head *block) {
  return (struct head*)((char*)(block) -(HEAD + block->bsize));
}

int countFlist(){

  int size = 1;

  struct head *iterator = flist;

  while (iterator->next != NULL){
    size++;
    iterator = iterator->next;
  }
  
  return size;
}

void printArena(){

  struct head *iterator = arena;

  int index = 0;

  printf("\nComplete Arena\n\n");

  do{
    printf("Index %d, address %p,free %d, bfree %d, bsize %d, size %d\n",index, iterator,iterator->free, iterator->bfree, iterator->bsize, iterator->size);
    iterator = after(iterator);
    index++;

  }while(iterator->size != 0);

  printf("Index %d, address %p, bfree %d, bsize %d, size %d\n",index, iterator, iterator->bfree, iterator->bsize, iterator->size);
}

 void insert(struct head *block) {

        assert(block->free);

        block->next = flist;
        block->prev = NULL;

        if (flist != NULL)
          flist->prev = block;
        
      flist = block;
} 

struct head *split(struct head *block, int size) {

  assert(block->free);
  int rsize = block->size - (HEAD + size); 
  block->size = rsize; 

  struct head *splt = after(block);
  splt->bsize = block->size;
  splt->bfree = block->free;
  splt->size = size;
  splt->free = TRUE;
  
  struct head *aft = after(splt);
  aft->bsize = splt->size;
  aft->bfree = splt->free;

  insert(splt);

  return splt;
}

void sanity(){

  struct head *iterator = flist;

  assert(flist->prev == NULL);

  while (iterator->next != NULL){
    assert(iterator->free);
    assert((iterator->size % ALIGN) == 0);

    iterator = iterator->next;
  }

  iterator = arena;
  struct head *next;

  while (iterator->size != 0){

    next = after(iterator);

    
    assert(iterator->size == next->bsize);
    assert(iterator->free == next->bfree);
    assert((iterator->size % ALIGN) == 0);
    assert((iterator->bsize % ALIGN) == 0);
    assert((next->size % ALIGN) == 0);
    assert((next->bsize % ALIGN) == 0);

    iterator = next;
  }
}

struct head *new() {

  if(arena != NULL) {
    printf("one arena already allocated \n");
    return NULL;
  }

  // using mmap, but could have used sbrk
  struct head *new = mmap(NULL, ARENA,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if(new == MAP_FAILED) {
    printf("mmap failed: error %d\n", errno);
    return NULL;
  }

  /* make room for head and dummy */
  uint size = ARENA - 2*HEAD;
  
  new->bfree = FALSE;
  new->bsize = 0;
  new->free= TRUE;
  new->size = size;
  
  struct head *sentinel = after(new);

 /* only touch the status fields */
  sentinel->bfree = TRUE;
  sentinel->bsize = new->size;
  sentinel->free = FALSE;
  sentinel->size = 0;

  /* this is the only arena we have */
  arena = (struct head*)new;

  return new;
} 

  
void detach(struct head *block) {

      if (block->next != NULL)
          block->next->prev = block->prev;
        
      if (block->prev != NULL)
          block->prev->next = block->next;
      else
          flist = block->next;
                                                          
}



int adjust(int request){

    while ((request % ALIGN) != 0)
      request++;
    
  return request;
}

struct head *find(int size){

  if(size <= 0)
    return NULL;

  if (flist == NULL)
    return NULL;

  struct head *iterator = flist;

  do{

      if (iterator->size == size){
          return iterator;
     }

      if (iterator->size >= LIMIT(size))   
         return split(iterator, size);
       
      iterator = iterator->next;

  }while(iterator !=  NULL);

  return NULL;
  
}

void *dalloc(size_t request) {

  if( request <= 0 )
    return NULL;

  int size = MIN(request);
  size = adjust(size);

  struct head *taken = find(size);

  if (taken == NULL)
    return NULL;
  else if(taken == flist && countFlist() == 1)
    return NULL;
  else{
    detach(taken);
    taken->free = FALSE;
    after(taken)->bfree = FALSE;
    sanity();
    return HIDE(taken);
  }
}

struct head *merge(struct head *block) {

  assert(block->free);
  struct head *aft = after(block);

  
  if(block->bfree) {
    
    struct head *bef = before(block);
    assert(bef->free);

    assert(block->bsize == bef->size);

    bef->size += block->size + HEAD;

    aft->bsize = bef->size;

    assert(after(bef)->bsize == bef->size);
  
    detach(bef);
    block = bef;
  }
    
  if(aft->free) {
      assert(aft->free);
      block->size = block->size + aft->size + HEAD;
      after(aft)->bsize = block->size; 
      after(aft)->bfree = TRUE;     
      detach(aft);
  }
  return block;
}


void dfree(void *memory) {

  if(memory != NULL) {

     struct head* iterator = flist;
    
     struct head *block = MAGIC(memory);

     do{
        if (block == iterator){
          printf("Memory %p has already been freed!\n", block);
          exit(1);
        }
        iterator = iterator->next;

     }while (iterator != NULL);
    
    block->free = TRUE;
    struct head *aft = after(block);
    aft->bfree = TRUE;

    block = merge(block);

    assert(block->free);
  
    aft->bsize = block->size;
    
    insert(block);
  }

  sanity();

  return;
}

void printFlist(){

  
  struct head *iterator = flist;

  int index = 0;

  printf("\nComplete free list\n\n");

  do{
    printf("Index %d, address %p, free %d, bfree %d, bsize %d, size %d, next %p, prev %p\n",index, iterator,iterator->free ,iterator->bfree, iterator->bsize, iterator->size, iterator->next, iterator->prev);
    
    iterator = iterator->next;
    index++;

  }while(iterator != NULL);

}

double countAverageBlockSize(){

  double size = 0;

  struct head *iterator = flist;

  do{

    size += iterator->size;
    iterator = iterator->next;

  }while (iterator != NULL);
    
  
  
  return size/countFlist();
}

void init(){
  flist = new();
}







