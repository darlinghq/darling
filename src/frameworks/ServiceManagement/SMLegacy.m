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

// The C API that predates SMAppService: SMJobSubmit, SMJobRemove, SMJobCopyDictionary,
// SMCopyAllJobDictionaries, SMJobBless and SMLoginItemSetEnabled.
//
// Darling runs a single launchd per container, as the user who started the container. Its "system"
// and "user" domains are the same launchd, so both domain constants reach it and the privileged
// helpers' daemon plists in /Library/LaunchDaemons are loaded by it at every container start, like
// the other daemons there; the LaunchAgents directories are loaded at that time as well.
//
// Darling policy: authorization is not enforced. Any AuthorizationRef, including NULL, is accepted,
// and neither SMPrivilegedExecutables nor SMAuthorizedClients is checked as a code requirement.
// What is still checked is structure: the label must be listed in the app's SMPrivilegedExecutables
// and the helper must exist and carry its embedded property lists. macOS reports error codes 2 and 4
// of CFErrorDomainLaunchd for a missing or insufficient authorization; these never occur here.

#import <ServiceManagement/ServiceManagement.h>
#import <CoreServices/CoreServices.h>
#import "SMInternal.h"
#include <errno.h>
#include <signal.h>
#include <libproc.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>

static NSString *const SMLegacyErrorDomain = @"CFErrorDomainLaunchd";

static void SMLegacyFail(CFErrorRef *outError, CFIndex code, NSString *description)
{
	if (!outError)
		return;
	NSDictionary *userInfo = [NSDictionary dictionaryWithObject: description forKey: NSLocalizedDescriptionKey];
	*outError = CFErrorCreate(NULL, (CFStringRef) SMLegacyErrorDomain, code, (CFDictionaryRef) userInfo);
}

// The domain argument of SMJobSubmit, SMJobRemove, SMJobCopyDictionary and SMCopyAllJobDictionaries is
// not interpreted: Darling has one launchd, and macOS treats a domain string it does not know as the
// user domain, so every value reaches the same launchd.

// Waits (bounded) until launchd no longer knows the job.
static void SMWaitForJobGone(NSString *label)
{
	for (int attempt = 0; attempt < 600; attempt++) {
		int ipcErrno;
		if (SMLaunchRequest(LAUNCH_KEY_GETJOB, launch_data_new_string([label UTF8String]), &ipcErrno) != 0)
			return;
		usleep(50000);
	}
}

// Removes the job; a job launchd does not know counts as removed. *existed tells the two apart.
static int SMRemoveJob(NSString *label, BOOL wait, BOOL *existed, int *ipcErrno)
{
	int result = SMLaunchRequest(LAUNCH_KEY_REMOVEJOB, launch_data_new_string([label UTF8String]), ipcErrno);
	if (existed)
		*existed = result == 0;
	if (result == ESRCH)
		return 0;
	if (result == 0 && wait)
		SMWaitForJobGone(label);
	return result;
}

static BOOL SMSubmitJob(NSDictionary *job, CFErrorRef *outError)
{
	NSString *label = [job objectForKey: @LAUNCH_JOBKEY_LABEL];
	launch_data_t data = SMLaunchDataFromObject(job);
	if (![label isKindOfClass: [NSString class]] || !data) {
		if (data)
			launch_data_free(data);
		// Code 10 (kSMErrorInvalidPlist) for a job without a Label, as observed on macOS.
		SMLegacyFail(outError, kSMErrorInvalidPlist, @"The job dictionary has no Label or holds a value launchd cannot represent");
		return NO;
	}
	int ipcErrno;
	int result = SMLaunchRequest(LAUNCH_KEY_SUBMITJOB, data, &ipcErrno);
	if (result == -1) {
		SMLegacyFail(outError, ipcErrno ?: kSMErrorServiceUnavailable, @"Unable to reach launchd");
		return NO;
	}
	// Submitting a label that is already loaded succeeds, as on macOS; the loaded job is kept.
	if (result != 0 && result != EEXIST) {
		SMLegacyFail(outError, result, [NSString stringWithFormat: @"launchd refused %@: %s", label, strerror(result)]);
		return NO;
	}
	return YES;
}

