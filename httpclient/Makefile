CC=gcc
CFLAGS=-g -Wall -pedantic

SUFFIXES=.c

PROGS=httpclient

all: $(PROGS)

httpclient: httpclient.c libhttpclient.c
	$(CC) $(CFLAGS) httpclient.c libhttpclient.c $(LDFLAGS) -o httpclient

clean:
	rm -f $(PROGS) *.txt index
