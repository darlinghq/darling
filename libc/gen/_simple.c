/*
 * Copyright (c) 2005, 2006, 2009 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef VARIANT_DYLD
#include <setjmp.h>
#endif /* !VARIANT_DYLD */
#include <sys/types.h>
#include <unistd.h>
#include <mach/mach_init.h>
#include <mach/vm_map.h>
#include <asl.h>
#include <fcntl.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <errno.h>
#include <servers/bootstrap.h>
#include <asl_ipc.h>

#include "_simple.h"

#ifndef VM_PAGE_SIZE
#define VM_PAGE_SIZE	4096
#endif

#define SBUF_SIZE(s)	(((SBUF *)(s))->b.end - ((SBUF *)(s))->b.buf + 1)
/* we use a small buffer to minimize stack usage constraints */
#define MYBUFSIZE	32

typedef struct _BUF {
    char *buf;
    char *ptr;
    char *end;
    int fd;
    void (*full)(struct _BUF *);
} BUF;

typedef struct _SBUF {
    BUF b;
#ifndef VARIANT_DYLD
    jmp_buf j;
#endif /* !VARIANT_DYLD */
} SBUF;

#define ASL_SERVICE_NAME "com.apple.system.logger"

static mach_port_t asl_port;
static pthread_once_t asl_init_once = PTHREAD_ONCE_INIT;

/* private extern exports from asl.c */
const char *_asl_escape(unsigned char);

/* flush the buffer */
static void
_flush(BUF *b)
{
    char *buf = b->buf;
    int n = b->ptr - buf;
    int w;

    while(n > 0) {
	w = write(b->fd, buf, n);
	if(w < 0) {
	    if(errno == EINTR || errno == EAGAIN)
		continue;
	    break;
	}
	n -= w;
	buf += n;
    }
}

/* flush the buffer and reset the pointer */
static void
_flush_reset(BUF *b)
{
    _flush(b);
    b->ptr = b->buf;
}

/* enlarge the buffer */
static void
_enlarge(BUF *b)
{
    vm_address_t new;
    vm_size_t sold, snew;
    intptr_t diff;

    new = (vm_address_t)(b->end + 1);
    if(vm_allocate(mach_task_self(), &new, VM_PAGE_SIZE, 0) == 0) {
	/* page is adjacent */
	b->end += VM_PAGE_SIZE;
	return;
    }
    sold = SBUF_SIZE(b);
    snew = (sold + VM_PAGE_SIZE) & ~(VM_PAGE_SIZE - 1);
    if(vm_allocate(mach_task_self(), &new, snew, 1) != 0)
#ifndef VARIANT_DYLD
	longjmp(((SBUF *)b)->j, 1); /* out of memory */
#else /* VARIANT_DYLD */
	abort(); /* out of memory */
#endif /* !VARIANT_DYLD */
    diff = new - (vm_address_t)b->buf;
    memcpy((void *)new, b->buf, sold);
    if((intptr_t)(b->buf) & (VM_PAGE_SIZE - 1)) {
	sold &= ~(VM_PAGE_SIZE - 1);
	b->buf = (char *)((intptr_t)(b->buf + VM_PAGE_SIZE) & ~(VM_PAGE_SIZE - 1));
	b->end = (char *)(new + snew - 1);
    } else
	b->end += diff + VM_PAGE_SIZE;
    if(sold > 0) {
	vm_deallocate(mach_task_self(), (vm_address_t)b->buf, sold);
    }
    b->buf = (char *)new;
    b->ptr += diff;
}

static inline void put_s(BUF *, _esc_func, const char *);
/* output a single character */
static inline void
put_c(BUF *b, _esc_func esc, unsigned char c)
{
    const char *cp;

    if(esc && (cp = esc(c)) != NULL)
	put_s(b, NULL, cp);
    else {
	if(b->ptr >= b->end)
	    b->full(b);
	*b->ptr++ = c;
    }
}

