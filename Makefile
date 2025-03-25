CC = gcc
INCLUDEDIR = include
SRCDIR = src
CFLAGS = -g -Wall -I$(INCLUDEDIR)

all: server client

server: $(SRCDIR)/server.o $(SRCDIR)/csapp.o
	$(CC) $(CFLAGS) -o $@ $^

client: $(SRCDIR)/client.o $(SRCDIR)/csapp.o
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf src/*.o clientside/* server client