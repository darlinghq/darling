/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 The local directory node.

 Sessions, nodes, records and queries are CF types, so they can be retained,
 released and wrapped by the OpenDirectory.framework classes. There is one
 node, "/Local/Default", which serves the user and group accounts that
 Libinfo knows about (getpwnam_r(), getgrnam_r() and their enumerators); its
 UUIDs come from the membership API. "/Search" is the search node and, with
 nothing else configured, finds exactly what the local node holds.

 Records are snapshots of the account entry taken when the record is
 copied. Password hashes are never stored in a record or returned as an
 attribute; ODRecordVerifyPassword() asks Libinfo again.
*/

#include "../include/generated-stubs.h"
#include <CFOpenDirectory/CFOpenDirectoryPriv.h>
#include <CFOpenDirectory/CFOpenDirectoryConstants.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include "od_passwd_file.h"

#include <errno.h>
#include <grp.h>
#include <membership.h>
#include <pthread.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>

const CFStringRef kODErrorDomainFramework = CFSTR("com.apple.OpenDirectory");

#define LOCAL_NODE_NAME CFSTR("/Local/Default")
#define SEARCH_NODE_NAME CFSTR("/Search")

// Bit that turns a match type into its case-insensitive variant
// (kODMatchInsensitiveEqualTo is kODMatchEqualTo | 0x100).
#define MATCH_INSENSITIVE_BIT 0x0100

struct __ODSession
{
	CFRuntimeBase base;
};

struct __ODNode
{
	CFRuntimeBase base;
	CFStringRef name;
};

struct __ODRecord
{
	CFRuntimeBase base;
	CFStringRef recordType;
	CFStringRef recordName;
	// attribute name -> CFArray of CFString values
	CFDictionaryRef attributes;
	// attributes changed by ODRecordSetValue() and not yet written back
	CFMutableSetRef changed;
	// UniqueID of a user record, -1 for other records
	long long uid;
};

struct __ODQuery
{
	CFRuntimeBase base;
	ODNodeRef node;
	CFArrayRef recordTypes;
	CFStringRef attribute;
	ODMatchType matchType;
	CFArrayRef values;
	CFIndex maxResults;
};

#pragma mark - CF classes

static void nodeFinalize(CFTypeRef cf)
{
	struct __ODNode* node = (struct __ODNode*) cf;
	if (node->name)
		CFRelease(node->name);
}

static void recordFinalize(CFTypeRef cf)
{
	struct __ODRecord* record = (struct __ODRecord*) cf;
	if (record->recordType)
		CFRelease(record->recordType);
	if (record->recordName)
		CFRelease(record->recordName);
	if (record->attributes)
		CFRelease(record->attributes);
	if (record->changed)
		CFRelease(record->changed);
}

static void queryFinalize(CFTypeRef cf)
{
	struct __ODQuery* query = (struct __ODQuery*) cf;
	if (query->node)
		CFRelease(query->node);
	if (query->recordTypes)
		CFRelease(query->recordTypes);
	if (query->attribute)
		CFRelease(query->attribute);
	if (query->values)
		CFRelease(query->values);
}

static const CFRuntimeClass sessionClass = { 0, "ODSession", NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static const CFRuntimeClass nodeClass = { 0, "ODNode", NULL, NULL, nodeFinalize, NULL, NULL, NULL, NULL };
static const CFRuntimeClass recordClass = { 0, "ODRecord", NULL, NULL, recordFinalize, NULL, NULL, NULL, NULL };
static const CFRuntimeClass queryClass = { 0, "ODQuery", NULL, NULL, queryFinalize, NULL, NULL, NULL, NULL };

static CFTypeID sessionTypeID, nodeTypeID, recordTypeID, queryTypeID;
// Guards the attributes of records, which ODRecordSetValue() can replace
static pthread_mutex_t recordLock = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t classesOnce = PTHREAD_ONCE_INIT;

static void registerClasses(void)
{
	sessionTypeID = _CFRuntimeRegisterClass(&sessionClass);
	nodeTypeID = _CFRuntimeRegisterClass(&nodeClass);
	recordTypeID = _CFRuntimeRegisterClass(&recordClass);
	queryTypeID = _CFRuntimeRegisterClass(&queryClass);
}

static void* createInstance(CFAllocatorRef allocator, const CFTypeID* typeID, size_t size)
{
	pthread_once(&classesOnce, registerClasses);
	return (void*) _CFRuntimeCreateInstance(allocator, *typeID, size - sizeof(CFRuntimeBase), NULL);
}

CFTypeID ODSessionGetTypeID(void)
{
	pthread_once(&classesOnce, registerClasses);
	return sessionTypeID;
}

CFTypeID ODNodeGetTypeID(void)
{
	pthread_once(&classesOnce, registerClasses);
	return nodeTypeID;
}

CFTypeID ODRecordGetTypeID(void)
{
	pthread_once(&classesOnce, registerClasses);
	return recordTypeID;
}

CFTypeID ODQueryGetTypeID(void)
{
	pthread_once(&classesOnce, registerClasses);
	return queryTypeID;
}

static bool isA(CFTypeRef object, CFTypeID typeID)
{
	return object != NULL && CFGetTypeID(object) == typeID;
}

#pragma mark - Helpers

static void setError(CFErrorRef* error, CFIndex code)
{
	if (error != NULL)
		*error = CFErrorCreate(kCFAllocatorDefault, kODErrorDomainFramework, code, NULL);
}

static void clearError(CFErrorRef* error)
{
	if (error != NULL)
		*error = NULL;
}

static CFStringRef createString(const char* s)
{
	return CFStringCreateWithCString(kCFAllocatorDefault, s, kCFStringEncodingUTF8);
}

// Returns a malloc'ed UTF-8 copy of the string
static char* copyCString(CFStringRef s)
{
	if (!isA(s, CFStringGetTypeID()))
		return NULL;

	CFIndex max = CFStringGetMaximumSizeForEncoding(CFStringGetLength(s), kCFStringEncodingUTF8) + 1;
	char* buffer = malloc(max);
	if (buffer != NULL && !CFStringGetCString(s, buffer, max, kCFStringEncodingUTF8))
	{
		free(buffer);
		buffer = NULL;
	}
	return buffer;
}

// Stores a single value for the attribute; consumes the value
static void addValue(CFMutableDictionaryRef attributes, CFStringRef attribute, CFStringRef value)
{
	if (value == NULL)
		return;

	CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void**) &value, 1, &kCFTypeArrayCallBacks);
	CFDictionarySetValue(attributes, attribute, array);
	CFRelease(array);
	CFRelease(value);
}