/* output a null-terminated string */
static inline void
put_s(BUF *b, _esc_func esc, const char *str)
{
    while(*str)
	put_c(b, esc, *str++);
}

/* output a string of the specified size */
static inline void
put_n(BUF *b, _esc_func esc, const char *str, int n)
{
    while(n-- > 0)
	put_c(b, esc, *str++);
}

/*
 * Output the signed decimal string representing the number in "in".  "width" is
 * the minimum field width, and "zero" is a boolean value, true for zero padding
 * (otherwise blank padding).
 */
static void
dec(BUF *b, _esc_func esc, long long in, int width, int zero)
{
    char buf[32];
    char *cp = buf + sizeof(buf);
    int pad;
    int neg = 0;
    unsigned long long n = (unsigned long long)in;

    if(in < 0) {
	neg++;
	width--;
	n = ~n + 1;
    }
    *--cp = 0;
    if(n) {
	while(n) {
	    *--cp = (n % 10) + '0';
	    n /= 10;
	}
    } else
	*--cp = '0';
    if(neg && zero) {
	put_c(b, esc, '-');
	neg = 0;
    }
    pad = width - strlen(cp);
    zero = zero ? '0' : ' ';
    while(pad-- > 0)
	put_c(b, esc, zero);
    if(neg)
	put_c(b, esc, '-');
    put_s(b, esc, cp);
}

/*
 * Output the hex string representing the number in "n".  "width" is the
 * minimum field width, and "zero" is a boolean value, true for zero padding
 * (otherwise blank padding).  "upper" is a boolean value, true for upper
 * case hex characters, lower case otherwise.  "p" is a boolean value, true
 * if 0x should be prepended (for %p), otherwise nothing.
 */
static char _h[] = "0123456789abcdef";
static char _H[] = "0123456789ABCDEF";
static char _0x[] = "0x";

static void
hex(BUF *b, _esc_func esc, unsigned long long n, int width, int zero, int upper, int p)
{
    char buf[32];
    char *cp = buf + sizeof(buf);
    char *h = upper ? _H : _h;

    *--cp = 0;
    if(n) {
	while(n) {
	    *--cp = h[n & 0xf];
	    n >>= 4;
	}
    } else
	*--cp = '0';
    if(p) {
	width -= 2;
	if(zero) {
	    put_s(b, esc, _0x);
	    p = 0;
	}
    }
    width -= strlen(cp);
    zero = zero ? '0' : ' ';
    while(width-- > 0)
	put_c(b, esc, zero);
    if(p)
	put_s(b, esc, _0x);
    put_s(b, esc, cp);
}

/*
 * Output the unsigned decimal string representing the number in "n".  "width"
 * is the minimum field width, and "zero" is a boolean value, true for zero
 * padding (otherwise blank padding).
 */
static void
udec(BUF *b, _esc_func esc, unsigned long long n, int width, int zero)
{
    char buf[32];
    char *cp = buf + sizeof(buf);
    int pad;

    *--cp = 0;
    if(n) {
	while(n) {
	    *--cp = (n % 10) + '0';
	    n /= 10;
	}
    } else
	*--cp = '0';
    pad = width - strlen(cp);
    zero = zero ? '0' : ' ';
    while(pad-- > 0)
	put_c(b, esc, zero);
    put_s(b, esc, cp);
}

/*
 * Output the unsigned decimal string representing the number in "n", rounded
 * to the nearest MB, KB or b.  "width" is the minimum field width, and "zero"
 * is a boolean value, true for zero padding (otherwise blank padding).
 */
static void
ydec(BUF *b, _esc_func esc, unsigned long long n, int width, int zero)
{
    if(n >= 10 * (1 << 20)) {
	n += (1 << 19);
	udec(b, esc, n >> 20, width, zero);
	put_s(b, esc, "MB");
    } else if (n >= 10 * (1 << 10)) {
	n += (1 << 9);
	udec(b, esc, n >> 10, width, zero);
	put_s(b, esc, "KB");
    } else {
	udec(b, esc, n, width, zero);
	put_s(b, esc, "b");
    }
}