CFDictionaryRef SMJobCopyDictionary(CFStringRef domain, CFStringRef jobLabel)
{
	int ipcErrno;
	launch_data_t reply = NULL;
	if (SMLaunchRequestWithReply(LAUNCH_KEY_GETJOB, launch_data_new_string([(NSString *) jobLabel UTF8String]), &ipcErrno, &reply) != 0)
		return NULL;
	id object = SMObjectFromLaunchData(reply);
	launch_data_free(reply);
	return [object isKindOfClass: [NSDictionary class]] ? (CFDictionaryRef) [object retain] : NULL;
}

CFArrayRef SMCopyAllJobDictionaries(CFStringRef domain)
{
	int ipcErrno;
	launch_data_t reply = NULL;
	if (SMLaunchRequestWithReply(LAUNCH_KEY_GETJOBS, NULL, &ipcErrno, &reply) != 0)
		return NULL;
	id object = SMObjectFromLaunchData(reply);
	launch_data_free(reply);
	if (![object isKindOfClass: [NSDictionary class]])
		return NULL;
	NSMutableArray *jobs = [[NSMutableArray alloc] init];
	for (id value in [object allValues]) {
		if ([value isKindOfClass: [NSDictionary class]])
			[jobs addObject: value];
	}
	return (CFArrayRef) jobs;
}

Boolean SMJobSubmit(CFStringRef domain, CFDictionaryRef job, AuthorizationRef auth, CFErrorRef *outError)
{
	if (outError)
		*outError = NULL;
	return SMSubmitJob((NSDictionary *) job, outError);
}

Boolean SMJobRemove(CFStringRef domain, CFStringRef jobLabel, AuthorizationRef auth, Boolean wait, CFErrorRef *outError)
{
	if (outError)
		*outError = NULL;
	int ipcErrno;
	int result = SMRemoveJob((NSString *) jobLabel, wait, NULL, &ipcErrno);
	if (result == -1) {
		SMLegacyFail(outError, ipcErrno ?: kSMErrorServiceUnavailable, @"Unable to reach launchd");
		return false;
	}
	if (result != 0) {
		SMLegacyFail(outError, result, [NSString stringWithFormat: @"launchd could not remove %@: %s", jobLabel, strerror(result)]);
		return false;
	}
	return true;
}

static void SMCollectDictionaryEntry(const launch_data_t value, const char *key, void *context)
{
	id item = SMObjectFromLaunchData(value);
	if (item)
		[(NSMutableDictionary *) context setObject: item forKey: [NSString stringWithUTF8String: key]];
}

id SMObjectFromLaunchData(launch_data_t data)
{
	switch (launch_data_get_type(data)) {
		case LAUNCH_DATA_STRING:
			return [NSString stringWithUTF8String: launch_data_get_string(data)];
		case LAUNCH_DATA_INTEGER:
			return [NSNumber numberWithLongLong: launch_data_get_integer(data)];
		case LAUNCH_DATA_REAL:
			return [NSNumber numberWithDouble: launch_data_get_real(data)];
		case LAUNCH_DATA_BOOL:
			return [NSNumber numberWithBool: launch_data_get_bool(data)];
		case LAUNCH_DATA_ERRNO:
			return [NSNumber numberWithInt: launch_data_get_errno(data)];
		case LAUNCH_DATA_OPAQUE:
			return [NSData dataWithBytes: launch_data_get_opaque(data) length: launch_data_get_opaque_size(data)];
		case LAUNCH_DATA_ARRAY: {
			NSMutableArray *array = [NSMutableArray array];
			for (size_t i = 0, count = launch_data_array_get_count(data); i < count; i++) {
				id item = SMObjectFromLaunchData(launch_data_array_get_index(data, i));
				if (item)
					[array addObject: item];
			}
			return array;
		}
		case LAUNCH_DATA_DICTIONARY: {
			NSMutableDictionary *dict = [NSMutableDictionary dictionary];
			launch_data_dict_iterate(data, SMCollectDictionaryEntry, dict);
			return dict;
		}
		default:
			return nil;
	}
}

