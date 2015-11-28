/*
 * Copyright (c) 2005-2014 Apple Inc. All rights reserved.
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
 * January 15, 2005		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/filio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_types.h>
#include <net/if_var.h>
#include <sys/kern_event.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet6/in6_var.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <TargetConditionals.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <IOKit/pwr_mgt/IOPM.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/pwr_mgt/IOPMLibPrivate.h>

#include <dnsinfo.h>
#include <network_information.h>
#include <notify.h>
#ifndef	TARGET_OS_EMBEDDED
#include <utmpx.h>
#include <utmpx_thread.h>
#endif	// !TARGET_OS_EMBEDDED


/* generic MessageTracer keys */
#define MSGTRACER_KEY_DOMAIN    "com.apple.message.domain"
#define MSGTRACER_KEY_SIG       "com.apple.message.signature"
#define MSGTRACER_KEY_UUID      "com.apple.message.uuid"
#define MSGTRACER_KEY_VALUE1    "com.apple.message.value"


#define MY_ASL_FACILITY		"com.apple.SystemConfiguration.Logger"
#define MY_MSGTRACER_DOMAIN	"com.apple.network.log"


static	asl_object_t	log_msg	= NULL;
static	io_connect_t	power	= MACH_PORT_NULL;
static	Boolean		verbose	= FALSE;


static char *
elapsed()
{
	static char		str[128];
	struct tm		tm_diff;
	struct tm		tm_now;
	struct timeval		tv_diff;
	struct timeval		tv_now;
	static struct timeval	tv_then	= { 0, 0 };

	(void)gettimeofday(&tv_now, NULL);

	(void)localtime_r(&tv_now.tv_sec, &tm_now);

	timersub(&tv_now, &tv_then, &tv_diff);
	(void)localtime_r(&tv_diff.tv_sec, &tm_diff);
#ifdef MAIN
	sprintf(str, "%2d:%02d:%02d.%03d (+%ld.%03d)",
		tm_now.tm_hour,
		tm_now.tm_min,
		tm_now.tm_sec,
		tv_now.tv_usec / 1000,
		tv_diff.tv_sec,
		tv_diff.tv_usec / 1000);
#else
	sprintf(str, ".%03d (+%ld.%03d)",
		tv_now.tv_usec / 1000,
		tv_diff.tv_sec,
		tv_diff.tv_usec / 1000);
#endif

	tv_then = tv_now;
	return str;
}


#pragma mark -
#pragma mark [Network] Kernel Events


static CFStringRef
copyInterfaceFlags(const char *if_name)
{
	const char *		iff_up		= "?  ";
	struct ifreq		ifr;
	const char		*ifm_active	= "?  ";
	int			sock;
	CFStringRef		str		= NULL;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("socket() failed"));
		return NULL;
	}

	bzero((char *)&ifr, sizeof(ifr));
	(void) strlcpy(ifr.ifr_name, if_name, sizeof(ifr.ifr_name));
	if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) == 0) {
		struct ifmediareq	ifm;

		iff_up = (ifr.ifr_flags & IFF_UP) ? "yes" : "no ";

		bzero((char *)&ifm, sizeof(ifm));
		(void) strncpy(ifm.ifm_name, if_name, sizeof(ifm.ifm_name));
		if ((ioctl(sock, SIOCGIFMEDIA, (caddr_t)&ifm) == 0) &&
		    (ifm.ifm_count > 0) &&
		    (ifm.ifm_status & IFM_AVALID)) {
			ifm_active = (ifm.ifm_status & IFM_ACTIVE) ? "yes" : "no ";
		}

		str = CFStringCreateWithFormat(NULL,
					       NULL,
					       CFSTR("\n%-5s: IFF_UP = %s IFM_ACTIVE = %s"),
					       if_name,
					       iff_up,
					       ifm_active);
	}

	(void)close(sock);

	return str;
}


static int
prefixLength(struct sockaddr_in6 *sin6)
{
	register u_int8_t	*name		= &sin6->sin6_addr.s6_addr[0];
	register int		byte;
	register int		bit;
	int			plen		= 0;

	for (byte = 0; byte < sizeof(struct in6_addr); byte++, plen += 8) {
		if (name[byte] != 0xff) {
			break;
		}
	}

	if (byte == sizeof(struct in6_addr)) {
		return plen;
	}

	for (bit = 7; bit != 0; bit--, plen++) {
		if (!(name[byte] & (1 << bit))) {
			break;
		}
	}

	for (; bit != 0; bit--) {
		if (name[byte] & (1 << bit)) {
			return 0;
		}
	}

	byte++;
	for (; byte < sizeof(struct in6_addr); byte++) {
		if (name[byte]) {
			return 0;
		}
	}

	return plen;
}


