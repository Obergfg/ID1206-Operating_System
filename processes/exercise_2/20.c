#include <stdio.h>
#include <unistd.h>  

int main(){

    int p = getpid();

    printf("Mothers pid: %d\n", p);
    
    int pid = fork();

    if(0 == pid)
        printf("I'm the child %d\n", getpid());
    else
        printf("My child is called %d\n", pid);
    

    printf("That's it %d\n", getpid());
    


    return 0;
}