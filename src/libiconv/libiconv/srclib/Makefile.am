## Makefile for the srclib subdirectory of GNU libiconv
## Copyright (C) 2003-2006 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
## USA.

## Process this file with automake to produce Makefile.in.

AUTOMAKE_OPTIONS = 1.5 gnits no-dependencies
BUILT_SOURCES =
EXTRA_DIST =
MOSTLYCLEANFILES =

RM = rm -f

noinst_LIBRARIES = libicrt.a

# Sources that are compiled on all platforms.

libicrt_a_SOURCES = \
  allocsa.h allocsa.c \
  exit.h \
  pathmax.h \
  progname.h progname.c progreloc.c \
  unlocked-io.h \
  width.c uniwidth.h unitypes.h cjk.h streq.h \
  xalloc.h xmalloc.c xstrdup.c \
  xreadlink.h xreadlink.c

# Sources that are compiled only on platforms that lack the functions.

LIBADD_SOURCE = \
  canonicalize.h canonicalize.c \
  error.h error.c \
  memmove.c \
  readlink.c \
  relocatable.h relocatable.c \
  setenv.h setenv.c unsetenv.c \
  strerror.c

# How to build libicrt.a.

libicrt_a_LIBADD = @SRCLIBOBJS@
libicrt_a_DEPENDENCIES = @SRCLIBOBJS@

# List of files to be distributed.

EXTRA_DIST += \
  $(LIBADD_SOURCE) relocwrapper.c

INCLUDES = -I. -I$(srcdir)

DEFS = -DEXEEXT=\"$(EXEEXT)\" -DDEPENDS_ON_LIBICONV=1 -DDEPENDS_ON_LIBINTL=1 @DEFS@


# >>> gnulib module stdbool.
EXTRA_DIST += stdbool_.h

# The following is needed in order to create an <stdbool.h> when the system
# doesn't have one that works.
all-local $(libicrt_a_OBJECTS): @STDBOOL_H@
stdbool.h: stdbool_.h
	sed -e 's/@''HAVE__BOOL''@/@HAVE__BOOL@/g' < $(srcdir)/stdbool_.h > stdbool.h
MOSTLYCLEANFILES += stdbool.h
# <<< gnulib module stdbool.


# >>> gnulib module stdint.
BUILT_SOURCES += $(STDINT_H)
EXTRA_DIST += stdint_.h

