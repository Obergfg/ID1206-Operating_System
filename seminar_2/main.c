#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "dlmalloc.h"




#define MIN 8
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

        while (samples[i] % MIN != 0)
            samples[i]++;    
    }
}

void bench(int *samples){

    void *memory;
  
    printf("listsize avgblocksize\n");

    for (size_t i = 0; i < REQUESTS; i++){

        memory = dalloc(samples[i]);
   
        push(memory);
        dfree(memory);

       // if((samples[i] % 100 < 60) && TOP != -1){dfree(memory);}
           // dfree(pop());
        

        printFlist();
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