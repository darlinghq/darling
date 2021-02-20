#
# File lists.
#

# Internal headers.
LIBXAR_IINCS := asprintf.h

LIBXAR_IINCS := $(patsubst %, include/%, $(LIBXAR_IINCS))
LIBXAR_IINCS += include/config.h

# Headers.
LIBXAR_INCS :=

LIBXAR_INCS := $(patsubst %, include/%, $(LIBXAR_INCS))
LIBXAR_INCS += include/xar.h

# Sources.
LIBXAR_SRCS := archive.c arcmod.c b64.c bzxar.c darwinattr.c data.c ea.c err.c
LIBXAR_SRCS += ext2.c fbsdattr.c filetree.c io.c lzmaxar.c linuxattr.c hash.c
LIBXAR_SRCS += signature.c stat.c subdoc.c util.c zxar.c script.c macho.c

LIBXAR_SRCS := $(patsubst %, lib/%, $(LIBXAR_SRCS))

# Libraries.  librxar is created such that it's possible to run xar without
# first installing libxar.

LIBXAR_LANAME := libxar.la
LIBXAR_ANAME := libxar.a
ifeq (elf, macho)
LIBRXAR_SNAME := librxar.so.1
LIBRXAR_LNAME := librxar.so
LIBRXAR_L := lib/$(LIBRXAR_LNAME)
LIBXAR_SNAME := libxar.so.1
LIBXAR_LNAME := libxar.so
LIBXAR_L := lib/$(LIBXAR_LNAME)
endif
ifeq (macho, macho)
LIBRXAR_SNAME := librxar.1.dylib
LIBRXAR_LNAME := librxar.dylib
LIBRXAR_L := lib/$(LIBRXAR_LNAME)
LIBXAR_SNAME := libxar.1.dylib
LIBXAR_LNAME := libxar.dylib
LIBXAR_L := lib/$(LIBXAR_LNAME)
endif
ifeq (aout, macho)
LIBRXAR_SNAME := librxar.so.1.0
LIBRXAR_LNAME :=
LIBRXAR_L :=
LIBXAR_SNAME := libxar.so.1.0
LIBXAR_LNAME :=
LIBXAR_L :=
endif
LIBXAR_LA := lib/$(LIBXAR_LANAME)
LIBXAR_A := lib/$(LIBXAR_ANAME)
LIBRXAR_S := lib/$(LIBRXAR_SNAME)
LIBXAR_S := lib/$(LIBXAR_SNAME)

#
# Include generated dependency files.
#
-include $(LIBXAR_SRCS:%.c=%.d)

LDFLAGS := -Llib $(LDFLAGS)

#
# User make'ables.
#

lib_all : lib_shared lib_static $(LIBXAR_LA)
ifeq (yes, yes)
lib_shared : $(LIBRXAR_S) $(LIBXAR_S)
else
lib_shared :
endif
ifeq (yes, yes)
lib_static : $(LIBXAR_A)
else
lib_static :
endif

lib_install : lib_shared lib_static
	.././install-sh -c -d $(DESTDIR)$(INCLUDEDIR)/xar
	.././install-sh -c -m 0644 $(LIBXAR_INCS) $(DESTDIR)$(INCLUDEDIR)/xar
	.././install-sh -c -d $(DESTDIR)$(LIBDIR)
ifeq (yes, yes)
	.././install-sh -c -m 0755 $(LIBXAR_S) $(DESTDIR)$(LIBDIR)
ifneq ($(words "" $(LIBXAR_LNAME)), 1)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBXAR_LNAME)
	ln -s $(LIBXAR_SNAME) $(DESTDIR)$(LIBDIR)/$(LIBXAR_LNAME)
endif
endif
ifeq (yes, yes)
	.././install-sh -c -m 0644 $(LIBXAR_A) $(DESTDIR)$(LIBDIR)
endif
	.././install-sh -c -m 0644 $(LIBXAR_LA) $(DESTDIR)$(LIBDIR)

lib_uninstall :
	rm -rf $(DESTDIR)$(INCLUDEDIR)/xar
ifeq (yes, yes)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBXAR_SNAME)
ifneq ($(words "" $(LIBXAR_LNAME)), 1)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBXAR_LNAME)
endif
endif
ifeq (yes, yes)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBXAR_ANAME)
endif
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBXAR_LANAME)

