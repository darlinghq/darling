/*
 * Copyright (c) 2011 Mark Heily <mark@heily.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef  _DEBUG_H
#define  _DEBUG_H

#include <assert.h>
#include <stdio.h>
#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

extern int DEBUG_KQUEUE;
extern char *KQUEUE_DEBUG_IDENT;

#if defined(__linux__)
# include <sys/syscall.h>
# define THREAD_ID ((pid_t)  syscall(__NR_gettid))
#elif defined(__sun)
# define THREAD_ID ((int) pthread_self())
#elif defined(_WIN32)
# define THREAD_ID (int)(GetCurrentThreadId())
#else 
# error Unsupported platform
#endif

#ifndef NDEBUG
#define dbg_puts(str)           do {                                \
    if (DEBUG_KQUEUE)                                                      \
      fprintf(stderr, "%s [%d]: %s(): %s\n",                        \
              KQUEUE_DEBUG_IDENT, THREAD_ID, __func__, str);               \
} while (0)

#define dbg_printf(fmt,...)     do {                                \
    if (DEBUG_KQUEUE)                                                      \
      fprintf(stderr, "%s [%d]: %s(): "fmt"\n",                     \
              KQUEUE_DEBUG_IDENT, THREAD_ID, __func__, __VA_ARGS__);       \
} while (0)

#define dbg_perror(str)         do {                                \
    if (DEBUG_KQUEUE)                                                      \
      fprintf(stderr, "%s [%d]: %s(): %s: %s (errno=%d)\n",         \
              KQUEUE_DEBUG_IDENT, THREAD_ID, __func__, str,                \
              strerror(errno), errno);                              \
} while (0)

# define reset_errno()          do { errno = 0; } while (0)

# if defined(_WIN32)
#  define dbg_lasterror(str)     do {                                \
    if (DEBUG_KQUEUE)                                                      \
      fprintf(stderr, "%s: [%d] %s(): %s: (LastError=%d)\n",        \
              KQUEUE_DEBUG_IDENT, THREAD_ID, __func__, str, (int)GetLastError());            \
} while (0)

#  define dbg_wsalasterror(str)  do {                                \
    if (DEBUG_KQUEUE)                                                      \
      fprintf(stderr, "%s: [%d] %s(): %s: (WSALastError=%d)\n",        \
              KQUEUE_DEBUG_IDENT, THREAD_ID, __func__, str, (int)WSAGetLastError());            \
} while (0)

# else
#  define dbg_lasterror(str)     ;
#  define dbg_wsalasterror(str)  ;
# endif

/*
 * Tracing mutexes are a thin wrapper around the pthread_mutex_t 
 * datatype that tracks and reports when a mutex is locked or unlocked.
 * It also allows you to assert that a mutex has (or has not) been locked
 * by calling tracing_mutex_assert().
 */

# define MTX_UNLOCKED    0
# define MTX_LOCKED      1

typedef struct {
    pthread_mutex_t mtx_lock; 
    int mtx_status; 
    int mtx_owner;
} tracing_mutex_t; 

# define tracing_mutex_init(mtx, attr) do { \
    pthread_mutex_init(&(mtx)->mtx_lock, (attr)); \
    (mtx)->mtx_status = MTX_UNLOCKED; \
    (mtx)->mtx_owner = -1; \
} while (0)

# define tracing_mutex_destroy(mtx) pthread_mutex_destroy(&(mtx)->mtx_lock)

# define tracing_mutex_assert(x,y) do { \
  if ((y) == MTX_UNLOCKED) \
      assert((x)->mtx_status == MTX_UNLOCKED || (x)->mtx_owner != THREAD_ID); \
  else if ((y) == MTX_LOCKED) \
      assert((x)->mtx_status == MTX_LOCKED && (x)->mtx_owner == THREAD_ID); \
  else \
    abort(); \
} while (0)

# define tracing_mutex_lock(x)  do { \
    dbg_printf("waiting for %s", #x); \
    pthread_mutex_lock(&((x)->mtx_lock)); \
    dbg_printf("locked %s", #x); \
    (x)->mtx_owner = THREAD_ID; \
    (x)->mtx_status = MTX_LOCKED; \
} while (0)

# define tracing_mutex_unlock(x)  do { \
    (x)->mtx_status = MTX_UNLOCKED; \
    (x)->mtx_owner = -1; \
    pthread_mutex_unlock(&((x)->mtx_lock)); \
    dbg_printf("unlocked %s", # x); \
} while (0)

#else /* NDEBUG */
# define dbg_puts(str)           do {} while (0)
# define dbg_printf(fmt,...)     do {} while (0)
# define dbg_perror(str)         do {} while (0)
# define dbg_lasterror(str)      do {} while (0)
# define dbg_wsalasterror(str)   do {} while (0)
# define reset_errno()           do {} while (0)
# define MTX_UNLOCKED                
# define MTX_LOCKED                 
# define tracing_mutex_t            pthread_mutex_t
# define tracing_mutex_init         pthread_mutex_init
# define tracing_mutex_destroy      pthread_mutex_destroy
# define tracing_mutex_assert(x,y)  do {} while (0)
# define tracing_mutex_lock         pthread_mutex_lock
# define tracing_mutex_unlock       pthread_mutex_unlock
#endif 

#endif  /* ! _DEBUG_H */
