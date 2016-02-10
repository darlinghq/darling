#
# Copyright (c) 2011 Mark Heily <mark@heily.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

.PHONY :: install uninstall check dist dist-upload publish-www clean merge distclean fresh-build rpm edit cscope valgrind

include ../../config.mk

all: test-$(PROGRAM)

test-$(PROGRAM): test.c
	$(CC) $(CFLAGS) -g -O0 -o test-$(PROGRAM) -I../.. -I../../include -L../.. test.c -lpthread_workqueue -lpthread

check: test-$(PROGRAM)
	LD_LIBRARY_PATH=../..:/usr/sfw/lib/amd64 ./test-$(PROGRAM)

debug: test-$(PROGRAM)
	LD_LIBRARY_PATH=../.. gdb ./test-$(PROGRAM)

valgrind: test-$(PROGRAM)
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes --num-callers=20 --track-fds=yes ./test-$(PROGRAM)

clean:
	rm -f test-$(PROGRAM)
