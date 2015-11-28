/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
#include "cache.h"
#include "ev_dlil.h"
#include "ev_ipv4.h"
#include "ev_ipv6.h"
#include <notify.h>

// from ip_fw2.c
#define KEV_LOG_SUBCLASS	10

static const char *inetEventName[] = {
	"",
	"INET address added",
	"INET address changed",
	"INET address deleted",
	"INET destination address changed",
	"INET broadcast address changed",
	"INET netmask changed",
	"INET ARP collision",
	"INET port in use",
};

static const char *dlEventName[] = {
	"",
	"KEV_DL_SIFFLAGS",
	"KEV_DL_SIFMETRICS",
	"KEV_DL_SIFMTU",
	"KEV_DL_SIFPHYS",
	"KEV_DL_SIFMEDIA",
	"KEV_DL_SIFGENERIC",
	"KEV_DL_ADDMULTI",
	"KEV_DL_DELMULTI",
	"KEV_DL_IF_ATTACHED",
	"KEV_DL_IF_DETACHING",
	"KEV_DL_IF_DETACHED",
	"KEV_DL_LINK_OFF",
	"KEV_DL_LINK_ON",
	"KEV_DL_PROTO_ATTACHED",
	"KEV_DL_PROTO_DETACHED",
	"KEV_DL_LINK_ADDRESS_CHANGED",
	"KEV_DL_WAKEFLAGS_CHANGED",
#ifdef	KEV_DL_IF_IDLE_ROUTE_REFCNT
	"KEV_DL_IF_IDLE_ROUTE_REFCNT",
#endif
#ifdef  KEV_DL_IFCAP_CHANGED
	"KEV_DL_IFCAP_CHANGED",
#endif
#ifdef  KEV_DL_LINK_QUALITY_METRIC_CHANGED
	"KEV_DL_LINK_QUALITY_METRIC_CHANGED",
#endif
#ifdef	KEV_DL_NODE_PRESENCE
	"KEV_DL_NODE_PRESENCE"
#endif
#ifdef	KEV_DL_NODE_ABSENCE
	"KEV_DL_NODE_ABSENCE"
#endif
#ifdef	KEV_DL_MASTER_ELECTED
	"KEV_DL_MASTER_ELECTED"
#endif
#ifdef	KEV_DL_ISSUES
	"KEV_DL_ISSUES",
#endif
#ifdef	KEV_DL_IFDELEGATE_CHANGED
	"KEV_DL_IFDELEGATE_CHANGED",
#endif
};

static const char *inet6EventName[] = {
	"",
	"KEV_INET6_NEW_USER_ADDR",
	"KEV_INET6_CHANGED_ADDR",
	"KEV_INET6_ADDR_DELETED",
	"KEV_INET6_NEW_LL_ADDR",
	"KEV_INET6_NEW_RTADV_ADDR",
	"KEV_INET6_DEFROUTER"
};

#ifdef	KEV_ND6_SUBCLASS
static const char *nd6EventNameString[] = {
	"",
	"KEV_ND6_RA"
};
#endif	// KEV_ND6_SUBCLASS

static dispatch_queue_t			S_kev_queue;
static dispatch_source_t		S_kev_source;
__private_extern__ Boolean		network_changed	= FALSE;
__private_extern__ SCDynamicStoreRef	store		= NULL;
__private_extern__ Boolean		_verbose	= FALSE;

__private_extern__
int
dgram_socket(int domain)
{
    return (socket(domain, SOCK_DGRAM, 0));
}

static int
ifflags_set(int s, char * name, short flags)
{
    struct ifreq	ifr;
    int 		ret;

    bzero(&ifr, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    ret = ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr);
    if (ret == -1) {
		return (ret);
    }
    ifr.ifr_flags |= flags;
    return (ioctl(s, SIOCSIFFLAGS, &ifr));
}

