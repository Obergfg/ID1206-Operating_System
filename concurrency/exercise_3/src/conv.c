#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#define TEST "This is a TeSt to SEE if iT wORks"
#define SERVER "lower"
#define CLIENT "help"
#define MAX 512

int main(void)
{
    char buffer[MAX];
    int sock;

    /* A socket is created */
    assert((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) != -1);

    struct sockaddr_un name = {AF_UNIX, CLIENT};

    assert(bind(sock, (struct sockaddr *) &name, sizeof(struct sockaddr_un)) != -1);
    
    struct sockaddr_un server = {AF_UNIX, SERVER};
    int size = sizeof(struct sockaddr_un);
    
    int n = sizeof(TEST);

    assert(sendto(sock, TEST, n, 0, (struct sockaddr *) &server, size) == n);
    n = recvfrom(sock, buffer, MAX-1, 0, (struct sockaddr *) &server, &size);
    if (n == -1)
      perror("server");

    buffer[n] = 0;
    printf("Client received: %s\n", buffer);
    unlink(CLIENT);
    exit(0);
}