static void
KernelEvent_notification(CFSocketRef s, CFSocketCallBackType type, CFDataRef address, const void *data, void *info)
{
	int			so		= CFSocketGetNative(s);
	size_t			status;
	union {
		char			bytes[1024];
		struct kern_event_msg	ev_msg1;	// first kernel event
	} buf;
	struct kern_event_msg	*ev_msg		= &buf.ev_msg1;
	size_t			offset		= 0;

	status = recv(so, &buf, sizeof(buf), 0);
	if (status == -1) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("recv() failed: %s"), strerror(errno));
		CFSocketInvalidate(s);
		return;
	}

	while (offset < status) {
		if ((offset + ev_msg->total_size) > status) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("missed SYSPROTO_EVENT event, buffer not big enough"));
			break;
		}

		switch (ev_msg->vendor_code) {
			case KEV_VENDOR_APPLE :
				switch (ev_msg->kev_class) {
					case KEV_NETWORK_CLASS : {
						void	*event_data	= &ev_msg->event_data[0];

						switch (ev_msg->kev_subclass) {
							case KEV_DL_SUBCLASS : {
								struct net_event_data	*ev;
								char			if_name[IFNAMSIZ];

								ev = (struct net_event_data *)event_data;

								snprintf(if_name, IFNAMSIZ, "%s%d",
									 ev->if_name,
									 ev->if_unit);

								switch (ev_msg->event_code) {
									case KEV_DL_IF_ATTACHED : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: attached"),
										      elapsed(),
										      if_name);
										break;
									}
									case KEV_DL_IF_DETACHING  : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: detaching"),
										      elapsed(),
										      if_name);
										break;
									}
									case KEV_DL_IF_DETACHED  : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: detached"),
										      elapsed(),
										      if_name);
										break;
									}
									case KEV_DL_LINK_OFF : {
										CFStringRef	str;

										str = verbose ? copyInterfaceFlags(if_name) : NULL;
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: link down%@"),
										      elapsed(),
										      if_name,
										      str != NULL ? str : CFSTR(""));
										if (str != NULL) CFRelease(str);
										break;
									}
									case KEV_DL_LINK_ON  : {
										CFStringRef	str;

										str = verbose ? copyInterfaceFlags(if_name) : NULL;
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: link up%@"),
										      elapsed(),
										      if_name,
										      str != NULL ? str : CFSTR(""));
										if (str != NULL) CFRelease(str);
										break;
									}
									default :
										break;
								}
								break;
							}
							case KEV_INET_SUBCLASS : {
								char			addr[128];
								struct kev_in_data	*ev;
								char			if_name[IFNAMSIZ];
								char			mask[128];

								ev = (struct kev_in_data *)event_data;

								snprintf(if_name, IFNAMSIZ, "%s%d",
									 ev->link_data.if_name,
									 ev->link_data.if_unit);

								switch (ev_msg->event_code) {
									case KEV_INET_NEW_ADDR :
									case KEV_INET_CHANGED_ADDR :
									case KEV_INET_ADDR_DELETED : {
										struct sockaddr_in	sin;

										bzero(&sin, sizeof(sin));
										sin.sin_len    = sizeof(sin);
										sin.sin_family = AF_INET;
										sin.sin_addr   = ev->ia_addr;
										_SC_sockaddr_to_string((struct sockaddr *)&sin, addr, sizeof(addr));

										bzero(&sin, sizeof(sin));
										sin.sin_len         = sizeof(sin);
										sin.sin_family      = AF_INET;
										sin.sin_addr.s_addr = ntohl(ev->ia_subnetmask);
										_SC_sockaddr_to_string((struct sockaddr *)&sin, mask, sizeof(mask));
										break;
									}
									default :
										break;
								}

								switch (ev_msg->event_code) {
									case KEV_INET_NEW_ADDR : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv4 address added (%s/%s)"),
										      elapsed(),
										      if_name,
										      addr,
										      mask);
										break;
									}
									case KEV_INET_CHANGED_ADDR : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv4 address changed (%s/%s)"),
										      elapsed(),
										      if_name,
										      addr,
										      mask);
										break;
									}
									case KEV_INET_ADDR_DELETED : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv4 address removed (%s/%s)"),
										      elapsed(),
										      if_name,
										      addr,
										      mask);
										break;
									}
									default :
										break;
								}
								break;
							}
							case KEV_INET6_SUBCLASS : {
								char			addr[128];
								struct kev_in6_data	*ev;
								char			if_name[IFNAMSIZ];
								int			plen	= 0;

								ev = (struct kev_in6_data *)event_data;

								snprintf(if_name, IFNAMSIZ, "%s%d",
									 ev->link_data.if_name,
									 ev->link_data.if_unit);

								switch (ev_msg->event_code) {
									case KEV_INET6_NEW_USER_ADDR :
									case KEV_INET6_NEW_LL_ADDR :
									case KEV_INET6_CHANGED_ADDR :
									case KEV_INET6_ADDR_DELETED : {
										_SC_sockaddr_to_string((struct sockaddr *)&ev->ia_addr, addr, sizeof(addr));
										plen = prefixLength(&ev->ia_prefixmask);
										break;
									}
									default :
										break;
								}

								switch (ev_msg->event_code) {
									case KEV_INET6_NEW_USER_ADDR :
									case KEV_INET6_NEW_LL_ADDR   : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv6 address added (%s/%d)"),
										      elapsed(),
										      if_name,
										      addr,
										      plen);
										break;
									}
									case KEV_INET6_CHANGED_ADDR : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv6 address changed (%s/%d)"),
										      elapsed(),
										      if_name,
										      addr,
										      plen);
										break;
									}
									case KEV_INET6_ADDR_DELETED : {
										SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
										      CFSTR("%s kernel event: %s: IPv6 address removed"),
										      elapsed(),
										      if_name);
										break;
									}
									default :
										break;
								}
								break;
							}
							default :
								break;
						}
						break;
					}
					default :
						break;
				}
				break;
			default :
				/* unrecognized vendor code */
				break;
		}
		offset += ev_msg->total_size;
		ev_msg = (struct kern_event_msg *)(void *)&buf.bytes[offset];
	}

	return;
}


static void
add_KernelEvent_notification()
{
	CFSocketRef		es;
	CFSocketContext		es_context	= { 0, NULL, NULL, NULL, NULL };
	struct kev_request	kev_req;
	CFRunLoopSourceRef	rls;
	int			so;
	int			yes = 1;

	/* Open an event socket */
	so = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
	if (so == -1) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("socket() failed"));
		return;
	}

	/* establish filter to return all events */
	kev_req.vendor_code  = KEV_VENDOR_APPLE;
	kev_req.kev_class    = KEV_NETWORK_CLASS;
	kev_req.kev_subclass = KEV_ANY_SUBCLASS;
	if (ioctl(so, SIOCSKEVFILT, &kev_req) == -1) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("ioctl(, SIOCSKEVFILT, ) failed"));
		(void)close(so);
		return;
	}

	if (ioctl(so, FIONBIO, &yes) == -1) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("ioctl(, FIONBIO, ) failed"));
		(void)close(so);
		return;
	}

	/* Create a CFSocketRef for the PF_SYSTEM kernel event socket */
	es = CFSocketCreateWithNative(NULL,
				      so,
				      kCFSocketReadCallBack,
				      KernelEvent_notification,
				      &es_context);

	/* Create and add a run loop source for the event socket */
	rls = CFSocketCreateRunLoopSource(NULL, es, -1);
	CFRelease(es);

	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	return;
}


#pragma mark -
#pragma mark Power Management Events


