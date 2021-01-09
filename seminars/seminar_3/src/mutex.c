#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "green.h"

green_mutex_t mutex;
green_cond_t cond;
green_t *greens;
int loops;
int threads;
int flag = 0;

void *test(void *arg)
{

    int id = *(int *)arg;
    int increments = 0;

    while (loops > 0)
    {
        green_mutex_lock(&mutex);

        while (flag != id)
            green_cond_wait(&cond, &mutex);

        flag = (id + 1) % threads;
      //  printf("thread %d: %d\n", id, loops);
        increments++;
        loops--;
        green_cond_broadcast(&cond);
        //green_cond_signal(&cond);
        green_mutex_unlock(&mutex);
    }

    printf("Thread %d did %d number of increments!\n", id, increments);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Argument count is not 3!\n");
        exit(0);
    }

    loops = atoi(argv[1]);
    threads = atoi(argv[2]);

    interrupts = 0;

    printf("Number of loops %d\n", loops);
    printf("Number of threads %d\n", threads);

    green_mutex_init(&mutex);
    green_cond_init(&cond);

    greens = malloc(threads * sizeof(green_t));
    int *arg = malloc(threads * sizeof(int));

    for (int i = 0; i < threads; i++)
    {
        arg[i] = i;
        green_create(&greens[i], test, &arg[i]);
    }

    for (size_t i = 0; i < threads; i++)
        green_join(&greens[i], NULL);

    // green_t g0, g1, g2, g3;

    // int a0 = 0;
    // int a1 = 1;
    // int a2 = 2;
    // int a3 = 3;
    // interrupts = 0;
 
    // green_cond_init(&cond);
    // green_mutex_init(&mutex);

    // green_create(&g0, test, &a0);
    // green_create(&g1, test, &a1);
    // green_create(&g2, test, &a2);
    // green_create(&g3, test, &a3);

    // green_join(&g0, NULL);
    // green_join(&g1, NULL);
    // green_join(&g2, NULL);
    // green_join(&g3, NULL);

    printf("Number of interrupts %d\n", interrupts);
    printf("done\n");

    return 0;
}
