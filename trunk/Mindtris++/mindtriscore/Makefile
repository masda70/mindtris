SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I ../Crypto++/

OBJS = includes.o p2pprotocol.o serverprotocol.o socket.o util.o
COBJS =
PROGS =

all: $(OBJS) $(COBJS) $(PROGS)


clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<


all: $(PROGS)

includes.o: includes.h
p2pprotocol.o: includes.h util.h p2pprotocol.h
serverprotocol.o: includes.h util.h serverprotocol.h
socket.o: includes.h util.h socket.h
util.o: includes.h util.h
