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

#define _GNU_SOURCE

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

size_t mbstowcs_l(wchar_t* pwcs, const char* s, size_t n, void* l) {
  return mbstowcs(pwcs, s, n);
}

size_t wcswidth_l(wchar_t* pwcs, size_t n, void* l) {
  return wcswidth(pwcs, n);
}

void libiconv_set_relocation_prefix(const char* orig, const char* curr) {
  // TODO: What should we do?
  abort();
}

void *__darwin_mmap(void *addr, size_t length, int prot, int flags,
                    int fd, off_t offset) {
  LOGF("mmap: addr=%p length=%lu prot=%d flags=%d fd=%d offset=%lu\n",
       addr, (unsigned long)length, prot, flags, fd, offset);

  // MAP_ANON is 0x1000 on darwin but 0x20 on linux.
  //
  // The following flags are darwin only.
  // #define MAP_RENAME       0x0020 /* Sun: rename private pages to file */
  // #define MAP_NORESERVE    0x0040 /* Sun: don't reserve needed swap area */
  // #define MAP_RESERVED0080 0x0080 /* previously unimplemented MAP_INHERIT */
  // #define MAP_NOEXTEND     0x0100 /* for MAP_FILE, don't change file size */
  // #define MAP_HASSEMAPHORE 0x0200 /* region may contain semaphores */
  // #define MAP_NOCACHE      0x0400 /* don't cache pages for this mapping */
  flags = (flags & 0x1f) | (flags & 0x1000 ? MAP_ANONYMOUS : 0);
  return mmap(addr, length, prot, flags, fd, offset);
}

extern char* __loader_path;

static char** add_loader_to_argv(char* argv[]) {
  int i, argc;
  for (argc = 0; argv[argc]; argc++);
  LOGF("\n");
  char** new_argv = malloc(sizeof(char*) * (argc + 2));
  new_argv[0] = __loader_path;
  for (i = 0; i < argc + 1; i++) {
    new_argv[i + 1] = argv[i];
  }
  return new_argv;
}

// It seems the size of posix_spawn_file_actions_t of Mac is smaller
// than Linux so modifying posix_spawn_file_actions_t allocated in
// Mac's stack would cause stack overflow. Let's wrap it.

int __darwin_posix_spawn(pid_t* pid,
                         const char* path,
                         const posix_spawn_file_actions_t** file_actions,
                         const posix_spawnattr_t* attrp,
                         char* argv[],
                         char* const envp[]) {
  char** new_argv = add_loader_to_argv(argv);
  const posix_spawn_file_actions_t* fa = NULL;
  if (file_actions)
    fa = *file_actions;
  int r = posix_spawn(pid,
                      __loader_path,
                      fa,
                      attrp,
                      new_argv,
                      envp);
  free(new_argv);
  return r;
}

int __darwin_posix_spawnp(pid_t *pid,
                          const char* file,
                          const posix_spawn_file_actions_t* file_actions,
                          const posix_spawnattr_t* attrp,
                          char* const argv[],
                          char* const envp[]) {
  err(1, "posix_spawnp is not implemented yet\n");
  return 0;
}

int __darwin_posix_spawn_file_actions_init(posix_spawn_file_actions_t** p) {
  *p = (posix_spawn_file_actions_t*)malloc(sizeof(posix_spawn_file_actions_t));
  return posix_spawn_file_actions_init(*p);
}

int __darwin_posix_spawn_file_actions_destroy(posix_spawn_file_actions_t** p) {
  int r = posix_spawn_file_actions_destroy(*p);
  free(*p);
  return r;
}

int __darwin_posix_spawn_file_actions_addopen(
    posix_spawn_file_actions_t** file_actions,
    int fd,
    const char* path,
    int oflag,
    int mode) {
    return posix_spawn_file_actions_addopen(
        *file_actions, fd, path, oflag, mode);
}

int __darwin_posix_spawn_file_actions_addclose(
    posix_spawn_file_actions_t** file_actions,
    int fd) {
    return posix_spawn_file_actions_addclose(*file_actions, fd);
}

