# CC=gcc
IDIR = "./includes"
PKG_SOURCE_VERSION:=$(COMMIT)
INCLUDES = "-I/usr/include/libnl3"

CFLAGS= -g -O2 -Wall -Wextra -Isrc -I$(IDIR) $(INCLUDES) $(CT_ARCH)

ODIR=obj
LDIR =../lib

LIBS=-lm -lssl -lcrypto -lrt -lpthread -lcurl -ljson-c -lmosquitto -lnl-3 -lnl-route-3 -lnl-genl-3 -lz

TARGET=socketman
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

_DEPS=$(wildcard *.h include/**/*.h include/*.h)
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

SOURCES=$(wildcard *.c src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,$(ODIR)/%.o,$(SOURCES))

$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p obj
	$(CC) -g -c -o $@ $< $(CFLAGS)

$(SO_TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
