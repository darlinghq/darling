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
#include <mach-o/ldsyms.h>
#include <string.h>
#ifndef __OPENSTEP__

#ifndef RLD
#include <crt_externs.h>
#endif /* !defined(RLD) */

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
 * This routine returns the segment_command structure for the named segment if
 * it exist in the mach executible it is linked into.  Otherwise it returns
 * zero.  It uses the link editor defined symbol _mh_execute_header and just
 * looks through the load commands.  Since these are mapped into the text
 * segment they are read only and thus const.
 */
#ifndef __LP64__

const struct segment_command *
getsegbyname(
char *segname)
{
    struct segment_command *sgp;
    uint32_t i;
#ifndef RLD
#ifndef __OPENSTEP__
    struct mach_header *mhp = _NSGetMachExecuteHeader();
#else /* defined(__OPENSTEP__) */
    static struct mach_header *mhp = NULL;
        DECLARE_VAR(_mh_execute_header, struct mach_header);
        SETUP_VAR(_mh_execute_header);
	mhp = (struct mach_header *)(& USE_VAR(_mh_execute_header));
#endif /* __OPENSTEP__ */
#else /* defined(RLD) */
	mhp = (struct mach_header *)(&_mh_execute_header);
#endif /* defined(RLD) */
        
	sgp = (struct segment_command *)
	      ((char *)mhp + sizeof(struct mach_header));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT)
		if(strncmp(sgp->segname, segname, sizeof(sgp->segname)) == 0)
		    return(sgp);
	    sgp = (struct segment_command *)((char *)sgp + sgp->cmdsize);
	}
	return(NULL);
}

#else /* defined(__LP64__) */

const struct segment_command_64 *
getsegbyname(
char *segname)
{
    struct mach_header_64 *mhp = NULL;
    struct segment_command_64 *sgp;
    uint32_t i;

#ifndef RLD
	mhp = _NSGetMachExecuteHeader();
#else /* defined(RLD) */
	mhp = (struct mach_header_64 *)(&_mh_execute_header);
#endif /* defined(RLD) */
        
	sgp = (struct segment_command_64 *)
	      ((char *)mhp + sizeof(struct mach_header_64));
	for(i = 0; i < mhp->ncmds; i++){
	    if(sgp->cmd == LC_SEGMENT_64)
		if(strncmp(sgp->segname, segname, sizeof(sgp->segname)) == 0)
		    return(sgp);
	    sgp = (struct segment_command_64 *)((char *)sgp + sgp->cmdsize);
	}
	return(NULL);
}
#endif /* defined(__LP64__) */
