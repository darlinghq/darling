#
# Include generated dependency files.
#
XAR_SRCS := xar.c
XAR_SRCS := $(patsubst %, src/%, $(XAR_SRCS))
-include $(XAR_SRCS:%.c=%.d)

src_all : src/xar src/ixar

src_install : src/ixar
	.././install-sh -c -d $(DESTDIR)$(BINDIR)
	.././install-sh -c -m 0755 $< $(DESTDIR)$(BINDIR)/xar
	.././install-sh -c -d $(DESTDIR)$(MANDIR)/man1
	.././install-sh -c -m 0644 src/xar.1 $(DESTDIR)$(MANDIR)/man1

src_uninstall :
	rm -f $(DESTDIR)/$(BINDIR)/xar
	rm -f $(DESTDIR)/$(MANDIR)/man1/xar.1

src_clean :
	rm -f src/xar
	rm -f src/ixar
	rm -f $(XAR_SRCS:%.c=%.o)
	rm -f $(XAR_SRCS:%.c=%.d)

src_distclean :


ifeq (yes, yes)
LIBRXAR := $(LIBRXAR_S)
endif
ifeq (yes, yes)
LIBRXAR := $(LIBXAR_A)
endif

# xar links against librxar, so that it can be run without first installing
# libxar.
src/% : src/%.o $(LIBRXAR)
	@mkdir -p $(@D)
ifneq ($(words "" ), 1)
	$(CC) $(CFLAGS) -o $@ $< /Users/ariel/LinuxHome/code/darling/src/xar/xar/lib $(LDFLAGS) $(LIBRXAR) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
else
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBRXAR) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif

ifeq (yes, yes)
LIBXAR := $(LIBXAR_A)
endif
ifeq (yes, yes)
LIBXAR := $(LIBXAR_S)
endif

# ixar is the version of the xar binary that gets installed.
src/i% : src/%.o $(LIBXAR)
	@mkdir -p $(@D)
ifneq ($(words "" ), 1)
	$(CC) $(CFLAGS) -o $@ $< $(LIBDIR) $(LDFLAGS) $(LIBXAR) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
else
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(LIBXAR) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif

src/%.o : src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	@$(SHELL) -ec "$(CC) -MM $(CPPFLAGS) $< | sed \"s/\($(subst /,\/,$(notdir $(basename $@)))\)\.o\([ :]*\)/$(subst /,\/,$(strip $(dir $@)))\1.o \2/g\" > $(@:%.o=%.d)"
