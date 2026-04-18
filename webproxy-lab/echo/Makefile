# Meta Makefile for Echo project (dummy scaffolding)

CC = gcc
CFLAGS = -g -Wall -I..
LDFLAGS = -lpthread
TOPDIR = ..
CSAPP_OBJ = $(TOPDIR)/csapp.o

$(CSAPP_OBJ):
\t$(MAKE) -C $(TOPDIR) csapp.o

echo_server.o: echo_server.c echo_common.h $(TOPDIR)/csapp.h
	$(CC) $(CFLAGS) -c echo_server.c

echo_client.o: echo_client.c echo_common.h $(TOPDIR)/csapp.h
	$(CC) $(CFLAGS) -c echo_client.c

echo_common.o: echo_common.c echo_common.h
	$(CC) $(CFLAGS) -c echo_common.c

echo_server: $(CSAPP_OBJ) echo_server.o echo_common.o
	$(CC) $(CFLAGS) echo_server.o echo_common.o $(CSAPP_OBJ) -o echo_server $(LDFLAGS)

echo_client: $(CSAPP_OBJ) echo_client.o echo_common.o
	$(CC) $(CFLAGS) echo_client.o echo_common.o $(CSAPP_OBJ) -o echo_client $(LDFLAGS)

server: echo_server
	@true

client: echo_client
	@true

all: server client

clean:
	rm -f *~ *.o echo_server echo_client
