/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * December 3, 2002		Dieter Siegmund <dieter@apple.com>
 * - handle the new KEV_INET_ARPCOLLISION event
 * - format the event into a DynamicStore key
 *     State:/Network/Interface/ifname/IPv4Collision/ip_addr/hw_addr
 *   and send a notification on the key
 *
 * August 8, 2002		Allan Nathanson <ajn@apple.com>
 * - added support for KEV_INET6_xxx events
 *
 * January 6, 2002		Jessica Vazquez <vazquez@apple.com>
 * - added handling for KEV_ATALK_xxx events
 *
 * July 2, 2001			Dieter Siegmund <dieter@apple.com>
 * - added handling for KEV_DL_PROTO_{ATTACHED, DETACHED}
 * - mark an interface up if the number of protocols remaining is not 0,
 *   mark an interface down if the number is zero
 * - allocate socket on demand instead of keeping it open all the time
 *
 * June 23, 2001		Allan Nathanson <ajn@apple.com>
 * - update to public SystemConfiguration.framework APIs
 *
 * May 17, 2001			Allan Nathanson <ajn@apple.com>
 * - add/maintain per-interface address/netmask/destaddr information
 *   in the dynamic store.
 *
 * June 30, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include "eventmon.h"
#include "ev_dlil.h"
#include "ev_ipv4.h"
#include "ev_ipv6.h"
#include <notify.h>
#include <sys/sysctl.h>
#include <sys/kern_event.h>
#include <nw/private.h>
#include <netinet6/nd6.h>

static dispatch_queue_t			S_kev_queue;
static dispatch_source_t		S_kev_source;
__private_extern__ Boolean		network_changed		= FALSE;
__private_extern__ SCDynamicStoreRef	store			= NULL;
__private_extern__ Boolean		_verbose		= FALSE;


__private_extern__ os_log_t
__log_KernelEventMonitor(void)
{
    static os_log_t	log	= NULL;

    if (log == NULL) {
	log = os_log_create("com.apple.SystemConfiguration", "KernelEventMonitor");
    }

    return log;
}


#define MESSAGES_MAX			100
static CFMutableArrayRef		S_messages;
static Boolean				S_messages_modified;

static void
messages_init(void)
{
	S_messages = CFArrayCreateMutable(NULL,
					  0,
					  &kCFTypeArrayCallBacks);
	return;
}

static void
messages_free(void)
{
	if (S_messages != NULL) {
		CFRelease(S_messages);
		S_messages = NULL;
	}
	return;
}

static Boolean
messages_should_add_message(void)
{
	if (S_messages == NULL
	    || CFArrayGetCount(S_messages) >= MESSAGES_MAX) {
		return (FALSE);
	}
	return (TRUE);
}

static void
messages_add_message(CFStringRef message)
{
	if (messages_should_add_message()) {
		CFArrayAppendValue(S_messages, message);
		S_messages_modified = TRUE;
	}
	return;
}

__private_extern__ void
messages_add_msg_with_arg(const char * msg, const char * arg)
{
	if (messages_should_add_message()) {
		CFStringRef	str;

		str = CFStringCreateWithFormat(NULL, NULL,
					       CFSTR("%12.8f: %s %s"),
					       CFAbsoluteTimeGetCurrent(),
					       msg, arg);
		messages_add_message(str);
		CFRelease(str);
	}
	return;
}

static void
messages_post(void)
{
	if (S_messages != NULL && S_messages_modified) {
		SCDynamicStoreSetValue(NULL,
				       CFSTR("Plugin:KernelEventMonitor"),
				       S_messages);
		S_messages_modified = FALSE;
	}
	return;
}

static void
check_interface_link_status(const char * if_name)
{
	if (S_messages == NULL) {
		return; /* we're not in early boot of system */
	}
	link_update_status_if_missing(if_name);
	return;
}

__private_extern__
int
dgram_socket(int domain)
{
    int	s;

    s = socket(domain, SOCK_DGRAM, 0);
    if (s == -1) {
	SC_log(LOG_ERR, "socket() failed: %s", strerror(errno));
    }

    return s;
}

