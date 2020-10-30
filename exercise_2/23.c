#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>

int main(){

    int p = getpid();

    printf("Mothers pid: %d\n", p);
    
    int pid = fork();

    if(0 == pid){
        printf("Child checking the status\n");
        sleep(10);
        printf("Child Checking again\n");
        return 42;
    }
    else{
        sleep(20);
        int res;
        wait(&res);
        printf("Res value: %d \n", res);
        printf("The result was %d \n", WEXITSTATUS(res));
        printf("Checking again\n");
        sleep(10);
    }


    printf("The end of the process: %d\n", getpid());
    
    return 0;
}