static int
ifflags_clear(int s, char * name, short flags)
{
    struct ifreq	ifr;
    int 		ret;

    bzero(&ifr, sizeof(ifr));
    strlcpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    ret = ioctl(s, SIOCGIFFLAGS, (caddr_t)&ifr);
    if (ret == -1) {
		return (ret);
    }
    ifr.ifr_flags &= ~flags;
    return (ioctl(s, SIOCSIFFLAGS, &ifr));
}

static void
mark_if_up(char * name)
{
	int s = dgram_socket(AF_INET);
	if (s == -1)
		return;
	ifflags_set(s, name, IFF_UP);
	close(s);
}

static void
mark_if_down(char * name)
{
	int s = dgram_socket(AF_INET);
	if (s == -1)
		return;
	ifflags_clear(s, name, IFF_UP);
	close(s);
}

static void
post_network_changed(void)
{
	if (network_changed) {
		uint32_t	status;

		status = notify_post(_SC_NOTIFY_NETWORK_CHANGE);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_post() failed: error=%u"), status);
		}

		network_changed = FALSE;
	}

	return;
}

static void
logEvent(CFStringRef evStr, struct kern_event_msg *ev_msg)
{
	int	i;
	int	j;

	if (!_verbose) {
		return;
	}

	SCLog(TRUE, LOG_DEBUG, CFSTR("%@ event:"), evStr);
	SCLog(TRUE, LOG_DEBUG,
	      CFSTR("  Event size=%d, id=%d, vendor=%d, class=%d, subclass=%d, code=%d"),
	      ev_msg->total_size,
	      ev_msg->id,
	      ev_msg->vendor_code,
	      ev_msg->kev_class,
	      ev_msg->kev_subclass,
	      ev_msg->event_code);
	for (i = 0, j = KEV_MSG_HEADER_SIZE; j < ev_msg->total_size; i++, j+=4) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("  Event data[%2d] = %08x"), i, ev_msg->event_data[i]);
	}
}

static const char *
inetEventNameString(uint32_t event_code)
{
	if (event_code < sizeof(inetEventName) / sizeof(inetEventName[0])) {
		return (inetEventName[event_code]);
	}
	return ("New Apple network INET subcode");
}

static const char *
inet6EventNameString(uint32_t event_code)
{
	if (event_code < sizeof(inet6EventName) / sizeof(inet6EventName[0])) {
		return (inet6EventName[event_code]);
	}
	return ("New Apple network INET6 subcode");
}

static const char *
dlEventNameString(uint32_t event_code)
{
	if (event_code < sizeof(dlEventName) / sizeof(dlEventName[0])) {
		return (dlEventName[event_code]);
	}
	return ("New Apple network DL subcode");
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
	const char *			eventName = NULL;
	int				dataLen = (ev_msg->total_size - KEV_MSG_HEADER_SIZE);
	void *				event_data = &ev_msg->event_data[0];
	Boolean				handled = TRUE;
	char				ifr_name[IFNAMSIZ];

	switch (ev_msg->kev_subclass) {
		case KEV_INET_SUBCLASS : {
			eventName = inetEventNameString(ev_msg->event_code);
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
					ipv4_interface_update(NULL, ifr_name);
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
					ipv4_router_arp_alive(ifr_name);
					break;
				}
				default :
					handled = FALSE;
					break;
			}
			break;
		}
		case KEV_INET6_SUBCLASS : {
			struct kev_in6_data * ev;

			eventName = inet6EventNameString(ev_msg->event_code);
			ev = (struct kev_in6_data *)event_data;
			switch (ev_msg->event_code) {
				case KEV_INET6_NEW_USER_ADDR :
				case KEV_INET6_CHANGED_ADDR :
				case KEV_INET6_ADDR_DELETED :
				case KEV_INET6_NEW_LL_ADDR :
				case KEV_INET6_NEW_RTADV_ADDR :
				case KEV_INET6_DEFROUTER :
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(&ev->link_data, ifr_name, sizeof(ifr_name));
					interface_update_ipv6(NULL, ifr_name);
					break;

				default :
					handled = FALSE;
					break;
			}
			break;
		}
		case KEV_DL_SUBCLASS : {
			struct net_event_data * ev;

			eventName = dlEventNameString(ev_msg->event_code);
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
					if (protoEvent->proto_remaining_count == 0) {
						mark_if_down(ifr_name);
					} else {
						mark_if_up(ifr_name);
					}
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
					interface_update_idle_state(ifr_name);
					break;
				}
