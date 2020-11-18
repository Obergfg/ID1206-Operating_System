#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>


int main(){

    int pid = fork();

    if(0 == pid){
        execlp("ls", "", NULL);
        printf("this will only happen if exec fails\n");
    }
    else{
        wait(NULL);
        printf("we are done!\n");
        
    }
    
  
    return 0;
}