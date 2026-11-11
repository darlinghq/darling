/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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


#include <LaunchServices/LaunchServices.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSNumber.h>
#import <fmdb/FMDatabaseQueue.h>

extern FMDatabaseQueue* getDatabaseQueue(void);
extern NSString* escape(NSString* str);

// Bounds the walk over a conformance graph, which declarations could make cyclic or huge.
static const NSUInteger kMaxVisitedTypes = 1024;

// Dynamic identifiers are generated for tags that no declared type carries. They are "dyn."
// followed by the hex UTF-8 of "tagClass<US>tag<US>parent", so a dynamic type can be described
// from its identifier alone and needs no database row. The encoding is Darling's own.
static NSString* const kDynamicPrefix = @"dyn.";
static NSString* const kDynamicSeparator = @"\x1f";

static BOOL isDynamicIdentifier(NSString* uti)
{
	return [uti length] > [kDynamicPrefix length] &&
		[uti compare: kDynamicPrefix options: NSCaseInsensitiveSearch range: NSMakeRange(0, [kDynamicPrefix length])] == NSOrderedSame;
}

static NSString* dynamicIdentifier(NSString* tagClass, NSString* tag, NSString* parent)
{
	NSString* payload = [@[tagClass, tag, parent ?: @""] componentsJoinedByString: kDynamicSeparator];
	NSData* data = [payload dataUsingEncoding: NSUTF8StringEncoding];
	const unsigned char* bytes = [data bytes];
	NSMutableString* hex = [NSMutableString stringWithString: kDynamicPrefix];

	for (NSUInteger i = 0; i < [data length]; i++)
		[hex appendFormat: @"%02x", bytes[i]];
	return hex;
}

// Returns the tag class, tag and parent of a dynamic identifier, or NO if it is not one of ours.
static BOOL parseDynamicIdentifier(NSString* uti, NSString** tagClass, NSString** tag, NSString** parent)
{
	if (!isDynamicIdentifier(uti))
		return NO;

	NSString* hex = [uti substringFromIndex: [kDynamicPrefix length]];
	if ([hex length] % 2 != 0)
		return NO;

	NSMutableData* data = [NSMutableData dataWithLength: [hex length] / 2];
	unsigned char* bytes = [data mutableBytes];
	for (NSUInteger i = 0; i < [hex length]; i++)
	{
		unichar c = [hex characterAtIndex: i];
		int nibble = (c >= '0' && c <= '9') ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
		if (nibble < 0)
			return NO;
		bytes[i / 2] = (unsigned char) ((bytes[i / 2] << 4) | nibble);
	}

	NSString* payload = [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease];
	NSArray* parts = [payload componentsSeparatedByString: kDynamicSeparator];
	if ([parts count] != 3)
		return NO;

	*tagClass = parts[0];
	*tag = parts[1];
	*parent = [parts[2] length] ? parts[2] : nil;
	return YES;
}

static BOOL declaredInDatabase(FMDatabase* db, NSString* uti)
{
	FMResultSet* rs = [db executeQuery: @"select 1 from uti where type_identifier = ?", uti];
	BOOL found = [rs next];
	[rs close];
	return found;
}

// The types uti directly conforms to. A type nobody declared conforms to nothing.
static NSArray<NSString*>* directParents(FMDatabase* db, NSString* uti)
{
	NSString *tagClass, *tag, *parent;
	if (parseDynamicIdentifier(uti, &tagClass, &tag, &parent))
		return parent ? @[parent] : @[];

	NSMutableArray* parents = [NSMutableArray array];
	FMResultSet* rs = [db executeQuery: @"select UC.conforms_to from uti inner join uti_conforms UC on UC.uti=uti.id "
		@"where uti.type_identifier = ? order by UC.id", uti];
	while ([rs next])
		[parents addObject: [rs stringForColumnIndex: 0]];
	[rs close];
	return parents;
}

// A type conforms to itself and, transitively, to everything its parents conform to.
static BOOL conformsLocked(FMDatabase* db, NSString* uti, NSString* target)
{
	NSMutableArray<NSString*>* pending = [NSMutableArray arrayWithObject: uti];
	NSMutableSet<NSString*>* seen = [NSMutableSet set];

	while ([pending count] > 0 && [seen count] < kMaxVisitedTypes)
	{
		NSString* current = [pending lastObject];
		[pending removeLastObject];

		if ([current caseInsensitiveCompare: target] == NSOrderedSame)
			return YES;
		if ([seen member: [current lowercaseString]] != nil)
			continue;
		[seen addObject: [current lowercaseString]];

		[pending addObjectsFromArray: directParents(db, current)];
	}
	return NO;
}

