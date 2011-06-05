/*
** listener.c -- a datagram sockets "server" demo
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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[], char *envp[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, SERVERPORT_S, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);


    addr_len = sizeof their_addr;
    while ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
           (struct sockaddr *)&their_addr, &addr_len)) != -1) {
      // decode packet & launch handler
      buf[numbytes] = '\0';
      char *task = buf,
	*cl_argv[3] = {task, (strlen(task)<numbytes) ? (buf+strlen(task)+1) : NULL, NULL };

      if (task[0] == '/' || strstr(task, "..")) {
	fprintf(stderr, "payload tried directory traversal\n");
	continue;
      }

      switch (fork()) {
      case 0:
	// TODO: clean file descriptors
	execve(task, cl_argv, envp);
	perror("exec");
	exit(-1);
      case -1:
	perror("fork (serious!)");
	exit(-1);
      default:
	wait();
      }
    }

    perror("recvfrom");
    close(sockfd);

    return -1;
}