static void addCString(CFMutableDictionaryRef attributes, CFStringRef attribute, const char* value)
{
	if (value != NULL && value[0] != '\0')
		addValue(attributes, attribute, createString(value));
}

static void addNumber(CFMutableDictionaryRef attributes, CFStringRef attribute, long long value)
{
	addValue(attributes, attribute, CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%lld"), value));
}

static void addUUID(CFMutableDictionaryRef attributes, const uuid_t uuid)
{
	uuid_string_t text;
	uuid_unparse_upper(uuid, text);
	addCString(attributes, kODAttributeTypeGUID, text);
}

static bool isRecordType(CFTypeRef type, CFStringRef wanted)
{
	return isA(type, CFStringGetTypeID()) && CFEqual(type, wanted);
}

static bool isSupportedRecordType(CFTypeRef type)
{
	return isRecordType(type, kODRecordTypeUsers) || isRecordType(type, kODRecordTypeGroups);
}

static ODRecordRef createRecord(CFStringRef recordType, CFMutableDictionaryRef attributes, const char* name)
{
	struct __ODRecord* record = createInstance(kCFAllocatorDefault, &recordTypeID, sizeof(struct __ODRecord));
	if (record == NULL)
		return NULL;

	addValue(attributes, kODAttributeTypeRecordType, CFStringCreateCopy(kCFAllocatorDefault, recordType));
	// Records found through the search node also live in the local node.
	addValue(attributes, kODAttributeTypeMetaNodeLocation, CFStringCreateCopy(kCFAllocatorDefault, LOCAL_NODE_NAME));

	record->uid = -1;
	record->recordType = CFStringCreateCopy(kCFAllocatorDefault, recordType);
	record->recordName = createString(name);
	record->attributes = CFDictionaryCreateCopy(kCFAllocatorDefault, attributes);
	return record;
}

// The gecos field of an account holds comma separated parts: the full name,
// the office (building), the office phone and the home phone.
static CFStringRef const* gecosAttributes(void)
{
	static CFStringRef attributes[4];
	attributes[0] = kODAttributeTypeFullName;
	attributes[1] = kODAttributeTypeBuilding;
	attributes[2] = kODAttributeTypePhoneNumber;
	attributes[3] = kODAttributeTypeHomePhoneNumber;
	return attributes;
}

static void addGecos(CFMutableDictionaryRef attributes, const char* gecos)
{
	CFStringRef const* names = gecosAttributes();

	if (gecos == NULL)
		return;
	for (int i = 0; i < 4; i++)
	{
		const char* end = i < 3 ? strchr(gecos, ',') : NULL;
		size_t length = end != NULL ? (size_t) (end - gecos) : strlen(gecos);

		if (length > 0)
			addValue(attributes, names[i], CFStringCreateWithBytes(kCFAllocatorDefault,
					(const UInt8*) gecos, length, kCFStringEncodingUTF8, false));
		if (end == NULL)
			break;
		gecos = end + 1;
	}
}

static ODRecordRef createUserRecord(const struct passwd* pw)
{
	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	uuid_t uuid;

	addCString(attributes, kODAttributeTypeRecordName, pw->pw_name);
	addNumber(attributes, kODAttributeTypeUniqueID, (long long) pw->pw_uid);
	addNumber(attributes, kODAttributeTypePrimaryGroupID, (long long) pw->pw_gid);
	addGecos(attributes, pw->pw_gecos);
	addCString(attributes, kODAttributeTypeNFSHomeDirectory, pw->pw_dir);
	addCString(attributes, kODAttributeTypeUserShell, pw->pw_shell);
	if (mbr_uid_to_uuid(pw->pw_uid, uuid) == 0)
		addUUID(attributes, uuid);

	ODRecordRef record = createRecord(kODRecordTypeUsers, attributes, pw->pw_name);
	if (record != NULL)
		record->uid = (long long) pw->pw_uid;
	CFRelease(attributes);
	return record;
}

static ODRecordRef createGroupRecord(const struct group* gr)
{
	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	uuid_t uuid;

	addCString(attributes, kODAttributeTypeRecordName, gr->gr_name);
	addNumber(attributes, kODAttributeTypePrimaryGroupID, (long long) gr->gr_gid);
	if (mbr_gid_to_uuid(gr->gr_gid, uuid) == 0)
		addUUID(attributes, uuid);

	if (gr->gr_mem != NULL && gr->gr_mem[0] != NULL)
	{
		CFMutableArrayRef members = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		for (char** member = gr->gr_mem; *member != NULL; member++)
		{
			CFStringRef value = createString(*member);
			if (value != NULL)
			{
				CFArrayAppendValue(members, value);
				CFRelease(value);
			}
		}
		CFDictionarySetValue(attributes, kODAttributeTypeGroupMembership, members);
		CFRelease(members);
	}

	ODRecordRef record = createRecord(kODRecordTypeGroups, attributes, gr->gr_name);
	CFRelease(attributes);
	return record;
}

// getpwent()/getgrent() keep process-wide iteration state
static pthread_mutex_t enumerationLock = PTHREAD_MUTEX_INITIALIZER;

static void appendRecord(CFMutableArrayRef records, ODRecordRef record)
{
	if (record != NULL)
	{
		CFArrayAppendValue(records, record);
		CFRelease(record);
	}
}

// All records of the type, in the order Libinfo enumerates them
static CFArrayRef copyAllRecords(CFTypeRef recordType)
{
	CFMutableArrayRef records = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	pthread_mutex_lock(&enumerationLock);
	if (isRecordType(recordType, kODRecordTypeUsers))
	{
		struct passwd* pw;
		setpwent();
		while ((pw = getpwent()) != NULL)
			appendRecord(records, createUserRecord(pw));
		endpwent();
	}
	else if (isRecordType(recordType, kODRecordTypeGroups))
	{
		struct group* gr;
		setgrent();
		while ((gr = getgrent()) != NULL)
			appendRecord(records, createGroupRecord(gr));
		endgrent();
	}
	pthread_mutex_unlock(&enumerationLock);

	return records;
}

// Record names are matched without regard to case, as the directory schema
// defines for them; this finds "Admin" when the account is "admin".
static ODRecordRef copyRecordIgnoringCase(CFTypeRef recordType, CFStringRef name)
{
	CFArrayRef all = copyAllRecords(recordType);
	ODRecordRef found = NULL;

	for (CFIndex i = 0; i < CFArrayGetCount(all) && found == NULL; i++)
	{
		ODRecordRef record = (ODRecordRef) CFArrayGetValueAtIndex(all, i);
		if (record->recordName != NULL
			&& CFStringCompare(record->recordName, name, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			found = (ODRecordRef) CFRetain(record);
	}

	CFRelease(all);
	return found;
}

static ODRecordRef copyRecordNamed(CFTypeRef recordType, CFStringRef name)
{
	char* cname = copyCString(name);
	ODRecordRef record = NULL;
	size_t size = 4096;

	if (cname == NULL)
		return NULL;

	for (;;)
	{
		char* buffer = malloc(size);
		int err;

		if (buffer == NULL)
			break;

		if (isRecordType(recordType, kODRecordTypeUsers))
		{
			struct passwd pw, *result = NULL;
			err = getpwnam_r(cname, &pw, buffer, size, &result);
			if (err == 0 && result != NULL)
				record = createUserRecord(result);
		}
		else
		{
			struct group gr, *result = NULL;
			err = getgrnam_r(cname, &gr, buffer, size, &result);
			if (err == 0 && result != NULL)
				record = createGroupRecord(result);
		}

		free(buffer);
		if (err != ERANGE || size >= 1024 * 1024)
			break;
		size *= 2;
	}

	free(cname);

	if (record == NULL)
		record = copyRecordIgnoringCase(recordType, name);
	return record;
}

#pragma mark - Session

ODSessionRef ODSessionCreate(CFAllocatorRef allocator, CFDictionaryRef options, CFErrorRef* error)
{
	(void) options;

	ODSessionRef session = createInstance(allocator, &sessionTypeID, sizeof(struct __ODSession));
	if (session == NULL)
		setError(error, kODErrorRecordParameterError);
	else
		clearError(error);
	return session;
}

CFArrayRef ODSessionCopyNodeNames(CFAllocatorRef allocator, ODSessionRef session, CFErrorRef* error)
{
	(void) session;
	const void* names[] = { LOCAL_NODE_NAME, SEARCH_NODE_NAME };

	clearError(error);
	return CFArrayCreate(allocator, names, 2, &kCFTypeArrayCallBacks);
}

#pragma mark - Node

static ODNodeRef createNode(CFAllocatorRef allocator, CFStringRef name)
{
	struct __ODNode* node = createInstance(allocator, &nodeTypeID, sizeof(struct __ODNode));
	if (node != NULL)
		node->name = CFRetain(name);
	return node;
}

ODNodeRef ODNodeCreateWithName(CFAllocatorRef allocator, ODSessionRef session, CFStringRef nodeName, CFErrorRef* error)
{
	(void) session;

	if (isA(nodeName, CFStringGetTypeID()))
	{
		if (CFEqual(nodeName, LOCAL_NODE_NAME))
		{
			clearError(error);
			return createNode(allocator, LOCAL_NODE_NAME);
		}
		if (CFEqual(nodeName, SEARCH_NODE_NAME))
		{
			clearError(error);
			return createNode(allocator, SEARCH_NODE_NAME);
		}
	}

	setError(error, kODErrorNodeUnknownName);
	return NULL;
}

ODNodeRef ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session, ODNodeType nodeType, CFErrorRef* error)
{
	(void) session;

	switch (nodeType)
	{
		case kODNodeTypeLocalNodes:
			clearError(error);
			return createNode(allocator, LOCAL_NODE_NAME);
		case kODNodeTypeAuthentication:
		case kODNodeTypeNetwork:
			clearError(error);
			return createNode(allocator, SEARCH_NODE_NAME);
		default:
			setError(error, kODErrorNodeUnknownType);
			return NULL;
	}
}

CFStringRef ODNodeGetName(ODNodeRef node)
{
	return isA(node, ODNodeGetTypeID()) ? node->name : NULL;
}

static bool isSearchNode(ODNodeRef node)
{
	return CFEqual(node->name, SEARCH_NODE_NAME);
}

CFArrayRef ODNodeCopySubnodeNames(ODNodeRef node, CFErrorRef* error)
{
	if (!isA(node, ODNodeGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	// The search node searches the local node; the local node has none.
	if (isSearchNode(node))
	{
		const void* names[] = { LOCAL_NODE_NAME };
		clearError(error);
		return CFArrayCreate(kCFAllocatorDefault, names, 1, &kCFTypeArrayCallBacks);
	}

	setError(error, kODErrorPluginOperationNotSupported);
	return NULL;
}

CFArrayRef ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef* error)
{
	if (!isA(node, ODNodeGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	// NULL without an error means that every subnode is reachable.
	if (isSearchNode(node))
	{
		clearError(error);
		return NULL;
	}

	setError(error, kODErrorPluginOperationNotSupported);
	return NULL;
}

CFArrayRef ODNodeCopySupportedRecordTypes(ODNodeRef node, CFErrorRef* error)
{
	const void* types[] = { kODRecordTypeUsers, kODRecordTypeGroups };

	if (!isA(node, ODNodeGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	clearError(error);
	return CFArrayCreate(kCFAllocatorDefault, types, 2, &kCFTypeArrayCallBacks);
}

ODRecordRef ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFTypeRef attributes, CFErrorRef* error)
{
	// Records always carry every attribute this node has, so the requested
	// set is only a hint.
	(void) attributes;

	if (!isA(node, ODNodeGetTypeID()) || !isA(recordName, CFStringGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}
	if (!isSupportedRecordType(recordType))
	{
		setError(error, kODErrorRecordAttributeUnknownType);
		return NULL;
	}

	// A record that does not exist is not an error.
	clearError(error);
	return copyRecordNamed(recordType, recordName);
}

#pragma mark - Record

CFStringRef ODRecordGetRecordType(ODRecordRef record)
{
	return isA(record, ODRecordGetTypeID()) ? record->recordType : NULL;
}

CFStringRef ODRecordGetRecordName(ODRecordRef record)
{
	return isA(record, ODRecordGetTypeID()) ? record->recordName : NULL;
}

CFArrayRef ODRecordCopyValues(ODRecordRef record, ODAttributeType attribute, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isA(attribute, CFStringGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	// An attribute the record does not have yields NULL without an error.
	clearError(error);
	pthread_mutex_lock(&recordLock);
	CFArrayRef values = CFDictionaryGetValue(record->attributes, attribute);
	if (values != NULL)
		CFRetain(values);
	pthread_mutex_unlock(&recordLock);
	return values;
}

CFDictionaryRef ODRecordCopyDetails(ODRecordRef record, CFArrayRef attributes, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	clearError(error);
	pthread_mutex_lock(&recordLock);
	if (attributes == NULL)
	{
		CFDictionaryRef all = CFDictionaryCreateCopy(kCFAllocatorDefault, record->attributes);
		pthread_mutex_unlock(&recordLock);
		return all;
	}

	CFMutableDictionaryRef details = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
			&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	for (CFIndex i = 0; i < CFArrayGetCount(attributes); i++)
	{
		CFTypeRef attribute = CFArrayGetValueAtIndex(attributes, i);
		CFTypeRef values = CFDictionaryGetValue(record->attributes, attribute);
		if (values != NULL)
			CFDictionarySetValue(details, attribute, values);
	}
	pthread_mutex_unlock(&recordLock);
	return details;
}

bool ODRecordAuthenticationAllowed(ODRecordRef record, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isRecordType(record->recordType, kODRecordTypeUsers))
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}

	// No account policies are kept, so any user may try to authenticate.
	clearError(error);
	return true;
}

// crypt() returns a static buffer
static pthread_mutex_t cryptLock = PTHREAD_MUTEX_INITIALIZER;

static bool constantTimeEquals(const char* a, const char* b)
{
	size_t la = strlen(a), lb = strlen(b);
	unsigned char diff = (la != lb);

	for (size_t i = 0; i < la && i < lb; i++)
		diff |= (unsigned char) (a[i] ^ b[i]);
	return diff == 0;
}

// Checks the password against the hash Libinfo returns for the account. An
// account without a usable hash (empty, "*" or disabled with "!") never
// verifies, and Libinfo hands out hashes only to callers allowed to see them.
static bool verifyAgainstAccountEx(const char* name, const char* password, bool* hasHash)
{
	size_t size = 4096;
	bool matches = false;

	if (hasHash != NULL)
		*hasHash = false;

	for (;;)
	{
		char* buffer = malloc(size);
		struct passwd pw, *result = NULL;
		int err;

		if (buffer == NULL)
			return false;

		err = getpwnam_r(name, &pw, buffer, size, &result);
		if (err == 0 && result != NULL)
		{
			const char* hash = result->pw_passwd;
			if (hash != NULL && strlen(hash) > 2 && hash[0] != '*' && hash[0] != '!')
			{
				if (hasHash != NULL)
					*hasHash = true;
				pthread_mutex_lock(&cryptLock);
				const char* computed = crypt(password, hash);
				matches = computed != NULL && constantTimeEquals(computed, hash);
				pthread_mutex_unlock(&cryptLock);
			}
		}

		memset(buffer, 0, size);
		free(buffer);
		if (err != ERANGE || size >= 1024 * 1024)
			break;
		size *= 2;
	}

	return matches;
}

static bool verifyAgainstAccount(const char* name, const char* password)
{
	return verifyAgainstAccountEx(name, password, NULL);
}

bool ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isRecordType(record->recordType, kODRecordTypeUsers)
		|| !isA(password, CFStringGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}

	char* name = copyCString(record->recordName);
	char* given = copyCString(password);
	bool matches = name != NULL && given != NULL && verifyAgainstAccount(name, given);

	if (given != NULL)
	{
		memset(given, 0, strlen(given));
		free(given);
	}
	free(name);

	if (matches)
		clearError(error);
	else
		setError(error, kODErrorCredentialsInvalid);
	return matches;
}

#pragma mark - Changing records

// What the local node stores for a user: the shell and the parts of the
// gecos field. Everything else of an account (ids, home directory, password,
// groups) is not changed through the directory API here.
static bool isGecosAttribute(CFStringRef attribute)
{
	CFStringRef const* gecos = gecosAttributes();

	for (int i = 0; i < 4; i++)
	{
		if (CFEqual(attribute, gecos[i]))
			return true;
	}
	return false;
}

static bool isStoredUserAttribute(CFStringRef attribute)
{
	return isGecosAttribute(attribute) || CFEqual(attribute, kODAttributeTypeUserShell);
}

// Account changes are made by the account's owner or by root; the files they
// end up in are the ones the user database reads.
static bool callerMayChange(ODRecordRef record)
{
	return geteuid() == 0 || record->uid == (long long) geteuid();
}

static bool valueIsStorable(CFStringRef value, bool forGecos)
{
	char* text = copyCString(value);
	bool ok = text != NULL && strpbrk(text, ":\n\r") == NULL && !(forGecos && strchr(text, ',') != NULL);
	free(text);
	return ok;
}

// Replaces the values of an attribute in the record's own copy and remembers
// that it has to be written back. values may be empty, which removes the
// attribute.
static bool changeValues(ODRecordRef record, CFStringRef attribute, CFArrayRef values, CFErrorRef* error)
{
	if (!isRecordType(record->recordType, kODRecordTypeUsers))
	{
		setError(error, kODErrorRecordReadOnlyNode);
		return false;
	}
	if (!isStoredUserAttribute(attribute) || !callerMayChange(record))
	{
		setError(error, kODErrorRecordPermissionError);
		return false;
	}
	if (CFArrayGetCount(values) > 1)
	{
		setError(error, kODErrorRecordAttributeValueSchemaError);
		return false;
	}
	for (CFIndex i = 0; i < CFArrayGetCount(values); i++)
	{
		CFTypeRef value = CFArrayGetValueAtIndex(values, i);
		if (!isA(value, CFStringGetTypeID()))
		{
			setError(error, kODErrorRecordParameterError);
			return false;
		}
		if (!valueIsStorable(value, isGecosAttribute(attribute)))
		{
			setError(error, kODErrorRecordAttributeValueSchemaError);
			return false;
		}
	}

	pthread_mutex_lock(&recordLock);
	CFMutableDictionaryRef attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, record->attributes);
	if (CFArrayGetCount(values) == 0)
		CFDictionaryRemoveValue(attributes, attribute);
	else
		CFDictionarySetValue(attributes, attribute, values);
	CFRelease(record->attributes);
	record->attributes = attributes;
	if (record->changed == NULL)
		record->changed = CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks);
	CFSetAddValue(record->changed, attribute);
	pthread_mutex_unlock(&recordLock);

	clearError(error);
	return true;
}

bool ODRecordSetValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef valueOrValues, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isA(attribute, CFStringGetTypeID()) || valueOrValues == NULL)
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}

	CFArrayRef values;
	if (isA(valueOrValues, CFArrayGetTypeID()))
		values = CFRetain(valueOrValues);
	else
		values = CFArrayCreate(kCFAllocatorDefault, &valueOrValues, 1, &kCFTypeArrayCallBacks);

	bool ok = changeValues(record, attribute, values, error);
	CFRelease(values);
	return ok;
}

