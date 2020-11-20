#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

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
  return (struct head*)((char*)(block) + sizeof(struct head) + block->size); 
}

struct head *before(struct head *block) {
  return (struct head*)((char*)(block) -(sizeof(struct head) + block->bsize));
}

struct head *split(struct head *block, int size) {

  int rsize = block->size - (sizeof(struct head) + size); 
  block->size = rsize; 

  struct head *splt = after(block);
  splt->bsize = block->size;
  splt->bfree = block->free;
  splt->size = size;
  splt->free = FALSE;
  
  struct head *aft = after(splt);
  aft->bsize = splt->size;

  return splt;
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
  sentinel->bfree = new->free;
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

 void insert(struct head *block) {

  block->next = flist;
  block->prev = flist->prev;

  if (flist != NULL)
    flist->prev = block;
  
     
  flist = block;
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
      if (iterator->size >= LIMIT(size))   
         return split(iterator, size);
       
      iterator = iterator->next;

  }while(iterator !=  NULL);

  return NULL;
  
}

void *dalloc(size_t request) {

  if( request <= 0 )
    return NULL;
  
  int size = adjust(request);

  struct head *taken = find(size);

  if (taken == NULL)
    return NULL;
  else
    return HIDE(taken);

}


void dfree(void *memory) {

  if(memory != NULL) {

    struct head *block = MAGIC(memory);

    struct head *aft = after(block);
  
    block->free = TRUE;
    aft->bfree = TRUE;
    aft->bsize = block->size;
    
    insert(block);
  }

  return;
}

void printFlist(){

  struct head *iterator = flist;

  int index = 0;

  printf("Complete free list\n\n");

  do{
    printf("Index %d, address %p, bfree %d, bsize %d, size %d, next %p, prev %p\n",index, iterator, iterator->bfree, iterator->bsize, iterator->size, iterator->next, iterator->prev);
    
    iterator = iterator->next;
    index++;

  }while(iterator != NULL);

}



int main(){

  flist = new();

  int size = 100;

  
  
  printFlist();

  

  void* memory = dalloc(100);
  void* memory2 = dalloc(10000);

  dfree(memory);
  dfree(memory2);

  printFlist();



  struct head *test = split(arena, 100);
  struct head *afterTest = after(test);

  assert(MIN(0) == 8);
  assert(MIN(9) == 9);
  assert(LIMIT(0) == 32);
  assert(LIMIT(8) == 40);
  assert(test->size == size);
  assert(test->free == FALSE);
  assert(afterTest->bsize == test->size);
  assert(adjust(5) == 8);
  assert(adjust(17) == 24);
  assert(adjust(123) == 128);




  printf("\nAll assertions passed!!\n");

  return 0;
}
