GCCFLAGS=-g -Iinclude -Wall -MMD -fno-omit-frame-pointer -DNOLOG
CXXFLAGS=$(GCCFLAGS) -W -Werror
CFLAGS=$(GCCFLAGS) -fPIC

EXES=libmac/libmac.so extract macho2elf ld-mac

MACSRCS=$(wildcard mach/*.c)
MACBINS=$(MACSRCS:.c=.bin)
MACTXTS=$(MACSRCS:.c=.txt)

all: $(EXES)

mac: $(MACBINS) $(MACTXTS)

$(MACBINS): %.bin: %.c
	$(CC) -g $^ -o $@

$(MACTXTS): %.txt: %.bin
	otool -hLltvV $^ > $@

#ok: macho2elf
#	./genelf.sh
#	touch $@

extract: extract.o
	g++ $< -o $@ -g -I. -W -Wall

macho2elf: macho2elf.o mach-o.o
	g++ $^ -o $@ -g

ld-mac: ld-mac.o mach-o.o
	g++ $^ -o $@ -g -ldl

libmac/libmac.so: libmac/mac.o
	gcc -shared $^ -o $@

clean:
	rm -f *.o *.d */*.o */*.d $(EXES)

-include *.d */*.d
