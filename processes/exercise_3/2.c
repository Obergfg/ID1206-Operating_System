#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int volatile count;

void handler(int sig){
    printf("signal %d ouch that hurt\n", sig);
    count++;
}

int main(){

    struct sigaction sa;

    int pid = getpid();

    printf("ok, lets go! I am process (%d). Kill me if you can!\n", pid);

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if(0 != sigaction(SIGINT, &sa, NULL))
        return(1);

    while (4 != count){
        /* code */
    }

    printf("I've hade enough!\n");

    return(0);
    
}