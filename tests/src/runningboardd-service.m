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

// Guest test for the com.apple.runningboard service, against the reply layouts observed on macOS 26.7.3 (see the
// daemon's sources). Prints "PASS: ..." and exits 0, or prints "FAIL: ..." lines and exits 1.
// Without the daemon installed the lookup fails (bootstrap_look_up returns 1102).
#import <Foundation/Foundation.h>
#include <dispatch/dispatch.h>
#include <mach/mach.h>
#include <servers/bootstrap.h>
#include <string.h>
#include <unistd.h>
#include <xpc/xpc.h>

static int failures = 0;
#define CHECK(condition, ...) do { if (!(condition)) { NSLog(@"FAIL: " __VA_ARGS__); ++failures; } } while (0)

// The client's view of the archived audit token: only the class name and the key matter.
@interface BSAuditToken : NSObject <NSSecureCoding> {
@public
	NSData* _auditToken;
}
@end
@implementation BSAuditToken
+ (BOOL)supportsSecureCoding { return YES; }
- (instancetype)initWithCoder: (NSCoder*)coder {
	if ((self = [super init])) _auditToken = [coder decodeObjectOfClass: [NSData class] forKey: @"_auditToken"];
	return self;
}
- (void)encodeWithCoder: (NSCoder*)coder {}
@end

static xpc_connection_t open_connection(void) {
	xpc_connection_t connection = xpc_connection_create_mach_service("com.apple.runningboard", NULL, 0);
	xpc_connection_set_event_handler(connection, ^(xpc_object_t event) { (void)event; });
	xpc_connection_resume(connection);
	return connection;
}

static xpc_object_t ask(xpc_connection_t connection, const char* selector, xpc_object_t argument) {
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	if (selector != NULL) xpc_dictionary_set_string(request, "rbs_selector", selector);
	if (argument != NULL) xpc_dictionary_set_value(request, "rbs_argument_0", argument);
	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(connection, request);
	xpc_release(request);
	return reply;
}

static xpc_object_t member(xpc_object_t dictionary, const char* key, xpc_type_t type, const char* where) {
	xpc_object_t value = (dictionary != NULL && xpc_get_type(dictionary) == XPC_TYPE_DICTIONARY) ? xpc_dictionary_get_value(dictionary, key) : NULL;
	if (value == NULL || xpc_get_type(value) != type) {
		NSLog(@"FAIL: %s.%s is missing or has the wrong type", where, key);
		++failures;
		return NULL;
	}
	return value;
}

static void check_class(xpc_object_t dictionary, const char* expected, const char* where) {
	xpc_object_t value = member(dictionary, "bsx_class", XPC_TYPE_STRING, where);
	if (value != NULL) CHECK(strcmp(xpc_string_get_string_ptr(value), expected) == 0, "%s.bsx_class is '%s', expected '%s'", where, xpc_string_get_string_ptr(value), expected);
}

static NSData* archive_of(xpc_object_t dictionary, const char* where) {
	xpc_object_t data = member(dictionary, "bsx_archive", XPC_TYPE_DATA, where);
	return data == NULL ? nil : [NSData dataWithBytes: xpc_data_get_bytes_ptr(data) length: xpc_data_get_length(data)];
}

// An error reply: __RBSXPCError__ with an index naming an archived NSError, and no return-value.
static void check_error(xpc_object_t reply, NSString* domain, NSInteger code, NSString* reason, const char* what) {
	CHECK(xpc_get_type(reply) == XPC_TYPE_DICTIONARY, "%s: the reply is not a dictionary", what);
	if (xpc_get_type(reply) != XPC_TYPE_DICTIONARY) return;
	CHECK(xpc_dictionary_get_value(reply, "return-value") == NULL, "%s: an error reply carries a return-value", what);
	xpc_object_t reference = member(reply, "__RBSXPCError__", XPC_TYPE_DICTIONARY, what);
	xpc_object_t index = member(reference, "bsx_index", XPC_TYPE_STRING, what);
	NSData* archive = archive_of(reply, what);
	if (index == NULL || archive == nil) return;
	NSError* decodeError = nil;
	NSKeyedUnarchiver* unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData: archive error: &decodeError];
	NSError* error = [unarchiver decodeObjectOfClass: [NSError class] forKey: [NSString stringWithUTF8String: xpc_string_get_string_ptr(index)]];
	CHECK(error != nil, "%s: the archive holds no NSError under the index (%@)", what, decodeError);
	if (error == nil) return;
	CHECK([error.domain isEqualToString: domain], "%s: domain %@, expected %@", what, error.domain, domain);
	CHECK(error.code == code, "%s: code %ld, expected %ld", what, (long)error.code, (long)code);
	CHECK([error.userInfo[NSLocalizedFailureReasonErrorKey] isEqual: reason], "%s: reason %@", what, error.userInfo[NSLocalizedFailureReasonErrorKey]);
}

