#
# Copyright (c) 2010 Mark Heily <mark@heily.com>
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

# Flags to pass to dpkg-buildpackage
DPKGFLAGS=-uc -us

.PHONY :: install uninstall check dist dist-upload publish-www clean merge distclean fresh-build rpm edit cscope valgrind testing

include config.mk

all: $(PROGRAM).so testing

%.dll: $(OBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LDADD)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $(CFLAGS) $<

$(PROGRAM).a: $(OBJS)
	$(AR) rcs $(PROGRAM).a $(OBJS)

$(PROGRAM).so: $(OBJS)
	$(LD) -shared $(LDFLAGS) $(OBJS) $(LDADD)
	$(LN) -sf $(PROGRAM).so.$(ABI_VERSION) $(PROGRAM).so
	$(LN) -sf $(PROGRAM).so.$(ABI_VERSION) $(PROGRAM).so.$(ABI_MAJOR)

install: $(PROGRAM).so
	$(INSTALL) -d -m 755 $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -d -m 755 $(DESTDIR)$(LIBDIR)
	$(INSTALL) -m 644 $(HEADERS) $(DESTDIR)$(INCLUDEDIR)
	$(INSTALL) -m 644 $(PROGRAM).so.$(ABI_VERSION) $(DESTDIR)$(LIBDIR)
	$(LN) -sf $(PROGRAM).so.$(ABI_VERSION) $(DESTDIR)$(LIBDIR)/$(PROGRAM).so.$(ABI_MAJOR)
	$(LN) -sf $(PROGRAM).so.$(ABI_VERSION) $(DESTDIR)$(LIBDIR)/$(PROGRAM).so
	$(INSTALL) -d -m 755 $(DESTDIR)$(MANDIR)/man3
	$(INSTALL) -m 644 pthread_workqueue.3 $(DESTDIR)$(MANDIR)/man3/pthread_workqueue.3 

uninstall:
	rm -f $(INCLUDEDIR)/pthread_workqueue.h
	rm -f $(LIBDIR)/pthread_workqueue.so 
	rm -f $(LIBDIR)/pthread_workqueue.so.*
	rm -f $(LIBDIR)/pthread_workqueue.a
	rm -f $(MANDIR)/man3/pthread_workqueue.3 

reinstall: uninstall install
 
check: $(PROGRAM).so
	cd testing && make check

edit:
	$(EDITOR) `find ./src -name '*.c' -o -name '*.h'` Makefile

$(PROGRAM)-$(VERSION).tar.gz: 
	mkdir $(PROGRAM)-$(VERSION)
	cp Makefile ChangeLog configure config.inc $(MANS) $(PROGRAM)-$(VERSION)
	cp -R src testing include $(PROGRAM)-$(VERSION)
	find $(PROGRAM)-$(VERSION) -name '.svn' -exec rm -rf {} \; 2>/dev/null || true
	tar zcf $(PROGRAM)-$(VERSION).tar.gz $(PROGRAM)-$(VERSION)
	rm -rf $(PROGRAM)-$(VERSION)

testing:
	cd testing && make

dist: clean $(PROGRAM)-$(VERSION).tar.gz

%.asc:
	gpg --armor --detach-sign `echo '$@' | sed 's/.asc$$//'`

dist-upload: dist $(DISTFILE).asc
	scp $(DISTFILE) $(DISTFILE).asc heily.com:/var/www/heily.com/dist/$(PROGRAM)

publish-www:
	cp -R www/* ~/public_html/libkqueue/

clean:
	rm -f tags $(DISTFILE) $(DISTFILE).asc *.a $(OBJS) *.pc *.so *.so.* test-$(PROGRAM)
	cd testing && make clean
	rm -rf pkg

distclean: clean
	rm -f *.tar.gz config.mk config.h $(PROGRAM).pc $(PROGRAM).la rpm.spec
	rm -rf $(PROGRAM)-$(VERSION) 2>/dev/null || true

rpm: clean $(DISTFILE)
	rm -rf rpm *.rpm *.deb
	mkdir -p rpm/BUILD rpm/RPMS rpm/SOURCES rpm/SPECS rpm/SRPMS
	mkdir -p rpm/RPMS/i386 rpm/RPMS/x86_64
	cp $(DISTFILE) rpm/SOURCES 
	rpmbuild -bb rpm.spec
	mv ./rpm/RPMS/* .
	rm -rf rpm
	rmdir i386 x86_64    # WORKAROUND: These aren't supposed to exist
	fakeroot alien --scripts *.rpm

deb: clean $(DISTFILE)
	mkdir pkg
	cd pkg && tar zxf ../$(DISTFILE) && mv libpthread_workqueue-$(VERSION) libpthread-workqueue-$(VERSION)
	cp $(DISTFILE) pkg/libpthread-workqueue_$(VERSION).orig.tar.gz
	cp -R ports/debian pkg/libpthread-workqueue-$(VERSION) 
	cd pkg && \
	rm -rf `find libpthread-workqueue-$(VERSION)/debian -type d -name .svn` ; \
	perl -pi -e 's/\@\@VERSION\@\@/$(VERSION)/' libpthread-workqueue-$(VERSION)/debian/changelog ; \
	cd libpthread-workqueue-$(VERSION) && dpkg-buildpackage $(DPKGFLAGS)
	lintian -i pkg/*.deb
	@printf "\nThe following packages have been created:\n"
	@find ./pkg -name '*.deb' | sed 's/^/    /'
