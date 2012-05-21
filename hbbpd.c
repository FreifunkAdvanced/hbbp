#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "common.h"
#include "crypto.h"

int main(int argc, char **argv, char **envp) {
  int fd;
  int numbytes;
  struct sockaddr_storage their_addr;
  byte buf[MAXBUFLEN];
  socklen_t addr_len;

  /* setup socket, parse cmd line */
  {
    struct sockaddr_in6 ba;
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
    /* decode packet */
    buf[numbytes] = '\0';
    char *task = (char*) buf;
    int task_len = strlen(task);
    byte *payload = buf + task_len + 1;
    int payload_len = numbytes - task_len - 1;

    if (task[0] == '/' || strstr(task, "..")) {
      fprintf(stderr, "payload tried directory traversal\n");
      continue;
    }

    /* TODO: check if the task exists and is executable; save cpu
       cycles trying to decode messages not intended for us */

    /* decipher packet */
    if (!decipher(task, &payload, &payload_len)) {
      fprintf(stderr, "unable to decrypt payload\n");
      continue;
    }

    /* launch handler */
    int fd_payload[2];
    ENP(pipe(fd_payload), "pipe");
      
    switch (fork()) {
    case 0:
      /* redirect payload to stdio, leave stdout/err, close the rest */
      ENP(dup2(fd_payload[0], 0), "dup");
      ENP(close(fd_payload[0]),  "close");
      ENP(close(fd_payload[1]),  "close");
      ENP(close(fd),             "close");

      /* run our task task */
      char *cl_argv[2] = {task, NULL};
      execve(task, cl_argv, envp);
      perror("exec");
      exit(1);
    case -1:
      perror("fork");
      exit(1);
    }

    /* write payload to child process; the return value of write is
       intentionally ignored */
    if (task_len < numbytes)
      IGN(write(fd_payload[1], payload, payload_len)) 
    ENP(close(fd_payload[1]), "close");
    ENP(close(fd_payload[0]), "close");
    ENP(wait(NULL), "wait");
  }
    
  perror("recvfrom");
  return 1;
}
