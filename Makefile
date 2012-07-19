BIN=hbbpd hbbpc hbbp_keygen hbbpp

ifndef EXTERNAL_NACL
override CFLAGS  += -Inacl/include/
override LDFLAGS += -Lnacl/lib/
NACL_DEP = nacl/include
endif

override CFLAGS +=-std=c99 -Wall -Wextra -fwhole-program -Os
override LDFLAGS +=-lnacl

all: $(BIN)

.PHONY: clean
clean:
	-rm $(BIN) *~

distclean: clean
	-rm -r nacl

%: %.c common.h crypto.h $(NACL_DEP)
	$(CC) -o $@ $< $(LDFLAGS) $(CFLAGS)

nacl/include:
	mkdir nacl
	wget http://hyperelliptic.org/nacl/nacl-20110221.tar.bz2 -O - | tar jxf - -C nacl
	cd nacl/* && ./do
	mkdir nacl/include
	ln -s ../../$$(ls -d nacl/*/build/*/include/*/ | head) nacl/include/nacl
	ln -s ../$$(ls -d nacl/*/build/*/lib/*/     | head) nacl/lib