bool ODRecordAddValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isA(attribute, CFStringGetTypeID()) || !isA(value, CFStringGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}

	// The attributes that can be stored hold one value each.
	CFArrayRef current = ODRecordCopyValues(record, attribute, NULL);
	bool occupied = current != NULL && CFArrayGetCount(current) > 0;
	if (current != NULL)
		CFRelease(current);
	if (occupied)
	{
		setError(error, kODErrorRecordAttributeValueSchemaError);
		return false;
	}
	return ODRecordSetValue(record, attribute, value, error);
}

bool ODRecordRemoveValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isA(attribute, CFStringGetTypeID()) || !isA(value, CFStringGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}

	CFArrayRef current = ODRecordCopyValues(record, attribute, NULL);
	bool present = current != NULL && CFArrayContainsValue(current, CFRangeMake(0, CFArrayGetCount(current)), value);
	if (current != NULL)
		CFRelease(current);
	if (!present)
	{
		setError(error, kODErrorRecordAttributeValueNotFound);
		return false;
	}
	CFArrayRef none = CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
	bool ok = changeValues(record, attribute, none, error);
	CFRelease(none);
	return ok;
}

// First value of the attribute as a C string ("" when it has none)
static char* copyFirstValue(CFDictionaryRef attributes, CFStringRef attribute)
{
	CFArrayRef values = CFDictionaryGetValue(attributes, attribute);
	char* text = values != NULL && CFArrayGetCount(values) > 0 ? copyCString(CFArrayGetValueAtIndex(values, 0)) : NULL;
	return text != NULL ? text : strdup("");
}

