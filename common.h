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

/* the port users will be connecting to */
#define SERVERPORT 4950
#define SERVERPORT_S "4950"

/* RFC 2460 requires IPv6 link MTU >= 1280, IPv6 UDP headers sum up to
   48 byte, the rest is for us. One extra byte is for the final \0
   (not transmitted) */
#define MAXBUFLEN 1233

#define ENP(Cmd, Msg) \
  if ((Cmd) == -1) {		\
    perror(Msg);		\
    exit(1);			\
  }

#endif // UDP_BROADCAST_COMMON
