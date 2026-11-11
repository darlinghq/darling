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

#include "LSBundle.h"
#include <FSEvents/FSEvents.h>
#include <sqlite3.h>
#import <Foundation/Foundation.h>
#import <fmdb/FMDatabase.h>
#include <zlib.h>
#include <LaunchServices/UTType.h>

#define DATABASE_VERSION 1

static FMDatabase* g_database;
// Every registration write must participate in the caller's transaction.
#define LS_UPDATE(...) do { if (![g_database executeUpdate:__VA_ARGS__]) return NO; } while (0)
extern dispatch_queue_t g_serverQueue;

static void setupDBSchema(void);
static void fsEventCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents,
	void *eventPaths, const FSEventStreamEventFlags *eventFlags, const FSEventStreamEventId *eventIds);

static NSArray<NSString*>* MONITORED_DIRECTORIES;
static FSEventStreamRef g_eventStream;

@implementation NSString (CRC32)
-(uint32_t)crc32
{
	const char* str = [self UTF8String];
	return crc32(0, (const Bytef*) str, strlen(str));
}
@end

@implementation LSBundle

@synthesize bundleId = _bundleId;

-(id) initWithBundle:(CFBundleRef) bundle
{
	_bundle = (CFBundleRef) CFRetain(bundle);

	return self;
}

-(void)dealloc
{
	CFRelease(_bundle);
	[super dealloc];
}

-(BOOL)processUTIs:(NSArray<NSDictionary<NSString*,id>*>*)utis
{
	NSNumber* ourBundleId = [NSNumber numberWithInt: _bundleId];
	// Maintain a set of previously existing UTIs so that we know which ones need to be deleted
	NSMutableSet<NSString*>* previousUtis = [NSMutableSet setWithCapacity: 0];

	FMResultSet* rs = [g_database executeQuery:@"select type_identifier from uti where bundle = ?",
		ourBundleId];

	if (!rs) return NO;
	NSError* queryError = nil;
	while ([rs nextWithError:&queryError])
		[previousUtis addObject: [rs stringForColumn: @"type_identifier"]];
	[rs close];
	if (queryError) return NO;

	for (NSDictionary<NSString*,id>* uti in utis)
	{
		NSNumber* utiId;
		NSString* typeId = uti[(NSString*) kUTTypeIdentifierKey];

		if (![typeId isKindOfClass: [NSString class]])
			continue;

		NSString* description = uti[(NSString*) kUTTypeDescriptionKey];
		if (![description isKindOfClass: [NSString class]])
			description = typeId;

		FMResultSet* rs = [g_database executeQuery:@"select id, description from uti where type_identifier = ? and bundle = ?",
			typeId, ourBundleId];
		if (!rs) return NO;
		queryError = nil;
		BOOL found = [rs nextWithError:&queryError];
		if (queryError) { [rs close]; return NO; }
		if (found)
		{
			// We already have this UTI record
			utiId = [NSNumber numberWithInt: [rs intForColumn:@"id"]];
			NSString* oldDesc = [[[rs stringForColumn:@"description"] retain] autorelease];
			[rs close];

			// Update the description text if it changed
			if (![oldDesc isEqualToString: description])
			{
				LS_UPDATE(@"update uti set description = ? where id = ?", description, utiId);
			}
		}
		else
		{
			[rs close];
			// Insert a new UTI record
			LS_UPDATE(@"insert into uti (type_identifier, description, bundle) values (?,?,?)",
				typeId, description, ourBundleId);
			utiId = [NSNumber numberWithInt: [g_database lastInsertRowId]];
		}
		[rs close];

		[previousUtis removeObject: typeId];

		// Process UTTypeConformsTo
		LS_UPDATE(@"delete from uti_conforms where uti = ?", utiId);

		id conformsTo = uti[(NSString*) kUTTypeConformsToKey];
		if ([conformsTo isKindOfClass: [NSString class]])
		{
			LS_UPDATE(@"insert into uti_conforms (uti, conforms_to) values (?,?)", utiId, conformsTo);
		}
		else if ([conformsTo isKindOfClass: [NSArray class]])
		{
			for (NSString* ct in ((NSArray<NSString*>*) conformsTo))
			{
				if (![ct isKindOfClass: [NSString class]])
					continue;
				LS_UPDATE(@"insert into uti_conforms (uti, conforms_to) values (?,?)", utiId, ct);
			}
		}

		// Process UTTypeIconFile / UTTypeIconFiles
		LS_UPDATE(@"delete from uti_icon where uti = ?", utiId);
		NSString* icon = uti[(NSString*) kUTTypeIconFileKey];
		if (icon != nil)
		{
			LS_UPDATE(@"insert into uti_icon (uti, file) values (?,?)", utiId, icon);
		}
		NSArray<NSString*>* icons = uti[@"UTTypeIconFiles"];
		if (icons != nil)
		{
			for (NSString* icon in icons)
			{
				if (![icon isKindOfClass: [NSString class]])
					continue;
				LS_UPDATE(@"insert into uti_icon (uti, file) values (?,?)", utiId, icon);
			}
		}

		// Process UTTypeTagSpecification
		LS_UPDATE(@"delete from uti_tag where uti = ?", utiId);
		NSDictionary<NSString*, id>* tags = uti[(NSString*) kUTTypeTagSpecificationKey];
		if (tags != nil)
		{
			for (NSString* tag in tags)
			{
				id tagValue = tags[tag];
				if ([tagValue isKindOfClass: [NSString class]])
				{
					LS_UPDATE(@"insert into uti_tag (uti, tag, value) values (?,?,?)", utiId, tag, tagValue);
				}
				else if ([tagValue isKindOfClass: [NSArray class]])
				{
					for (NSString* value in (NSArray<NSString*>*) tagValue)
					{
						if (![value isKindOfClass: [NSString class]])
							continue;
						LS_UPDATE(@"insert into uti_tag (uti, tag, value) values (?,?,?)", utiId, tag, value);
					}
				}
			}
		}
	}

	for (NSString* deletedId in previousUtis)
	{
		LS_UPDATE(@"delete from uti where type_identifier = ? and bundle = ?", deletedId, ourBundleId);
	}
	return YES;
}