static void
post_network_changed(void)
{
	if (network_changed) {
		uint32_t	status;

		status = notify_post(_SC_NOTIFY_NETWORK_CHANGE);
		if (status != NOTIFY_STATUS_OK) {
			SC_log(LOG_NOTICE, "notify_post() failed: error=%u", status);
		}

		network_changed = FALSE;
	}

	return;
}

static void
logEvent(CFStringRef evStr, struct kern_event_msg *ev_msg)
{
	int	    i;
	uint32_t    j;

	if (!_verbose) {
		return;
	}

	SC_log(LOG_DEBUG, "%@ event:", evStr);
	SC_log(LOG_DEBUG, "  Event size=%d, id=%d, vendor=%d, class=%d, subclass=%d, code=%d",
	      ev_msg->total_size,
	      ev_msg->id,
	      ev_msg->vendor_code,
	      ev_msg->kev_class,
	      ev_msg->kev_subclass,
	      ev_msg->event_code);
	for (i = 0, j = KEV_MSG_HEADER_SIZE; j < ev_msg->total_size; i++, j+=4) {
		SC_log(LOG_DEBUG, "  Event data[%2d] = %08x", i, ev_msg->event_data[i]);
	}
}

static void
copy_if_name(const struct net_event_data * ev, char * ifr_name, int ifr_len)
{
	snprintf(ifr_name, ifr_len, "%s%d", ev->if_name, ev->if_unit);
	return;
}

static uint8_t info_zero[DLIL_MODARGLEN];

