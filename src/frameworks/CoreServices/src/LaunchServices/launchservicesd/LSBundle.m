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

-(void)processUTIs:(NSArray<NSDictionary<NSString*,id>*>*)utis
{
	NSNumber* ourBundleId = [NSNumber numberWithInt: _bundleId];
	// Maintain a set of previously existing UTIs so that we know which ones need to be deleted
	NSMutableSet<NSString*>* previousUtis = [NSMutableSet setWithCapacity: 0];

	FMResultSet* rs = [g_database executeQuery:@"select type_identifier from uti where bundle = ?",
		ourBundleId];

	while ([rs next])
		[previousUtis addObject: [rs stringForColumn: @"type_identifier"]];
	[rs close];

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
		if ([rs next])
		{
			// We already have this UTI record
			utiId = [NSNumber numberWithInt: [rs intForColumn:@"id"]];
			NSString* oldDesc = [rs stringForColumn:@"description"];

			// Update the description text if it changed
			if (![oldDesc isEqualToString: description])
			{
				[g_database executeUpdate:@"update uti set description = ? where id = ?", oldDesc, utiId];
			}
		}
		else
		{
			// Insert a new UTI record
			[g_database executeUpdate:@"insert into uti (type_identifier, description, bundle) values (?,?,?)",
				typeId, description, ourBundleId];
			utiId = [NSNumber numberWithInt: [g_database lastInsertRowId]];
		}
		[rs close];

		[previousUtis removeObject: typeId];

		// Process UTTypeConformsTo
		[g_database executeUpdate: @"delete from uti_conforms where uti = ?", utiId];

		id conformsTo = uti[(NSString*) kUTTypeConformsToKey];
		if ([conformsTo isKindOfClass: [NSString class]])
		{
			[g_database executeUpdate:@"insert into uti_conforms (uti, conforms_to) values (?,?)",
				utiId, conformsTo];
		}
		else if ([conformsTo isKindOfClass: [NSArray class]])
		{
			for (NSString* ct in ((NSArray<NSString*>*) conformsTo))
			{
				if (![ct isKindOfClass: [NSString class]])
					continue;
				[g_database executeUpdate:@"insert into uti_conforms (uti, conforms_to) values (?,?)",
					utiId, ct];
			}
		}

		// Process UTTypeIconFile / UTTypeIconFiles
		[g_database executeUpdate:@"delete from uti_icon where uti = ?", utiId];
		NSString* icon = uti[(NSString*) kUTTypeIconFileKey];
		if (icon != nil)
		{
			[g_database executeUpdate:@"insert into uti_icon (uti, file) values (?,?)",
				utiId, icon];
		}
		NSArray<NSString*>* icons = uti[@"UTTypeIconFiles"];
		if (icons != nil)
		{
			for (NSString* icon in icons)
			{
				if (![icon isKindOfClass: [NSString class]])
					continue;
				[g_database executeUpdate:@"insert into uti_icon (uti, file) values (?,?)",
					utiId, icon];
			}
		}

		// Process UTTypeTagSpecification
		[g_database executeUpdate:@"delete from uti_tag where uti = ?", utiId];
		NSDictionary<NSString*, id>* tags = uti[(NSString*) kUTTypeTagSpecificationKey];
		if (tags != nil)
		{
			for (NSString* tag in tags)
			{
				id tagValue = tags[tag];
				if ([tagValue isKindOfClass: [NSString class]])
				{
					[g_database executeUpdate:@"insert into uti_tag (uti, tag, value) values (?,?,?)",
						utiId, tag, tagValue];
				}
				else if ([tagValue isKindOfClass: [NSArray class]])
				{
					for (NSString* value in (NSArray<NSString*>*) tagValue)
					{
						if (![value isKindOfClass: [NSString class]])
							continue;
						[g_database executeUpdate:@"insert into uti_tag (uti, tag, value) values (?,?,?)",
							utiId, tag, value];
					}
				}
			}
		}
	}

	for (NSString* deletedId in previousUtis)
	{
		[g_database executeUpdate:@"delete from uti where type_identifier = ? and bundle = ?",
			deletedId, ourBundleId];
	}
}

