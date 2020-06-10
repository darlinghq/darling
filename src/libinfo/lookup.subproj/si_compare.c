//
//  si_compare.c
//  Libinfo
//
//  Copyright (c) 2011-2017 Apple Inc. All rights reserved.
//
//

#include "si_compare.h"

#include <dispatch/dispatch.h>
#include <sys/ioctl.h>
#include <sys/kern_control.h>
#include <sys/socketvar.h>
#include <sys/sys_domain.h>
#include <netinet/in.h>
#include <net/netsrc.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <os/lock.h>
#include <os/log.h>
#include <mach/mach_time.h>
#include <net/ntstat.h>
#include <ifaddrs.h>

enum
{
	kPrefer_Dest1	= 1,
	kPrefer_Dest2	= -1,
	kPrefer_Equal	= 0,
	kPrefer_Dest1_Slightly	= 2,
	kPrefer_Dest2_Slightly	= -2,
};

enum
{
	kLookupFailure = 0,
	kLookupSuccess_Found = 1,
	kLookupSuccess_Created = 2,
};

typedef struct RFC6724Address
{
	union sockaddr_in_4_6		addr;
	uint32_t					flags;
	uint16_t					label;
	uint16_t					precedence;
} RFC6724Address;

typedef struct Destination
{
	RFC6724Address			d_dst;
	RFC6724Address			d_src;
	uint32_t				d_ifindex;
	uint32_t				d_resolved_ifindex;
	uint32_t				d_min_rtt;
	uint32_t				d_connection_attempts;
	uint32_t				d_connection_successes;
	unsigned				d_routable : 1;
	unsigned				d_direct : 1;
	unsigned				d_awdl : 1;
} Destination;

typedef struct DestCacheEntry
{
	uint64_t					dce_time;
	struct DestCacheEntry		*dce_next;
	Destination					dce_entry;
} DestCacheEntry;

typedef struct DestCompareSettings {
	uint32_t rtt_leeway;
	uint32_t rtt_leeway_small;
	int64_t rtt_failure_threshold;
	bool	bypass_stats;
} DestCompareSettings;

static DestCacheEntry	*cache = NULL;
static os_unfair_lock	cache_lock = OS_UNFAIR_LOCK_INIT;
static uint64_t			cache_timeout = 0ULL;
static os_log_t			si_destination_log = OS_LOG_DEFAULT;
static DestCompareSettings	si_compare_settings = {};

#ifndef TCP_RTT_SCALE
#define TCP_RTT_SCALE 32 // see netinet/tcp_var.h
#endif // defined(TCP_RTT_SCALE)

#define SI_DESTINATION_COMPARE_UNLIKELY_BOOL(b) (__builtin_expect(!!((long)(b)), 0L))
#define SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(arg, toReturn) \
	do {																										\
		if (SI_DESTINATION_COMPARE_UNLIKELY_BOOL(!arg)) {														\
			os_log_fault(si_destination_log, "%{public}s " #arg " is NULL", __FUNCTION__);						\
			return toReturn;																					\
		}																										\
	}																											\
	while (0)

static const uint32_t	kFailureAttemptsTolerated = 10;
static const uint32_t	kDefaultRTTLeeway = 100 * TCP_RTT_SCALE;
static const uint32_t	kDefaultRTTLeewaySmall = 10 * TCP_RTT_SCALE;

static void
si_destination_compare_child_has_forked(void)
{
	cache_lock = OS_UNFAIR_LOCK_INIT;
	// Cannot use os_log_t object from parent process in child process.
	si_destination_log = OS_LOG_DEFAULT;
}

static void
si_destination_compare_init_once(void)
{
	mach_timebase_info_data_t	time_base;
	mach_timebase_info(&time_base);

	cache_timeout = NSEC_PER_SEC;
	cache_timeout *= time_base.denom;
	cache_timeout /= time_base.numer;

	si_destination_log = os_log_create("com.apple.network.libinfo", "si_destination_compare");

	si_compare_settings = (DestCompareSettings){
		.rtt_leeway = kDefaultRTTLeeway,
		.rtt_leeway_small = kDefaultRTTLeewaySmall,
		.rtt_failure_threshold = kFailureAttemptsTolerated,
		.bypass_stats = false
	};

	(void)pthread_atfork(NULL, NULL, si_destination_compare_child_has_forked);
}

static void
si_destination_compare_init(void)
{
	static pthread_once_t	cache_init = PTHREAD_ONCE_INIT;
	pthread_once(&cache_init, si_destination_compare_init_once);
}

#pragma mark -- Netsrc --

int
si_destination_create_control_socket(const char *control_name)
{
	// Creat a socket
	const int fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
	if (fd == -1)
	{
		os_log_error(si_destination_log, "socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL) failed: %m");
		return fd;
	}

	// Disable SIGPIPE <rdar://problem/9049030>
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on)) != 0)
	{
		os_log_error(si_destination_log, "setsockopt(%d, SOL_SOCKET, SO_NOSIGPIPE, ...) failed: %m", fd);
		close(fd);
		return -1;
	}

	// Get the control ID for statistics
	struct ctl_info ctl = {
		.ctl_id = 0,
		.ctl_name = { 0 }
	};
	strlcpy(ctl.ctl_name, control_name, sizeof(ctl.ctl_name));
	if (ioctl(fd, CTLIOCGINFO, &ctl) == -1)
	{
		os_log_error(si_destination_log, "ioctl(%d, CTLIOCGINFO, ...) failed: %m", fd);
		close(fd);
		return -1;
	}

	// Connect to the statistics control
	struct sockaddr_ctl sc = {
		.sc_len = sizeof(sc),
		.sc_family = AF_SYSTEM,
		.ss_sysaddr = SYSPROTO_CONTROL,
		.sc_id = ctl.ctl_id,
		.sc_unit = 0,
		.sc_reserved = { 0, 0, 0, 0, 0 }
	};
	if (connect(fd, (struct sockaddr *)&sc, sc.sc_len) != 0)
	{
		os_log_error(si_destination_log, "connect(%d, ...) ctl_id=%u, failed: %m", fd, ctl.ctl_id);
		close(fd);
		return -1;
	}

	// increase the receive socket buffer size
	int rcvbuf = 128 * 1024;
	if (0 != setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf))) {
		os_log_error(si_destination_log, "setsockopt(%d, SOL_SOCKET, SO_RCVBUF, %d) failed: %m",
					 fd, rcvbuf);
	}

	// Set socket to non-blocking operation
	const int flags = fcntl(fd, F_GETFL, 0);
	if (-1 != flags) {
		if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK)) {
			os_log_error(si_destination_log, "fcntl(%d, F_SETFL, %d) failed: %m", fd, flags | O_NONBLOCK);
		}
	} else {
		os_log_error(si_destination_log, "fcntl(%d, F_GETFL, 0) failed: %m", fd);
	}

	return fd;
}

