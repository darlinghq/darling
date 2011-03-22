GCCFLAGS=-g -Iinclude -Wall -MMD -fno-omit-frame-pointer -DNOLOG
CXXFLAGS=$(GCCFLAGS) -W -Werror
CFLAGS=$(GCCFLAGS) -fPIC

EXES=libmac/libmac.so extract macho2elf ld-mac

MACSRCS=$(wildcard mach/*.c)
MACBINS=$(MACSRCS:.c=.bin)
MACTXTS=$(MACSRCS:.c=.txt)

OS=$(shell uname)

ifeq ($(OS), Linux)
MAC_TOOL_DIR=/usr/i686-apple-darwin10
MAC_BIN_DIR=$(MAC_TOOL_DIR)/usr/bin
MAC_CC=PATH=$(MAC_BIN_DIR) ./ld-mac $(MAC_BIN_DIR)/gcc --sysroot=$(MAC_TOOL_DIR)
MAC_OTOOL=./ld-mac $(MAC_BIN_DIR)/otool
MAC_TARGETS=ld-mac $(MACBINS) $(MACTXTS)
else
MAC_CC=$(CC)
MAC_OTOOL=otool
MAC_TARGETS=$(MACBINS) $(MACTXTS)
endif

all: $(EXES)

mach: $(MAC_TARGETS)

check: all mach
	./runtests.sh

$(MACBINS): %.bin: %.c
	$(MAC_CC) -g $^ -o $@

$(MACTXTS): %.txt: %.bin
	$(MAC_OTOOL) -hLltvV $^ > $@

#ok: macho2elf
#	./genelf.sh
#	touch $@

extract: extract.o fat.o
	$(CXX) $^ -o $@ -g -I. -W -Wall

macho2elf: macho2elf.o mach-o.o fat.o
	$(CXX) $^ -o $@ -g

ld-mac: ld-mac.o mach-o.o fat.o
	$(CXX) $^ -o $@ -g -ldl -lpthread

# TODO(hamaji): autotoolize?
libmac/libmac.so: libmac/mac.o
	$(CC) -shared $^ -o $@ -luuid -lcrypto -lCoreFoundation || (echo "*** libCoreFoundation not found ***"; gcc -shared $^ -o $@ -luuid -lcrypto)

clean:
	rm -f *.o *.d */*.o */*.d $(EXES)

-include *.d */*.d