static xpc_object_t descriptor(BOOL withAttribute) {
	xpc_object_t d = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(d, "bsx_class", "RBSAssertionDescriptor");
	xpc_dictionary_set_string(d, "explanation", "guest test");
	xpc_object_t attributes = xpc_array_create(NULL, 0);
	if (withAttribute) {
		xpc_object_t a = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(a, "bsx_class", "RBSDomainAttribute");
		xpc_dictionary_set_string(a, "domain", "com.apple.common");
		xpc_dictionary_set_string(a, "name", "FinishTaskUninterruptable");
		xpc_array_append_value(attributes, a);
		xpc_release(a);
	}
	xpc_dictionary_set_value(d, "attributes", attributes);
	xpc_release(attributes);
	return d;
}

static int64_t acquire(xpc_connection_t connection, const char* where) {
	xpc_object_t argument = descriptor(YES);
	xpc_object_t reply = ask(connection, "acquireAssertionWithDescriptor:error:", argument);
	xpc_release(argument);
	int64_t count = 0;
	CHECK(xpc_get_type(reply) == XPC_TYPE_DICTIONARY && xpc_dictionary_get_value(reply, "__RBSXPCError__") == NULL, "%s: the reply is an error", where);
	if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY) {
		xpc_object_t identifier = member(reply, "return-value", XPC_TYPE_DICTIONARY, where);
		if (identifier != NULL) {
			check_class(identifier, "RBSAssertionIdentifier", "acquire.return-value");
			xpc_object_t client = member(identifier, "clientPid", XPC_TYPE_INT64, "acquire.return-value");
			xpc_object_t server = member(identifier, "serverPid", XPC_TYPE_INT64, "acquire.return-value");
			xpc_object_t counter = member(identifier, "count", XPC_TYPE_INT64, "acquire.return-value");
			CHECK(client == NULL || xpc_int64_get_value(client) == getpid(), "acquire.clientPid is not the caller");
			CHECK(server == NULL || (xpc_int64_get_value(server) > 0 && xpc_int64_get_value(server) != getpid()), "acquire.serverPid is not another process");
			if (counter != NULL) count = xpc_int64_get_value(counter);
		}
	}
	return count;
}

