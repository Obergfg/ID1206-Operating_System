#include <stdio.h>
#include "green.h"

green_cond_t cond;
int loops = 1000000;

void *test(void *arg){

    int* id = (int*)arg;
    int increments = 0;

    while (loops > 0)
    {
        loops--;
        increments++;
    }

    printf("Thread %d did %d number of increments!\n",*id, increments);
}

int main()
{
    green_t g0, g1, g2;

    int a0 = 0;
    int a1 = 1;
    int a2 = 2;
    interrupts = 0;

    green_cond_init(&cond);

    green_create(&g0, test, &a0);
    green_create(&g1, test, &a1);

    green_join(&g0, NULL);
    green_join(&g1, NULL);
 
    printf("Number of interrupts %d\n", interrupts);
    printf("done\n");

    return 0;
}