// The gecos field made of the record's parts; trailing empty parts are left
// out as finger(1) writes them.
static char* copyGecos(CFDictionaryRef attributes)
{
	CFStringRef const* names = gecosAttributes();
	char* parts[4];
	size_t total = 4;
	int last = -1;

	for (int i = 0; i < 4; i++)
	{
		parts[i] = copyFirstValue(attributes, names[i]);
		total += strlen(parts[i]);
		if (parts[i][0] != '\0')
			last = i;
	}

	char* gecos = calloc(1, total);
	for (int i = 0; gecos != NULL && i <= last; i++)
	{
		if (i > 0)
			strcat(gecos, ",");
		strcat(gecos, parts[i]);
	}
	for (int i = 0; i < 4; i++)
		free(parts[i]);
	return gecos;
}

static CFIndex errorForErrno(int err)
{
	switch (err)
	{
		case EACCES:
		case EPERM:
			return kODErrorRecordPermissionError;
		case EROFS:
			return kODErrorRecordReadOnlyNode;
		case EINVAL:
			return kODErrorRecordAttributeValueSchemaError;
		default:
			return kODErrorPluginError;
	}
}

bool ODRecordSynchronize(ODRecordRef record, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return false;
	}
	if (!isRecordType(record->recordType, kODRecordTypeUsers))
	{
		setError(error, kODErrorRecordReadOnlyNode);
		return false;
	}

	pthread_mutex_lock(&recordLock);
	CFDictionaryRef attributes = CFDictionaryCreateCopy(kCFAllocatorDefault, record->attributes);
	CFSetRef changed = record->changed != NULL ? CFSetCreateCopy(kCFAllocatorDefault, record->changed) : NULL;
	pthread_mutex_unlock(&recordLock);

	bool ok = false;
	if (changed == NULL || CFSetGetCount(changed) == 0)
	{
		// Nothing to write
		clearError(error);
		ok = true;
	}
	else if (!callerMayChange(record))
	{
		setError(error, kODErrorRecordPermissionError);
	}
	else
	{
		struct odpw_changes changes = { NULL, NULL };
		char* shell = NULL;
		char* gecos = NULL;
		char* name = copyCString(record->recordName);

		bool gecosChanged = false;

		if (CFSetContainsValue(changed, kODAttributeTypeUserShell))
			changes.shell = shell = copyFirstValue(attributes, kODAttributeTypeUserShell);
		for (int i = 0; i < 4; i++)
			gecosChanged = gecosChanged || CFSetContainsValue(changed, gecosAttributes()[i]);
		if (gecosChanged)
			changes.gecos = gecos = copyGecos(attributes);

		if (name == NULL || (gecosChanged && gecos == NULL))
		{
			setError(error, kODErrorPluginError);
		}
		else
		{
			// master.passwd first: it is what getpwnam() returns to root and
			// what passwd is derived from. A prefix may lack either file.
			static const struct { const char* path; enum odpw_format format; } files[] = {
				{ "/etc/master.passwd", ODPW_MASTER },
				{ "/etc/passwd", ODPW_PASSWD },
			};
			bool any = false;
			int err = 0;

			for (int i = 0; i < 2 && err == 0; i++)
			{
				bool found = false;
				err = odpw_update_user(files[i].path, files[i].format, name, &changes, &found);
				if (err == ENOENT)
					err = 0;
				any = any || found;
			}

			if (err != 0)
				setError(error, errorForErrno(err));
			else if (!any)
				setError(error, kODErrorRecordParameterError);
			else
			{
				pthread_mutex_lock(&recordLock);
				CFSetRemoveAllValues(record->changed);
				pthread_mutex_unlock(&recordLock);
				clearError(error);
				ok = true;
			}
		}

		free(shell);
		free(gecos);
		free(name);
	}

	CFRelease(attributes);
	if (changed != NULL)
		CFRelease(changed);
	return ok;
}