// ---- SMJobBless ----

// Returns the bytes of segment __TEXT, section sectionName inside one Mach-O image of a slice, or nil.
static NSData *SMSectionOfSlice(NSData *file, uint64_t sliceOffset, uint64_t sliceSize, const char *sectionName)
{
	const uint8_t *base = [file bytes];
	uint64_t length = [file length];
	if (sliceOffset > length || sliceSize > length - sliceOffset || sliceSize < sizeof(struct mach_header))
		return nil;
	const uint8_t *slice = base + sliceOffset;
	uint32_t magic = *(const uint32_t *) slice;
	size_t headerSize;
	uint32_t ncmds, sizeofcmds;
	// Only images in the host's byte order are read; Darling guests are little endian.
	if (magic == MH_MAGIC_64) {
		if (sliceSize < sizeof(struct mach_header_64))
			return nil;
		headerSize = sizeof(struct mach_header_64);
		ncmds = ((const struct mach_header_64 *) slice)->ncmds;
		sizeofcmds = ((const struct mach_header_64 *) slice)->sizeofcmds;
	} else if (magic == MH_MAGIC) {
		headerSize = sizeof(struct mach_header);
		ncmds = ((const struct mach_header *) slice)->ncmds;
		sizeofcmds = ((const struct mach_header *) slice)->sizeofcmds;
	} else {
		return nil;
	}
	if (sizeofcmds > sliceSize - headerSize)
		return nil;

	uint64_t position = headerSize;
	for (uint32_t i = 0; i < ncmds; i++) {
		if (headerSize + sizeofcmds - position < sizeof(struct load_command))
			return nil;
		const struct load_command *command = (const struct load_command *) (slice + position);
		if (command->cmdsize < sizeof(struct load_command) || command->cmdsize > headerSize + sizeofcmds - position)
			return nil;
		if (command->cmd == LC_SEGMENT_64 && magic == MH_MAGIC_64 && command->cmdsize >= sizeof(struct segment_command_64)) {
			const struct segment_command_64 *segment = (const struct segment_command_64 *) command;
			if (strncmp(segment->segname, "__TEXT", sizeof(segment->segname)) == 0 &&
					(uint64_t) segment->nsects * sizeof(struct section_64) <= command->cmdsize - sizeof(*segment)) {
				const struct section_64 *sections = (const struct section_64 *) (segment + 1);
				for (uint32_t s = 0; s < segment->nsects; s++) {
					if (strncmp(sections[s].sectname, sectionName, sizeof(sections[s].sectname)) == 0) {
						if (sections[s].offset > sliceSize || sections[s].size > sliceSize - sections[s].offset)
							return nil;
						return [NSData dataWithBytes: slice + sections[s].offset length: sections[s].size];
					}
				}
			}
		} else if (command->cmd == LC_SEGMENT && magic == MH_MAGIC && command->cmdsize >= sizeof(struct segment_command)) {
			const struct segment_command *segment = (const struct segment_command *) command;
			if (strncmp(segment->segname, "__TEXT", sizeof(segment->segname)) == 0 &&
					(uint64_t) segment->nsects * sizeof(struct section) <= command->cmdsize - sizeof(*segment)) {
				const struct section *sections = (const struct section *) (segment + 1);
				for (uint32_t s = 0; s < segment->nsects; s++) {
					if (strncmp(sections[s].sectname, sectionName, sizeof(sections[s].sectname)) == 0) {
						if (sections[s].offset > sliceSize || sections[s].size > sliceSize - sections[s].offset)
							return nil;
						return [NSData dataWithBytes: slice + sections[s].offset length: sections[s].size];
					}
				}
			}
		}
		position += command->cmdsize;
	}
	return nil;
}