static NSArray<NSString*>* tagValuesLocked(FMDatabase* db, NSString* uti, NSString* tagClass)
{
	NSString *dynClass, *dynTag, *dynParent;
	if (parseDynamicIdentifier(uti, &dynClass, &dynTag, &dynParent))
		return [dynClass isEqualToString: tagClass] ? @[dynTag] : @[];

	NSMutableArray* values = [NSMutableArray array];
	FMResultSet* rs = [db executeQuery: @"select UT.value from uti inner join uti_tag UT on UT.uti=uti.id "
		@"where UT.tag = ? and uti.type_identifier = ? order by UT.id", tagClass, uti];
	while ([rs next])
		[values addObject: [rs stringForColumnIndex: 0]];
	[rs close];
	return values;
}

// Declared types carrying the tag, in registration order, optionally restricted to a supertype.
// When there is none, the result is a single dynamic identifier that conforms to the supertype.
static NSArray<NSString*>* identifiersForTagLocked(FMDatabase* db, NSString* tagClass, NSString* tag, NSString* conformingTo)
{
	NSMutableArray* result = [NSMutableArray array];
	FMResultSet* rs = [db executeQuery: @"select uti.type_identifier from uti inner join uti_tag UT on UT.uti=uti.id "
		@"where UT.tag = ? and UT.value = ? order by uti.id", tagClass, tag];

	while ([rs next])
	{
		NSString* identifier = [rs stringForColumnIndex: 0];
		if ([result containsObject: identifier])
			continue;
		[result addObject: identifier];
	}
	[rs close];

	if (conformingTo != nil)
	{
		NSMutableArray* filtered = [NSMutableArray array];
		for (NSString* identifier in result)
		{
			if (conformsLocked(db, identifier, conformingTo))
				[filtered addObject: identifier];
		}
		result = filtered;
	}

	if ([result count] == 0)
		[result addObject: dynamicIdentifier(tagClass, tag, conformingTo)];

	return result;
}

_Nullable CFArrayRef
UTTypeCreateAllIdentifiersForTag(
  CFStringRef inTagClass,
  CFStringRef inTag,
  _Nullable CFStringRef inConformingToUTI)
{
	if (inTagClass == NULL || inTag == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block NSArray* result = nil;
	[dq inDatabase:^(FMDatabase* db) {
		result = [identifiersForTagLocked(db, (NSString*) inTagClass, (NSString*) inTag, (NSString*) inConformingToUTI) retain];
	}];

	return (CFArrayRef) result;
}

_Nullable CFStringRef
UTTypeCreatePreferredIdentifierForTag(
  CFStringRef inTagClass,
  CFStringRef inTag,
  _Nullable CFStringRef inConformingToUTI)
{
	CFArrayRef array = UTTypeCreateAllIdentifiersForTag(inTagClass, inTag, inConformingToUTI);
	if (array == NULL)
		return NULL;

	CFStringRef str = (CFStringRef) CFRetain(CFArrayGetValueAtIndex(array, 0));
	CFRelease(array);

	return str;
}

_Nullable CFArrayRef
UTTypeCopyAllTagsWithClass(
  CFStringRef inUTI,
  CFStringRef inTagClass)
{
	if (inUTI == NULL || inTagClass == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block NSArray* result = nil;
	[dq inDatabase:^(FMDatabase* db) {
		NSArray* values = tagValuesLocked(db, (NSString*) inUTI, (NSString*) inTagClass);
		if ([values count] > 0)
			result = [values retain];
	}];

	return (CFArrayRef) result;
}

_Nullable CFStringRef
UTTypeCopyPreferredTagWithClass(
  CFStringRef inUTI,
  CFStringRef inTagClass)
{
	CFArrayRef array = UTTypeCopyAllTagsWithClass(inUTI, inTagClass);
	if (array == NULL)
		return NULL;

	CFStringRef str = (CFStringRef) CFRetain(CFArrayGetValueAtIndex(array, 0));
	CFRelease(array);

	return str;
}

Boolean
UTTypeEqual(
  CFStringRef inUTI1,
  CFStringRef inUTI2)
{
	return CFStringCompare(inUTI1, inUTI2, kCFCompareCaseInsensitive) == kCFCompareEqualTo;
}

Boolean
UTTypeConformsTo(
  CFStringRef inUTI,
  CFStringRef inConformsToUTI)
{
	if (inUTI == NULL || inConformsToUTI == NULL)
		return FALSE;
	if (UTTypeEqual(inUTI, inConformsToUTI))
		return TRUE;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return FALSE;

	__block Boolean retval = FALSE;
	[dq inDatabase:^(FMDatabase* db) {
		retval = conformsLocked(db, (NSString*) inUTI, (NSString*) inConformsToUTI);
	}];

	return retval;
}

_Nullable CFStringRef
UTTypeCopyDescription(CFStringRef inUTI)
{
	if (inUTI == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFStringRef retval = NULL;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select description from uti where type_identifier = ?", inUTI];
		if ([rs next])
			retval = (CFStringRef) [[rs stringForColumnIndex:0] retain];
		[rs close];
	}];

	return retval;
}

Boolean
UTTypeIsDeclared(CFStringRef inUTI)
{
	if (inUTI == NULL)
		return FALSE;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return FALSE;

	__block Boolean retval = FALSE;
	[dq inDatabase:^(FMDatabase* db) {
		retval = declaredInDatabase(db, (NSString*) inUTI);
	}];

	return retval;
}

Boolean
UTTypeIsDynamic(CFStringRef inUTI)
{
	return inUTI != NULL && isDynamicIdentifier((NSString*) inUTI);
}

_Nullable CFDictionaryRef
UTTypeCopyDeclaration(CFStringRef inUTI)
{
	if (inUTI == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFDictionaryRef retval = NULL;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select id, description from uti where type_identifier = ?", inUTI];
		if (![rs next])
		{
			[rs close];
			return;
		}

		NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity: 5];
		NSNumber* utiId = [NSNumber numberWithInt: [rs intForColumn:@"id"]];

		dict[(NSString*) kUTTypeIdentifierKey] = (NSString*) inUTI;
		dict[(NSString*) kUTTypeDescriptionKey] = [rs stringForColumn:@"description"];
		[rs close];

		NSMutableArray* conformsTo = [[NSMutableArray alloc] initWithCapacity:0];
		rs = [db executeQuery:@"select conforms_to from uti_conforms where uti = ? order by id", utiId];

		while ([rs next])
			[conformsTo addObject: [rs stringForColumnIndex:0]];

		[rs close];
		dict[(NSString*) kUTTypeConformsToKey] = conformsTo;
		[conformsTo release];

		// Every tag class maps to the list of its values, in declaration order.
		NSMutableDictionary* tags = [[NSMutableDictionary alloc] initWithCapacity:0];
		rs = [db executeQuery:@"select tag, value from uti_tag where uti = ? order by id", utiId];

		while ([rs next])
		{
			NSString* tagClass = [rs stringForColumn:@"tag"];
			NSMutableArray* values = tags[tagClass];
			if (values == nil)
			{
				values = [NSMutableArray array];
				tags[tagClass] = values;
			}
			[values addObject: [rs stringForColumn:@"value"]];
		}

		[rs close];
		dict[(NSString*) kUTTypeTagSpecificationKey] = tags;
		[tags release];

		NSMutableArray* icons = [[NSMutableArray alloc] initWithCapacity:0];
		rs = [db executeQuery:@"select file from uti_icon where uti = ? order by id", utiId];

		while ([rs next])
			[icons addObject: [rs stringForColumnIndex:0]];

		[rs close];
		dict[@"UTTypeIconFiles"] = icons;
		[icons release];

		retval = (CFDictionaryRef) [[NSDictionary alloc] initWithDictionary:dict copyItems:YES];
		[dict release];
	}];

	return retval;
}