static void
power_notification(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
{
	switch (messageType) {
		case kIOMessageCanDevicePowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: can device power off?"),
			      elapsed());
			break;
		case kIOMessageDeviceWillPowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: device will power off"),
			      elapsed());
			break;
		case kIOMessageDeviceWillNotPowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: device will not power off"),
			      elapsed());
			break;
		case kIOMessageDeviceHasPoweredOn :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: device has powered on"),
			      elapsed());
			break;
		case kIOMessageCanSystemPowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: can system power off?"),
			      elapsed());
			break;
		case kIOMessageSystemWillPowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will power off"),
			      elapsed());
			break;
		case kIOMessageSystemWillNotPowerOff :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will not power off"),
			      elapsed());
			break;
		case kIOMessageCanSystemSleep :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: can system sleep?"),
			      elapsed());
			/*
			 * Idle sleep is about to kick in, but applications have
			 * a chance to allow sleep (by calling IOAllowPowerChange)
			 * or to prevent sleep (by calling IOCancelPowerChange).
			 */
			IOAllowPowerChange(power, (long)messageArgument);
			break;
		case kIOMessageSystemWillSleep :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will sleep"),
			      elapsed());
			IOAllowPowerChange(power, (long)messageArgument);
			break;
		case kIOMessageSystemWillNotSleep :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will not sleep"),
			      elapsed());
			break;
		case kIOMessageSystemHasPoweredOn :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system has powered on"),
			      elapsed());
			break;
		case kIOMessageSystemWillRestart :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will restart"),
			      elapsed());
			break;
		case kIOMessageSystemWillPowerOn :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: system will power on"),
			      elapsed());
			break;
		default :
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s IORegisterForSystemPower: message=%08lx"),
			      elapsed(),
			      (long unsigned int)messageType);
			break;
	}

	return;
}


static void
add_power_notification()
{
	io_object_t		iterator;
	IONotificationPortRef	notify;

	power = IORegisterForSystemPower(0, &notify, power_notification, &iterator);
	if (power == MACH_PORT_NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("IORegisterForSystemPower() failed"));
		return;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(),
			   IONotificationPortGetRunLoopSource(notify),
			   kCFRunLoopCommonModes);

	return;
}


#ifdef	kIOPMMessageSleepWakeUUIDChange
static void
wake_uuid_notification(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
{
	CFStringRef	wake_uuid	= NULL;

	if (messageType == kIOPMMessageSleepWakeUUIDChange) {
		if (messageArgument == kIOPMMessageSleepWakeUUIDSet) {
			wake_uuid = IORegistryEntryCreateCFProperty(service, CFSTR(kIOPMSleepWakeUUIDKey), NULL, 0);
		}

		if (wake_uuid != NULL) {
			char	uuid[256];

			_SC_cfstring_to_cstring(wake_uuid, uuid, sizeof(uuid), kCFStringEncodingUTF8);
			asl_set(log_msg, MSGTRACER_KEY_DOMAIN, MY_MSGTRACER_DOMAIN);
			asl_set(log_msg, MSGTRACER_KEY_UUID  , uuid);

			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s wake UUID notification: UUID set (%@)"),
			      elapsed(),
			      wake_uuid);

			CFRelease(wake_uuid);
		} else {
			asl_unset(log_msg, MSGTRACER_KEY_DOMAIN);
			asl_unset(log_msg, MSGTRACER_KEY_UUID);

			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s wake UUID notification: UUID not set"),
			      elapsed());
		}
	}

	return;
}


static void
add_wake_uuid_notification()
{
	kern_return_t		kr;
	io_object_t		notification	= IO_OBJECT_NULL;
	IONotificationPortRef	notifyPort;
	io_service_t		service;

	notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
	service = IORegistryEntryFromPath(kIOMasterPortDefault,
					  kIOPowerPlane ":/IOPowerConnection/IOPMrootDomain");
	kr = IOServiceAddInterestNotification(notifyPort,
					      service,
					      kIOGeneralInterest,
					      wake_uuid_notification,
					      NULL,			// refCon
					      &notification);
	if (kr != KERN_SUCCESS) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR,
		      CFSTR("IOServiceAddInterestNotification() failed, kr=0x%x"),
		      kr);
		return;
	}

	CFRunLoopAddSource(CFRunLoopGetCurrent(),
			   IONotificationPortGetRunLoopSource(notifyPort),
			   kCFRunLoopDefaultMode);

	wake_uuid_notification(NULL,
			       service,
			       kIOPMMessageSleepWakeUUIDChange,
			       kIOPMMessageSleepWakeUUIDSet);

	return;
}
#endif	// kIOPMMessageSleepWakeUUIDChange


#pragma mark -
#pragma mark SCDynamicStore "network" Events


