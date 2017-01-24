/*-
 * Copyright (c) 2005 Poul-Henning Kamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/include/printf.h,v 1.5 2011/03/06 17:45:37 pjd Exp $
 */

#ifndef _XPRINTF_PRIVATE_H_
#define _XPRINTF_PRIVATE_H_

#include <printf.h>
#include <pthread.h>

#ifndef VECTORS
#define VECTORS
typedef __attribute__ ((vector_size(16))) unsigned char VECTORTYPE;
#ifdef __SSE2__
#define V64TYPE
#endif /* __SSE2__ */
#endif /* !VECTORS */

/* FreeBSD extension */
struct __printf_io;
typedef int printf_render(struct __printf_io *, const struct printf_info *, const void *const *);

#if 0
int register_printf_render(int spec, printf_render *render, printf_arginfo_function *arginfo);
#endif

/*
 * Unlike register_printf_domain_function(), register_printf_domain_render()
 * doesn't have a context pointer, because none of the internal rendering
 * functions use it.
 */
int register_printf_domain_render(printf_domain_t, int, printf_render *, printf_arginfo_function *);

/* xprintf.c */
extern const char __lowercase_hex[17];
extern const char __uppercase_hex[17];

void __printf_flush(struct __printf_io *io);
int __printf_puts(struct __printf_io *io, const void *ptr, int len);
int __printf_pad(struct __printf_io *io, int n, int zero);
int __printf_out(struct __printf_io *io, const struct printf_info *pi, const void *ptr, int len);

int __v2printf(printf_comp_t restrict pc, printf_domain_t restrict domain, FILE * restrict fp, locale_t restrict loc, const char * restrict fmt0, va_list ap);
int __xvprintf(printf_comp_t restrict pc, printf_domain_t restrict domain, FILE * restrict fp, locale_t restrict loc, const char * restrict fmt0, va_list ap);
extern int __use_xprintf;

printf_arginfo_function		__printf_arginfo_pct;
printf_render			__printf_render_pct;

printf_arginfo_function		__printf_arginfo_n;
printf_function			__printf_render_n;

#ifdef VECTORS
printf_render			__xprintf_vector;
#endif /* VECTORS */

#ifdef XPRINTF_PERF
#define CALLOC(x,y)	xprintf_calloc((x),(y))
#define MALLOC(x)	xprintf_malloc((x))
void *xprintf_calloc(size_t, size_t) __attribute__((__malloc__));
void *xprintf_malloc(size_t) __attribute__((__malloc__));
#else /* !XPRINTF_PERF */
#define CALLOC(x,y)	calloc((x),(y))
#define MALLOC(x)	malloc((x))
#endif /* !XPRINTF_PERF */

/* xprintf_domain.c */
void __xprintf_domain_init(void);
extern pthread_once_t __xprintf_domain_once;
#ifdef XPRINTF_DEBUG
extern printf_domain_t xprintf_domain_global;
#endif
#ifdef XPRINTF_PERF
struct array; /* forward reference */
void arrayfree(struct array *);
#endif

#define xprintf_domain_init()	pthread_once(&__xprintf_domain_once, __xprintf_domain_init)

/* xprintf_errno.c */
printf_arginfo_function		__printf_arginfo_errno;
printf_render			__printf_render_errno;

/* xprintf_float.c */
printf_arginfo_function		__printf_arginfo_float;
printf_render			__printf_render_float;

/* xprintf_hexdump.c */
printf_arginfo_function		__printf_arginfo_hexdump;
printf_render 			__printf_render_hexdump;

/* xprintf_int.c */
printf_arginfo_function		__printf_arginfo_ptr;
printf_arginfo_function		__printf_arginfo_int;
printf_render			__printf_render_ptr;
printf_render			__printf_render_int;

/* xprintf_quoute.c */
printf_arginfo_function		__printf_arginfo_quote;
printf_render 			__printf_render_quote;

/* xprintf_str.c */
printf_arginfo_function		__printf_arginfo_chr;
printf_render			__printf_render_chr;
printf_arginfo_function		__printf_arginfo_str;
printf_render			__printf_render_str;

/* xprintf_time.c */
printf_arginfo_function		__printf_arginfo_time;
printf_render			__printf_render_time;

/* xprintf_vis.c */
printf_arginfo_function		__printf_arginfo_vis;
printf_render 			__printf_render_vis;

#ifdef XPRINTF_PERF
struct array; /* forward reference */
#endif /* XPRINTF_PERF */
struct _printf_compiled {
    pthread_mutex_t mutex;
#ifdef XPRINTF_PERF
    struct array *aa;
    struct array *pa;
    struct array *ua;
#endif /* XPRINTF_PERF */
    const char *fmt;
    printf_domain_t domain;
    locale_t loc;
    struct printf_info *pi;
    struct printf_info *pil;
    int *argt;
    union arg *args;
    int maxarg;
};

#define	XPRINTF_PLAIN	((printf_comp_t)-1)

int __printf_comp(printf_comp_t restrict, printf_domain_t restrict);
int __printf_exec(printf_comp_t restrict, FILE * restrict, va_list);

#ifdef XPRINTF_PERF
void arg_type_enqueue(struct array *);
void print_info_enqueue(struct array *);
void union_arg_enqueue(struct array *);
#endif /* XPRINTF_PERF */

#endif /* !_XPRINTF_PRIVATE_H_ */