_Nullable CFURLRef
UTTypeCopyDeclaringBundleURL(CFStringRef inUTI)
{
	if (inUTI == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFURLRef retval = NULL;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select B.path from uti inner join bundle B on B.id=uti.bundle where type_identifier = ?", inUTI];
		if ([rs next])
			retval = CFURLCreateWithFileSystemPath(NULL, (CFStringRef) [rs stringForColumnIndex:0], kCFURLPOSIXPathStyle, TRUE);
		[rs close];
	}];

	return retval;
}

CFArrayRef
UTTypeCopyParentIdentifiers(CFStringRef inUTI)
{
	if (inUTI == NULL)
		return NULL;

	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block NSArray* retval = nil;
	[dq inDatabase:^(FMDatabase* db) {
		retval = [[NSArray alloc] initWithArray: directParents(db, (NSString*) inUTI)];
	}];

	return (CFArrayRef) retval;
}

CFStringRef
UTCreateStringForOSType(OSType inOSType)
{
	char buf[5];
	int pos = 0;
	int shift = 24;

	do
	{
		buf[pos++] = (inOSType >> shift) & 0xff;
		shift -= 8;
	}
	while (shift >= 0);
	buf[pos] = '\0';

	return CFStringCreateWithCString(NULL, buf, kCFStringEncodingASCII);
}

OSType
UTGetOSTypeFromString(CFStringRef inString)
{
	if (!inString)
		return 0;
	if (CFStringGetLength(inString) > 4)
		return 0;

	CFIndex length = CFStringGetLength(inString);
	UniChar str[4];
	CFStringGetCharacters(inString, CFRangeMake(0, length), str);
	OSType retval = 0;

	for (int i = 0; i < 4; i++)
	{
		retval <<= 8;
		if (i < length)
			retval |= ((UInt32) str[i]) & 0xff;
	}

	return retval;
}