static void
NetworkChange_notification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	CFIndex			i;
	CFIndex			nk;
	CFMutableStringRef	str;

	str = CFStringCreateMutable(NULL, 0);
	CFStringAppendFormat(str,
			     NULL,
			     CFSTR("%s SCDynamicStore \"network\" notification"),
			     elapsed());

	nk = CFArrayGetCount(changedKeys);
	for (i = 0; i < nk; i++) {
		CFArrayRef	components;
		CFStringRef	key;
		CFIndex		nc;

		key = CFArrayGetValueAtIndex(changedKeys, i);

		components = CFStringCreateArrayBySeparatingStrings(NULL, key, CFSTR("/"));
		if (components == NULL) {
			CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
			continue;
		}

		nc = CFArrayGetCount(components);
		switch (nc) {
			case 5 : {
				CFStringRef	entity_id;

				entity_id  = CFArrayGetValueAtIndex(components, 4);
				if (CFEqual(entity_id, kSCEntNetLink)) {
					CFDictionaryRef	dict;
					const char	*val	= "?";

					dict = SCDynamicStoreCopyValue(store, key);
					if (dict != NULL) {
						CFBooleanRef	link;

						link = CFDictionaryGetValue(dict, kSCPropNetLinkActive);
						if (link != NULL) {
							val = CFBooleanGetValue(link) ? "up" : "down";
						}

						CFRelease(dict);
					}
					CFStringAppendFormat(str, NULL, CFSTR("\n%@ (%s)"), key, val);
				} else if (CFEqual(entity_id, kSCEntNetIPv4) ||
					   CFEqual(entity_id, kSCEntNetIPv6) ||
					   CFEqual(entity_id, kSCEntNetDNS)) {
					CFDictionaryRef	dict;

					dict = SCDynamicStoreCopyValue(store, key);
					if (dict != NULL) {
						CFStringRef	val;

						val = _SCCopyDescription(dict, NULL);
						CFStringAppendFormat(str, NULL, CFSTR("\n%@ : %@"), key, val);
						CFRelease(val);
						CFRelease(dict);
					} else {
						CFStringAppendFormat(str, NULL, CFSTR("\n%@ : removed"), key);
					}
				} else if (CFEqual(entity_id, kSCEntNetAirPort)) {
					CFDictionaryRef	dict;

					dict = SCDynamicStoreCopyValue(store, key);
					if (dict != NULL) {
						CFStringRef	ssid_str;

						ssid_str = CFDictionaryGetValue(dict, CFSTR("SSID_STR"));
						if (ssid_str != NULL) {
							CFDataRef	bssid;

							bssid = CFDictionaryGetValue(dict, CFSTR("BSSID"));
							CFStringAppendFormat(str, NULL, CFSTR("\n%@ : SSID: %@ BSSID: %s"),
									     key,
									     ssid_str,
									     (bssid != NULL) ? ether_ntoa((struct ether_addr *)CFDataGetBytePtr(bssid)) : "<unknown>");
						} else {
							CFStringAppendFormat(str, NULL, CFSTR("\n%@ : no SSID"), key);
						}
						CFRelease(dict);
					} else {
						CFStringAppendFormat(str, NULL, CFSTR("\n%@ : removed"), key);
					}
				} else if (CFEqual(entity_id, kSCEntNetService)) {
					CFDictionaryRef	dict;
					CFStringRef	rank	= kSCNetworkServicePrimaryRankDefault;

					dict = SCDynamicStoreCopyValue(store, key);
					if ((dict == NULL) ||
					    !CFDictionaryGetValueIfPresent(dict,
									   kSCPropNetServicePrimaryRank,
									   (const void **)&rank)) {
						rank = kSCNetworkServicePrimaryRankDefault;
					}
					CFStringAppendFormat(str, NULL, CFSTR("\n%@ : Rank = %@"), key, rank);
					if (dict != NULL) CFRelease(dict);
				} else {
					CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
				}
				break;
			}

			case 4 : {
				static CFStringRef	rank_setup_prefix	= NULL;
				static CFStringRef	rank_state_prefix	= NULL;

				if (rank_setup_prefix == NULL) {
					rank_setup_prefix = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
													kSCDynamicStoreDomainSetup,
													CFSTR(""),
													NULL);
					rank_state_prefix = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
													kSCDynamicStoreDomainState,
													CFSTR(""),
													NULL);
				}

				if (CFStringHasPrefix(key, rank_setup_prefix) ||
				    CFStringHasPrefix(key, rank_state_prefix)) {
					CFDictionaryRef	dict;
					CFStringRef	rank	= kSCNetworkServicePrimaryRankDefault;

					dict = SCDynamicStoreCopyValue(store, key);
					if ((dict == NULL) ||
					    !CFDictionaryGetValueIfPresent(dict,
									   kSCPropNetServicePrimaryRank,
									   (const void **)&rank)) {
						rank = kSCNetworkServicePrimaryRankDefault;
					}
					CFStringAppendFormat(str, NULL, CFSTR("\n%@ : Rank = %@"), key, rank);
					if (dict != NULL) CFRelease(dict);
				} else {
					CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
				}
				break;
			}

			case 2 :
				if (CFEqual(CFArrayGetValueAtIndex(components, 1),
					    CFSTR(kIOPMSystemPowerCapabilitiesKeySuffix))) {
					CFNumberRef	num;

					num = SCDynamicStoreCopyValue(store, key);
					if (num != NULL) {
						IOPMSystemPowerStateCapabilities	capabilities;

						if (isA_CFNumber(num) &&
						    CFNumberGetValue(num, kCFNumberSInt32Type, &capabilities)) {
							CFStringAppendFormat(str, NULL, CFSTR("\n%@ (0x%x)"), key, capabilities);
						}

						CFRelease(num);
					}
				} else {
					CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
				}
				break;

			default :
				CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
				break;
		}

		CFRelease(components);
	}

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO, CFSTR("%@"), str);
	CFRelease(str);
	return;
}


static void
add_NetworkChange_keys(CFMutableArrayRef	keys,
		       CFMutableArrayRef	patterns,
		       CFStringRef		entity,
		       Boolean			doGlobal,
		       Boolean			doService,
		       Boolean			doInterface)
{
	CFStringRef	key;
	CFStringRef	pattern;

	if (doGlobal) {
		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainSetup, entity);
		CFArrayAppendValue(keys, key);
		CFRelease(key);

		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, entity);
		CFArrayAppendValue(keys, key);
		CFRelease(key);
	}

	if (doService) {
		pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, kSCCompAnyRegex, entity);
		CFArrayAppendValue(patterns, pattern);
		CFRelease(pattern);

		pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, entity);
		CFArrayAppendValue(patterns, pattern);
		CFRelease(pattern);
	}

	if (doInterface) {
		pattern = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, entity);
		CFArrayAppendValue(patterns, pattern);
		CFRelease(pattern);
	}

	return;
}


