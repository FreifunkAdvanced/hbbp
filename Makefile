BIN=hbbpd hbbpc

all: $(BIN)

.PHONY: clean
clean:
	rm -f $(BIN) *~
