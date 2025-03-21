CC = gcc
CFLAGS = -g -Wall -Werror
LDFLAGS =
LIBS += -lpthread

SRCDIR = src
INCDIR = include
INCLUDE = $(INCDIR)/csapp.h $(INCDIR)/server.h $(INCDIR)/handlers.h
CSAPP_OBJ = $(SRCDIR)/csapp.o

server_OBJS = $(SRCDIR)/server.o $(CSAPP_OBJ) $(SRCDIR)/handlers.o
client_OBJS = $(SRCDIR)/client.o $(CSAPP_OBJ)

PROGS = server client

all: $(PROGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(INCLUDE)
	$(CC) $(CFLAGS) -I$(INCDIR) -c -o $@ $<

server: $(server_OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

client: $(client_OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f $(PROGS) $(SRCDIR)/*.o
