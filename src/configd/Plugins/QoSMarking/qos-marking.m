/*
 * Copyright (c) 2016-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * Modification History
 *
 * March 1, 2016		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <mach/mach.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/necp.h>
#include <os/overflow.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/sys_domain.h>

#define	SC_LOG_HANDLE	__log_QoSMarking
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>

#import <Foundation/Foundation.h>
#import <NetworkExtension/NEPolicySession.h>
#import <NEHelperClient.h>

// define the QoSMarking.bundle Info.plist key containing [application] bundleIDs to be white-listed
#define kQoSMarkingBundleIdentifiersAppleAudioVideoCallsKey	CFSTR("QoSMarking_AppleAudioVideoCalls_BundleIDs")

// define the QoSMarking.bundle Info.plist key containing paths to be white-listed
#define kQoSMarkingExecutablePathsAppleAudioVideoCallsKey	CFSTR("QoSMarking_AppleAudioVideoCalls_ExecutablePaths")

// define the starting "order" value for any QoS Marking NEPolicy rules
#define QOS_MARKING_PRIORITY_BLOCK_AV_APPS	1000
#define QOS_MARKING_PRIORITY_BLOCK_AV_PATHS	1500
#define QOS_MARKING_PRIORITY_BLOCK_APPS		2000


static CFStringRef	interfacesKey					= NULL;
static NSArray *	qosMarkingAudioVideoCalls_bundleIDs		= nil;
static NSArray *	qosMarkingAudioVideoCalls_executablePaths	= nil;


#pragma mark -
#pragma mark Logging


__private_extern__ os_log_t
__log_QoSMarking(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.SystemConfiguration", "QoSMarking");
	}

	return log;
}


#pragma mark -
#pragma mark QoSMarking support (system)


static void
qosMarkingSetPolicyRestriction(const char *ctl, BOOL yn)
{
	int	restricted	= yn ? 1 : 0;
	int	ret;

	ret = sysctlbyname(ctl, NULL, 0, &restricted, sizeof(restricted));
	if (ret != -1) {
		SC_log(LOG_NOTICE, "QoS marking policy: sysctl %s=%d", ctl, restricted);
	} else {
		if (errno != ENOENT) {
			SC_log(LOG_ERR, "sysctlbyname() failed: %s", strerror(errno));
		}
	}

	return;
}


static void
qosMarkingSetHavePolicies(BOOL havePolicies)
{
	qosMarkingSetPolicyRestriction("net.qos.policy.restricted", havePolicies);
	return;
}


static void
qosMarkingSetRestrictAVApps(BOOL restrictApps)
{
	qosMarkingSetPolicyRestriction("net.qos.policy.restrict_avapps", restrictApps);
	return;
}


#pragma mark -
#pragma mark QoSMarking support (per-interface)


static BOOL
supportsQoSMarking(int s, const char *ifname)
{
	struct ifreq	ifr;

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCGIFTYPE, (caddr_t)&ifr) == -1) {
		SC_log(LOG_NOTICE, "%s: ioctl(SIOCGIFTYPE) failed: %s",
		       ifname,
		       strerror(errno));
		ifr.ifr_type.ift_type = 0;
		ifr.ifr_type.ift_family = IFRTYPE_FAMILY_ANY;
		ifr.ifr_type.ift_subfamily = IFRTYPE_SUBFAMILY_ANY;
	}

#if	!TARGET_OS_IPHONE
	if (ifr.ifr_type.ift_family == IFRTYPE_FAMILY_ETHERNET) {
		return true;
	}
#else	// !TARGET_OS_IPHONE
	if ((ifr.ifr_type.ift_family == IFRTYPE_FAMILY_ETHERNET) &&
	    (ifr.ifr_type.ift_subfamily == IFRTYPE_SUBFAMILY_WIFI)) {
		return true;
	}
#endif	// !TARGET_OS_IPHONE

	return false;
}


static void
qosMarkingSetEnabled(int s, const char *ifname, BOOL enabled)
{
	struct ifreq	ifr;
	int		ret;

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_qosmarking_enabled = enabled ? 1 : 0;
	ret = ioctl(s, SIOCSQOSMARKINGENABLED, &ifr);
	if (ret == -1) {
		SC_log(LOG_ERR, "%s: ioctl(SIOCSQOSMARKINGENABLED) failed: %s",
		       ifname,
		       strerror(errno));
	}

	return;
}


#pragma mark -
#pragma mark QoSMarking Policy support


@interface QoSMarkingController : NSObject

+ (QoSMarkingController *)sharedController;
- (void)setInterfaces:(NSArray *)interfaces;
- (void)setPolicy:(NSDictionary *)policy forInterface:(NSString *)interface;

@end


@interface QoSMarkingController()

/*
 * interfaces
 *   An array of network interface names on the system/device
 */
