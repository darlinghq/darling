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

static CFArrayRef arrayForStringColumn0(FMResultSet* rs)
{
	if ([rs next])
	{
		NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: 0];
		do
		{
			[array addObject: [rs stringForColumnIndex:0]];
		}
		while ([rs next]);
		
		CFArrayRef retval = (CFArrayRef) [[NSArray alloc] initWithArray:array];
		[array release];

		return retval;
	}
	else
		return NULL;
}

_Nullable CFArrayRef
UTTypeCreateAllIdentifiersForTag(
  CFStringRef inTagClass,
  CFStringRef inTag,
  _Nullable CFStringRef inConformingToUTI)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFArrayRef retval;
	[dq inDatabase:^(FMDatabase* db) {
		NSString* query = @"select type_identifier from uti inner join uti_tag UT on UT.uti=uti.id "
			@"where UT.tag = ? and UT.value = ?";

		if (inConformingToUTI != NULL)
		{
			query = [query stringByAppendingFormat:@" and uti.id in (select uti from uti_conforms where conforms_to='%@')",
				escape((NSString*) inConformingToUTI)];
		}

		FMResultSet* rs = [db executeQuery:query, inTagClass, inTag];

		retval = arrayForStringColumn0(rs);

		[rs close];
	}];

	return retval;
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

_Nullable CFArrayRef
UTTypeCopyAllTagsWithClass(
  CFStringRef inUTI,
  CFStringRef inTagClass)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFArrayRef retval;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select UT.value from uti inner join uti_tag UT on UT.uti=uti.id where UT.tag = ? and type_identifier = ?",
			inTagClass, inUTI];

		retval = arrayForStringColumn0(rs);

		[rs close];
	}];

	return retval;
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
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return FALSE;

	__block Boolean retval;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select UC.conforms_to from uti inner join uti_conforms UC on UC.uti=uti.id where type_identifier=? and UC.conforms_to=?",
			inUTI, inConformsToUTI];

		retval = [rs next];
		[rs close];
	}];

	return retval;
}

_Nullable CFStringRef
UTTypeCopyDescription(CFStringRef inUTI)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFStringRef retval;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select description from uti where type_identifier = ?", inUTI];
		if ([rs next])
			retval = (CFStringRef) [[rs stringForColumnIndex:0] retain];
		else
			retval = NULL;
		[rs close];
	}];

	return retval;
}

Boolean
UTTypeIsDeclared(CFStringRef inUTI)
{
	CFStringRef str = UTTypeCopyDescription(inUTI);
	if (str != NULL)
	{
		CFRelease(str);
		return TRUE;
	}
	else
		return FALSE;
}

Boolean
UTTypeIsDynamic(CFStringRef inUTI)
{
	return FALSE;
}

_Nullable CFDictionaryRef
UTTypeCopyDeclaration(CFStringRef inUTI)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFDictionaryRef retval;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select id, description from uti where type_identifier = ?", inUTI];
		if ([rs next])
		{
			NSMutableDictionary* dict = [[NSMutableDictionary alloc] initWithCapacity: 5];
			NSNumber* utiId = [NSNumber numberWithInt: [rs intForColumn:@"id"]];

			dict[(NSString*) kUTTypeIdentifierKey] = (NSString*) inUTI;
			dict[(NSString*) kUTTypeDescriptionKey] = [rs stringForColumn:@"description"];
			[rs close];

			NSMutableArray* conformsTo = [[NSMutableArray alloc] initWithCapacity:0];
			rs = [db executeQuery:@"select conforms_to from uti_conforms where uti = ?", utiId];

			while ([rs next])
				[conformsTo addObject: [rs stringForColumnIndex:0]];
			
			[rs close];
			dict[(NSString*) kUTTypeConformsToKey] = conformsTo;
			[conformsTo release];

			NSMutableDictionary* tags = [[NSMutableDictionary alloc] initWithCapacity:0];
			rs = [db executeQuery:@"select tag, value from uti_tag where uti = ?", utiId];

			while ([rs next])
				tags[[rs stringForColumn:@"tag"]] = [rs stringForColumn:@"value"];
			
			[rs close];
			dict[(NSString*) kUTTypeTagSpecificationKey] = tags;
			[tags release];

			NSMutableArray* icons = [[NSMutableArray alloc] initWithCapacity:0];
			rs = [db executeQuery:@"select file from uti_icon where uti = ?", utiId];

			while ([rs next])
				[icons addObject: [rs stringForColumnIndex:0]];

			[rs close];
			dict[@"UTTypeIconFiles"] = icons;
			[icons release];

			retval = (CFDictionaryRef) [[NSDictionary alloc] initWithDictionary:dict copyItems:YES];
			[dict release];
		}
		else
		{
			[rs close];
			retval = NULL;
		}
	}];

	return retval;
}

_Nullable CFURLRef
UTTypeCopyDeclaringBundleURL(CFStringRef inUTI)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFURLRef retval;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select B.path from uti inner join bundle B on B.id=uti.bundle where type_identifier = ?", inUTI];
		if ([rs next])
			retval = CFURLCreateWithFileSystemPath(NULL, (CFStringRef) [rs stringForColumnIndex:0], kCFURLPOSIXPathStyle, TRUE);
		else
			retval = NULL;
		[rs close];
	}];

	return retval;
}

CFArrayRef
UTTypeCopyParentIdentifiers(CFStringRef inUTI)
{
	FMDatabaseQueue* dq = getDatabaseQueue();
	if (!dq)
		return NULL;

	__block CFArrayRef retval = NULL;
	[dq inDatabase:^(FMDatabase* db) {
		FMResultSet* rs = [db executeQuery:@"select id, description from uti where type_identifier = ?", inUTI];
		if ([rs next])
		{
			NSNumber* utiId = [NSNumber numberWithInt: [rs intForColumn:@"id"]];
			[rs close];

			NSMutableArray* conformsTo = [[NSMutableArray alloc] init];
			rs = [db executeQuery:@"select conforms_to from uti_conforms where uti = ?", utiId];

			while ([rs next])
				[conformsTo addObject: [rs stringForColumnIndex:0]];

			[rs close];

			retval = (CFArrayRef)[[NSArray alloc] initWithArray: conformsTo copyItems: YES];
			[conformsTo release];
		}
		else
		{
			[rs close];
		}
	}];

	return retval;
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
	while (shift != 0);
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

	OSType retval = 0;
	const char* str = CFStringGetCStringPtr(inString, kCFStringEncodingASCII);

	for (int i = 0; i < 4; i++)
	{
		if (i < CFStringGetLength(inString))
			retval |= ((UInt32) str[i]) & 0xff;
		retval <<= 8;
	}

	return retval;
}