// The local node takes the account's own credentials. An account without a
// usable password (as the accounts of a Darling prefix are) is accepted for the
// user it belongs to and for root, since a process that runs as that user can
// already change the account's files.
bool ODRecordSetNodeCredentials(ODRecordRef record, CFStringRef username, CFStringRef password, CFErrorRef* error)
{
	if (!isA(record, ODRecordGetTypeID()) || !isA(username, CFStringGetTypeID()) || !isA(password, CFStringGetTypeID()))
	{
		setError(error, kODErrorCredentialsParameterError);
		return false;
	}

	char* name = copyCString(username);
	char* given = copyCString(password);
	bool hasHash = false;
	bool ok = false;

	if (name != NULL && given != NULL)
	{
		struct passwd pw, *result = NULL;
		char buffer[4096];

		ok = verifyAgainstAccountEx(name, given, &hasHash);
		if (!ok && !hasHash && getpwnam_r(name, &pw, buffer, sizeof(buffer), &result) == 0 && result != NULL)
			ok = geteuid() == 0 || result->pw_uid == geteuid();
	}

	if (given != NULL)
	{
		memset(given, 0, strlen(given));
		free(given);
	}
	free(name);

	if (ok)
		clearError(error);
	else
		setError(error, kODErrorCredentialsInvalid);
	return ok;
}

