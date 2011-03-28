GCCFLAGS=-g -Iinclude -Wall -MMD -fno-omit-frame-pointer -O
CXXFLAGS=$(GCCFLAGS) -W -Werror
CFLAGS=$(GCCFLAGS) -fPIC

EXES=libmac/libmac.so extract macho2elf ld-mac

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

mach: $(MAC_TARGETS)

check: all mach
	./runtests.sh

$(MAC_C_BINS): %.c.bin: %.c
	$(MAC_CC) -g $^ -o $@

$(MAC_CXX_BINS): %.cc.bin: %.cc
	$(MAC_CXX) -g $^ -o $@

$(MACTXTS): %.txt: %.bin
	$(MAC_OTOOL) -hLltvV $^ > $@

#ok: macho2elf
#	./genelf.sh
#	touch $@

extract: extract.o fat.o
	$(CXX) $^ -o $@ -g -I. -W -Wall

macho2elf: macho2elf.o mach-o.o fat.o log.o
	$(CXX) $^ -o $@ -g

ld-mac: ld-mac.o mach-o.o fat.o log.o
	$(CXX) $^ -o $@ -g -ldl -lpthread

# TODO(hamaji): autotoolize?
libmac/libmac.so: libmac/mac.o
	$(CC) -shared $^ -o $@ -luuid -lcrypto -lCoreFoundation || (echo "*** libCoreFoundation not found ***"; gcc -shared $^ -o $@ -luuid -lcrypto)

clean:
	rm -f *.o *.d */*.o */*.d $(EXES)

-include *.d */*.d
