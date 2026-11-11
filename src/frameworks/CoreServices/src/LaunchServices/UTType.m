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
// followed by a base-32 rendering of a bit string, so a dynamic type can be described from its
// identifier alone and needs no database row. The bit string is, in order: seven zero bits; when
// the identifier conforms to a type, a marker, that type's identifier and a terminator; a marker
// and a code for the tag class; and the tag. A tag, like the type conformed to, is a header
// followed by 7-bit characters separated by zero bits. The string is padded with zero bits to a
// multiple of five. Extension and MIME type tags are lowercased, as is the type conformed to; OS
// type tags keep their case. This is the form LaunchServices produces on macOS for these tag
// classes, so identifiers created here and there agree.
//
// A few well-known types are written as a short token instead of their identifier; the table
// below lists the ones that were observed. Anything that cannot be written in this form (another
// tag class, a tag with non-ASCII characters) gets a form of Darling's own: "dyn." followed by
// the hex UTF-8 of "tagClass<US>tag<US>parent". The two forms cannot be confused: the first
// character of the base-32 form is always "a", which hex of ASCII text never starts with.
static NSString* const kDynamicPrefix = @"dyn.";
static NSString* const kDynamicSeparator = @"\x1f";
static NSString* const kDynamicAlphabet = @"abcdefghkmnpqrstuvwxyz0123456789";
static NSString* const kDynamicLead = @"0000000";
static NSString* const kDynamicHeader = @"1111010";
static NSString* const kDynamicConformsMarker = @"1111110011000000";
static NSString* const kDynamicConformsEnd = @"11101000";
static NSString* const kDynamicClassMarker = @"1100";

static const struct { const char* identifier; const char* bits; } kDynamicTokens[] = {
	{ "com.apple.device-model-code", "111101001100010011010000" },
	{ "com.apple.nspboard-type", "1111010011010000" },
	{ "com.apple.ostype", "1111010011001000" },
	{ "com.apple.package", "111101001100010011000000" },
	{ "com.apple.traditional-mac-plain-text", "1111010100000100" },
	{ "public.audio", "1111010100010000" },
	{ "public.bluetooth-vendor-product-id", "111101001100010011010100" },
	{ "public.content", "111101001100010011001100" },
	{ "public.data", "1111010011011000" },
	{ "public.directory", "1111010100010100" },
	{ "public.filename-extension", "1111010011000100" },
	{ "public.folder", "1111010100011000" },
	{ "public.image", "1111010100001000" },
	{ "public.mime-type", "1111010011001100" },
	{ "public.plain-text", "1111010011100000" },
	{ "public.text", "1111010011011100" },
	{ "public.url", "111101001100010011000100" },
	{ "public.utf16-external-plain-text", "111101001100010011001000" },
	{ "public.utf16-plain-text", "1111010011100100" },
	{ "public.video", "1111010100001100" }
};

static NSString* dynamicClassCode(NSString* tagClass, BOOL* lowercase)
{
	// Codes and case handling of the tag classes observed on macOS.
	if ([tagClass isEqualToString: (NSString*) kUTTagClassFilenameExtension])
	{
		*lowercase = YES;
		return @"0100";
	}
	if ([tagClass isEqualToString: (NSString*) kUTTagClassMIMEType])
	{
		*lowercase = YES;
		return @"1100";
	}
	if ([tagClass isEqualToString: (NSString*) kUTTagClassOSType])
	{
		*lowercase = NO;
		return @"1000";
	}
	return nil;
}

static NSString* dynamicClassForCode(NSString* code)
{
	if ([code isEqualToString: @"0100"])
		return (NSString*) kUTTagClassFilenameExtension;
	if ([code isEqualToString: @"1100"])
		return (NSString*) kUTTagClassMIMEType;
	if ([code isEqualToString: @"1000"])
		return (NSString*) kUTTagClassOSType;
	return nil;
}

static BOOL isDynamicIdentifier(NSString* uti)
{
	return [uti length] > [kDynamicPrefix length] &&
		[uti compare: kDynamicPrefix options: NSCaseInsensitiveSearch range: NSMakeRange(0, [kDynamicPrefix length])] == NSOrderedSame;
}

