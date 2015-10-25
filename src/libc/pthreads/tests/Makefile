MACHINE:=`arch`
MAKEOBJDIR:=../../obj.`arch`

TESTS = pthread_atfork_test
LDFLAGS = -nostdlib -lcrt1.o
LDLIBS = $(MAKEOBJDIR)/libc.a /usr/local/lib/system/libnotify.a \
		/usr/local/lib/system/libm.a /usr/local/lib/system/libmCommon.a \
		-lgcc /usr/local/lib/system/libkeymgr.a \
		/usr/local/lib/system/libdyld.a /usr/local/lib/system/libmacho.a \
		$(MAKEOBJDIR)/libc.a
CFLAGS = -I..

.SUFFIXES:
.SUFFIXES: .c

.PHONY: tests

tests: $(TESTS)
	@for i in $(TESTS); do \
		$$i && echo $$i passed; \
	done

clean:
	rm -f $(TESTS)

Makefile pthread_atfork_test.c : ;
