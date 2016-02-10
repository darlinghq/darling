/*
 * Copyright (c) 2000, 2007 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1982, 1986, 1992, 1993
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
 *	@(#)gmon.h	8.2 (Berkeley) 1/4/94
 */

#ifndef _SYS_GMON_H_
#define _SYS_GMON_H_
#include <stdint.h>

/*
 * Structure prepended to gmon.out profiling data file.
 */
struct gmonhdr {
	uint32_t lpc;		/* base pc address of sample buffer */
	uint32_t hpc;		/* max pc address of sampled buffer */
	uint32_t ncnt;		/* size of sample buffer (plus this header) */
	int32_t version;	/* version number */
	int32_t profrate;	/* profiling clock rate */
	int32_t spare[3];	/* reserved */
};
#define GMONVERSION	0x00051879

struct gmonhdr_64 {
	uint64_t lpc;		/* base pc address of sample buffer */
	uint64_t hpc;		/* max pc address of sampled buffer */
	uint32_t ncnt;		/* size of sample buffer (plus this header) */
	int32_t version;	/* version number */
	int32_t profrate;	/* profiling clock rate */
	int32_t spare[3];	/* reserved */
};

typedef struct
#ifndef __LP64__
        gmonhdr
#else
        gmonhdr_64
#endif
gmonhdr_t;

/*
 * histogram counters are unsigned shorts (according to the kernel).
 */
#define	HISTCOUNTER	unsigned short

/*
 * fraction of text space to allocate for histogram counters here, 1/2
 */
#define	HISTFRACTION	2

/*
 * Fraction of text space to allocate for from hash buckets.
 * The value of HASHFRACTION is based on the minimum number of bytes
 * of separation between two subroutine call points in the object code.
 * Given MIN_SUBR_SEPARATION bytes of separation the value of
 * HASHFRACTION is calculated as:
 *
 *	HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
 *
 * For example, on the VAX, the shortest two call sequence is:
 *
 *	calls	$0,(r0)
 *	calls	$0,(r0)
 *
 * which is separated by only three bytes, thus HASHFRACTION is 
 * calculated as:
 *
 *	HASHFRACTION = 3 / (2 * 2 - 1) = 1
 *
 * Note that the division above rounds down, thus if MIN_SUBR_FRACTION
 * is less than three, this algorithm will not work!
 *
 * In practice, however, call instructions are rarely at a minimal 
 * distance.  Hence, we will define HASHFRACTION to be 2 across all
 * architectures.  This saves a reasonable amount of space for 
 * profiling data structures without (in practice) sacrificing
 * any granularity.
 */
#define	HASHFRACTION	2

/*
 * percent of text space to allocate for tostructs with a minimum.
 */
#define ARCDENSITY	2
#define MINARCS		50
#define MAXARCS		((1 << (8 * sizeof(HISTCOUNTER))) - 2)

struct tostruct {
	uint32_t	selfpc;
	int32_t		count;
	uint16_t	link;
	uint16_t	order;
};

struct tostruct_64 {
	uint64_t	selfpc;
	int32_t		count;
	uint16_t	link;
	uint16_t	order;
};

typedef struct
#ifndef __LP64__
        tostruct
#else
        tostruct_64
#endif
tostruct_t;

/*
 * a raw arc, with pointers to the calling site and 
 * the called site and a count.
 */
struct rawarc {
	uint32_t	raw_frompc;
	uint32_t	raw_selfpc;
	int32_t		raw_count;
};

struct rawarc_64 {
	uint64_t	raw_frompc;
	uint64_t	raw_selfpc;
	int32_t		raw_count;
};

typedef struct
#ifndef __LP64__
        rawarc
#else
        rawarc_64
#endif
rawarc_t;

/*
 * general rounding functions.
 */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))

/*
 * The profiling data structures are housed in this structure.
 */
