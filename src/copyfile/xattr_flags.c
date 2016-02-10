/*
 * Copyright (c) 2013 Apple, Inc. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <dispatch/dispatch.h>
#include <xpc/private.h>

#include <xattr_flags.h>

#define FLAG_DELIM_CHAR	'#'
#define FLAG_DELIM_STR "#"

/*
 * Some default propeteries for EAs we know about internally.
 */
struct defaultList {
	const char *eaName;
	const char *propList;
	int flags;	// See below
};

#define propFlagsPrefix	0x0001	// The name is a prefix, so only look at that part

static const struct defaultList *defaultPropertyTable = NULL;

static const struct defaultList
defaultUnboxedPropertyTable[] = {
	{ "com.apple.quarantine", "PCS", 0 },	// not public
	{ "com.apple.TextEncoding", "CS", 0 },	// Content-dependent, public
	{ "com.apple.metadata:", "PS", propFlagsPrefix },	// Don't export, keep for copy & safe save
	{ "com.apple.security.", "S", propFlagsPrefix },
	{ XATTR_RESOURCEFORK_NAME, "PCS", 0 },	// Don't keep for safe save
	{ XATTR_FINDERINFO_NAME, "PCS", 0 },	// Same as ResourceFork
	{ 0, 0, 0 },
};

static const struct defaultList
defaultSandboxedPropertyTable[] = {
    { "com.apple.quarantine", "PCS", 0 },	// not public
    { "com.apple.TextEncoding", "CS", 0 },	// Content-dependent, public
    { "com.apple.metadata:", "PS", propFlagsPrefix },	// Don't export, keep for copy & safe save
    { "com.apple.security.", "N", propFlagsPrefix },
    { XATTR_RESOURCEFORK_NAME, "PCS", 0 },	// Don't keep for safe save
    { XATTR_FINDERINFO_NAME, "PCS", 0 },	// Same as ResourceFork
    { 0, 0, 0 },
};

/*
 * The property lists on an EA are set by having a suffix character,
 * and then a list of characters.  In general, we're choosing upper-case
 * to indicate the property is set, and lower-case to indicate it's to be
 * cleared.
 */
struct propertyListMapping {
	char enable;	// Character to enable
	char disable;	// Character to disable -- usually lower-case of enable
	xattr_operation_intent_t	value;
};
static const struct propertyListMapping
PropertyListMapTable[] = {
	{ 'C', 'c', XATTR_FLAG_CONTENT_DEPENDENT },
	{ 'P', 'p', XATTR_FLAG_NO_EXPORT },
	{ 'N', 'n', XATTR_FLAG_NEVER_PRESERVE },
	{ 'S', 's', XATTR_FLAG_SYNCABLE },
	{ 0, 0, 0 },
};
	
/*
 * Given a converted property list (that is, converted to the
 * xattr_operation_intent_t type), and an intent, determine if
 * it should be preserved or not.
 *
 * I've chosen to use a block instead of a simple mask on the belief
 * that the question may be moderately complex.  If it ends up not being
 * so, then this can simply be turned into a mask of which bits to check
 * as being exclusionary.
 */
static const struct divineIntent {
	xattr_operation_intent_t intent;
	int (^checker)(xattr_flags_t);
} intentTable[] = {
	{ XATTR_OPERATION_INTENT_COPY, ^(xattr_flags_t flags) {
			if (flags & XATTR_FLAG_NEVER_PRESERVE)
				return 0;
			return 1;
		} },
	{ XATTR_OPERATION_INTENT_SAVE, ^(xattr_flags_t flags) {
			if (flags & (XATTR_FLAG_CONTENT_DEPENDENT | XATTR_FLAG_NEVER_PRESERVE))
				return 0;
			return 1;
		} },
	{ XATTR_OPERATION_INTENT_SHARE, ^(xattr_flags_t flags) {
			if ((flags & (XATTR_FLAG_NO_EXPORT | XATTR_FLAG_NEVER_PRESERVE)) != 0)
				return 0;
			return 1;
		} },
	{ XATTR_OPERATION_INTENT_SYNC, ^(xattr_flags_t flags) {
			return (flags & (XATTR_FLAG_SYNCABLE | XATTR_FLAG_NEVER_PRESERVE)) == XATTR_FLAG_SYNCABLE;
		} },
	{ 0, 0 },
};


/*
 * If an EA name is in the default list, find it, and return the property
 * list string for it.
 */
static const char *
nameInDefaultList(const char *eaname)
{
	const struct defaultList *retval;
	static dispatch_once_t onceToken;
    
	dispatch_once(&onceToken, ^{
		if (_xpc_runtime_is_app_sandboxed()) {
			defaultPropertyTable = defaultSandboxedPropertyTable;
		} else {
			defaultPropertyTable = defaultUnboxedPropertyTable;
		}
	});

	for (retval = defaultPropertyTable; retval->eaName; retval++) {
		if ((retval->flags & propFlagsPrefix) != 0 &&
		    strncmp(retval->eaName, eaname, strlen(retval->eaName)) == 0)
			return retval->propList;
		if (strcmp(retval->eaName, eaname) == 0)
			return retval->propList;
	}
	return NULL;
}

/*
 * Given an EA name, see if it has a property list in it, and
 * return a pointer to it.  All this is doing is looking for
 * the delimiter, and returning the string after that.  Returns
 * NULL if the delimiter isn't found.  Note that an empty string
 * is a valid property list, as far as we're concerned.
 */
