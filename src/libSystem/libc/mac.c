// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

// Emulation for functions in Mac.

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "config.h"
#include <dirent.h>
#include <err.h>
#include <locale.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <mac-ctype.h>
#include <runetype.h>

#ifdef NOLOG
# define LOGF(...) if (0) fprintf(stderr, __VA_ARGS__)
#else
int LIBMAC_LOG;
# define LOGF(...) if (LIBMAC_LOG) fprintf(stderr, __VA_ARGS__)
#endif

typedef __darwin_rune_t rune_t;
typedef int __darwin_ct_rune_t;     /* ct_rune_t */

int __maskrune(__darwin_ct_rune_t _c, unsigned long _f) {
  return _DefaultRuneLocale.__runetype[_c & 0xff] & _f;
}

int __maskrune_l(__darwin_ct_rune_t _c, unsigned long _f, void* l) {
  return __maskrune(_c, _f);
}

void libiconv_set_relocation_prefix(const char* orig, const char* curr) {
}

int task_get_exception_ports() {
  fprintf(stderr, "task_get_exception_ports\n");
  return 0;
}

int task_set_exception_ports() {
  fprintf(stderr, "task_set_exception_ports\n");
  return 0;
}

#define __DARWIN_LC_ALL_MASK            (  __DARWIN_LC_COLLATE_MASK \
                                         | __DARWIN_LC_CTYPE_MASK \
                                         | __DARWIN_LC_MESSAGES_MASK \
                                         | __DARWIN_LC_MONETARY_MASK \
                                         | __DARWIN_LC_NUMERIC_MASK \
                                         | __DARWIN_LC_TIME_MASK )
#define __DARWIN_LC_COLLATE_MASK        (1 << 0)
#define __DARWIN_LC_CTYPE_MASK          (1 << 1)
#define __DARWIN_LC_MESSAGES_MASK       (1 << 2)
#define __DARWIN_LC_MONETARY_MASK       (1 << 3)
#define __DARWIN_LC_NUMERIC_MASK        (1 << 4)
#define __DARWIN_LC_TIME_MASK           (1 << 5)

locale_t __darwin_newlocale(int category_mask, const char* locale,
                            locale_t base) {
  LOGF("newlocale: %d %p %p\n", category_mask, locale, base);
  int linux_category_mask = 0;
  if (__DARWIN_LC_COLLATE_MASK & category_mask)
    linux_category_mask |= LC_COLLATE_MASK;
  if (__DARWIN_LC_CTYPE_MASK & category_mask)
    linux_category_mask |= LC_CTYPE_MASK;
  if (__DARWIN_LC_MESSAGES_MASK & category_mask)
    linux_category_mask |= LC_MESSAGES_MASK;
  if (__DARWIN_LC_MONETARY_MASK & category_mask)
    linux_category_mask |= LC_MONETARY_MASK;
  if (__DARWIN_LC_NUMERIC_MASK & category_mask)
    linux_category_mask |= LC_NUMERIC_MASK;
  if (__DARWIN_LC_TIME_MASK & category_mask)
    linux_category_mask |= LC_TIME_MASK;
  // Apple's newlocale allows base=LC_GLOBAL_LOCALE, while glibc crashes.
  // It seems the behavior is unspecified for this case.
  // http://pubs.opengroup.org/onlinepubs/9699919799/functions/newlocale.html
  // We'll use current locale (NULL) instead of the global locale.
  if (base == LC_GLOBAL_LOCALE)
    base = NULL;
  // It seems the following 5 locales are the same as "C" for Mac.
  if (!strcmp(locale, "en_US") ||
      !strcmp(locale, "en_US.ISO8859-1") ||
      !strcmp(locale, "en_US.ISO8859-15") ||
      !strcmp(locale, "en_US.US-ASCII") ||
      !strcmp(locale, "en_US.UTF-8"))
    locale = "C";
  return newlocale(linux_category_mask, locale, base);
}

int __darwin_compat_mode(const char* function, const char* mode) {
  LOGF("compat_mode: %s %s\n", function, mode);
  // We don't consider bootstrap, legacy, and error.
  // http://opensource.apple.com/source/Libc/Libc-763.13/gen/get_compat.c
  // TODO(hamaji): Support binary operators.
  return !strcasecmp(mode, "unix2003");
}