// https://developer.apple.com/documentation/bundleresources/information_property_list/cfbundledocumenttypes?language=objc
-(BOOL)processFileAssociation:(NSDictionary*)dict
{
	NSNumber* myId = [NSNumber numberWithInt: _bundleId];
	NSNumber* appDocId;

	NSString* iconFile = dict[@"CFBundleTypeIconFile"];
	NSString* displayName = dict[@"CFBundleTypeName"];
	NSString* role = dict[@"CFBundleTypeRole"];
	NSString* rank = dict[@"LSHandlerRank"];
	NSString* documentClass = dict[@"NSDocumentClass"];

	// TODO: exportable types?

	if (!role)
		role = @"None";
	if (!rank)
		rank = @"Default";

	LS_UPDATE(@"insert into app_doc (icon,name,role,rank,class,bundle) values (?,?,?,?,?,?)",
		iconFile, displayName, role, rank, documentClass, myId);
	appDocId = [NSNumber numberWithInt: [g_database lastInsertRowId]];

	NSArray<NSString*>* contentTypes = dict[@"LSItemContentTypes"];
	if (contentTypes)
	{
		for (NSString* uti in contentTypes)
		{
			if (![uti isKindOfClass: [NSString class]])
				continue;

			LS_UPDATE(@"insert into app_doc_uti (doc, uti) values (?,?)", appDocId, uti);
		}
	}
	else
	{
		// Support for obsolete CFBundleTypeExtensions and CFBundleTypeMIMETypes
		NSArray<NSString*>* extensions = dict[@"CFBundleTypeExtensions"];

		if (extensions)
		{
			for (NSString* extension in extensions)
			{
				if (![extension isKindOfClass: [NSString class]])
					continue;

				LS_UPDATE(@"insert into app_doc_extension (doc, extension) values (?,?)", appDocId, extension);
			}
		}

		NSArray<NSString*>* mimeTypes = dict[@"CFBundleTypeMIMETypes"];
		if (mimeTypes)
		{
			for (NSString* mime in mimeTypes)
			{
				if (![mime isKindOfClass: [NSString class]])
					continue;
				
				LS_UPDATE(@"insert into app_doc_mime (doc, mime) values (?,?)", appDocId, mime);
			}
		}
	}
	return YES;
}