@property (nonatomic) NSArray *			interfaces;

/*
 * policySessions
 *   A dictionary for the maintaining the QoS marking policies.
 *
 *   Key   : interface [name]
 *   Value : the NEPolicySession* for the interface
 */
@property (nonatomic) NSMutableDictionary *	policySessions;

/*
 * requested
 *   A dictionary for the tracking the QoS marking policies.
 *
 *   Key   : interface [name]
 *   Value : the [requested] NSDictionary* "policy" for the interface
 */
@property (nonatomic) NSMutableDictionary *	requested;

/*
 * enabled, enabledAV
 *   Dictionaries for tracking the "enabled" interfaces with QoS [AV]
 *   marking policies.
 *
 *   Key   : interface [name]
 *   Value : the enabled NSDictionary* "policy" for the interface
 */
@property (nonatomic) NSMutableDictionary *	enabled;
@property (nonatomic) NSMutableDictionary *	enabledAV;

@end


@implementation QoSMarkingController

- (NEPolicySession *)createPolicySession
{
	return [[NEPolicySession alloc] init];
}


#pragma mark -


- (BOOL)qosMarkingPolicyEnabled:(NSDictionary *)policy forKey:(NSString *)key
{
	NSNumber *	enabled;

	enabled = policy[key];
	if (enabled != nil) {
		if (![enabled isKindOfClass:[NSNumber class]]) {
			SC_log(LOG_ERR, "%@ not valid", key);
			return false;
		}
	} else {
		// assume "enabled" if no key
		return true;
	}

	return enabled.boolValue;
}


- (BOOL)qosMarkingIsEnabled:(NSDictionary *)policy
{
	return [self qosMarkingPolicyEnabled:policy
				      forKey:(NSString *)kSCPropNetQoSMarkingEnabled];
}


- (BOOL)qosMarkingIsAppleAudioVideoCallsEnabled:(NSDictionary *)policy
{
	return [self qosMarkingPolicyEnabled:policy
				      forKey:(NSString *)kSCPropNetQoSMarkingAppleAudioVideoCalls];
}


- (NSArray *)qosMarkingWhitelistedAppIdentifiers:(NSDictionary *)policy
{
	NSArray *	appIDs;

	appIDs = policy[(NSString *)kSCPropNetQoSMarkingWhitelistedAppIdentifiers];
	if ((appIDs != nil) && ![appIDs isKindOfClass:[NSArray class]]) {
		SC_log(LOG_ERR, "QoSMarkingWhitelistedAppIdentifier list not valid");
		return nil;
	}

	for (NSString *appID in appIDs) {
		if ((appID != nil) &&
		    (![appID isKindOfClass:[NSString class]] || (appID.length == 0))) {
			SC_log(LOG_ERR, "QoSMarkingWhitelistedAppIdentifier not valid");
			return nil;
		}
	}

	return appIDs;
}


#pragma mark -


- (NSUUID *)copyUUIDForSingleArch:(int)fd
{
	uint64_t		bytes	= 0;
	struct mach_header	header;
	NSUUID *		uuid	= nil;

	if (read(fd, &header, sizeof(header)) != sizeof(header)) {
		return nil;
	}

	// Go past the 64 bit header if we have a 64 arch
	if (header.magic == MH_MAGIC_64) {
		if (lseek(fd, sizeof(uint32_t), SEEK_CUR) == -1) {
			SC_log(LOG_ERR, "could not lseek() past 64 bit header");
			return nil;
		}
	}

	if(os_mul_overflow((uint64_t)header.ncmds, sizeof(struct load_command), &bytes) ||
	   (bytes > header.sizeofcmds)) {
		SC_log(LOG_ERR, "mach_header error with \".ncmds\" (%llu), \".sizeofcmds\" (%llu)",
		       (uint64_t)header.ncmds,
		       (uint64_t)header.sizeofcmds);
		return nil;
	}

	// Find LC_UUID in the load commands
	for (size_t i = 0; i < header.ncmds; i++) {
		struct load_command	lcmd;

		if (read(fd, &lcmd, sizeof(lcmd)) != sizeof(lcmd)) {
			SC_log(LOG_ERR, "could not read() load_command");
			break;
		}

		if (lcmd.cmd == LC_UUID) {
			struct uuid_command	uuid_cmd;

			if (read(fd, uuid_cmd.uuid, sizeof(uuid_t)) != sizeof(uuid_t)) {
				SC_log(LOG_ERR, "could not read() uuid_command");
				break;
			}

			uuid = [[NSUUID alloc] initWithUUIDBytes:uuid_cmd.uuid];
			break;
		} else {
			if (lseek(fd, lcmd.cmdsize - sizeof(lcmd), SEEK_CUR) == -1) {
				SC_log(LOG_ERR, "could not lseek() past load command");
				return nil;
			}
		}
	}

	return uuid;
}