// Appends text as 7-bit characters, each followed by a zero bit. Returns NO for anything but ASCII.
static BOOL appendDynamicCharacters(NSMutableString* bits, NSString* text)
{
	for (NSUInteger i = 0; i < [text length]; i++)
	{
		unichar c = [text characterAtIndex: i];
		if (c == 0 || c > 127)
			return NO;
		for (int bit = 6; bit >= 0; bit--)
			[bits appendString: ((c >> bit) & 1) ? @"1" : @"0"];
		[bits appendString: @"0"];
	}
	return YES;
}

static NSString* dynamicBase32(NSString* bits)
{
	NSMutableString* text = [NSMutableString stringWithString: kDynamicPrefix];
	NSMutableString* padded = [NSMutableString stringWithString: bits];

	while ([padded length] % 5 != 0)
		[padded appendString: @"0"];
	for (NSUInteger i = 0; i < [padded length]; i += 5)
	{
		NSUInteger value = (NSUInteger) strtoul([[padded substringWithRange: NSMakeRange(i, 5)] UTF8String], NULL, 2);
		[text appendString: [kDynamicAlphabet substringWithRange: NSMakeRange(value, 1)]];
	}
	return text;
}

// The bits of the base-32 body of a dynamic identifier, or nil when it is not in that form.
static NSString* dynamicBitsFromBase32(NSString* body)
{
	NSMutableString* bits = [NSMutableString string];

	for (NSUInteger i = 0; i < [body length]; i++)
	{
		NSRange r = [kDynamicAlphabet rangeOfString: [body substringWithRange: NSMakeRange(i, 1)]];
		if (r.location == NSNotFound)
			return nil;
		for (int bit = 4; bit >= 0; bit--)
			[bits appendString: ((r.location >> bit) & 1) ? @"1" : @"0"];
	}
	return bits;
}

static NSString* dynamicIdentifierHex(NSString* tagClass, NSString* tag, NSString* parent)
{
	NSString* payload = [@[tagClass, tag, parent ?: @""] componentsJoinedByString: kDynamicSeparator];
	NSData* data = [payload dataUsingEncoding: NSUTF8StringEncoding];
	const unsigned char* bytes = [data bytes];
	NSMutableString* hex = [NSMutableString stringWithString: kDynamicPrefix];

	for (NSUInteger i = 0; i < [data length]; i++)
		[hex appendFormat: @"%02x", bytes[i]];
	return hex;
}

// The bits that stand for the identifier of a type that a dynamic type conforms to.
static NSString* dynamicParentRegion(NSString* parent)
{
	NSString* lowered = [parent lowercaseString];

	for (size_t i = 0; i < sizeof(kDynamicTokens) / sizeof(kDynamicTokens[0]); i++)
	{
		if ([lowered isEqualToString: [NSString stringWithUTF8String: kDynamicTokens[i].identifier]])
			return [NSString stringWithUTF8String: kDynamicTokens[i].bits];
	}

	NSMutableString* region = [NSMutableString stringWithString: kDynamicHeader];
	if (!appendDynamicCharacters(region, lowered))
		return nil;
	[region appendString: @"0"];
	return region;
}

static NSString* dynamicIdentifier(NSString* tagClass, NSString* tag, NSString* parent)
{
	BOOL lowercase = NO;
	NSString* classCode = dynamicClassCode(tagClass, &lowercase);
	NSMutableString* bits = [NSMutableString stringWithString: kDynamicLead];

	if (classCode != nil && parent != nil)
	{
		NSString* region = dynamicParentRegion(parent);
		if (region == nil)
			classCode = nil;
		else
			[bits appendFormat: @"%@%@%@", kDynamicConformsMarker, region, kDynamicConformsEnd];
	}
	if (classCode != nil)
	{
		[bits appendFormat: @"%@%@%@", kDynamicClassMarker, classCode, kDynamicHeader];

		NSMutableString* tagBits = [NSMutableString string];
		if (appendDynamicCharacters(tagBits, lowercase ? [tag lowercaseString] : tag) && [tagBits length] > 0)
		{
			// No separator follows the last character.
			[bits appendString: [tagBits substringToIndex: [tagBits length] - 1]];
			return dynamicBase32(bits);
		}
	}
	return dynamicIdentifierHex(tagClass, tag, parent);
}

