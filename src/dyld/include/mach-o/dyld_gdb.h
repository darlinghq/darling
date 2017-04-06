/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
#ifndef _DYLD_GDB_
#define _DYLD_GDB_

/*
 * For Mac OS X 10.4 or later, use the interface in mach-o/dylib_images.h
 */
#include <mach-o/dyld_images.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Prior to Mac OS 10.4, this is the interface gdb used to discover the mach-o images loaded in a process
 */
#if __i386__
/*
 * gdb_dyld_version is the version of gdb interface that dyld is currently
 * exporting.  For the interface described in this header file gdb_dyld_version
 * is 2.  As the gdb/dyld interface changes this number will be incremented and
 * comments will be added as to what are the are changes for the various
 * versions.
 */
extern unsigned int gdb_dyld_version;

/* 
 * gdb_dyld_state_changed is the internal dyld routine called by dyld to notify
 * gdb that the state of the data structures has changed.  gdb is expected to
 * put a break point on this routine and re-read the internal dyld data
 * structures below when this break point is hit.
 */
extern void gdb_dyld_state_changed(void);

/*
 * gdb looks directly at parts of two of dyld's internal data structures.  The
 * list of object file images and the list of library images.  The parts of
 * these structures that gdb looks at will not change unless the value of
 * gdb_dyld_version changes.  The size of these structures and the other fields
 * that gdb does not look at may change.
 *
 *  struct object_images {
 *      struct object_image images[NOBJECT_IMAGES];
 *      unsigned long nimages; 
 *      struct object_images *next_images; 
 *      ...
 *  };
 *
 *  struct library_images { 
 *      struct library_image images[NLIBRARY_IMAGES];
 *      unsigned long nimages;
 *      struct library_images *next_images;
 *      ...
 *  };
 *
 * Both the object_image structure and the library_image structure
 * start with a structure containing the following fields:
 *
 *  struct image {   
 *      char *physical_name;        physical image name (file name)
 *      unsigned long vmaddr_slide; the slide from the staticly linked address
 *      struct mach_header *mh;     address of the mach header of the image
 *	unsigned long valid;        TRUE if this is struct is valid
 *      char *name;                 image name for reporting errors
 *      ...
 *  };
 *
 * In gdb_dyld_version 1 the first field was "name".  In gdb_dyld_version 2 the
 * first field was changed to "physical_name" and a new fifth field "name" was
 * added.  These two fields are set to the same values except in the case of
 * zero-link.  In zero-link the NSLinkModule() option
 * NSLINKMODULE_OPTION_TRAILING_PHYS_NAME is used and then the physical_name is
 * the file name of the module zero-link loaded that is part of the logical
 * image "name".
 */

/* object_images is the global object_images structure */

/* the number of gdb_object_image structures present per bucket */
extern unsigned int gdb_nobject_images;

/* the size of each gdb_object_image structure */
extern unsigned int gdb_object_image_size;

/* library_images is the global library_images structure */

/* the number of gdb_library_image structures present per bucket */
extern unsigned int gdb_nlibrary_images;

/* the size of each gdb_library_image structure */
extern unsigned int gdb_library_image_size;

#endif 



#ifdef __cplusplus
}
#endif

#endif /* _DYLD_GDB_ */
