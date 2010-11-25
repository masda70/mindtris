SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
CFLAGS = $(OFLAGS) $(DFLAGS) -I ../ -I ../Crypto++

CLIENTDIR = MindTris++
SERVERDIR = mt_server

SUBDIRS = $(CLIENTDIR) $(SERVERDIR)

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


OBJS = mindtris.o peer.o
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

peer.o: peer.h ../mindtriscore/includes.h ../mindtriscore/util.h ../mindtriscore/socket.h ../mindtriscore/serverprotocol.h ../mindtriscore/p2pprotocol.h mindtris.h
mindtris.o: peer.h mindtris.h ../mindtriscore/includes.h ../mindtriscore/util.h ../mindtriscore/socket.h ../mindtriscore/serverprotocol.h ../mindtriscore/p2pprotocol.h
