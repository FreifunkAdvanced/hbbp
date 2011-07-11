/*
** broadcaster.c -- a datagram "client" like talker.c, except
**                  this one can broadcast
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "common.h"

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int numbytes;
    int broadcast = 1;

    // assemble packet
    if (argc < 3 || argc > 4) {
        fprintf(stderr,"usage: broadcaster hostname task [message]\n");
        exit(1);
    }
    char buf[MAXBUFLEN],
      *task = argv[2],
      *message = (argc == 4) ? argv[3] : NULL;
    int task_len = strlen(task),
      total_len = task_len + 1 + strlen(message);
    if (total_len > MAXBUFLEN) {
      fprintf(stderr,"payload to long: max %d bytes, was %d\n", MAXBUFLEN, total_len);
      exit(1);
    }
    strcpy(buf, task);
    buf[task_len] = 0;
    strcpy(buf + 1 + task_len, message);

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    if ((numbytes=sendto(sockfd, buf, total_len, 0,
             (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        perror("sendto");
        exit(1);
    }

    return 0;
}