#define	MAX_NFAT_ARCH	32

- (NSArray *)copyUUIDsForFatBinary:(int)fd
{
	struct fat_arch *	arches		= NULL;
	mach_msg_type_number_t	count;
	struct fat_header	hdr;
	struct host_basic_info	hostinfo;
	kern_return_t		kr;
	NSMutableArray *	uuids		= nil;

	// For a fat architecture, we need find the section that is closet to the host cpu
	memset(&hostinfo, 0, sizeof(hostinfo));
	count = HOST_BASIC_INFO_COUNT;
	kr = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hostinfo, &count);
	if (kr != KERN_SUCCESS) {
		SC_log(LOG_ERR, "host_info() failed: %d", kr);
		return nil;
	}

	if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
		SC_log(LOG_ERR, "could not read() fat_header");
		return nil;
	}

	// Fat header info are always big-endian
	hdr.nfat_arch = OSSwapInt32(hdr.nfat_arch);
	if (hdr.nfat_arch > MAX_NFAT_ARCH) {
		SC_log(LOG_ERR, "fat_header.nfat_arch (%d) > %d", hdr.nfat_arch, MAX_NFAT_ARCH);
		return nil;
	}

	arches = (struct fat_arch *)malloc(hdr.nfat_arch * sizeof(struct fat_arch));
	if (arches == NULL) {
		// if we could not allocate space for architectures
		return nil;
	}

	uuids = [[NSMutableArray alloc] init];

	for (size_t i = 0; i < hdr.nfat_arch; ++i) {
		struct fat_arch	arch;

		if (read(fd, &arch, sizeof(arch)) != sizeof(arch)) {
			SC_log(LOG_ERR, "could not read() fat_arch");
			goto done;
		}
		arch.cputype = (int)OSSwapInt32(arch.cputype);
		arch.offset = OSSwapInt32(arch.offset);
		memcpy(&arches[i], &arch, sizeof(arch));
	}

	for (size_t i = 0; i < hdr.nfat_arch; ++i) {
		struct fat_arch arch;
		NSUUID *	uuid;

		memcpy(&arch, &arches[i], sizeof(arch));

		if (arch.offset == 0) {
			SC_log(LOG_ERR, "invalid offset for arch %d", arch.cputype);
			goto done;
		}

		if (lseek(fd, arch.offset, SEEK_SET) == -1) {
			SC_log(LOG_ERR, "could not lseek() to arch %d", arch.cputype);
			goto done;
		}

		uuid = [self copyUUIDForSingleArch:fd];
		if (uuid == nil) {
			SC_log(LOG_ERR, "could not get uuid for arch %d", arch.cputype);
			goto done;
		}

		if (arch.cputype == hostinfo.cpu_type) {
			[uuids insertObject:uuid atIndex:0];
		} else {
			[uuids addObject:uuid];
		}
	}

    done:

	if (arches != NULL) {
		free(arches);
	}
	if (uuids.count == 0) {
		uuids = nil;
	}
	return uuids;
}

- (NSArray *)copyUUIDsForExecutable:(const char *)executablePath
{
	int		fd;
	uint32_t	magic;
	NSArray *	uuids	= nil;

	if (executablePath == NULL) {
		return nil;
	}

	fd = open(executablePath, O_RDONLY);
	if (fd == -1) {
		if (errno != ENOENT) {
			SC_log(LOG_ERR, "open(\"%s\", O_RDONLY) failed: %s", executablePath, strerror(errno));
		}

		return nil;
	}

	// Read the magic format to decide which path to take
	if (read(fd, &magic, sizeof(magic)) != sizeof(magic)) {
		SC_log(LOG_ERR, "read() no magic format: %s", executablePath);
		goto done;
	}

	// Rewind to the beginning
	lseek(fd, 0, SEEK_SET);
	switch (magic) {
		case FAT_CIGAM: {
			uuids = [self copyUUIDsForFatBinary:fd];
			break;
		}

		case MH_MAGIC:
		case MH_MAGIC_64: {
			NSUUID *	uuid;

			uuid = [self copyUUIDForSingleArch:fd];
			if (uuid == nil) {
				SC_log(LOG_ERR, "%s: failed to get UUID for single arch", __FUNCTION__);
				goto done;
			}

			uuids = @[ uuid ];
			break;
		}

		default: {
			break;
		}
	}

    done:

	close(fd);
	return uuids;
}


