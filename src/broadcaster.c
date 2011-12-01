/*
** broadcaster.c -- a datagram "client" like talker.c, except
**                  this one can broadcast
*/

#include "common.h"

int main(int argc, char **argv)
{
    int fd;
    struct sockaddr_in6 addr;
    int broadcast = 1;

    /* assemble packet, parse cmd line */
    if (argc < 3 || argc > 4) {
      fprintf(stderr, "usage: %s interface task [message]\n", argv[0]);
      exit(1);
    }
    char buf[MAXBUFLEN],
      *task = argv[2],
      *message = (argc == 4) ? argv[3] : "";
    int task_len = strlen(task),
      total_len = task_len + 1 + strlen(message);
    if (total_len > MAXBUFLEN) {
      fprintf(stderr,"payload to long: max %d bytes, was %d\n", MAXBUFLEN, total_len);
      exit(1);
    }
    strcpy(buf, task);
    buf[task_len] = 0;
    strcpy(buf + 1 + task_len, message);

    /* setup socket */
    if ((fd = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(SERVERPORT);
    addr.sin6_flowinfo = 0;
    inet_pton(AF_INET6, "ff02::1", &(addr.sin6_addr));
    if ((addr.sin6_scope_id = if_nametoindex(argv[1])) == 0) {
      fprintf(stderr, "interface not found\n");
      exit(1);
    }

    /* send packet */
    if (sendto(fd, buf, total_len, 0, (struct sockaddr *)&addr, sizeof addr) == -1) {
        perror("sendto");
        exit(1);
    }

    return 0;
}
