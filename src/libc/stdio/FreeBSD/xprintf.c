/*-
 * Copyright (c) 2005 Poul-Henning Kamp
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libc/stdio/xprintf.c,v 1.9 2010/03/11 17:03:32 jhb Exp $
 */

#include "namespace.h"
#include <err.h>
#include <sys/types.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <locale.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <namespace.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include "un-namespace.h"

//#define MACHTIME
#ifdef MACHTIME
#include <mach/mach_time.h>
#endif // MACHTIME

#ifdef XPRINTF_PERF
#include <libkern/OSAtomic.h>
#endif /* XPRINTF_PERF */

#include "local.h"
#include "xprintf_private.h"
#include "xprintf_domain.h"
#include "fvwrite.h"

/*
 * Defining XPRINTF_DEBUG allows the __private_extern__ variable __use_xprintf
 * to be set so that regular printf variants will use the extensible printf
 * code path.  This is normally off, and is only used to test the extensible
 * printf code in the conformance tests.
 */
#ifdef XPRINTF_DEBUG
#include <unistd.h>
int __use_xprintf = 0;
#endif

/* private stuff -----------------------------------------------------*/

union arg {
	int			intarg;
	long			longarg;
	intmax_t 		intmaxarg;
#ifndef NO_FLOATING_POINT
	double			doublearg;
	long double 		longdoublearg;
#endif
	wint_t			wintarg;
	char			*pchararg;
	wchar_t			*pwchararg;
	void			*pvoidarg;
#ifdef VECTORS
	VECTORTYPE		vectorarg;
	unsigned char		vuchararg[16];
	signed char		vchararg[16];
	unsigned short		vushortarg[8];
	signed short		vshortarg[8];
	unsigned int		vuintarg[4];
	signed int		vintarg[4];
	float			vfloatarg[4];
#ifdef V64TYPE
	double			vdoublearg[2];
	unsigned long long	vulonglongarg[2];
	long long		vlonglongarg[2];
#endif /* V64TYPE */
#endif /* VECTORS */
};

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	(((unsigned)to_digit(c)) <= 9)

/* various globals ---------------------------------------------------*/

__private_extern__ const char __lowercase_hex[17] = "0123456789abcdef?";	/*lint !e784 */
__private_extern__ const char __uppercase_hex[17] = "0123456789ABCDEF?";	/*lint !e784 */

#define PADSIZE 16
static char blanks[PADSIZE] =
	 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