- (void)addWhitelistedPathPolicy:(NSString *)interface forPath:(NSString *)path order:(uint32_t)order
{
	NEPolicyCondition *	allInterfacesCondition;
	NEPolicyResult *	result;
	NEPolicyRouteRule *	routeRule;
	NEPolicySession *	session;
	NSArray *		uuids;

	session = _policySessions[interface];
	if (session == nil) {
		SC_log(LOG_ERR, "QoS marking policy: %@: no session", interface);
		return;
	}

	// create QoS route rule
	routeRule = [NEPolicyRouteRule routeRuleWithAction:NEPolicyRouteRuleActionQoSMarking
					  forInterfaceName:interface];
	result = [NEPolicyResult routeRules:@[ routeRule ]];

	// create "all interfaces" condition
	allInterfacesCondition = [NEPolicyCondition allInterfaces];

	uuids = [self copyUUIDsForExecutable:[path UTF8String]];
	if ((uuids == nil) || (uuids.count == 0)) {
		SC_log(LOG_ERR, "QoS marking policy: %@: could not add path \"%@\"",
		       interface,
		       path);
		return;
	}

	for (NSUUID *uuid in uuids) {
		NEPolicy *		policy;
		NSUInteger		policyID;
		NEPolicyCondition *	uuidCondition;

		// create per-app bundleID-->UUID condition
		uuidCondition = [NEPolicyCondition effectiveApplication:uuid];

		// create and add policy
		policy = [[NEPolicy alloc] initWithOrder:order
						  result:result
					      conditions:@[ uuidCondition, allInterfacesCondition ]];
		policyID = [session addPolicy:policy];
		if (policyID != 0) {
			SC_log(LOG_NOTICE, "QoS marking policy: %@: %u: whitelist path \"%@\" (%@)",
			       interface,
			       order,
			       path,
			       uuid.UUIDString);

		} else {
			SC_log(LOG_ERR, "QoS marking policy: %@: could not add whitelist policy for path \"%@\" (%@)",
			       interface,
			       path,
			       uuid.UUIDString);
		}
	}


	return;
}


#pragma mark -


- (NSArray *)copyUUIDsForUUIDMapping:(xpc_object_t)mapping
{
	NSMutableArray *	uuids		= nil;

	if ((mapping != NULL) &&
	    (xpc_get_type(mapping) == XPC_TYPE_ARRAY)) {
		uuids = [NSMutableArray array];

		xpc_array_apply(mapping, ^bool(size_t index, xpc_object_t value) {
#pragma unused(index)
			if ((value != NULL) &&
			    (xpc_get_type(value) == XPC_TYPE_UUID)) {
				NSUUID *	uuid;

				uuid = [[NSUUID alloc] initWithUUIDBytes:xpc_uuid_get_bytes(value)];
				[uuids addObject:uuid];
			}
			return YES;
		});

		if (uuids.count == 0) {
			uuids = nil;
		}
	}

	return uuids;
}


- (NSArray *)copyUUIDsForBundleID:(NSString *)bundleID
{
	NSArray *	uuids;
	xpc_object_t	uuidsFromHelper;

	uuidsFromHelper = NEHelperCacheCopyAppUUIDMapping([bundleID UTF8String], NULL);

	uuids = [self copyUUIDsForUUIDMapping:uuidsFromHelper];
	return uuids;
}