static void
si_destination_fill_netsrc(Destination *d)
{
	static int			netsrc_sockfd = -1;
	static int			version = NETSRC_CURVERS;

	if (netsrc_sockfd < 0) {
		netsrc_sockfd = si_destination_create_control_socket(NETSRC_CTLNAME);
	}
	if (netsrc_sockfd < 0) {
		return;
	}

	// Make sure we have a socket, create one if necessary
	struct netsrc_req	request = {
		.nrq_ver = version,
		.nrq_ifscope = d->d_ifindex,
	};

	if (d->d_dst.addr.sa.sa_len > sizeof(request._usa)) {
		os_log_debug(si_destination_log, "sockaddr is too big");
		return;
	}
	memcpy(&request._usa, &d->d_dst, d->d_dst.addr.sa.sa_len);
	if (send(netsrc_sockfd, &request, sizeof(request), 0) != sizeof(request))
	{
		if (errno == EINVAL && version != NETSRC_VERSION1) {
			// fall back to version 1
			version = NETSRC_VERSION1;
			si_destination_fill_netsrc(d);
		}
		// Expected when there is no route to host
		os_log_error(si_destination_log, "send failed: %m");
		return;
	}

	// Read the response
	if (version == NETSRC_VERSION1) {
#ifdef NETSRC_VERSION2
		struct netsrc_repv1	v1;
#else	// NETSRC_VERSION2
		struct netsrc_rep	v1;
#endif	// NETSRC_VERSION2
		if (recv(netsrc_sockfd, &v1, sizeof(v1), 0) != sizeof(v1)) {
			// Not expected
			os_log_debug(si_destination_log, "recv failed: %m");
			return;
		}
		d->d_dst.label = v1.nrp_dstlabel;
		d->d_dst.precedence = v1.nrp_dstprecedence;
		if (v1.nrq_sin.sin_len <= sizeof(d->d_src.addr)) {
			memcpy( &d->d_src.addr, &v1._usa, v1.nrq_sin.sin_len);
		}
		d->d_src.label = v1.nrp_label;
		d->d_src.precedence = v1.nrp_precedence;
		d->d_src.flags = v1.nrp_flags;
		return;
	}
#ifdef NETSRC_VERSION2
	else if (version == NETSRC_VERSION2) {
		struct netsrc_repv2	v2;
		if (recv(netsrc_sockfd, &v2, sizeof(v2), 0) != sizeof(v2)) {
			// Not expected
			os_log_debug(si_destination_log, "recv failed: %m");
			return;
		}
		d->d_dst.label = v2.nrp_dstlabel;
		d->d_dst.precedence = v2.nrp_dstprecedence;
		d->d_src.addr = v2.nrp_src;
		d->d_src.label = v2.nrp_label;
		d->d_src.precedence = v2.nrp_precedence;
		d->d_src.flags = v2.nrp_flags;
		d->d_resolved_ifindex = v2.nrp_ifindex;
		d->d_min_rtt = v2.nrp_min_rtt;
		d->d_connection_attempts = v2.nrp_connection_attempts;
		d->d_connection_successes = v2.nrp_connection_successes;
		d->d_routable = (v2.nrp_flags & NETSRC_FLAG_ROUTEABLE) ? 1 : 0;
		d->d_direct = (v2.nrp_flags & NETSRC_FLAG_DIRECT) ? 1 : 0;
		d->d_awdl = (v2.nrp_flags & NETSRC_FLAG_AWDL) ? 1 : 0;
		return;
	}
#endif // NETSRC_VERSION2

	return;
}

