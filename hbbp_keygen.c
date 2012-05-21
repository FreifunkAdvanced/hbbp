#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "crypto.h"


#define pub_len  crypto_box_PUBLICKEYBYTES
#define priv_len crypto_box_SECRETKEYBYTES

const char *name[4] = {"recv.pub", "recv.priv", "send.pub", "send.priv"};

void error_cleanup(int num) {
    for (int i = 0; i < num; i++)
	unlink(name[i]);
    exit(1);
}

int main() {
  int fd[4], i;
  byte key_pub [2][pub_len],
       key_priv[2][priv_len];

  /* open files */
  for (i=0; i<4; i++) {
    if ((fd[i] = open(name[i], O_WRONLY|O_CREAT|O_EXCL,
		      (i%2) ? 0400 : 0444)) == -1) {
      fprintf(stderr, 
	      (errno == EEXIST)
	      ? "%s already exists\n" : "could not create file %s\n",
	      name[i]);
      error_cleanup(i);
    }
  }

  /* generate keys */
  for (i=0; i<2; i++) {
    crypto_box_keypair(key_pub[i], key_priv[i]);
    if ((write(fd[2*i  ], key_pub[i],  pub_len)  != pub_len)
     || (write(fd[2*i+1], key_priv[i], priv_len) != priv_len)
     || (close(fd[2*i  ]) == -1)
     || (close(fd[2*i+1]) == -1)) {
      error_cleanup(4);
    }
  }

  return 0;
}
