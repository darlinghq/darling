/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

// Agents, daemons and login items are submitted to the launchd this process runs under, which is how
// SMJobSubmit and `launchctl submit` register jobs. Darling has no Background Task Management database
// or approval UI, so a registration lasts until the container stops and never needs user approval.

#import <ServiceManagement/ServiceManagement.h>
#import <CoreServices/CoreServices.h>
#include <launch.h>
#include <errno.h>

// This file implements the macOS 13/15 API itself, below the framework's macOS 11 deployment target.
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

NSString *const SMAppServiceErrorDomain = @"SMAppServiceErrorDomain";

typedef enum {
	SMAppServiceKindMainApp,
	SMAppServiceKindLoginItem,
	SMAppServiceKindAgent,
	SMAppServiceKindDaemon,
} SMAppServiceKind;

typedef enum {
	SMJobStateLoaded,
	SMJobStateNotLoaded,
	SMJobStateUnknown,
} SMJobState;

static NSError *SMError(NSInteger code, int posixError, NSString *description)
{
	NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithObject: description forKey: NSLocalizedDescriptionKey];
	if (posixError)
		[userInfo setObject: [NSError errorWithDomain: NSPOSIXErrorDomain code: posixError userInfo: nil] forKey: NSUnderlyingErrorKey];
	return [NSError errorWithDomain: SMAppServiceErrorDomain code: code userInfo: userInfo];
}

static BOOL SMFail(NSError **error, NSInteger code, int posixError, NSString *description)
{
	if (error)
		*error = SMError(code, posixError, description);
	return NO;
}

// Returns NULL for values a launchd job dictionary cannot hold.
static launch_data_t SMLaunchDataFromObject(id object)
{
	if ([object isKindOfClass: [NSString class]])
		return launch_data_new_string([object UTF8String]);
	if ([object isKindOfClass: [NSNumber class]]) {
		if (CFGetTypeID((CFTypeRef) object) == CFBooleanGetTypeID())
			return launch_data_new_bool([object boolValue]);
		if (CFNumberIsFloatType((CFNumberRef) object))
			return launch_data_new_real([object doubleValue]);
		return launch_data_new_integer([object longLongValue]);
	}
	if ([object isKindOfClass: [NSData class]])
		return launch_data_new_opaque((void *) [object bytes], [object length]);
	if ([object isKindOfClass: [NSArray class]]) {
		launch_data_t array = launch_data_alloc(LAUNCH_DATA_ARRAY);
		size_t index = 0;
		for (id item in object) {
			launch_data_t value = SMLaunchDataFromObject(item);
			if (!value) {
				launch_data_free(array);
				return NULL;
			}
			launch_data_array_set_index(array, value, index++);
		}
		return array;
	}
	if ([object isKindOfClass: [NSDictionary class]]) {
		launch_data_t dict = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
		for (id key in object) {
			launch_data_t value = [key isKindOfClass: [NSString class]] ? SMLaunchDataFromObject([object objectForKey: key]) : NULL;
			if (!value) {
				launch_data_free(dict);
				return NULL;
			}
			launch_data_dict_insert(dict, value, [key UTF8String]);
		}
		return dict;
	}
	return NULL;
}

// Sends { key: value } to launchd (taking ownership of value) and returns the reply's errno, or -1 with
// *ipcErrno set when launchd could not be reached. A dictionary reply (GetJob) counts as success.
static int SMLaunchRequest(const char *key, launch_data_t value, int *ipcErrno)
{
	launch_data_t request = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
	launch_data_dict_insert(request, value, key);
	errno = 0;
	launch_data_t reply = launch_msg(request);
	*ipcErrno = errno;
	launch_data_free(request);
	if (!reply)
		return -1;

	int result = 0;
	if (launch_data_get_type(reply) == LAUNCH_DATA_ERRNO)
		result = launch_data_get_errno(reply);
	else if (launch_data_get_type(reply) != LAUNCH_DATA_DICTIONARY)
		result = EINVAL;
	launch_data_free(reply);
	return result;
}

static SMJobState SMJobStateForLabel(NSString *label)
{
	int ipcErrno;
	int result = SMLaunchRequest(LAUNCH_KEY_GETJOB, launch_data_new_string([label UTF8String]), &ipcErrno);
	if (result == 0)
		return SMJobStateLoaded;
	if (result == ESRCH)
		return SMJobStateNotLoaded;
	return SMJobStateUnknown;
}