int __darwin_posix_spawn_file_actions_adddup2(
    posix_spawn_file_actions_t** file_actions,
    int fd,
    int newfd) {
    return posix_spawn_file_actions_adddup2(*file_actions, fd, newfd);
}

void __assert_rtn(const char* func, const char* file, int line,
                  const char* failedexpr) {
  fprintf(stderr,
          "Assertion failed: (%s), function %s, file %s, line %d.\n",
          failedexpr, func, file, line);
  abort();
}

// We don't have DWARF to handle exception. Let's show some info and abort.
// This works well for apple's ld.
void __darwin___cxa_throw(char** obj) {
  fprintf(stderr, "__cxa_throw: obj=%p\n", obj);
  fprintf(stderr, "%s\n", *obj);
  abort();
}

typedef struct {
  void* ss_sp;
  size_t ss_size;
  int ss_flags;
} __darwin_stack_t;

int __darwin_sigaltstack(const __darwin_stack_t* ss, __darwin_stack_t* oss) {
  return 0;

#if 0
  stack_t linux_ss;
  stack_t linux_oss;
  linux_ss.ss_sp = ss->ss_sp;
  linux_ss.ss_size = ss->ss_size;
  linux_ss.ss_flags = ss->ss_flags;
  if (oss) {
    linux_oss.ss_sp = oss->ss_sp;
    linux_oss.ss_size = oss->ss_size;
    linux_oss.ss_flags = oss->ss_flags;
  }
  int r = sigaltstack(&linux_ss, &linux_oss);
  if (oss) {
    oss->ss_sp = linux_oss.ss_sp;
    oss->ss_size = linux_oss.ss_size;
    oss->ss_flags = linux_oss.ss_flags;
  }
  return r;
#endif
}

int __darwin_sigaction() {
  return 0;
}

void* __darwin_signal() {
  return NULL;
}

typedef struct malloc_statistics_t {
  unsigned blocks_in_use;
  size_t size_in_use;
  size_t max_size_in_use;
  size_t size_allocated;
} __darwin_malloc_statistics_t;

void* malloc_default_zone() {
  return NULL;
}

// TODO: implement via mallinfo
void malloc_zone_statistics(void* zone, __darwin_malloc_statistics_t* stats) {
  fprintf(stderr, "malloc_zone_statistics\n");
  memset(stats, 0, sizeof(*stats));
}

int task_get_exception_ports() {
  fprintf(stderr, "task_get_exception_ports\n");
  return 0;
}

int task_set_exception_ports() {
  fprintf(stderr, "task_set_exception_ports\n");
  return 0;
}



static void do_nothing(void) {
}
static void* do_nothing2(void) {
	return 0;
}
static void do_nothing3(int status) {
}

void (*mach_init_routine)(void) = do_nothing;
void* (*_cthread_init_routine)(void) = do_nothing2;
void (*_cthread_exit_routine) (int) = do_nothing3;

void __keymgr_dwarf2_register_sections() {
}

#define __DARWIN_SYS_NAMELEN 256
typedef struct {
	char sysname[__DARWIN_SYS_NAMELEN];
	char nodename[__DARWIN_SYS_NAMELEN];
	char release[__DARWIN_SYS_NAMELEN];
	char version[__DARWIN_SYS_NAMELEN];
	char machine[__DARWIN_SYS_NAMELEN];
} __darwin_utsname;

int __darwin_uname(__darwin_utsname* buf) {
	struct utsname linux_buf;
	int r = uname(&linux_buf);
	if (r)
	return r;

	// Linux's buf size is currently much smaller, only 65, so strcpy() is safe
	strcpy(buf->sysname, linux_buf.sysname);
	strcpy(buf->nodename, linux_buf.nodename);
	strcpy(buf->release, linux_buf.release);
	strcpy(buf->version, linux_buf.version);
	strcpy(buf->machine, linux_buf.machine);
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