static void
add_NetworkChange_notification()
{
	CFStringRef		dns_key;
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	CFMutableArrayRef	patterns;
	SCDynamicStoreRef	store;
	CFRunLoopSourceRef	rls;

	store = SCDynamicStoreCreate(NULL, CFSTR("Logger.bundle-NetworkChange"), NetworkChange_notification, NULL);
	if (store == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreate() failed"));
		return;
	}

	keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// Interface list

	key = SCDynamicStoreKeyCreateNetworkInterface(NULL, kSCDynamicStoreDomainState);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	// IPv4

	add_NetworkChange_keys(keys, patterns, kSCEntNetIPv4, TRUE, TRUE, TRUE);

	// IPv6

	add_NetworkChange_keys(keys, patterns, kSCEntNetIPv6, TRUE, TRUE, TRUE);

	// PPP, VPN

	add_NetworkChange_keys(keys, patterns, kSCEntNetPPP,   FALSE, TRUE, TRUE);
	add_NetworkChange_keys(keys, patterns, kSCEntNetVPN,   FALSE, TRUE, TRUE);
	add_NetworkChange_keys(keys, patterns, kSCEntNetL2TP,  FALSE, TRUE, TRUE);
	add_NetworkChange_keys(keys, patterns, kSCEntNetPPTP,  FALSE, TRUE, TRUE);
	add_NetworkChange_keys(keys, patterns, kSCEntNetIPSec, FALSE, TRUE, TRUE);

	// Link

	add_NetworkChange_keys(keys, patterns, kSCEntNetLink, FALSE, FALSE, TRUE);

	// AirPort (e.g. BSSID)

	add_NetworkChange_keys(keys, patterns, kSCEntNetAirPort, FALSE, FALSE, TRUE);

	// DNS

	add_NetworkChange_keys(keys, patterns, kSCEntNetDNS, TRUE, TRUE, TRUE);

	dns_key = CFStringCreateWithCString(NULL,
					    dns_configuration_notify_key(),
					    kCFStringEncodingASCII);
	key = CFStringCreateWithFormat(NULL, NULL, CFSTR("Notify:%@"), dns_key);
	CFRelease(dns_key);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	// Proxies

	key = SCDynamicStoreKeyCreateProxies(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	// Rank

	add_NetworkChange_keys(keys, patterns, NULL, FALSE, TRUE, FALSE);		// per-service
	add_NetworkChange_keys(keys, patterns, kSCEntNetService, FALSE, FALSE, TRUE);	// per-interface

	// ComputerName, LocalHostName

	key = SCDynamicStoreKeyCreateComputerName(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	key = SCDynamicStoreKeyCreateHostNames(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	// Power Management

	key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@%@"),
				      kSCDynamicStoreDomainState,
				      CFSTR(kIOPMSystemPowerCapabilitiesKeySuffix));
	CFArrayAppendValue(keys, key);
	CFRelease(key);


	// Setup monitoring

	ok = SCDynamicStoreSetNotificationKeys(store, keys, patterns);
	CFRelease(keys);
	CFRelease(patterns);
	if (!ok) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
		CFRelease(store);
		return;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(store);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(store);
	return;
}


static void
PrimaryService_notification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	CFDictionaryRef		entity;
	CFStringRef		key;
	static CFStringRef	oldPrimary	= NULL;
	CFStringRef		newPrimary	= NULL;

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
	entity = SCDynamicStoreCopyValue(store, key);
	CFRelease(key);
	if (isA_CFDictionary(entity) &&
	    CFDictionaryGetValueIfPresent(entity,
					  kSCDynamicStorePropNetPrimaryService,
					  (const void **)&newPrimary) &&
	    isA_CFString(newPrimary)) {
		CFRetain(newPrimary);
	} else {
		newPrimary = NULL;
	}

	if (!_SC_CFEqual(oldPrimary, newPrimary)) {
		if (newPrimary != NULL) {
			CFStringRef	newInterface;

			newInterface = CFDictionaryGetValue(entity, kSCDynamicStorePropNetPrimaryInterface);
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s Primary service: %@ (%@)"),
			      elapsed(),
			      newPrimary,
			      newInterface != NULL ? newInterface : CFSTR("?"));
		} else {
			SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
			      CFSTR("%s Primary service: removed"),
			      elapsed());
		}
	}

	if (oldPrimary != NULL) CFRelease(oldPrimary);
	oldPrimary = newPrimary;

	if (entity != NULL)	CFRelease(entity);
	return;
}


static void
add_PrimaryService_notification()
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	SCDynamicStoreRef	store;
	CFRunLoopSourceRef	rls;

	store = SCDynamicStoreCreate(NULL, CFSTR("Logger.bundle-PrimaryService"), PrimaryService_notification, NULL);
	if (store == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreate() failed"));
		return;
	}

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	key  = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
		CFRelease(store);
		return;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(store);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(store);
	return;
}


#pragma mark -
#pragma mark Reachability Events


static void
reachability_notification(SCNetworkReachabilityRef ref, SCNetworkReachabilityFlags flags, void *info)
{
	CFStringRef	hostname	= (CFStringRef)info;

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
	      CFSTR("%s reachability changed: %@: flags=0x%08x"),
	      elapsed(),
	      hostname,
	      flags);
	return;
}


static void
add_reachability_notification(CFArrayRef hosts)
{
	SCNetworkReachabilityContext	context	= { 0, NULL, CFRetain, CFRelease, CFCopyDescription };
	CFIndex				i;
	CFIndex				n;
	SCNetworkReachabilityRef	target;

	struct watch {
		in_addr_t	addr;
		CFStringRef	name;
	} watchAddresses[]	= { { 0,			CFSTR("0.0.0.0")	},
				    { IN_LINKLOCALNETNUM,	CFSTR("169.254.0.0")	},
				    { (u_int32_t)0xe00000fb,	CFSTR("224.0.0.251")	},
				  };

	for (i = 0; i < sizeof(watchAddresses)/sizeof(watchAddresses[0]); i++) {
		struct sockaddr_in		sin;

		bzero(&sin, sizeof(sin));
		sin.sin_len    = sizeof(sin);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(watchAddresses[i].addr);

		target = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&sin);
		if (target == NULL) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilityCreateWithAddress() failed"));
			return;
		}

		context.info = (void *)watchAddresses[i].name;
		if (!SCNetworkReachabilitySetCallback(target, reachability_notification, &context)) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilitySetCallback() failed"));
			CFRelease(target);
			return;
		}

		if (!SCNetworkReachabilityScheduleWithRunLoop(target, CFRunLoopGetCurrent(), kCFRunLoopCommonModes)) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilityScheduleWithRunLoop() failed"));
			CFRelease(target);
			return;
		}

		CFRelease(target);
	}

	n = (hosts != NULL) ? CFArrayGetCount(hosts) : 0;
	for (i = 0; i < n; i++) {
		CFStringRef	host;
		char		*nodename;

		host = CFArrayGetValueAtIndex(hosts, i);
		if (!isA_CFString(host) || (CFStringGetLength(host) == 0)) {
			continue;
		}

		nodename = _SC_cfstring_to_cstring(host, NULL, 0, kCFStringEncodingUTF8);
		target = SCNetworkReachabilityCreateWithName(NULL, nodename);
		CFAllocatorDeallocate(NULL, nodename);
		if (target == NULL) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilityCreateWithName() failed"));
			return;
		}

		context.info = (void *)host;
		if (!SCNetworkReachabilitySetCallback(target, reachability_notification, &context)) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilitySetCallback() failed"));
			CFRelease(target);
			return;
		}

		if (!SCNetworkReachabilityScheduleWithRunLoop(target, CFRunLoopGetCurrent(), kCFRunLoopCommonModes)) {
			SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCNetworkReachabilityScheduleWithRunLoop() failed"));
			CFRelease(target);
			return;
		}

		CFRelease(target);
	}

	return;
}


