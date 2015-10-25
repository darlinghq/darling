/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*!
	@header kpi_mbuf.h
	This header defines an API for interacting with mbufs. mbufs are the
	primary method of storing packets in the networking stack.

	mbufs are used to store various items in the networking stack. The
	most common usage of an mbuf is to store a packet or data on a
	socket waiting to be sent or received. The mbuf is a contiguous
	structure with some header followed by some data. To store more data
	than would fit in an mbuf, external data is used. Most mbufs with
	external data use clusters to store the external data.

	mbufs can be chained, contiguous data in a packet can be found by
	following the m_next chain. Packets may be bundled together using
	m_nextpacket. Many parts of the stack do not properly handle chains
	of packets. When in doubt, don't chain packets.
 */

#ifndef __KPI_MBUF__
#define __KPI_MBUF__
#include <sys/kernel_types.h>
#include <mach/vm_types.h>

/*!
	@enum mbuf_flags_t
	@abstract Constants defining mbuf flags. Only the flags listed below
		can be set or retrieved.
	@constant MBUF_EXT Indicates this mbuf has external data.
	@constant MBUF_PKTHDR Indicates this mbuf has a packet header.
	@constant MBUF_EOR Indicates this mbuf is the end of a record.
	@constant MBUF_LOOP Indicates this packet is looped back.
	@constant MBUF_BCAST Indicates this packet will be sent or was
		received as a brodcast.
	@constant MBUF_MCAST Indicates this packet will be sent or was
		received as a multicast.
	@constant MBUF_FRAG Indicates this packet is a fragment of a larger
		packet.
	@constant MBUF_FIRSTFRAG Indicates this packet is the first fragment.
	@constant MBUF_LASTFRAG Indicates this packet is the last fragment.
	@constant MBUF_PROMISC Indicates this packet was only received
		because the interface is in promiscuous mode. This should be set
		by the demux function. These packets will be discarded after
		being passed to any interface filters.
*/
enum {
	MBUF_EXT	= 0x0001,	/* has associated external storage */
	MBUF_PKTHDR	= 0x0002,	/* start of record */
	MBUF_EOR	= 0x0004,	/* end of record */
	MBUF_LOOP	= 0x0040,	/* packet is looped back */

	MBUF_BCAST	= 0x0100,	/* send/received as link-level broadcast */
	MBUF_MCAST	= 0x0200,	/* send/received as link-level multicast */
	MBUF_FRAG	= 0x0400,	/* packet is a fragment of a larger packet */
	MBUF_FIRSTFRAG	= 0x0800,	/* packet is first fragment */
	MBUF_LASTFRAG	= 0x1000,	/* packet is last fragment */
	MBUF_PROMISC	= 0x2000,	/* packet is promiscuous */
	MBUF_HASFCS	= 0x4000	/* packet has FCS */
};
typedef u_int32_t mbuf_flags_t;

/*!
	@enum mbuf_type_t
	@abstract Types of mbufs.
	@discussion Some mbufs represent packets, some represnt data waiting
		on sockets. Other mbufs store control data or other various
		structures. The mbuf type is used to store what sort of data the
		mbuf contains.
	@constant MBUF_MT_FREE Indicates the mbuf is free and is
		sitting on the queue of free mbufs. If you find that an mbuf you
		have a reference to has this type, something has gone terribly
		wrong.
	@constant MBUF_MT_DATA Indicates this mbuf is being used to store
		data.
	@constant MBUF_MT_HEADER Indicates this mbuf has a packet header,
		this is probably a packet.
	@constant MBUF_MT_SOCKET Socket structure.
	@constant MBUF_MT_PCB Protocol control block.
	@constant MBUF_MT_RTABLE Routing table entry.
	@constant MBUF_MT_HTABLE IMP host tables???.
	@constant MBUF_MT_ATABLE Address resolution table data.
	@constant MBUF_MT_SONAME Socket name, usually a sockaddr of some
		sort.
	@constant MBUF_MT_FTABLE Fragment reassembly header.
	@constant MBUF_MT_RIGHTS Access rights.
	@constant MBUF_MT_IFADDR Interface address.
	@constant MBUF_MT_CONTROL Extra-data protocol message (control
		message).
	@constant MBUF_MT_OOBDATA Out of band data.
*/
enum {
	MBUF_TYPE_FREE		= 0,	/* should be on free list */
	MBUF_TYPE_DATA		= 1,	/* dynamic (data) allocation */
	MBUF_TYPE_HEADER	= 2,	/* packet header */
	MBUF_TYPE_SOCKET	= 3,	/* socket structure */
	MBUF_TYPE_PCB		= 4,	/* protocol control block */
	MBUF_TYPE_RTABLE	= 5,	/* routing tables */
	MBUF_TYPE_HTABLE	= 6,	/* IMP host tables */
	MBUF_TYPE_ATABLE	= 7,	/* address resolution tables */
	MBUF_TYPE_SONAME	= 8,	/* socket name */
	MBUF_TYPE_SOOPTS	= 10,	/* socket options */
	MBUF_TYPE_FTABLE	= 11,	/* fragment reassembly header */
	MBUF_TYPE_RIGHTS	= 12,	/* access rights */
	MBUF_TYPE_IFADDR	= 13,	/* interface address */
	MBUF_TYPE_CONTROL	= 14,	/* extra-data protocol message */
	MBUF_TYPE_OOBDATA	= 15	/* expedited data  */
};
typedef u_int32_t mbuf_type_t;

/*!
	@enum mbuf_csum_request_flags_t
	@abstract Checksum performed/requested flags.
	@discussion Mbufs often contain packets. Some hardware supports
		performing checksums in hardware. The stack uses these flags to
		indicate to the driver what sort of checksumming should be
		handled in by the driver/hardware. These flags will only be set
		if the driver indicates that it supports the corresponding
		checksums using ifnet_set_offload.
	@constant MBUF_CSUM_REQ_IP Indicates the IP checksum has not been
		calculated yet.
	@constant MBUF_CSUM_REQ_TCP Indicates the TCP checksum has not been
		calculated yet.
	@constant MBUF_CSUM_REQ_UDP Indicates the UDP checksum has not been
		calculated yet.
	@constant MBUF_CSUM_REQ_TCPIPV6 Indicates the TCP checksum for IPv6
       		has not been calculated yet.
	@constant MBUF_CSUM_REQ_UDPIPV6 Indicates the UDP checksum for IPv6
		has not been calculated yet.
*/
enum {
	MBUF_TSO_IPV4		= 0x100000,
	MBUF_TSO_IPV6		= 0x200000
};
typedef u_int32_t mbuf_tso_request_flags_t;

enum {
#ifdef KERNEL_PRIVATE
	MBUF_CSUM_REQ_SUM16	= 0x1000, /* Weird apple hardware checksum */
#endif /* KERNEL_PRIVATE */
	MBUF_CSUM_REQ_IP	= 0x0001,
	MBUF_CSUM_REQ_TCP	= 0x0002,
	MBUF_CSUM_REQ_UDP	= 0x0004,
	MBUF_CSUM_REQ_TCPIPV6	= 0x0020,
	MBUF_CSUM_REQ_UDPIPV6	= 0x0040
};
typedef u_int32_t mbuf_csum_request_flags_t;

