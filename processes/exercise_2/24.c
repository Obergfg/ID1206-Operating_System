#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>

int main(){

    int pid;
    int x = 123;
    pid = fork();

    if(0 == pid){
        printf("   child:   x is %d and the adress is 0x%p\n", x, &x);
        x=42;
        sleep(1);
        printf("   child:   x is %d  and the adress is 0x%p\n", x, &x);
        return 42;
    }
    else{
        printf("   mother:   x is %d  and the adress is 0x%p\n", x, &x);
        x = 13;
        sleep(1);
        printf("   mother:   x is %d  and the adress is 0x%p\n", x, &x);
        wait(NULL);
    }

    return 0;
}