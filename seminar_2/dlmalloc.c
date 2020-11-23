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

void sanity();
void printArena();
void insert();


struct head *after(struct head *block) {
  return (struct head*)((char*)(block) + sizeof(struct head) + block->size); 
}

struct head *before(struct head *block) {
  return (struct head*)((char*)(block) -(sizeof(struct head) + block->bsize));
}

struct head *split(struct head *block, int size) {

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
  new->prev = NULL;
  
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

    if(block != flist){

        block->next = flist;
        block->prev = flist->prev;

        if (flist != NULL)
          flist->prev = block;
        
          
        flist = block;
    }
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

      if (iterator->size == size)
          return iterator;
     
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

  sanity();

  if (taken == NULL)
    return NULL;
  else{
    detach(taken);
    taken->free = FALSE;
    after(taken)->bfree = FALSE;
    return HIDE(taken);
  }
}

struct head *merge(struct head *block) {

  assert(block->free);
  struct head *aft = after(block);
  if (block->bfree) {
    /* unlink the block before */
    
    struct head *bef = before(block);
    assert(bef->free);
    detach(block);
    assert(block->bsize == bef->size);

    /* calculate and set the total size of the merged blocks */    
    bef->size += block->size + HEAD;

    /* update the block after the merged blocks*/
    aft->bsize = bef->size;

    assert(after(bef)->bsize == bef->size);
    /* continue with the merged block */
    block = bef;
  }
    
  if(aft->free) {
    /* unlink the block  */
      detach(aft);
    /* calculate and set the total size of merged blocks */
      block->size = block->size + aft->size + HEAD;
    /* update the block after the merged block */
      after(aft)->bsize = block->size;     
  }
  return block;
}


void dfree(void *memory) {

  if(memory != NULL) {

    struct head *block = MAGIC(memory);

    block->free = TRUE;
    block = merge(block);

    struct head *aft = after(block);
  
    aft->bfree = TRUE;
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
    printf("Index %d, address %p, bfree %d, bsize %d, size %d, next %p, prev %p\n",index, iterator, iterator->bfree, iterator->bsize, iterator->size, iterator->next, iterator->prev);
    
    iterator = iterator->next;
    index++;

  }while(iterator != NULL);

  sanity();

}

void printArena(){

  struct head *iterator = arena;

  int index = 0;

  printf("\nComplete Arena\n\n");

  do{
    printf("Index %d, address %p, bfree %d, bsize %d, size %d\n",index, iterator, iterator->bfree, iterator->bsize, iterator->size);
    iterator = after(iterator);
    index++;

  }while(iterator->size != 0);

  printf("Index %d, address %p, bfree %d, bsize %d, size %d\n",index, iterator, iterator->bfree, iterator->bsize, iterator->size);
  sanity();

}

void sanity(){

  struct head *iterator = flist;

  assert(flist->prev == NULL);

  while (iterator->next != NULL){
    assert(iterator->free == TRUE);
    assert(iterator->next->prev == iterator);
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

int countFlist(){

  int size = 1;

  struct head *iterator = flist;

  while (iterator->next != NULL){
    size++;
    iterator = iterator->next;
  }
  
  return size;
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




#include <time.h>
//#include "dlmalloc.h"




#define MINI 8
#define MAX 1024
#define REQUESTS 10



int *stack[REQUESTS];
int TOP = -1;



void push(void *memory){
   stack[++TOP] = memory;
}

void* pop(){
    return stack[TOP--];
}

void requests (int *samples){

    srand(time(NULL));

    for (size_t i = 0; i < REQUESTS; i++){
        samples[i] = rand() % MAX;

        while (samples[i] % MINI != 0)
            samples[i]++;    
    }
}

void bench(int *samples){

    void *memory;
  
    printf("listsize avgblocksize\n");

    for (size_t i = 0; i < REQUESTS; i++){

        memory = dalloc(samples[i]);
   
        push(memory);
       // dfree(memory);

       if((samples[i] % 100 < 90) && TOP != -1)
           dfree(pop());
        

        printFlist();
        printArena();
        printf("%d %.2f\n", countFlist(), countAverageBlockSize());
    }

}



int main(){

    init();
    int req[REQUESTS]; 
    requests(req);  
    bench(req);
         
    return 0;
}