/*!
	@enum mbuf_csum_performed_flags_t
	@abstract Checksum performed/requested flags.
	@discussion Mbufs often contain packets. Some hardware supports
		performing checksums in hardware. The driver uses these flags to
		communicate to the stack the checksums that were calculated in
		hardware.
	@constant MBUF_CSUM_DID_IP Indicates that the driver/hardware verified
		the IP checksum in hardware.
	@constant MBUF_CSUM_IP_GOOD Indicates whether or not the IP checksum
		was good or bad. Only valid when MBUF_CSUM_DID_IP is set.
	@constant MBUF_CSUM_DID_DATA Indicates that the TCP or UDP checksum
		was calculated. The value for the checksum calculated in
		hardware should be passed as the second parameter of
		mbuf_set_csum_performed. The hardware calculated checksum value
		can be retrieved using the second parameter passed to
		mbuf_get_csum_performed. This should be done for IPv4 or IPv6.
	@constant MBUF_CSUM_PSEUDO_HDR If set, this indicates that the
		checksum value for MBUF_CSUM_DID_DATA includes the pseudo header
		value. If this is not set, the stack will calculate the pseudo
		header value and add that to the checksum. The value of this bit
		is only valid when MBUF_CSUM_DID_DATA is set.
*/
enum {
#ifdef KERNEL_PRIVATE
	MBUF_CSUM_TCP_SUM16	= MBUF_CSUM_REQ_SUM16,	/* Weird apple hardware checksum */
#endif /* KERNEL_PRIVATE */
	MBUF_CSUM_DID_IP	= 0x0100,
	MBUF_CSUM_IP_GOOD	= 0x0200,
	MBUF_CSUM_DID_DATA	= 0x0400,
	MBUF_CSUM_PSEUDO_HDR	= 0x0800
};
typedef u_int32_t mbuf_csum_performed_flags_t;

/*!
	@enum mbuf_how_t
	@abstract Method of allocating an mbuf.
	@discussion Blocking will cause the funnel to be dropped. If the
		funnel is dropped, other threads may make changes to networking
		data structures. This can lead to very bad things happening.
		Blocking on the input our output path can also impact
		performance. There are some cases where making a blocking call
		is acceptable. When in doubt, use MBUF_DONTWAIT.
	@constant MBUF_WAITOK Allow a call to allocate an mbuf to block.
	@constant MBUF_DONTWAIT Don't allow the mbuf allocation call to
		block, if blocking is necessary fail and return immediately.
*/
enum {
	MBUF_WAITOK	= 0,	/* Ok to block to get memory */
	MBUF_DONTWAIT	= 1	/* Don't block, fail if blocking would be required */
};
typedef u_int32_t mbuf_how_t;

typedef u_int32_t mbuf_tag_id_t;
typedef	u_int16_t mbuf_tag_type_t;

/*!
	@struct mbuf_stat
	@discussion The mbuf_stat contains mbuf statistics.
	@field mbufs Number of mbufs (free or otherwise).
	@field clusters Number of clusters (free or otherwise).
	@field clfree Number of free clusters.
	@field drops Number of times allocation failed.
	@field wait Number of times allocation blocked.
	@field drain Number of times protocol drain functions were called.
	@field mtypes An array of counts of each type of mbuf allocated.
	@field mcfail Number of times m_copym failed.
	@field mpfail Number of times m_pullup failed.
	@field msize Length of an mbuf.
	@field mclbytes Length of an mbuf cluster.
	@field minclsize Minimum length of data to allocate a cluster.
		Anything smaller than this should be placed in chained mbufs.
	@field mlen Length of data in an mbuf.
	@field mhlen Length of data in an mbuf with a packet header.
	@field bigclusters Number of big clusters.
	@field bigclfree Number of unused big clusters.
	@field bigmclbytes Length of a big mbuf cluster.
*/
struct mbuf_stat {
	u_int32_t	mbufs;		/* mbufs obtained from page pool */
	u_int32_t	clusters;	/* clusters obtained from page pool */
	u_int32_t	clfree;		/* free clusters */
	u_int32_t	drops;		/* times failed to find space */
	u_int32_t	wait;		/* times waited for space */
	u_int32_t	drain;		/* times drained protocols for space */
	u_short		mtypes[256];	/* type specific mbuf allocations */
	u_int32_t	mcfail;		/* times m_copym failed */
	u_int32_t	mpfail;		/* times m_pullup failed */
	u_int32_t	msize;		/* length of an mbuf */
	u_int32_t	mclbytes;	/* length of an mbuf cluster */
	u_int32_t	minclsize;	/* min length of data to allocate a cluster */
	u_int32_t	mlen;		/* length of data in an mbuf */
	u_int32_t	mhlen;		/* length of data in a header mbuf */
	u_int32_t	bigclusters;	/* number of big clusters */
	u_int32_t	bigclfree;	/* number of big clustser free */
	u_int32_t	bigmclbytes;	/* length of data in a big cluster */
};

/* Parameter for m_copym to copy all bytes */
#define	MBUF_COPYALL	1000000000

__BEGIN_DECLS
/* Data access */
/*!
	@function mbuf_data
	@discussion Returns a pointer to the start of data in this mbuf.
		There may be additional data on chained mbufs. The data you're
		looking for may not be virtually contiguous if it spans more
		than one mbuf.  In addition, data that is virtually contiguous
		might not be represented by physically contiguous pages; see
		further comments in mbuf_data_to_physical.  Use mbuf_len to
		determine the lenght of data available in this mbuf. If a data
		structure you want to access stradles two mbufs in a chain,
		either use mbuf_pullup to get the data contiguous in one mbuf
		or copy the pieces of data from each mbuf in to a contiguous
		buffer. Using mbuf_pullup has the advantage of not having to
		copy the data. On the other hand, if you don't make sure there
		is space in the mbuf, mbuf_pullup may fail and free the mbuf.
	@param mbuf The mbuf.
	@result A pointer to the data in the mbuf.
 */
extern void *mbuf_data(mbuf_t mbuf);

/*!
	@function mbuf_datastart
	@discussion Returns the start of the space set aside for storing
		data in an mbuf. An mbuf's data may come from a cluster or be
		embedded in the mbuf structure itself. The data pointer
		retrieved by mbuf_data may not be at the start of the data
		(mbuf_leadingspace will be non-zero). This function will return
		a pointer that matches mbuf_data() - mbuf_leadingspace().
	@param mbuf The mbuf.
	@result A pointer to smallest possible value for data.
 */
extern void *mbuf_datastart(mbuf_t mbuf);

/*!
	@function mbuf_setdata
	@discussion Sets the data and length values for an mbuf. The data
	value must be in a valid range. In the case of an mbuf with a cluster,
	the data value must point to a location in the cluster and the data
	value plus the length, must be less than the end of the cluster. For
	data embedded directly in an mbuf (no cluster), the data value must
	fall somewhere between the start and end of the data area in the
	mbuf and the data + length must also be in the same range.
	@param mbuf The mbuf.
	@param data The new pointer value for data.
	@param len The new length of data in the mbuf.
	@result 0 on success, errno error on failure.
 */
extern errno_t mbuf_setdata(mbuf_t mbuf, void *data, size_t len);

/*!
	@function mbuf_align_32
	@discussion mbuf_align_32 is a replacement for M_ALIGN and MH_ALIGN.
		mbuf_align_32 will set the data pointer to a location aligned on
		a four byte boundry with at least 'len' bytes between the data
		pointer and the end of the data block.
	@param mbuf The mbuf.
	@param len The minimum length of space that should follow the new
		data location.
	@result 0 on success, errno error on failure.
 */
extern errno_t mbuf_align_32(mbuf_t mbuf, size_t len);

/*!
	@function mbuf_data_to_physical
	@discussion mbuf_data_to_physical is a replacement for mcl_to_paddr.
		Given a pointer returned from mbuf_data of mbuf_datastart,
		mbuf_data_to_physical will return the phyical address for that
		block of data.  Note that even though the data is in virtually
		contiguous span, the underlying physical pages might not be
		physically contiguous.  Because of this, callers must ensure
		to call this routine for each page boundary.  Device drivers
		that deal with DMA are strongly encouraged to utilize the
		IOMbufNaturalMemoryCursor and walk down the list of vectors
		instead of using this interface to obtain the physical address.
		Use of this routine is therefore discouraged.
	@param ptr A pointer to data stored in an mbuf.
	@result The 64 bit physical address of the mbuf data or NULL if ptr
		does not point to data stored in an mbuf.
 */