static const char *
findPropertyList(const char *eaname)
{
	const char *ptr = strrchr(eaname, '#');
	if (ptr)
		return ptr+1;
	return NULL;
}

/*
 * Convert a property list string (e.g., "pCd") into a
 * xattr_operation_intent_t type.
 */
static xattr_operation_intent_t
stringToProperties(const char *proplist)
{
	xattr_operation_intent_t retval = 0;
	const char *ptr;

	// A switch would be more efficient, but less generic.
	for (ptr = proplist; *ptr; ptr++) {
		const struct propertyListMapping *mapPtr;
		for (mapPtr = PropertyListMapTable; mapPtr->enable; mapPtr++) {
			if (*ptr == mapPtr->enable) {
				retval |= mapPtr->value;
			} else if (*ptr == mapPtr->disable) {
				retval &= ~mapPtr->value;
			}
		}
	}
	return retval;
}

/*
 * Given an EA name (e.g., "com.apple.lfs.hfs.test"), and a
 * xattr_operation_intent_t value (it's currently an integral value, so
 * just a bitmask), cycle through the list of known properties, and return
 * a string with the EA name, and the property list appended.  E.g., we
 * might return "com.apple.lfs.hfs.test#pD".
 *
 * The tricky part of this funciton is that it will not append any letters
 * if the value is only the default properites.  In that case, it will copy
 * the EA name, and return that.
 *
 * It returns NULL if there was an error.  The two errors right now are
 * no memory (strdup failed), in which case it will set errno to ENOMEM; and
 * the resulting EA name is longer than XATTR_MAXNAMELEN, in which case it
 * sets errno to ENAMETOOLONG.
 *
 * (Note that it also uses ENAMETOOLONG if the buffer it's trying to set
 * gets too large.  I honestly can't see how that would happen, but it's there
 * for sanity checking.  That would require having more than 64 bits to use.)
 */
char *
xattr_name_with_flags(const char *orig, xattr_flags_t propList)
{
	char *retval = NULL;
	char suffix[66] = { 0 }; // 66:  uint64_t for property types, plus '#', plus NUL
	char *cur = suffix;
	const struct propertyListMapping *mapPtr;

	*cur++ = '#';
	for (mapPtr = PropertyListMapTable; mapPtr->enable; mapPtr++) {
		if ((propList & mapPtr->value) != 0) {
			*cur++ = mapPtr->enable;
		}
		if (cur >= (suffix + sizeof(suffix))) {
			errno = ENAMETOOLONG;
			return NULL;
		}

	}
	
		
	if (cur == suffix + 1) {
		// No changes made
		retval = strdup(orig);
		if (retval == NULL)
			errno = ENOMEM;
	} else {
		const char *defaultEntry = NULL;
		if ((defaultEntry = nameInDefaultList(orig)) != NULL &&
		    strcmp(defaultEntry, suffix + 1) == 0) {
			// Just use the name passed in
			retval = strdup(orig);
		} else {
			asprintf(&retval, "%s%s", orig, suffix);
		}
		if (retval == NULL) {
			errno = ENOMEM;
		} else {
			if (strlen(retval) > XATTR_MAXNAMELEN) {
				free(retval);
				retval = NULL;
				errno = ENAMETOOLONG;
			}
		}
	}
	return retval;
}

char *
xattr_name_without_flags(const char *eaname)
{
	char *retval = NULL;
	char *tmp;

	if ((tmp = strrchr(eaname, FLAG_DELIM_CHAR)) == NULL) {
		retval = strdup(eaname);
	} else {
		retval = calloc(tmp - eaname + 1, 1);
		if (retval) {
			strlcpy(retval, eaname, tmp - eaname + 1);
		}
	}
	if (retval == NULL) {
		errno = ENOMEM;
	}
	return retval;
}

int
xattr_intent_with_flags(xattr_operation_intent_t intent, xattr_flags_t flags)
{
	const struct divineIntent *ip;

	for (ip = intentTable; ip->intent; ip++) {
		if (ip->intent == intent) {
			return ip->checker(flags);
		}
	}
	if ((flags & XATTR_FLAG_NEVER_PRESERVE) != 0)
		return 0;	// Special case, don't try to copy this one

	return 1;	// Default
}

xattr_flags_t
xattr_flags_from_name(const char *eaname)
{
	xattr_flags_t retval = 0;
	const char *propList;

	propList = findPropertyList(eaname);
	if (propList == NULL) {
		propList = nameInDefaultList(eaname);
	}
	if (propList != NULL) {
		retval = stringToProperties(propList);
	}
	
	return retval;
}

/*
 * Indicate whether an EA should be preserved, when using the
 * given intent.
 * 
 * This returns 0 if it should not be preserved, and 1 if it should.
 * 
 * It simply looks through the tables we have above, and compares the
 * xattr_operation_intent_t for the EA with the intent.  If the
 * EA doesn't have any properties, and it's not on the default list, the
 * default is to preserve it.
 */

int
xattr_preserve_for_intent(const char *eaname, xattr_operation_intent_t intent)
{
	xattr_flags_t flags = xattr_flags_from_name(eaname);

	return xattr_intent_with_flags(intent, flags);
}

#include "xattr_properties.h"
