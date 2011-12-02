/*
** listener.c -- a datagram sockets "server" demo
*/

#include "common.h"

int main(int argc, char **argv, char **envp) {
  int fd;
  int numbytes;
  struct sockaddr_storage their_addr;
  char buf[MAXBUFLEN];
  socklen_t addr_len;

  /* setup socket, parse cmd line */
  {
    struct sockaddr_in6 ba;
    struct ipv6_mreq ga;
    if (argc > 2) {
      fprintf(stderr, "usage: %s [interface]\n", argv[0]);
      exit(1);
    }
    ENP((fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)), "listener: socket");
    if (argc == 2) {
      inet_pton(AF_INET6, "ff02::1", &(ba.sin6_addr));
      if ((ba.sin6_scope_id = if_nametoindex(argv[1])) == 0) {
	fprintf(stderr, "interface not found\n");
	exit(1);
      }
    }else{
      ba.sin6_addr = in6addr_any;
      ba.sin6_scope_id = 0;
    }
    ba.sin6_family = AF_INET6;
    ba.sin6_port = htons(SERVERPORT);
    ba.sin6_flowinfo = 0;
    ENP(bind(fd, (struct sockaddr*) &ba, sizeof ba), "listener: bind");
  }

  /* daemonize */
#ifndef DEBUG
  switch (fork()) {
  case 0:
    ENP(setsid(), "setsid");
    umask(0);
    break;
  case -1:
    perror("fork");
    exit(1);
  default:
    exit(0);
  }
#endif
    
  /* receive loop */
  addr_len = sizeof their_addr;
  while ((numbytes = recvfrom(fd, buf, MAXBUFLEN-1 , 0,
			      (struct sockaddr *) &their_addr, &addr_len)) != -1) {
    /* decode packet & launch handler */
    buf[numbytes] = '\0';
    char *task = buf,
      *cl_argv[2] = {task, NULL};
    if (task[0] == '/' || strstr(task, "..")) {
      fprintf(stderr, "payload tried directory traversal\n");
      continue;
    }

    int fd_payload[2];
    pipe(fd_payload);
      
    switch (fork()) {
    case 0:
      /* redirect payload to stdio, leave stdout/err, close the rest */
      ENP(dup(fd_payload[0], 1), "dup");
      ENP(close(fd_payload[0]),  "close");
      ENP(close(fd_payload[1]),  "close");
      ENP(close(fd),             "close");

      /* run our task task */
      execve(task, cl_argv, envp);
      perror("exec");
      exit(1);
    case -1:
      perror("fork");
      exit(1);
    default:
      ENP(wait(), "wait");
    }

    /* write payload to child process; the return value of write is
       intentionally ignored */
    int tasklen = strlen(task);
    if (tasklen < numbytes)
      write(fd_payload[1], buf + tasklen + 1, numbytes - tasklen - 1);
    ENP(close(fd_payload[1]), "close");
    ENP(close(fd_payload[0]), "close");
  }
    
  perror("recvfrom");
  return 1;
}
