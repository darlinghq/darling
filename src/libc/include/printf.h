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

#ifndef _PRINTF_H_
#define _PRINTF_H_

/****************************************************************************
 * This is the header file for extensible printf, a set of APIs that allow
 * adding/modifying conversion specifier(s) for stdio formatted printing.
 * It is based on the GLIBC API documented in:
 *
 *   http://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html
 *
 * Because that API affects printf behavior process-wide and so is unsafe,
 * we adapt a modified form, based on the concept of printf domains in which
 * changes to conversion specifiers can be made independent of one another
 * and which don't affect the normal printf behavior.  In addition, there
 * is now a set of printf variants that take a printf domain as an argument.
 *
 * See xprintf(5) for more details.
 ****************************************************************************/

#include <stdio.h>
#include <wchar.h>
#include <xlocale.h>
#include <Availability.h>

#ifdef __GNUC__
#define __XPRINTF_ATTR(x)	__attribute__(x)
#else /* !__GNUC__ */
#define __XPRINTF_ATTR(x)	/* nothing */
#endif /* !__GNUC__ */

/*
 * The API defined by GLIBC allows a renderer to take multiple arguments
 * This is obviously usable for things like (ptr+len) pairs etc.
 * The current limit is to deal with up to __PRINTFMAXARG arguments (any
 * above this limit are ignored).
 */
#define __PRINTFMAXARG		2

struct printf_info {
	/* Mac OS X extensions */
	void		*context;		/* User context pointer */
	locale_t	loc;			/* Extended locale */
	wchar_t		vsep;			/* Vector separator char */
						/* one of ,:;_ flag or X by default */

	/* GLIBC compatible */
	int		prec;			/* precision */
	int		width;			/* Width */
	wchar_t		spec;			/* Format letter */
	wchar_t		pad;			/* Padding char */
						/* 0 if 0 flag set, otherwise space */

	/* FreeBSD extensions */
	wchar_t		signchar;		/* Sign char */

	/* GLIBC compatible flags */
	unsigned	is_long_double	:1;	/* L or ll flag */
	unsigned	is_char		:1;	/* hh flag */
	unsigned	is_short	:1;	/* h flag */
	unsigned	is_long		:1;	/* l flag */
	unsigned	alt		:1;	/* # flag */
	unsigned	space		:1;	/* Space flag */
	unsigned	left		:1;	/* - flag */
	unsigned	showsign	:1;	/* + flag */
	unsigned	group		:1;	/* ' flag */
	unsigned	extra		:1;	/* For special use (currently unused) */
	unsigned	wide		:1;	/* Nonzero for wide character streams (currently unused) */

	/* FreeBSD flags */
	unsigned	is_quad		:1;	/* q flag */
	unsigned	is_intmax	:1;	/* j flag */
	unsigned	is_ptrdiff	:1;	/* t flag */
	unsigned	is_size		:1;	/* z flag */

	/* Mac OS X flags */
	unsigned	is_vec		:1;	/* v flag */

	/* private */
	int		sofar;
	unsigned	get_width;
	unsigned	get_prec;
	const char	*begin;
	const char	*end;
	void 		*arg[__PRINTFMAXARG];
};

enum {
	PA_INT		= (1 << 0),	/* int */
	PA_CHAR		= (1 << 1),	/* int, cast to char */
	PA_WCHAR	= (1 << 2),	/* wide char */
	PA_STRING	= (1 << 3),	/* const char * (with '\0') */
	PA_WSTRING	= (1 << 4),	/* const wchar_t * */
	PA_POINTER	= (1 << 5),	/* void * */
	PA_FLOAT	= (1 << 6),	/* float (Defined but unused; best to avoid.) */
	PA_DOUBLE	= (1 << 7), 	/* double */
	PA_VECTOR	= (1 << 8), 	/* vector */
};

#define	PA_FLAG_MASK		0xff0000
#define	PA_FLAG_LONG_LONG	(1 << 16)
#define	PA_FLAG_LONG		(1 << 17)
#define	PA_FLAG_SHORT		(1 << 18)
#define	PA_FLAG_PTR		(1 << 19)
#define	PA_FLAG_QUAD		(1 << 20)
#define	PA_FLAG_INTMAX		(1 << 21)
#define	PA_FLAG_SIZE		(1 << 22)
#define	PA_FLAG_PTRDIFF		(1 << 23)
#define	PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG

