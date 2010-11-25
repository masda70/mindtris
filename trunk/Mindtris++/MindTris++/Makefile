
CFLAGS = $(OFLAGS) $(DFLAGS) -I.

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
