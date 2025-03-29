CC = gcc
INCLUDEDIR = include
SRCDIR = src
CFLAGS = -g -Wall -I$(INCLUDEDIR) 

all: server client

server: $(SRCDIR)/server.o $(SRCDIR)/csapp.o $(SRCDIR)/handlers.o 
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

client: $(SRCDIR)/client.o $(SRCDIR)/csapp.o $(SRCDIR)/get.o $(SRCDIR)/request.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

src/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf src/*.o clientside/* clientside/.[^.]*  server client