#pragma mark -
#pragma mark Console User/Information Events


#if	!TARGET_OS_EMBEDDED
static void
console_notification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	gid_t			gid;
	CFArrayRef		info;
	CFMutableStringRef	str	= CFStringCreateMutable(NULL, 0);
	uid_t			uid;
	CFStringRef		user;

	CFStringAppendFormat(str,
			     NULL,
			     CFSTR("%s SCDynamicStore console notification"),
			     elapsed());

	user = SCDynamicStoreCopyConsoleUser(store, &uid, &gid);
	if (user != NULL) {
		CFStringAppendFormat(str, NULL, CFSTR("\nconsole user = %@"), user);
		CFRelease(user);
	} else {
		CFStringAppendFormat(str, NULL, CFSTR("\nno console user"));
	}

	info = SCDynamicStoreCopyConsoleInformation(store);
	if (info != NULL) {
		CFIndex		i;
		CFIndex		n;

		n = CFArrayGetCount(info);
		for (i = 0; i < n; i++) {
			CFDictionaryRef	session;
			CFNumberRef	sessionID;
			CFStringRef	sessionUserName;
			CFBooleanRef	sessionOnConsole;

			session          = CFArrayGetValueAtIndex(info, i);
			sessionID        = CFDictionaryGetValue(session, kSCConsoleSessionID);
			sessionUserName  = CFDictionaryGetValue(session, kSCConsoleSessionUserName);
			sessionOnConsole = CFDictionaryGetValue(session, kSCConsoleSessionOnConsole);

			CFStringAppendFormat(str, NULL, CFSTR("\n%ld : id=%@, user=%@, console=%s"),
					     i,
					     sessionID,
					     sessionUserName  != NULL ? sessionUserName : CFSTR("?"),
					     sessionOnConsole != NULL ? CFBooleanGetValue(sessionOnConsole) ? "yes"  : "no" : "?");
		}

		CFRelease(info);
	}

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO, CFSTR("%@"), str);
	CFRelease(str);
	return;
}


static void
add_console_notification()
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	SCDynamicStoreRef	store;
	CFRunLoopSourceRef	rls;

	store = SCDynamicStoreCreate(NULL, CFSTR("Logger.bundle-console"), console_notification, NULL);
	if (store == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreate() failed"));
		return;
	}

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	key = SCDynamicStoreKeyCreateConsoleUser(NULL);
	CFArrayAppendValue(keys, key);
	CFRelease(key);

	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
		CFRelease(store);
		return;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(store);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(store);
	return;
}
#endif	// !TARGET_OS_EMBEDDED


#pragma mark -
#pragma mark Directory Services Events


//#include <DirectoryServices/DirServicesPriv.h>
#ifndef	kDSStdNotifySearchPolicyChanged
#define	kDSStdNotifySearchPolicyChanged	"com.apple.DirectoryService.NotifyTypeStandard:SearchPolicyChanged"
#endif


#if	!TARGET_OS_EMBEDDED
static void
directoryServices_notification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	CFIndex			i;
	CFIndex			n;
	CFMutableStringRef	str	= CFStringCreateMutable(NULL, 0);

	CFStringAppendFormat(str,
			     NULL,
			     CFSTR("%s SCDynamicStore DirectoryServices notification"),
			     elapsed());

	n = CFArrayGetCount(changedKeys);
	for (i = 0; i < n; i++) {
		CFStringRef	key;

		key = CFArrayGetValueAtIndex(changedKeys, i);
		CFStringAppendFormat(str, NULL, CFSTR("\n%@"), key);
	}

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO, CFSTR("%@"), str);
	CFRelease(str);
	return;
}


static void
add_DirectoryServices_notification()
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	SCDynamicStoreRef	store;
	CFRunLoopSourceRef	rls;

	store = SCDynamicStoreCreate(NULL, CFSTR("Logger.bundle-directoryServices"), directoryServices_notification, NULL);
	if (store == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreate() failed"));
		return;
	}

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	key = CFSTR(kDSStdNotifySearchPolicyChanged);
	CFArrayAppendValue(keys, key);
//	CFRelease(key);

	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
		CFRelease(store);
		return;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(store);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(store);
	return;
}
#endif	// !TARGET_OS_EMBEDDED


#pragma mark -
#pragma mark DNS Configuration Events


static void
dnsinfo_notification(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
		CFSTR("%s dnsinfo notification"),
		elapsed());

	return;
}


static void
add_dnsinfo_notification()
{
	const char		*key;
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	key = dns_configuration_notify_key();
	status = notify_register_mach_port(key, &notify_port, 0, &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("notify_register_mach_port() failed"));
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("Logger/dns_configuration", notify_port, dnsinfo_notification, NULL);
	if (mp == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("CFMachPortCreateWithPort() failed"));
		(void)notify_cancel(notify_token);
		return;
	}

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}


#pragma mark -
#pragma mark Network Information Events


static void
nwi_notification(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
	      CFSTR("%s network_information notification"),
	      elapsed());

	return;
}


static void
add_nwi_notification()
{
	const char		*key;
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	key = nwi_state_get_notify_key();
	status = notify_register_mach_port(key, &notify_port, 0, &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("notify_register_mach_port() failed"));
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("Logger/nwi", notify_port, nwi_notification, NULL);
	if (mp == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("CFMachPortCreateWithPort() failed"));
		(void)notify_cancel(notify_token);
		return;
	}

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}


#pragma mark -
#pragma mark Network Configuration Change Events


static void
network_notification(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
	      CFSTR("%s network_change notification"),
	      elapsed());

	return;
}


static void
add_network_notification()
{
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	status = notify_register_mach_port(_SC_NOTIFY_NETWORK_CHANGE,
					   &notify_port,
					   0,
					   &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("notify_register_mach_port() failed"));
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("Logger/network_change", notify_port, network_notification, NULL);
	if (mp == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("CFMachPortCreateWithPort() failed"));
		(void)notify_cancel(notify_token);
		return;
	}

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}


#pragma mark -
#pragma mark SMB Configuration Events


#define	SMBCONFIGURATION_NOTIFY_KEY	"com.apple.system.SystemConfiguration.smb_configuration"


