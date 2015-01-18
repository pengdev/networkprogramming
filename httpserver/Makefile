CC=gcc
CFLAGS=-g -Wall

SUFFIXES=.c

PROGS=httpserver

all: $(PROGS)

httpserver: httpserver.c libhttpserver.c
	$(CC) $(CFLAGS) httpserver.c libhttpserver.c $(LDFLAGS) -o httpserver

clean:
	rm -f $(PROGS) 
