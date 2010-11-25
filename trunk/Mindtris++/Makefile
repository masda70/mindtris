SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
LFLAGS = -L. -L./Crypto++ -lcurses
CFLAGS =

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

CFLAGS += $(OFLAGS) $(DFLAGS) -I.

CLIENTOJBS = $(CLIENTDIR)/peer.o $(CLIENTDIR)/mindtris.o
CLIENTLFLAGS = $(LFLAGS) -lcurses

SERVEROBJS = $(SERVERDIR)/database.o $(SERVERDIR)/lobby.o $(SERVERDIR)/$(SERVERDIR).o $(SERVERDIR)/user.o
SERVERLFLAGS = $(LFLAGS)

PROGS = $(CLIENTEXEC) $(SERVEREXEC)

all: $(OBJS) $(COBJS) $(PROGS)
	@echo
	@echo "#######################################"
	@echo "### BUILDING ALL TARGETS ###"
	@echo "#######################################"
	@echo 
	for i in $(SUBDIRS) ; do \
	( cd $$i ; make ) ; \
	done

$(CLIENTEXEC): $(CLIENTOBJS) $(COBJS)
	$(C++) -o $(CLIENTEXEC) $(CLIENTOBJS) $(CLIENTLFLAGS)
$(SERVEREXEC): $(SERVEROBJS) $(COBJS)
	$(C++) -o $(SERVEREXEC) $(SERVEROBJS) $(SERVERLFLAGS) 

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

$( $(OBJS) $(COBJS)

all: $(PROGS)

peer.o: peer.h ../mindtriscore/includes.h ../mindtriscore/util.h ../mindtriscore/socket.h ../mindtriscore/serverprotocol.h ../mindtriscore/p2pprotocol.h mindtris.h
mindtris.o: peer.h mindtris.h ../mindtriscore/includes.h ../mindtriscore/util.h ../mindtriscore/socket.h ../mindtriscore/serverprotocol.h ../mindtriscore/p2pprotocol.h