# We need the following in order to create an <stdint.h> when the system
# doesn't have one that works with the given compiler.
all-local $(libicrt_a_OBJECTS): $(STDINT_H)
stdint.h: stdint_.h
	sed -e 's/@''HAVE_WCHAR_H''@/$(HAVE_WCHAR_H)/g' \
	    -e 's/@''HAVE_STDINT_H''@/$(HAVE_STDINT_H)/g' \
	    -e 's|@''FULL_PATH_STDINT_H''@|$(FULL_PATH_STDINT_H)|g' \
	    -e 's/@''HAVE_INTTYPES_H''@/$(HAVE_INTTYPES_H)/g' \
	    -e 's|@''FULL_PATH_INTTYPES_H''@|$(FULL_PATH_INTTYPES_H)|g' \
	    -e 's/@''HAVE_SYS_INTTYPES_H''@/$(HAVE_SYS_INTTYPES_H)/g' \
	    -e 's/@''HAVE_SYS_BITYPES_H''@/$(HAVE_SYS_BITYPES_H)/g' \
	    -e 's/@''HAVE_LONG_64BIT''@/$(HAVE_LONG_64BIT)/g' \
	    -e 's/@''HAVE_LONG_LONG_64BIT''@/$(HAVE_LONG_LONG_64BIT)/g' \
	    -e 's/@''HAVE_INT8_T''@/$(HAVE_INT8_T)/g' \
	    -e 's/@''HAVE_INT16_T''@/$(HAVE_INT16_T)/g' \
	    -e 's/@''HAVE_INT32_T''@/$(HAVE_INT32_T)/g' \
	    -e 's/@''HAVE_INT64_T''@/$(HAVE_INT64_T)/g' \
	    -e 's/@''HAVE_UINT8_T''@/$(HAVE_UINT8_T)/g' \
	    -e 's/@''HAVE_UINT16_T''@/$(HAVE_UINT16_T)/g' \
	    -e 's/@''HAVE_UINT32_T''@/$(HAVE_UINT32_T)/g' \
	    -e 's/@''HAVE_UINT64_T''@/$(HAVE_UINT64_T)/g' \
	    -e 's/@''HAVE_INT_LEAST8_T''@/$(HAVE_INT_LEAST8_T)/g' \
	    -e 's/@''HAVE_INT_LEAST16_T''@/$(HAVE_INT_LEAST16_T)/g' \
	    -e 's/@''HAVE_INT_LEAST32_T''@/$(HAVE_INT_LEAST32_T)/g' \
	    -e 's/@''HAVE_INT_LEAST64_T''@/$(HAVE_INT_LEAST64_T)/g' \
	    -e 's/@''HAVE_UINT_LEAST8_T''@/$(HAVE_UINT_LEAST8_T)/g' \
	    -e 's/@''HAVE_UINT_LEAST16_T''@/$(HAVE_UINT_LEAST16_T)/g' \
	    -e 's/@''HAVE_UINT_LEAST32_T''@/$(HAVE_UINT_LEAST32_T)/g' \
	    -e 's/@''HAVE_UINT_LEAST64_T''@/$(HAVE_UINT_LEAST64_T)/g' \
	    -e 's/@''HAVE_INT_FAST8_T''@/$(HAVE_INT_FAST8_T)/g' \
	    -e 's/@''HAVE_INT_FAST16_T''@/$(HAVE_INT_FAST16_T)/g' \
	    -e 's/@''HAVE_INT_FAST32_T''@/$(HAVE_INT_FAST32_T)/g' \
	    -e 's/@''HAVE_INT_FAST64_T''@/$(HAVE_INT_FAST64_T)/g' \
	    -e 's/@''HAVE_UINT_FAST8_T''@/$(HAVE_UINT_FAST8_T)/g' \
	    -e 's/@''HAVE_UINT_FAST16_T''@/$(HAVE_UINT_FAST16_T)/g' \
	    -e 's/@''HAVE_UINT_FAST32_T''@/$(HAVE_UINT_FAST32_T)/g' \
	    -e 's/@''HAVE_UINT_FAST64_T''@/$(HAVE_UINT_FAST64_T)/g' \
	    -e 's/@''HAVE_INTPTR_T''@/$(HAVE_INTPTR_T)/g' \
	    -e 's/@''HAVE_UINTPTR_T''@/$(HAVE_UINTPTR_T)/g' \
	    -e 's/@''HAVE_INTMAX_T''@/$(HAVE_INTMAX_T)/g' \
	    -e 's/@''HAVE_UINTMAX_T''@/$(HAVE_UINTMAX_T)/g' \
	    -e 's/@''BITSIZEOF_UNSIGNED_INT''@/$(BITSIZEOF_UNSIGNED_INT)/g' \
	    -e 's/@''BITSIZEOF_LONG''@/$(BITSIZEOF_LONG)/g' \
	    -e 's/@''BITSIZEOF_UNSIGNED_LONG''@/$(BITSIZEOF_UNSIGNED_LONG)/g' \
	    -e 's/@''BITSIZEOF_INT8_T''@/$(BITSIZEOF_INT8_T)/g' \
	    -e 's/@''BITSIZEOF_INT16_T''@/$(BITSIZEOF_INT16_T)/g' \
	    -e 's/@''BITSIZEOF_INT32_T''@/$(BITSIZEOF_INT32_T)/g' \
	    -e 's/@''BITSIZEOF_INT64_T''@/$(BITSIZEOF_INT64_T)/g' \
	    -e 's/@''BITSIZEOF_UINT8_T''@/$(BITSIZEOF_UINT8_T)/g' \
	    -e 's/@''BITSIZEOF_UINT16_T''@/$(BITSIZEOF_UINT16_T)/g' \
	    -e 's/@''BITSIZEOF_UINT32_T''@/$(BITSIZEOF_UINT32_T)/g' \
	    -e 's/@''BITSIZEOF_UINT64_T''@/$(BITSIZEOF_UINT64_T)/g' \
	    -e 's/@''BITSIZEOF_INT_LEAST8_T''@/$(BITSIZEOF_INT_LEAST8_T)/g' \
	    -e 's/@''BITSIZEOF_INT_LEAST16_T''@/$(BITSIZEOF_INT_LEAST16_T)/g' \
	    -e 's/@''BITSIZEOF_INT_LEAST32_T''@/$(BITSIZEOF_INT_LEAST32_T)/g' \
	    -e 's/@''BITSIZEOF_INT_LEAST64_T''@/$(BITSIZEOF_INT_LEAST64_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_LEAST8_T''@/$(BITSIZEOF_UINT_LEAST8_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_LEAST16_T''@/$(BITSIZEOF_UINT_LEAST16_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_LEAST32_T''@/$(BITSIZEOF_UINT_LEAST32_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_LEAST64_T''@/$(BITSIZEOF_UINT_LEAST64_T)/g' \
	    -e 's/@''BITSIZEOF_INT_FAST8_T''@/$(BITSIZEOF_INT_FAST8_T)/g' \
	    -e 's/@''BITSIZEOF_INT_FAST16_T''@/$(BITSIZEOF_INT_FAST16_T)/g' \
	    -e 's/@''BITSIZEOF_INT_FAST32_T''@/$(BITSIZEOF_INT_FAST32_T)/g' \
	    -e 's/@''BITSIZEOF_INT_FAST64_T''@/$(BITSIZEOF_INT_FAST64_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_FAST8_T''@/$(BITSIZEOF_UINT_FAST8_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_FAST16_T''@/$(BITSIZEOF_UINT_FAST16_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_FAST32_T''@/$(BITSIZEOF_UINT_FAST32_T)/g' \
	    -e 's/@''BITSIZEOF_UINT_FAST64_T''@/$(BITSIZEOF_UINT_FAST64_T)/g' \
	    -e 's/@''BITSIZEOF_INTPTR_T''@/$(BITSIZEOF_INTPTR_T)/g' \
	    -e 's/@''BITSIZEOF_UINTPTR_T''@/$(BITSIZEOF_UINTPTR_T)/g' \
	    -e 's/@''BITSIZEOF_INTMAX_T''@/$(BITSIZEOF_INTMAX_T)/g' \
	    -e 's/@''BITSIZEOF_UINTMAX_T''@/$(BITSIZEOF_UINTMAX_T)/g' \
	    -e 's/@''BITSIZEOF_PTRDIFF_T''@/$(BITSIZEOF_PTRDIFF_T)/g' \
	    -e 's/@''SAME_TYPE_PTRDIFF_T_LONG''@/$(SAME_TYPE_PTRDIFF_T_LONG)/g' \
	    -e 's/@''BITSIZEOF_SIG_ATOMIC_T''@/$(BITSIZEOF_SIG_ATOMIC_T)/g' \
	    -e 's/@''HAVE_SIGNED_SIG_ATOMIC_T''@/$(HAVE_SIGNED_SIG_ATOMIC_T)/g' \
	    -e 's/@''SAME_TYPE_SIG_ATOMIC_T_LONG''@/$(SAME_TYPE_SIG_ATOMIC_T_LONG)/g' \
	    -e 's/@''SAME_TYPE_SIG_ATOMIC_T_UNSIGNED_LONG''@/$(SAME_TYPE_SIG_ATOMIC_T_UNSIGNED_LONG)/g' \
	    -e 's/@''BITSIZEOF_SIZE_T''@/$(BITSIZEOF_SIZE_T)/g' \
	    -e 's/@''SAME_TYPE_SIZE_T_UNSIGNED_LONG''@/$(SAME_TYPE_SIZE_T_UNSIGNED_LONG)/g' \
	    -e 's/@''BITSIZEOF_WCHAR_T''@/$(BITSIZEOF_WCHAR_T)/g' \
	    -e 's/@''HAVE_SIGNED_WCHAR_T''@/$(HAVE_SIGNED_WCHAR_T)/g' \
	    -e 's/@''SAME_TYPE_WCHAR_T_LONG''@/$(SAME_TYPE_WCHAR_T_LONG)/g' \
	    -e 's/@''SAME_TYPE_WCHAR_T_UNSIGNED_LONG''@/$(SAME_TYPE_WCHAR_T_UNSIGNED_LONG)/g' \
	    -e 's/@''BITSIZEOF_WINT_T''@/$(BITSIZEOF_WINT_T)/g' \
	    -e 's/@''HAVE_SIGNED_WINT_T''@/$(HAVE_SIGNED_WINT_T)/g' \
	    -e 's/@''SAME_TYPE_WINT_T_LONG''@/$(SAME_TYPE_WINT_T_LONG)/g' \
	    -e 's/@''SAME_TYPE_WINT_T_UNSIGNED_LONG''@/$(SAME_TYPE_WINT_T_UNSIGNED_LONG)/g' \
	    < $(srcdir)/stdint_.h > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += stdint.h stdint.h-t
# <<< gnulib module stdint.


# >>> gnulib module alloca.
EXTRA_DIST += alloca_.h

# The following is needed in order to create an <alloca.h> when the system
# doesn't have one that works with the given compiler.
all-local $(libicrt_a_OBJECTS): @ALLOCA_H@
alloca.h: alloca_.h
	cp $(srcdir)/alloca_.h alloca.h
MOSTLYCLEANFILES += alloca.h
# <<< gnulib module alloca.


# >>> gnulib module unistd.
BUILT_SOURCES += $(UNISTD_H)

# We need the following in order to create an empty placeholder for
# <unistd.h> when the system doesn't have one.
unistd.h:
	echo '/* Empty placeholder for $@.  */' >$@
MOSTLYCLEANFILES += unistd.h
# <<< gnulib module unistd.


# Woe32 support.

EXTRA_DIST += Makefile.msvc
