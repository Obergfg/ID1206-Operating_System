#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "smalldlmalloc.h"

#define MINI 8
#define MAX 1024
#define REQUESTS 1000
#define LOOP 10000


void timer(){

    int * list[REQUESTS];
    int value = 123;

    printf("normal\n");

    for(int i = 0; i < REQUESTS; i++)
        list[i] = NULL;

    for(int i = 0; i < REQUESTS; i++)
        list[i] = dalloc(16);

      for(int k = 0; k < 100; k++){
        clock_t begin = clock();

        for(int i = 0; i < LOOP; i++)
          for(int j = 0; j < REQUESTS; j++)
              *list[j] = value;

        clock_t end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    
      
          printf("%f\n", time_spent);
    
        }
}


int main(){

    init();  
    timer();
           
    return 0;
}