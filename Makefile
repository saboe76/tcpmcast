PREFIX=/usr/bin
CFLAGS = -g -O2 -Wall
BIN = tcpmcast

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all:	$(BIN)

tcpmcast:	tcpmcast.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(BIN) *.o

.PHONY: all clean tcpmcast
