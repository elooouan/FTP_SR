CC = gcc
INCLUDEDIR = include
SRCDIR = src
CFLAGS = -g -Wall -I$(INCLUDEDIR)

all: master server client

server: $(SRCDIR)/server.o $(SRCDIR)/csapp.o $(SRCDIR)/handlers.o
	$(CC) $(CFLAGS) -o $@ $^

master: $(SRCDIR)/master.o $(SRCDIR)/csapp.o
	$(CC) $(CFLAGS) -o $@ $^

client: $(SRCDIR)/client.o $(SRCDIR)/csapp.o $(SRCDIR)/get.o $(SRCDIR)/ls.o $(SRCDIR)/request.o
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf src/*.o clientside/* clientside/.[^.]*  server client master