/*
 * The actual engine for all the _simple_*printf routines.
 */
static void
__simple_bprintf(BUF *b, _esc_func esc, const char *fmt, va_list ap)
{
    while(*fmt) {
	int lflag, zero, width;
	char *cp;
	if(!(cp = strchr(fmt, '%'))) {
	    put_s(b, esc, fmt);
	    break;
	}
	put_n(b, esc, fmt, cp - fmt);
	fmt = cp + 1;
	if(*fmt == '%') {
	    put_c(b, esc, '%');
	    fmt++;
	    continue;
	}
	lflag = zero = width = 0;
	for(;;) {
	    switch(*fmt) {
	    case '0':
		zero++;
		fmt++;
		/* drop through */
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9':
		while(*fmt >= '0' && *fmt <= '9')
		    width = 10 * width + (*fmt++ - '0');
		continue;
	    case 'c':
		zero = zero ? '0' : ' ';
		width--;
		while(width-- > 0)
		    put_c(b, esc, zero);
		put_c(b, esc, va_arg(ap, int));
		break;
	    case 'd': case 'i':
		switch(lflag) {
		case 0:
		    dec(b, esc, va_arg(ap, int), width, zero);
		    break;
		case 1:
		    dec(b, esc, va_arg(ap, long), width, zero);
		    break;
		default:
		    dec(b, esc, va_arg(ap, long long), width, zero);
		    break;
		}
		break;
	    case 'l':
		lflag++;
		fmt++;
		continue;
	    case 'p':
		hex(b, esc, (unsigned long)va_arg(ap, void *), width, zero, 0, 1);
		break;
	    case 's':
		cp = va_arg(ap, char *);
		width -= strlen(cp);
		zero = zero ? '0' : ' ';
		while(width-- > 0)
		    put_c(b, esc, zero);
		put_s(b, esc, cp);
		break;
	    case 'u':
		switch(lflag) {
		case 0:
		    udec(b, esc, va_arg(ap, unsigned int), width, zero);
		    break;
		case 1:
		    udec(b, esc, va_arg(ap, unsigned long), width, zero);
		    break;
		default:
		    udec(b, esc, va_arg(ap, unsigned long long), width, zero);
		    break;
		}
		break;
	    case 'X': case 'x':
		switch(lflag) {
		case 0:
		    hex(b, esc, va_arg(ap, unsigned int), width, zero,
			*fmt == 'X', 0);
		    break;
		case 1:
		    hex(b, esc, va_arg(ap, unsigned long), width, zero,
			*fmt == 'X', 0);
		    break;
		default:
		    hex(b, esc, va_arg(ap, unsigned long long), width, zero,
			*fmt == 'X', 0);
		    break;
		}
		break;
	    case 'y':
		switch(lflag) {
		case 0:
		    ydec(b, esc, va_arg(ap, unsigned int), width, zero);
		    break;
		case 1:
		    ydec(b, esc, va_arg(ap, unsigned long), width, zero);
		    break;
		default:
		    ydec(b, esc, va_arg(ap, unsigned long long), width, zero);
		    break;
		}
		break;
	    default:
		put_c(b, esc, *fmt);
		break;
	    }
	    break;
	}
	fmt++;
    }
}

/*
 * A simplified vfprintf variant.  The format string is interpreted with
 * arguments from the va_list, and the results are written to the given
 * file descriptor.
 */
void
_simple_vdprintf(int fd, const char *fmt, va_list ap)
{
    BUF b;
    char buf[MYBUFSIZE];

    b.buf = buf;
    b.fd = fd;
    b.ptr = b.buf;
    b.end = b.buf + MYBUFSIZE;
    b.full = _flush_reset;
    __simple_bprintf(&b, NULL, fmt, ap);
    _flush(&b);
}

/*
 * A simplified fprintf variant.  The format string is interpreted with
 * arguments from the variable argument list, and the results are written
 * to the given file descriptor.
 */
