#pragma once

/* WARNING: none of these functions is reentrant because all use the
   same scratch buffer */

#include <fcntl.h>
#include <strings.h>

#include "nacl/crypto_box.h"

#include "common.h"

/* scratchpad for crypto operations */
byte crypto_buf[MAXBUFLEN]; 

char *key_path(char *task, char *type) {
  char *base = getenv("HBBP_KEYDIR"),
    *res = (char*) crypto_buf;
  if (!base) base = "/etc/hbbp/keys";
  if ((unsigned) snprintf(res, MAXBUFLEN, "%s/%s/%s", base, task, type)
      > MAXBUFLEN)
    return NULL;
  return res;  
}

int key_load(char *task, char *type, byte *key, int key_len) {
  int fd = open(key_path(task, type), O_RDONLY);
  if (fd == -1) /* no specific key? -> try default */
    fd = open(key_path("default", type), O_RDONLY);
  if (fd == -1) return 0;
  if (read(fd, key, key_len) != key_len) {
    fprintf(stderr, "invalid key %s\n", crypto_buf);
    return 0;
  }
  ENP(close(fd), "close");
  return 1;
}

/* return error msg or null on sucess */
char* encipher(char *task, byte *data, int *len) {
  byte key_priv[crypto_box_SECRETKEYBYTES],
       key_pub [crypto_box_PUBLICKEYBYTES];
  switch(key_load(task, "send.priv", key_priv, crypto_box_SECRETKEYBYTES)
       + key_load(task, "recv.pub",  key_pub,  crypto_box_PUBLICKEYBYTES)) {
  case 0: return NULL; /* no keys -> no encryption required */
  case 1: return "missing key";
  }

  /* copy data to scratch buffer */
  if ((*len + crypto_box_ZEROBYTES > MAXBUFLEN)
   || (*len + crypto_box_ZEROBYTES + crypto_box_NONCEBYTES + strlen(task) + 1 > MAXBUFLEN))
    return "payload to large (no space left for crypto)";
  bzero (crypto_buf, crypto_box_ZEROBYTES);
  memcpy(crypto_buf + crypto_box_ZEROBYTES, data, *len);

  /* encrypt */
  byte *nonce = data,
       *cdata = data + crypto_box_NONCEBYTES;
  memset(nonce, 42, crypto_box_NONCEBYTES); /* URGENT TODO: random nonce */
  if (crypto_box(cdata, crypto_buf, *len + crypto_box_ZEROBYTES, nonce, key_pub, key_priv) != 0) return "oooh";
  memmove(data + crypto_box_NONCEBYTES,
	  data + crypto_box_NONCEBYTES + crypto_box_BOXZEROBYTES,
	  *len + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES);
  *len += crypto_box_NONCEBYTES + crypto_box_ZEROBYTES - crypto_box_BOXZEROBYTES;
  return NULL;
}

int decipher(char *task, byte **data, int *len) {
  byte key_priv[crypto_box_SECRETKEYBYTES],
       key_pub [crypto_box_PUBLICKEYBYTES];
  switch(key_load(task, "recv.priv", key_priv, crypto_box_SECRETKEYBYTES)
       + key_load(task, "send.pub",  key_pub,  crypto_box_PUBLICKEYBYTES)) {
  case 0: return 1; /* no keys -> no decryption required */
  case 1: return 0; /* one missing key -> error */
  }

  byte nonce[crypto_box_NONCEBYTES];
  memcpy(nonce, *data, crypto_box_NONCEBYTES);
  byte *cdata   = *data + (crypto_box_NONCEBYTES - crypto_box_BOXZEROBYTES);
  int cdata_len = *len  - (crypto_box_NONCEBYTES - crypto_box_BOXZEROBYTES);
  if (cdata_len < crypto_box_BOXZEROBYTES) return 0;
  bzero(cdata, crypto_box_BOXZEROBYTES);
  if (crypto_box_open(crypto_buf, cdata, cdata_len, nonce,
		      key_pub, key_priv) == -1) return 0;

  *data = crypto_buf + crypto_box_ZEROBYTES;
  *len = cdata_len - crypto_box_ZEROBYTES;
  return 1;
}
