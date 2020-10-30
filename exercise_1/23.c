#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void zot(unsigned long *stop, long a,long b, long c, long d){

    unsigned long r = 0x789;

    unsigned long *i;

    for(i = &r; i <= stop; i++)
        printf(" %p     0x%lx\n", i, *i);
    
}

void foo(unsigned long *stop, long a, long b,long c,long d, long e, long f, long g, long h){
    unsigned long q=0x456;


    zot(stop, 0x12,0x13,0x14,0x15);
    
    foo:
    printf("  foo:  %p \n", &&foo);
}


int main() {

    //int pid = getpid();

    unsigned long p = 0x123;

    foo(&p, 2,3,4,5,6,7,8,9);

    back:
    printf("  p:    %p \n", &p);
    printf("  back: %p \n", &&back);
    /*
    printf("\n\n /proc/%d/maps \n\n", pid);
    char command[50];
    sprintf(command, "cat /proc/%d/maps \n\n", pid);
    system(command);
*/
    return 0;
}
