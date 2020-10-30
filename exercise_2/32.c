#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>


int main(){

    int pid = fork();

    if(0 == pid){
        int child = getpid();
        printf("child: parent %d, group %d\n", getppid(), getpgid(child));
        sleep(4);
        printf("child: parent %d, group %d\n", getppid(), getpgid(child));
        sleep(4);
        printf("child: parent %d, group %d\n", getppid(), getpgid(child));
    }
    else{
        int parent = getpid();
        printf("parent: parent %d, group %d\n", getppid(), getpgid(parent));
        sleep(2);
        int i = 3 / 0;
    }
    
  
    return 0;
}