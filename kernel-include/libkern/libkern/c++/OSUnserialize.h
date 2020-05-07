/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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
/* OSUnserialize.h created by rsulack on Mon 23-Nov-1998 */

#ifndef _OS_OSUNSERIALIZE_H
#define _OS_OSUNSERIALIZE_H

#include <libkern/c++/OSMetaClass.h>
#include <libkern/c++/OSString.h>

#include <sys/appleapiopts.h>
#include <sys/types.h>

class OSObject;
class OSString;

/*!
 * @header
 *
 * @abstract
 * This header declares the <code>OSUnserializeXML</code> function.
 */


/*!
 * @function OSUnserializeXML
 *
 * @abstract
 * Recreates an OSContainer object
 * from its previously serialized OSContainer class instance data.
 *
 * @param buffer      A buffer containing nul-terminated XML data
 *                    representing the object to be recreated.
 * @param errorString If non-<code>NULL</code>, and the XML parser
 *                    finds an error in <code>buffer</code>,
 *                    <code>*errorString</code> indicates the line number
 *                    and type of error encountered.
 *
 * @result
 * The recreated object, or <code>NULL</code> on failure.
 *
 * @discussion
 * <b>Not safe</b> to call in a primary interrupt handler.
 */
extern "C++" OSObjectPtr OSUnserializeXML(
	const char  * buffer,
	OSStringPtr * errorString = NULL);

/*!
 * @function OSUnserializeXML
 *
 * @abstract
 * Recreates an OSContainer object
 * from its previously serialized OSContainer class instance data.
 *
 * @param buffer      A buffer containing nul-terminated XML data
 *                    representing the object to be recreated.
 * @param bufferSize  The size of the block of memory. The function
 *                    never scans beyond the first bufferSize bytes.
 * @param errorString If non-<code>NULL</code>, and the XML parser
 *                    finds an error in <code>buffer</code>,
 *                    <code>*errorString</code> indicates the line number
 *                    and type of error encountered.
 *
 * @result
 * The recreated object, or <code>NULL</code> on failure.
 *
 * @discussion
 * <b>Not safe</b> to call in a primary interrupt handler.
 */
extern "C++" OSObjectPtr OSUnserializeXML(
	const char  * buffer,
	size_t        bufferSize,
	OSStringPtr   *errorString = NULL);

extern "C++" OSObjectPtr
OSUnserializeBinary(const char *buffer, size_t bufferSize, OSStringPtr *errorString);

#ifdef __APPLE_API_OBSOLETE
extern OSObjectPtr OSUnserialize(const char *buffer, OSStringPtr *errorString = NULL);
#endif /* __APPLE_API_OBSOLETE */

#endif /* _OS_OSUNSERIALIZE_H */