extern addr64_t mbuf_data_to_physical(void *ptr);


/* Allocation */

/*!
	@function mbuf_get
	@discussion Allocates an mbuf without a cluster for external data.
	@param how Blocking or non-blocking.
	@param type The type of the mbuf.
	@param mbuf The mbuf.
	@result 0 on success, errno error on failure.
 */
extern errno_t mbuf_get(mbuf_how_t how, mbuf_type_t type, mbuf_t *mbuf);

/*!
	@function mbuf_gethdr
	@discussion Allocates an mbuf without a cluster for external data.
		Sets a flag to indicate there is a packet header and initializes
		the packet header.
	@param how Blocking or non-blocking.
	@param type The type of the mbuf.
	@param mbuf The mbuf.
	@result 0 on success, errno error on failure.
 */
extern errno_t mbuf_gethdr(mbuf_how_t how, mbuf_type_t type, mbuf_t *mbuf);

/*!
	@function mbuf_attachcluster
	@discussion Attach an external buffer as a cluster for an mbuf.  If mbuf
		points to a NULL mbuf_t, an mbuf will be allocated for you.  If
		mbuf points to a non-NULL mbuf_t, the user-supplied mbuf will
		be used instead.  The caller is responsible for allocating the
		external buffer by calling mbuf_alloccluster().
	@param how Blocking or non-blocking.
	@param type The type of the mbuf if mbuf is non-NULL; otherwise ignored.
	@param mbuf Pointer to the address of the mbuf; if NULL, an mbuf will
		be allocated, otherwise, it must point to a valid mbuf address.
		If the user-supplied mbuf is already attached to a cluster, the
		current cluster will be freed before the mbuf gets attached to
		the supplied external buffer.  Note that this routine may return
		a different mbuf_t than the one you passed in.
	@param extbuf Address of the external buffer.
	@param extfree Free routine for the external buffer; the caller is
		required to defined a routine that will be invoked when the
		mbuf is freed.
	@param extsize Size of the external buffer.
	@param extarg Private value that will be passed to the free routine
		when it is called at the time the mbuf is freed.
	@result 0 on success
		EINVAL - Invalid parameter
		ENOMEM - Not enough memory available
 */
extern errno_t mbuf_attachcluster(mbuf_how_t how, mbuf_type_t type,
    mbuf_t *mbuf, caddr_t extbuf, void (*extfree)(caddr_t , u_int, caddr_t),
    size_t extsize, caddr_t extarg);

/*!
	@function mbuf_alloccluster
	@discussion Allocate a cluster that can be later attached to an
		mbuf by calling mbuf_attachcluster().  The allocated cluster
		can also be freed (without being attached to an mbuf) by
		calling mbuf_freecluster().  At the moment this routine
		will either return a cluster of 2048, 4096 or 16384 bytes
		depending on the requested size.  Note that clusters greater
		than 4096 bytes might not be available in all configurations;
		the caller must additionally check for ENOTSUP (see below).
	@param how Blocking or non-blocking.
	@param size Pointer to size of requested cluster.  Sizes up to 2048
		will be rounded up to 2048; sizes greater than 2048 and up
		to 4096 will be rounded up to 4096.  Sizes greater than 4096
		will be rounded up to 16384.
	@param addr Pointer to the address of the requested cluster.
	@result 0 on success or ENOMEM if failure.  If the caller requests
		greater than 4096 bytes and the system is unable to fulfill
		the request due to the lack of jumbo clusters support based
		on the configuration, this routine will return ENOTSUP.
		In this case, the caller is advised to use 4096 bytes or
		smaller during subseqent requests.
 */
extern errno_t mbuf_alloccluster(mbuf_how_t how, size_t *size, caddr_t *addr);

/*!
	@function mbuf_freecluster
	@discussion Free a cluster that was previously allocated by a call
		to mbuf_alloccluster().  The caller must pass the actual
		size of the cluster as returned by mbuf_alloccluster(),
		which at this point must be either 2048, 4096 or 16384 bytes.
	@param addr The address of the cluster.
	@param size The actual size of the cluster.
 */
extern void mbuf_freecluster(caddr_t addr, size_t size);

/*!
	@function mbuf_getcluster
	@discussion Allocate a cluster of the requested size and attach it to
		an mbuf for use as external data. If mbuf points to a NULL
		mbuf_t, an mbuf will be allocated for you. If mbuf points to
		a non-NULL mbuf_t, mbuf_getcluster may return a different
		mbuf_t than the one you passed in.
	@param how Blocking or non-blocking.
	@param type The type of the mbuf.
	@param size The size of the cluster to be allocated. Supported sizes
		for a cluster are be 2048, 4096, or 16384. Any other value
		with return EINVAL.  Note that clusters greater than 4096
		bytes might not be available in all configurations; the
		caller must additionally check for ENOTSUP (see below).
	@param mbuf The mbuf the cluster will be attached to.
	@result 0 on success, errno error on failure. If you specified NULL
		for the mbuf, any intermediate mbuf that may have been allocated
		will be freed. If you specify an mbuf value in *mbuf,
		mbuf_mclget will not free it.
		EINVAL - Invalid parameter
		ENOMEM - Not enough memory available
		ENOTSUP - The caller had requested greater than 4096 bytes
		    cluster and the system is unable to fulfill it due to the
		    lack of jumbo clusters support based on the configuration.
		    In this case, the caller is advised to use 4096 bytes or
		    smaller during subsequent requests.
 */
extern errno_t mbuf_getcluster(mbuf_how_t how, mbuf_type_t type, size_t size,
    mbuf_t *mbuf);

/*!
	@function mbuf_mclget
	@discussion Allocate a cluster and attach it to an mbuf for use as
		external data. If mbuf points to a NULL mbuf_t, an mbuf will be
		allocated for you. If mbuf points to a non-NULL mbuf_t,
		mbuf_mclget may return a different mbuf_t than the one you
		passed in.
	@param how Blocking or non-blocking.
	@param type The type of the mbuf.
	@param mbuf The mbuf the cluster will be attached to.
	@result 0 on success, errno error on failure. If you specified NULL
		for the mbuf, any intermediate mbuf that may have been allocated
		will be freed. If you specify an mbuf value in *mbuf,
		mbuf_mclget will not free it.
 */
extern errno_t mbuf_mclget(mbuf_how_t how, mbuf_type_t type, mbuf_t *mbuf);

/*!
	@function mbuf_allocpacket
	@discussion Allocate an mbuf chain to store a single packet of the
		requested length.  According to the requested length, a chain
		of mbufs will be created. The mbuf type will be set to
		MBUF_TYPE_DATA. The caller may specify the maximum number of
		buffer.
	@param how Blocking or non-blocking
	@param packetlen The total length of the packet mbuf to be allocated.
		The length must be greater than zero.
	@param maxchunks An input/output pointer to the maximum number of mbufs
		segments making up the chain.  On input, if maxchunks is NULL,
		or the value pointed to by maxchunks is zero, the packet will
		be made up of as few or as many buffer segments as necessary
		to fit the length.  The allocation will fail with ENOBUFS if
		the number of segments requested is too small and the sum of
		the maximum size of each individual segment is less than the
		packet length.  On output, if the allocation succeed and
		maxchunks is non-NULL, it will point to the actual number
		of segments allocated.
		Additional notes for packetlen greater than 4096 bytes:
		the caller may pass a non-NULL maxchunks and initialize it
		with zero such that upon success, it can find out whether
		or not the system configuration allows for larger than
		4096 bytes cluster allocations, by checking on the value
		pointed to by maxchunks.  E.g. a request for 9018 bytes may
		result in 1 chunk when jumbo clusters are available, or
		3 chunks otherwise.
	@param Upon success, *mbuf will be a reference to the new mbuf.
	@result Returns 0 upon success or the following error code:
		EINVAL - Invalid parameter
		ENOMEM - Not enough memory available
		ENOBUFS - Buffers not big enough for the maximum number of
		    chunks requested
*/
extern errno_t mbuf_allocpacket(mbuf_how_t how, size_t packetlen,
    unsigned int * maxchunks, mbuf_t *mbuf);