#if	!TARGET_OS_EMBEDDED
static void
smbconf_notification(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO,
	      CFSTR("%s smb.conf notification"),
	      elapsed());

	return;
}


static void
add_smbconf_notification()
{
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	status = notify_register_mach_port(SMBCONFIGURATION_NOTIFY_KEY,
					   &notify_port,
					   0,
					   &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("notify_register_mach_port() failed"));
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("Logger/smb_configuration", notify_port, smbconf_notification, NULL);
	if (mp == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("CFMachPortCreateWithPort() failed"));
		(void)notify_cancel(notify_token);
		return;
	}

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}
#endif	// !TARGET_OS_EMBEDDED


#pragma mark -
#pragma mark pututxline Events


#ifndef	TARGET_OS_EMBEDDED
static const char *
ut_time(struct utmpx *utmpx)
{
	static char	str[16];
	struct tm	tm;

	(void)localtime_r(&utmpx->ut_tv.tv_sec, &tm);
	snprintf(str, sizeof(str), "%2d:%02d:%02d.%03d",
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec,
		utmpx->ut_tv.tv_usec / 1000);

	return str;
}


static const char *
ut_id(struct utmpx *utmpx)
{
	char		*cp;
	static char	str[16];

	cp = utmpx->ut_id + sizeof(utmpx->ut_id);
	while(--cp >= utmpx->ut_id && isprint(*cp)) {}
	if(cp < utmpx->ut_id) {
		snprintf(str, sizeof(str), "%-4.4s", utmpx->ut_id);
	} else {
		snprintf(str, sizeof(str),
			 "0x%2.2x%2.2x%2.2x%2.2x",
			 utmpx->ut_id[0],
			 utmpx->ut_id[1],
			 utmpx->ut_id[2],
			 utmpx->ut_id[3]);
	}

	return str;
}


static const char *
ut_pid(struct utmpx *utmpx)
{
	static char	pid[16];

	snprintf(pid, sizeof(pid), "%d", utmpx->ut_pid);

	return pid;
}


static void
pututxline_notification(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	CFMutableStringRef	str	= CFStringCreateMutable(NULL, 0);
	struct utmpx		*utmpx;
	utmpx_t			utx;

	CFStringAppendFormat(str,
			     NULL,
			     CFSTR("%s pututxline notification"),
			     elapsed());

	utx = _openutx(NULL);
	while ((utmpx = _getutxent(utx)) != NULL) {
		const char *	entry_id	= NULL;
		const char *	entry_line	= NULL;
		const char *	entry_pid	= NULL;
		const char *	entry_tv	= NULL;
		const char *	entry_type;
		const char *	entry_user	= NULL;
		char		line[128];
		int		n;

		switch (utmpx->ut_type) {
			case BOOT_TIME :	// Time of a system boot.
				entry_type = "Boot";
				entry_tv   = ut_time(utmpx);
				break;
			case DEAD_PROCESS :	// A session leader exited.
				entry_type = "Dead process";
				entry_id   = ut_id  (utmpx);
				entry_pid  = ut_pid (utmpx);
				entry_tv   = ut_time(utmpx);
				break;
			case EMPTY :		// No valid user accounting information.
				continue;
			case INIT_PROCESS :	// A process spawned by init(8).
				entry_type = "Init process";
				entry_id   = ut_id  (utmpx);
				entry_pid  = ut_pid (utmpx);
				entry_tv   = ut_time(utmpx);
				break;
			case LOGIN_PROCESS :	// The session leader of a logged-in user.
				entry_type = "Login";
				entry_id   = ut_id  (utmpx);
				entry_user = utmpx->ut_user;
				entry_pid  = ut_pid (utmpx);
				entry_tv   = ut_time(utmpx);
				break;
			case NEW_TIME :		// Time after system clock change.
				entry_type = "New time";
				entry_tv   = ut_time(utmpx);
				break;
			case OLD_TIME :		// Time before system clock change.
				entry_type = "Old time";
				entry_tv   = ut_time(utmpx);
				break;
			case RUN_LVL :		// Run level.	Provided for compatibility, not used.
				entry_type = "Run level";
				break;
			case USER_PROCESS :	// A user process.
				entry_type = "User Process";
				entry_id   = ut_id  (utmpx);
				entry_user = utmpx->ut_user;
				entry_line = utmpx->ut_line;
				entry_pid  = ut_pid (utmpx);
				entry_tv   = ut_time(utmpx);
				break;
			case SHUTDOWN_TIME :	// Time of system shutdown
				entry_type = "Shutdown time";
				entry_tv   = ut_time(utmpx);
				break;
			default :
				entry_type = "Unknown";
				break;
		}

		snprintf(line, sizeof(line),
			 // type  time    id=0x12345678 pid=12345 user=abcdefgh line
			 "\n%-13s %2s%12s %3s%-10s %4s%-5s %5s%-8s %5s%s",
			 entry_type,
			 entry_tv   != NULL ? "@ "       : "",
			 entry_tv   != NULL ? entry_tv   : "",	// hh:mm:ss.ddd
			 entry_id   != NULL ? "id="      : "",
			 entry_id   != NULL ? entry_id   : "",	// 0x12345678
			 entry_pid  != NULL ? "pid="     : "",
			 entry_pid  != NULL ? entry_pid  : "",	// #####
			 entry_user != NULL ? "user="    : "",
			 entry_user != NULL ? entry_user : "",	// <=256 chars
			 entry_line != NULL ? "line="    : "",
			 entry_line != NULL ? entry_line : ""	// <= 32 chars
			);

		n = (int)strlen(line) - 1;
		while ((n > 0) && (line[n] == ' ')) {
			line[n] = '\0';
			--n;
		}

		CFStringAppendFormat(str, NULL, CFSTR("%s"), line);
	}
	_endutxent(utx);

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO, CFSTR("%@"), str);
	CFRelease(str);
	return;
}


static void
add_pututxline_notification()
{
	CFMachPortRef		mp;
	mach_port_t		notify_port;
	int			notify_token;
	CFRunLoopSourceRef	rls;
	uint32_t		status;

	status = notify_register_mach_port(UTMPX_CHANGE_NOTIFICATION, &notify_port, 0, &notify_token);
	if (status != NOTIFY_STATUS_OK) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("notify_register_mach_port() failed"));
		return;
	}

	mp = _SC_CFMachPortCreateWithPort("Logger/utmpx", notify_port, pututxline_notification, NULL);
	if (mp == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("CFMachPortCreateWithPort() failed"));
		(void)notify_cancel(notify_token);
		return;
	}

	rls = CFMachPortCreateRunLoopSource(NULL, mp, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(mp);
		(void)notify_cancel(notify_token);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(mp);
	return;
}
#endif	// !TARGET_OS_EMBEDDED