static SMAppServiceStatus SMStatusForJobState(SMJobState state)
{
	switch (state) {
		case SMJobStateLoaded:
			return SMAppServiceStatusEnabled;
		case SMJobStateNotLoaded:
			return SMAppServiceStatusNotRegistered;
		default:
			return SMAppServiceStatusNotFound;
	}
}

@implementation SMAppService

- (instancetype)_initWithKind:(SMAppServiceKind)kind name:(NSString *)name
{
	if ((self = [super init])) {
		_kind = kind;
		_name = [name copy];
	}
	return self;
}

- (void)dealloc
{
	[_name release];
	[super dealloc];
}

+ (instancetype)loginItemServiceWithIdentifier:(NSString *)identifier
{
	return [[[self alloc] _initWithKind: SMAppServiceKindLoginItem name: identifier] autorelease];
}

+ (SMAppService *)mainAppService
{
	return [[[self alloc] _initWithKind: SMAppServiceKindMainApp name: [[NSBundle mainBundle] bundleIdentifier]] autorelease];
}

+ (instancetype)agentServiceWithPlistName:(NSString *)plistName
{
	return [[[self alloc] _initWithKind: SMAppServiceKindAgent name: plistName] autorelease];
}

+ (instancetype)daemonServiceWithPlistName:(NSString *)plistName
{
	return [[[self alloc] _initWithKind: SMAppServiceKindDaemon name: plistName] autorelease];
}

- (NSDictionary *)_loginItemJobWithError:(NSError **)error
{
	NSString *directory = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"Contents/Library/LoginItems"];
	for (NSString *entry in [[NSFileManager defaultManager] contentsOfDirectoryAtPath: directory error: NULL]) {
		NSBundle *bundle = [NSBundle bundleWithPath: [directory stringByAppendingPathComponent: entry]];
		if (![[bundle bundleIdentifier] isEqualToString: _name])
			continue;
		NSString *executable = [bundle executablePath];
		if (!executable) {
			SMFail(error, kSMErrorToolNotValid, 0, [NSString stringWithFormat: @"Login item %@ has no executable", _name]);
			return nil;
		}
		// A login item is started right away and restarted if it crashes or exits non-zero.
		return @{
			@LAUNCH_JOBKEY_LABEL: _name,
			@LAUNCH_JOBKEY_PROGRAM: executable,
			@LAUNCH_JOBKEY_RUNATLOAD: @YES,
			@LAUNCH_JOBKEY_KEEPALIVE: @{ @LAUNCH_JOBKEY_KEEPALIVE_SUCCESSFULEXIT: @NO },
		};
	}
	SMFail(error, kSMErrorToolNotValid, 0, [NSString stringWithFormat: @"No login item with identifier %@ in %@", _name, directory]);
	return nil;
}

- (NSDictionary *)_plistJobWithError:(NSError **)error
{
	NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
	NSString *directory = _kind == SMAppServiceKindAgent ? @"Contents/Library/LaunchAgents" : @"Contents/Library/LaunchDaemons";
	NSString *path = [[bundlePath stringByAppendingPathComponent: directory] stringByAppendingPathComponent: _name];

	NSDictionary *plist = [NSDictionary dictionaryWithContentsOfFile: path];
	if (!plist) {
		SMFail(error, kSMErrorJobPlistNotFound, 0, [NSString stringWithFormat: @"Unable to read plist: %@", path]);
		return nil;
	}
	if (![[plist objectForKey: @LAUNCH_JOBKEY_LABEL] isKindOfClass: [NSString class]]) {
		SMFail(error, kSMErrorInvalidPlist, 0, [NSString stringWithFormat: @"%@ has no Label", path]);
		return nil;
	}

	// BundleProgram is relative to the app bundle and unknown to launchd, which needs Program.
	NSMutableDictionary *job = [[plist mutableCopy] autorelease];
	id bundleProgram = [job objectForKey: @"BundleProgram"];
	if (bundleProgram) {
		if (![bundleProgram isKindOfClass: [NSString class]]) {
			SMFail(error, kSMErrorInvalidPlist, 0, [NSString stringWithFormat: @"%@ has a non-string BundleProgram", path]);
			return nil;
		}
		[job setObject: [bundlePath stringByAppendingPathComponent: bundleProgram] forKey: @LAUNCH_JOBKEY_PROGRAM];
		[job removeObjectForKey: @"BundleProgram"];
	}
	return job;
}

