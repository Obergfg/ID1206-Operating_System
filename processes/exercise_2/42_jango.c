#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>


int main(){

    int pid = fork();

    if(0 == pid){
        
        int fd = open("quotes.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fd, 1);
        close(fd);
        
        execl("boba", "boba", NULL);
        printf("this will only happen if exec fails\n");
    }
    else{
        wait(NULL); 
    }
    
  
    return 0;
}