// Returns the tag class, tag and parent of a dynamic identifier, or NO if it is not one of ours.
static BOOL parseDynamicIdentifier(NSString* uti, NSString** tagClass, NSString** tag, NSString** parent)
{
	if (!isDynamicIdentifier(uti))
		return NO;

	NSString* body = [uti substringFromIndex: [kDynamicPrefix length]];
	if ([body length] == 0)
		return NO;

	if ([body hasPrefix: @"a"] || [body hasPrefix: @"A"])
	{
		NSString* bits = dynamicBitsFromBase32([body lowercaseString]);
		NSUInteger pos = [kDynamicLead length];
		NSString* parentIdentifier = nil;

		if (bits == nil || ![bits hasPrefix: kDynamicLead])
			return NO;

		if ([[bits substringFromIndex: pos] hasPrefix: kDynamicConformsMarker])
		{
			pos += [kDynamicConformsMarker length];

			NSString* rest = [bits substringFromIndex: pos];
			for (size_t i = 0; i < sizeof(kDynamicTokens) / sizeof(kDynamicTokens[0]) && parentIdentifier == nil; i++)
			{
				NSString* token = [NSString stringWithFormat: @"%s%@", kDynamicTokens[i].bits, kDynamicConformsEnd];
				if ([rest hasPrefix: token])
				{
					parentIdentifier = [NSString stringWithUTF8String: kDynamicTokens[i].identifier];
					pos += [token length];
				}
			}
			if (parentIdentifier == nil)
			{
				if (![rest hasPrefix: kDynamicHeader])
					return NO;
				pos += [kDynamicHeader length];

				NSMutableString* characters = [NSMutableString string];
				NSString* end = [NSString stringWithFormat: @"0%@%@", kDynamicConformsEnd, kDynamicClassMarker];
				while (YES)
				{
					if ([bits length] - pos < 8)
						return NO;
					unichar c = (unichar) strtoul([[bits substringWithRange: NSMakeRange(pos, 7)] UTF8String], NULL, 2);
					if (c == 0)
						return NO;
					[characters appendFormat: @"%C", c];
					pos += 7;
					if ([bits characterAtIndex: pos] != '0')
						return NO;
					pos++;
					if ([bits length] - pos >= [end length] &&
						[[bits substringFromIndex: pos] hasPrefix: end])
					{
						pos += [kDynamicConformsEnd length] + 1;
						break;
					}
				}
				parentIdentifier = characters;
			}
		}

		if ([bits length] - pos < [kDynamicClassMarker length] + 4 + [kDynamicHeader length] + 7)
			return NO;
		if (![[bits substringWithRange: NSMakeRange(pos, [kDynamicClassMarker length])] isEqualToString: kDynamicClassMarker])
			return NO;
		pos += [kDynamicClassMarker length];

		NSString* classIdentifier = dynamicClassForCode([bits substringWithRange: NSMakeRange(pos, 4)]);
		pos += 4;
		if (classIdentifier == nil || ![[bits substringFromIndex: pos] hasPrefix: kDynamicHeader])
			return NO;
		pos += [kDynamicHeader length];

		NSMutableString* characters = [NSMutableString string];
		while ([bits length] - pos >= 7)
		{
			unichar c = (unichar) strtoul([[bits substringWithRange: NSMakeRange(pos, 7)] UTF8String], NULL, 2);
			if (c == 0)
				return NO;
			[characters appendFormat: @"%C", c];
			pos += 7;
			// A separator and another character follow, or only padding.
			if ([bits length] - pos < 8 || [bits characterAtIndex: pos] != '0')
				break;
			pos++;
		}
		if ([characters length] == 0 || [bits length] - pos > 4)
			return NO;

		*tagClass = classIdentifier;
		*tag = characters;
		*parent = parentIdentifier;
		return YES;
	}

	NSString* hex = body;
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