/************************ Basic Extensible Printf APIs ************************/

typedef int printf_arginfo_function(const struct printf_info *__info,
	     size_t __n, int *__argtypes);
typedef int printf_function(FILE *__stream,
	     const struct printf_info *__info, const void *const *__args);

/*
 * We don't support the GLIBC register_printf_function() or FreeBSD
 * register_printf_render_std(), because they affect printf globally
 * and are unsafe.
 */

/*************** Extensible Printf Domains APIs ****************/

struct _printf_domain; /* forward reference */
typedef struct _printf_domain *printf_domain_t;

__BEGIN_DECLS

printf_domain_t	copy_printf_domain(printf_domain_t __domain)
		 __XPRINTF_ATTR((__nonnull__(1)))
		 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
void		free_printf_domain(printf_domain_t __domain)
		 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
printf_domain_t	new_printf_domain(void)
		 __XPRINTF_ATTR((__malloc__))
		 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	register_printf_domain_function(printf_domain_t __domain,
	 int __spec, printf_function *__render,
	 printf_arginfo_function *__arginfo, void *__context)
	 __XPRINTF_ATTR((__nonnull__(1)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	register_printf_domain_render_std(printf_domain_t __domain,
	 const char *__specs)
	 __XPRINTF_ATTR((__nonnull__(1)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/**** All-in-one extensible printf variants ****/
int	asxprintf(char ** __restrict __ret,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	dxprintf(int __fd, printf_domain_t __restrict __domain,
	 locale_t __restrict __loc, const char * __restrict __format, ...)
	 __XPRINTF_ATTR((__nonnull__(2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	fxprintf(FILE * __restrict __stream,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	sxprintf(char * __restrict __str, size_t __size,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 3, 5)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	xprintf(printf_domain_t __restrict __domain,
	 locale_t __restrict __loc, const char * __restrict __format, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 3)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

int	vasxprintf(char ** __restrict __ret,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vdxprintf(int __fd, printf_domain_t __restrict __domain,
	 locale_t __restrict __loc, const char * __restrict __format,
	 va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vfxprintf(FILE * __restrict __stream,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 2, 4)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vsxprintf(char * __restrict __str, size_t __size,
	 printf_domain_t __restrict __domain, locale_t __restrict __loc,
	 const char * __restrict __format, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 3, 5)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vxprintf(printf_domain_t __restrict __domain,
	 locale_t __restrict __loc, const char * __restrict __format,
	 va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 3)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

__END_DECLS

/******** Extensible Printf Compilation/Execution APIs *********/
struct _printf_compiled; /* forward reference */
typedef struct _printf_compiled *printf_comp_t;

__BEGIN_DECLS

void		free_printf_comp(printf_comp_t __pc)
		 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
printf_comp_t	new_printf_comp(printf_domain_t __restrict __domain,
		 locale_t __restrict __loc, const char * __restrict __fmt)
		 __XPRINTF_ATTR((__nonnull__(1, 3)))
		 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

/**** Extensible printf execution ****/
int	asxprintf_exec(char ** __restrict __ret,
	 printf_comp_t __restrict __pc, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	dxprintf_exec(int __fd, printf_comp_t __restrict __pc, ...)
	 __XPRINTF_ATTR((__nonnull__(2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	fxprintf_exec(FILE * __restrict __stream,
	 printf_comp_t __restrict __pc, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	sxprintf_exec(char * __restrict __str, size_t __size,
	 printf_comp_t __restrict __pc, ...)
	 __XPRINTF_ATTR((__nonnull__(1, 3)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	xprintf_exec(printf_comp_t __restrict __pc, ...)
	 __XPRINTF_ATTR((__nonnull__(1)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

int	vasxprintf_exec(char ** __restrict __ret,
	 printf_comp_t __restrict __pc, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vdxprintf_exec(int __fd, printf_comp_t __restrict __pc,
	 va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vfxprintf_exec(FILE * __restrict __stream,
	 printf_comp_t __restrict __pc, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 2)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vsxprintf_exec(char * __restrict __str, size_t __size,
	 printf_comp_t __restrict __pc, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1, 3)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);
int	vxprintf_exec(printf_comp_t __restrict __pc, va_list __ap)
	 __XPRINTF_ATTR((__nonnull__(1)))
	 __OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0);

__END_DECLS

#endif /* !_PRINTF_H */