#endif	// KEV_DL_IF_IDLE_ROUTE_REFCNT

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
					link_update_status(ifr_name, FALSE);
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
					interface_update_quality_metric(ifr_name,
								   lqm_data->link_quality_metric);
					break;
				}
#endif  // KEV_DL_LINK_QUALITY_METRIC_CHANGED

#ifdef	KEV_DL_ISSUES
				case KEV_DL_ISSUES: {
					struct kev_dl_issues *issues;

					issues = (struct kev_dl_issues *)event_data;
					if (dataLen < sizeof(*ev)) {
						handled = FALSE;
						break;
					}
					copy_if_name(ev, ifr_name, sizeof(ifr_name));
					interface_update_link_issues(ifr_name,
								     issues->timestamp,
								     issues->modid,
								     DLIL_MODIDLEN,
								     issues->info,
								     (bcmp(issues->info, info_zero, DLIL_MODIDLEN) != 0)
									?DLIL_MODARGLEN
									:0);
					break;
				}
#endif	// KEV_DL_ISSUES

				case KEV_DL_SIFFLAGS :
				case KEV_DL_SIFMETRICS :
				case KEV_DL_SIFMTU :
				case KEV_DL_SIFPHYS :
				case KEV_DL_SIFMEDIA :
				case KEV_DL_SIFGENERIC :
				case KEV_DL_ADDMULTI :
				case KEV_DL_DELMULTI :
				case KEV_DL_LINK_ADDRESS_CHANGED :
				case KEV_DL_WAKEFLAGS_CHANGED :
#ifdef  KEV_DL_IFCAP_CHANGED
				case KEV_DL_IFCAP_CHANGED :
#endif	// KEV_DL_IFCAP_CHANGED
					break;

				default :
					handled = FALSE;
					break;
			}
			break;
		}
#ifdef	KEV_ND6_SUBCLASS
		case KEV_ND6_SUBCLASS : {
			eventName = nd6EventNameString(ev_msg->event_code);
			switch (ev_msg->event_code) {
				case KEV_KEV_ND6_RA :
					break;

				default :
					handled = FALSE;
					break;
			}
			break;
		}
#endif	// KEV_ND6_SUBCLASS
		case KEV_LOG_SUBCLASS : {
			break;
		}
		default :
			handled = FALSE;
			break;
	}

	if (handled == FALSE) {
		CFStringRef	evStr;

		evStr = CFStringCreateWithCString(NULL,
						  (eventName != NULL) ? eventName : "New Apple network subclass",
						  kCFStringEncodingASCII);
		logEvent(evStr, ev_msg);
		CFRelease(evStr);
	}
	return;
}

static Boolean
eventCallback(int so)
{
	ssize_t			status;
	union {
		char			bytes[1024];
		struct kern_event_msg	ev_msg1;	// first kernel event
	} buf;
	struct kern_event_msg	*ev_msg		= &buf.ev_msg1;
	ssize_t			offset		= 0;

	status = recv(so, &buf, sizeof(buf), 0);
	if (status == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("recv() failed: %s"), strerror(errno));
		return FALSE;
	}

	cache_open();

	while (offset < status) {
		if ((offset + ev_msg->total_size) > status) {
			SCLog(TRUE, LOG_NOTICE, CFSTR("missed SYSPROTO_EVENT event, buffer not big enough"));
			break;
		}

		switch (ev_msg->vendor_code) {
			case KEV_VENDOR_APPLE :
				switch (ev_msg->kev_class) {
					case KEV_NETWORK_CLASS :
						processEvent_Apple_Network(ev_msg);
						break;
					case KEV_IOKIT_CLASS :
					case KEV_SYSTEM_CLASS :
					case KEV_APPLESHARE_CLASS :
					case KEV_FIREWALL_CLASS :
					case KEV_IEEE80211_CLASS :
						break;
					default :
						/* unrecognized (Apple) event class */
						logEvent(CFSTR("New (Apple) class"), ev_msg);
						break;
				}
				break;
			default :
				/* unrecognized vendor code */
				logEvent(CFSTR("New vendor"), ev_msg);
				break;
		}
		offset += ev_msg->total_size;
		ev_msg = (struct kern_event_msg *)(void *)&buf.bytes[offset];
	}

	cache_write(store);
	cache_close();
	post_network_changed();

	return TRUE;
}


