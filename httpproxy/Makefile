CC=gcc
CFLAGS=-g -Wall

SUFFIXES=.c

PROGS=httpproxy

all: $(PROGS)

httpproxy: libhttpclient.c libhttpproxy.c httpproxy.c
	$(CC) $(CFLAGS) libhttpclient.c libhttpproxy.c httpproxy.c $(LDFLAGS) -o httpproxy

clean:
	rm -f $(PROGS) *.txt index