#pragma mark -- Statistics --

static uint32_t	kLoopbackIndex = 1;

// Only update kLoopbackIndex from the default value of 1 if an entry with the IFF_LOOPBACK flag set is found.
void
set_loopback_ifindex()
{
    struct ifaddrs *ifaddrs, *ifa;

    if (getifaddrs(&ifaddrs) < 0)
        return;

    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr &&
            ifa->ifa_addr->sa_family == AF_LINK &&
            (ifa->ifa_flags & IFF_LOOPBACK) != 0) {
            kLoopbackIndex = (unsigned int)((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index;
            break;
        }
    }

    freeifaddrs(ifaddrs);
    return;
}

static int
si_destination_compare_statistics(
	Destination	*dst1,
	Destination	*dst2)
{
	int slightPreference = kPrefer_Equal;
	// Initialize kLoopbackIndex value
	static pthread_once_t   once = PTHREAD_ONCE_INIT;
	pthread_once(&once, set_loopback_ifindex);

	// If we have min round trip times for both, use that
	if (dst1->d_min_rtt && dst2->d_min_rtt)
	{
#define RTT_MSEC_DOUBLE_FROM_DST( dst ) (((double)(dst->d_min_rtt)) / ((double)(TCP_RTT_SCALE)))
		if (dst1->d_min_rtt < dst2->d_min_rtt)
		{
			if (dst1->d_min_rtt + si_compare_settings.rtt_leeway < dst2->d_min_rtt)
			{
				os_log_debug(si_destination_log,
							 "prefering 1, dst1 min rtt %f < dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
				return kPrefer_Dest1;
			}
			else if (dst1->d_min_rtt + si_compare_settings.rtt_leeway_small < dst2->d_min_rtt)
			{
				slightPreference = kPrefer_Dest1_Slightly;
				os_log_debug(si_destination_log,
							 "prefering 1 slightly, dst1 min rtt %f < dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
			}
			else
			{
				os_log_debug(si_destination_log,
							 "not prefering 1, dst1 min rtt %f < dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
			}
		}
		else if (dst1->d_min_rtt > dst2->d_min_rtt)
		{
			if (dst1->d_min_rtt > si_compare_settings.rtt_leeway + dst2->d_min_rtt)
			{
				os_log_debug(si_destination_log,
							 "prefering 2, dst1 min rtt %f > dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
				return kPrefer_Dest2;
			}
			else if (dst1->d_min_rtt > si_compare_settings.rtt_leeway_small + dst2->d_min_rtt)
			{
				slightPreference = kPrefer_Dest2_Slightly;
				os_log_debug(si_destination_log,
							 "prefering 2 slightly, dst1 min rtt %f > dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
			}
			else
			{
				os_log_debug(si_destination_log,
							 "not prefering 2, dst1 min rtt %f > dst2 min rtt %f, leeway %u small leeway %u",
							 RTT_MSEC_DOUBLE_FROM_DST(dst1), RTT_MSEC_DOUBLE_FROM_DST(dst2),
							 si_compare_settings.rtt_leeway / TCP_RTT_SCALE,
							 si_compare_settings.rtt_leeway_small / TCP_RTT_SCALE);
			}
		}
		else
		{
			os_log_debug(si_destination_log, "no preference, equal min rtt %f", RTT_MSEC_DOUBLE_FROM_DST(dst1));
		}
		// we hold on to slightPreference and only use it if no other part of the evaluation had a stronger preference
#undef RTT_MSEC_DOUBLE_FROM_DST
	}
	else if (dst1->d_min_rtt || dst2->d_min_rtt)
	{
		// If only dest1 is missing round trip make sure it doesn't have zero successful attempts
		if (dst1->d_min_rtt == 0 &&
			dst1->d_connection_successes == 0 &&
			dst1->d_connection_attempts > 0)
		{
			if (dst1->d_connection_attempts > si_compare_settings.rtt_failure_threshold)
			{
				os_log_debug(si_destination_log, "prefer 2, dst 1 has no successful connections and %u attempts",
							 dst1->d_connection_attempts);
				return kPrefer_Dest2;
			}
		}
		// If only dest2 is missing round trip make sure it doesn't have zero successful attempts
		else if (dst2->d_min_rtt == 0 &&
				 dst2->d_connection_successes == 0 &&
				 dst2->d_connection_attempts > 0)
		{
			if (dst2->d_connection_attempts > si_compare_settings.rtt_failure_threshold)
			{
				os_log_debug(si_destination_log, "prefer 1, dst 2 has no successful connections and %u attempts",
							 dst2->d_connection_attempts);
				return kPrefer_Dest1;
			}
		}
	}

	// If we have routes for both...
	if (dst1->d_routable != 0 && dst2->d_routable != 0)
	{
		// <rdar://problem/9070784> Prefer interface that are not peer to peer
		if (dst1->d_awdl != dst2->d_awdl)
		{
			if (dst2->d_awdl)
			{
				os_log_debug(si_destination_log, "prefer dst 1, dst 2 is p2p, dst 1 is not");
				return kPrefer_Dest1;
			}
			else if (dst1->d_awdl)
			{
				os_log_debug(si_destination_log, "prefer dst 2, dst 1 is p2p, dst 2 is not");
				return kPrefer_Dest2;
			}
		}

		// Prefer loopback routes
		bool dst1loopback = dst1->d_resolved_ifindex == kLoopbackIndex ||
			(dst1->d_dst.addr.sa.sa_family == AF_INET6 &&
			 IN6_IS_ADDR_LINKLOCAL(&dst1->d_dst.addr.sin6.sin6_addr) &&
			 dst1->d_dst.addr.sin6.sin6_scope_id == kLoopbackIndex);
		bool dst2loopback = dst2->d_resolved_ifindex == kLoopbackIndex ||
			(dst2->d_dst.addr.sa.sa_family == AF_INET6 &&
			 IN6_IS_ADDR_LINKLOCAL(&dst2->d_dst.addr.sin6.sin6_addr) &&
			 dst2->d_dst.addr.sin6.sin6_scope_id == kLoopbackIndex);

		if (dst1loopback && !dst2loopback)
		{
			os_log_debug(si_destination_log, "prefer 1, dst 1 is loopback, dst 2 is not");
			return kPrefer_Dest1;
		}
		else if (dst2loopback && !dst1loopback)
		{
			os_log_debug(si_destination_log, "prefer 2, dst 2 is loopback, dst 1 is not");
			return kPrefer_Dest2;
		}

		// Prefer direct routes
		if (dst1->d_direct && !dst2->d_direct)
		{
			os_log_debug(si_destination_log, "prefer 1, dst 1 is local, dst 2 is not");
			return kPrefer_Dest1;
		}
		else if (dst2->d_direct && !dst1->d_direct)
		{
			os_log_debug(si_destination_log, "prefer 2, dst 2 is local, dst 1 is not");
			return kPrefer_Dest2;
		}
	}
	else if (dst1->d_routable != 0)
	{
		// prefer destination we have a route to
		os_log_debug(si_destination_log, "prefer 1, dst 2 has no route");
		return kPrefer_Dest1;
	}
	else if (dst2->d_routable != 0)
	{
		// prefer destination we have a route to
		os_log_debug(si_destination_log, "prefer 2, dst 1 has no route");
		return kPrefer_Dest2;
	}
	return slightPreference;
}

#pragma mark -- Cache --

static bool
sa_equal(
	const struct sockaddr	*sa1,
	const struct sockaddr	*sa2)
{
	if (sa1->sa_family != sa2->sa_family)
		return false;

	/* We don't use bcmp because we don't care about the port number */
	if (sa1->sa_family == AF_INET)
	{
		const struct sockaddr_in *sin1 = (const struct sockaddr_in *)sa1;
		const struct sockaddr_in *sin2 = (const struct sockaddr_in *)sa2;

		if (sin1->sin_addr.s_addr != sin2->sin_addr.s_addr)
			return false;
		return true;
	}

	if (sa1->sa_family == AF_INET6)
	{
		const struct sockaddr_in6 *sin61 = (const struct sockaddr_in6 *)sa1;
		const struct sockaddr_in6 *sin62 = (const struct sockaddr_in6 *)sa2;

		if (memcmp(&sin61->sin6_addr, &sin62->sin6_addr, sizeof(sin61->sin6_addr)) != 0) {
			return false;
		}

		// TBD: Is the flow info relevant?

		if (sin61->sin6_scope_id != sin62->sin6_scope_id) {
			return false;
		}

		return true;
	}

	if (sa1->sa_len != sa2->sa_len) {
		return false;
	}

	return (memcmp(sa1, sa2, sa1->sa_len) == 0);
}

static bool
si_destination_cache_find(
	const struct sockaddr	*dst_sa,
	uint32_t				ifscope,
	Destination				*out_dst)
{
	// Loop through the entries looking for:
	// - expired items
	// - macthing item
	DestCacheEntry	**pprev = &cache;
	uint64_t		now = mach_absolute_time();
	bool			result = false;

	while (*pprev != NULL)
	{
		// If the item has expired, pull it out of the list
		if ((now - (*pprev)->dce_time) >= cache_timeout)
		{
			DestCacheEntry	*expired = *pprev;
			*pprev = expired->dce_next;
			memset(expired, 0, sizeof(*expired));
			free(expired);
		}
		else
		{
			// If the item matches, copy the entry
			if (!result && (*pprev)->dce_entry.d_ifindex == ifscope &&
				sa_equal(dst_sa, &(*pprev)->dce_entry.d_dst.addr.sa))
			{
				*out_dst = (*pprev)->dce_entry;
				result = true;
			}

			pprev = &(*pprev)->dce_next;
		}
	}

	return result;
}

static bool
si_destination_cache_create(
	const struct sockaddr	*dst_sa,
	uint32_t				ifscope,
	Destination				*out_dst)
{
	DestCacheEntry	*cache_entry;
	Destination		*d;

	if (dst_sa->sa_len > sizeof(cache_entry->dce_entry.d_dst.addr)) {
		os_log_error(si_destination_log, "(dst_sa->sa_len %u > sizeof(cache_entry->dce_entry.d_dst.addr))",
					 dst_sa->sa_len);
		return false;
	}

	// Allocate entry
	cache_entry = calloc(1, sizeof(*cache_entry));
	if (NULL == cache_entry) {
		os_log_error(si_destination_log, "calloc(%zu) failed: %m", sizeof(*cache_entry));
		return false;
	}
	d = &cache_entry->dce_entry;

	// Copy the destination sockaddr
	memcpy(&d->d_dst.addr, dst_sa, dst_sa->sa_len);
	d->d_ifindex = ifscope;

	// Query the kernel for the matching source, precedence and label
	si_destination_fill_netsrc(d);
	*out_dst = *d;

	cache_entry->dce_time = mach_absolute_time();
	cache_entry->dce_next = cache;
	cache = cache_entry;

	return true;
}

static int
si_destination_lookup(
	const struct sockaddr	*dst_sa,
	uint32_t				ifscope,
	Destination				*out_dst)
{
	int lookupResult = kLookupFailure;

	si_destination_compare_init();
	os_unfair_lock_lock(&cache_lock);
	if (si_destination_cache_find(dst_sa, ifscope, out_dst)) {
		lookupResult = kLookupSuccess_Found;
	} else if (si_destination_cache_create(dst_sa, ifscope, out_dst)) {
		lookupResult = kLookupSuccess_Created;
	}
	os_unfair_lock_unlock(&cache_lock);

	return lookupResult;
}

#pragma mark -- RFC 6724 --

// https://tools.ietf.org/html/rfc6724

#ifndef	IN6_IS_ADDR_TEREDO
#define	IN6_IS_ADDR_TEREDO(x)	((((x)->__u6_addr.__u6_addr16[0]) == htons(0x2001)) && \
								 (((x)->__u6_addr.__u6_addr16[1]) == 0x0000))
#endif // IN6_IS_ADDR_TEREDO

#ifndef s6_addr16
#undef	IN6_IS_ADDR_6TO4
#define	IN6_IS_ADDR_6TO4(x)	(((x)->__u6_addr.__u6_addr16[0]) == htons(0x2002))
#endif // s6_addr16

#ifndef IN6_IS_ADDR_6BONE
#  define IN6_IS_ADDR_6BONE(x) (((x)->__u6_addr.__u6_addr16[0]) == htons(0x3ffe))
#endif // IN6_IS_ADDR_6BONE

static int
rfc6724_scope_ip6(const struct in6_addr *addr)
{
	int scope;

	if (addr->s6_addr[0] == 0xfe) {
		scope = addr->s6_addr[1] & 0xc0;

		switch (scope) {
			case 0x80:
				return __IPV6_ADDR_SCOPE_LINKLOCAL;
			case 0xc0:
				return __IPV6_ADDR_SCOPE_SITELOCAL;
			default:
				return __IPV6_ADDR_SCOPE_GLOBAL; /* just in case */
		}
	}


	if (addr->s6_addr[0] == 0xff) {
		scope = addr->s6_addr[1] & 0x0f;

		/*
		 * due to other scope such as reserved,
		 * return scope doesn't work.
		 */
		switch (scope) {
			case __IPV6_ADDR_SCOPE_NODELOCAL:
				return __IPV6_ADDR_SCOPE_NODELOCAL;
			case __IPV6_ADDR_SCOPE_LINKLOCAL:
				return __IPV6_ADDR_SCOPE_LINKLOCAL;
			case __IPV6_ADDR_SCOPE_SITELOCAL:
				return __IPV6_ADDR_SCOPE_SITELOCAL;
			default:
				return __IPV6_ADDR_SCOPE_GLOBAL;
		}
	}

	/*
	 * Regard loopback and unspecified addresses as global, since
	 * they have no ambiguity.
	 */
	static const struct in6_addr in6addr_lo = IN6ADDR_LOOPBACK_INIT;
	if (memcmp(&in6addr_lo, addr, sizeof(*addr) - 1) == 0) {
		if (addr->s6_addr[15] == 1) {	/* loopback */
			return __IPV6_ADDR_SCOPE_LINKLOCAL;
		}
		if (addr->s6_addr[15] == 0) {	/* unspecified */
			return __IPV6_ADDR_SCOPE_GLOBAL; /* XXX: correct? */
		}
	}

	return __IPV6_ADDR_SCOPE_GLOBAL;
}

static int
rfc6724_scope_ip(const struct in_addr *addr)
{
	uint32_t hostbyteaddr = ntohl(addr->s_addr);
	if (IN_LOOPBACK(hostbyteaddr) || IN_LINKLOCAL(hostbyteaddr)) {
		return __IPV6_ADDR_SCOPE_LINKLOCAL;
	}
	return __IPV6_ADDR_SCOPE_GLOBAL;
}

static int
rfc6724_scope_sa(const struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET6) {
		const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;
		return rfc6724_scope_ip6(&sin6->sin6_addr);
	} else if (sa->sa_family == AF_INET) {
		const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
		return rfc6724_scope_ip(&sin->sin_addr);
	}
	return 0;
}

static int
rfc6724_scope(RFC6724Address *addr)
{
	return rfc6724_scope_sa(&addr->addr.sa);
}

//	RFC 6724 Section 2.1
//	https://tools.ietf.org/html/rfc6724#section-2.1

//	Prefix        Precedence Label
//	::1/128               50     0
//	::/0                  40     1
//	::ffff:0:0/96         35     4
//	2002::/16             30     2
//	2001::/32              5     5
//	fc00::/7               3    13
//	::/96                  1     3
//	fec0::/10              1    11
//	3ffe::/16              1    12

static int
rfc6724_precedence(const struct sockaddr *sa)
{
	if (sa == NULL) {

		return 0;
	}
	SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(sa, 0);
	if (sa->sa_family == AF_INET6) {
		const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;

		// ::1/128
		if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
			return 50;
		}

		// ::ffff:0:0/96
		if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
			return 35;
		}

		// 2002::/16
		if (IN6_IS_ADDR_6TO4(&sin6->sin6_addr)) {
			return 30;
		}

		// 2001::/32
		if (IN6_IS_ADDR_TEREDO(&sin6->sin6_addr)) {
			return 5;
		}

		// fc00::/7
		if (IN6_IS_ADDR_UNIQUE_LOCAL(&sin6->sin6_addr)) {
			return 3;
		}

		// ::/96
		if (IN6_IS_ADDR_V4COMPAT(&sin6->sin6_addr)) {
			return 1;
		}

		// fec0::/10
		if (IN6_IS_ADDR_SITELOCAL(&sin6->sin6_addr)) {
			return 1;
		}

		// 3ffe::/16
		if (IN6_IS_ADDR_6BONE(&sin6->sin6_addr)) {
			return 1;
		}

		// ::/0
		return 40;
	} else if (sa->sa_family == AF_INET) {

		// ::ffff:0:0/96 (IPv4 is treated as a v4-mapped v6 address)
		return 35;
	}
	return 0;
}

static bool
rfc6724_native(const RFC6724Address *addr)
{
	return !(addr->addr.sa.sa_family == AF_INET6 &&
			 (IN6_IS_ADDR_6TO4(&addr->addr.sin6.sin6_addr) ||
			  IN6_IS_ADDR_TEREDO(&addr->addr.sin6.sin6_addr)));
}

static int
common_prefix_length(const union sockaddr_in_4_6 *addr1, const union sockaddr_in_4_6 *addr2)
{
	int match = 0;
	if (addr1->sa.sa_family == AF_INET6 && addr2->sa.sa_family == AF_INET6)
	{
		const unsigned char *s = (const unsigned char *)&addr1->sin6.sin6_addr;
		const unsigned char *d = (const unsigned char *)&addr2->sin6.sin6_addr;
		const unsigned char *lim = s + 8;
		unsigned char r;

		while (s < lim) {
			if ((r = (*d++ ^ *s++)) != 0) {
				while (r < 128) {
					match++;
					r <<= 1;
				}
				break;
			} else {
				match += 8;
			}
		}
	}
	return match;
}

static int
si_destination_compare_rfc6724(Destination *d1, Destination *d2, int statResult)
{
	// Rule 1: Avoid unusable destinations (no source means unusable dest)
	if (d1->d_src.addr.sa.sa_family == AF_UNSPEC || d2->d_src.addr.sa.sa_family == AF_UNSPEC)
	{
		if (d1->d_src.addr.sa.sa_family != AF_UNSPEC)
		{
			os_log_debug(si_destination_log, "Rule 1, prefer d1, d2 is not routable");
			return kPrefer_Dest1;
		}
		else if (d2->d_src.addr.sa.sa_family != AF_UNSPEC)
		{
			os_log_debug(si_destination_log, "Rule 1, prefer d2, d1 is not routable");
			return kPrefer_Dest2;
		}
	}

	// Rule 2: Prefer matching scope
	if (rfc6724_scope(&d1->d_dst) != rfc6724_scope(&d1->d_src) || rfc6724_scope(&d2->d_dst) != rfc6724_scope(&d2->d_src))
	{
		if (rfc6724_scope(&d1->d_dst) == rfc6724_scope(&d1->d_src))
		{
			os_log_debug(si_destination_log, "Rule 2, prefer d1, d2 dst scope does not match src scope");
			return kPrefer_Dest1;
		}
		if (rfc6724_scope(&d2->d_dst) == rfc6724_scope(&d2->d_src))
		{
			os_log_debug(si_destination_log, "Rule 2, prefer d2, d1 dst scope does not match src scope");
			return kPrefer_Dest2;
		}
	}

	// Rule 3: Avoid deprecated addresses
	if ((d1->d_src.flags & NETSRC_IP6_FLAG_DEPRECATED) != (d2->d_src.flags & NETSRC_IP6_FLAG_DEPRECATED))
	{
		if ((d1->d_src.flags & NETSRC_IP6_FLAG_DEPRECATED) != 0)
		{
			os_log_debug(si_destination_log, "Rule 3, prefer d2, d1 source is deprecated");
			return kPrefer_Dest1;
		}
		if ((d2->d_src.flags & NETSRC_IP6_FLAG_DEPRECATED) != 0)
		{
			os_log_debug(si_destination_log, "Rule 3, prefer d1, d2 source is deprecated");
			return kPrefer_Dest2;
		}
	}

	// Rule 3bis: Avoid optimistic addresses, c.f. RFC 4429 which defines them as conceptually similar to deprecated
	// Note that this rule is not part of RFC 6724
	if ((d1->d_src.flags & NETSRC_IP6_FLAG_OPTIMISTIC) != (d2->d_src.flags & NETSRC_IP6_FLAG_OPTIMISTIC))
	{
		if ((d1->d_src.flags & NETSRC_IP6_FLAG_OPTIMISTIC) != 0)
		{
			os_log_debug(si_destination_log, "Rule 3a, prefer d2, d1 source is optimistic");
			return kPrefer_Dest1;
		}
		if ((d2->d_src.flags & NETSRC_IP6_FLAG_OPTIMISTIC) != 0)
		{
			os_log_debug(si_destination_log, "Rule 3a, prefer d1, d2 source is optimistic");
			return kPrefer_Dest2;
		}
	}

	// Rule 4: Prefer home addresses
	// TODO: requires Mobile IPv6 support

	// Rule 5: Prefer matching label
	if (d1->d_dst.label != d1->d_src.label || d2->d_dst.label != d2->d_src.label)
	{
		if (d1->d_dst.label == d1->d_src.label)
		{
			os_log_debug(si_destination_log, "Rule 5, prefer d1, d2 dst label does not match src label");
			return kPrefer_Dest1;
		}
		if (d2->d_dst.label == d2->d_src.label)
		{
			os_log_debug(si_destination_log, "Rule 5, prefer d2, d1 dst label does not match src label");
			return kPrefer_Dest2;
		}
	}

	// Rule 6: Prefer higher precedence
	if (d1->d_dst.precedence > d2->d_dst.precedence)
	{
		os_log_debug(si_destination_log, "Rule 6, prefer d1, d1 precedence %d > d2 precedence %d",
					 d1->d_dst.precedence, d2->d_dst.precedence);
		return kPrefer_Dest1;
	}
	else if (d2->d_dst.precedence > d1->d_dst.precedence)
	{
		os_log_debug(si_destination_log, "Rule 6, prefer d2, d2 precedence %d > d1 precedence %d",
					 d2->d_dst.precedence, d1->d_dst.precedence);
		return kPrefer_Dest2;
	}

	// Rule 7: Prefer native transport
	const bool d1_native = rfc6724_native(&d1->d_src);
	const bool d2_native = rfc6724_native(&d2->d_src);
	if (d1_native && !d2_native)
	{
		os_log_debug(si_destination_log, "Rule 7, prefer d1, d2 src is not native");
		return kPrefer_Dest1;
	}
	else if (d2_native && !d1_native)
	{
		os_log_debug(si_destination_log, "Rule 7, prefer d2, d1 src is not native");
		return kPrefer_Dest2;
	}

	// Rule 8: Prefer smaller scope
	const int scope1 = rfc6724_scope(&d1->d_dst);
	const int scope2 = rfc6724_scope(&d2->d_dst);
	if (scope1 < scope2)
	{
		os_log_debug(si_destination_log, "Rule 8, prefer d1, d1 scope %d < d2 scope %d", scope1, scope2);
		return kPrefer_Dest1;
	}
	else if (scope2 < scope1)
	{
		os_log_debug(si_destination_log, "Rule 8, prefer d2, d2 scope %d < d1 scope %d", scope2, scope1);
		return kPrefer_Dest2;
	}

	// RFC6724: Rules 9 and 10 MAY be superseded if the implementation has other means of sorting destination addresses.
	if ((kPrefer_Dest1 == statResult) || (kPrefer_Dest1_Slightly == statResult)) {
		return kPrefer_Dest1;
	} else if ((kPrefer_Dest2 == statResult) || (kPrefer_Dest2_Slightly == statResult)) {
		return kPrefer_Dest2;
	}

	// Rule 9: Use longest matching prefix
	int matchlen1 = common_prefix_length(&d1->d_dst.addr, &d1->d_src.addr);
	int matchlen2 = common_prefix_length(&d2->d_dst.addr, &d2->d_src.addr);
	if (matchlen1 && matchlen2)
	{
		if (matchlen1 > matchlen2)
		{
			os_log_debug(si_destination_log, "Rule 9, prefer d1, d1 shares more common prefix");
			return kPrefer_Dest1;
		}
		else if (matchlen2 > matchlen1)
		{
			os_log_debug(si_destination_log, "Rule 9, prefer d2, d2 shares more common prefix");
			return kPrefer_Dest2;
		}
	}

	// Rule 10: Otherwise, leave the order unchanged
	return kPrefer_Equal;
}

#pragma mark -- Internal Helper --

static int
si_destination_compare_internal(
	const struct sockaddr	*dst1,
	uint32_t				dst1ifindex,
	const struct sockaddr	*dst2,
	uint32_t				dst2ifindex,
	bool					statistics)
{
	// If either of the destinations is not AF_INET/AF_INET6
	if ((dst1->sa_family != AF_INET && dst1->sa_family != AF_INET6) ||
		(dst2->sa_family != AF_INET && dst2->sa_family != AF_INET6))
	{
		if (dst1->sa_family == AF_INET || dst1->sa_family == AF_INET6) {
			return kPrefer_Dest1;
		} else if (dst2->sa_family == AF_INET || dst2->sa_family == AF_INET6) {
			return kPrefer_Dest2;
		} else {
			return kPrefer_Equal;
		}
	}

	Destination	d1;
	Destination	d2;

	// Lookup d1 and d2 in the cache
	int lookupResultD1 = si_destination_lookup(dst1, dst1ifindex, &d1);
	int lookupResultD2 = si_destination_lookup(dst2, dst2ifindex, &d2);
	if (lookupResultD1 == kLookupFailure)
	{
		os_log_debug(si_destination_log, "si_destination_lookup for dst1 failed");
		return kPrefer_Equal;
	}
	if (lookupResultD2 == kLookupFailure)
	{
		os_log_debug(si_destination_log, "si_destination_lookup for dst2 failed");
		return kPrefer_Equal;
	}

	int statResult = kPrefer_Equal;
	if (statistics && !si_compare_settings.bypass_stats)
	{
		statResult = si_destination_compare_statistics(&d1, &d2);
		if ((kPrefer_Dest1 == statResult) || (kPrefer_Dest2 == statResult))
		{
			return statResult;
		}
	}

	statResult = si_destination_compare_rfc6724(&d1, &d2, statResult);

	if (statResult == kPrefer_Equal) {
		// Only if all other comparisons are equal, prefer entries that were already in the cache over
		// ones that are new and we just created.

		// Found < Created
		if (lookupResultD1 == kLookupSuccess_Found && lookupResultD2 == kLookupSuccess_Created) {
			os_log_debug(si_destination_log, "prefer d1, known while d2 not known");
			statResult = kPrefer_Dest1;
		} else if (lookupResultD2 == kLookupSuccess_Found && lookupResultD1 == kLookupSuccess_Created) {
			os_log_debug(si_destination_log, "prefer d2, known while d1 not known");
			statResult = kPrefer_Dest2;
		}
	}

	return statResult;
}

#pragma mark -- SPI --

int
si_destination_compare(
	const struct sockaddr	*dst1,
	int						dst1ifindex,
	const struct sockaddr	*dst2,
	int						dst2ifindex,
	bool					statistics)
{
	SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(dst1, kPrefer_Equal);
	SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(dst2, kPrefer_Equal);

	si_destination_compare_init();

	const int result = si_destination_compare_internal(dst1, dst1ifindex, dst2, dst2ifindex, statistics);

	os_log_debug(si_destination_log, "%{network:sockaddr}.*P@%u %c %{network:sockaddr}.*P@%u",
				 dst1->sa_len, dst1, dst1ifindex, result == 0 ? '=' : result < 0 ? '<' : '>',
				 dst2->sa_len, dst2, dst2ifindex);

	return result;
}

int
si_destination_compare_no_dependencies(const struct sockaddr *dst1,
							   const struct sockaddr *dst2)
{
	SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(dst1, kPrefer_Equal);
	SI_DESTINATION_COMPARE_CHECK_ARG_RETURN(dst2, kPrefer_Equal);

	//	Skip rule 1 (requires route to destination address)
	//	Skip rule 2, 3, 5, 7, 9 (requires corresponding source address)
	//	Skip rule 4 (not supported by si_destination_compare() today)

	// Rule 6: Prefer higher precedence
	const int precedence1 = rfc6724_precedence(dst1);
	const int precedence2 = rfc6724_precedence(dst2);
	if (precedence1 > precedence2)
	{
		os_log_debug(si_destination_log, "ND Rule 6, prefer d1, d1 precedence %d > d2 precedence %d",
					 precedence1, precedence2);
		return kPrefer_Dest1;
	}
	else if (precedence2 > precedence1)
	{
		os_log_debug(si_destination_log, "ND Rule 6, prefer d2, d2 precedence %d > d1 precedence %d",
					 precedence2, precedence1);
		return kPrefer_Dest2;
	}

	// Rule 8: Prefer smaller scope
	const int scope1 = rfc6724_scope_sa(dst1);
	const int scope2 = rfc6724_scope_sa(dst2);
	if (scope1 < scope2)
	{
		os_log_debug(si_destination_log, "ND Rule 8, prefer d1, d1 scope %d < d2 scope %d",
					 scope1, scope2);
		return kPrefer_Dest1;
	}
	else if (scope2 < scope1)
	{
		os_log_debug(si_destination_log, "ND Rule 8, prefer d2, d2 scope %d < d1 scope %d",
					 scope2, scope1);
		return kPrefer_Dest2;
	}

	// Rule 10: Otherwise, leave the order unchanged

	return kPrefer_Equal;
}
