#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "joshua.h"

int main(){

    char *file_name = "/dev/joshua";
    int fd;

    fd = open(file_name, O_RDONLY);

    if(-1 == fd){
        perror("Joshua is not available");
        return 2;
    }

    char buffer[JOSHUA_MAX];

    if(-1 == ioctl(fd, JOSHUA_GET_QUOTE, &buffer))
        perror("Hmm, not so good");
    else
        printf("Quote - %s\n", buffer);

    close(fd);

    return 0;
}