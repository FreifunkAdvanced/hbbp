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
      fprintf(stderr, "usage: %s interface task [message|-]\n", argv[0]);
      exit(1);
    }
    char buf[MAXBUFLEN],
      *task = argv[2],
      *message = (argc == 4) ? argv[3] : "";
    int task_len = strlen(task),
      total_len = task_len + 1;
    strcpy(buf, task);
    buf[task_len] = 0;
    if (strcmp(message, "-") == 0) {
      /* read payload from stdin */
      int i;
      while ((MAXBUFLEN - total_len - 1 > 0)
	     && ((i = read(0, &(buf[total_len]), MAXBUFLEN - total_len - 1)) > 0))
	total_len += i;
      ENP(i, "read(stdin)");
      if (read(0, &i, 1) != 0) {
	fprintf(stderr, "payload to long: max %d bytes\n", MAXBUFLEN - 1);
	exit(1);
      }
    }else{
      /* use cmd line for payload */
      total_len += strlen(message);
      if (total_len <= MAXBUFLEN)
	strcpy(buf + 1 + task_len, message);
    }
    if (total_len > MAXBUFLEN) {
      fprintf(stderr,"payload to long: max %d bytes, was %d\n",
	      MAXBUFLEN - 1, total_len);
      exit(1);
    }

    /* setup socket */
    ENP((fd = socket(AF_INET6, SOCK_DGRAM, 0)), "socket");
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(SERVERPORT);
    addr.sin6_flowinfo = 0;
    inet_pton(AF_INET6, "ff02::1", &(addr.sin6_addr));
    if ((addr.sin6_scope_id = if_nametoindex(argv[1])) == 0) {
      fprintf(stderr, "interface not found\n");
      exit(1);
    }

    /* send packet */
    ENP(sendto(fd, buf, total_len, 0, (struct sockaddr *) &addr, sizeof addr), 
	"sendto");

    return 0;
}