/*!
	@function mbuf_allocpacket_list
	@discussion Allocate a linked list of packets.  According to the
		requested length, each packet will made of a chain of one
		or more mbufs.  The mbuf type will be set to MBUF_TYPE_DATA.
		The caller may specify the maximum number of element for
		each mbuf chain making up a packet.
	@param numpkts Number of packets to allocate
	@param how Blocking or non-blocking
	@param packetlen The total length of the packet mbuf to be allocated.
		The length must be greater than zero.
	@param maxchunks An input/output pointer to the maximum number of
		mbufs segments making up the chain.  On input, if maxchunks is
		zero, or the value pointed to by maxchunks is zero, the packet
		will be made of as few or as many buffer segments as necessary
		to fit the length.  The allocation will fail with ENOBUFS if
		the number of segments requested is too small and the sum of
		the maximum size of each individual segment is less than the
		packet length.  On output, if the allocation succeed and
		maxchunks is non zero, it will point to the actual number
		of segments allocated.
		Additional notes for packetlen greater than 4096 bytes:
		the caller may pass a non-NULL maxchunks and initialize it
		with zero such that upon success, it can find out whether
		or not the system configuration allows for larger than
		4096 bytes cluster allocations, by checking on the value
		pointed to by maxchunks.  E.g. a request for 9018 bytes may
		result in 1 chunk when jumbo clusters are available, or
		3 chunks otherwise.
	@param Upon success, *mbuf will be a reference to the new mbuf.
	@result Returns 0 upon success or the following error code:
		EINVAL - Invalid parameter
		ENOMEM - Not enough memory available
		ENOBUFS - Buffers not big enough for the maximum number of
		    chunks requested
*/
extern errno_t mbuf_allocpacket_list(unsigned int numpkts, mbuf_how_t how,
    size_t packetlen, unsigned int * maxchunks, mbuf_t *mbuf);


/*!
	@function mbuf_getpacket
	@discussion Allocate an mbuf, allocate and attach a cluster, and set
		the packet header flag.
	@param how Blocking or non-blocking.
	@param mbuf Upon success, *mbuf will be a reference to the new mbuf.
	@result 0 on success, errno error on failure.
 */
extern errno_t mbuf_getpacket(mbuf_how_t how, mbuf_t *mbuf);

/*!
	@function mbuf_free
	@discussion Frees a single mbuf. Not commonly used because it
		doesn't touch the rest of the mbufs on the chain.
	@param mbuf The mbuf to free.
	@result The next mbuf in the chain.
 */
extern mbuf_t mbuf_free(mbuf_t mbuf);

/*!
	@function mbuf_freem
	@discussion Frees a chain of mbufs link through mnext.
	@param mbuf The first mbuf in the chain to free.
 */
extern void mbuf_freem(mbuf_t mbuf);

/*!
	@function mbuf_freem_list
	@discussion Frees linked list of mbuf chains. Walks through
		mnextpackt and does the equivalent of mbuf_freem to each.
	@param mbuf The first mbuf in the linked list to free.
	@result The number of mbufs freed.
 */
extern int mbuf_freem_list(mbuf_t mbuf);

/*!
	@function mbuf_leadingspace
	@discussion Determines the space available in the mbuf proceeding
		the current data.
	@param mbuf The mbuf.
	@result The number of unused bytes at the start of the mbuf.
 */
extern size_t mbuf_leadingspace(const mbuf_t mbuf);

/*!
	@function mbuf_trailingspace
	@discussion Determines the space available in the mbuf following
		the current data.
	@param mbuf The mbuf.
	@result The number of unused bytes following the current data.
 */
extern size_t mbuf_trailingspace(const mbuf_t mbuf);

/* Manipulation */

/*!
	@function mbuf_copym
	@discussion Copies len bytes from offset from src to a new mbuf.  If
	    the original mbuf contains a packet header, the new mbuf will
	    contain similar packet header except for any tags which may be
	    associated with the original mbuf.  mbuf_dup() should be used
	    instead if tags are to be copied to the new mbuf.
	@param src The source mbuf.
	@param offset The offset in the mbuf to start copying from.
	@param len The the number of bytes to copy.
	@param how To block or not to block, that is a question.
	@param new_mbuf Upon success, the newly allocated mbuf.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_copym(const mbuf_t src, size_t offset, size_t len,
    mbuf_how_t how, mbuf_t *new_mbuf);

/*!
	@function mbuf_dup
	@discussion Exactly duplicates an mbuf chain.  If the original mbuf
	    contains a packet header (including tags), the new mbuf will have
	    the same packet header contents and a copy of each tag associated
	    with the original mbuf.
	@param src The source mbuf.
	@param how Blocking or non-blocking.
	@param new_mbuf Upon success, the newly allocated mbuf.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_dup(const mbuf_t src, mbuf_how_t how, mbuf_t *new_mbuf);

/*!
	@function mbuf_prepend
	@discussion Prepend len bytes to an mbuf. If there is space
		(mbuf_leadingspace >= len), the mbuf's data ptr is changed and
		the same mbuf is returned. If there is no space, a new mbuf may
		be allocated and prepended to the mbuf chain. If the operation
		fails, the mbuf may be freed (*mbuf will be NULL).
	@param mbuf The mbuf to prepend data to. This may change if a new
		mbuf must be allocated or may be NULL if the operation fails.
	@param len The length, in bytes, to be prepended to the mbuf.
	@param how Blocking or non-blocking.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_prepend(mbuf_t *mbuf, size_t len, mbuf_how_t how);

/*!
	@function mbuf_split
	@discussion Split an mbuf chain at a specific offset.
	@param src The mbuf to be split.
	@param offset The offset in the buffer where the mbuf should be
		split.
	@param how Blocking or non-blocking.
	@param new_mbuf Upon success, the second half of the split mbuf
		chain.
	@result 0 upon success otherwise the errno error. In the case of
		failure, the original mbuf chain passed in to src will be
		preserved.
 */
extern errno_t mbuf_split(mbuf_t src, size_t offset, mbuf_how_t how,
    mbuf_t *new_mbuf);

/*!
	@function mbuf_pullup
	@discussion Move the next len bytes in to mbuf from other mbufs in
		the chain. This is commonly used to get the IP and TCP or UDP
		header contiguous in the first mbuf. If mbuf_pullup fails, the
		entire mbuf chain will be freed.
	@param mbuf The mbuf in the chain the data should be contiguous in.
	@param len The number of bytes to pull from the next mbuf(s).
	@result 0 upon success otherwise the errno error. In the case of an
		error, the mbuf chain has been freed.
 */
extern errno_t mbuf_pullup(mbuf_t *mbuf, size_t len);