// Reads a __TEXT section of a thin or universal Mach-O executable; the first architecture that has it wins.
static NSData *SMCopyTextSection(NSString *path, const char *sectionName)
{
	NSData *file = [NSData dataWithContentsOfFile: path options: NSDataReadingMappedIfSafe error: NULL];
	if ([file length] < sizeof(uint32_t))
		return nil;
	const uint8_t *bytes = [file bytes];
	uint32_t magic = *(const uint32_t *) bytes;
	if (magic != FAT_CIGAM)
		return SMSectionOfSlice(file, 0, [file length], sectionName);

	// Fat headers are big endian.
	if ([file length] < sizeof(struct fat_header))
		return nil;
	uint32_t count = OSSwapBigToHostInt32(((const struct fat_header *) bytes)->nfat_arch);
	if (count > ([file length] - sizeof(struct fat_header)) / sizeof(struct fat_arch))
		return nil;
	const struct fat_arch *arches = (const struct fat_arch *) (bytes + sizeof(struct fat_header));
	for (uint32_t i = 0; i < count; i++) {
		NSData *section = SMSectionOfSlice(file, OSSwapBigToHostInt32(arches[i].offset), OSSwapBigToHostInt32(arches[i].size), sectionName);
		if (section)
			return section;
	}
	return nil;
}

static NSDictionary *SMPropertyListFromData(NSData *data)
{
	if (!data)
		return nil;
	// Linkers pad the section with NUL bytes.
	const char *bytes = [data bytes];
	NSUInteger length = [data length];
	while (length && bytes[length - 1] == 0)
		length--;
	id plist = [NSPropertyListSerialization propertyListWithData: [NSData dataWithBytes: bytes length: length] options: NSPropertyListImmutable format: NULL error: NULL];
	return [plist isKindOfClass: [NSDictionary class]] ? plist : nil;
}