#pragma mark -
#pragma mark BackToMyMac Status Events


#ifndef	kDSStdNotifyBTMMStatusChanged
#define	kDSStdNotifyBTMMStatusChanged "State:/Network/BackToMyMac"
#endif


#if	!TARGET_OS_EMBEDDED
static void
BTMM_notification(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
	CFIndex			i;
	CFIndex			n;
	CFMutableStringRef	str	= CFStringCreateMutable(NULL, 0);

	CFStringAppendFormat(str,
			     NULL,
			     CFSTR("%s SCDynamicStore Back to My Mac notification"),
			     elapsed());

	n = CFArrayGetCount(changedKeys);
	for (i = 0; i < n; i++) {
		CFStringRef	key;
		CFDictionaryRef	dict;

		key = CFArrayGetValueAtIndex(changedKeys, i);
		dict = SCDynamicStoreCopyValue(store, key);
		if (dict != NULL) {
			CFStringRef	val;

			val = _SCCopyDescription(dict, NULL);
			CFStringAppendFormat(str, NULL, CFSTR("\n%@ : %@"), key, val);
			CFRelease(val);
			CFRelease(dict);
		} else {
			CFStringAppendFormat(str, NULL, CFSTR("\n%@ : removed"), key);
		}
	}

	SCLOG(NULL, log_msg, ~ASL_LEVEL_INFO, CFSTR("%@"), str);
	CFRelease(str);
	return;
}


static void
add_BTMM_notification()
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			ok;
	SCDynamicStoreRef	store;
	CFRunLoopSourceRef	rls;

	store = SCDynamicStoreCreate(NULL, CFSTR("Logger.bundle-BackToMyMac"), BTMM_notification, NULL);
	if (store == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreate() failed"));
		return;
	}

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	key = CFSTR(kDSStdNotifyBTMMStatusChanged);
	CFArrayAppendValue(keys, key);

	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreSetNotificationKeys() failed"));
		CFRelease(store);
		return;
	}

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, -1);
	if (rls == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCDynamicStoreCreateRunLoopSource() failed"));
		CFRelease(store);
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	CFRelease(store);
	return;
}
#endif	// !TARGET_OS_EMBEDDED


#pragma mark -


static __inline__ Boolean
bValFromDictionary(CFDictionaryRef dict, CFStringRef key)
{
	CFBooleanRef	bVal;
	Boolean		result	= FALSE;

	if ((dict != NULL) &&
	    CFDictionaryGetValueIfPresent(dict, key, (const void **)&bVal) &&
	    isA_CFBoolean(bVal)) {
		result = CFBooleanGetValue(bVal);
	}

	return result;
}


void
load(CFBundleRef bundle, Boolean bundleVerbose)
{
	CFDictionaryRef	config;
	Boolean		log_all;

	verbose = bundleVerbose;

	log_msg = asl_new(ASL_TYPE_MSG);
	asl_set(log_msg, ASL_KEY_FACILITY, MY_ASL_FACILITY);

	elapsed();

	config = CFBundleGetInfoDictionary(bundle);
	config = isA_CFDictionary(config);
	log_all = bValFromDictionary(config, CFSTR("LOG_ALL"));

#ifdef	kIOPMMessageSleepWakeUUIDChange
	if (log_all || bValFromDictionary(config, CFSTR("LOG_IO_WAKEUUID_EVENTS"))) {
		add_wake_uuid_notification();
	}
#endif	// kIOPMMessageSleepWakeUUIDChange

	if (log_all || bValFromDictionary(config, CFSTR("LOG_IO_SYSTEMPOWER_EVENTS"))) {
		add_power_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_NETWORK_KERNEL_EVENTS"))) {
		add_KernelEvent_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_NETWORK_INFORMATION"))) {
		add_nwi_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_NOTIFY_DNS_CONFIGURATION"))) {
		add_dnsinfo_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_NOTIFY_NETWORK_CHANGE"))) {
		add_network_notification();
	}

#if	!TARGET_OS_EMBEDDED
	if (log_all || bValFromDictionary(config, CFSTR("LOG_NOTIFY_SMB_CONFIGURATION"))) {
		add_smbconf_notification();
	}
#endif	// !TARGET_OS_EMBEDDED

#ifndef	TARGET_OS_EMBEDDED
	if (log_all || bValFromDictionary(config, CFSTR("LOG_NOTIFY_UTMPX_CHANGE"))) {
		add_pututxline_notification();
	}
#endif	// !TARGET_OS_EMBEDDED

#if	!TARGET_OS_EMBEDDED
	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_BTMM_CONFIGURATION"))) {
		add_BTMM_notification();
	}
#endif	// !TARGET_OS_EMBEDDED

#if	!TARGET_OS_EMBEDDED
	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_CONSOLEUSER"))) {
		add_console_notification();
	}
#endif	// !TARGET_OS_EMBEDDED

#if	!TARGET_OS_EMBEDDED
	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_DIRECTORYSERVICES_SEARCHPOLICY"))) {
		add_DirectoryServices_notification();
	}
#endif	// !TARGET_OS_EMBEDDED

	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_NETWORKCHANGE"))) {
		add_NetworkChange_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_PRIMARYSERVICE"))) {
		add_PrimaryService_notification();
	}

	if (log_all || bValFromDictionary(config, CFSTR("LOG_SC_REACHABILITY"))) {
		CFArrayRef	hosts	= NULL;

		if ((config == NULL) ||
		    !CFDictionaryGetValueIfPresent(config, CFSTR("LOG_SC_REACHABILITY_HOSTS"), (const void **)&hosts) ||
		    !isA_CFArray(hosts) ||
		    (CFArrayGetCount(hosts) == 0)) {
			hosts = NULL;
		}

		if (verbose) {
			_sc_debug = TRUE;
		}

		add_reachability_notification(hosts);
	}

	return;
}

#ifdef	MAIN

int
main(int argc, char **argv)
{
	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;
	_sc_debug   = TRUE;

	load(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}

#endif	/* MAIN */