static void
prime(void)
{
	struct ifaddrs	*ifap	= NULL;
	struct ifaddrs	*scan;
	int		sock	= -1;

	SCLog(_verbose, LOG_DEBUG, CFSTR("prime() called"));

	cache_open();

	sock = dgram_socket(AF_INET);
	if (sock == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("could not get interface list, socket() failed: %s"), strerror(errno));
		goto done;
	}

	if (getifaddrs(&ifap) == -1) {
		SCLog(TRUE,
		      LOG_ERR,
		      CFSTR("could not get interface info, getifaddrs() failed: %s"),
		      strerror(errno));
		goto done;
	}

	/* update list of interfaces & link status */
	for (scan = ifap; scan != NULL; scan = scan->ifa_next) {
		if (scan->ifa_addr == NULL
		    || scan->ifa_addr->sa_family != AF_LINK) {
			continue;
		}
		/* get the per-interface link/media information */
		link_add(scan->ifa_name);
	}

	/*
	 * update IPv4 network addresses already assigned to
	 * the interfaces.
	 */
	ipv4_interface_update(ifap, NULL);

	/*
	 * update IPv6 network addresses already assigned to
	 * the interfaces.
	 */
	interface_update_ipv6(ifap, NULL);

	freeifaddrs(ifap);

 done:
	if (sock != -1)
		close(sock);

	cache_write(store);
	cache_close();

	network_changed = TRUE;
	post_network_changed();

	/* start handling kernel events */
	dispatch_resume(S_kev_source);

	return;
}


__private_extern__
void
prime_KernelEventMonitor()
{
	dispatch_async(S_kev_queue, ^{ prime(); });
	return;
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

	SCLog(_verbose, LOG_DEBUG, CFSTR("load() called"));
	SCLog(_verbose, LOG_DEBUG, CFSTR("  bundle ID = %@"), CFBundleGetIdentifier(bundle));

	/* open a "configd" session to allow cache updates */
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("Kernel Event Monitor plug-in"),
				     NULL,
				     NULL);
	if (store == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCDnamicStoreCreate() failed: %s"), SCErrorString(SCError()));
		SCLog(TRUE, LOG_ERR, CFSTR("kernel event monitor disabled."));
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
		if (status) {
			SCLog(TRUE, LOG_ERR, CFSTR("could not establish event filter, ioctl() failed: %s"), strerror(errno));
			(void) close(so);
			so = -1;
		}
	} else {
		SCLog(TRUE, LOG_ERR, CFSTR("could not open event socket, socket() failed: %s"), strerror(errno));
	}

	if (so != -1) {
		int	yes = 1;

		status = ioctl(so, FIONBIO, &yes);
		if (status) {
			SCLog(TRUE, LOG_ERR, CFSTR("could not set non-blocking io, ioctl() failed: %s"), strerror(errno));
			(void) close(so);
			so = -1;
		}
	}

	if (so == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("kernel event monitor disabled."));
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
		Boolean	ok;

		ok = eventCallback(so);
		if (!ok) {
			SCLog(TRUE, LOG_ERR, CFSTR("kernel event monitor disabled."));
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
