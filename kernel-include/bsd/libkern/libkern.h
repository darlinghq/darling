/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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
 *	@(#)libkern.h	8.1 (Berkeley) 6/10/93
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _LIBKERN_LIBKERN_H_
#define _LIBKERN_LIBKERN_H_

#include <sys/appleapiopts.h>
#include <stdint.h>
#include <stdarg.h>     /* for platform-specific va_list */
#include <string.h>
#include <machine/limits.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <mach/vm_param.h>
#include <libkern/crc.h>
#include <libkern/copyio.h>

#if defined(__arm__) || defined(__arm64__)
#include <arm/arch.h> /* for _ARM_ARCH_* */
#endif

#ifdef __APPLE_API_OBSOLETE
/* BCD conversions. */
extern u_char const     bcd2bin_data[];
extern u_char const     bin2bcd_data[];

#define bcd2bin(bcd)    (bcd2bin_data[bcd])
#define bin2bcd(bin)    (bin2bcd_data[bin])
#endif /* __APPLE_API_OBSOLETE */

#ifdef __APPLE_API_PRIVATE
extern char const       hex2ascii_data[];

#define hex2ascii(hex)  (hex2ascii_data[hex])
#endif /* __APPLE_API_PRIVATE */

__BEGIN_DECLS
static inline int
imax(int a, int b)
{
	return a > b ? a : b;
}
static inline int
imin(int a, int b)
{
	return a < b ? a : b;
}
static inline long
lmax(long a, long b)
{
	return a > b ? a : b;
}
static inline long
lmin(long a, long b)
{
	return a < b ? a : b;
}
static inline u_int
max(u_int a, u_int b)
{
	return a > b ? a : b;
}
static inline u_int
min(u_int a, u_int b)
{
	return a < b ? a : b;
}
static inline u_int32_t
ulmax(u_int32_t a, u_int32_t b)
{
	return a > b ? a : b;
}
static inline u_int32_t
ulmin(u_int32_t a, u_int32_t b)
{
	return a < b ? a : b;
}



/* Prototypes for non-quad routines. */
extern int      ffs(unsigned int);
extern int      ffsll(unsigned long long);
extern int      fls(unsigned int);
extern int      flsll(unsigned long long);
extern u_int32_t        random(void);
extern int      scanc(u_int, u_char *, const u_char *, int);
extern int      skpc(int, int, char *);
extern long     strtol(const char*, char **, int);
extern u_long   strtoul(const char *, char **, int);
extern quad_t   strtoq(const char *, char **, int);
extern u_quad_t strtouq(const char *, char **, int);
extern char     *strsep(char **, const char *);
extern void     *memchr(const void *, int, size_t);
extern void     url_decode(char *str);

/*
 * NOTE: snprintf() returns the full length of the formatted string even if it
 * couldn't fit in the supplied buffer.
 * Use scnprintf() if you need the actual number of bytes (minus the \0)
 */
int     snprintf(char *, size_t, const char *, ...) __printflike(3, 4);
int     scnprintf(char *, size_t, const char *, ...) __printflike(3, 4);

/* sprintf() is being deprecated. Please use snprintf() instead. */
int     sprintf(char *bufp, const char *, ...) __deprecated __printflike(2, 3);
int     sscanf(const char *, char const *, ...) __scanflike(2, 3);
int     printf(const char *, ...) __printflike(1, 2);

#if KERNEL_PRIVATE
int     _consume_printf_args(int, ...);
#endif

#if CONFIG_NO_PRINTF_STRINGS
#if KERNEL_PRIVATE
#define printf(x, ...)  _consume_printf_args( 0, ## __VA_ARGS__ )
#else
#define printf(x, ...)  do {} while (0)
#endif
#endif

uint16_t        crc16(uint16_t crc, const void *bufp, size_t len);
uint32_t        crc32(uint32_t crc, const void *bufp, size_t len);

#if XNU_KERNEL_PRIVATE
#if KASAN
uint16_t __nosan_crc16(uint16_t crc, const void *bufp, size_t len);
#else
static inline uint16_t
__nosan_crc16(uint16_t crc, const void *bufp, size_t len)
{
	return crc16(crc, bufp, len);
}
#endif
#endif

int     copystr(const void *kfaddr, void *kdaddr, size_t len, size_t *done);
int     copyinstr(const user_addr_t uaddr, void *kaddr, size_t len, size_t *done) OS_WARN_RESULT;
int     copyoutstr(const void *kaddr, user_addr_t udaddr, size_t len, size_t *done);
#if XNU_KERNEL_PRIVATE
int     copyin_atomic32(const user_addr_t user_addr, uint32_t *u32);
int     copyin_atomic32_wait_if_equals(const user_addr_t user_addr, uint32_t u32);
int     copyin_atomic64(const user_addr_t user_addr, uint64_t *u64);
int     copyout_atomic32(uint32_t u32, user_addr_t user_addr);
int     copyout_atomic64(uint64_t u64, user_addr_t user_addr);
int     copyoutstr_prevalidate(const void *kaddr, user_addr_t uaddr, size_t len);
#endif

int vsscanf(const char *, char const *, va_list);

extern int      vprintf(const char *, va_list) __printflike(1, 0);
extern int      vsnprintf(char *, size_t, const char *, va_list) __printflike(3, 0);
extern int      vscnprintf(char *, size_t, const char *, va_list) __printflike(3, 0);

#if XNU_KERNEL_PRIVATE
extern int      vprintf_log_locked(const char *, va_list, bool addcr) __printflike(1, 0);
extern void     osobject_retain(void * object);
extern void     osobject_release(void * object);
#endif

/* vsprintf() is being deprecated. Please use vsnprintf() instead. */
extern int      vsprintf(char *bufp, const char *, va_list) __deprecated __printflike(2, 0);

#ifdef KERNEL_PRIVATE
#ifdef __arm__
void flush_inner_dcaches(void);
void clean_inner_dcaches(void);
#endif
extern void invalidate_icache(vm_offset_t, unsigned, int);
extern void flush_dcache(vm_offset_t, unsigned, int);
#else
extern void invalidate_icache(vm_offset_t, unsigned, int);
extern void flush_dcache(vm_offset_t, unsigned, int);
#endif
extern void invalidate_icache64(addr64_t, unsigned, int);
extern void flush_dcache64(addr64_t, unsigned, int);


static inline int
clz(unsigned int num)
{
#if (__arm__ || __arm64__)
	// On ARM, clz(0) is defined to return number of bits in the input type
	return __builtin_clz(num);
#else
	// On Intel, clz(0) is undefined
	return num ? __builtin_clz(num) : sizeof(num) * CHAR_BIT;
#endif
}

#if XNU_KERNEL_PRIVATE

/*
 * Define a function that for whatever reason needs to exist, but must never be
 * called.
 */
#define UNSUPPORTED_API(funcname, ...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \
	funcname(__VA_ARGS__) { panic(__func__ ": unsupported API\n"); } \
	_Pragma("clang diagnostic pop")

#endif

__END_DECLS

#endif /* _LIBKERN_LIBKERN_H_ */