- (void)addWhitelistedAppIdentifierPolicy:(NSString *)interface forApp:(NSString *)appBundleID order:(uint32_t)order
{
	NEPolicyCondition *	allInterfacesCondition;
	NEPolicyResult *	result;
	NEPolicyRouteRule *	routeRule;
	NEPolicySession *	session;
	NSArray *		uuids;

	if ([appBundleID hasPrefix:@"/"]) {
		if (_SC_isAppleInternal()) {
			// special case executable path handling (if internal)
			[self addWhitelistedPathPolicy:interface forPath:appBundleID order:order];
		}

		return;
	}

	session = _policySessions[interface];
	if (session == nil) {
		SC_log(LOG_ERR, "QoS marking policy: %@: no session", interface);
		return;
	}

	// create QoS route rule
	routeRule = [NEPolicyRouteRule routeRuleWithAction:NEPolicyRouteRuleActionQoSMarking
					forInterfaceName:interface];
	result = [NEPolicyResult routeRules:@[ routeRule ]];

	// create "all interfaces" condition
	allInterfacesCondition = [NEPolicyCondition allInterfaces];

	uuids = [self copyUUIDsForBundleID:appBundleID];
	if ((uuids == nil) || (uuids.count == 0)) {
		SC_log(LOG_ERR, "QoS marking policy: %@: could not add bundleID \"%@\"",
		       interface,
		       appBundleID);
		return;
	}

	for (NSUUID *uuid in uuids) {
		NEPolicy *		policy;
		NSUInteger		policyID;
		NEPolicyCondition *	uuidCondition;

		// create per-app bundleID-->UUID condition
		uuidCondition = [NEPolicyCondition effectiveApplication:uuid];

		// create and add policy
		policy = [[NEPolicy alloc] initWithOrder:order
						  result:result
					      conditions:@[ uuidCondition, allInterfacesCondition ]];
		policyID = [session addPolicy:policy];
		if (policyID != 0) {
			SC_log(LOG_NOTICE, "QoS marking policy: %@: %u: whitelist bundleID \"%@\" (%@)",
			       interface,
			       order,
			       appBundleID,
			       uuid.UUIDString);

		} else {
			SC_log(LOG_ERR, "QoS marking policy: %@: could not add whitelist policy for bundleID \"%@\" (%@)",
			       interface,
			       appBundleID,
			       uuid.UUIDString);
		}
	}

	return;
}


#pragma mark -


- (instancetype)init
{
	self = [super init];
	if (self != nil) {
		_interfaces = nil;
		_policySessions = [NSMutableDictionary dictionary];
		_requested = [NSMutableDictionary dictionary];
		_enabled = [NSMutableDictionary dictionary];
		_enabledAV = [NSMutableDictionary dictionary];
	}

	return self;
}


/*

  Have      QoS    Whitelist  AppleAVCalls |       net.qos.policy.       |  Interface   Interface     Interface
 Profile  Enabled   Apps(#)      Enabled   | restricted  restrict_avapps | QoS Enabled  NECP rules  NECP AV rules
 =======  =======  =========  ============ + ==========  =============== + ===========  ==========  =============
1  [N]                                     |     0              0        |     [Y]         [N]          [N]
					   |                             |
2  [Y]      [N]       [0]         [N]      |     0              0        |     [N]         [N]          [N]
3  [Y]      [N]       [0]         [Y]      |     0              0        |     [N]         [N]          [N]
					   |                             |
4  [Y]      [N]      [> 0]        [N]      |     0              0        |     [N]         [N]          [N]
5  [Y]      [N]      [> 0]        [Y]      |     0              0        |     [N]         [N]          [N]
					   |                             |
6  [Y]      [Y]       [0]         [N]      |     1              1        |     [Y]         [N]          [N]
7  [Y]      [Y]       [0]         [Y]      |     1              0        |     [Y]         [N]          [Y]
					   |                             |
8  [Y]      [Y]      [> 0]        [N]      |     1              1        |     [Y]         [Y]          [N]
9  [Y]      [Y]      [> 0]        [Y]      |     1              0        |     [Y]         [Y]          [Y]

  Notes (QoSMarking policy) :
  * If "QoSEnabled" is not present, assume "Y"
  * If "QoSMarkingAppleAudioVideoCalls" is not present, assume "Y"
  * If "QoSMarkingWhitelistedAppIdentifiers" is not present (or empty), assume no whitelisted applications

  Notes (sysctl) :
  * net.qos.policy.restricted should be "1" when NECP policies are present
  * net.qos.policy.restrict_avapps should be "1" when "QoSMarkingAppleAudioVideoCalls" is "N"

 */

