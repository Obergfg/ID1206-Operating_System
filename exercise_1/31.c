#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(){

    int pid = getpid();

    char *heap = malloc(20);
    *heap = 0x61;
    printf("heap pointing to: 0x%x\n", *heap);

    free(heap);

    char *foo = malloc(20);

    *foo = 0x62;
    printf("foo pointing to: 0x%x\n", *foo);

    *heap = 0x63;
    printf("or is it pointing to: 0x%x\n", *foo);

    long *heap2 = (unsigned long*)calloc(40, sizeof(unsigned long));

    printf("heap2[2]:   0x%lx\n", heap2[2]);
    printf("heap2[1]:   0x%lx\n", heap2[1]);
    printf("heap2[0]:   0x%lx\n", heap2[0]);
    printf("heap2[-1]:   0x%lx\n", heap2[-1]);
    printf("heap2[-2]:   0x%lx\n", heap2[-2]);

    free(heap2);

    printf("heap2[2]:   0x%lx\n", heap2[2]);
    printf("heap2[1]:   0x%lx\n", heap2[1]);
    printf("heap2[0]:   0x%lx\n", heap2[0]);
    printf("heap2[-1]:   0x%lx\n", heap2[-1]);
    printf("heap2[-2]:   0x%lx\n", heap2[-2]);


    printf("\n\n /proc/%d/maps \n\n", pid);
    char command[50];
    sprintf(command, "cat /proc/%d/maps \n\n", pid);
    system(command);

    return 0;
}