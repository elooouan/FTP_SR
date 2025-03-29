CC = gcc
INCLUDEDIR = include
SRCDIR = src
CFLAGS = -g -Wall -I$(INCLUDEDIR)

all: master server client

server: $(SRCDIR)/server.o $(SRCDIR)/csapp.o $(SRCDIR)/handlers_server.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

master: $(SRCDIR)/master.o $(SRCDIR)/csapp.o $(SRCDIR)/handlers_master.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

client: $(SRCDIR)/client.o $(SRCDIR)/csapp.o $(SRCDIR)/get.o $(SRCDIR)/ls.o $(SRCDIR)/request.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

src/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf src/*.o clientside/* clientside/.[^.]*  server client master