- (void)updatePolicy:(NSDictionary *)reqPolicy forInterface:(NSString *)interface
{
	// currently enabled settings
	NSDictionary *	nowPolicy	= _enabled[interface];
	BOOL		nowDisabled	= false;
	BOOL		nowEnabled	= false;
	BOOL		nowAppleAV	= false;

	// requested settings
	BOOL		reqDefault	= false;
	BOOL		reqDisabled	= false;
	BOOL		reqEnabled	= false;
	BOOL		reqAppleAV	= false;

	if (nowPolicy != nil) {
		if ([self qosMarkingIsEnabled:nowPolicy]) {
			// if we have an enabled QoS marking policy
			nowEnabled = true;
		} else {
			// if we have a disabled QoS marking policy
			nowDisabled = true;
		}

		nowAppleAV = [self qosMarkingIsAppleAudioVideoCallsEnabled:nowPolicy];
	}

	if (reqPolicy != nil) {
		if ([self qosMarkingIsEnabled:reqPolicy]) {
			// if QoS marking policy requested
			reqEnabled = true;
		} else {
			// if QoS marking policy present (but disabled)
			reqDisabled = true;
		}

		reqAppleAV = [self qosMarkingIsAppleAudioVideoCallsEnabled:reqPolicy];
	} else {
		reqDefault = true;
	}

	if ((!nowEnabled  && reqDefault ) ||
	    ( nowEnabled  != reqEnabled ) ||
	    ( nowDisabled != reqDisabled) ||
	    ( nowEnabled && reqEnabled && ![nowPolicy isEqual:reqPolicy])) {
		int	s;

		if (reqEnabled) {
			// if we are transitioning to enabled or we have a policy
			// change, ensure that we rebuild policies
			nowPolicy = nil;
		} else {
			if ((nowPolicy != nil) && (reqPolicy == nil)) {
				SC_log(LOG_NOTICE, "QoS marking policy: %@: remove", interface);
			}

			// if QoS marking was enabled (for this interface), close session
			[_policySessions removeObjectForKey:interface];

			// QoS marking policy for this interface is no longer enabled
			[_enabled   removeObjectForKey:interface];
			[_enabledAV removeObjectForKey:interface];
		}

		// update QoSMarking enabled (per-interface)
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s != -1) {
			BOOL	enable	= reqEnabled || reqDefault;

			SC_log(LOG_NOTICE, "QoS marking policy: %@: %s%s",
			       interface,
			       enable ? "enable" : "disable",
			       reqDefault ? " (default)" : "");
			qosMarkingSetEnabled(s, interface.UTF8String, enable);
			close(s);
		} else {
			SC_log(LOG_ERR, "socket() failed: %s", strerror(errno));
		}
	}

	if (reqEnabled) {
		NSArray *	curAppIDs;
		NSArray *	reqAppIDs;
		BOOL		update		= FALSE;

		if (nowAppleAV != reqAppleAV) {
			update = true;
		}

		curAppIDs = [self qosMarkingWhitelistedAppIdentifiers:nowPolicy];
		reqAppIDs = [self qosMarkingWhitelistedAppIdentifiers:reqPolicy];
		if (![curAppIDs isEqual:reqAppIDs]) {
			update = true;
		}

		if (update) {
			BOOL		ok;
			uint32_t	order;
			NEPolicySession *	session;

			// QoS marking being (or still) enabled for this interface
			if (_enabled.count == 0) {
				// if we now have a profile requiring us to check NECP policies
				qosMarkingSetHavePolicies(true);
			}

			// the QoS marking policy for this interface is now enabled
			_enabled[interface] = reqPolicy;

			SC_log(LOG_NOTICE, "QoS marking policy: %@: %s",
			       interface,
			       nowEnabled ? "update" : "add");

			// prepare [new] per-interface NECP session

			session = _policySessions[interface];
			if ((session == nil) && ((reqAppIDs.count > 0) || reqAppleAV)) {
				// if we need to add NECP policies
				session = [self createPolicySession];
				if (session != nil) {
					_policySessions[interface] = session;
				} else {
					SC_log(LOG_ERR, "%@: failed to create policy session", interface);
				}
			}

			// zap any previously stored policies
			if (session != nil) {
				ok = [session removeAllPolicies];
				if (!ok) {
					SC_log(LOG_ERR, "%@: could not remove policies", interface);
				}
			}

			// if needed, add policies for any whitelisted applications
			if ((session != nil) && (reqAppIDs.count > 0)) {
				order = QOS_MARKING_PRIORITY_BLOCK_APPS;
				for (NSString *app in reqAppIDs) {
					[self addWhitelistedAppIdentifierPolicy:interface forApp:app order:order++];
				}
			}

			if (reqAppleAV) {
				if (_enabledAV.count == 0) {
					// if we are enabling the marking of Apple AV application
					// then we do not want to restrict handling of traffic that
					// cannot be handled by NECP
					qosMarkingSetRestrictAVApps(false);
				}

				// the QoS [AV] marking policy for this interface is now enabled
				_enabledAV[interface] = reqPolicy;

				if (session != nil) {
					// if needed, add Apple audio/video application policies

					order = QOS_MARKING_PRIORITY_BLOCK_AV_APPS;
					for (NSString *app in qosMarkingAudioVideoCalls_bundleIDs) {
						[self addWhitelistedAppIdentifierPolicy:interface forApp:app order:order++];
					}

					order = QOS_MARKING_PRIORITY_BLOCK_AV_PATHS;
					for (NSString *path in qosMarkingAudioVideoCalls_executablePaths) {
						[self addWhitelistedPathPolicy:interface forPath:path order:order++];
					}
				}
			} else {
				// the QoS [AV] marking policy for this interface is no longer enabled
				[_enabledAV removeObjectForKey:interface];

				if (_enabledAV.count == 0) {
					// if we do not (no longer want to) be marking AV then restrict
					// handling traffic that cannot be handled by NECP
					qosMarkingSetRestrictAVApps(true);
				}
			}

			if (session != nil) {
				ok = [session apply];
				if (!ok) {
					SC_log(LOG_ERR, "%@: could not apply new policies", interface);
				}
			}
		}
	}

	// Restore "default" state if no policies
	if (_enabled.count == 0) {
		qosMarkingSetRestrictAVApps(false);
		qosMarkingSetHavePolicies(false);
	}
}