/*!
	@function mbuf_pulldown
	@discussion Make length bytes at offset in the mbuf chain
		contiguous. Nothing before offset bytes in the chain will be
		modified. Upon return, location will be the mbuf the data is
		contiguous in and offset will be the offset in that mbuf at
		which the data is located.  In the case of a failure, the mbuf
		chain will be freed.
	@param src The start of the mbuf chain.
	@param offset Pass in a pointer to a value with the offset of the
		data you're interested in making contiguous. Upon success, this
		will be overwritten with the offset from the mbuf returned in
		location.
	@param length The length of data that should be made contiguous.
	@param location Upon success, *location will be the mbuf the data is
		in.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_pulldown(mbuf_t src, size_t *offset, size_t length,
    mbuf_t *location);

/*!
	@function mbuf_adj
	@discussion Trims len bytes from the mbuf. If the length is greater
		than zero, the bytes are trimmed from the front of the mbuf. If
		the length is less than zero, the bytes are trimmed from the end
		of the mbuf chain.
	@param mbuf The mbuf chain to trim.
	@param len The number of bytes to trim from the mbuf chain.
 */
extern void mbuf_adj(mbuf_t mbuf, int len);

/*!
	@function mbuf_adjustlen
	@discussion Adds amount to the mbuf len. Verifies that the new
		length is valid (greater than or equal to zero and less than
		maximum amount of data that may be stored in the mbuf). This
		function will not adjust the packet header length field or
		affect any other mbufs in a chain.
	@param mbuf The mbuf to adjust.
	@param amount The number of bytes increment the length by.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_adjustlen(mbuf_t mbuf, int amount);

/*!
	@function mbuf_concatenate
	@discussion Concatenate mbuf chain src to dst using m_next and return
		a chain which represents the concatenated chain.  The routine
		does not prevent two chains of different mbuf types to be
		concatenated, nor does it modify any packet header in the
		destination chain.  Therefore, it's the responsibility of the
		caller to ensure that the resulted concatenated mbuf chain is
		correct for further usages.
	@param dst The destination mbuf chain.
	@param src The source mbuf chain.
	@result A pointer to the head of the concatenated mbuf chain.  This
		should be treated as the updated destination mbuf chain; the
		caller must no longer refer to the original src or dst mbuf
		chain.  Otherwise it returns NULL if the original dst mbuf
		chain is NULL.
 */
extern mbuf_t mbuf_concatenate(mbuf_t dst, mbuf_t src);

/*!
	@function mbuf_copydata
	@discussion Copies data out of an mbuf in to a specified buffer. If
		the data is stored in a chain of mbufs, the data will be copied
		from each mbuf in the chain until length bytes have been copied.
	@param mbuf The mbuf chain to copy data out of.
	@param offset The offset in to the mbuf to start copying.
	@param length The number of bytes to copy.
	@param out_data A pointer to the location where the data will be
		copied.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_copydata(const mbuf_t mbuf, size_t offset, size_t length,
    void *out_data);

/*!
	@function mbuf_copyback
	@discussion Copies data from a buffer to an mbuf chain.
		mbuf_copyback will grow the chain to fit the specified buffer.

		If mbuf_copydata is unable to allocate enough mbufs to grow the
		chain, ENOBUFS will be returned. The mbuf chain will be shorter
		than expected but all of the data up to the end of the mbuf
		chain will be valid.

		If an offset is specified, mbuf_copyback will skip that many
		bytes in the mbuf chain before starting to write the buffer in
		to the chain. If the mbuf chain does not contain this many
		bytes, mbufs will be allocated to create the space.
	@param mbuf The first mbuf in the chain to copy the data in to.
	@param offset Offset in bytes to skip before copying data.
	@param length The length, in bytes, of the data to copy in to the mbuf
		chain.
	@param data A pointer to data in the kernel's address space.
	@param how Blocking or non-blocking.
	@result 0 upon success, EINVAL or ENOBUFS upon failure.
 */
extern errno_t mbuf_copyback(mbuf_t mbuf, size_t offset, size_t length,
    const void *data, mbuf_how_t how);

/*!
	@function mbuf_mclhasreference
	@discussion Check if a cluster of an mbuf is referenced by another mbuf.
		References may be taken, for example, as a result of a call to 
		mbuf_split or mbuf_copym
	@param mbuf The mbuf with the cluster to test.
	@result 0 if there is no reference by another mbuf, 1 otherwise.
 */
extern int mbuf_mclhasreference(mbuf_t mbuf);


/* mbuf header */

/*!
	@function mbuf_next
	@discussion Returns the next mbuf in the chain.
	@param mbuf The mbuf.
	@result The next mbuf in the chain.
 */
extern mbuf_t mbuf_next(const mbuf_t mbuf);

/*!
	@function mbuf_setnext
	@discussion Sets the next mbuf in the chain.
	@param mbuf The mbuf.
	@param next The new next mbuf.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_setnext(mbuf_t mbuf, mbuf_t next);

/*!
	@function mbuf_nextpkt
	@discussion Gets the next packet from the mbuf.
	@param mbuf The mbuf.
	@result The nextpkt.
 */
extern mbuf_t mbuf_nextpkt(const mbuf_t mbuf);

/*!
	@function mbuf_setnextpkt
	@discussion Sets the next packet attached to this mbuf.
	@param mbuf The mbuf.
	@param nextpkt The new next packet.
 */
extern void mbuf_setnextpkt(mbuf_t mbuf, mbuf_t nextpkt);

/*!
	@function mbuf_len
	@discussion Gets the length of data in this mbuf.
	@param mbuf The mbuf.
	@result The length.
 */
extern size_t mbuf_len(const mbuf_t mbuf);

/*!
	@function mbuf_setlen
	@discussion Sets the length of data in this packet. Be careful to
		not set the length over the space available in the mbuf.
	@param mbuf The mbuf.
	@param len The new length.
	@result 0 upon success otherwise the errno error.
 */
extern void mbuf_setlen(mbuf_t mbuf, size_t len);

/*!
	@function mbuf_maxlen
	@discussion Retrieves the maximum length of data that may be stored
		in this mbuf. This value assumes that the data pointer was set
		to the start of the possible range for that pointer
		(mbuf_data_start).
	@param mbuf The mbuf.
	@result The maximum lenght of data for this mbuf.
 */
extern size_t mbuf_maxlen(const mbuf_t mbuf);

/*!
	@function mbuf_type
	@discussion Gets the type of mbuf.
	@param mbuf The mbuf.
	@result The type.
 */
extern mbuf_type_t mbuf_type(const mbuf_t mbuf);

/*!
	@function mbuf_settype
	@discussion Sets the type of mbuf.
	@param mbuf The mbuf.
	@param new_type The new type.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_settype(mbuf_t mbuf, mbuf_type_t new_type);

/*!
	@function mbuf_flags
	@discussion Returns the set flags.
	@param mbuf The mbuf.
	@result The flags.
 */
extern mbuf_flags_t mbuf_flags(const mbuf_t mbuf);

/*!
	@function mbuf_setflags
	@discussion Sets the set of set flags.
	@param mbuf The mbuf.
	@param flags The flags that should be set, all other flags will be cleared.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_setflags(mbuf_t mbuf, mbuf_flags_t flags);

/*!
	@function mbuf_setflags_mask
	@discussion Useful for setting or clearing individual flags. Easier
		than calling mbuf_setflags(m, mbuf_flags(m) | M_FLAG).
	@param mbuf The mbuf.
	@param flags The flags that should be set or cleared.
	@param mask The mask controlling which flags will be modified.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_setflags_mask(mbuf_t mbuf, mbuf_flags_t flags,
    mbuf_flags_t mask);

/*!
	@function mbuf_copy_pkthdr
	@discussion Copies the packet header from src to dest.
	@param src The mbuf from which the packet header will be copied.
	@param mbuf The mbuf to which the packet header will be copied.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_copy_pkthdr(mbuf_t dest, const mbuf_t src);

/*!
	@function mbuf_pkthdr_len
	@discussion Returns the length as reported by the packet header.
	@param mbuf The mbuf containing the packet header with the length to
		be changed.
	@result The length, in bytes, of the packet.
 */
extern size_t mbuf_pkthdr_len(const mbuf_t mbuf);

