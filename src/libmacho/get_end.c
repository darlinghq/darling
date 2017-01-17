/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef RLD
#include <string.h>
#include <mach-o/ldsyms.h>
#include <mach-o/getsect.h>
#ifndef __OPENSTEP__
#include <crt_externs.h>
#else /* defined(__OPENSTEP__) */
#ifdef __DYNAMIC__
#include "mach-o/dyld.h" /* defines _dyld_lookup_and_bind() */
#endif

#if !defined(__DYNAMIC__)
#define DECLARE_VAR(var, type) \
extern type var
#define SETUP_VAR(var)
#define USE_VAR(var) var
#else
#define STRINGIFY(a) # a
#define DECLARE_VAR(var, type)	\
static type * var ## _pointer = 0
#define SETUP_VAR(var)						\
if ( var ## _pointer == 0) {				\
    _dyld_lookup_and_bind( STRINGIFY(_ ## var),		\
                           (uint32_t *) & var ## _pointer, 0);	\
}
#define USE_VAR(var) (* var ## _pointer)
#endif
#endif /* __OPENSTEP__ */

/*
 * This routine returns the highest address of the segments in the program (NOT
 * the shared libraries).  It is intended to be used as a stop gap for programs
 * that make UNIX style assumptions about how memory is allocated.  Typicly the
 * asumptions under which this is used is that memory is contiguously allocated
 * by the program's text and data from address 0 with no gaps.  The value of
 * this differs from the value of &_end in a UNIX program in that this routine
 * returns the address of the end of the segment not the end of the last section
 * in that segment as would be the value of the symbol &_end.
 */
unsigned long
get_end(void)
{
#ifndef __LP64__

    struct segment_command *sgp;
    unsigned long _end;
    uint32_t i;
#ifndef __OPENSTEP__
    struct mach_header *mhp = _NSGetMachExecuteHeader();
#else /* defined(__OPENSTEP__) */
    static struct mach_header *mhp = NULL;
	DECLARE_VAR(_mh_execute_header, struct mach_header);
	SETUP_VAR(_mh_execute_header);

	mhp = (struct mach_header *)(& USE_VAR(_mh_execute_header));
#endif /* __OPENSTEP__ */
	_end = 0;
	sgp = (struct segment_command *)
	      ((char *)mhp + sizeof(struct mach_header));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT)
		if(sgp->vmaddr + sgp->vmsize > _end)
		    _end = sgp->vmaddr + sgp->vmsize;
	    sgp = (struct segment_command *)((char *)sgp + sgp->cmdsize);
	}
	return(_end);

#else /* defined(__LP64__) */

    struct mach_header_64 *mhp = _NSGetMachExecuteHeader();
    struct segment_command_64 *sgp;
    unsigned long _end;
    uint32_t i;

	_end = 0;
	sgp = (struct segment_command_64 *)
	      ((char *)mhp + sizeof(struct mach_header_64));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT_64)
		if(sgp->vmaddr + sgp->vmsize > _end)
		    _end = sgp->vmaddr + sgp->vmsize;
	    sgp = (struct segment_command_64 *)((char *)sgp + sgp->cmdsize);
	}
	return(_end);

#endif /* defined(__LP64__) */
}

/*
 * This returns what was the value of the UNIX link editor defined symbol
 * _etext (the first address after the text section).  Note this my or may not
 * be the only section in the __TEXT segment.
 */
unsigned long
get_etext(void)
{
#ifndef __LP64__
    const struct section *sp;
#else /* defined(__LP64__) */
    const struct section_64 *sp;
#endif /* defined(__LP64__) */

	sp = getsectbyname(SEG_TEXT, SECT_TEXT);
	if(sp)
	    return(sp->addr + sp->size);
	else
	    return(0);
}

/*
 * This returns what was the value of the UNIX link editor defined symbol
 * _edata (the first address after the data section).  Note this my or may not
 * be the last non-zero fill section in the __DATA segment.
 */
unsigned long
get_edata(void)
{
#ifndef __LP64__
    const struct section *sp;
#else /* defined(__LP64__) */
    const struct section_64 *sp;
#endif /* defined(__LP64__) */

	sp = getsectbyname(SEG_DATA, SECT_DATA);
	if(sp)
	    return(sp->addr + sp->size);
	else
	    return(0);
}
#endif /* !defined(RLD) */
