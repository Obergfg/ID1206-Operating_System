#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "smalldlmalloc.h"

#define MINI 8
#define MAX 1024
#define REQUESTS 1000
#define LOOP 10000

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