Boolean SMJobBless(CFStringRef domain, CFStringRef executableLabel, AuthorizationRef auth, CFErrorRef *outError)
{
	if (outError)
		*outError = NULL;
	NSString *label = (NSString *) executableLabel;
	NSFileManager *manager = [NSFileManager defaultManager];

	// Structural refusals. Without an administrator's rights macOS refuses SMJobBless before it looks at
	// the label or the domain (code 4 for a NULL AuthorizationRef, code 2 for a reference without the
	// right), and Darling never enforces authorization, so what macOS reports once authorized could not
	// be observed. The codes below are a choice from SMErrors.h without evidence: a domain other than the
	// system one is an internal failure (2), a label that is not a key of SMPrivilegedExecutables is
	// kSMErrorJobNotFound, and a listed helper missing from the bundle is kSMErrorToolNotValid.
	NSDictionary *listed = [[[NSBundle mainBundle] infoDictionary] objectForKey: @"SMPrivilegedExecutables"];
	NSString *helperPath = [[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"Contents/Library/LaunchServices"] stringByAppendingPathComponent: label];
	if (!domain || !CFEqual(domain, kSMDomainSystemLaunchd)) {
		SMLegacyFail(outError, kSMErrorInternalFailure, @"SMJobBless only installs into the system launchd domain");
		return false;
	}
	if (![label length] || ![listed isKindOfClass: [NSDictionary class]] || ![listed objectForKey: label]) {
		SMLegacyFail(outError, kSMErrorJobNotFound, [NSString stringWithFormat: @"%@ is not a privileged executable of this application", label]);
		return false;
	}
	if (![manager fileExistsAtPath: helperPath]) {
		SMLegacyFail(outError, kSMErrorToolNotValid, [NSString stringWithFormat: @"%@ is missing from the application bundle", helperPath]);
		return false;
	}

	NSDictionary *info = (NSDictionary *) CFBundleCopyInfoDictionaryForURL((CFURLRef) [NSURL fileURLWithPath: helperPath]);
	[info autorelease];
	if (![info isKindOfClass: [NSDictionary class]]) {
		SMLegacyFail(outError, kSMErrorToolNotValid, [NSString stringWithFormat: @"%@ has no embedded Info.plist", helperPath]);
		return false;
	}
	NSDictionary *launchdPlist = SMPropertyListFromData(SMCopyTextSection(helperPath, "__launchd_plist"));
	if (!launchdPlist) {
		SMLegacyFail(outError, kSMErrorJobPlistNotFound, [NSString stringWithFormat: @"%@ has no embedded launchd plist", helperPath]);
		return false;
	}
	if (![[launchdPlist objectForKey: @LAUNCH_JOBKEY_LABEL] isEqual: label]) {
		SMLegacyFail(outError, kSMErrorInvalidPlist, @"The embedded launchd plist's Label differs from the helper's label");
		return false;
	}

	// Install the executable and its daemon plist where a system boot finds them.
	NSString *installedPath = [@"/Library/PrivilegedHelperTools" stringByAppendingPathComponent: label];
	NSString *plistPath = [[@"/Library/LaunchDaemons" stringByAppendingPathComponent: label] stringByAppendingPathExtension: @"plist"];
	NSString *temporaryPath = [installedPath stringByAppendingString: @".new"];
	NSDictionary *directoryAttributes = [NSDictionary dictionaryWithObject: [NSNumber numberWithShort: 0755] forKey: NSFilePosixPermissions];
	NSError *error = nil;
	[manager createDirectoryAtPath: @"/Library/PrivilegedHelperTools" withIntermediateDirectories: YES attributes: directoryAttributes error: NULL];
	[manager createDirectoryAtPath: @"/Library/LaunchDaemons" withIntermediateDirectories: YES attributes: directoryAttributes error: NULL];
	[manager removeItemAtPath: temporaryPath error: NULL];
	if (![manager copyItemAtPath: helperPath toPath: temporaryPath error: &error] ||
			![manager setAttributes: directoryAttributes ofItemAtPath: temporaryPath error: &error] ||
			rename([temporaryPath fileSystemRepresentation], [installedPath fileSystemRepresentation]) != 0) {
		[manager removeItemAtPath: temporaryPath error: NULL];
		SMLegacyFail(outError, kSMErrorInternalFailure, [NSString stringWithFormat: @"Unable to install %@: %@", installedPath, error ? [error localizedDescription] : [NSString stringWithUTF8String: strerror(errno)]]);
		return false;
	}

	NSMutableDictionary *job = [[launchdPlist mutableCopy] autorelease];
	[job setObject: installedPath forKey: @LAUNCH_JOBKEY_PROGRAM];
	if (![job objectForKey: @LAUNCH_JOBKEY_PROGRAMARGUMENTS])
		[job setObject: [NSArray arrayWithObject: installedPath] forKey: @LAUNCH_JOBKEY_PROGRAMARGUMENTS];
	NSData *plistData = [NSPropertyListSerialization dataWithPropertyList: job format: NSPropertyListXMLFormat_v1_0 options: 0 error: &error];
	if (!plistData || ![plistData writeToFile: plistPath options: NSDataWritingAtomic error: &error]) {
		SMLegacyFail(outError, kSMErrorInternalFailure, [NSString stringWithFormat: @"Unable to write %@: %@", plistPath, [error localizedDescription]]);
		return false;
	}

	// An earlier installation of the job is replaced.
	int ipcErrno;
	BOOL replaced;
	int result = SMRemoveJob(label, YES, &replaced, &ipcErrno);
	if (result == -1) {
		SMLegacyFail(outError, ipcErrno ?: kSMErrorServiceUnavailable, @"Unable to reach launchd");
		return false;
	}
	return SMSubmitJob(job, outError);
}

// ---- SMLoginItemSetEnabled ----

