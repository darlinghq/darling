VERSION=0.3
BITS=64

GCC_EXTRA_FLAGS=-m$(BITS)
GCCFLAGS=-g -Iinclude -Wall -MMD -fno-omit-frame-pointer -O $(GCC_EXTRA_FLAGS)
CXXFLAGS=$(GCCFLAGS) -W -Werror
CFLAGS=$(GCCFLAGS) -fPIC

EXES=libmac.so extract macho2elf ld-mac

MAC_C_SRCS=$(wildcard mach/*.c)
MAC_CXX_SRCS=$(wildcard mach/*.cc)
MAC_C_BINS=$(MAC_C_SRCS:.c=.c.bin)
MAC_CXX_BINS=$(MAC_CXX_SRCS:.cc=.cc.bin)
MACBINS=$(MAC_C_BINS) $(MAC_CXX_BINS)
MACTXTS=$(MACBINS:.bin=.txt)

OS=$(shell uname)

ifeq ($(OS), Linux)
MAC_TOOL_DIR=/usr/i686-apple-darwin10
MAC_BIN_DIR=$(MAC_TOOL_DIR)/usr/bin
MAC_CC=PATH=$(MAC_BIN_DIR) ./ld-mac $(MAC_BIN_DIR)/gcc --sysroot=$(MAC_TOOL_DIR)
MAC_CXX=PATH=$(MAC_BIN_DIR) ./ld-mac $(MAC_BIN_DIR)/g++ --sysroot=$(MAC_TOOL_DIR)
MAC_OTOOL=./ld-mac $(MAC_BIN_DIR)/otool
MAC_TARGETS=ld-mac $(MACBINS) $(MACTXTS)
else
MAC_CC=$(CC)
MAC_CXX=$(CXX)
MAC_OTOOL=otool
MAC_TARGETS=$(MACBINS) $(MACTXTS)
endif

all: $(EXES)

profile:
	$(MAKE) clean
	$(MAKE) all GCC_EXTRA_FLAGS=-pg

release:
	$(MAKE) clean
	$(MAKE) all "GCC_EXTRA_FLAGS=-DNOLOG -DNDEBUG"

both:
	$(MAKE) clean
	$(MAKE) BITS=32 all
	mv ld-mac ld-mac32
	mv libmac.so libmac32.so
	$(MAKE) clean
	$(MAKE) BITS=64 all

mach: $(MAC_TARGETS)

check: all mach
	./runtests.sh

$(MAC_C_BINS): %.c.bin: %.c
	$(MAC_CC) -g -arch i386 -arch x86_64 $^ -o $@

$(MAC_CXX_BINS): %.cc.bin: %.cc
	$(MAC_CXX) -g -arch i386 -arch x86_64 $^ -o $@

$(MACTXTS): %.txt: %.bin
	$(MAC_OTOOL) -hLltvV $^ > $@

#ok: macho2elf
#	./genelf.sh
#	touch $@

extract: extract.o fat.o
	$(CXX) $^ -o $@ -g -I. -W -Wall $(GCC_EXTRA_FLAGS)

macho2elf: macho2elf.o mach-o.o fat.o log.o
	$(CXX) $^ -o $@ -g $(GCC_EXTRA_FLAGS)

ld-mac: ld-mac.o mach-o.o fat.o log.o
	$(CXX) $^ -o $@ -g -ldl -lpthread $(GCC_EXTRA_FLAGS)

# TODO(hamaji): autotoolize?
libmac.so: libmac/mac.o libmac/strmode.c
	$(CC) -shared $^ $(CFLAGS) -o $@ -lcrypto $(GCC_EXTRA_FLAGS)

dist:
	cd /tmp && rm -fr maloader-$(VERSION) && git clone git@github.com:shinh/maloader.git && rm -fr maloader/.git && mv maloader maloader-$(VERSION) && tar -cvzf maloader-$(VERSION).tar.gz maloader-$(VERSION)

clean:
	rm -f *.o *.d */*.o */*.d $(EXES)

-include *.d */*.d