void
_simple_dprintf(int fd, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    _simple_vdprintf(fd, fmt, ap);
    va_end(ap);
}

/*
 * A simplified string allocate routine.  Pass the opaque pointer to structure
 * to _simple_*sprintf() routines.  Use _simple_string() to retrieve the
 * current string (the string is guaranteed to be null terminated only on
 * the call to _simple_string()).  Use _simple_sfree() to free the structure
 * and string memory.
 */
_SIMPLE_STRING
_simple_salloc(void)
{
    SBUF *b;

    if(vm_allocate(mach_task_self(), (vm_address_t *)&b, VM_PAGE_SIZE, 1))
	return NULL;
    b->b.ptr = b->b.buf = (char *)b + sizeof(SBUF);
    b->b.end = (char *)b + VM_PAGE_SIZE - 1;
    b->b.full = _enlarge;
    return (_SIMPLE_STRING)b;
}

/*
 * The format string is interpreted with arguments from the va_list, and the
 * results are appended to the string maintained by the opaque structure, as
 * returned by a previous call to _simple_salloc().  Non-zero is returned on
 * out-of-memory error.
 */
int
_simple_vsprintf(_SIMPLE_STRING b, const char *fmt, va_list ap)
{
    return _simple_vesprintf(b, NULL, fmt, ap);
}

/*
 * The format string is interpreted with arguments from the variable argument
 * list, and the results are appended to the string maintained by the opaque
 * structure, as returned by a previous call to _simple_salloc().  Non-zero is
 * returned on out-of-memory error.
 */
int
_simple_sprintf(_SIMPLE_STRING b, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = _simple_vesprintf(b, NULL, fmt, ap);
    va_end(ap);
    return ret;
}

/*
 * Like _simple_vsprintf(), except __esc is a function to call on each
 * character; the function returns NULL if the character should be passed
 * as is, otherwise, the returned character string is used instead.
 */
int
_simple_vesprintf(_SIMPLE_STRING b, _esc_func esc, const char *fmt, va_list ap)
{
#ifndef VARIANT_DYLD
    if(setjmp(((SBUF *)b)->j))
	return -1;
#endif /* !VARIANT_DYLD */
    __simple_bprintf((BUF *)b, esc, fmt, ap);
    return 0;
}

/*
 * Like _simple_sprintf(), except __esc is a function to call on each
 * character; the function returns NULL if the character should be passed
 * as is, otherwise, the returned character string is used instead.
 */
int _simple_esprintf(_SIMPLE_STRING b, _esc_func esc, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = _simple_vesprintf(b, esc, fmt, ap);
    va_end(ap);
    return ret;
}

/*
 * Return the null terminated string from the opaque structure, as returned
 * by a previous call to _simple_salloc().
 */
char *
_simple_string(_SIMPLE_STRING b)
{
    *((BUF *)b)->ptr = 0;
    return ((BUF *)b)->buf;
}

/*
 * Reposition the pointer to the first null in the buffer.  After a call to
 * _simple_string, the buffer can be modified, and shrunk.
 */
void
_simple_sresize(_SIMPLE_STRING b)
{
    ((BUF *)b)->ptr = ((BUF *)b)->buf + strlen(((BUF *)b)->buf);
}

/*
 * Append the null-terminated string to the string associated with the opaque
 * structure.  Non-zero is returned on out-of-memory error.
 */
int
_simple_sappend(_SIMPLE_STRING b, const char *str)
{
    return _simple_esappend(b, NULL, str);
}

/*
 * Like _simple_sappend(), except __esc is a function to call on each
 * character; the function returns NULL if the character should be passed
 * as is, otherwise, the returned character string is used instead.
 */
int _simple_esappend(_SIMPLE_STRING b, _esc_func esc, const char *str)
{
#ifndef VARIANT_DYLD
    if(setjmp(((SBUF *)b)->j))
	return -1;
#endif /* !VARIANT_DYLD */
    put_s((BUF *)b, esc, str);
    return 0;
}

/*
 * Write the string associated with the opaque structure to the file descriptor.
 */
