SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
DFLAGS = -Wno-multichar -fPIC
OFLAGS = -Wall -O3
CFLAGS = $(OFLAGS) $(DFLAGS) -I. -I .. -I ../Crypto++
LFLAGS = -L../Crypto++/ -lcryptopp

LIBNAME = libmindtriscore

ifeq ($(SYSTEM),Darwin)
LDFLAGS = -dynamiclib -lgmp -L/opt/local/lib
TARGET = $(LIBNAME).dylib
else
LDFLAGS = -shared -lgmp
TARGET = $(LIBNAME).so
endif

OBJS = includes.o p2pprotocol.o serverprotocol.o socket.o util.o

all: $(OBJS) $(COBJS) $(TARGET)


clean:
	rm -f $(OBJS) $(COBJS) $(TARGET)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<


$(TARGET): $(OBJS)
	$(C++) -o $(TARGET) $(OBJS) $(LFLAGS) $(LDFLAGS)

includes.o: includes.h
p2pprotocol.o: includes.h util.h p2pprotocol.h
serverprotocol.o: includes.h util.h serverprotocol.h
socket.o: includes.h util.h socket.h
util.o: includes.h util.h
