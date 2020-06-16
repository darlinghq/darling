/*
 * Copyright (c) 2013-19 Apple, Inc. All rights reserved.
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

#ifndef _XATTR_FLAGS_H
#define _XATTR_FLAGS_H

#include <stdint.h>

#include <sys/cdefs.h>
#include <Availability.h>

__BEGIN_DECLS

/*
 * xattr_operation_intent_t is used to declare what the intent of the copy is.
 * Not a bit-field (for now, at least).
 *
 * XATTR_OPERATION_INTENT_COPY indicates that the EA is attached to an object
 * that is simply being copied.  E.g., cp src dst
 *
 * XATTR_OPERATION_INTENT_SAVE indicates that the EA is attached to an object
 * being saved; as in a "safe save," the destination is being replaced by
 * the source, so the question is whether the EA should be applied to the
 * destination, or generated anew.
 *
 * XATTR_OPERATION_INTENT_SHARE  indicates that the EA is attached to an object that
 * is being given out to other people.  For example, saving to a public folder,
 * or attaching to an email message.
 *
 * XATTR_OPERATION_INTENT_SYNC  indicates that the EA is attached to an object that
 * is being synced to other storages for the same user.  For example synced to
 * iCloud.
 */

#define XATTR_OPERATION_INTENT_COPY	1
#define XATTR_OPERATION_INTENT_SAVE	2
#define XATTR_OPERATION_INTENT_SHARE	3
#define XATTR_OPERATION_INTENT_SYNC	4

typedef unsigned int xattr_operation_intent_t;

typedef uint64_t xattr_flags_t;

/*
 * Various properties used to determine how to handle the xattr during
 * copying.  The intent is that the default is reasonable for most xattrs.
 */

/*
 * XATTR_FLAG_NO_EXPORT
 * Declare that the extended property should not be exported; this is
 * deliberately a bit vague, but this is used by XATTR_OPERATION_INTENT_SHARE
 * to indicate not to preserve the xattr.
 */
#define XATTR_FLAG_NO_EXPORT	((xattr_flags_t)0x0001)

/*
 * XATTR_FLAG_CONTENT_DEPENDENT
 * Declares the extended attribute to be tied to the contents of the file (or
 * vice versa), such that it should be re-created when the contents of the
 * file change.  Examples might include cryptographic keys, checksums, saved
 * position or search information, and text encoding.
 *
 * This property causes the EA to be preserved for copy and share, but not for
 * safe save.  (In a safe save, the EA exists on the original, and will not
 * be copied to the new version.)
 */
#define	XATTR_FLAG_CONTENT_DEPENDENT	((xattr_flags_t)0x0002)

/*
 * XATTR_FLAG_NEVER_PRESERVE
 * Declares that the extended attribute is never to be copied, for any
 * intention type.
 */
#define XATTR_FLAG_NEVER_PRESERVE	((xattr_flags_t)0x0004)

/*
 * XATTR_FLAG_SYNCABLE
 * Declares that the extended attribute is to be synced, used by the
 * XATTR_OPERATION_ITENT_SYNC intention.  Syncing tends to want to minimize the
 * amount of metadata synced around, hence the default behavior is for the EA
 * NOT to be synced, even if it would else be preserved for the
 * XATTR_OPERATION_ITENT_COPY intention.
 */
#define XATTR_FLAG_SYNCABLE	((xattr_flags_t)0x0008)

/* Given a named extended attribute, and a copy intent, should the EA be preserved? */
extern int xattr_preserve_for_intent(const char *, xattr_operation_intent_t) __OSX_AVAILABLE_STARTING( __MAC_10_10, __IPHONE_8_0);

/*
 * Given an extended attribute name, and a set of properties, return an
 *  allocated C string with the name.  This will return NULL on error;
 * errno may be set to ENOMEM if the new name cannot be allocated, or
 * ENAMETOOLONG if the new name is longer than the maximum for EAs (127 UTF8
 * characters).  The caller must deallocate the return value otherwise.
 *
 * If no properties are set, it returns a copy of the EA name.
 *
 * If the EA name is in the internal list, and the properties are the same as
 * defined there, then it will also return an unmodified copy of the EA name.
 */
extern char *xattr_name_with_flags(const char *, xattr_flags_t) __OSX_AVAILABLE_STARTING( __MAC_10_10, __IPHONE_8_0);

/*
 * Given an extended attribute name, which may or may not have properties encoded
 * as a suffix, return just the name of the attribute.  E.g., com.example.mine#P
 * would return "com.example.mine".  The return value will be NULL on error;
 * errno will be set to ENOMEM if it cannot be allocated.  The caller must deallocate
 * the return value.
 */
extern char *xattr_name_without_flags(const char *) __OSX_AVAILABLE_STARTING( __MAC_10_10, __IPHONE_8_0);

/*
 * Given an EA name, return the properties.  If the name is in the internal list,
 * those properties will be returned.  Unknown property encodings are ignored.
 */
extern xattr_flags_t xattr_flags_from_name(const char *) __OSX_AVAILABLE_STARTING( __MAC_10_10, __IPHONE_8_0);

/*
 * Given an xattr_operation_intent_t and an xattr_flags_t, return whether it should
 * be preserved.  The default (in case either flags or intent is 0, or unknown
 * values) is to return 1; it only returns 0 if the flags and intent indicate it
 * should not be preserved.
 */
extern int xattr_intent_with_flags(xattr_operation_intent_t, xattr_flags_t) __OSX_AVAILABLE_STARTING( __MAC_10_10, __IPHONE_8_0);

__END_DECLS

#endif /* _XATTR_FLAGS_H */