/*!
	@function mbuf_pkthdr_setlen
	@discussion Sets the length of the packet in the packet header.
	@param mbuf The mbuf containing the packet header.
	@param len The new length of the packet.
 */
extern void mbuf_pkthdr_setlen(mbuf_t mbuf, size_t len);

/*!
	@function mbuf_pkthdr_adjustlen
	@discussion Adjusts the length of the packet in the packet header.
	@param mbuf The mbuf containing the packet header.
	@param amount The number of bytes to adjust the packet header length
		field by.
 */
extern void mbuf_pkthdr_adjustlen(mbuf_t mbuf, int amount);

/*!
	@function mbuf_pkthdr_rcvif
	@discussion Returns the interface the packet was received on. This
		funciton does not modify the reference count of the interface.
		The interface is only valid for as long as the mbuf is not freed
		and the rcvif for the mbuf is not changed. Take a reference on
		the interface that you will release later before doing any of
		the following: free the mbuf, change the rcvif, pass the mbuf to
		any function that may free the mbuf or change the rcvif.
	@param mbuf The mbuf containing the packet header.
	@result A reference to the interface.
 */
extern ifnet_t mbuf_pkthdr_rcvif(const mbuf_t mbuf);

/*!
	@function mbuf_pkthdr_setrcvif
	@discussion Sets the interface the packet was received on.
	@param mbuf The mbuf containing the packet header.
	@param ifnet A reference to an interface.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_pkthdr_setrcvif(mbuf_t mbuf, ifnet_t ifp);

/*!
	@function mbuf_pkthdr_header
	@discussion Returns a pointer to the packet header.
	@param mbuf The mbuf containing the packet header.
	@result A pointer to the packet header.
 */
extern void *mbuf_pkthdr_header(const mbuf_t mbuf);

/*!
	@function mbuf_pkthdr_setheader
	@discussion Sets the pointer to the packet header.
	@param mbuf The mbuf containing the packet header.
	@param ifnet A pointer to the header.
	@result 0 upon success otherwise the errno error.
 */
extern void mbuf_pkthdr_setheader(mbuf_t mbuf, void *header);

/* Checksums */

/*!
	@function mbuf_inbound_modified
	@discussion This function will clear the checksum flags to indicate
		that a hardware checksum should not be used. Any filter
		modifying data should call this function on an mbuf before
		passing the packet up the stack. If a filter modifies a packet
		in a way that affects any checksum, the filter is responsible
		for either modifying the checksum to compensate for the changes
		or verifying the checksum before making the changes and then
		modifying the data and calculating a new checksum only if the
		original checksum was valid.
	@param mbuf The mbuf that has been modified.
 */
extern void mbuf_inbound_modified(mbuf_t mbuf);

/*!
	@function mbuf_outbound_finalize
	@discussion This function will "finalize" the packet allowing your
		code to inspect the final packet.

		There are a number of operations that are performed in hardware,
		such as calculating checksums. This function will perform in
		software the various opterations that were scheduled to be done
		in hardware. Future operations may include IPSec processing or
		vlan support. If you are redirecting a packet to a new interface
		which may not have the same hardware support or encapsulating
		the packet, you should call this function to force the stack to
		calculate and fill out the checksums. This will bypass hardware
		checksums but give you a complete packet to work with. If you
		need to inspect aspects of the packet which may be generated by
		hardware, you must call this function to get an aproximate final
		packet. If you plan to modify the packet in any way, you should
		call this function.

		This function should be called before modifying any outbound
		packets.

		This function may be called at various levels, in some cases
		additional headers may have already been prepended, such as the
		case of a packet seen by an interface filter. To handle this,
		the caller must pass the protocol family of the packet as well
		as the offset from the start of the packet to the protocol
		header.
	@param mbuf The mbuf that should be finalized.
	@param protocol_family The protocol family of the packet in the
		mbuf.
	@param protocol_offset The offset from the start of the mbuf to the
		protocol header. For an IP packet with an ethernet header, this
		would be the length of an ethernet header.
 */
extern void mbuf_outbound_finalize(mbuf_t mbuf, u_int32_t protocol_family,
    size_t protocol_offset);

/*!
	@function mbuf_set_vlan_tag
	@discussion This function is used by interfaces that support vlan
		tagging in hardware. This function will set properties in the
		mbuf to indicate which vlan the packet was received for.
	@param mbuf The mbuf containing the packet.
	@param vlan The protocol family of the aux data to add.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_set_vlan_tag(mbuf_t mbuf, u_int16_t vlan);

/*!
	@function mbuf_get_vlan_tag
	@discussion This function is used by drivers that support hardware
		vlan tagging to determine which vlan this packet belongs to. To
		differentiate between the case where the vlan tag is zero and
		the case where there is no vlan tag, this function will return
		ENXIO when there is no vlan.
	@param mbuf The mbuf containing the packet.
	@param vlan The protocol family of the aux data to add.
	@result 0 upon success otherwise the errno error. ENXIO indicates
		that the vlan tag is not set.
 */
extern errno_t mbuf_get_vlan_tag(mbuf_t mbuf, u_int16_t *vlan);

/*!
	@function mbuf_clear_vlan_tag
	@discussion This function will clear any vlan tag associated with
		the mbuf.
	@param mbuf The mbuf containing the packet.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_clear_vlan_tag(mbuf_t mbuf);

#ifdef KERNEL_PRIVATE
/*
	@function mbuf_set_csum_requested
	@discussion This function is used by the stack to indicate which
		checksums should be calculated in hardware. The stack normally
		sets these flags as the packet is processed in the outbound
		direction. Just before send the packe to the interface, the
		stack will look at these flags and perform any checksums in
		software that are not supported by the interface.
	@param mbuf The mbuf containing the packet.
	@param request Flags indicating which checksums are being requested
		for this packet.
	@param value This parameter is currently unsupported.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_set_csum_requested(mbuf_t mbuf,
    mbuf_csum_request_flags_t request, u_int32_t value);
#endif /* KERNEL_PRIVATE */

/*!
	@function mbuf_get_csum_requested
	@discussion This function is used by the driver to determine which
		checksum operations should be performed in hardware.
	@param mbuf The mbuf containing the packet.
	@param request Flags indicating which checksums are being requested
		for this packet.
	@param value This parameter is currently unsupported.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_get_csum_requested(mbuf_t mbuf,
    mbuf_csum_request_flags_t *request, u_int32_t *value);

/*!
	@function mbuf_get_tso_requested
	@discussion This function is used by the driver to determine which
		checksum operations should be performed in hardware.
	@param mbuf The mbuf containing the packet.
	@param request Flags indicating which values are being requested
		for this packet.
	@param value The requested value.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_get_tso_requested(mbuf_t mbuf,
    mbuf_tso_request_flags_t *request, u_int32_t *value);

/*!
	@function mbuf_clear_csum_requested
	@discussion This function clears the checksum request flags.
	@param mbuf The mbuf containing the packet.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_clear_csum_requested(mbuf_t mbuf);

/*!
	@function mbuf_set_csum_performed
	@discussion This is used by the driver to indicate to the stack which
		checksum operations were performed in hardware.
	@param mbuf The mbuf containing the packet.
	@param flags Flags indicating which hardware checksum operations
		were performed.
	@param value If the MBUF_CSUM_DID_DATA flag is set, value should be
		set to the value of the TCP or UDP header as calculated by the
		hardware.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_set_csum_performed(mbuf_t mbuf,
    mbuf_csum_performed_flags_t flags, u_int32_t value);

#ifdef KERNEL_PRIVATE
/*
	@function mbuf_get_csum_performed
	@discussion This is used by the stack to determine which checksums
		were calculated in hardware on the inbound path.
	@param mbuf The mbuf containing the packet.
	@param flags Flags indicating which hardware checksum operations
		were performed.
	@param value If the MBUF_CSUM_DID_DATA flag is set, value will be
		set to the value of the TCP or UDP header as calculated by the
		hardware.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_get_csum_performed(mbuf_t mbuf,
    mbuf_csum_performed_flags_t *flags, u_int32_t *value);
#endif /* KERNEL_PRIVATE */