static void
processEvent_Apple_Network(struct kern_event_msg *ev_msg)
{
	size_t	    dataLen	= (ev_msg->total_size - KEV_MSG_HEADER_SIZE);
	void *	    event_data	= &ev_msg->event_data[0];
	char	    ifr_name[IFNAMSIZ];
	Boolean	    handled	= TRUE;

	switch (ev_msg->kev_subclass) {
		case KEV_INET_SUBCLASS : {
			switch (ev_msg->event_code) {
				case KEV_INET_NEW_ADDR :
				case KEV_INET_CHANGED_ADDR :
				case KEV_INET_ADDR_DELETED :
				case KEV_INET_SIFDSTADDR :
				case KEV_INET_SIFBRDADDR :
				case KEV_INET_SIFNETMASK : {
					struct kev_in_data * ev;

					ev = (struct kev_in_data *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process IPv4 address change: %s: %d", (char *)ifr_name, ev_msg->event_code);
					ipv4_interface_update(NULL, ifr_name);
					if (ev_msg->event_code
					    != KEV_INET_ADDR_DELETED) {
						check_interface_link_status(ifr_name);
					}
					break;
				}
				case KEV_INET_ARPCOLLISION : {
					struct kev_in_collision * ev;

					ev = (struct kev_in_collision *)event_data;
					if ((dataLen < sizeof(*ev))
					    || (dataLen < (sizeof(*ev) + ev->hw_len))) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process ARP collision: %s", (char *)ifr_name);
					ipv4_arp_collision(ifr_name,
							   ev->ia_ipaddr,
							   ev->hw_len,
							   ev->hw_addr);
					break;
				}
#if	!TARGET_OS_IPHONE
				case KEV_INET_PORTINUSE : {
					struct kev_in_portinuse	* ev;
					ev = (struct kev_in_portinuse *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					SC_log(LOG_INFO, "Process port-in-use: %hu, %u", ev->port, ev->req_pid);
					ipv4_port_in_use(ev->port, ev->req_pid);
					break;
				}
#endif	/* !TARGET_OS_IPHONE */
				case KEV_INET_ARPRTRFAILURE: {
					const struct kev_in_arpfailure * ev;

					ev = (const struct kev_in_arpfailure *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process router ARP failure: %s", (char *)ifr_name);
					ipv4_router_arp_failure(ifr_name);
					break;
				}
				case KEV_INET_ARPRTRALIVE: {
					const struct kev_in_arpalive * ev;

					ev = (const struct kev_in_arpalive *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process router ARP alive: %s", (char *)ifr_name);
					ipv4_router_arp_alive(ifr_name);
					break;
				}
				default :
					break;
			}
			break;
		}

		case KEV_INET6_SUBCLASS : {
			struct kev_in6_data * ev;

			ev = (struct kev_in6_data *)event_data;
			switch (ev_msg->event_code) {
				case KEV_INET6_NEW_USER_ADDR :
				case KEV_INET6_CHANGED_ADDR :
				case KEV_INET6_ADDR_DELETED :
				case KEV_INET6_NEW_LL_ADDR :
				case KEV_INET6_NEW_RTADV_ADDR :
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process IPv6 address change: %s: %d", (char *)ifr_name, ev_msg->event_code);
					interface_update_ipv6(NULL, ifr_name);
					if (ev_msg->event_code == KEV_INET6_NEW_USER_ADDR
					    && (ev->ia6_flags & IN6_IFF_DUPLICATED) != 0) {
						ipv6_duplicated_address(ifr_name,
									&ev->ia_addr.sin6_addr,
									ETHER_ADDR_LEN,
									&ev->ia_mac);
					}
					if (ev_msg->event_code
					    != KEV_INET6_ADDR_DELETED) {
						check_interface_link_status(ifr_name);
					}
					break;

				case KEV_INET6_REQUEST_NAT64_PREFIX :
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process NAT64 prefix request: %s", (char *)ifr_name);
					nat64_prefix_request(ifr_name);
					break;

				default :
					break;
			}
			break;
		}

		case KEV_DL_SUBCLASS : {
			struct net_event_data * ev;

			ev = (struct net_event_data *)event_data;
			switch (ev_msg->event_code) {
				case KEV_DL_IF_ATTACHED :
					/*
					 * new interface added
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface attach: %s", (char *)ifr_name);
					link_add(ifr_name);
					break;

				case KEV_DL_IF_DETACHED :
					/*
					 * interface removed
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface detach: %s", (char *)ifr_name);
					link_remove(ifr_name);
					break;

				case KEV_DL_IF_DETACHING :
					/*
					 * interface detaching
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface detaching: %s", (char *)ifr_name);
					interface_detaching(ifr_name);
					break;

				case KEV_DL_PROTO_ATTACHED :
				case KEV_DL_PROTO_DETACHED : {
					struct kev_dl_proto_data * protoEvent;

					protoEvent = (struct kev_dl_proto_data *)event_data;
					if (dataLen < sizeof(*protoEvent)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&protoEvent->link_data,
						     ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process protocol %s: %s (pf=%d, n=%d)",
						 (ev_msg->event_code == KEV_DL_PROTO_ATTACHED) ? "attach" : "detach",
						 (char *)ifr_name,
						 protoEvent->proto_family,
						 protoEvent->proto_remaining_count);
					break;
				}

#ifdef	KEV_DL_IF_IDLE_ROUTE_REFCNT
				case KEV_DL_IF_IDLE_ROUTE_REFCNT: {
					/*
					 * interface route refcnt idle
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface idle: %s", (char *)ifr_name);
					interface_update_idle_state(ifr_name);
					break;
				}
#endif	// KEV_DL_IF_IDLE_ROUTE_REFCNT

				case KEV_DL_IFDELEGATE_CHANGED: {
					/*
					 * interface delegation changed
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface delegation change: %s", (char *)ifr_name);
					interface_update_delegation(ifr_name);
					break;
				}

				case KEV_DL_LINK_OFF :
				case KEV_DL_LINK_ON :
					/*
					 * update the link status in the store
					 */
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface link %s: %s",
						 (ev_msg->event_code == KEV_DL_LINK_ON) ? "up" : "down",
						 (char *)ifr_name);
					link_update_status(ifr_name, FALSE, FALSE);
					break;

#ifdef  KEV_DL_LINK_QUALITY_METRIC_CHANGED
				case KEV_DL_LINK_QUALITY_METRIC_CHANGED: {
					struct kev_dl_link_quality_metric_data * lqm_data;
					lqm_data = (struct kev_dl_link_quality_metric_data *) event_data;

					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface quality: %s (q=%d)",
						 (char *)ifr_name,
						 lqm_data->link_quality_metric);
					interface_update_quality_metric(ifr_name,
								   lqm_data->link_quality_metric);
					break;
				}
#endif	// KEV_DL_LINK_QUALITY_METRIC_CHANGED

#ifdef	KEV_DL_ISSUES
				case KEV_DL_ISSUES: {
					struct kev_dl_issues *issues;

					issues = (struct kev_dl_issues *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					SC_log(LOG_INFO, "Process interface link issues: %s",
						 (char *)ifr_name);
					interface_update_link_issues(ifr_name,
								     issues->timestamp,
								     issues->modid,
								     DLIL_MODIDLEN,
								     issues->info,
								     (bcmp(issues->info, info_zero, DLIL_MODARGLEN) != 0)
									? DLIL_MODARGLEN
									: 0);
					break;
				}
#endif	// KEV_DL_ISSUES

				default :
					break;
			}
			break;
		}

#ifdef	KEV_NETPOLICY_SUBCLASS
		case KEV_NETPOLICY_SUBCLASS : {
		    break;
		}
#endif	// KEV_NETPOLICY_SUBCLASS

#ifdef	KEV_SOCKET_SUBCLASS
		case KEV_SOCKET_SUBCLASS : {
		    break;
		}
#endif	// KEV_SOCKET_SUBCLASS

#ifdef	KEV_ND6_SUBCLASS
		case KEV_ND6_SUBCLASS : {
			struct kev_nd6_event * ev;

			ev = (struct kev_nd6_event *)event_data;
			switch (ev_msg->event_code) {
			case KEV_ND6_ADDR_DETACHED :
			case KEV_ND6_ADDR_DEPRECATED :
			case KEV_ND6_ADDR_EXPIRED :
				if (dataLen < sizeof(*ev)) {
					handled = FALSE;
					break;
				}
				copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
				SC_log(LOG_INFO, "Process ND6 address change: %s: %d", (char *)ifr_name, ev_msg->event_code);
				interface_update_ipv6(NULL, ifr_name);
				break;
			case KEV_ND6_RTR_EXPIRED :
				if (dataLen < sizeof(*ev)) {
					handled = FALSE;
					break;
				}
				copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
				SC_log(LOG_INFO, "Process IPv6 router expired: %s: %d", (char *)ifr_name, ev_msg->event_code);
				ipv6_router_expired(ifr_name);
				break;
			default :
				break;
			}
			break;
		}
#endif	// KEV_ND6_SUBCLASS

#ifdef	KEV_NECP_SUBCLASS
		case KEV_NECP_SUBCLASS : {
		    break;
		}
#endif	// KEV_NECP_SUBCLASS

#ifdef	KEV_NETAGENT_SUBCLASS
		case KEV_NETAGENT_SUBCLASS : {
		    break;
		}
#endif	// KEV_NETAGENT_SUBCLASS

#ifdef	KEV_LOG_SUBCLASS
		case KEV_LOG_SUBCLASS : {
			break;
		}
#endif	// KEV_LOG_SUBCLASS

#ifdef	KEV_NETEVENT_SUBCLASS
		case KEV_NETEVENT_SUBCLASS : {
			break;
		}
#endif	// KEV_NETEVENT_SUBCLASS

		default :
			break;
	}

	if (!handled) {
		logEvent(CFSTR("Error processing (Apple network subclass)"), ev_msg);
	}
	return;
}

static Boolean
eventCallback(int so)
{
	union {
		char			bytes[1024];
		struct kern_event_msg	ev_msg1;	// first kernel event
	} buf;
	struct kern_event_msg	*ev_msg		= &buf.ev_msg1;
	ssize_t			offset		= 0;
	ssize_t			status;

	status = recv(so, &buf, sizeof(buf), 0);
	if (status == -1) {
		SC_log(LOG_NOTICE, "recv() failed: %s", strerror(errno));
		return FALSE;
	}

	_SCDynamicStoreCacheOpen(store);

	while (offset < status) {
		if ((offset + (ssize_t)ev_msg->total_size) > status) {
			SC_log(LOG_NOTICE, "missed SYSPROTO_EVENT event, buffer not big enough");
			break;
		}

		switch (ev_msg->vendor_code) {
			case KEV_VENDOR_APPLE :
				switch (ev_msg->kev_class) {
					case KEV_NETWORK_CLASS :
						processEvent_Apple_Network(ev_msg);
						break;

					default :
						break;
				}
				break;
			default :
				break;
		}
		offset += ev_msg->total_size;
		ev_msg = (struct kern_event_msg *)(void *)&buf.bytes[offset];
	}

	_SCDynamicStoreCacheCommitChanges(store);
	_SCDynamicStoreCacheClose(store);
	post_network_changed();
	messages_post();

	return TRUE;
}


__private_extern__ void
config_new_interface(const char * ifname)
{
	xpc_object_t if_list;

	if (ifname == NULL) {
		network_config_check_interface_settings(NULL);
		return;
	}
	if_list = xpc_array_create(NULL, 0);
	xpc_array_set_string(if_list, XPC_ARRAY_APPEND, ifname);
	network_config_check_interface_settings(if_list);
	xpc_release(if_list);
	return;
}

static void
update_interfaces(const char * msg, Boolean first_time)
{
	Boolean			added = FALSE;
	struct ifaddrs *	ifap = NULL;
	CFMutableArrayRef	ifList = NULL;
	struct ifaddrs *	scan;

	if (getifaddrs(&ifap) == -1) {
		messages_add_msg_with_arg("getifaddrs", strerror(errno));
		SC_log(LOG_NOTICE, "getifaddrs() failed: %s", strerror(errno));
		goto done;
	}

	/* update list of interfaces & link status */
	ifList = interfaceListCopy();
	for (scan = ifap; scan != NULL; scan = scan->ifa_next) {
		if (scan->ifa_addr == NULL
		    || scan->ifa_addr->sa_family != AF_LINK) {
			continue;
		}
		/* get the per-interface link/media information */
		if (interfaceListAddInterface(ifList, scan->ifa_name)) {
			messages_add_msg_with_arg(msg, scan->ifa_name);
			added = TRUE;
			if (!first_time) {
				config_new_interface(scan->ifa_name);
			}
		}
	}

	/* update the global list if an interface was added */
	if (added) {
		interfaceListUpdate(ifList);
	}
	CFRelease(ifList);

	/* update IPv4/IPv6 addresses that are already assigned */
	if (first_time) {
		ipv4_interface_update(ifap, NULL);
		interface_update_ipv6(ifap, NULL);
	}

	freeifaddrs(ifap);

 done:
	if (first_time) {
		/* tell networkd to get the interface list itself */
		config_new_interface(NULL);
	}
	return;
}

#define TIMER_INTERVAL		(6LL * NSEC_PER_SEC)
#define MAX_TIMER_COUNT		20

static void
check_for_new_interfaces(void * context);

static void
schedule_timer(void)
{
	dispatch_after_f(dispatch_time(DISPATCH_TIME_NOW, TIMER_INTERVAL),
			 S_kev_queue,
			 NULL,
			 check_for_new_interfaces);
	return;
}

static void
check_for_new_interfaces(void * context)
{
#pragma unused(context)
	static int	count;
	char		msg[32];

	count++;

	/* update KEV driven content in case a message got dropped */
	snprintf(msg, sizeof(msg), "update %d (of %d)", count, MAX_TIMER_COUNT);
	_SCDynamicStoreCacheOpen(store);
	update_interfaces(msg, FALSE);
	_SCDynamicStoreCacheCommitChanges(store);
	_SCDynamicStoreCacheClose(store);
	messages_post();

	/* schedule the next timer, if needed */
	if (count < MAX_TIMER_COUNT) {
		schedule_timer();
	}
	else {
		messages_free();
	}

	return;
}

static void
prime(void)
{
	SC_log(LOG_DEBUG, "prime() called");

	_SCDynamicStoreCacheOpen(store);
	messages_init();
	update_interfaces("prime", TRUE);
	_SCDynamicStoreCacheCommitChanges(store);
	_SCDynamicStoreCacheClose(store);

	network_changed = TRUE;
	post_network_changed();
	messages_post();

	/* start handling kernel events */
	dispatch_resume(S_kev_source);

	/* schedule polling timer */
	schedule_timer();

	return;
}


__private_extern__
void
prime_KernelEventMonitor()
{
	dispatch_async(S_kev_queue, ^{ prime(); });
	return;
}

static Boolean
initialize_store(void)
{
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("Kernel Event Monitor plug-in"),
				     NULL,
				     NULL);
	if (store == NULL) {
		SC_log(LOG_ERR, "SCDynamicStoreCreate() failed: %s", SCErrorString(SCError()));
		return (FALSE);
	}
	return (TRUE);
}


__private_extern__
void
load_KernelEventMonitor(CFBundleRef bundle, Boolean bundleVerbose)
{
	struct kev_request	kev_req;
	int			so;
	int			status;

	if (bundleVerbose) {
		_verbose = TRUE;
	}

	SC_log(LOG_DEBUG, "load() called");
	SC_log(LOG_DEBUG, "  bundle ID = %@", CFBundleGetIdentifier(bundle));

	if (!initialize_store()) {
		SC_log(LOG_ERR, "kernel event monitor disabled");
		return;
	}

	/* Open an event socket */
	so = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
	if (so != -1) {
		/* establish filter to return events of interest */
		kev_req.vendor_code  = KEV_VENDOR_APPLE;
		kev_req.kev_class    = KEV_NETWORK_CLASS;
		kev_req.kev_subclass = KEV_ANY_SUBCLASS;
		status = ioctl(so, SIOCSKEVFILT, &kev_req);
		if (status != 0) {
			SC_log(LOG_ERR, "could not establish event filter, ioctl() failed: %s", strerror(errno));
			(void) close(so);
			so = -1;
		}
	} else {
		SC_log(LOG_ERR, "could not open event socket, socket() failed: %s", strerror(errno));
	}

	if (so != -1) {
		int	yes = 1;

		status = ioctl(so, FIONBIO, &yes);
		if (status) {
			SC_log(LOG_ERR, "could not set non-blocking io, ioctl() failed: %s", strerror(errno));
			(void) close(so);
			so = -1;
		}
	}

	if (so == -1) {
		SC_log(LOG_ERR, "kernel event monitor disabled");
		CFRelease(store);
		return;
	}

	S_kev_queue = dispatch_queue_create("com.apple.SystemConfiguration.KernelEventMonitor", NULL);
	S_kev_source
		= dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, so, 0, S_kev_queue);
	dispatch_source_set_cancel_handler(S_kev_source, ^{
		close(so);
	});
	dispatch_source_set_event_handler(S_kev_source, ^{
		Boolean		ok;

		ok = eventCallback(so);
		if (!ok) {
			SC_log(LOG_ERR, "kernel event monitor disabled");
			dispatch_source_cancel(S_kev_source);
		}
	});
	// NOTE: dispatch_resume() will be called in prime()

	return;
}

#ifdef	MAIN

#include "ev_dlil.c"

#define appendAddress	appendAddress_v4
#define getIF		getIF_v4
#define updateStore	updateStore_v4
#include "ev_ipv4.c"
#undef appendAddress
#undef getIF
#undef updateStore

#define appendAddress	appendAddress_v6
#define getIF		getIF_v6
#define updateStore	updateStore_v6
#include "ev_ipv6.c"
#undef appendAddress
#undef getIF
#undef updateStore

int
main(int argc, char **argv)
{
	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	load_KernelEventMonitor(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	prime_KernelEventMonitor();
	dispatch_main();
	/* not reached */
	exit(0);
	return 0;
}
#endif