struct gmonparam {
	int		state;
	u_short		*kcount;
	u_long		kcountsize;
	u_short		*froms;
	u_long		fromssize;
	tostruct_t	*tos;
	u_long		tossize;
	long		tolimit;
	u_long		lowpc;
	u_long		highpc;
	u_long		textsize;
	u_long		hashfraction;
};
extern struct gmonparam _gmonparam;

/*
 * Possible states of profiling.
 */
#define	GMON_PROF_ON	0
#define	GMON_PROF_BUSY	1
#define	GMON_PROF_ERROR	2
#define	GMON_PROF_OFF	3

/*
 * Sysctl definitions for extracting profiling information from the kernel.
 */
#define	GPROF_STATE	0	/* int: profiling enabling variable */
#define	GPROF_COUNT	1	/* struct: profile tick count buffer */
#define	GPROF_FROMS	2	/* struct: from location hash bucket */
#define	GPROF_TOS	3	/* struct: destination/count structure */
#define	GPROF_GMONPARAM	4	/* struct: profiling parameters (see above) */


/*
 * Declarations for various profiling related functions from
 * bsd/kern/subr_prof.c
 */
#ifdef GPROF
#endif /* GPROF */


/*
 * In order to support more information than in the original mon.out and
 * gmon.out files there is an alternate gmon.out file format.  The alternate
 * gmon.out file format starts with a magic number then separates the
 * information with gmon_data_t's.
 */
#define GMON_MAGIC 0xbeefbabe
#define GMON_MAGIC_64 0xbeefbabf
typedef struct gmon_data {
    uint32_t type; /* constant for type of data following this struct */
    uint32_t size; /* size in bytes of the data following this struct */
} gmon_data_t;

/*
 * The GMONTYPE_SAMPLES gmon_data.type is for the histogram counters described
 * above and has a gmonhdr_t followed by the counters.
 */
#define GMONTYPE_SAMPLES	1
/*
 * The GMONTYPE_RAWARCS gmon_data.type is for the raw arcs described above.
 */
#define GMONTYPE_RAWARCS	2
/*
 * The GMONTYPE_ARCS_ORDERS gmon_data.type is for the raw arcs with a call
 * order field.  The order is the order is a sequence number for the order each
 * call site was executed.  Raw_order values start at 1 not zero.  Other than
 * the raw_order field this is the same information as in the rawarc_t.
 */
#define GMONTYPE_ARCS_ORDERS	3
struct rawarc_order {
    uint32_t	raw_frompc;
    uint32_t	raw_selfpc;
    uint32_t	raw_count;
    uint32_t	raw_order;

}; struct rawarc_order_64 {
    uint64_t	raw_frompc;
    uint64_t	raw_selfpc;
    uint32_t	raw_count;
    uint32_t	raw_order;
};

typedef struct
#ifndef __LP64__
        rawarc_order
#else
        rawarc_order_64
#endif
rawarc_order_t;

/*
 * The GMONTYPE_DYLD_STATE gmon_data.type is for the dynamic link editor state
 * of the program.
 * The informations starts with an uint32_t with the count of states:
 *      image_count
 * Then each state follows in the file.  The state is made up of 
 *      vmaddr_slide (the amount dyld slid this image from it's vmaddress)
 *      name (the file name dyld loaded this image from)
 * The vmaddr_slide is a 32-bit value for 32-bit programs and 64-bit value for
 * 64-bit programs.
 */
#define GMONTYPE_DYLD_STATE     4

/*
 * The GMONTYPE_DYLD2_STATE gmon_data.type is for the dynamic link editor state
 * of the program.
 * The informations starts with an uint32_t with the count of states:
 *      image_count
 * Then each state follows in the file.  The state is made up of 
 *      image_header (the address where dyld loaded this image)
 *      name (the file name dyld loaded this image from)
 * The image_header is a 32-bit value for 32-bit programs and 64-bit value for
 * 64-bit programs.
 */
#define GMONTYPE_DYLD2_STATE     5

#endif /* !_SYS_GMON_H_ */