#pragma mark - Query

// Wraps a single value in an array; arrays are copied
static CFArrayRef copyAsArray(CFTypeRef valueOrList)
{
	if (valueOrList == NULL)
		return CFArrayCreate(kCFAllocatorDefault, NULL, 0, &kCFTypeArrayCallBacks);
	if (CFGetTypeID(valueOrList) == CFArrayGetTypeID())
		return CFArrayCreateCopy(kCFAllocatorDefault, valueOrList);
	return CFArrayCreate(kCFAllocatorDefault, &valueOrList, 1, &kCFTypeArrayCallBacks);
}

// Query values may be strings or UTF-8 data
static CFStringRef copyQueryValue(CFTypeRef value)
{
	if (isA(value, CFStringGetTypeID()))
		return CFRetain(value);
	if (isA(value, CFDataGetTypeID()))
		return CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, value, kCFStringEncodingUTF8);
	return NULL;
}

ODQueryRef ODQueryCreateWithNode(CFAllocatorRef allocator, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute,
	ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef* error)
{
	// Records always carry every attribute, so the requested set is a hint.
	(void) returnAttributeOrList;

	if (!isA(node, ODNodeGetTypeID()) || recordTypeOrList == NULL)
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	struct __ODQuery* query = createInstance(allocator, &queryTypeID, sizeof(struct __ODQuery));
	if (query == NULL)
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	CFArrayRef values = copyAsArray(queryValueOrList);
	CFMutableArrayRef strings = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (CFIndex i = 0; i < CFArrayGetCount(values); i++)
	{
		CFStringRef value = copyQueryValue(CFArrayGetValueAtIndex(values, i));
		if (value != NULL)
		{
			CFArrayAppendValue(strings, value);
			CFRelease(value);
		}
	}
	CFRelease(values);

	query->node = (ODNodeRef) CFRetain(node);
	query->recordTypes = copyAsArray(recordTypeOrList);
	query->attribute = isA(attribute, CFStringGetTypeID()) ? CFStringCreateCopy(kCFAllocatorDefault, attribute) : NULL;
	query->matchType = matchType;
	query->values = strings;
	query->maxResults = maxResults;

	clearError(error);
	return query;
}