int main(void) {
	@autoreleasepool {
		mach_port_t port = MACH_PORT_NULL;
		kern_return_t status = bootstrap_look_up(bootstrap_port, "com.apple.runningboard", &port);
		CHECK(status == KERN_SUCCESS, "bootstrap_look_up(com.apple.runningboard) returned %d", status);

		// a watchdog turns a silent hang into a failure
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 30 * NSEC_PER_SEC), dispatch_get_global_queue(0, 0), ^{
			NSLog(@"FAIL: no reply from com.apple.runningboard within 30 seconds");
			_exit(1);
		});

		xpc_connection_t connection = open_connection();
		// messages that expect no reply are ignored: the opening empty message and an asynchronous operation
		xpc_object_t hello = xpc_dictionary_create(NULL, NULL, 0);
		xpc_connection_send_message(connection, hello);
		xpc_release(hello);
		xpc_object_t async = xpc_dictionary_create(NULL, NULL, 0);
		xpc_dictionary_set_string(async, "rbs_selector", "async_invalidateAssertionWithIdentifier:");
		xpc_connection_send_message(connection, async);
		xpc_release(async);

		// an operation the service does not implement
		xpc_object_t unknown = ask(connection, "noSuchOperation:", NULL);
		check_error(unknown, @"RBSServiceErrorDomain", 501, @"The server has no implementation for this message", "unknown operation");

		// the handshake: every field of the observed layout, with this process as the caller
		xpc_object_t reply = ask(connection, "handshakeWithRequest:", NULL);
		xpc_object_t response = member(reply, "return-value", XPC_TYPE_DICTIONARY, "handshake");
		if (response != NULL) {
			check_class(response, "RBSHandshakeResponse", "handshake.return-value");
			const char* arrays[] = { "success-keys", "success-values", "fail-keys", "fail-values" };
			for (int i = 0; i < 4; i++) {
				xpc_object_t array = member(response, arrays[i], XPC_TYPE_ARRAY, "handshake.return-value");
				CHECK(array == NULL || xpc_array_get_count(array) == 0, "handshake.return-value.%s is not empty", arrays[i]);
			}
			xpc_object_t handle = member(response, "_handle", XPC_TYPE_DICTIONARY, "handshake.return-value");
			check_class(handle, "RBSProcessHandle", "_handle");
			xpc_object_t coded = member(handle, "_codedHandle", XPC_TYPE_DICTIONARY, "_handle");
			xpc_object_t identity = member(coded, "_identity", XPC_TYPE_DICTIONARY, "_codedHandle");
			check_class(identity, "RBSOpaqueProcessIdentity", "_identity");
			xpc_object_t identity_pid = member(identity, "_pid", XPC_TYPE_INT64, "_identity");
			xpc_object_t identity_auid = member(identity, "_auid", XPC_TYPE_INT64, "_identity");
			xpc_object_t description = member(identity, "_description", XPC_TYPE_STRING, "_identity");
			CHECK(identity_pid == NULL || xpc_int64_get_value(identity_pid) == getpid(), "_identity._pid is not the caller");
			CHECK(identity_auid == NULL || xpc_int64_get_value(identity_auid) == (int64_t)getuid(), "_identity._auid is not the caller's uid");
			if (description != NULL) {
				NSString* text = [NSString stringWithUTF8String: xpc_string_get_string_ptr(description)];
				NSString* suffix = [NSString stringWithFormat: @">(%u)", (unsigned)getuid()];
				CHECK([text hasPrefix: @"anon<"] && [text hasSuffix: suffix], "_identity._description '%@' is not anon<name>(uid)", text);
			}
			xpc_object_t coded_pid = member(coded, "_pid", XPC_TYPE_UINT64, "_codedHandle");
			CHECK(coded_pid == NULL || xpc_uint64_get_value(coded_pid) == (uint64_t)getpid(), "_codedHandle._pid is not the caller");
			check_class(member(coded, "_bundle", XPC_TYPE_DICTIONARY, "_codedHandle"), "RBSProcessBundle", "_bundle");

			// the audit token is an archived BSAuditToken named by the index next to it
			xpc_object_t token_reference = member(coded, "_bsAuditToken", XPC_TYPE_DICTIONARY, "_codedHandle");
			xpc_object_t index = member(token_reference, "bsx_index", XPC_TYPE_STRING, "_bsAuditToken");
			NSData* archive = archive_of(coded, "_codedHandle");
			if (index != NULL && archive != nil) {
				NSKeyedUnarchiver* unarchiver = [[NSKeyedUnarchiver alloc] initForReadingFromData: archive error: NULL];
						BSAuditToken* token = [unarchiver decodeObjectOfClass: [BSAuditToken class] forKey: [NSString stringWithUTF8String: xpc_string_get_string_ptr(index)]];
				CHECK(token != nil && [token->_auditToken length] == sizeof(audit_token_t), "the archive holds no 32-byte BSAuditToken under the index");
				if (token != nil && [token->_auditToken length] == sizeof(audit_token_t)) {
					audit_token_t t;
					[token->_auditToken getBytes: &t length: sizeof(t)];
					CHECK(t.val[5] == (unsigned)getpid(), "the token's pid is not the caller");
					CHECK(t.val[1] == (unsigned)geteuid(), "the token's effective uid is not the caller's");
					CHECK(t.val[2] == (unsigned)getegid(), "the token's effective gid is not the caller's");
				}
			}
		}

		// acquire: the global counter rises by one per call; a descriptor without attributes is an error
		int64_t first = acquire(connection, "first acquire");
		int64_t second = acquire(connection, "second acquire");
		CHECK(first > 0 && second == first + 1, "the acquire count must rise by one per call (got %lld then %lld)", (long long)first, (long long)second);
		xpc_object_t empty = descriptor(NO);
		xpc_object_t none = ask(connection, "acquireAssertionWithDescriptor:error:", empty);
		xpc_release(empty);
		check_error(none, @"RBSAssertionErrorDomain", 2, @"Cannot acquire an assertion with no attributes", "acquire without attributes");

		// a request without an operation name closes the connection; the service still answers others
		xpc_connection_t other = open_connection();
		xpc_object_t nameless = ask(other, NULL, NULL);
		CHECK(xpc_get_type(nameless) == XPC_TYPE_ERROR, "a request without an operation name was answered with a message");
		xpc_connection_t third = open_connection();
		CHECK(acquire(third, "acquire after a nameless request") > 0, "the service stopped answering after a nameless request");

		if (failures == 0) {
			printf("PASS: com.apple.runningboard is registered and answers with the observed layouts and errors\n");
		}
	}
	return failures ? 1 : 0;
}