/*!
	@function mbuf_get_mlen
	@discussion This routine returns the number of data bytes in a normal
		mbuf, i.e. an mbuf that is not a packet header, nor one with
		an external cluster attached to it.  This is equivalent to the
		legacy MLEN macro.
	@result	The number of bytes of available data.
 */
extern u_int32_t mbuf_get_mlen(void);

/*!
	@function mbuf_get_mhlen
	@discussion This routine returns the number of data bytes in a packet
		header mbuf.  This is equivalent to the legacy MHLEN macro.
	@result	The number of bytes of available data.
 */
extern u_int32_t mbuf_get_mhlen(void);

/*!
	@function mbuf_get_minclsize
	@discussion This routine returns the minimum number of data bytes
		before an external cluster is used.  This is equivalent to the
		legacy MINCLSIZE macro.
	@result	The minimum number of bytes before a cluster will be used.
 */
extern u_int32_t mbuf_get_minclsize(void);

/*!
	@function mbuf_clear_csum_performed
	@discussion Clears the hardware checksum flags and values.
	@param mbuf The mbuf containing the packet.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_clear_csum_performed(mbuf_t mbuf);

/*!
	@function mbuf_inet_cksum
	@discussions Calculates 16-bit 1's complement Internet checksum of the
		transport segment with or without the pseudo header checksum
		of a given IPv4 packet.  If the caller specifies a non-zero
		transport protocol, the checksum returned will also include
		the pseudo header checksum for the corresponding transport
		header.  Otherwise, no header parsing will be done and the
		caller may use this to calculate the Internet checksum of
		an arbitrary span of data.

		This routine does not modify the contents of the packet.  If
		the caller specifies a non-zero protocol and/or offset, the
		routine expects the complete protocol header to be present
		at the beginning of the first mbuf.
	@param mbuf The mbuf (or chain of mbufs) containing the packet.
	@param protocol A zero or non-zero value.  A non-zero value specifies
		the transport protocol used for pseudo header checksum.
	@param offset A zero or non-zero value; if the latter, it specifies
		the offset of the transport header from the beginning of mbuf.
	@param length The total (non-zero) length of the transport segment.
	@param csum Pointer to the checksum variable; upon success, this
		routine will return the calculated Internet checksum through
		this variable.  The caller must set it to a non-NULL value.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_inet_cksum(mbuf_t mbuf, int protocol, u_int32_t offset,
    u_int32_t length, u_int16_t *csum);

/*!
	@function mbuf_inet6_cksum
	@discussions Calculates 16-bit 1's complement Internet checksum of the
		transport segment with or without the pseudo header checksum
		of a given IPv6 packet.  If the caller specifies a non-zero
		transport protocol, the checksum returned will also include
		the pseudo header checksum for the corresponding transport
		header.  Otherwise, no header parsing will be done and the
		caller may use this to calculate the Internet checksum of
		an arbitrary span of data.

		This routine does not modify the contents of the packet.  If
		the caller specifies a non-zero protocol and/or offset, the
		routine expects the complete protocol header(s) to be present
		at the beginning of the first mbuf.
	@param mbuf The mbuf (or chain of mbufs) containing the packet.
	@param protocol A zero or non-zero value.  A non-zero value specifies
		the transport protocol used for pseudo header checksum.
	@param offset A zero or non-zero value; if the latter, it specifies
		the offset of the transport header from the beginning of mbuf.
	@param length The total (non-zero) length of the transport segment.
	@param csum Pointer to the checksum variable; upon success, this
		routine will return the calculated Internet checksum through
		this variable.  The caller must set it to a non-NULL value.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_inet6_cksum(mbuf_t mbuf, int protocol, u_int32_t offset,
    u_int32_t length, u_int16_t *csum);

/* mbuf tags */

/*!
	@function mbuf_tag_id_find
	@discussion Lookup the module id for a string. If there is no module
		id assigned to this string, a new module id will be assigned.
		The string should be the bundle id of the kext. In the case of a
		tag that will be shared across multiple kexts, a common bundle
		id style string should be used.

		The lookup operation is not optimized. A module should call this
		function once during startup and chache the module id. The
		module id will not be resassigned until the machine reboots.
	@param module_string A unique string identifying your module.
		Example: com.apple.nke.SharedIP.
	@param module_id Upon return, a unique identifier for use with
		mbuf_tag_* functions. This identifier is valid until the machine
		is rebooted.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_tag_id_find(const char *module_string,
    mbuf_tag_id_t *module_id);

/*!
	@function mbuf_tag_allocate
	@discussion Allocate an mbuf tag. Mbuf tags allow various portions
		of the stack to tag mbufs with data that will travel with the
		mbuf through the stack.

		Tags may only be added to mbufs with packet headers
		(MBUF_PKTHDR flag is set). Mbuf tags are freed when the mbuf is
		freed or when mbuf_tag_free is called.
	@param mbuf The mbuf to attach this tag to.
	@param module_id A module identifier returned by mbuf_tag_id_find.
	@param type A 16 bit type value. For a given module_id, you can use
		a number of different tag types.
	@param length The length, in bytes, to allocate for storage that
		will be associated with this tag on this mbuf.
	@param how Indicate whether you want to block and wait for memory if
		memory is not immediately available.
	@param data_p Upon successful return, *data_p will point to the
		buffer allocated for the mtag.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_tag_allocate(mbuf_t mbuf, mbuf_tag_id_t module_id,
    mbuf_tag_type_t type, size_t length, mbuf_how_t how, void **data_p);

/*!
	@function mbuf_tag_find
	@discussion Find the data associated with an mbuf tag.
	@param mbuf The mbuf the tag is attached to.
	@param module_id A module identifier returned by mbuf_tag_id_find.
	@param type The 16 bit type of the tag to find.
	@param length Upon success, the length of data will be store in
		*length.
	@param data_p Upon successful return, *data_p will point to the
		buffer allocated for the mtag.
	@result 0 upon success otherwise the errno error.
 */
extern errno_t mbuf_tag_find(mbuf_t mbuf, mbuf_tag_id_t module_id,
    mbuf_tag_type_t type, size_t *length, void **data_p);

/*!
	@function mbuf_tag_free
	@discussion Frees a previously allocated mbuf tag.
	@param mbuf The mbuf the tag was allocated on.
	@param module_id The ID of the tag to free.
	@param type The type of the tag to free.
 */
extern void mbuf_tag_free(mbuf_t mbuf, mbuf_tag_id_t module_id,
    mbuf_tag_type_t type);

/* mbuf stats */

/*!
	@function mbuf_stats
	@discussion Get the mbuf statistics.
	@param stats Storage to copy the stats in to.
 */
extern void mbuf_stats(struct mbuf_stat *stats);


/*!
	@enum mbuf_traffic_class_t
	@abstract Traffic class of a packet
	@discussion Property that represent the category of traffic of a packet. 
		This information may be used by the driver and at the link level.
	@constant MBUF_TC_BE Best effort, normal class.
	@constant MBUF_TC_BK Background, low priority or bulk traffic.
	@constant MBUF_TC_VI Interactive video, constant bit rate, low latency.
	@constant MBUF_TC_VO Interactive voice, constant bit rate, lowest latency.
*/
typedef enum {
#ifdef XNU_KERNEL_PRIVATE
	MBUF_TC_UNSPEC	= -1,		/* Internal: not specified */
#endif
	MBUF_TC_BE		= 0,
	MBUF_TC_BK		= 1,
	MBUF_TC_VI		= 2,
	MBUF_TC_VO		= 3
#ifdef XNU_KERNEL_PRIVATE
        ,
	MBUF_TC_MAX		= 4	/* Internal: traffic class count */
#endif
} mbuf_traffic_class_t;