#pragma mark -
#pragma mark Update QoSMarking Policy Configuration per [SC] changes


+ (QoSMarkingController *)sharedController
{
	static QoSMarkingController *	controller;
	static dispatch_once_t		once;

	dispatch_once(&once, ^{
		controller = [[QoSMarkingController alloc] init];
	});

	return controller;
}


- (void)setInterfaces:(NSArray *)newInterfaces
{
	NSArray *	curInterfaces;
	int		s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		SC_log(LOG_ERR, "socket() failed: %s", strerror(errno));
		return;
	}

	curInterfaces = _interfaces;
	_interfaces = newInterfaces;

	for (NSString *interface in newInterfaces) {
		if (!supportsQoSMarking(s, interface.UTF8String)) {
			// skip interfaces that do not support QoS marking
			continue;
		}

		if (![curInterfaces containsObject:interface]) {
			NSDictionary *	policy;

			// if new interface
			policy = _requested[interface];
			[_requested removeObjectForKey:interface];	// make this look like a fresh "add"
			[self setPolicy:policy forInterface:interface];	// and "set" the new policy
		}
	}

	close(s);
	return;
}


- (void)setPolicy:(NSDictionary *)policy forInterface:(NSString *)interface
{
	if (policy != nil) {
		if ([_interfaces containsObject:interface]) {
			// set (update) per-interface policy
			[self updatePolicy:policy forInterface:interface];
		}

		// track policy for future changes
		[_requested setObject:policy forKey:interface];
	} else {
		// remove (update) per-interface policy
		[self updatePolicy:policy forInterface:interface];

		// track policy for future changes
		[_requested removeObjectForKey:interface];
	}

	return;
}

@end


#pragma mark -
#pragma mark Update QoS Marking Policy Plugin


/*
 * Function: parse_component
 * Purpose:
 *   Given a string 'key' and a string prefix 'prefix',
 *   return the next component in the slash '/' separated
 *   key.
 *
 * Examples:
 * 1. key = "a/b/c" prefix = "a/"
 *    returns "b"
 * 2. key = "a/b/c" prefix = "a/b/"
 *    returns "c"
 */
static CF_RETURNS_RETAINED CFStringRef
parse_component(CFStringRef key, CFStringRef prefix)
{
	CFMutableStringRef	comp;
	CFRange			range;

	if (!CFStringHasPrefix(key, prefix)) {
		return NULL;
	}
	comp = CFStringCreateMutableCopy(NULL, 0, key);
	CFStringDelete(comp, CFRangeMake(0, CFStringGetLength(prefix)));
	range = CFStringFind(comp, CFSTR("/"), 0);
	if (range.location == kCFNotFound) {
		return comp;
	}
	range.length = CFStringGetLength(comp) - range.location;
	CFStringDelete(comp, range);
	return comp;
}