// https://developer.apple.com/documentation/bundleresources/information_property_list/cfbundledocumenttypes?language=objc
-(void)processFileAssociation:(NSDictionary*)dict
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

	[g_database executeUpdate:@"insert into app_doc (icon,name,role,rank,class,bundle) values (?,?,?,?,?,?)",
		iconFile, displayName, role, rank, documentClass, myId];
	appDocId = [NSNumber numberWithInt: [g_database lastInsertRowId]];

	NSArray<NSString*>* contentTypes = dict[@"LSItemContentTypes"];
	if (contentTypes)
	{
		for (NSString* uti in contentTypes)
		{
			if (![uti isKindOfClass: [NSString class]])
				continue;

			[g_database executeUpdate:@"insert into app_doc_uti (doc, uti) values (?,?)", appDocId, uti];
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

				[g_database executeUpdate:@"insert into app_doc_extension (doc, extension) values (?,?)", appDocId, extension];
			}
		}

		NSArray<NSString*>* mimeTypes = dict[@"CFBundleTypeMIMETypes"];
		if (mimeTypes)
		{
			for (NSString* mime in mimeTypes)
			{
				if (![mime isKindOfClass: [NSString class]])
					continue;
				
				[g_database executeUpdate:@"insert into app_doc_mime (doc, mime) values (?,?)", appDocId, mime];
			}
		}
	}
}

-(void)processFileAssociations
{
	NSDictionary<NSString*,id>* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);
	NSNumber* myId = [NSNumber numberWithInt: _bundleId];

	[g_database executeUpdate:@"delete from app_doc where bundle = ?", [NSNumber numberWithInt: _bundleId]];

	NSArray<NSDictionary*>* types = (NSArray*) infoDict[@"CFBundleDocumentTypes"];
	if (types)
	{
		NSLog(@"Found type array\n");
		for (NSDictionary* type in types)
			[self processFileAssociation: type];
	}
	if (infoDict[@"CFBundleTypeRole"] != nil)
	{
		NSLog(@"Found a single type\n");
		[self processFileAssociation: infoDict];
	}
}

-(BOOL)setupBundleID
{
	NSURL* url = (NSURL*) CFBundleCopyBundleURL(_bundle);
	NSString* path = [url path];
	const uint32_t newChecksum = [[(NSDictionary*) CFBundleGetInfoDictionary(_bundle) description] crc32];

	[url release];
	FMResultSet* rs = [g_database executeQuery:@"select id, checksum from bundle where path = ?", path];

	if ([rs next])
	{
		_bundleId = [rs intForColumn:@"id"];

		// Has the info dict changed?
		uint32_t dbChecksum = [rs intForColumn:@"checksum"];
		if (dbChecksum == newChecksum)
		{
			NSLog(@"Bundle at '%@' hasn't changed\n", path);
			[rs close];
			return FALSE;
		}
	}

	[rs close];

	if (!_bundleId)
	{
		CFStringRef identifier = CFBundleGetIdentifier(_bundle);

		NSLog(@"Registering new bundle at '%@', identifier '%@'\n", path, identifier);

		[g_database executeUpdate:@"insert into bundle (path, bundle_id, checksum) values (?,?,?)",
			path, identifier, [NSNumber numberWithInt:newChecksum]];
			
		_bundleId = [g_database lastInsertRowId];
	}
	else
	{
		NSLog(@"Updating bundle at '%@'\n", path);

		// We're in a transaction, so it's OK to set the new checksum now
		[g_database executeUpdate:@"update bundle set checksum = ? where id = ?",
			[NSNumber numberWithInt:newChecksum], [NSNumber numberWithInt: _bundleId]];
	}

	return TRUE;
}

-(void)process
{
	BOOL needsProcessing;

	[g_database beginTransaction];
	needsProcessing = [self setupBundleID];

	if (!needsProcessing)
	{
		[g_database rollback];
		return;
	}

	NSDictionary<NSString*,id>* infoDict = (NSDictionary*) CFBundleGetInfoDictionary(_bundle);

	NSArray<NSDictionary*>* utis = infoDict[(NSString*) kUTExportedTypeDeclarationsKey];
	if (utis != nil)
	{
		[self processUTIs:utis];
	}

	[self processFileAssociations];
	[g_database commit];
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
				continue;

			@autoreleasepool
			{
				LSBundle* b = [[[LSBundle alloc] initWithBundle: (CFBundleRef) bundle] autorelease];
				[b process];
			}
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