// The registration of an enabled login item: a per-user LaunchAgent in ~/Library/LaunchAgents of the
// container's user that opens the helper app. Darling has no login session, but its launchd loads the
// user's LaunchAgents when the container starts, which is when a login item has to start again.
static NSString *SMLoginItemLabel(NSString *identifier)
{
	return [@"org.darlinghq.loginitem." stringByAppendingString: identifier];
}

static NSString *SMLoginItemRecordPath(NSString *identifier)
{
	NSString *directory = [NSHomeDirectory() stringByAppendingPathComponent: @"Library/LaunchAgents"];
	return [directory stringByAppendingPathComponent: [SMLoginItemLabel(identifier) stringByAppendingPathExtension: @"plist"]];
}

static NSBundle *SMFindLoginItem(NSString *identifier)
{
	NSString *directory = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"Contents/Library/LoginItems"];
	for (NSString *entry in [[NSFileManager defaultManager] contentsOfDirectoryAtPath: directory error: NULL]) {
		NSBundle *bundle = [NSBundle bundleWithPath: [directory stringByAppendingPathComponent: entry]];
		if ([[bundle bundleIdentifier] isEqualToString: identifier] && [bundle executablePath])
			return bundle;
	}
	return nil;
}

static void SMTerminateProcessesRunning(NSString *executable)
{
	int bytes = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
	if (bytes <= 0)
		return;
	pid_t *pids = malloc(bytes);
	if (!pids)
		return;
	bytes = proc_listpids(PROC_ALL_PIDS, 0, pids, bytes);
	for (int i = 0; i < bytes / (int) sizeof(pid_t); i++) {
		char path[PROC_PIDPATHINFO_MAXSIZE];
		if (pids[i] > 0 && pids[i] != getpid() && proc_pidpath(pids[i], path, sizeof(path)) > 0 &&
				strcmp(path, [executable fileSystemRepresentation]) == 0)
			kill(pids[i], SIGTERM);
	}
	free(pids);
}

Boolean SMLoginItemSetEnabled(CFStringRef identifier, Boolean enabled)
{
	NSString *name = (NSString *) identifier;
	NSBundle *item = SMFindLoginItem(name);
	if (!item)
		return false;

	NSString *recordPath = SMLoginItemRecordPath(name);
	if (!enabled) {
		[[NSFileManager defaultManager] removeItemAtPath: recordPath error: NULL];
		// An agent loaded when the container started is still known to launchd.
		SMJobRemove(kSMDomainUserLaunchd, (CFStringRef) SMLoginItemLabel(name), NULL, false, NULL);
		SMTerminateProcessesRunning([item executablePath]);
		return true;
	}

	NSDictionary *record = [NSDictionary dictionaryWithObjectsAndKeys:
		SMLoginItemLabel(name), @LAUNCH_JOBKEY_LABEL,
		[NSArray arrayWithObjects: @"/usr/bin/open", @"-g", [item bundlePath], nil], @LAUNCH_JOBKEY_PROGRAMARGUMENTS,
		[NSNumber numberWithBool: YES], @LAUNCH_JOBKEY_RUNATLOAD,
		nil];
	NSData *data = [NSPropertyListSerialization dataWithPropertyList: record format: NSPropertyListXMLFormat_v1_0 options: 0 error: NULL];
	[[NSFileManager defaultManager] createDirectoryAtPath: [recordPath stringByDeletingLastPathComponent] withIntermediateDirectories: YES attributes: nil error: NULL];
	if (![data writeToFile: recordPath options: NSDataWritingAtomic error: NULL])
		return false;

	LSLaunchURLSpec spec = { 0 };
	spec.appURL = (CFURLRef) [NSURL fileURLWithPath: [item bundlePath]];
	spec.launchFlags = kLSLaunchDefaults | kLSLaunchDontSwitch | kLSLaunchAsync;
	return LSOpenFromURLSpec(&spec, NULL) == noErr;
}