lib_clean :
	rm -f $(LIBRXAR_S) $(LIBRXAR_L)
	rm -f $(LIBXAR_S) $(LIBXAR_L)
	rm -f $(LIBXAR_A) $(LIBXAR_LA)
	@rm -f lib/.libs/$(LIBXAR_LNAME)
	@rm -f lib/.libs/$(LIBXAR_ANAME)
	@-rmdir lib/.libs
	rm -f $(LIBXAR_SRCS:%.c=%.o)
	rm -f $(LIBXAR_SRCS:%.c=%.d)
	rm -f $(LIBXAR_SRCS:%.c=%.static.o)
	rm -f $(LIBXAR_SRCS:%.c=%.static.d)

lib_distclean :

#
# Various flags.
#

CPPFLAGS := -Iinclude $(CPPFLAGS)
CPPFLAGS := -Iinclude $(CPPFLAGS)

#
# Build rules.
#

# librxar is a version of the xar library that is usable without first
# installing libxar.
$(LIBRXAR_S) : $(LIBXAR_SRCS:%.c=%.o)
	@mkdir -p $(@D)
ifeq (elf, macho)
	$(CC) -shared -Wl,-soname,$(LIBXAR_SNAME) -o $@ $+ $(LDFLAGS) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif
ifeq (macho, macho)
	$(CC) -dynamiclib -compatibility_version 1 -current_version 1 -install_name /Users/ariel/LinuxHome/code/darling/src/xar/xar/$(LIBRXAR_S) -o $@ $+ $(LDFLAGS) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif
ifeq (aout, macho)
	$(CC) -shared -o $@ $+
endif
ifneq ($(words "" $(LIBRXAR_L)), 1)
	rm -f $(LIBRXAR_L)
	ln -s $(LIBRXAR_SNAME) $(LIBRXAR_L)
endif

$(LIBXAR_S) : $(LIBXAR_SRCS:%.c=%.o)
	@mkdir -p $(@D)
ifeq (elf, macho)
	$(CC) -shared -Wl,-soname,$(LIBXAR_SNAME) -o $@ $+ $(LDFLAGS) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif
ifeq (macho, macho)
	$(CC) -dynamiclib -compatibility_version 1 -current_version 1 -install_name $(LIBDIR)/$(LIBXAR_SNAME) -o $@ $+ $(LDFLAGS) -lpthread -lbz2 -lz  -lxml2 -lz -lpthread -licucore -lm
endif
ifeq (aout, macho)
	$(CC) -shared -o $@ $+
endif
ifneq ($(words "" $(LIBXAR_L)), 1)
	rm -f $(LIBXAR_L)
	ln -s $(LIBXAR_SNAME) $(LIBXAR_L)
endif

ifeq (yes, yes)
LT_LIBXAR_SNAME := $(LIBXAR_SNAME)
LT_LIBXAR_LNAME := $(LIBXAR_LNAME)
else
LT_LIBXAR_SNAME :=
LT_LIBXAR_LNAME :=
endif
ifeq (yes, yes)
LT_LIBXAR_ANAME := $(LIBXAR_ANAME)
else
LT_LIBXAR_ANAME :=
endif

$(LIBXAR_LA) : $(LIBXAR_LA).in
	@mkdir -p lib/.libs
ifeq (yes, yes)
	@ln -sf ../$(LIBXAR_LNAME) lib/.libs/$(LIBXAR_LNAME)
endif
ifeq (yes, yes)
	@ln -sf ../$(LIBXAR_ANAME) lib/.libs/$(LIBXAR_ANAME)
endif
	sed -e s/@LIBXAR_SNAME@/$(LT_LIBXAR_SNAME)/ -e s/@LIBXAR_LNAME@/$(LT_LIBXAR_LNAME)/ -e s/@LIBXAR_ANAME@/$(LT_LIBXAR_ANAME)/ < $< > $@

$(LIBXAR_A) : $(LIBXAR_SRCS:%.c=%.static.o)
	@mkdir -p $(@D)
	$(AR) cvr $@ $+
	$(RANLIB) $@ 

lib/%.o : lib/%.c
	@mkdir -p $(@D)
	$(CC) $(S_CFLAGS) $(CPPFLAGS) -c $< -o $@
	@$(SHELL) -ec "$(CC) -MM $(CPPFLAGS) $< | sed \"s/\($(subst /,\/,$(notdir $(basename $@)))\)\.o\([ :]*\)/$(subst /,\/,$(strip $(dir $@)))\1.o \2/g\" > $(@:%.o=%.d)"

lib/%.static.o : lib/%.c
	@mkdir -p $(@D)
	$(CC) $(A_CFLAGS) $(CPPFLAGS) -c $< -o $@
	@$(SHELL) -ec "$(CC) -MM $(CPPFLAGS) $< | sed \"s/\($(subst /,\/,$(notdir $(basename $@)))\)\.o\([ :]*\)/$(subst /,\/,$(strip $(dir $@)))\1.o \2/g\" > $(@:%.o=%.d)"
