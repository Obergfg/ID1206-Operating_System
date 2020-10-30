#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>

int main(){

    int p = getpid();

    printf("Mothers pid: %d\n", p);
    
    int pid = fork();

    if(0 == pid){
        printf("I'm the child %d\n", getpid());
        sleep(1);
    }
    else{
        printf("My child is called %d\n", pid);
        wait(NULL);
        printf("My child is terminated \n");
    }


    printf("The end of the process: %d\n", getpid());
    


    return 0;
}