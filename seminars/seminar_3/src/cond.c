#include <stdio.h>
#include "green.h"

int flag = 0;
green_cond_t cond;

void *test(void *arg)
{
    int id = *(int *)arg;
    int loops = 4;
    while (loops > 0)
    {
        if (flag == id)
        {
            printf("thread %d: %d\n", id, loops);
            loops--;
            flag = (id + 1) % 3;
            green_cond_signal(&cond);
        }
        else
        {
            green_cond_wait(&cond);
        }
    }
}

int main()
{
    green_t g0, g1, g2;

    int a0 = 0;
    int a1 = 1;
    int a2 = 2;

    green_cond_init(&cond);
    green_create(&g0, test, &a0);
    green_create(&g1, test, &a1);
    green_create(&g2, test, &a2);

    green_join(&g0, NULL);
    green_join(&g1, NULL);
    green_join(&g2, NULL);
    
    printf("done\n");

    return 0;
}