static char zeroes[PADSIZE] =
	 {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

/* printing and padding functions ------------------------------------*/

#define NIOV 8

struct __printf_io {
	FILE		*fp;
	struct __suio	uio;
	struct __siov	iov[NIOV];
	struct __siov	*iovp;
};

static void
__printf_init(struct __printf_io *io)
{

	io->uio.uio_iov = io->iovp = &io->iov[0];
	io->uio.uio_resid = 0;
	io->uio.uio_iovcnt = 0;
}

__private_extern__ void
__printf_flush(struct __printf_io *io)
{

	__sfvwrite(io->fp, &io->uio);
	__printf_init(io);
}

__private_extern__ int
__printf_puts(struct __printf_io *io, const void *ptr, int len)
{


#if 0
	if (io->fp->_flags & __SERR)
		return (0);
#endif
	if (len == 0)
		return (0);
	io->iovp->iov_base = __DECONST(void *, ptr);
	io->iovp->iov_len = len;
	io->uio.uio_resid += len;
	io->iovp++;
	io->uio.uio_iovcnt++;
	if (io->uio.uio_iovcnt >= NIOV)
		__printf_flush(io);
	return (len);
}

__private_extern__ int
__printf_pad(struct __printf_io *io, int howmany, int zero)
{
	int n;
	const char *with;
	int ret = 0;

	if (zero)
		with = zeroes;
	else
		with = blanks;

	if ((n = (howmany)) > 0) {
		while (n > PADSIZE) { 
			ret += __printf_puts(io, with, PADSIZE);
			n -= PADSIZE;
		}
		ret += __printf_puts(io, with, n);
	}
	return (ret);
}

__private_extern__ int
__printf_out(struct __printf_io *io, const struct printf_info *pi, const void *ptr, int len)
{
	int ret = 0;

	if ((!pi->left) && pi->width > len)
		ret += __printf_pad(io, pi->width - len, pi->pad == '0');
	ret += __printf_puts(io, ptr, len);
	if (pi->left && pi->width > len)
		ret += __printf_pad(io, pi->width - len, pi->pad == '0');
	return (ret);
}


/* percent handling  -------------------------------------------------*/

__private_extern__ int
__printf_arginfo_pct(const struct printf_info *pi __unused, size_t n __unused, int *argt __unused)
{

	return (0);
}

__private_extern__ int
__printf_render_pct(struct __printf_io *io, const struct printf_info *pi __unused, const void *const *arg __unused)
{

	return (__printf_puts(io, "%", 1));
}

/* 'n' ---------------------------------------------------------------*/

__private_extern__ int
__printf_arginfo_n(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

/*
 * This is a printf_render so that all output has been flushed before it
 * gets called.
 */

__private_extern__ int
__printf_render_n(FILE *io __unused, const struct printf_info *pi, const void *const *arg)
{

	if (pi->is_char)
		**((signed char **)arg[0]) = (signed char)pi->sofar;
	else if (pi->is_short)
		**((short **)arg[0]) = (short)pi->sofar;
	else if (pi->is_long)
		**((long **)arg[0]) = pi->sofar;
	else if (pi->is_long_double)
		**((long long **)arg[0]) = pi->sofar;
	else if (pi->is_intmax)
		**((intmax_t **)arg[0]) = pi->sofar;
	else if (pi->is_ptrdiff)
		**((ptrdiff_t **)arg[0]) = pi->sofar;
	else if (pi->is_quad)
		**((quad_t **)arg[0]) = pi->sofar;
	else if (pi->is_size)
		**((size_t **)arg[0]) = pi->sofar;
	else
		**((int **)arg[0]) = pi->sofar;

	return (0);
}

/* dynamic array handling  -------------------------------------------------*/
#define ARRAYDELTA 8

struct array {
#ifdef XPRINTF_PERF
	struct array *next;
#endif /* XPRINTF_PERF */
	void *data;
	int itemsize;
	int max;
};

#ifdef XPRINTF_PERF
__private_extern__
#else /* !XPRINTF_PERF */
static
#endif /* !XPRINTF_PERF */
void
arrayfree(struct array *a)
{
	if(a) free(a->data);
}

static void *
arrayget(struct array *a, int i)
{
	if (i >= a->max) {
		int oldsize = a->max * a->itemsize;
		int newmax = i + ARRAYDELTA;
		int newsize = newmax * a->itemsize;
		void *newdata = realloc(a->data, newsize);
		if(!newdata) return NULL;
		bzero(newdata + oldsize, newsize - oldsize);
		a->data = newdata;
		a->max = newmax;
	}
	return a->data + i * a->itemsize;
}

static struct array *
arrayinit(struct array *a, int itemsize)
{
	a->data = CALLOC(ARRAYDELTA, itemsize);
	if(!a->data) return NULL;
	a->itemsize = itemsize;
	a->max = ARRAYDELTA;
	return a;
}

/* dynamic array caching  -------------------------------------------------*/
/*
 * Normally, dynamic array structures are created on the stack, and array
 * itself is freshly allocated, and then freed when no longer needed.  When
 * the XPRINTF_PERF macro is defined, the dynamic array structures associated
 * with all-in-one printf variants are not freed, but store in a cache for
 * later use (dynamic array structures used for compile/execute continue to
 * be freed after they are no longer needed).  This means there should be
 * at most one structure in the cached per thread that actually used the
 * all-in-one printf variant.
 *
 * The amount of memory that is cached is fairly small, totally about 1K
 * for three structures used by a format string using ten conversion
 * specifiers.  This is too small for purgeable memory.
 *
 * However, we do flush these caches in case we every are unable to allocate
 * memory, and retry the allocation, just in case.
 */
#ifdef XPRINTF_PERF
static OSQueueHead arg_type_queue = OS_ATOMIC_QUEUE_INIT;
static OSQueueHead printf_info_queue = OS_ATOMIC_QUEUE_INIT;
static OSQueueHead union_arg_queue = OS_ATOMIC_QUEUE_INIT;

#define DEFINE_DEQUEUE(which, type) \
static struct array * \
which ## _dequeue(void) \
{ \
	struct array *a = (struct array *)OSAtomicDequeue(&which ## _queue, offsetof(struct array, next)); \
 \
	if (a) { \
		bzero(a->data, a->max * a->itemsize); \
		return a; \
	} \
	a = (struct array *)MALLOC(sizeof(*a)); \
	if (!a) return NULL; \
	if (!arrayinit(a, sizeof(type))) { \
		free(a); \
		return NULL; \
	} \
	return a; \
}

#define DEFINE_ENQUEUE(which) \
__private_extern__ void \
which ## _enqueue(struct array *a) \
{ \
	if (!a) return; \
	OSAtomicEnqueue(&which ## _queue, a, offsetof(struct array, next)); \
}

#define DEFINE_FLUSH(which) \
static void \
which ## _flush(void) \
{ \
	struct array *a; \
	while((a = (struct array *)OSAtomicDequeue(&which ## _queue, offsetof(struct array, next))) != NULL) { \
		arrayfree(a); \
		free(a); \
	} \
}

DEFINE_DEQUEUE(arg_type, int)
DEFINE_ENQUEUE(arg_type)
DEFINE_FLUSH(arg_type)
DEFINE_DEQUEUE(printf_info, struct printf_info)
DEFINE_ENQUEUE(printf_info)
DEFINE_FLUSH(printf_info)
DEFINE_DEQUEUE(union_arg, union arg)
DEFINE_ENQUEUE(union_arg)
DEFINE_FLUSH(union_arg)

static void
flush_queues(void)
{
    arg_type_flush();
    printf_info_flush();
    union_arg_flush();
}

__private_extern__ void *
xprintf_calloc(size_t count, size_t size)
{
    void *x = calloc(count, size);
    if(!x) {
	flush_queues();
	x = calloc(count, size);
    }
    return x;
}

__private_extern__ void *
xprintf_malloc(size_t size)
{
    void *x = malloc(size);
    if(!x) {
	flush_queues();
	x = malloc(size);
    }
    return x;
}

#if 0
void
show_queues(void)
{
    struct array *a;
    printf("arg_type:");
    while((a = (struct array *)OSAtomicDequeue(&arg_type_queue, offsetof(struct array, next))) != NULL) printf("\n%p", a);
    printf("\nprintf_info:");
    while((a = (struct array *)OSAtomicDequeue(&printf_info_queue, offsetof(struct array, next))) != NULL) printf("\n%p", a);
    printf("\nunion_arg:");
    while((a = (struct array *)OSAtomicDequeue(&union_arg_queue, offsetof(struct array, next))) != NULL) printf("\n%p", a);
    printf("\n");
}
#endif
#endif /* XPRINTF_PERF */

/* -------------------------------------------------------------------------*/

__private_extern__ int
__printf_comp(printf_comp_t restrict pc, printf_domain_t restrict domain)
{
	struct printf_info	*pi, *pil;
	const char		*fmt;
	int			ch, pii;
	int			*argt;
	int			nextarg;
	int			maxarg;
	int			ret = 0;
	int			n;
#ifndef XPRINTF_PERF
	struct array		piarr, argtarr;
#endif /* XPRINTF_PERF */
	struct array		*pa, *aa;

	fmt = pc->fmt;
	maxarg = 0;
	nextarg = 1;
#ifdef XPRINTF_PERF
	pa = printf_info_dequeue();
#else /* !XPRINTF_PERF */
	pa = arrayinit(&piarr, sizeof(*pi));
#endif /* !XPRINTF_PERF */
	if (!pa) {
#ifdef XPRINTF_PERF
	    flush_queues();
#endif /* XPRINTF_PERF */
	    return EOF;
	}
#ifdef XPRINTF_PERF
	pc->pa = pa;
	aa = arg_type_dequeue();
#else /* !XPRINTF_PERF */
	aa = arrayinit(&argtarr, sizeof(*argt));
#endif /* !XPRINTF_PERF */
	if (!aa) {
		arrayfree(pa);
#ifdef XPRINTF_PERF
		free(pa);
		flush_queues();
#endif /* XPRINTF_PERF */
		return EOF;
	}
#ifdef XPRINTF_PERF
	pc->aa = aa;
#endif /* XPRINTF_PERF */
	for (pii = 0; ; pii++) {
		pi = arrayget(pa, pii);
		if (!pi) {
			ret = EOF;
			goto error;
		}
		pil = pi;
		if (*fmt == '\0')
			break;
		pil = pi + 1;
		pi->prec = -1;
		pi->pad = ' ';
#ifdef VECTORS
		pi->vsep = 'X'; /* Illegal value, changed to defaults later. */
#endif /* VECTORS */
		pi->begin = pi->end = fmt;
		while (*fmt != '\0' && *fmt != '%')
			pi->end = ++fmt;
		if (*fmt == '\0') 
			break;
		fmt++;
		for (;;) {
			pi->spec = *fmt;
			switch (pi->spec) {
			case ' ':
				/*-
				 * ``If the space and + flags both appear, the space
				 * flag will be ignored.''
				 *      -- ANSI X3J11
				 */
				if (pi->showsign == 0) {
					pi->space = 1;
					pi->signchar = ' ';
				}
				fmt++;
				continue;
			case '#':
				pi->alt = 1;
				fmt++;
				continue;
#ifdef VECTORS
			case ',': case ';': case ':': case '_':
				pi->vsep = pi->spec;
				fmt++;
				continue;
#endif /* VECTORS */
			case '.':
				pi->prec = 0;
				fmt++;
				if (*fmt == '*') {
					fmt++;
					/* Look for *nn$ and deal with it */
					n = 0;
					while (*fmt != '\0' && is_digit(*fmt)) {
						n *= 10;
						n += to_digit(*fmt);
						fmt++;
					}
					if (*fmt == '$') {
						if ((n + 1) > maxarg)
							maxarg = (n + 1);
						fmt++;
					} else n = nextarg++;
					pi->get_prec = n;
					argt = (int *)arrayget(aa, n);
					if (!argt) {
						ret = EOF;
						goto error;
					}
					*argt = PA_INT;
					continue;
				}
				while (*fmt != '\0' && is_digit(*fmt)) {
					pi->prec *= 10;
					pi->prec += to_digit(*fmt);
					fmt++;
				}
				continue;
			case '-':
				pi->left = 1;
				fmt++;
				continue;
			case '+':
				pi->showsign = 1;
				pi->signchar = '+';
				fmt++;
				continue;
			case '*':
				fmt++;
				/* Look for *nn$ and deal with it */
				n = 0;
				while (*fmt != '\0' && is_digit(*fmt)) {
					n *= 10;
					n += to_digit(*fmt);
					fmt++;
				}
				if (*fmt == '$') {
					if ((n + 1) > maxarg)
						maxarg = (n + 1);
					fmt++;
				} else n = nextarg++;
				pi->get_width = n;
				argt = (int *)arrayget(aa, n);
				if (!argt) {
					ret = EOF;
					goto error;
				}
				*argt = PA_INT;
				continue;
			case '%':
				fmt++;
				break;
			case '\'':
				pi->group = 1;
				fmt++;
				continue;
			case '0':
				/*-
				 * ``Note that 0 is taken as a flag, not as the
				 * beginning of a field width.''
				 *      -- ANSI X3J11
				 */
				pi->pad = '0';
				fmt++;
				continue;
			case '1': case '2': case '3':
			case '4': case '5': case '6':
			case '7': case '8': case '9':
				n = 0;
				while (*fmt != '\0' && is_digit(*fmt)) {
					n *= 10;
					n += to_digit(*fmt);
					fmt++;
				}
				if (*fmt == '$') {
					if (nextarg > maxarg)
						maxarg = nextarg;
					nextarg = n;
					fmt++;
				} else 
					pi->width = n;
				continue;
#if 0
			case 'D':
			case 'O':
			case 'U':
				pi->spec += ('a' - 'A');
				pi->is_intmax = 0;
				if (pi->is_long_double || pi->is_quad) {
					pi->is_long = 0;
					pi->is_long_double = 1;
				} else {
					pi->is_long = 1;
					pi->is_long_double = 0;
				}
				fmt++;
				break;
#endif
			case 'j':
				pi->is_intmax = 1;
				fmt++;
				continue;
			case 'q':
				pi->is_long = 0;
				pi->is_quad = 1;
				fmt++;
				continue;
			case 'L':
				pi->is_long_double = 1;
				fmt++;
				continue;
			case 'h':
				fmt++;
				if (*fmt == 'h') {
					fmt++;
					pi->is_char = 1;
				} else {
					pi->is_short = 1;
				}
				continue;
			case 'l':
				fmt++;
				if (*fmt == 'l') {
					fmt++;
					pi->is_long_double = 1;
					pi->is_quad = 0;
				} else {
					pi->is_quad = 0;
					pi->is_long = 1;
				}
				continue;
			case 't':
				pi->is_ptrdiff = 1;
				fmt++;
				continue;
			case 'v':
#ifdef VECTORS
				pi->is_vec = 1;
#endif /* VECTORS */
				fmt++;
				continue;
			case 'z':
				pi->is_size = 1;
				fmt++;
				continue;
			default:
				fmt++;
				break;
			}
			if (printf_tbl_in_range(pi->spec)) {
				switch(domain->type[printf_tbl_index(pi->spec)]) {
				/* ignore PRINTF_DOMAIN_UNUSED until later */
				case PRINTF_DOMAIN_FLAG:
					errx(1, "Unexpected flag: %c", pi->spec);
				case PRINTF_DOMAIN_GLIBC_API:
				case PRINTF_DOMAIN_FBSD_API:
					/*
					 * Insure that there are always
					 * __PRINTFMAXARG available.
					 */
					if (!arrayget(aa, nextarg + __PRINTFMAXARG - 1)) {
						ret = EOF;
						goto error;
					}
					pi->context = domain->tbl[printf_tbl_index(pi->spec)].context;
					pi->loc = pc->loc;
					ch = domain->tbl[printf_tbl_index(pi->spec)].arginfo(
					    pi, __PRINTFMAXARG, arrayget(aa, nextarg));
					if (ch > 0)
						pi->arg[0] = (void *)(long)nextarg;
					if (ch > 1)
						pi->arg[1] = (void *)(long)(nextarg + 1);
					nextarg += ch;
					break;
				}
			}
			break;
		}
	}
	if (nextarg > maxarg)
		maxarg = nextarg;
	pc->argt = aa->data;
	pc->pi = pa->data;
	pc->pil = pil;
	pc->maxarg = ch = maxarg;
	if (ch < 1) ch = 1;
#ifdef XPRINTF_PERF
	pc->ua = union_arg_dequeue();
	if (!pc->ua) {
	    ret = EOF;
	    goto error;
	}
	if (!arrayget(pc->ua, ch)) {
	    union_arg_enqueue(pc->ua);
	    ret = EOF;
	    goto error;
	}
	pc->args = pc->ua->data;
#else /* !XPRINTF_PERF */
	pc->args = (union arg *)malloc(ch * sizeof(*pc->args));
	if (!pc->args) {
		ret = EOF;
		goto error;
	}
#endif /* !XPRINTF_PERF */
	for (pi = pc->pi; pi < pil; pi++) {
		if (pi->arg[0]) pi->arg[0] = &pc->args[(long)pi->arg[0]];
		if (pi->arg[1]) pi->arg[1] = &pc->args[(long)pi->arg[1]];
	}
#if 0
	fprintf(stderr, "fmt0 <%s>\n", fmt0);
	fprintf(stderr, "pil %p\n", pil);
#endif
	pc->domain = domain;

	return (ret);
error:
	arrayfree(pa);
	arrayfree(aa);
#ifdef XPRINTF_PERF
	free(pa);
	free(aa);
	flush_queues();
#endif /* XPRINTF_PERF */
	return (ret);
}

__private_extern__ int
__printf_exec(printf_comp_t restrict pc, FILE * restrict fp, va_list ap)
{
	struct printf_info	*pi;
	int			ch;
	int			ret = 0;
	int			n;
	struct __printf_io	io;

	__printf_init(&io);
	io.fp = fp;

	for (ch = 1; ch < pc->maxarg; ch++) {
#if 0
		fprintf(stderr, "arg %d %x\n", ch, pc->argt[ch]);
#endif
		switch(pc->argt[ch]) {
		case PA_CHAR:
			pc->args[ch].intarg = (char)va_arg (ap, int);
			break;
		case PA_INT:
			pc->args[ch].intarg = va_arg (ap, int);
			break;
		case PA_INT | PA_FLAG_SHORT:
			pc->args[ch].intarg = (short)va_arg (ap, int);
			break;
		case PA_INT | PA_FLAG_LONG:
			pc->args[ch].longarg = va_arg (ap, long);
			break;
		case PA_INT | PA_FLAG_INTMAX:
			pc->args[ch].intmaxarg = va_arg (ap, intmax_t);
			break;
		case PA_INT | PA_FLAG_QUAD:
			pc->args[ch].intmaxarg = va_arg (ap, quad_t);
			break;
		case PA_INT | PA_FLAG_LONG_LONG:
			pc->args[ch].intmaxarg = va_arg (ap, long long);
			break;
		case PA_INT | PA_FLAG_SIZE:
			pc->args[ch].intmaxarg = va_arg (ap, size_t);
			break;
		case PA_INT | PA_FLAG_PTRDIFF:
			pc->args[ch].intmaxarg = (unsigned long)va_arg (ap, ptrdiff_t);
			break;
		case PA_WCHAR:
			pc->args[ch].wintarg = va_arg (ap, wint_t);
			break;
		case PA_POINTER:
			pc->args[ch].pvoidarg = va_arg (ap, void *);
			break;
		case PA_STRING:
			pc->args[ch].pchararg = va_arg (ap, char *);
			break;
		case PA_WSTRING:
			pc->args[ch].pwchararg = va_arg (ap, wchar_t *);
			break;
		case PA_DOUBLE:
#ifndef NO_FLOATING_POINT
			pc->args[ch].doublearg = va_arg (ap, double);
#endif
			break;
		case PA_DOUBLE | PA_FLAG_LONG_DOUBLE:
#ifndef NO_FLOATING_POINT
			pc->args[ch].longdoublearg = va_arg (ap, long double);
#endif
			break;
#ifdef VECTORS
		case PA_VECTOR:
			pc->args[ch].vectorarg = va_arg (ap, VECTORTYPE);
			break;
#endif /* VECTORS */
		default:
			errx(1, "argtype = %x (fmt = \"%s\")\n",
			    pc->argt[ch], pc->fmt);
		}
	}
	for (pi = pc->pi; pi < pc->pil; pi++) {
#if 0
		fprintf(stderr, "pi %p", pi);
		fprintf(stderr, " spec '%c'", pi->spec);
		fprintf(stderr, " args %d",
		    ((uintptr_t)pi->arg[0] - (uintptr_t)pc->args) / sizeof pc->args[0]);
		if (pi->width) fprintf(stderr, " width %d", pi->width);
		if (pi->pad) fprintf(stderr, " pad 0x%x", pi->pad);
		if (pi->left) fprintf(stderr, " left");
		if (pi->showsign) fprintf(stderr, " showsign");
		if (pi->signchar) fprintf(stderr, " signchar 0x%x", pi->signchar);
		if (pi->prec != -1) fprintf(stderr, " prec %d", pi->prec);
		if (pi->is_char) fprintf(stderr, " char");
		if (pi->is_short) fprintf(stderr, " short");
		if (pi->is_long) fprintf(stderr, " long");
		if (pi->is_long_double) fprintf(stderr, " long_double");
		fprintf(stderr, "\n");
		fprintf(stderr, "\t\"%.*s\"\n", pi->end - pi->begin, pi->begin);
#endif
		if (pi->get_width) {
			pi->width = pc->args[pi->get_width].intarg;
			/*-
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *      -- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			if (pi->width < 0) {
				pi->left = 1;
				pi->width = -pi->width;
			}
		}
		if (pi->get_prec) 
			pi->prec = pc->args[pi->get_prec].intarg;
		ret += __printf_puts(&io, pi->begin, pi->end - pi->begin);
		if (pi->spec) {
			if (!printf_tbl_in_range(pi->spec)) goto unused;
			switch(pc->domain->type[printf_tbl_index(pi->spec)]) {
			case PRINTF_DOMAIN_UNUSED:
		unused:
			{
				char unknown = pi->spec;
				ret += __printf_out(&io, pi, &unknown, 1);
				break;
			}
			case PRINTF_DOMAIN_GLIBC_API:
				__printf_flush(&io);
				pi->sofar = ret;
				ret += ((printf_function *)pc->domain->tbl[printf_tbl_index(pi->spec)].render)(
				    fp, pi, (const void *)pi->arg);
				break;
			case PRINTF_DOMAIN_FBSD_API:
				pi->sofar = ret;
				n = ((printf_render *)pc->domain->tbl[printf_tbl_index(pi->spec)].render)(
				    &io, pi, (const void *)pi->arg);
				if (n < 0)
					io.fp->_flags |= __SERR;
				else
					ret += n;
				break;
			}
		}
	}
	__printf_flush(&io);
	return (ret);
}

__private_extern__ int
__v2printf(printf_comp_t restrict pc, printf_domain_t restrict domain, FILE * restrict fp, locale_t restrict loc, const char * restrict fmt, va_list ap)
{
	struct _printf_compiled spc;
	int ret, saverrno;

	/*
	 * All the printf family (including extensible printf variants) funnel
	 * down to this point.  So we can do common work here, and then fork
	 * out to the appropriate handler.
	 */
	/* sorry, fprintf(read_only_file, "") returns EOF, not 0 */
	if (prepwrite(fp) != 0) {
		errno = EBADF;
		return (EOF);
	}
	ORIENT(fp, -1);

	if (pc == XPRINTF_PLAIN) {
		NORMALIZE_LOCALE(loc);
#ifdef XPRINTF_DEBUG
		if (!__use_xprintf)
#endif
				    return __vfprintf(fp, loc, fmt, ap);
#ifdef XPRINTF_DEBUG
		xprintf_domain_init();
		domain = xprintf_domain_global;
#endif
	} else if (pc) {
	    pthread_mutex_lock(&pc->mutex);
	    pthread_rwlock_rdlock(&pc->domain->rwlock);
	    ret = __printf_exec(pc, fp, ap);
	    saverrno = errno;
	    pthread_rwlock_unlock(&pc->domain->rwlock);
	    pthread_mutex_unlock(&pc->mutex);
	    errno = saverrno;
	    return ret;
	}
	if (!domain) {
		errno = EINVAL;
		return EOF;
	}
	xprintf_domain_init();
	bzero(&spc, sizeof(spc));
	spc.fmt = fmt;
	DEFAULT_CURRENT_LOCALE(loc);
	XL_RETAIN(loc);
	spc.loc = loc;
	/*
	 * We don't need to lock the printf_comp_t mutex, since the
	 * printf_comp_t was just created on the stack, and is private.
	 */
	pthread_rwlock_rdlock(&domain->rwlock);
	if (__printf_comp(&spc, domain) < 0) {
	    saverrno = errno;
	    pthread_rwlock_unlock(&domain->rwlock);
	    XL_RELEASE(loc);
	    errno = saverrno;
	    return EOF;
	}
	ret = __printf_exec(&spc, fp, ap);
	saverrno = errno;
	pthread_rwlock_unlock(&domain->rwlock);
	XL_RELEASE(loc);

#ifdef XPRINTF_PERF
	printf_info_enqueue(spc.pa);
	arg_type_enqueue(spc.aa);
	union_arg_enqueue(spc.ua);
#else /* !XPRINTF_PERF */
	free(spc.pi);
	free(spc.argt);
	free(spc.args);
#endif /* !XPRINTF_PERF */
	errno = saverrno;
	return ret;
}

extern int      __fflush(FILE *fp);

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__v3printf(printf_comp_t restrict pc, printf_domain_t restrict domain, FILE * restrict fp, locale_t restrict loc, const char * restrict fmt, va_list ap)
{
	int ret;
	FILE fake;
	struct __sFILEX extra;
	unsigned char buf[BUFSIZ];

	fake._extra = &extra;
	INITEXTRA(&fake);

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;
	fake._orientation = fp->_orientation;
	fake._mbstate = fp->_mbstate;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof(buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */

	/* do the work, then copy any error status */
	ret = __v2printf(pc, domain, &fake, loc, fmt, ap);
	if (ret >= 0 && __fflush(&fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;
	return (ret);
}

__private_extern__ int
__xvprintf(printf_comp_t restrict pc, printf_domain_t restrict domain, FILE * restrict fp, locale_t restrict loc, const char * restrict fmt0, va_list ap)
{
	int ret;

	/* optimise fprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0)
		ret = __v3printf(pc, domain, fp, loc, fmt0, ap);
	else
		ret = __v2printf(pc, domain, fp, loc, fmt0, ap);
	return ret;
}

/* extending ---------------------------------------------------------*/

// No global domain support
#if 0
int
register_printf_function(int spec, printf_function *render, printf_arginfo_function *arginfo)
{
	return register_printf_domain_function(NULL, spec, render, arginfo);
}

__private_extern__ int
register_printf_render(int spec, printf_render *render, printf_arginfo_function *arginfo)
{
	return register_printf_domain_render(NULL, spec, render, arginfo);
}

int
register_printf_render_std(const char *specs)
{
	return register_printf_domain_render_std(NULL, specs);
}
#endif

#ifdef VECTORS
/* vector support ----------------------------------------------------*/

#define PRINTVECTOR(_io, _pi, _arg, _cnt, _type, _elem, _render, _ret) { \
	int i; \
	_type a, *ap; \
	a = (_type)(_arg)->_elem[0]; \
	ap = &a; \
	(_ret) += _render((_io), (_pi), (const void *)&ap); \
	for(i = 1; i < (_cnt); i++) { \
		(_ret) += __printf_puts((_io), (_pi)->begin, (_pi)->end - (_pi)->begin); \
		a = (_type)(_arg)->_elem[i]; \
		(_ret) += _render((_io), (_pi), (const void *)&ap); \
	} \
}

#define PRINTVECTOR_P(_io, _pi, _arg, _cnt, _elem, _render, _ret) { \
	int i; \
	void * a, *ap; \
	a = (void *)(uintptr_t)(_arg)->_elem[0]; \
	ap = &a; \
	(_ret) += _render((_io), (_pi), (const void *)&ap); \
	for(i = 1; i < (_cnt); i++) { \
		(_ret) += __printf_puts((_io), (_pi)->begin, (_pi)->end - (_pi)->begin); \
		a = (void *)(uintptr_t)(_arg)->_elem[i]; \
		(_ret) += _render((_io), (_pi), (const void *)&ap); \
	} \
}

__private_extern__ int
__xprintf_vector(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	char vsep;	/* Vector separator character. */
	const union arg *argp;
	int ret = 0;
	struct printf_info info = *pi;

	argp = arg[0];
	vsep = pi->vsep;
	if (vsep == 'X') {
		if (pi->spec == 'c')
			vsep = '\0';
		else
			vsep = ' ';
	}
	info.begin = info.end = &vsep;
	if (vsep) info.end++;
	info.is_vec = 0;

	if (pi->is_short) {
		if (pi->spec == 'p') {
			PRINTVECTOR_P(io, &info, argp, 8, vushortarg, __printf_render_ptr, ret);
		} else {
			PRINTVECTOR(io, &info, argp, 8, unsigned int, vushortarg, __printf_render_int, ret);
		}
	} else if (pi->is_long) {
		info.is_long = 0;
		if (pi->spec == 'p') {
			PRINTVECTOR_P(io, &info, argp, 4, vuintarg, __printf_render_ptr, ret);
		} else {
			PRINTVECTOR(io, &info, argp, 4, unsigned int, vuintarg, __printf_render_int, ret);
		}
#ifdef V64TYPE
	} else if (pi->is_long_double) {
		switch (pi->spec) {
		case 'a':
		case 'A':
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			info.is_long_double = 0;
			PRINTVECTOR(io, &info, argp, 2, double, vdoublearg, __printf_render_float, ret);
			break;
		case 'p':
			info.is_long_double = 0;
			PRINTVECTOR_P(io, &info, argp, 2, vulonglongarg, __printf_render_ptr, ret);
			break;
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
			PRINTVECTOR(io, &info, argp, 2, unsigned long long, vulonglongarg, __printf_render_int, ret);
			break;
		default:
			/*
			 * The default case should never
			 * happen.
			 */
		case 'c':
			info.is_long_double = 0;
			PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, __printf_render_chr, ret);
		}
#endif /* V64TYPE */
	} else {
		switch (pi->spec) {
		case 'a':
		case 'A':
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			PRINTVECTOR(io, &info, argp, 4, double, vfloatarg, __printf_render_float, ret);
			break;
		default:
			/*
			 * The default case should never
			 * happen.
			 */
		case 'p':
			PRINTVECTOR_P(io, &info, argp, 16, vuchararg, __printf_render_ptr, ret);
			break;
		case 'd':
		case 'i':
		case 'u':
		case 'o':
		case 'x':
		case 'X':
			info.is_char = 1;
			PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, __printf_render_int, ret);
			break;
		case 'c':
			PRINTVECTOR(io, &info, argp, 16, unsigned int, vuchararg, __printf_render_chr, ret);
		}
	}
	return ret;
}
#endif /* VECTORS */