-(BOOL)processFileAssociations
{
	NSDictionary<NSString*,id>* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);
	NSNumber* myId = [NSNumber numberWithInt: _bundleId];

	LS_UPDATE(@"delete from app_doc where bundle = ?", [NSNumber numberWithInt: _bundleId]);

	NSArray<NSDictionary*>* types = (NSArray*) infoDict[@"CFBundleDocumentTypes"];
	if (types)
	{
		for (NSDictionary* type in types)
			if (![self processFileAssociation: type]) return NO;
	}
	if (infoDict[@"CFBundleTypeRole"] != nil)
	{
		if (![self processFileAssociation: infoDict]) return NO;
	}
	return YES;
}

-(BOOL)processURLTypes
{
	NSDictionary* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);
	NSArray<NSDictionary*>* urlTypes = (NSArray*) infoDict[@"CFBundleURLTypes"];
	NSNumber* myId = [NSNumber numberWithInt: _bundleId];

	LS_UPDATE(@"delete from bundle_url_type where bundle = ?", myId);
	if (urlTypes != nil)
	{
		for (NSDictionary<NSString*, id>* type in urlTypes)
		{
			NSString* role = type[@"CFBundleTypeRole"];
			NSString* iconFile = type[@"CFBundleURLTypes"];
			NSString* name = type[@"CFBundleURLName"];

			if (role == nil)
				role = @"None";

			NSArray<NSString*>* schemes = type[@"CFBundleURLSchemes"];

			LS_UPDATE(@"insert into bundle_url_type (bundle, role, name, icon) values (?,?,?,?)",
				myId, role, name, iconFile);

			if (schemes)
			{
				NSNumber* typeId = [NSNumber numberWithInt:[g_database lastInsertRowId]];
				
				for (NSString* scheme in schemes)
					LS_UPDATE(@"insert into bundle_url_type_scheme (type, scheme) values (?,?)", typeId, scheme);
			}
		}
	}
	return YES;
}

