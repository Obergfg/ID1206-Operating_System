#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int volatile done;


void handler(int sig, siginfo_t *siginfo, void *context){
    printf("signal %d was caught\n", sig);

    printf("your UID is %d\n", siginfo->si_uid);
    printf("your PID is %d\n", siginfo->si_pid);
    
    done = 1;
}


int main(){

    struct sigaction sa;

    int pid = getpid();

    printf("Ok, let's go - kill me (%d)!\n", pid);

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    

    if(0 != sigaction(SIGINT, &sa, NULL))
        return 1;

    while (!done){
        /* code */
    }
    

    printf("Told you so!\n");

    return(0);
}