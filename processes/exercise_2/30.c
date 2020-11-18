#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>


int main(){

    int pid = fork();

    if(0 == pid){
        printf("I am the child %d with parent %d\n",getpid(), getppid());
    }
    else{
        printf("I am the parent %d with parent %d\n",getpid(), getppid());
        wait(NULL);
    }
    
    system("ps a");
    return 0;
}