-(BOOL)setupBundleID:(BOOL*)needsProcessing
{
	NSURL* url = (NSURL*) CFBundleCopyBundleURL(_bundle);
	NSString* path = [[[url path] retain] autorelease];
	NSDictionary* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);
	const uint32_t newChecksum = [[infoDict description] crc32];

	[url release];
	FMResultSet* rs = [g_database executeQuery:@"select id, checksum from bundle where path = ?", path];
	if (!rs) return NO;
	NSError* queryError = nil;
	BOOL found = [rs nextWithError:&queryError];
	if (queryError) { [rs close]; return NO; }
	_bundleId = 0;
	*needsProcessing = YES;
	if (found)
	{
		_bundleId = [rs intForColumn:@"id"];

		// Has the info dict changed?
		uint32_t dbChecksum = [rs intForColumn:@"checksum"];
		if (dbChecksum == newChecksum)
		{
			NSLog(@"Bundle at '%@' hasn't changed\n", path);
			[rs close];
			*needsProcessing = NO;
			return YES;
		}
	}

	[rs close];

	UInt32 packageType = 0, packageCreator = 0;
	CFBundleGetPackageInfo(_bundle, &packageType, &packageCreator);

	NSString* packageTypeStr = nil;
	NSString* packageCreatorStr = nil;
	NSString* bundleSignature = infoDict[@"CFBundleSignature"];

	if (packageType != 0)
		packageTypeStr = [LSBundle fourcc:packageType];
	if (packageCreator != 0)
		packageCreatorStr = [LSBundle fourcc:packageCreator];

	if (!_bundleId)
	{
		CFStringRef identifier = CFBundleGetIdentifier(_bundle);

		NSLog(@"Registering new bundle at '%@', identifier '%@'\n", path, identifier);

		LS_UPDATE(@"insert into bundle (path, bundle_id, checksum, package_type, creator, signature) values (?,?,?,?,?,?)",
			path, identifier, [NSNumber numberWithInt:newChecksum], packageTypeStr, packageCreatorStr, bundleSignature);
			
		_bundleId = [g_database lastInsertRowId];
	}
	else
	{
		NSLog(@"Updating bundle at '%@'\n", path);

		// We're in a transaction, so it's OK to set the new checksum now
		LS_UPDATE(@"update bundle set checksum = ?, package_type = ?, creator = ?, signature = ? where id = ?",
			[NSNumber numberWithInt:newChecksum], packageTypeStr, packageCreatorStr,
			bundleSignature, [NSNumber numberWithInt: _bundleId]);
	}

	return TRUE;
}

-(BOOL)process
{
	if (!g_database || ![g_database beginTransaction]) return NO;
	BOOL committed = NO;
	@try {
		BOOL needsProcessing = NO;
		if (![self setupBundleID:&needsProcessing]) return NO;
		if (needsProcessing) {
			NSDictionary* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);
			NSArray* utis = [infoDict objectForKey:(NSString*) kUTExportedTypeDeclarationsKey];
			if (![self processUTIs:utis] || ![self processFileAssociations] || ![self processURLTypes])
				return NO;
		}
		committed = [g_database commit];
		return committed;
	} @finally {
		if (!committed) {
			[g_database rollback];
			_bundleId = 0;
		}
	}
}

+(BOOL)registerBundleAtPath:(NSString*)path
{
	if (![path length]) return NO;
	NSURL* url = [NSURL fileURLWithPath:[path stringByStandardizingPath] isDirectory:YES];
	CFBundleRef bundle = CFBundleCreate(NULL, (CFURLRef)url);
	if (!bundle) return NO;
	BOOL success = NO;
	@try {
		if (CFBundleGetIdentifier(bundle)) {
			LSBundle* entry = [[[self alloc] initWithBundle:bundle] autorelease];
			success = [entry process];
		}
	} @finally {
		CFRelease(bundle);
	}
	return success;
}

+(void)initialize
{
	if (self == [LSBundle class])
	{
		MONITORED_DIRECTORIES = @[
			// This path doesn't contain apps, but may still contain UTI definitions
			@"/System/Library/Frameworks",

			@"/Applications",
			@"/System/Applications",
		];

		FMDatabase* db = [FMDatabase databaseWithPath: @"/private/var/db/launchservices.db"];
		if (![db open])
		{
			NSLog(@"Cannot open LS database: %@", [db lastErrorMessage]);
			return;
		}
		g_database = [db retain];

		setupDBSchema();
	}
}

+(NSString*)fourcc:(UInt32)code
{
	char str[5];
	str[0] = (code >> 24) & 0xff;
	str[1] = (code >> 16) & 0xff;
	str[2] = (code >> 8) & 0xff;
	str[3] = code & 0xff;
	str[4] = '\0';
	return [NSString stringWithCString:str encoding:NSASCIIStringEncoding];
}