void
_simple_put(_SIMPLE_STRING b, int fd)
{
    ((BUF *)b)->fd = fd;
    _flush((BUF *)b);
}

/*
 * Write the string associated with the opaque structure and a trailing newline,
 * to the file descriptor.
 */
void
_simple_putline(_SIMPLE_STRING b, int fd)
{
    ((BUF *)b)->fd = fd;
    *((BUF *)b)->ptr++ = '\n';
    _flush((BUF *)b);
    ((BUF *)b)->ptr--;
}

/*
 * Free the opaque structure, and the associated string.
 */
void
_simple_sfree(_SIMPLE_STRING b)
{
    vm_size_t s;

	if(b == NULL) return;
    if(((intptr_t)(((SBUF *)b)->b.buf) & (VM_PAGE_SIZE - 1)) == 0) {
	vm_deallocate(mach_task_self(), (vm_address_t)((SBUF *)b)->b.buf, SBUF_SIZE(b));
	s = VM_PAGE_SIZE;
    } else
	s = ((SBUF *)b)->b.end - (char *)b + 1;
    vm_deallocate(mach_task_self(), (vm_address_t)b, s);
}

/*
 * Simplified ASL log interface; does not use malloc.  Unfortunately, this
 * requires knowledge of the format used by ASL.
 */

static void
_simple_asl_init(void)
{
	kern_return_t status;
	char *str;

	if (asl_port == MACH_PORT_NULL) 
	{
		str = getenv("ASL_DISABLE");
		if ((str != NULL) && (!strcmp(str, "1"))) return;

		status = bootstrap_look_up(bootstrap_port, ASL_SERVICE_NAME, &asl_port);
		if (status != KERN_SUCCESS) asl_port = MACH_PORT_NULL;
	}
}

void
_simple_asl_log_prog(int level, const char *facility, const char *message, const char *prog)
{
    _SIMPLE_STRING b;

    if (pthread_once(&asl_init_once, _simple_asl_init) != 0) return;
	if (asl_port == MACH_PORT_NULL) return;

    if ((b = _simple_salloc()) == NULL) return;

    do
	{
		kern_return_t kstatus;
		vm_address_t out;
		int outlen;
		char *cp;
		struct timeval tv;

		gettimeofday(&tv, NULL);

		if (_simple_sprintf(b, "         0 [Time ", 0)) break;
		if (_simple_esprintf(b, _asl_escape, "%lu", tv.tv_sec)) break;
		if (_simple_sappend(b, "] [Sender ")) break;
		if (_simple_esappend(b, _asl_escape, prog)) break;
		if (_simple_sappend(b, "] [Level ")) break;
		if (_simple_esprintf(b, _asl_escape, "%d", level)) break;
		if (_simple_sappend(b, "] [Facility ")) break;
		if (_simple_esappend(b, _asl_escape, facility)) break;
		if (_simple_sappend(b, "] [Message ")) break;
		if (_simple_esappend(b, _asl_escape, message)) break;

		/* remove trailing (escaped) newlines */
		cp = _simple_string(b);
		cp += strlen(cp);
		for (;;)
		{
			cp -= 2;
			if (strcmp(cp, "\\n") != 0) break;
			*cp = 0;
		}

		_simple_sresize(b);

		if (_simple_sappend(b, "]\n")) break;

		cp = _simple_string(b);

		/*
		 * The MIG defs for _asl_server_message specifies "dealloc",
		 * so we copy the string into a new vm buffer and send that.
		 */
		outlen = strlen(cp);
		kstatus = vm_allocate(mach_task_self(), &out, outlen, TRUE);
		if (kstatus != KERN_SUCCESS) break;

		memcpy((void *)out, cp, outlen);
		_asl_server_message(asl_port, (caddr_t)out, outlen);
    } while (0);

   _simple_sfree(b);
}

void
_simple_asl_log(int level, const char *facility, const char *message)
{
    _simple_asl_log_prog(level, facility, message, getprogname());
}
