#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#define TRUE 1
#define FALSE 0 
#define ALIGN 8 //Size of an address. 8 bytes.
#define HEAD  (sizeof(struct head))
#define THEAD  (sizeof(struct thead))
#define ARENA (64*1024) // The size of the heap.

#define MIN(size) (((size)>(16))?(size):(16)) // The minimum size of a block. It can not be smaller than 8 bytes.
#define LIMIT(size) (MIN(0) + THEAD + size) // The smallest block we will split i.e. 40 bytes. It is (40 + 8) bytes large(block + theader) which can be divided into two sixteen size blocks((16+8)+(16+8)) 
#define MAGIC(memory) ((struct thead*)memory - 1) // Retreiving the head one head block (24 bytes) behind the block.
#define HIDE(block) (void*)((struct thead* )block + 1) // Hiding the head behind the block of data.


struct head {
  uint16_t bfree;  
  uint16_t bsize;  
  uint16_t free;    
  uint16_t size;   
  struct head *next;   
  struct head *prev;   
};

struct thead{
  uint16_t bfree;  
  uint16_t bsize;  
  uint16_t free;    
  uint16_t size;   
};

struct thead *arena = NULL;  
struct head *flist;

void sanity();
void printArena();
void insert();


struct thead *after(struct thead *block) {
  return (struct thead*)((char*)(block) + THEAD + block->size); 
}

struct thead *before(struct thead *block) {
  return (struct thead*)((char*)(block) -(THEAD + block->bsize));
}

struct head *split(struct thead *block, int size) {

  assert(block->free);
  int rsize = block->size - (THEAD + size); 
  block->size = rsize; 

  struct thead *splt = after(block);
  splt->bsize = block->size;
  splt->bfree = block->free;
  splt->size = size;
  splt->free = TRUE;
  
  struct thead *aft = after(splt);
  aft->bsize = splt->size;
  aft->bfree = splt->free;
  
  splt = (struct head*)splt;

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
  uint size = ARENA - 2*THEAD;
  
  new->bfree = FALSE;
  new->bsize = 0;
  new->free= TRUE;
  new->size = size;
  
  struct thead *sentinel = after(new);

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

 void insert(struct head *block) {

        assert(block->free);

        block->next = flist;
        block->prev = NULL;

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

  int size = MIN(request);
  size = adjust(size);

  struct head *taken = find(size);



    if (taken == NULL)
        return NULL;
    else if(taken == flist && countFlist() == 1)
        return NULL;
    else{
        detach(taken);
        taken = (struct thead*)taken;
        taken->free = FALSE;
        after(taken)->bfree = FALSE;
        sanity();
        return HIDE(taken);
    }
}

struct thead *merge(struct thead *block) {

    
  assert(block->free);
  struct thead *aft = after(block);

  
  if(block->bfree) {
    
    struct thead *bef = before(block);
    assert(bef->free);

    assert(block->bsize == bef->size);

    bef->size += block->size + THEAD;

    aft->bsize = bef->size;

    assert(after(bef)->bsize == bef->size);
  
    detach(bef);
    block = bef;
  }
    
  if(aft->free) {
      assert(aft->free);
      block->size = block->size + aft->size + THEAD;
      after(aft)->bsize = block->size; 
      after(aft)->bfree = TRUE;     
      detach(aft);
  }
  return block;
}


void dfree(void *memory) {

  if(memory != NULL) {

     struct head* iterator = flist;
    
     struct thead *block = MAGIC(memory);

     do{
        if (block == iterator){
          printf("Memory %p has already been freed!\n", block);
          exit(1);
        }
        iterator = iterator->next;

    }while (iterator != NULL);
    
    block->free = TRUE;
    struct thead *aft = after(block);
    aft->bfree = TRUE;

    block = (struct head*)merge(block);

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

void sanity(){

  struct head *iterator = flist;

  if(flist == NULL)
    printArena();

  assert(flist->prev == NULL);

  while (iterator->next != NULL){
    assert(iterator->free);
    assert((iterator->size % ALIGN) == 0);

    iterator = iterator->next;
  }

  iterator = (struct thead*)arena;
  struct thead *next;

  while (iterator->size != 0){

    next = after(iterator);

     if(!(iterator->size == next->bsize)){
         printArena();
    }
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
  flist->next = NULL;
  flist->prev = NULL;
}







#define MINI 16
#define MAX 1024
#define REQUESTS 1000

int *memoryList[REQUESTS];
int TOP = -1;



void push(void *memory){
   memoryList[++TOP] = memory;
}

void* pop(int index){

    void* memory = memoryList[index];
    for(int i = index; i < TOP; i++)
        memoryList[i] = memoryList[i + 1];
      
    TOP--;

    return memory;
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

    srand(time(NULL));
    void *memory;
    int pusher;
    int popper;
    int index;
  
    printf("listsize avgblocksize\n");

    for (size_t i = 0; i < REQUESTS; i++){


        if (i > 40){
          
            pusher = rand() % 100;
            popper = rand() % 100;

            if (pusher < 50)
            {
              memory = dalloc(samples[i]);
              push(memory);
            }

            if(popper < 50 && TOP > 0){
                index = rand() % TOP;
                dfree(pop(index));
            }
            

        }else{
          memory = dalloc(samples[i]);
          push(memory);
       }
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