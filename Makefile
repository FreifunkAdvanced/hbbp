BIN=hbbpd hbbpc

all: $(BIN)

.PHONY: clean
clean:
	rm -f $(BIN) *~

nacl/include:
	mkdir nacl
	wget http://hyperelliptic.org/nacl/nacl-20110221.tar.bz2 -O - | tar jxf - -C nacl
	cd nacl/* && ./do
	mkdir nacl/include
	ln -s ../../$$(ls -d nacl/*/build/*/include/*/ | head) nacl/include/nacl
	ln -s ../$$(ls -d nacl/*/build/*/lib/*/     | head) nacl/lib