ODQueryRef ODQueryCreateWithNodeType(CFAllocatorRef allocator, ODNodeType nodeType, CFTypeRef recordTypeOrList, ODAttributeType attribute,
	ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef* error)
{
	ODNodeRef node = ODNodeCreateWithNodeType(allocator, NULL, nodeType, error);
	if (node == NULL)
		return NULL;

	ODQueryRef query = ODQueryCreateWithNode(allocator, node, recordTypeOrList, attribute, matchType,
			queryValueOrList, returnAttributeOrList, maxResults, error);
	CFRelease(node);
	return query;
}

static bool valueMatches(CFStringRef value, CFStringRef wanted, ODMatchType matchType, bool caseInsensitive)
{
	CFStringCompareFlags flags = caseInsensitive ? kCFCompareCaseInsensitive : 0;
	CFRange all = CFRangeMake(0, CFStringGetLength(value));

	switch (matchType & ~MATCH_INSENSITIVE_BIT)
	{
		case kODMatchEqualTo:
			return CFStringCompare(value, wanted, flags) == kCFCompareEqualTo;
		case kODMatchBeginsWith:
			return CFStringFindWithOptions(value, wanted, all, flags | kCFCompareAnchored, NULL);
		case kODMatchEndsWith:
			return CFStringFindWithOptions(value, wanted, all, flags | kCFCompareAnchored | kCFCompareBackwards, NULL);
		case kODMatchContains:
			return CFStringFindWithOptions(value, wanted, all, flags, NULL);
		default:
			return false;
	}
}