- (NSDictionary *)_jobWithError:(NSError **)error
{
	switch (_kind) {
		case SMAppServiceKindLoginItem:
			return [self _loginItemJobWithError: error];
		case SMAppServiceKindAgent:
		case SMAppServiceKindDaemon:
			return [self _plistJobWithError: error];
		case SMAppServiceKindMainApp:
			break;
	}
	SMFail(error, kSMErrorServiceUnavailable, 0, @"Darling has no login session to launch the main app at");
	return nil;
}

- (BOOL)registerAndReturnError:(NSError **)error
{
	NSDictionary *job = [self _jobWithError: error];
	if (!job)
		return NO;
	NSString *label = [job objectForKey: @LAUNCH_JOBKEY_LABEL];

	launch_data_t data = SMLaunchDataFromObject(job);
	if (!data)
		return SMFail(error, kSMErrorInvalidPlist, 0, [NSString stringWithFormat: @"Job %@ holds a value launchd cannot represent", label]);

	int ipcErrno;
	int result = SMLaunchRequest(LAUNCH_KEY_SUBMITJOB, data, &ipcErrno);
	if (result == -1)
		return SMFail(error, kSMErrorServiceUnavailable, ipcErrno, @"Unable to reach launchd");
	if (result == EEXIST)
		return SMFail(error, kSMErrorAlreadyRegistered, result, [NSString stringWithFormat: @"%@ is already registered", label]);
	if (result != 0)
		return SMFail(error, kSMErrorInternalFailure, result, [NSString stringWithFormat: @"launchd refused %@", label]);
	if (error)
		*error = nil;
	return YES;
}

- (BOOL)unregisterAndReturnError:(NSError **)error
{
	if (_kind == SMAppServiceKindMainApp)
		return SMFail(error, kSMErrorJobNotFound, 0, @"The main app is not registered");

	NSDictionary *job = [self _jobWithError: error];
	if (!job)
		return NO;
	NSString *label = [job objectForKey: @LAUNCH_JOBKEY_LABEL];

	int ipcErrno;
	int result = SMLaunchRequest(LAUNCH_KEY_REMOVEJOB, launch_data_new_string([label UTF8String]), &ipcErrno);
	if (result == -1)
		return SMFail(error, kSMErrorServiceUnavailable, ipcErrno, @"Unable to reach launchd");
	if (result == ESRCH)
		return SMFail(error, kSMErrorJobNotFound, result, [NSString stringWithFormat: @"%@ is not registered", label]);
	if (result != 0)
		return SMFail(error, kSMErrorInternalFailure, result, [NSString stringWithFormat: @"launchd could not remove %@", label]);
	if (error)
		*error = nil;
	return YES;
}

- (void)unregisterWithCompletionHandler:(void (^)(NSError *error))handler
{
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		@autoreleasepool {
			NSError *error = nil;
			[self unregisterAndReturnError: &error];
			handler(error);
		}
	});
}

- (SMAppServiceStatus)status
{
	if (_kind == SMAppServiceKindMainApp)
		return SMAppServiceStatusNotRegistered;
	NSDictionary *job = [self _jobWithError: NULL];
	if (!job)
		return SMAppServiceStatusNotFound;
	return SMStatusForJobState(SMJobStateForLabel([job objectForKey: @LAUNCH_JOBKEY_LABEL]));
}

+ (SMAppServiceStatus)statusForLegacyURL:(NSURL *)url
{
	NSString *label = [[NSDictionary dictionaryWithContentsOfURL: url] objectForKey: @LAUNCH_JOBKEY_LABEL];
	if (![label isKindOfClass: [NSString class]])
		return SMAppServiceStatusNotFound;
	return SMStatusForJobState(SMJobStateForLabel(label));
}

+ (void)openSystemSettingsLoginItems
{
	CFURLRef url = CFURLCreateWithString(NULL, CFSTR("x-apple.systempreferences:com.apple.LoginItems-Settings.extension"), NULL);
	OSStatus status = LSOpenCFURLRef(url, NULL);
	CFRelease(url);
	if (status != noErr)
		NSLog(@"SMAppService: unable to open the Login Items settings (LaunchServices error %d)", (int) status);
}

@end
