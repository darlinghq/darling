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

#ifndef _MACHO_LDSYMS_H_
#define _MACHO_LDSYMS_H_

#include <mach-o/loader.h>

/*
 * This file describes the link editor defined symbols.  The semantics of a
 * link editor symbol is that it is defined by the link editor only if it is
 * referenced and it is an error for the user to define them (see the man page
 * ld(1)).  The standard UNIX link editor symbols: __end, __etext and __edata
 * are not not supported by the Apple Mach-O link editor.  These symbols are
 * really not meaningful in a Mach-O object file and the link editor symbols
 * that are supported (described here) replace them.  In the case of the
 * standard UNIX link editor symbols the program can use the symbol
 * __mh_execute_header and walk the load commands of it's program to determine
 * the ending (or beginning) of any section or segment in the program.  Note
 * that the compiler prepends an underbar to all external symbol names coded
 * in a high level language.  Thus in 'C' names are coded without an underbar
 * and symbol names in the symbol table have an underbar.  There are two cpp
 * macros for each link editor defined name in this file.  The macro with a
 * leading underbar is the symbol name and the one without is the name as
 * coded in 'C'.
 */

/*
 * The value of the link editor defined symbol _MH_EXECUTE_SYM is the address
 * of the mach header in a Mach-O executable file type.  It does not appear in
 * any file type other than a MH_EXECUTE file type.  The type of the symbol is
 * absolute as the header is not part of any section.
 */
#define _MH_EXECUTE_SYM	"__mh_execute_header"
#define MH_EXECUTE_SYM	"_mh_execute_header"
extern const struct
#ifdef __LP64__
mach_header_64
#else
mach_header
#endif
_mh_execute_header;

/*
 * The value of the link editor defined symbol _MH_BUNDLE_SYM is the address
 * of the mach header in a Mach-O bundle file type.  It does not appear in
 * any file type other than a MH_BUNDLE file type.  The type of the symbol is
 * an N_SECT symbol even thought the header is not part of any section.  This
 * symbol is private to the code in the bundle it is a part of.
 */
#define _MH_BUNDLE_SYM	"__mh_bundle_header"
#define MH_BUNDLE_SYM	"_mh_bundle_header"
extern const struct
#ifdef __LP64__
mach_header_64
#else
mach_header
#endif
_mh_bundle_header;

/*
 * The value of the link editor defined symbol _MH_DYLIB_SYM is the address
 * of the mach header in a Mach-O dylib file type.  It does not appear in
 * any file type other than a MH_DYLIB file type.  The type of the symbol is
 * an N_SECT symbol even thought the header is not part of any section.  This
 * symbol is private to the code in the library it is a part of.
 */
#define _MH_DYLIB_SYM	"__mh_dylib_header"
#define MH_DYLIB_SYM	"_mh_dylib_header"
extern const struct
#ifdef __LP64__
mach_header_64
#else
mach_header
#endif
_mh_dylib_header;

/*
 * The value of the link editor defined symbol _MH_DYLINKER_SYM is the address
 * of the mach header in a Mach-O dylinker file type.  It does not appear in
 * any file type other than a MH_DYLINKER file type.  The type of the symbol is
 * an N_SECT symbol even thought the header is not part of any section.  This
 * symbol is private to the code in the dynamic linker it is a part of.
 */
#define _MH_DYLINKER_SYM	"__mh_dylinker_header"
#define MH_DYLINKER_SYM		"_mh_dylinker_header"
extern const struct
#ifdef __LP64__
mach_header_64
#else
mach_header
#endif
_mh_dylinker_header;

/*
 * For the MH_PRELOAD file type the headers are not loaded as part of any
 * segment so the link editor defines symbols defined for the beginning
 * and ending of each segment and each section in each segment.  The names for
 * the symbols for a segment's beginning and end will have the form:
 * __SEGNAME__begin and  __SEGNAME__end where __SEGNAME is the name of the
 * segment.  The names for the symbols for a section's beginning and end will
 * have the form: __SEGNAME__sectname__begin and __SEGNAME__sectname__end
 * where __sectname is the name of the section and __SEGNAME is the segment it
 * is in.
 * 
 * The above symbols' types are those of the section they are referring to.
 * This is true even for symbols who's values are end's of a section and
 * that value is next address after that section and not really in that
 * section.  This results in these symbols having types referring to sections
 * who's values are not in that section.
 */

#endif /* _MACHO_LDSYMS_H_ */