static void
qosMarkingConfigChangedCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *arg)
{
#pragma unused(arg)
	CFDictionaryRef		changes;
	CFIndex			n;
	static CFStringRef	prefix		= NULL;

	if (prefix == NULL) {
		prefix = SCDynamicStoreKeyCreate(NULL,
						 CFSTR("%@/%@/%@/"),
						 kSCDynamicStoreDomainSetup,
						 kSCCompNetwork,
						 kSCCompInterface);
	}

	changes = SCDynamicStoreCopyMultiple(store, changedKeys, NULL);

	n = CFArrayGetCount(changedKeys);
	for (CFIndex i = 0; i < n; i++) {
		CFStringRef	key;

		key  = CFArrayGetValueAtIndex(changedKeys, i);

		if (CFEqual(key, interfacesKey)) {
			CFDictionaryRef	info;

			info = (changes != NULL) ? CFDictionaryGetValue(changes, key) : NULL;
			if (isA_CFDictionary(info) != NULL) {
				CFArrayRef	interfaces;

				interfaces = CFDictionaryGetValue(info, kSCPropNetInterfaces);
				if (isA_CFArray(interfaces)) {
					@autoreleasepool {
						QoSMarkingController *	controller;

						controller = [QoSMarkingController sharedController];
						[controller setInterfaces:(__bridge NSArray *)interfaces];
					}
				}
			}
		} else {
			CFStringRef	interface;

			interface = parse_component(key, prefix);
			if (interface != NULL) {
				CFDictionaryRef	policy;

				policy = (changes != NULL) ? CFDictionaryGetValue(changes, key) : NULL;
				@autoreleasepool {
					QoSMarkingController *	controller;

					controller = [QoSMarkingController sharedController];
					[controller setPolicy:(__bridge NSDictionary *)policy
						 forInterface:(__bridge NSString *)interface];
				}
				CFRelease(interface);
			}
		}
	}

	if (changes != NULL) {
		CFRelease(changes);
	}

	return;
}


static Boolean
haveNetworkExtensionFramework()
{
	Boolean	haveFramework;

	haveFramework = ([NEPolicy class] != nil);
	return haveFramework;
}


__private_extern__
void
load_QoSMarking(CFBundleRef bundle, Boolean bundleVerbose)
{
#pragma unused(bundleVerbose)
	CFDictionaryRef		dict;
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	CFMutableArrayRef	patterns;
	CFRunLoopSourceRef	rls;
	SCDynamicStoreRef	store;

	SC_log(LOG_DEBUG, "load() called");
	SC_log(LOG_DEBUG, "  bundle ID = %@", CFBundleGetIdentifier(bundle));

	if (!haveNetworkExtensionFramework()) {
		return;
	}

	// initialize a few globals
	interfacesKey = SCDynamicStoreKeyCreateNetworkInterface(NULL,
								kSCDynamicStoreDomainState);

	dict = CFBundleGetInfoDictionary(bundle);
	if (isA_CFDictionary(dict)) {
		CFArrayRef	bundleIDs;
		CFArrayRef	paths;

		bundleIDs = CFDictionaryGetValue(dict, kQoSMarkingBundleIdentifiersAppleAudioVideoCallsKey);
		bundleIDs = isA_CFArray(bundleIDs);
		qosMarkingAudioVideoCalls_bundleIDs = (__bridge NSArray *)bundleIDs;

		paths = CFDictionaryGetValue(dict, kQoSMarkingExecutablePathsAppleAudioVideoCallsKey);
		paths = isA_CFArray(paths);
		qosMarkingAudioVideoCalls_executablePaths = (__bridge NSArray *)paths;
	}

	// open a "configd" store to allow cache updates
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("QoS Marking Configuraton plug-in"),
				     qosMarkingConfigChangedCallback,
				     NULL);
	if (store == NULL) {
		SC_log(LOG_ERR, "SCDynamicStoreCreate() failed: %s", SCErrorString(SCError()));
		goto error;
	}

	// establish notification keys and patterns
	keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// ...watch for a change in the list of network interfaces
	CFArrayAppendValue(keys, interfacesKey);

	// ...watch for (per-interface) QoS marking policy changes
	key = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL,
							    kSCDynamicStoreDomainSetup,
							    kSCCompAnyRegex,
							    kSCEntNetQoSMarkingPolicy);
	CFArrayAppendValue(patterns, key);
	CFRelease(key);

	// register the keys/patterns
	ok = SCDynamicStoreSetNotificationKeys(store, keys, patterns);
	CFRelease(keys);
	CFRelease(patterns);
	if (!ok) {
		SC_log(LOG_NOTICE, "SCDynamicStoreSetNotificationKeys() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (rls == NULL) {
		SC_log(LOG_NOTICE, "SCDynamicStoreCreateRunLoopSource() failed: %s",
		       SCErrorString(SCError()));
		goto error;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

    error :

	if (store != NULL) 	CFRelease(store);
	return;
}


#ifdef	MAIN


#pragma mark -
#pragma mark Standalone test code


int
main(int argc, char **argv)
{
	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	load_QoSMarking(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	CFRunLoopRun();
	// not reached
	exit(0);
	return 0;
}
#endif
