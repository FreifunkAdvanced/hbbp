#ifndef UDP_BROADCAST_COMMON
#define UDP_BROADCAST_COMMON

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVERPORT 4950    // the port users will be connecting to
#define SERVERPORT_S "4950"
#define MAXBUFLEN 100

#define ENP(Cmd, Msg) \
  if ((Cmd) == -1) {		\
    perror(Msg);		\
    exit(1);			\
  }

#endif // UDP_BROADCAST_COMMON
