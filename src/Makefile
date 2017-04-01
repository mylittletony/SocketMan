# CC=gcc
LIBNL3_PATH = "-I/usr/include/libnl3"

# When packaging, someone else sets the CFLAGS, so don't override them if they
# have already been set. If there is no CFLAGS, then party hard!
CFLAGS ?= -g -O2 -Wall -Wextra -I$(LIBNL3_PATH) $(CT_ARCH) -DCT_VERSION='"$(VERSION)"'
# However, only we know about these paths, so append them to CFLAGS
CFLAGS += -Isrc -Iincludes

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