static bool recordMatches(ODQueryRef query, ODRecordRef record)
{
	if (query->matchType == kODMatchAny)
		return true;
	if (query->attribute == NULL)
		return false;

	pthread_mutex_lock(&recordLock);
	CFArrayRef values = CFDictionaryGetValue(record->attributes, query->attribute);
	if (values != NULL)
		CFRetain(values);
	pthread_mutex_unlock(&recordLock);
	if (values == NULL)
		return false;

	// Record names compare without regard to case, as their schema says.
	bool caseInsensitive = (query->matchType & MATCH_INSENSITIVE_BIT) != 0
		|| CFEqual(query->attribute, kODAttributeTypeRecordName);

	bool matched = false;
	for (CFIndex i = 0; i < CFArrayGetCount(values) && !matched; i++)
	{
		for (CFIndex j = 0; j < CFArrayGetCount(query->values) && !matched; j++)
		{
			matched = valueMatches(CFArrayGetValueAtIndex(values, i), CFArrayGetValueAtIndex(query->values, j),
					query->matchType, caseInsensitive);
		}
	}
	CFRelease(values);
	return matched;
}

CFArrayRef ODQueryCopyResults(ODQueryRef query, bool allowPartialResults, CFErrorRef* error)
{
	// Results are computed synchronously, so they are always complete.
	(void) allowPartialResults;

	if (!isA(query, ODQueryGetTypeID()))
	{
		setError(error, kODErrorRecordParameterError);
		return NULL;
	}

	bool anySupported = false;
	CFMutableArrayRef results = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	for (CFIndex t = 0; t < CFArrayGetCount(query->recordTypes); t++)
	{
		CFTypeRef recordType = CFArrayGetValueAtIndex(query->recordTypes, t);
		if (!isSupportedRecordType(recordType))
			continue;
		anySupported = true;

		CFArrayRef candidates;
		if ((query->matchType & ~MATCH_INSENSITIVE_BIT) == kODMatchEqualTo && query->attribute != NULL
			&& CFEqual(query->attribute, kODAttributeTypeRecordName))
		{
			// Direct lookups instead of enumerating every account
			CFMutableArrayRef named = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			for (CFIndex i = 0; i < CFArrayGetCount(query->values); i++)
				appendRecord(named, copyRecordNamed(recordType, CFArrayGetValueAtIndex(query->values, i)));
			candidates = named;
		}
		else
			candidates = copyAllRecords(recordType);

		for (CFIndex i = 0; i < CFArrayGetCount(candidates); i++)
		{
			if (query->maxResults > 0 && CFArrayGetCount(results) >= query->maxResults)
				break;

			ODRecordRef record = (ODRecordRef) CFArrayGetValueAtIndex(candidates, i);
			if (recordMatches(query, record))
				CFArrayAppendValue(results, record);
		}
		CFRelease(candidates);
	}

	if (!anySupported)
	{
		CFRelease(results);
		setError(error, kODErrorRecordAttributeUnknownType);
		return NULL;
	}

	clearError(error);
	return results;
}