+(void)scanForBundles:(NSString*)dir
{
	@autoreleasepool
	{
		NSURL* url = [NSURL fileURLWithPath:dir isDirectory:YES];
		NSArray* bundles = (NSArray*) CFBundleCreateBundlesFromDirectory(NULL, (CFURLRef) url, NULL);

		for (id bundle in bundles)
		{
			// CFBundle is not exactly clever and gives us CFBundles of empty directories
			if (CFBundleGetIdentifier((CFBundleRef) bundle) == NULL)
			{
				// CFBundleCreateBundlesFromDirectory transfers ownership of
				// each bundle as well as the array, including skipped entries.
				CFRelease((CFBundleRef) bundle);
				continue;
			}

			@autoreleasepool
			{
				LSBundle* b = [[[LSBundle alloc] initWithBundle: (CFBundleRef) bundle] autorelease];
				[b process];
			}

			CFRelease((CFBundleRef) bundle);
		}

		[bundles release];
	}
}

+(void)scanForBundles
{
	for (NSString* dir in MONITORED_DIRECTORIES)
		[LSBundle scanForBundles: dir];
	NSLog(@"scanForBundles done\n");
}

+(void)watchForBundles
{
	g_eventStream = FSEventStreamCreate(NULL, fsEventCallback, NULL, (CFArrayRef) MONITORED_DIRECTORIES,
		kFSEventStreamEventIdSinceNow, 1, kFSEventStreamCreateFlagUseCFTypes);

	FSEventStreamSetDispatchQueue(g_eventStream, g_serverQueue);
	FSEventStreamStart(g_eventStream);
}

+(void)deleteBundleAtPath:(NSString*) path
{
	// Delete DB entries referring to path
	[g_database beginTransaction];
	[g_database executeQuery: @"delete from bundle where path = ?", path];
	[g_database commit];
}

@end

static void createDBSchema(void)
{
	NSString* sqlSchema = [NSString stringWithContentsOfFile: @"/System/Library/Frameworks/CoreServices.framework/Versions/A/Resources/launchservicesd-schema.sql"
		encoding: NSUTF8StringEncoding
		error: nil];

	const char* sql = [sqlSchema cStringUsingEncoding: NSUTF8StringEncoding];
	char* errorMessage;

	sqlite3_exec(g_database.sqliteHandle, sql, NULL, NULL, &errorMessage);

	if (errorMessage != NULL)
	{
		NSLog(@"launchservicesd: createDBSchema error: %s\n", errorMessage);
		sqlite3_free(errorMessage);

		// TODO: abort_with_payload() instead?
		exit(1);
	}
}

static void setupDBSchema(void)
{
	FMResultSet* rs = [g_database executeQuery:@"select value from `globals` where key = ?", @"version"];
	if (![rs next])
	{
		createDBSchema();
	}
	else
	{
		if ([rs intForColumn: @"value"] != DATABASE_VERSION)
		{
			// TODO: This is where we'll do DB schema updates or possibly just delete everything and index from scratch
		}
		[rs close];
	}
}

static void fsEventCallback(ConstFSEventStreamRef streamRef, void *clientCallBackInfo, size_t numEvents,
	void *eventPaths, const FSEventStreamEventFlags *eventFlags, const FSEventStreamEventId *eventIds)
{
	NSArray<NSString*>* changes = (NSArray*) eventPaths;
	int index = 0;
	for (NSString* change in changes)
	{
		@autoreleasepool
		{
			if (eventFlags[index] & kFSEventStreamEventFlagItemIsFile)
			{
				if ([[change lastPathComponent] caseInsensitiveCompare: @"Info.plist"] == NSOrderedSame)
				{
					NSString* bundlePath = [[change stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
					CFBundleRef bundle = CFBundleCreate(NULL, (CFURLRef) [NSURL fileURLWithPath:bundlePath isDirectory:YES]);

					if (bundle != NULL)
					{
						LSBundle* lsb = [[[LSBundle alloc] initWithBundle: bundle] autorelease];
						[lsb process];

						CFRelease(bundle);
					}
					else
					{
						[LSBundle deleteBundleAtPath:bundlePath];
					}
				}

				// TODO: If somebody renames the bundle's directory, we still need to pick it up
			}
		}
		index++;
	}
}
