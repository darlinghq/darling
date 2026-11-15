/**
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Objects that do not fit the plain XPC dictionary encoding of the service travel as a keyed archive: the reply (or
// a nested dictionary) carries the archive under "bsx_archive", and a "bsx_index" string names the object in it
// (the key under which it was archived).
//
// QUIRK: undocumented layout. Evidence: black-box observation, tier 2, of the replies of the system's
// com.apple.runningboard service to a client of our own, macOS 26.7.3 x86_64:
// - the audit token of the caller is an archived BSAuditToken whose "_auditToken" is the 32 bytes of the audit_token_t
//   (uid, euid, gid, ruid, rgid, pid, asid, pidversion), archived under the key of the "bsx_index" next to it;
// - an error is an archived NSError, with the same index in the reply's "__RBSXPCError__" dictionary.
// The indexes of one daemon run are distinct numbers (in the observation: one counter that rose by one per archive).
// Test: tests/runningboardd-service.c

#import <Foundation/Foundation.h>
#include <string.h>

#include "archive.h"

// Only the class name and the "_auditToken" key matter to the client that decodes it.
@interface BSAuditToken : NSObject <NSSecureCoding> {
	NSData* _auditToken;
}
- (instancetype)initWithAuditTokenData: (NSData*)data;
@end

@implementation BSAuditToken
+ (BOOL)supportsSecureCoding { return YES; }
- (instancetype)initWithAuditTokenData: (NSData*)data {
	if ((self = [super init])) {
		_auditToken = [data copy];
	}
	return self;
}
- (instancetype)initWithCoder: (NSCoder*)coder {
	if ((self = [super init])) {
		_auditToken = [[coder decodeObjectOfClass: [NSData class] forKey: @"_auditToken"] copy];
	}
	return self;
}
- (void)encodeWithCoder: (NSCoder*)coder {
	[coder encodeObject: _auditToken forKey: @"_auditToken"];
}
- (void)dealloc {
	[_auditToken release];
	[super dealloc];
}
@end

static int64_t next_index = 0;

static NSString* archive_object(NSObject<NSSecureCoding>* object, xpc_object_t dictionary, const char* archive_key) {
	NSString* index = [NSString stringWithFormat: @"%lld", (long long)__atomic_add_fetch(&next_index, 1, __ATOMIC_RELAXED)];
	NSKeyedArchiver* archiver = [[NSKeyedArchiver alloc] initRequiringSecureCoding: YES];
	[archiver encodeObject: object forKey: index];
	[archiver finishEncoding];
	NSData* data = [archiver encodedData];
	xpc_dictionary_set_data(dictionary, archive_key, [data bytes], [data length]);
	[archiver release];
	return index;
}

xpc_object_t archive_audit_token_create(xpc_object_t dictionary, audit_token_t token) {
	xpc_object_t reference = NULL;
	@autoreleasepool {
		BSAuditToken* object = [[[BSAuditToken alloc] initWithAuditTokenData: [NSData dataWithBytes: &token length: sizeof(token)]] autorelease];
		NSString* index = archive_object(object, dictionary, ARCHIVE_KEY);
		reference = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(reference, INDEX_KEY, [index UTF8String]);
	}
	return reference;
}

void archive_error_reply(xpc_object_t reply, const char* domain, int code, const char* reason) {
	@autoreleasepool {
		NSError* error = [NSError errorWithDomain: [NSString stringWithUTF8String: domain] code: code
		                                 userInfo: @{ NSLocalizedFailureReasonErrorKey: [NSString stringWithUTF8String: reason] }];
		NSString* index = archive_object(error, reply, ARCHIVE_KEY);
		xpc_object_t reference = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(reference, INDEX_KEY, [index UTF8String]);
		xpc_dictionary_set_value(reply, ERROR_KEY, reference);
		xpc_release(reference);
	}
}