/*!
	@function mbuf_get_traffic_class
	@discussion Get the traffic class of an mbuf packet
	@param mbuf The mbuf to get the traffic class of.
	@result The traffic class
*/
extern mbuf_traffic_class_t mbuf_get_traffic_class(mbuf_t mbuf);

/*!
	@function mbuf_set_traffic_class
	@discussion Set the traffic class of an mbuf packet.
	@param mbuf The mbuf to set the traffic class on.
	@tc The traffic class
	@result 0 on success, EINVAL if bad paramater is passed
*/
extern errno_t mbuf_set_traffic_class(mbuf_t mbuf, mbuf_traffic_class_t tc);

/*!
	@function mbuf_is_traffic_class_privileged
	@discussion Returns the privileged status of the traffic class
		of the packet specified by the mbuf.
	@param mbuf The mbuf to retrieve the status from.
	@result Non-zero if privileged, 0 otherwise.
 */
extern int mbuf_is_traffic_class_privileged(mbuf_t mbuf);

#ifdef KERNEL_PRIVATE
/*!
	@enum mbuf_svc_class_t
	@abstract Service class of a packet
	@discussion Property that represents the category of service
		of a packet. This information may be used by the driver
		and at the link level.
	@constant MBUF_SC_BK_SYS "Background System-Initiated", high delay
		tolerant, high loss tolerant, elastic flow, variable size &
		long-lived.
	@constant MBUF_SC_BK "Background", user-initiated, high delay tolerant,
		high loss tolerant, elastic flow, variable size.  This level
		corresponds to WMM access class "BG", or MBUF_TC_BK.
	@constant MBUF_SC_BE "Best Effort", unclassified/standard.  This is
		the default service class; pretty much a mix of everything.
		This level corresponds to WMM access class "BE" or MBUF_TC_BE.
	@constant MBUF_SC_RD
		"Responsive Data", a notch higher than "Best Effort", medium
		delay tolerant, medium loss tolerant, elastic flow, bursty,
		long-lived.
	@constant MBUF_SC_OAM "Operations, Administration, and Management",
		medium delay tolerant, low-medium loss tolerant, elastic &
		inelastic flows, variable size.
	@constant MBUF_SC_AV "Multimedia Audio/Video Streaming", medium delay
		tolerant, low-medium loss tolerant, elastic flow, constant
		packet interval, variable rate & size.
	@constant MBUF_SC_RV "Responsive Multimedia Audio/Video", low delay
		tolerant, low-medium loss tolerant, elastic flow, variable
		packet interval, rate and size.
	@constant MBUF_SC_VI "Interactive Video", low delay tolerant, low-
		medium loss tolerant, elastic flow, constant packet interval,
		variable rate & size.  This level corresponds to WMM access
		class "VI" or MBUF_TC_VI.
	@constant MBUF_SC_VO "Interactive Voice", low delay tolerant, low loss
		tolerant, inelastic flow, constant packet rate, somewhat fixed
		size.  This level corresponds to WMM access class "VO" or
		MBUF_TC_VO.
	@constant MBUF_SC_CTL "Network Control", low delay tolerant, low loss
		tolerant, inelastic flow, rate is short & burst, variable size.
*/
typedef enum {
#ifdef XNU_KERNEL_PRIVATE
	MBUF_SC_UNSPEC		= -1,		/* Internal: not specified */
#endif
	MBUF_SC_BK_SYS		= 0x00080090,	/* lowest class */
	MBUF_SC_BK		= 0x00100080,

	MBUF_SC_BE		= 0x00000000,
	MBUF_SC_RD		= 0x00180010,
	MBUF_SC_OAM		= 0x00200020,

	MBUF_SC_AV		= 0x00280120,
	MBUF_SC_RV		= 0x00300110,
	MBUF_SC_VI		= 0x00380100,

	MBUF_SC_VO		= 0x00400180,
	MBUF_SC_CTL		= 0x00480190,	/* highest class */
} mbuf_svc_class_t;

/*!
	@function mbuf_get_service_class
	@discussion Get the service class of an mbuf packet
	@param mbuf The mbuf to get the service class of.
	@result The service class
*/
extern mbuf_svc_class_t mbuf_get_service_class(mbuf_t mbuf);

/*!
	@function mbuf_set_servicec_class
	@discussion Set the service class of an mbuf packet.
	@param mbuf The mbuf to set the service class on.
	@sc The service class
	@result 0 on success, EINVAL if bad paramater is passed
*/
extern errno_t mbuf_set_service_class(mbuf_t mbuf, mbuf_svc_class_t sc);

/*!
	@function mbuf_is_service_class_privileged
	@discussion Returns the privileged status of the service class
		of the packet specified by the mbuf.
	@param mbuf The mbuf to retrieve the status from.
	@result Non-zero if privileged, 0 otherwise.
 */
extern int mbuf_is_service_class_privileged(mbuf_t mbuf);

/*
	@enum mbuf_pkthdr_aux_flags_t
	@abstract Constants defining mbuf auxiliary flags.  Only the flags
		listed below can be retrieved.
	@constant MBUF_PKTAUXF_INET_RESOLVE_RTR Indicates this is an ARP
		request packet, whose target is the address of the default
		IPv4 router.
	@constant MBUF_PKTAUXF_INET6_RESOLVE_RTR Indicates this is an ICMPv6
		Neighbor Solicitation packet, whose target is the address of
		the default IPv6 router.
 */
enum {
	MBUF_PKTAUXF_INET_RESOLVE_RTR	= 0x0004,
	MBUF_PKTAUXF_INET6_RESOLVE_RTR	= 0x0008,
};
typedef u_int32_t mbuf_pkthdr_aux_flags_t;

/*
	@function mbuf_pkthdr_aux_flags
	@discussion Returns the auxiliary flags of a packet.
	@param mbuf The mbuf containing the packet header.
	@param paux_flags Pointer to mbuf_pkthdr_aux_flags_t variable.
	@result 0 upon success otherwise the errno error.
*/
extern errno_t mbuf_pkthdr_aux_flags(mbuf_t mbuf,
    mbuf_pkthdr_aux_flags_t *paux_flags);
#endif /* KERNEL_PRIVATE */

/* IF_QUEUE interaction */

#define IF_ENQUEUE_MBUF(ifq, m) {					\
	mbuf_setnextpkt((m), 0);					\
	if ((ifq)->ifq_tail == 0)					\
		(ifq)->ifq_head = (m);					\
	else								\
		mbuf_setnextpkt((mbuf_t)(ifq)->ifq_tail, (m));		\
	(ifq)->ifq_tail = (m);						\
	(ifq)->ifq_len++;						\
}

#define	IF_PREPEND_MBUF(ifq, m) {					\
	mbuf_setnextpkt((m), (ifq)->ifq_head);				\
	if ((ifq)->ifq_tail == 0)					\
		(ifq)->ifq_tail = (m);					\
	(ifq)->ifq_head = (m);						\
	(ifq)->ifq_len++;						\
}

#define	IF_DEQUEUE_MBUF(ifq, m) {					\
	(m) = (ifq)->ifq_head;						\
	if (m) {							\
		if (((ifq)->ifq_head = mbuf_nextpkt((m))) == 0)		\
			(ifq)->ifq_tail = 0;				\
		mbuf_setnextpkt((m), 0);				\
		(ifq)->ifq_len--;					\
	}								\
}

__END_DECLS
#endif /* __KPI_MBUF__ */
