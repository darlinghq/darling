/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2013 Apple Computer, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*! @header     DNS Service Discovery
 *
 * @discussion  This section describes the functions, callbacks, and data structures
 *              that make up the DNS Service Discovery API.
 *
 *              The DNS Service Discovery API is part of Bonjour, Apple's implementation
 *              of zero-configuration networking (ZEROCONF).
 *
 *              Bonjour allows you to register a network service, such as a
 *              printer or file server, so that it can be found by name or browsed
 *              for by service type and domain. Using Bonjour, applications can
 *              discover what services are available on the network, along with
 *              all the information -- such as name, IP address, and port --
 *              necessary to access a particular service.
 *
 *              In effect, Bonjour combines the functions of a local DNS server and
 *              AppleTalk. Bonjour allows applications to provide user-friendly printer
 *              and server browsing, among other things, over standard IP networks.
 *              This behavior is a result of combining protocols such as multicast and
 *              DNS to add new functionality to the network (such as multicast DNS).
 *
 *              Bonjour gives applications easy access to services over local IP
 *              networks without requiring the service or the application to support
 *              an AppleTalk or a Netbeui stack, and without requiring a DNS server
 *              for the local network.
 */

/* _DNS_SD_H contains the API version number for this header file
 * The API version defined in this header file symbol allows for compile-time
 * checking, so that C code building with earlier versions of the header file
 * can avoid compile errors trying to use functions that aren't even defined
 * in those earlier versions. Similar checks may also be performed at run-time:
 *  => weak linking -- to avoid link failures if run with an earlier
 *     version of the library that's missing some desired symbol, or
 *  => DNSServiceGetProperty(DaemonVersion) -- to verify whether the running daemon
 *     ("system service" on Windows) meets some required minimum functionality level.
 */

#ifndef _DNS_SD_H
#define _DNS_SD_H 5610101

#ifdef  __cplusplus
extern "C" {
#endif

/* Set to 1 if libdispatch is supported
 * Note: May also be set by project and/or Makefile
 */
#ifndef _DNS_SD_LIBDISPATCH
#define _DNS_SD_LIBDISPATCH  1
#endif /* ndef _DNS_SD_LIBDISPATCH */

/* standard calling convention under Win32 is __stdcall */
/* Note: When compiling Intel EFI (Extensible Firmware Interface) under MS Visual Studio, the */
/* _WIN32 symbol is defined by the compiler even though it's NOT compiling code for Windows32 */
#if defined(_WIN32) && !defined(EFI32) && !defined(EFI64)
#define DNSSD_API __stdcall
#else
#define DNSSD_API
#endif

/* stdint.h does not exist on FreeBSD 4.x; its types are defined in sys/types.h instead */
#if defined(__FreeBSD__) && (__FreeBSD__ < 5)
#include <sys/types.h>

/* Likewise, on Sun, standard integer types are in sys/types.h */
#elif defined(__sun__)
#include <sys/types.h>

/* EFI does not have stdint.h, or anything else equivalent */
#elif defined(EFI32) || defined(EFI64) || defined(EFIX64)
#include "Tiano.h"
#if !defined(_STDINT_H_)
typedef UINT8 uint8_t;
typedef INT8 int8_t;
typedef UINT16 uint16_t;
typedef INT16 int16_t;
typedef UINT32 uint32_t;
typedef INT32 int32_t;
#endif
/* Windows has its own differences */
#elif defined(_WIN32)
#include <windows.h>
#define _UNUSED
#ifndef _MSL_STDINT_H
typedef UINT8 uint8_t;
typedef INT8 int8_t;
typedef UINT16 uint16_t;
typedef INT16 int16_t;
typedef UINT32 uint32_t;
typedef INT32 int32_t;
#endif

/* All other Posix platforms use stdint.h */
#else
#include <stdint.h>
#endif

#if _DNS_SD_LIBDISPATCH
#include <dispatch/dispatch.h>
#endif

/* DNSServiceRef, DNSRecordRef
 *
 * Opaque internal data types.
 * Note: client is responsible for serializing access to these structures if
 * they are shared between concurrent threads.
 */

typedef struct _DNSServiceRef_t *DNSServiceRef;
typedef struct _DNSRecordRef_t *DNSRecordRef;

struct sockaddr;

/*! @enum General flags
 * Most DNS-SD API functions and callbacks include a DNSServiceFlags parameter.
 * As a general rule, any given bit in the 32-bit flags field has a specific fixed meaning,
 * regardless of the function or callback being used. For any given function or callback,
 * typically only a subset of the possible flags are meaningful, and all others should be zero.
 * The discussion section for each API call describes which flags are valid for that call
 * and callback. In some cases, for a particular call, it may be that no flags are currently
 * defined, in which case the DNSServiceFlags parameter exists purely to allow future expansion.
 * In all cases, developers should expect that in future releases, it is possible that new flag
 * values will be defined, and write code with this in mind. For example, code that tests
 *     if (flags == kDNSServiceFlagsAdd) ...
 * will fail if, in a future release, another bit in the 32-bit flags field is also set.
 * The reliable way to test whether a particular bit is set is not with an equality test,
 * but with a bitwise mask:
 *     if (flags & kDNSServiceFlagsAdd) ...
 * With the exception of kDNSServiceFlagsValidate, each flag can be valid(be set) 
 * EITHER only as an input to one of the DNSService*() APIs OR only as an output
 * (provide status) through any of the callbacks used. For example, kDNSServiceFlagsAdd
 * can be set only as an output in the callback, whereas the kDNSServiceFlagsIncludeP2P
 * can be set only as an input to the DNSService*() APIs. See comments on kDNSServiceFlagsValidate  
 * defined in enum below.
 */
enum
{
    kDNSServiceFlagsMoreComing          = 0x1,
    /* MoreComing indicates to a callback that at least one more result is
     * queued and will be delivered following immediately after this one.
     * When the MoreComing flag is set, applications should not immediately
     * update their UI, because this can result in a great deal of ugly flickering
     * on the screen, and can waste a great deal of CPU time repeatedly updating
     * the screen with content that is then immediately erased, over and over.
     * Applications should wait until MoreComing is not set, and then
     * update their UI when no more changes are imminent.
     * When MoreComing is not set, that doesn't mean there will be no more
     * answers EVER, just that there are no more answers immediately
     * available right now at this instant. If more answers become available
     * in the future they will be delivered as usual.
     */

    kDNSServiceFlagsAdd                 = 0x2,
    kDNSServiceFlagsDefault             = 0x4,
    /* Flags for domain enumeration and browse/query reply callbacks.
     * "Default" applies only to enumeration and is only valid in
     * conjunction with "Add". An enumeration callback with the "Add"
     * flag NOT set indicates a "Remove", i.e. the domain is no longer
     * valid.
     */

    kDNSServiceFlagsNoAutoRename        = 0x8,
    /* Flag for specifying renaming behavior on name conflict when registering
     * non-shared records. By default, name conflicts are automatically handled
     * by renaming the service. NoAutoRename overrides this behavior - with this
     * flag set, name conflicts will result in a callback. The NoAutorename flag
     * is only valid if a name is explicitly specified when registering a service
     * (i.e. the default name is not used.)
     */

    kDNSServiceFlagsShared              = 0x10,
    kDNSServiceFlagsUnique              = 0x20,
    /* Flag for registering individual records on a connected
     * DNSServiceRef. Shared indicates that there may be multiple records
     * with this name on the network (e.g. PTR records). Unique indicates that the
     * record's name is to be unique on the network (e.g. SRV records).
     */

    kDNSServiceFlagsBrowseDomains       = 0x40,
    kDNSServiceFlagsRegistrationDomains = 0x80,
    /* Flags for specifying domain enumeration type in DNSServiceEnumerateDomains.
     * BrowseDomains enumerates domains recommended for browsing, RegistrationDomains
     * enumerates domains recommended for registration.
     */

    kDNSServiceFlagsLongLivedQuery      = 0x100,
    /* Flag for creating a long-lived unicast query for the DNSServiceQueryRecord call. */

    kDNSServiceFlagsAllowRemoteQuery    = 0x200,
    /* Flag for creating a record for which we will answer remote queries
     * (queries from hosts more than one hop away; hosts not directly connected to the local link).
     */

    kDNSServiceFlagsForceMulticast      = 0x400,
    /* Flag for signifying that a query or registration should be performed exclusively via multicast
     * DNS, even for a name in a domain (e.g. foo.apple.com.) that would normally imply unicast DNS.
     */

    kDNSServiceFlagsForce               = 0x800,    // This flag is deprecated.

    kDNSServiceFlagsKnownUnique         = 0x800,
    /* 
     * Client guarantees that record names are unique, so we can skip sending out initial
     * probe messages.  Standard name conflict resolution is still done if a conflict is discovered.
     * Currently only valid for a DNSServiceRegister call.
     */

    kDNSServiceFlagsReturnIntermediates = 0x1000,
    /* Flag for returning intermediate results.
     * For example, if a query results in an authoritative NXDomain (name does not exist)
     * then that result is returned to the client. However the query is not implicitly
     * cancelled -- it remains active and if the answer subsequently changes
     * (e.g. because a VPN tunnel is subsequently established) then that positive
     * result will still be returned to the client.
     * Similarly, if a query results in a CNAME record, then in addition to following
     * the CNAME referral, the intermediate CNAME result is also returned to the client.
     * When this flag is not set, NXDomain errors are not returned, and CNAME records
     * are followed silently without informing the client of the intermediate steps.
     * (In earlier builds this flag was briefly calledkDNSServiceFlagsReturnCNAME)
     */

    kDNSServiceFlagsNonBrowsable        = 0x2000,
    /* A service registered with the NonBrowsable flag set can be resolved using
     * DNSServiceResolve(), but will not be discoverable using DNSServiceBrowse().
     * This is for cases where the name is actually a GUID; it is found by other means;
     * there is no end-user benefit to browsing to find a long list of opaque GUIDs.
     * Using the NonBrowsable flag creates SRV+TXT without the cost of also advertising
     * an associated PTR record.
     */

    kDNSServiceFlagsShareConnection     = 0x4000,
    /* For efficiency, clients that perform many concurrent operations may want to use a
     * single Unix Domain Socket connection with the background daemon, instead of having a
     * separate connection for each independent operation. To use this mode, clients first
     * call DNSServiceCreateConnection(&MainRef) to initialize the main DNSServiceRef.
     * For each subsequent operation that is to share that same connection, the client copies
     * the MainRef, and then passes the address of that copy, setting the ShareConnection flag
     * to tell the library that this DNSServiceRef is not a typical uninitialized DNSServiceRef;
     * it's a copy of an existing DNSServiceRef whose connection information should be reused.
     *
     * For example:
     *
     * DNSServiceErrorType error;
     * DNSServiceRef MainRef;
     * error = DNSServiceCreateConnection(&MainRef);
     * if (error) ...
     * DNSServiceRef BrowseRef = MainRef;  // Important: COPY the primary DNSServiceRef first...
     * error = DNSServiceBrowse(&BrowseRef, kDNSServiceFlagsShareConnection, ...); // then use the copy
     * if (error) ...
     * ...
     * DNSServiceRefDeallocate(BrowseRef); // Terminate the browse operation
     * DNSServiceRefDeallocate(MainRef);   // Terminate the shared connection
     * Also see Point 4.(Don't Double-Deallocate if the MainRef has been Deallocated) in Notes below:
     *
     * Notes:
     *
     * 1. Collective kDNSServiceFlagsMoreComing flag
     * When callbacks are invoked using a shared DNSServiceRef, the
     * kDNSServiceFlagsMoreComing flag applies collectively to *all* active
     * operations sharing the same parent DNSServiceRef. If the MoreComing flag is
     * set it means that there are more results queued on this parent DNSServiceRef,
     * but not necessarily more results for this particular callback function.
     * The implication of this for client programmers is that when a callback
     * is invoked with the MoreComing flag set, the code should update its
     * internal data structures with the new result, and set a variable indicating
     * that its UI needs to be updated. Then, later when a callback is eventually
     * invoked with the MoreComing flag not set, the code should update *all*
     * stale UI elements related to that shared parent DNSServiceRef that need
     * updating, not just the UI elements related to the particular callback
     * that happened to be the last one to be invoked.
     *
     * 2. Canceling operations and kDNSServiceFlagsMoreComing
     * Whenever you cancel any operation for which you had deferred UI updates
     * waiting because of a kDNSServiceFlagsMoreComing flag, you should perform
     * those deferred UI updates. This is because, after cancelling the operation,
     * you can no longer wait for a callback *without* MoreComing set, to tell
     * you do perform your deferred UI updates (the operation has been canceled,
     * so there will be no more callbacks). An implication of the collective
     * kDNSServiceFlagsMoreComing flag for shared connections is that this
     * guideline applies more broadly -- any time you cancel an operation on
     * a shared connection, you should perform all deferred UI updates for all
     * operations sharing that connection. This is because the MoreComing flag
     * might have been referring to events coming for the operation you canceled,
     * which will now not be coming because the operation has been canceled.
     *
     * 3. Only share DNSServiceRef's created with DNSServiceCreateConnection
     * Calling DNSServiceCreateConnection(&ref) creates a special shareable DNSServiceRef.
     * DNSServiceRef's created by other calls like DNSServiceBrowse() or DNSServiceResolve()
     * cannot be shared by copying them and using kDNSServiceFlagsShareConnection.
     *
     * 4. Don't Double-Deallocate if the MainRef has been Deallocated
     * Calling DNSServiceRefDeallocate(ref) for a particular operation's DNSServiceRef terminates
     * just that operation. Calling DNSServiceRefDeallocate(ref) for the main shared DNSServiceRef
     * (the parent DNSServiceRef, originally created by DNSServiceCreateConnection(&ref))
     * automatically terminates the shared connection and all operations that were still using it.
     * After doing this, DO NOT then attempt to deallocate any remaining subordinate DNSServiceRef's.
     * The memory used by those subordinate DNSServiceRef's has already been freed, so any attempt
     * to do a DNSServiceRefDeallocate (or any other operation) on them will result in accesses
     * to freed memory, leading to crashes or other equally undesirable results.
     *
     * 5. Thread Safety
     * The dns_sd.h API does not presuppose any particular threading model, and consequently
     * does no locking of its own (which would require linking some specific threading library).
     * If client code calls API routines on the same DNSServiceRef concurrently
     * from multiple threads, it is the client's responsibility to use a mutext
     * lock or take similar appropriate precautions to serialize those calls.
     */

    kDNSServiceFlagsSuppressUnusable    = 0x8000,
    /*
     * This flag is meaningful only in DNSServiceQueryRecord which suppresses unusable queries on the
     * wire. If "hostname" is a wide-area unicast DNS hostname (i.e. not a ".local." name)
     * but this host has no routable IPv6 address, then the call will not try to look up IPv6 addresses
     * for "hostname", since any addresses it found would be unlikely to be of any use anyway. Similarly,
     * if this host has no routable IPv4 address, the call will not try to look up IPv4 addresses for
     * "hostname".
     */

    kDNSServiceFlagsTimeout            = 0x10000,
    /*
     * When kDNServiceFlagsTimeout is passed to DNSServiceQueryRecord or DNSServiceGetAddrInfo, the query is
     * stopped after a certain number of seconds have elapsed. The time at which the query will be stopped
     * is determined by the system and cannot be configured by the user. The query will be stopped irrespective
     * of whether a response was given earlier or not. When the query is stopped, the callback will be called
     * with an error code of kDNSServiceErr_Timeout and a NULL sockaddr will be returned for DNSServiceGetAddrInfo
     * and zero length rdata will be returned for DNSServiceQueryRecord.
     */

    kDNSServiceFlagsIncludeP2P          = 0x20000,
    /*
     * Include P2P interfaces when kDNSServiceInterfaceIndexAny is specified.
     * By default, specifying kDNSServiceInterfaceIndexAny does not include P2P interfaces.
     */

    kDNSServiceFlagsWakeOnResolve      = 0x40000,
    /*
    * This flag is meaningful only in DNSServiceResolve. When set, it tries to send a magic packet
    * to wake up the client.
    */

    kDNSServiceFlagsBackgroundTrafficClass  = 0x80000,
    /*
    * This flag is meaningful for Unicast DNS queries. When set, it uses the background traffic 
    * class for packets that service the request.
    */

    kDNSServiceFlagsIncludeAWDL      = 0x100000,
   /*
    * Include AWDL interface when kDNSServiceInterfaceIndexAny is specified.
    */

    kDNSServiceFlagsValidate               = 0x200000,
   /*
    * This flag is meaningful in DNSServiceGetAddrInfo and DNSServiceQueryRecord. This is the ONLY flag to be valid 
    * as an input to the APIs and also an output through the callbacks in the APIs.
    *
    * When this flag is passed to DNSServiceQueryRecord and DNSServiceGetAddrInfo to resolve unicast names, 
    * the response  will be validated using DNSSEC. The validation results are delivered using the flags field in 
    * the callback and kDNSServiceFlagsValidate is marked in the flags to indicate that DNSSEC status is also available.
    * When the callback is called to deliver the query results, the validation results may or may not be available. 
    * If it is not delivered along with the results, the validation status is delivered when the validation completes.
    * 
    * When the validation results are delivered in the callback, it is indicated by marking the flags with
    * kDNSServiceFlagsValidate and kDNSServiceFlagsAdd along with the DNSSEC status flags (described below) and a NULL
    * sockaddr will be returned for DNSServiceGetAddrInfo and zero length rdata will be returned for DNSServiceQueryRecord.
    * DNSSEC validation results are for the whole RRSet and not just individual records delivered in the callback. When
    * kDNSServiceFlagsAdd is not set in the flags, applications should implicitly assume that the DNSSEC status of the 
    * RRSet that has been delivered up until that point is not valid anymore, till another callback is called with
    * kDNSServiceFlagsAdd and kDNSServiceFlagsValidate.
    *
    * The following four flags indicate the status of the DNSSEC validation and marked in the flags field of the callback.
    * When any of the four flags is set, kDNSServiceFlagsValidate will also be set. To check the validation status, the 
    * other applicable output flags should be masked. See kDNSServiceOutputFlags below.
    */

    kDNSServiceFlagsSecure                 = 0x200010,
   /*
    * The response has been validated by verifying all the signaures in the response and was able to
    * build a successful authentication chain starting from a known trust anchor.   
    */

    kDNSServiceFlagsInsecure               = 0x200020,
   /*
    * A chain of trust cannot be built starting from a known trust anchor to the response.
    */

    kDNSServiceFlagsBogus                  = 0x200040,
   /*
    * If the response cannot be verified to be secure due to expired signatures, missing signatures etc.,
    * then the results are considered to be bogus.
    */

    kDNSServiceFlagsIndeterminate          = 0x200080,
   /*
    * There is no valid trust anchor that can be used to determine whether a response is secure or not.
    */

    kDNSServiceFlagsUnicastResponse        = 0x400000,
   /*
    * Request unicast response to query.
    */
    kDNSServiceFlagsValidateOptional       = 0x800000,

    /*
     * This flag is identical to kDNSServiceFlagsValidate except for the case where the response
     * cannot be validated. If this flag is set in DNSServiceQueryRecord or DNSServiceGetAddrInfo,
     * the DNSSEC records will be requested for validation. If they cannot be received for some reason
     * during the validation (e.g., zone is not signed, zone is signed but cannot be traced back to
     * root, recursive server does not understand DNSSEC etc.), then this will fallback to the default
     * behavior where the validation will not be performed and no DNSSEC results will be provided.
     *
     * If the zone is signed and there is a valid path to a known trust anchor configured in the system
     * and the application requires DNSSEC validation irrespective of the DNSSEC awareness in the current
     * network, then this option MUST not be used. This is only intended to be used during the transition
     * period where the different nodes participating in the DNS resolution may not understand DNSSEC or
     * managed properly (e.g. missing DS record) but still want to be able to resolve DNS successfully.
     */

    kDNSServiceFlagsWakeOnlyService        = 0x1000000,
    /*
     * This flag is meaningful only in DNSServiceRegister. When set, the service will not be registered
     * with sleep proxy server during sleep.
     */

    kDNSServiceFlagsThresholdOne           = 0x2000000,
    kDNSServiceFlagsThresholdFinder        = 0x4000000,
    kDNSServiceFlagsThresholdReached       = kDNSServiceFlagsThresholdOne,
    /*
     * kDNSServiceFlagsThresholdOne is meaningful only in DNSServiceBrowse. When set,
     * the system will stop issuing browse queries on the network once the number
     * of answers returned is one or more.  It will issue queries on the network
     * again if the number of answers drops to zero.
     * This flag is for Apple internal use only. Third party developers
     * should not rely on this behavior being supported in any given software release.
     *
     * kDNSServiceFlagsThresholdFinder is meaningful only in DNSServiceBrowse. When set,
     * the system will stop issuing browse queries on the network once the number
     * of answers has reached the threshold set for Finder.
     * It will issue queries on the network again if the number of answers drops below
     * this threshold.
     * This flag is for Apple internal use only. Third party developers
     * should not rely on this behavior being supported in any given software release.
     *
     * When kDNSServiceFlagsThresholdReached is set in the client callback add or remove event,
     * it indicates that the browse answer threshold has been reached and no 
     * browse requests will be generated on the network until the number of answers falls
     * below the threshold value.  Add and remove events can still occur based
     * on incoming Bonjour traffic observed by the system.
     * The set of services return to the client is not guaranteed to represent the 
     * entire set of services present on the network once the threshold has been reached.
     *
     * Note, while kDNSServiceFlagsThresholdReached and kDNSServiceFlagsThresholdOne
     * have the same value, there  isn't a conflict because kDNSServiceFlagsThresholdReached
     * is only set in the callbacks and kDNSServiceFlagsThresholdOne is only set on
     * input to a DNSServiceBrowse call.
     */
     kDNSServiceFlagsDenyCellular           = 0x8000000,
    /*
     * This flag is meaningful only for Unicast DNS queries. When set, the kernel will restrict
     * DNS resolutions on the cellular interface for that request.
     */

     kDNSServiceFlagsServiceIndex           = 0x10000000,
    /*
     * This flag is meaningful only for DNSServiceGetAddrInfo() for Unicast DNS queries.
     * When set, DNSServiceGetAddrInfo() will interpret the "interfaceIndex" argument of the call
     * as the "serviceIndex".
     */

     kDNSServiceFlagsDenyExpensive          = 0x20000000
    /*
     * This flag is meaningful only for Unicast DNS queries. When set, the kernel will restrict
     * DNS resolutions on interfaces defined as expensive for that request.
     */

};

#define kDNSServiceOutputFlags (kDNSServiceFlagsValidate | kDNSServiceFlagsValidateOptional | kDNSServiceFlagsMoreComing | kDNSServiceFlagsAdd | kDNSServiceFlagsDefault)
   /* All the output flags excluding the DNSSEC Status flags. Typically used to check DNSSEC Status */

/* Possible protocol values */
enum
{
    /* for DNSServiceGetAddrInfo() */
    kDNSServiceProtocol_IPv4 = 0x01,
    kDNSServiceProtocol_IPv6 = 0x02,
    /* 0x04 and 0x08 reserved for future internetwork protocols */

    /* for DNSServiceNATPortMappingCreate() */
    kDNSServiceProtocol_UDP  = 0x10,
    kDNSServiceProtocol_TCP  = 0x20
                               /* 0x40 and 0x80 reserved for future transport protocols, e.g. SCTP [RFC 2960]
                                * or DCCP [RFC 4340]. If future NAT gateways are created that support port
                                * mappings for these protocols, new constants will be defined here.
                                */
};

/*
 * The values for DNS Classes and Types are listed in RFC 1035, and are available
 * on every OS in its DNS header file. Unfortunately every OS does not have the
 * same header file containing DNS Class and Type constants, and the names of
 * the constants are not consistent. For example, BIND 8 uses "T_A",
 * BIND 9 uses "ns_t_a", Windows uses "DNS_TYPE_A", etc.
 * For this reason, these constants are also listed here, so that code using
 * the DNS-SD programming APIs can use these constants, so that the same code
 * can compile on all our supported platforms.
 */

enum
{
    kDNSServiceClass_IN       = 1       /* Internet */
};

enum
{
    kDNSServiceType_A          = 1,      /* Host address. */
    kDNSServiceType_NS         = 2,      /* Authoritative server. */
    kDNSServiceType_MD         = 3,      /* Mail destination. */
    kDNSServiceType_MF         = 4,      /* Mail forwarder. */
    kDNSServiceType_CNAME      = 5,      /* Canonical name. */
    kDNSServiceType_SOA        = 6,      /* Start of authority zone. */
    kDNSServiceType_MB         = 7,      /* Mailbox domain name. */
    kDNSServiceType_MG         = 8,      /* Mail group member. */
    kDNSServiceType_MR         = 9,      /* Mail rename name. */
    kDNSServiceType_NULL       = 10,     /* Null resource record. */
    kDNSServiceType_WKS        = 11,     /* Well known service. */
    kDNSServiceType_PTR        = 12,     /* Domain name pointer. */
    kDNSServiceType_HINFO      = 13,     /* Host information. */
    kDNSServiceType_MINFO      = 14,     /* Mailbox information. */
    kDNSServiceType_MX         = 15,     /* Mail routing information. */
    kDNSServiceType_TXT        = 16,     /* One or more text strings (NOT "zero or more..."). */
    kDNSServiceType_RP         = 17,     /* Responsible person. */
    kDNSServiceType_AFSDB      = 18,     /* AFS cell database. */
    kDNSServiceType_X25        = 19,     /* X_25 calling address. */
    kDNSServiceType_ISDN       = 20,     /* ISDN calling address. */
    kDNSServiceType_RT         = 21,     /* Router. */
    kDNSServiceType_NSAP       = 22,     /* NSAP address. */
    kDNSServiceType_NSAP_PTR   = 23,     /* Reverse NSAP lookup (deprecated). */
    kDNSServiceType_SIG        = 24,     /* Security signature. */
    kDNSServiceType_KEY        = 25,     /* Security key. */
    kDNSServiceType_PX         = 26,     /* X.400 mail mapping. */
    kDNSServiceType_GPOS       = 27,     /* Geographical position (withdrawn). */
    kDNSServiceType_AAAA       = 28,     /* IPv6 Address. */
    kDNSServiceType_LOC        = 29,     /* Location Information. */
    kDNSServiceType_NXT        = 30,     /* Next domain (security). */
    kDNSServiceType_EID        = 31,     /* Endpoint identifier. */
    kDNSServiceType_NIMLOC     = 32,     /* Nimrod Locator. */
    kDNSServiceType_SRV        = 33,     /* Server Selection. */
    kDNSServiceType_ATMA       = 34,     /* ATM Address */
    kDNSServiceType_NAPTR      = 35,     /* Naming Authority PoinTeR */
    kDNSServiceType_KX         = 36,     /* Key Exchange */
    kDNSServiceType_CERT       = 37,     /* Certification record */
    kDNSServiceType_A6         = 38,     /* IPv6 Address (deprecated) */
    kDNSServiceType_DNAME      = 39,     /* Non-terminal DNAME (for IPv6) */
    kDNSServiceType_SINK       = 40,     /* Kitchen sink (experimental) */
    kDNSServiceType_OPT        = 41,     /* EDNS0 option (meta-RR) */
    kDNSServiceType_APL        = 42,     /* Address Prefix List */
    kDNSServiceType_DS         = 43,     /* Delegation Signer */
    kDNSServiceType_SSHFP      = 44,     /* SSH Key Fingerprint */
    kDNSServiceType_IPSECKEY   = 45,     /* IPSECKEY */
    kDNSServiceType_RRSIG      = 46,     /* RRSIG */
    kDNSServiceType_NSEC       = 47,     /* Denial of Existence */
    kDNSServiceType_DNSKEY     = 48,     /* DNSKEY */
    kDNSServiceType_DHCID      = 49,     /* DHCP Client Identifier */
    kDNSServiceType_NSEC3      = 50,     /* Hashed Authenticated Denial of Existence */
    kDNSServiceType_NSEC3PARAM = 51,     /* Hashed Authenticated Denial of Existence */

    kDNSServiceType_HIP        = 55,     /* Host Identity Protocol */

    kDNSServiceType_SPF        = 99,     /* Sender Policy Framework for E-Mail */
    kDNSServiceType_UINFO      = 100,    /* IANA-Reserved */
    kDNSServiceType_UID        = 101,    /* IANA-Reserved */
    kDNSServiceType_GID        = 102,    /* IANA-Reserved */
    kDNSServiceType_UNSPEC     = 103,    /* IANA-Reserved */

    kDNSServiceType_TKEY       = 249,    /* Transaction key */
    kDNSServiceType_TSIG       = 250,    /* Transaction signature. */
    kDNSServiceType_IXFR       = 251,    /* Incremental zone transfer. */
    kDNSServiceType_AXFR       = 252,    /* Transfer zone of authority. */
    kDNSServiceType_MAILB      = 253,    /* Transfer mailbox records. */
    kDNSServiceType_MAILA      = 254,    /* Transfer mail agent records. */
    kDNSServiceType_ANY        = 255     /* Wildcard match. */
};

/* possible error code values */
enum
{
    kDNSServiceErr_NoError                   = 0,
    kDNSServiceErr_Unknown                   = -65537,  /* 0xFFFE FFFF */
    kDNSServiceErr_NoSuchName                = -65538,
    kDNSServiceErr_NoMemory                  = -65539,
    kDNSServiceErr_BadParam                  = -65540,
    kDNSServiceErr_BadReference              = -65541,
    kDNSServiceErr_BadState                  = -65542,
    kDNSServiceErr_BadFlags                  = -65543,
    kDNSServiceErr_Unsupported               = -65544,
    kDNSServiceErr_NotInitialized            = -65545,
    kDNSServiceErr_AlreadyRegistered         = -65547,
    kDNSServiceErr_NameConflict              = -65548,
    kDNSServiceErr_Invalid                   = -65549,
    kDNSServiceErr_Firewall                  = -65550,
    kDNSServiceErr_Incompatible              = -65551,  /* client library incompatible with daemon */
    kDNSServiceErr_BadInterfaceIndex         = -65552,
    kDNSServiceErr_Refused                   = -65553,
    kDNSServiceErr_NoSuchRecord              = -65554,
    kDNSServiceErr_NoAuth                    = -65555,
    kDNSServiceErr_NoSuchKey                 = -65556,
    kDNSServiceErr_NATTraversal              = -65557,
    kDNSServiceErr_DoubleNAT                 = -65558,
    kDNSServiceErr_BadTime                   = -65559,  /* Codes up to here existed in Tiger */
    kDNSServiceErr_BadSig                    = -65560,
    kDNSServiceErr_BadKey                    = -65561,
    kDNSServiceErr_Transient                 = -65562,
    kDNSServiceErr_ServiceNotRunning         = -65563,  /* Background daemon not running */
    kDNSServiceErr_NATPortMappingUnsupported = -65564,  /* NAT doesn't support PCP, NAT-PMP or UPnP */
    kDNSServiceErr_NATPortMappingDisabled    = -65565,  /* NAT supports PCP, NAT-PMP or UPnP, but it's disabled by the administrator */
    kDNSServiceErr_NoRouter                  = -65566,  /* No router currently configured (probably no network connectivity) */
    kDNSServiceErr_PollingMode               = -65567,
    kDNSServiceErr_Timeout                   = -65568

                                               /* mDNS Error codes are in the range
                                                * FFFE FF00 (-65792) to FFFE FFFF (-65537) */
};

/* Maximum length, in bytes, of a service name represented as a */
/* literal C-String, including the terminating NULL at the end. */

#define kDNSServiceMaxServiceName 64

/* Maximum length, in bytes, of a domain name represented as an *escaped* C-String */
/* including the final trailing dot, and the C-String terminating NULL at the end. */

#define kDNSServiceMaxDomainName 1009

/*
 * Notes on DNS Name Escaping
 *   -- or --
 * "Why is kDNSServiceMaxDomainName 1009, when the maximum legal domain name is 256 bytes?"
 *
 * All strings used in the DNS-SD APIs are UTF-8 strings. Apart from the exceptions noted below,
 * the APIs expect the strings to be properly escaped, using the conventional DNS escaping rules:
 *
 *   '\\' represents a single literal '\' in the name
 *   '\.' represents a single literal '.' in the name
 *   '\ddd', where ddd is a three-digit decimal value from 000 to 255,
 *        represents a single literal byte with that value.
 *   A bare unescaped '.' is a label separator, marking a boundary between domain and subdomain.
 *
 * The exceptions, that do not use escaping, are the routines where the full
 * DNS name of a resource is broken, for convenience, into servicename/regtype/domain.
 * In these routines, the "servicename" is NOT escaped. It does not need to be, since
 * it is, by definition, just a single literal string. Any characters in that string
 * represent exactly what they are. The "regtype" portion is, technically speaking,
 * escaped, but since legal regtypes are only allowed to contain letters, digits,
 * and hyphens, there is nothing to escape, so the issue is moot. The "domain"
 * portion is also escaped, though most domains in use on the public Internet
 * today, like regtypes, don't contain any characters that need to be escaped.
 * As DNS-SD becomes more popular, rich-text domains for service discovery will
 * become common, so software should be written to cope with domains with escaping.
 *
 * The servicename may be up to 63 bytes of UTF-8 text (not counting the C-String
 * terminating NULL at the end). The regtype is of the form _service._tcp or
 * _service._udp, where the "service" part is 1-15 characters, which may be
 * letters, digits, or hyphens. The domain part of the three-part name may be
 * any legal domain, providing that the resulting servicename+regtype+domain
 * name does not exceed 256 bytes.
 *
 * For most software, these issues are transparent. When browsing, the discovered
 * servicenames should simply be displayed as-is. When resolving, the discovered
 * servicename/regtype/domain are simply passed unchanged to DNSServiceResolve().
 * When a DNSServiceResolve() succeeds, the returned fullname is already in
 * the correct format to pass to standard system DNS APIs such as res_query().
 * For converting from servicename/regtype/domain to a single properly-escaped
 * full DNS name, the helper function DNSServiceConstructFullName() is provided.
 *
 * The following (highly contrived) example illustrates the escaping process.
 * Suppose you have an service called "Dr. Smith\Dr. Johnson", of type "_ftp._tcp"
 * in subdomain "4th. Floor" of subdomain "Building 2" of domain "apple.com."
 * The full (escaped) DNS name of this service's SRV record would be:
 * Dr\.\032Smith\\Dr\.\032Johnson._ftp._tcp.4th\.\032Floor.Building\0322.apple.com.
 */


/*
 * Constants for specifying an interface index
 *
 * Specific interface indexes are identified via a 32-bit unsigned integer returned
 * by the if_nametoindex() family of calls.
 *
 * If the client passes 0 for interface index, that means "do the right thing",
 * which (at present) means, "if the name is in an mDNS local multicast domain
 * (e.g. 'local.', '254.169.in-addr.arpa.', '{8,9,A,B}.E.F.ip6.arpa.') then multicast
 * on all applicable interfaces, otherwise send via unicast to the appropriate
 * DNS server." Normally, most clients will use 0 for interface index to
 * automatically get the default sensible behaviour.
 *
 * If the client passes a positive interface index, then for multicast names that
 * indicates to do the operation only on that one interface. For unicast names the
 * interface index is ignored unless kDNSServiceFlagsForceMulticast is also set.
 *
 * If the client passes kDNSServiceInterfaceIndexLocalOnly when registering
 * a service, then that service will be found *only* by other local clients
 * on the same machine that are browsing using kDNSServiceInterfaceIndexLocalOnly
 * or kDNSServiceInterfaceIndexAny.
 * If a client has a 'private' service, accessible only to other processes
 * running on the same machine, this allows the client to advertise that service
 * in a way such that it does not inadvertently appear in service lists on
 * all the other machines on the network.
 *
 * If the client passes kDNSServiceInterfaceIndexLocalOnly when browsing
 * then it will find *all* records registered on that same local machine.
 * Clients explicitly wishing to discover *only* LocalOnly services can
 * accomplish this by inspecting the interfaceIndex of each service reported
 * to their DNSServiceBrowseReply() callback function, and discarding those
 * where the interface index is not kDNSServiceInterfaceIndexLocalOnly.
 *
 * kDNSServiceInterfaceIndexP2P is meaningful only in Browse, QueryRecord, Register,
 * and Resolve operations. It should not be used in other DNSService APIs.
 *
 * - If kDNSServiceInterfaceIndexP2P is passed to DNSServiceBrowse or
 *   DNSServiceQueryRecord, it restricts the operation to P2P.
 *
 * - If kDNSServiceInterfaceIndexP2P is passed to DNSServiceRegister, it is
 *   mapped internally to kDNSServiceInterfaceIndexAny with the kDNSServiceFlagsIncludeP2P
 *   set.
 *
 * - If kDNSServiceInterfaceIndexP2P is passed to DNSServiceResolve, it is
 *   mapped internally to kDNSServiceInterfaceIndexAny with the kDNSServiceFlagsIncludeP2P
 *   set, because resolving a P2P service may create and/or enable an interface whose 
 *   index is not known a priori. The resolve callback will indicate the index of the
 *   interface via which the service can be accessed.
 *
 * If applications pass kDNSServiceInterfaceIndexAny to DNSServiceBrowse
 * or DNSServiceQueryRecord, they must set the kDNSServiceFlagsIncludeP2P flag
 * to include P2P. In this case, if a service instance or the record being queried
 * is found over P2P, the resulting ADD event will indicate kDNSServiceInterfaceIndexP2P
 * as the interface index.
 */

#define kDNSServiceInterfaceIndexAny 0
#define kDNSServiceInterfaceIndexLocalOnly ((uint32_t)-1)
#define kDNSServiceInterfaceIndexUnicast   ((uint32_t)-2)
#define kDNSServiceInterfaceIndexP2P       ((uint32_t)-3)

typedef uint32_t DNSServiceFlags;
typedef uint32_t DNSServiceProtocol;
typedef int32_t DNSServiceErrorType;


/*********************************************************************************************
*
* Version checking
*
*********************************************************************************************/

/* DNSServiceGetProperty() Parameters:
 *
 * property:        The requested property.
 *                  Currently the only property defined is kDNSServiceProperty_DaemonVersion.
 *
 * result:          Place to store result.
 *                  For retrieving DaemonVersion, this should be the address of a uint32_t.
 *
 * size:            Pointer to uint32_t containing size of the result location.
 *                  For retrieving DaemonVersion, this should be sizeof(uint32_t).
 *                  On return the uint32_t is updated to the size of the data returned.
 *                  For DaemonVersion, the returned size is always sizeof(uint32_t), but
 *                  future properties could be defined which return variable-sized results.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, or kDNSServiceErr_ServiceNotRunning
 *                  if the daemon (or "system service" on Windows) is not running.
 */

DNSServiceErrorType DNSSD_API DNSServiceGetProperty
(
    const char *property,  /* Requested property (i.e. kDNSServiceProperty_DaemonVersion) */
    void       *result,    /* Pointer to place to store result */
    uint32_t   *size       /* size of result location */
);

/*
 * When requesting kDNSServiceProperty_DaemonVersion, the result pointer must point
 * to a 32-bit unsigned integer, and the size parameter must be set to sizeof(uint32_t).
 *
 * On return, the 32-bit unsigned integer contains the API version number
 *
 * For example, Mac OS X 10.4.9 has API version 1080400.
 * This allows applications to do simple greater-than and less-than comparisons:
 * e.g. an application that requires at least API version 1080400 can check:
 *   if (version >= 1080400) ...
 *
 * Example usage:
 * uint32_t version;
 * uint32_t size = sizeof(version);
 * DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &version, &size);
 * if (!err) printf("DNS_SD API version is %d.%d\n", version / 10000, version / 100 % 100);
 */

#define kDNSServiceProperty_DaemonVersion "DaemonVersion"


// Map the source port of the local UDP socket that was opened for sending the DNS query
// to the process ID of the application that triggered the DNS resolution.
//
/* DNSServiceGetPID() Parameters:
 *
 * srcport:         Source port (in network byte order) of the UDP socket that was created by
 *                  the daemon to send the DNS query on the wire.
 *
 * pid:             Process ID of the application that started the name resolution which triggered
 *                  the daemon to send the query on the wire. The value can be -1 if the srcport
 *                  cannot be mapped.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, or kDNSServiceErr_ServiceNotRunning
 *                  if the daemon is not running. The value of the pid is undefined if the return
 *                  value has error.
 */
DNSServiceErrorType DNSSD_API DNSServiceGetPID
(
    uint16_t srcport,
    int32_t *pid
);

/*********************************************************************************************
*
* Unix Domain Socket access, DNSServiceRef deallocation, and data processing functions
*
*********************************************************************************************/

/* DNSServiceRefSockFD()
 *
 * Access underlying Unix domain socket for an initialized DNSServiceRef.
 * The DNS Service Discovery implementation uses this socket to communicate between the client and
 * the daemon. The application MUST NOT directly read from or write to this socket.
 * Access to the socket is provided so that it can be used as a kqueue event source, a CFRunLoop
 * event source, in a select() loop, etc. When the underlying event management subsystem (kqueue/
 * select/CFRunLoop etc.) indicates to the client that data is available for reading on the
 * socket, the client should call DNSServiceProcessResult(), which will extract the daemon's
 * reply from the socket, and pass it to the appropriate application callback. By using a run
 * loop or select(), results from the daemon can be processed asynchronously. Alternatively,
 * a client can choose to fork a thread and have it loop calling "DNSServiceProcessResult(ref);"
 * If DNSServiceProcessResult() is called when no data is available for reading on the socket, it
 * will block until data does become available, and then process the data and return to the caller.
 * The application is reponsible for checking the return value of DNSServiceProcessResult() to determine
 * if the socket is valid and if it should continue to process data on the socket.
 * When data arrives on the socket, the client is responsible for calling DNSServiceProcessResult(ref)
 * in a timely fashion -- if the client allows a large backlog of data to build up the daemon
 * may terminate the connection.
 *
 * sdRef:           A DNSServiceRef initialized by any of the DNSService calls.
 *
 * return value:    The DNSServiceRef's underlying socket descriptor, or -1 on
 *                  error.
 */

int DNSSD_API DNSServiceRefSockFD(DNSServiceRef sdRef);


/* DNSServiceProcessResult()
 *
 * Read a reply from the daemon, calling the appropriate application callback. This call will
 * block until the daemon's response is received. Use DNSServiceRefSockFD() in
 * conjunction with a run loop or select() to determine the presence of a response from the
 * server before calling this function to process the reply without blocking. Call this function
 * at any point if it is acceptable to block until the daemon's response arrives. Note that the
 * client is responsible for ensuring that DNSServiceProcessResult() is called whenever there is
 * a reply from the daemon - the daemon may terminate its connection with a client that does not
 * process the daemon's responses.
 *
 * sdRef:           A DNSServiceRef initialized by any of the DNSService calls
 *                  that take a callback parameter.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceProcessResult(DNSServiceRef sdRef);


/* DNSServiceRefDeallocate()
 *
 * Terminate a connection with the daemon and free memory associated with the DNSServiceRef.
 * Any services or records registered with this DNSServiceRef will be deregistered. Any
 * Browse, Resolve, or Query operations called with this reference will be terminated.
 *
 * Note: If the reference's underlying socket is used in a run loop or select() call, it should
 * be removed BEFORE DNSServiceRefDeallocate() is called, as this function closes the reference's
 * socket.
 *
 * Note: If the reference was initialized with DNSServiceCreateConnection(), any DNSRecordRefs
 * created via this reference will be invalidated by this call - the resource records are
 * deregistered, and their DNSRecordRefs may not be used in subsequent functions. Similarly,
 * if the reference was initialized with DNSServiceRegister, and an extra resource record was
 * added to the service via DNSServiceAddRecord(), the DNSRecordRef created by the Add() call
 * is invalidated when this function is called - the DNSRecordRef may not be used in subsequent
 * functions.
 *
 * Note: This call is to be used only with the DNSServiceRef defined by this API.
 *
 * sdRef:           A DNSServiceRef initialized by any of the DNSService calls.
 *
 */

void DNSSD_API DNSServiceRefDeallocate(DNSServiceRef sdRef);


/*********************************************************************************************
*
* Domain Enumeration
*
*********************************************************************************************/

/* DNSServiceEnumerateDomains()
 *
 * Asynchronously enumerate domains available for browsing and registration.
 *
 * The enumeration MUST be cancelled via DNSServiceRefDeallocate() when no more domains
 * are to be found.
 *
 * Note that the names returned are (like all of DNS-SD) UTF-8 strings,
 * and are escaped using standard DNS escaping rules.
 * (See "Notes on DNS Name Escaping" earlier in this file for more details.)
 * A graphical browser displaying a hierarchical tree-structured view should cut
 * the names at the bare dots to yield individual labels, then de-escape each
 * label according to the escaping rules, and then display the resulting UTF-8 text.
 *
 * DNSServiceDomainEnumReply Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceEnumerateDomains().
 *
 * flags:           Possible values are:
 *                  kDNSServiceFlagsMoreComing
 *                  kDNSServiceFlagsAdd
 *                  kDNSServiceFlagsDefault
 *
 * interfaceIndex:  Specifies the interface on which the domain exists. (The index for a given
 *                  interface is determined via the if_nametoindex() family of calls.)
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise indicates
 *                  the failure that occurred (other parameters are undefined if errorCode is nonzero).
 *
 * replyDomain:     The name of the domain.
 *
 * context:         The context pointer passed to DNSServiceEnumerateDomains.
 *
 */

typedef void (DNSSD_API *DNSServiceDomainEnumReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char                          *replyDomain,
    void                                *context
);


/* DNSServiceEnumerateDomains() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the enumeration operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Possible values are:
 *                  kDNSServiceFlagsBrowseDomains to enumerate domains recommended for browsing.
 *                  kDNSServiceFlagsRegistrationDomains to enumerate domains recommended
 *                  for registration.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to look for domains.
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.) Most applications will pass 0 to enumerate domains on
 *                  all interfaces. See "Constants for specifying an interface index" for more details.
 *
 * callBack:        The function to be called when a domain is found or the call asynchronously
 *                  fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is not invoked and the DNSServiceRef
 *                  is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceEnumerateDomains
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceDomainEnumReply callBack,
    void                                *context  /* may be NULL */
);


/*********************************************************************************************
*
*  Service Registration
*
*********************************************************************************************/

/* Register a service that is discovered via Browse() and Resolve() calls.
 *
 * DNSServiceRegisterReply() Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceRegister().
 *
 * flags:           When a name is successfully registered, the callback will be
 *                  invoked with the kDNSServiceFlagsAdd flag set. When Wide-Area
 *                  DNS-SD is in use, it is possible for a single service to get
 *                  more than one success callback (e.g. one in the "local" multicast
 *                  DNS domain, and another in a wide-area unicast DNS domain).
 *                  If a successfully-registered name later suffers a name conflict
 *                  or similar problem and has to be deregistered, the callback will
 *                  be invoked with the kDNSServiceFlagsAdd flag not set. The callback
 *                  is *not* invoked in the case where the caller explicitly terminates
 *                  the service registration by calling DNSServiceRefDeallocate(ref);
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred (including name conflicts,
 *                  if the kDNSServiceFlagsNoAutoRename flag was used when registering.)
 *                  Other parameters are undefined if errorCode is nonzero.
 *
 * name:            The service name registered (if the application did not specify a name in
 *                  DNSServiceRegister(), this indicates what name was automatically chosen).
 *
 * regtype:         The type of service registered, as it was passed to the callout.
 *
 * domain:          The domain on which the service was registered (if the application did not
 *                  specify a domain in DNSServiceRegister(), this indicates the default domain
 *                  on which the service was registered).
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceRegisterReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    const char                          *name,
    const char                          *regtype,
    const char                          *domain,
    void                                *context
);


/* DNSServiceRegister() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the registration will remain active indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to register the service
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.) Most applications will pass 0 to register on all
 *                  available interfaces. See "Constants for specifying an interface index" for more details.
 *
 * flags:           Indicates the renaming behavior on name conflict (most applications
 *                  will pass 0). See flag definitions above for details.
 *
 * name:            If non-NULL, specifies the service name to be registered.
 *                  Most applications will not specify a name, in which case the computer
 *                  name is used (this name is communicated to the client via the callback).
 *                  If a name is specified, it must be 1-63 bytes of UTF-8 text.
 *                  If the name is longer than 63 bytes it will be automatically truncated
 *                  to a legal length, unless the NoAutoRename flag is set,
 *                  in which case kDNSServiceErr_BadParam will be returned.
 *
 * regtype:         The service type followed by the protocol, separated by a dot
 *                  (e.g. "_ftp._tcp"). The service type must be an underscore, followed
 *                  by 1-15 characters, which may be letters, digits, or hyphens.
 *                  The transport protocol must be "_tcp" or "_udp". New service types
 *                  should be registered at <http://www.dns-sd.org/ServiceTypes.html>.
 *
 *                  Additional subtypes of the primary service type (where a service
 *                  type has defined subtypes) follow the primary service type in a
 *                  comma-separated list, with no additional spaces, e.g.
 *                      "_primarytype._tcp,_subtype1,_subtype2,_subtype3"
 *                  Subtypes provide a mechanism for filtered browsing: A client browsing
 *                  for "_primarytype._tcp" will discover all instances of this type;
 *                  a client browsing for "_primarytype._tcp,_subtype2" will discover only
 *                  those instances that were registered with "_subtype2" in their list of
 *                  registered subtypes.
 *
 *                  The subtype mechanism can be illustrated with some examples using the
 *                  dns-sd command-line tool:
 *
 *                  % dns-sd -R Simple _test._tcp "" 1001 &
 *                  % dns-sd -R Better _test._tcp,HasFeatureA "" 1002 &
 *                  % dns-sd -R Best   _test._tcp,HasFeatureA,HasFeatureB "" 1003 &
 *
 *                  Now:
 *                  % dns-sd -B _test._tcp             # will find all three services
 *                  % dns-sd -B _test._tcp,HasFeatureA # finds "Better" and "Best"
 *                  % dns-sd -B _test._tcp,HasFeatureB # finds only "Best"
 *
 *                  Subtype labels may be up to 63 bytes long, and may contain any eight-
 *                  bit byte values, including zero bytes. However, due to the nature of
 *                  using a C-string-based API, conventional DNS escaping must be used for
 *                  dots ('.'), commas (','), backslashes ('\') and zero bytes, as shown below:
 *
 *                  % dns-sd -R Test '_test._tcp,s\.one,s\,two,s\\three,s\000four' local 123
 *
 *                  When a service is registered, all the clients browsing for the registered
 *                  type ("regtype") will discover it. If the discovery should be
 *                  restricted to a smaller set of well known peers, the service can be
 *                  registered with additional data (group identifier) that is known
 *                  only to a smaller set of peers. The group identifier should follow primary
 *                  service type using a colon (":") as a delimeter. If subtypes are also present,
 *                  it should be given before the subtype as shown below.
 *
 *                  % dns-sd -R _test1 _http._tcp:mygroup1 local 1001 
 *                  % dns-sd -R _test2 _http._tcp:mygroup2 local 1001 
 *                  % dns-sd -R _test3 _http._tcp:mygroup3,HasFeatureA local 1001 
 *
 *                  Now:
 *                  % dns-sd -B _http._tcp:"mygroup1"                # will discover only test1
 *                  % dns-sd -B _http._tcp:"mygroup2"                # will discover only test2
 *                  % dns-sd -B _http._tcp:"mygroup3",HasFeatureA    # will discover only test3
 *                  
 *                  By specifying the group information, only the members of that group are
 *                  discovered.
 *
 *                  The group identifier itself is not sent in clear. Only a hash of the group
 *                  identifier is sent and the clients discover them anonymously. The group identifier
 *                  may be up to 256 bytes long and may contain any eight bit values except comma which
 *                  should be escaped.
 *
 * domain:          If non-NULL, specifies the domain on which to advertise the service.
 *                  Most applications will not specify a domain, instead automatically
 *                  registering in the default domain(s).
 *
 * host:            If non-NULL, specifies the SRV target host name. Most applications
 *                  will not specify a host, instead automatically using the machine's
 *                  default host name(s). Note that specifying a non-NULL host does NOT
 *                  create an address record for that host - the application is responsible
 *                  for ensuring that the appropriate address record exists, or creating it
 *                  via DNSServiceRegisterRecord().
 *
 * port:            The port, in network byte order, on which the service accepts connections.
 *                  Pass 0 for a "placeholder" service (i.e. a service that will not be discovered
 *                  by browsing, but will cause a name conflict if another client tries to
 *                  register that same name). Most clients will not use placeholder services.
 *
 * txtLen:          The length of the txtRecord, in bytes. Must be zero if the txtRecord is NULL.
 *
 * txtRecord:       The TXT record rdata. A non-NULL txtRecord MUST be a properly formatted DNS
 *                  TXT record, i.e. <length byte> <data> <length byte> <data> ...
 *                  Passing NULL for the txtRecord is allowed as a synonym for txtLen=1, txtRecord="",
 *                  i.e. it creates a TXT record of length one containing a single empty string.
 *                  RFC 1035 doesn't allow a TXT record to contain *zero* strings, so a single empty
 *                  string is the smallest legal DNS TXT record.
 *                  As with the other parameters, the DNSServiceRegister call copies the txtRecord
 *                  data; e.g. if you allocated the storage for the txtRecord parameter with malloc()
 *                  then you can safely free that memory right after the DNSServiceRegister call returns.
 *
 * callBack:        The function to be called when the registration completes or asynchronously
 *                  fails. The client MAY pass NULL for the callback -  The client will NOT be notified
 *                  of the default values picked on its behalf, and the client will NOT be notified of any
 *                  asynchronous errors (e.g. out of memory errors, etc.) that may prevent the registration
 *                  of the service. The client may NOT pass the NoAutoRename flag if the callback is NULL.
 *                  The client may still deregister the service at any time via DNSServiceRefDeallocate().
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceRegister
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                          *name,         /* may be NULL */
    const char                          *regtype,
    const char                          *domain,       /* may be NULL */
    const char                          *host,         /* may be NULL */
    uint16_t port,                                     /* In network byte order */
    uint16_t txtLen,
    const void                          *txtRecord,    /* may be NULL */
    DNSServiceRegisterReply callBack,                  /* may be NULL */
    void                                *context       /* may be NULL */
);


/* DNSServiceAddRecord()
 *
 * Add a record to a registered service. The name of the record will be the same as the
 * registered service's name.
 * The record can later be updated or deregistered by passing the RecordRef initialized
 * by this function to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *
 * Note that the DNSServiceAddRecord/UpdateRecord/RemoveRecord are *NOT* thread-safe
 * with respect to a single DNSServiceRef. If you plan to have multiple threads
 * in your program simultaneously add, update, or remove records from the same
 * DNSServiceRef, then it's the caller's responsibility to use a mutext lock
 * or take similar appropriate precautions to serialize those calls.
 *
 * Parameters;
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceRegister().
 *
 * RecordRef:       A pointer to an uninitialized DNSRecordRef. Upon succesfull completion of this
 *                  call, this ref may be passed to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *                  If the above DNSServiceRef is passed to DNSServiceRefDeallocate(), RecordRef is also
 *                  invalidated and may not be used further.
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * rrtype:          The type of the record (e.g. kDNSServiceType_TXT, kDNSServiceType_SRV, etc)
 *
 * rdlen:           The length, in bytes, of the rdata.
 *
 * rdata:           The raw rdata to be contained in the added resource record.
 *
 * ttl:             The time to live of the resource record, in seconds.
 *                  Most clients should pass 0 to indicate that the system should
 *                  select a sensible default value.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred (the RecordRef is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceAddRecord
(
    DNSServiceRef sdRef,
    DNSRecordRef                        *RecordRef,
    DNSServiceFlags flags,
    uint16_t rrtype,
    uint16_t rdlen,
    const void                          *rdata,
    uint32_t ttl
);


/* DNSServiceUpdateRecord
 *
 * Update a registered resource record. The record must either be:
 *   - The primary txt record of a service registered via DNSServiceRegister()
 *   - A record added to a registered service via DNSServiceAddRecord()
 *   - An individual record registered by DNSServiceRegisterRecord()
 *
 * Parameters:
 *
 * sdRef:           A DNSServiceRef that was initialized by DNSServiceRegister()
 *                  or DNSServiceCreateConnection().
 *
 * RecordRef:       A DNSRecordRef initialized by DNSServiceAddRecord, or NULL to update the
 *                  service's primary txt record.
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * rdlen:           The length, in bytes, of the new rdata.
 *
 * rdata:           The new rdata to be contained in the updated resource record.
 *
 * ttl:             The time to live of the updated resource record, in seconds.
 *                  Most clients should pass 0 to indicate that the system should
 *                  select a sensible default value.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceUpdateRecord
(
    DNSServiceRef sdRef,
    DNSRecordRef RecordRef,                            /* may be NULL */
    DNSServiceFlags flags,
    uint16_t rdlen,
    const void                          *rdata,
    uint32_t ttl
);


/* DNSServiceRemoveRecord
 *
 * Remove a record previously added to a service record set via DNSServiceAddRecord(), or deregister
 * an record registered individually via DNSServiceRegisterRecord().
 *
 * Parameters:
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceRegister() (if the
 *                  record being removed was registered via DNSServiceAddRecord()) or by
 *                  DNSServiceCreateConnection() (if the record being removed was registered via
 *                  DNSServiceRegisterRecord()).
 *
 * recordRef:       A DNSRecordRef initialized by a successful call to DNSServiceAddRecord()
 *                  or DNSServiceRegisterRecord().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceRemoveRecord
(
    DNSServiceRef sdRef,
    DNSRecordRef RecordRef,
    DNSServiceFlags flags
);


/*********************************************************************************************
*
*  Service Discovery
*
*********************************************************************************************/

/* Browse for instances of a service.
 *
 * DNSServiceBrowseReply() Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceBrowse().
 *
 * flags:           Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
 *                  See flag definitions for details.
 *
 * interfaceIndex:  The interface on which the service is advertised. This index should
 *                  be passed to DNSServiceResolve() when resolving the service.
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise will
 *                  indicate the failure that occurred. Other parameters are undefined if
 *                  the errorCode is nonzero.
 *
 * serviceName:     The discovered service name. This name should be displayed to the user,
 *                  and stored for subsequent use in the DNSServiceResolve() call.
 *
 * regtype:         The service type, which is usually (but not always) the same as was passed
 *                  to DNSServiceBrowse(). One case where the discovered service type may
 *                  not be the same as the requested service type is when using subtypes:
 *                  The client may want to browse for only those ftp servers that allow
 *                  anonymous connections. The client will pass the string "_ftp._tcp,_anon"
 *                  to DNSServiceBrowse(), but the type of the service that's discovered
 *                  is simply "_ftp._tcp". The regtype for each discovered service instance
 *                  should be stored along with the name, so that it can be passed to
 *                  DNSServiceResolve() when the service is later resolved.
 *
 * domain:          The domain of the discovered service instance. This may or may not be the
 *                  same as the domain that was passed to DNSServiceBrowse(). The domain for each
 *                  discovered service instance should be stored along with the name, so that
 *                  it can be passed to DNSServiceResolve() when the service is later resolved.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceBrowseReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char                          *serviceName,
    const char                          *regtype,
    const char                          *replyDomain,
    void                                *context
);


/* DNSServiceBrowse() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the browse operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to browse for services
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.) Most applications will pass 0 to browse on all available
 *                  interfaces. See "Constants for specifying an interface index" for more details.
 *
 * regtype:         The service type being browsed for followed by the protocol, separated by a
 *                  dot (e.g. "_ftp._tcp"). The transport protocol must be "_tcp" or "_udp".
 *                  A client may optionally specify a single subtype to perform filtered browsing:
 *                  e.g. browsing for "_primarytype._tcp,_subtype" will discover only those
 *                  instances of "_primarytype._tcp" that were registered specifying "_subtype"
 *                  in their list of registered subtypes. Additionally, a group identifier may
 *                  also be specified before the subtype e.g., _primarytype._tcp:GroupID, which
 *                  will discover only the members that register the service with GroupID. See
 *                  DNSServiceRegister for more details.
 *
 * domain:          If non-NULL, specifies the domain on which to browse for services.
 *                  Most applications will not specify a domain, instead browsing on the
 *                  default domain(s).
 *
 * callBack:        The function to be called when an instance of the service being browsed for
 *                  is found, or if the call asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is not invoked and the DNSServiceRef
 *                  is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceBrowse
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                          *regtype,
    const char                          *domain,    /* may be NULL */
    DNSServiceBrowseReply callBack,
    void                                *context    /* may be NULL */
);


/* DNSServiceResolve()
 *
 * Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and
 * txt record.
 *
 * Note: Applications should NOT use DNSServiceResolve() solely for txt record monitoring - use
 * DNSServiceQueryRecord() instead, as it is more efficient for this task.
 *
 * Note: When the desired results have been returned, the client MUST terminate the resolve by calling
 * DNSServiceRefDeallocate().
 *
 * Note: DNSServiceResolve() behaves correctly for typical services that have a single SRV record
 * and a single TXT record. To resolve non-standard services with multiple SRV or TXT records,
 * DNSServiceQueryRecord() should be used.
 *
 * DNSServiceResolveReply Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceResolve().
 *
 * flags:           Possible values: kDNSServiceFlagsMoreComing
 *
 * interfaceIndex:  The interface on which the service was resolved.
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise will
 *                  indicate the failure that occurred. Other parameters are undefined if
 *                  the errorCode is nonzero.
 *
 * fullname:        The full service domain name, in the form <servicename>.<protocol>.<domain>.
 *                  (This name is escaped following standard DNS rules, making it suitable for
 *                  passing to standard system DNS APIs such as res_query(), or to the
 *                  special-purpose functions included in this API that take fullname parameters.
 *                  See "Notes on DNS Name Escaping" earlier in this file for more details.)
 *
 * hosttarget:      The target hostname of the machine providing the service. This name can
 *                  be passed to functions like gethostbyname() to identify the host's IP address.
 *
 * port:            The port, in network byte order, on which connections are accepted for this service.
 *
 * txtLen:          The length of the txt record, in bytes.
 *
 * txtRecord:       The service's primary txt record, in standard txt record format.
 *
 * context:         The context pointer that was passed to the callout.
 *
 * NOTE: In earlier versions of this header file, the txtRecord parameter was declared "const char *"
 * This is incorrect, since it contains length bytes which are values in the range 0 to 255, not -128 to +127.
 * Depending on your compiler settings, this change may cause signed/unsigned mismatch warnings.
 * These should be fixed by updating your own callback function definition to match the corrected
 * function signature using "const unsigned char *txtRecord". Making this change may also fix inadvertent
 * bugs in your callback function, where it could have incorrectly interpreted a length byte with value 250
 * as being -6 instead, with various bad consequences ranging from incorrect operation to software crashes.
 * If you need to maintain portable code that will compile cleanly with both the old and new versions of
 * this header file, you should update your callback function definition to use the correct unsigned value,
 * and then in the place where you pass your callback function to DNSServiceResolve(), use a cast to eliminate
 * the compiler warning, e.g.:
 *   DNSServiceResolve(sd, flags, index, name, regtype, domain, (DNSServiceResolveReply)MyCallback, context);
 * This will ensure that your code compiles cleanly without warnings (and more importantly, works correctly)
 * with both the old header and with the new corrected version.
 *
 */

typedef void (DNSSD_API *DNSServiceResolveReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char                          *fullname,
    const char                          *hosttarget,
    uint16_t port,                                   /* In network byte order */
    uint16_t txtLen,
    const unsigned char                 *txtRecord,
    void                                *context
);


/* DNSServiceResolve() Parameters
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the resolve operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Specifying kDNSServiceFlagsForceMulticast will cause query to be
 *                  performed with a link-local mDNS query, even if the name is an
 *                  apparently non-local name (i.e. a name not ending in ".local.")
 *
 * interfaceIndex:  The interface on which to resolve the service. If this resolve call is
 *                  as a result of a currently active DNSServiceBrowse() operation, then the
 *                  interfaceIndex should be the index reported in the DNSServiceBrowseReply
 *                  callback. If this resolve call is using information previously saved
 *                  (e.g. in a preference file) for later use, then use interfaceIndex 0, because
 *                  the desired service may now be reachable via a different physical interface.
 *                  See "Constants for specifying an interface index" for more details.
 *
 * name:            The name of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * regtype:         The type of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * domain:          The domain of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceResolve
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                          *name,
    const char                          *regtype,
    const char                          *domain,
    DNSServiceResolveReply callBack,
    void                                *context  /* may be NULL */
);


/*********************************************************************************************
*
*  Querying Individual Specific Records
*
*********************************************************************************************/

/* DNSServiceQueryRecord
 *
 * Query for an arbitrary DNS record.
 *
 * DNSServiceQueryRecordReply() Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceQueryRecord().
 *
 * flags:           Possible values are kDNSServiceFlagsMoreComing and
 *                  kDNSServiceFlagsAdd. The Add flag is NOT set for PTR records
 *                  with a ttl of 0, i.e. "Remove" events.
 *
 * interfaceIndex:  The interface on which the query was resolved (the index for a given
 *                  interface is determined via the if_nametoindex() family of calls).
 *                  See "Constants for specifying an interface index" for more details.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred. Other parameters are undefined if
 *                  errorCode is nonzero.
 *
 * fullname:        The resource record's full domain name.
 *
 * rrtype:          The resource record's type (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * rdlen:           The length, in bytes, of the resource record rdata.
 *
 * rdata:           The raw rdata of the resource record.
 *
 * ttl:             If the client wishes to cache the result for performance reasons,
 *                  the TTL indicates how long the client may legitimately hold onto
 *                  this result, in seconds. After the TTL expires, the client should
 *                  consider the result no longer valid, and if it requires this data
 *                  again, it should be re-fetched with a new query. Of course, this
 *                  only applies to clients that cancel the asynchronous operation when
 *                  they get a result. Clients that leave the asynchronous operation
 *                  running can safely assume that the data remains valid until they
 *                  get another callback telling them otherwise.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceQueryRecordReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char                          *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    uint16_t rdlen,
    const void                          *rdata,
    uint32_t ttl,
    void                                *context
);


/* DNSServiceQueryRecord() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the query operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           kDNSServiceFlagsForceMulticast or kDNSServiceFlagsLongLivedQuery.
 *                  Pass kDNSServiceFlagsLongLivedQuery to create a "long-lived" unicast
 *                  query to a unicast DNS server that implements the protocol. This flag
 *                  has no effect on link-local multicast queries.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to issue the query
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.) Passing 0 causes the name to be queried for on all
 *                  interfaces. See "Constants for specifying an interface index" for more details.
 *
 * fullname:        The full domain name of the resource record to be queried for.
 *
 * rrtype:          The numerical type of the resource record to be queried for
 *                  (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceQueryRecord
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                          *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    DNSServiceQueryRecordReply callBack,
    void                                *context  /* may be NULL */
);


/*********************************************************************************************
*
*  Unified lookup of both IPv4 and IPv6 addresses for a fully qualified hostname
*
*********************************************************************************************/

/* DNSServiceGetAddrInfo
 *
 * Queries for the IP address of a hostname by using either Multicast or Unicast DNS.
 *
 * DNSServiceGetAddrInfoReply() parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceGetAddrInfo().
 *
 * flags:           Possible values are kDNSServiceFlagsMoreComing and
 *                  kDNSServiceFlagsAdd.
 *
 * interfaceIndex:  The interface to which the answers pertain.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred.  Other parameters are
 *                  undefined if errorCode is nonzero.
 *
 * hostname:        The fully qualified domain name of the host to be queried for.
 *
 * address:         IPv4 or IPv6 address.
 *
 * ttl:             If the client wishes to cache the result for performance reasons,
 *                  the TTL indicates how long the client may legitimately hold onto
 *                  this result, in seconds. After the TTL expires, the client should
 *                  consider the result no longer valid, and if it requires this data
 *                  again, it should be re-fetched with a new query. Of course, this
 *                  only applies to clients that cancel the asynchronous operation when
 *                  they get a result. Clients that leave the asynchronous operation
 *                  running can safely assume that the data remains valid until they
 *                  get another callback telling them otherwise.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceGetAddrInfoReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    const char                       *hostname,
    const struct sockaddr            *address,
    uint32_t ttl,
    void                             *context
);


/* DNSServiceGetAddrInfo() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds then it
 *                  initializes the DNSServiceRef, returns kDNSServiceErr_NoError, and the query
 *                  begins and will last indefinitely until the client terminates the query
 *                  by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           kDNSServiceFlagsForceMulticast
 *
 * interfaceIndex:  The interface on which to issue the query.  Passing 0 causes the query to be
 *                  sent on all active interfaces via Multicast or the primary interface via Unicast.
 *
 * protocol:        Pass in kDNSServiceProtocol_IPv4 to look up IPv4 addresses, or kDNSServiceProtocol_IPv6
 *                  to look up IPv6 addresses, or both to look up both kinds. If neither flag is
 *                  set, the system will apply an intelligent heuristic, which is (currently)
 *                  that it will attempt to look up both, except:
 *
 *                   * If "hostname" is a wide-area unicast DNS hostname (i.e. not a ".local." name)
 *                     but this host has no routable IPv6 address, then the call will not try to
 *                     look up IPv6 addresses for "hostname", since any addresses it found would be
 *                     unlikely to be of any use anyway. Similarly, if this host has no routable
 *                     IPv4 address, the call will not try to look up IPv4 addresses for "hostname".
 *
 * hostname:        The fully qualified domain name of the host to be queried for.
 *
 * callBack:        The function to be called when the query succeeds or fails asynchronously.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceGetAddrInfo
(
    DNSServiceRef                    *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceProtocol protocol,
    const char                       *hostname,
    DNSServiceGetAddrInfoReply callBack,
    void                             *context          /* may be NULL */
);


/*********************************************************************************************
*
*  Special Purpose Calls:
*  DNSServiceCreateConnection(), DNSServiceRegisterRecord(), DNSServiceReconfirmRecord()
*  (most applications will not use these)
*
*********************************************************************************************/

/* DNSServiceCreateConnection()
 *
 * Create a connection to the daemon allowing efficient registration of
 * multiple individual records.
 *
 * Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. Deallocating
 *                  the reference (via DNSServiceRefDeallocate()) severs the
 *                  connection and deregisters all records registered on this connection.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred (in which
 *                  case the DNSServiceRef is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceCreateConnection(DNSServiceRef *sdRef);

/* DNSServiceRegisterRecord
 *
 * Register an individual resource record on a connected DNSServiceRef.
 *
 * Note that name conflicts occurring for records registered via this call must be handled
 * by the client in the callback.
 *
 * DNSServiceRegisterRecordReply() parameters:
 *
 * sdRef:           The connected DNSServiceRef initialized by
 *                  DNSServiceCreateConnection().
 *
 * RecordRef:       The DNSRecordRef initialized by DNSServiceRegisterRecord(). If the above
 *                  DNSServiceRef is passed to DNSServiceRefDeallocate(), this DNSRecordRef is
 *                  invalidated, and may not be used further.
 *
 * flags:           Currently unused, reserved for future use.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred (including name conflicts.)
 *                  Other parameters are undefined if errorCode is nonzero.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceRegisterRecordReply)
(
    DNSServiceRef sdRef,
    DNSRecordRef RecordRef,
    DNSServiceFlags flags,
    DNSServiceErrorType errorCode,
    void                                *context
);


/* DNSServiceRegisterRecord() Parameters:
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceCreateConnection().
 *
 * RecordRef:       A pointer to an uninitialized DNSRecordRef. Upon succesfull completion of this
 *                  call, this ref may be passed to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *                  (To deregister ALL records registered on a single connected DNSServiceRef
 *                  and deallocate each of their corresponding DNSServiceRecordRefs, call
 *                  DNSServiceRefDeallocate()).
 *
 * flags:           Possible values are kDNSServiceFlagsShared or kDNSServiceFlagsUnique
 *                  (see flag type definitions for details).
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to register the record
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.) Passing 0 causes the record to be registered on all interfaces.
 *                  See "Constants for specifying an interface index" for more details.
 *
 * fullname:        The full domain name of the resource record.
 *
 * rrtype:          The numerical type of the resource record (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN)
 *
 * rdlen:           Length, in bytes, of the rdata.
 *
 * rdata:           A pointer to the raw rdata, as it is to appear in the DNS record.
 *
 * ttl:             The time to live of the resource record, in seconds.
 *                  Most clients should pass 0 to indicate that the system should
 *                  select a sensible default value.
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails (e.g. because of a name conflict.)
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSRecordRef is
 *                  not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceRegisterRecord
(
    DNSServiceRef sdRef,
    DNSRecordRef                        *RecordRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                          *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    uint16_t rdlen,
    const void                          *rdata,
    uint32_t ttl,
    DNSServiceRegisterRecordReply callBack,
    void                                *context    /* may be NULL */
);


/* DNSServiceReconfirmRecord
 *
 * Instruct the daemon to verify the validity of a resource record that appears
 * to be out of date (e.g. because TCP connection to a service's target failed.)
 * Causes the record to be flushed from the daemon's cache (as well as all other
 * daemons' caches on the network) if the record is determined to be invalid.
 * Use this routine conservatively. Reconfirming a record necessarily consumes
 * network bandwidth, so this should not be done indiscriminately.
 *
 * Parameters:
 *
 * flags:           Not currently used.
 *
 * interfaceIndex:  Specifies the interface of the record in question.
 *                  The caller must specify the interface.
 *                  This API (by design) causes increased network traffic, so it requires
 *                  the caller to be precise about which record should be reconfirmed.
 *                  It is not possible to pass zero for the interface index to perform
 *                  a "wildcard" reconfirmation, where *all* matching records are reconfirmed.
 *
 * fullname:        The resource record's full domain name.
 *
 * rrtype:          The resource record's type (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * rdlen:           The length, in bytes, of the resource record rdata.
 *
 * rdata:           The raw rdata of the resource record.
 *
 */

DNSServiceErrorType DNSSD_API DNSServiceReconfirmRecord
(
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char                         *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    uint16_t rdlen,
    const void                         *rdata
);

#ifndef __OPEN_SOURCE__

/* PeerConnectionRelease() Parameters
 *
 * Release P2P connection resources associated with the service instance.
 * When a service is resolved over a P2P interface, a connection is brought up to the
 * peer advertising the service instance.  This call will free the resources associated
 * with that connection.  Note that the reference to the service instance will only
 * be maintained by the daemon while the browse for the service type is still 
 * running.  Thus the sequence of calls to discover, resolve, and then terminate the connection 
 * associated with a given P2P service instance would be:
 *
 *   DNSServiceRef BrowseRef, ResolveRef;
 *      DNSServiceBrowse(&BrowseRef, ...)    // browse for all instances of the service
 *      DNSServiceResolve(&ResolveRef, ...)  // resolving a service instance creates a 
 *                                           // connection to the peer device advertising that service
 *      DNSServiceRefDeallocate(ResolveRef)  // Stop the resolve, which does not close the peer connection
 *
 *          // Communicate with the peer application.
 *         
 *      PeerConnectionRelease()  // release the connection to the peer device for the specified service instance
 *
 *      DNSServiceRefDeallocate(BrowseRef)  // stop the browse  
 *          // Any further calls to PeerConnectionRelease() will have no affect since the
 *          // service instance to peer connection relationship is only maintained by the 
 *          // daemon while the browse is running.
 *
 *
 * flags:           Not currently used.
 *
 * name:            The name of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * regtype:         The type of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * domain:          The domain of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * return value:    Returns kDNSServiceErr_NoError on success or the error that occurred.
 */

DNSServiceErrorType DNSSD_API PeerConnectionRelease
(
    DNSServiceFlags flags,
    const char      *name,
    const char      *regtype,
    const char      *domain
);

#endif // __OPEN_SOURCE__

/*********************************************************************************************
*
*  NAT Port Mapping
*
*********************************************************************************************/

/* DNSServiceNATPortMappingCreate
 *
 * Request a port mapping in the NAT gateway, which maps a port on the local machine
 * to an external port on the NAT. The NAT should support either PCP, NAT-PMP or the
 * UPnP/IGD protocol for this API to create a successful mapping. Note that this API
 * currently supports IPv4 addresses/mappings only. If the NAT gateway supports PCP and
 * returns an IPv6 address (incorrectly, since this API specifically requests IPv4
 * addresses), the DNSServiceNATPortMappingReply callback will be invoked with errorCode
 * kDNSServiceErr_NATPortMappingUnsupported.
 *
 * The port mapping will be renewed indefinitely until the client process exits, or
 * explicitly terminates the port mapping request by calling DNSServiceRefDeallocate().
 * The client callback will be invoked, informing the client of the NAT gateway's
 * external IP address and the external port that has been allocated for this client.
 * The client should then record this external IP address and port using whatever
 * directory service mechanism it is using to enable peers to connect to it.
 * (Clients advertising services using Wide-Area DNS-SD DO NOT need to use this API
 * -- when a client calls DNSServiceRegister() NAT mappings are automatically created
 * and the external IP address and port for the service are recorded in the global DNS.
 * Only clients using some directory mechanism other than Wide-Area DNS-SD need to use
 * this API to explicitly map their own ports.)
 *
 * It's possible that the client callback could be called multiple times, for example
 * if the NAT gateway's IP address changes, or if a configuration change results in a
 * different external port being mapped for this client. Over the lifetime of any long-lived
 * port mapping, the client should be prepared to handle these notifications of changes
 * in the environment, and should update its recorded address and/or port as appropriate.
 *
 * NOTE: There are two unusual aspects of how the DNSServiceNATPortMappingCreate API works,
 * which were intentionally designed to help simplify client code:
 *
 *  1. It's not an error to request a NAT mapping when the machine is not behind a NAT gateway.
 *     In other NAT mapping APIs, if you request a NAT mapping and the machine is not behind a NAT
 *     gateway, then the API returns an error code -- it can't get you a NAT mapping if there's no
 *     NAT gateway. The DNSServiceNATPortMappingCreate API takes a different view. Working out
 *     whether or not you need a NAT mapping can be tricky and non-obvious, particularly on
 *     a machine with multiple active network interfaces. Rather than make every client recreate
 *     this logic for deciding whether a NAT mapping is required, the PortMapping API does that
 *     work for you. If the client calls the PortMapping API when the machine already has a
 *     routable public IP address, then instead of complaining about it and giving an error,
 *     the PortMapping API just invokes your callback, giving the machine's public address
 *     and your own port number. This means you don't need to write code to work out whether
 *     your client needs to call the PortMapping API -- just call it anyway, and if it wasn't
 *     necessary, no harm is done:
 *
 *     - If the machine already has a routable public IP address, then your callback
 *       will just be invoked giving your own address and port.
 *     - If a NAT mapping is required and obtained, then your callback will be invoked
 *       giving you the external address and port.
 *     - If a NAT mapping is required but not obtained from the local NAT gateway,
 *       or the machine has no network connectivity, then your callback will be
 *       invoked giving zero address and port.
 *
 *  2. In other NAT mapping APIs, if a laptop computer is put to sleep and woken up on a new
 *     network, it's the client's job to notice this, and work out whether a NAT mapping
 *     is required on the new network, and make a new NAT mapping request if necessary.
 *     The DNSServiceNATPortMappingCreate API does this for you, automatically.
 *     The client just needs to make one call to the PortMapping API, and its callback will
 *     be invoked any time the mapping state changes. This property complements point (1) above.
 *     If the client didn't make a NAT mapping request just because it determined that one was
 *     not required at that particular moment in time, the client would then have to monitor
 *     for network state changes to determine if a NAT port mapping later became necessary.
 *     By unconditionally making a NAT mapping request, even when a NAT mapping not to be
 *     necessary, the PortMapping API will then begin monitoring network state changes on behalf of
 *     the client, and if a NAT mapping later becomes necessary, it will automatically create a NAT
 *     mapping and inform the client with a new callback giving the new address and port information.
 *
 * DNSServiceNATPortMappingReply() parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceNATPortMappingCreate().
 *
 * flags:           Currently unused, reserved for future use.
 *
 * interfaceIndex:  The interface through which the NAT gateway is reached.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success.
 *                  Will be kDNSServiceErr_DoubleNAT when the NAT gateway is itself behind one or
 *                  more layers of NAT, in which case the other parameters have the defined values.
 *                  For other failures, will indicate the failure that occurred, and the other
 *                  parameters are undefined.
 *
 * externalAddress: Four byte IPv4 address in network byte order.
 *
 * protocol:        Will be kDNSServiceProtocol_UDP or kDNSServiceProtocol_TCP or both.
 *
 * internalPort:    The port on the local machine that was mapped.
 *
 * externalPort:    The actual external port in the NAT gateway that was mapped.
 *                  This is likely to be different than the requested external port.
 *
 * ttl:             The lifetime of the NAT port mapping created on the gateway.
 *                  This controls how quickly stale mappings will be garbage-collected
 *                  if the client machine crashes, suffers a power failure, is disconnected
 *                  from the network, or suffers some other unfortunate demise which
 *                  causes it to vanish without explicitly removing its NAT port mapping.
 *                  It's possible that the ttl value will differ from the requested ttl value.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceNATPortMappingReply)
(
    DNSServiceRef sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceErrorType errorCode,
    uint32_t externalAddress,                           /* four byte IPv4 address in network byte order */
    DNSServiceProtocol protocol,
    uint16_t internalPort,                              /* In network byte order */
    uint16_t externalPort,                              /* In network byte order and may be different than the requested port */
    uint32_t ttl,                                       /* may be different than the requested ttl */
    void                             *context
);


/* DNSServiceNATPortMappingCreate() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds then it
 *                  initializes the DNSServiceRef, returns kDNSServiceErr_NoError, and the nat
 *                  port mapping will last indefinitely until the client terminates the port
 *                  mapping request by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * interfaceIndex:  The interface on which to create port mappings in a NAT gateway. Passing 0 causes
 *                  the port mapping request to be sent on the primary interface.
 *
 * protocol:        To request a port mapping, pass in kDNSServiceProtocol_UDP, or kDNSServiceProtocol_TCP,
 *                  or (kDNSServiceProtocol_UDP | kDNSServiceProtocol_TCP) to map both.
 *                  The local listening port number must also be specified in the internalPort parameter.
 *                  To just discover the NAT gateway's external IP address, pass zero for protocol,
 *                  internalPort, externalPort and ttl.
 *
 * internalPort:    The port number in network byte order on the local machine which is listening for packets.
 *
 * externalPort:    The requested external port in network byte order in the NAT gateway that you would
 *                  like to map to the internal port. Pass 0 if you don't care which external port is chosen for you.
 *
 * ttl:             The requested renewal period of the NAT port mapping, in seconds.
 *                  If the client machine crashes, suffers a power failure, is disconnected from
 *                  the network, or suffers some other unfortunate demise which causes it to vanish
 *                  unexpectedly without explicitly removing its NAT port mappings, then the NAT gateway
 *                  will garbage-collect old stale NAT port mappings when their lifetime expires.
 *                  Requesting a short TTL causes such orphaned mappings to be garbage-collected
 *                  more promptly, but consumes system resources and network bandwidth with
 *                  frequent renewal packets to keep the mapping from expiring.
 *                  Requesting a long TTL is more efficient on the network, but in the event of the
 *                  client vanishing, stale NAT port mappings will not be garbage-collected as quickly.
 *                  Most clients should pass 0 to use a system-wide default value.
 *
 * callBack:        The function to be called when the port mapping request succeeds or fails asynchronously.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on success (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred.
 *
 *                  If you don't actually want a port mapped, and are just calling the API
 *                  because you want to find out the NAT's external IP address (e.g. for UI
 *                  display) then pass zero for protocol, internalPort, externalPort and ttl.
 */

DNSServiceErrorType DNSSD_API DNSServiceNATPortMappingCreate
(
    DNSServiceRef                    *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    DNSServiceProtocol protocol,                        /* TCP and/or UDP          */
    uint16_t internalPort,                              /* network byte order      */
    uint16_t externalPort,                              /* network byte order      */
    uint32_t ttl,                                       /* time to live in seconds */
    DNSServiceNATPortMappingReply callBack,
    void                             *context           /* may be NULL             */
);


/*********************************************************************************************
*
*  General Utility Functions
*
*********************************************************************************************/

/* DNSServiceConstructFullName()
 *
 * Concatenate a three-part domain name (as returned by the above callbacks) into a
 * properly-escaped full domain name. Note that callbacks in the above functions ALREADY ESCAPE
 * strings where necessary.
 *
 * Parameters:
 *
 * fullName:        A pointer to a buffer that where the resulting full domain name is to be written.
 *                  The buffer must be kDNSServiceMaxDomainName (1009) bytes in length to
 *                  accommodate the longest legal domain name without buffer overrun.
 *
 * service:         The service name - any dots or backslashes must NOT be escaped.
 *                  May be NULL (to construct a PTR record name, e.g.
 *                  "_ftp._tcp.apple.com.").
 *
 * regtype:         The service type followed by the protocol, separated by a dot
 *                  (e.g. "_ftp._tcp").
 *
 * domain:          The domain name, e.g. "apple.com.". Literal dots or backslashes,
 *                  if any, must be escaped, e.g. "1st\. Floor.apple.com."
 *
 * return value:    Returns kDNSServiceErr_NoError (0) on success, kDNSServiceErr_BadParam on error.
 *
 */

DNSServiceErrorType DNSSD_API DNSServiceConstructFullName
(
    char                            * const fullName,
    const char                      * const service,      /* may be NULL */
    const char                      * const regtype,
    const char                      * const domain
);


/*********************************************************************************************
*
*   TXT Record Construction Functions
*
*********************************************************************************************/

/*
 * A typical calling sequence for TXT record construction is something like:
 *
 * Client allocates storage for TXTRecord data (e.g. declare buffer on the stack)
 * TXTRecordCreate();
 * TXTRecordSetValue();
 * TXTRecordSetValue();
 * TXTRecordSetValue();
 * ...
 * DNSServiceRegister( ... TXTRecordGetLength(), TXTRecordGetBytesPtr() ... );
 * TXTRecordDeallocate();
 * Explicitly deallocate storage for TXTRecord data (if not allocated on the stack)
 */


/* TXTRecordRef
 *
 * Opaque internal data type.
 * Note: Represents a DNS-SD TXT record.
 */

typedef union _TXTRecordRef_t { char PrivateData[16]; char *ForceNaturalAlignment; } TXTRecordRef;


/* TXTRecordCreate()
 *
 * Creates a new empty TXTRecordRef referencing the specified storage.
 *
 * If the buffer parameter is NULL, or the specified storage size is not
 * large enough to hold a key subsequently added using TXTRecordSetValue(),
 * then additional memory will be added as needed using malloc().
 *
 * On some platforms, when memory is low, malloc() may fail. In this
 * case, TXTRecordSetValue() will return kDNSServiceErr_NoMemory, and this
 * error condition will need to be handled as appropriate by the caller.
 *
 * You can avoid the need to handle this error condition if you ensure
 * that the storage you initially provide is large enough to hold all
 * the key/value pairs that are to be added to the record.
 * The caller can precompute the exact length required for all of the
 * key/value pairs to be added, or simply provide a fixed-sized buffer
 * known in advance to be large enough.
 * A no-value (key-only) key requires  (1 + key length) bytes.
 * A key with empty value requires     (1 + key length + 1) bytes.
 * A key with non-empty value requires (1 + key length + 1 + value length).
 * For most applications, DNS-SD TXT records are generally
 * less than 100 bytes, so in most cases a simple fixed-sized
 * 256-byte buffer will be more than sufficient.
 * Recommended size limits for DNS-SD TXT Records are discussed in
 * <http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt>
 *
 * Note: When passing parameters to and from these TXT record APIs,
 * the key name does not include the '=' character. The '=' character
 * is the separator between the key and value in the on-the-wire
 * packet format; it is not part of either the key or the value.
 *
 * txtRecord:       A pointer to an uninitialized TXTRecordRef.
 *
 * bufferLen:       The size of the storage provided in the "buffer" parameter.
 *
 * buffer:          Optional caller-supplied storage used to hold the TXTRecord data.
 *                  This storage must remain valid for as long as
 *                  the TXTRecordRef.
 */

void DNSSD_API TXTRecordCreate
(
    TXTRecordRef     *txtRecord,
    uint16_t bufferLen,
    void             *buffer
);


/* TXTRecordDeallocate()
 *
 * Releases any resources allocated in the course of preparing a TXT Record
 * using TXTRecordCreate()/TXTRecordSetValue()/TXTRecordRemoveValue().
 * Ownership of the buffer provided in TXTRecordCreate() returns to the client.
 *
 * txtRecord:           A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 */

void DNSSD_API TXTRecordDeallocate
(
    TXTRecordRef     *txtRecord
);


/* TXTRecordSetValue()
 *
 * Adds a key (optionally with value) to a TXTRecordRef. If the "key" already
 * exists in the TXTRecordRef, then the current value will be replaced with
 * the new value.
 * Keys may exist in four states with respect to a given TXT record:
 *  - Absent (key does not appear at all)
 *  - Present with no value ("key" appears alone)
 *  - Present with empty value ("key=" appears in TXT record)
 *  - Present with non-empty value ("key=value" appears in TXT record)
 * For more details refer to "Data Syntax for DNS-SD TXT Records" in
 * <http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt>
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * key:             A null-terminated string which only contains printable ASCII
 *                  values (0x20-0x7E), excluding '=' (0x3D). Keys should be
 *                  9 characters or fewer (not counting the terminating null).
 *
 * valueSize:       The size of the value.
 *
 * value:           Any binary value. For values that represent
 *                  textual data, UTF-8 is STRONGLY recommended.
 *                  For values that represent textual data, valueSize
 *                  should NOT include the terminating null (if any)
 *                  at the end of the string.
 *                  If NULL, then "key" will be added with no value.
 *                  If non-NULL but valueSize is zero, then "key=" will be
 *                  added with empty value.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_Invalid if the "key" string contains
 *                  illegal characters.
 *                  Returns kDNSServiceErr_NoMemory if adding this key would
 *                  exceed the available storage.
 */

DNSServiceErrorType DNSSD_API TXTRecordSetValue
(
    TXTRecordRef     *txtRecord,
    const char       *key,
    uint8_t valueSize,                 /* may be zero */
    const void       *value            /* may be NULL */
);


/* TXTRecordRemoveValue()
 *
 * Removes a key from a TXTRecordRef. The "key" must be an
 * ASCII string which exists in the TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * key:             A key name which exists in the TXTRecordRef.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_NoSuchKey if the "key" does not
 *                  exist in the TXTRecordRef.
 */

DNSServiceErrorType DNSSD_API TXTRecordRemoveValue
(
    TXTRecordRef     *txtRecord,
    const char       *key
);


/* TXTRecordGetLength()
 *
 * Allows you to determine the length of the raw bytes within a TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * return value:    Returns the size of the raw bytes inside a TXTRecordRef
 *                  which you can pass directly to DNSServiceRegister() or
 *                  to DNSServiceUpdateRecord().
 *                  Returns 0 if the TXTRecordRef is empty.
 */

uint16_t DNSSD_API TXTRecordGetLength
(
    const TXTRecordRef *txtRecord
);


/* TXTRecordGetBytesPtr()
 *
 * Allows you to retrieve a pointer to the raw bytes within a TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * return value:    Returns a pointer to the raw bytes inside the TXTRecordRef
 *                  which you can pass directly to DNSServiceRegister() or
 *                  to DNSServiceUpdateRecord().
 */

const void * DNSSD_API TXTRecordGetBytesPtr
(
    const TXTRecordRef *txtRecord
);


/*********************************************************************************************
*
*   TXT Record Parsing Functions
*
*********************************************************************************************/

/*
 * A typical calling sequence for TXT record parsing is something like:
 *
 * Receive TXT record data in DNSServiceResolve() callback
 * if (TXTRecordContainsKey(txtLen, txtRecord, "key")) then do something
 * val1ptr = TXTRecordGetValuePtr(txtLen, txtRecord, "key1", &len1);
 * val2ptr = TXTRecordGetValuePtr(txtLen, txtRecord, "key2", &len2);
 * ...
 * memcpy(myval1, val1ptr, len1);
 * memcpy(myval2, val2ptr, len2);
 * ...
 * return;
 *
 * If you wish to retain the values after return from the DNSServiceResolve()
 * callback, then you need to copy the data to your own storage using memcpy()
 * or similar, as shown in the example above.
 *
 * If for some reason you need to parse a TXT record you built yourself
 * using the TXT record construction functions above, then you can do
 * that using TXTRecordGetLength and TXTRecordGetBytesPtr calls:
 * TXTRecordGetValue(TXTRecordGetLength(x), TXTRecordGetBytesPtr(x), key, &len);
 *
 * Most applications only fetch keys they know about from a TXT record and
 * ignore the rest.
 * However, some debugging tools wish to fetch and display all keys.
 * To do that, use the TXTRecordGetCount() and TXTRecordGetItemAtIndex() calls.
 */

/* TXTRecordContainsKey()
 *
 * Allows you to determine if a given TXT Record contains a specified key.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * key:             A null-terminated ASCII string containing the key name.
 *
 * return value:    Returns 1 if the TXT Record contains the specified key.
 *                  Otherwise, it returns 0.
 */

int DNSSD_API TXTRecordContainsKey
(
    uint16_t txtLen,
    const void       *txtRecord,
    const char       *key
);


/* TXTRecordGetValuePtr()
 *
 * Allows you to retrieve the value for a given key from a TXT Record.
 *
 * txtLen:          The size of the received TXT Record
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * key:             A null-terminated ASCII string containing the key name.
 *
 * valueLen:        On output, will be set to the size of the "value" data.
 *
 * return value:    Returns NULL if the key does not exist in this TXT record,
 *                  or exists with no value (to differentiate between
 *                  these two cases use TXTRecordContainsKey()).
 *                  Returns pointer to location within TXT Record bytes
 *                  if the key exists with empty or non-empty value.
 *                  For empty value, valueLen will be zero.
 *                  For non-empty value, valueLen will be length of value data.
 */

const void * DNSSD_API TXTRecordGetValuePtr
(
    uint16_t txtLen,
    const void       *txtRecord,
    const char       *key,
    uint8_t          *valueLen
);


/* TXTRecordGetCount()
 *
 * Returns the number of keys stored in the TXT Record. The count
 * can be used with TXTRecordGetItemAtIndex() to iterate through the keys.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * return value:    Returns the total number of keys in the TXT Record.
 *
 */

uint16_t DNSSD_API TXTRecordGetCount
(
    uint16_t txtLen,
    const void       *txtRecord
);


/* TXTRecordGetItemAtIndex()
 *
 * Allows you to retrieve a key name and value pointer, given an index into
 * a TXT Record. Legal index values range from zero to TXTRecordGetCount()-1.
 * It's also possible to iterate through keys in a TXT record by simply
 * calling TXTRecordGetItemAtIndex() repeatedly, beginning with index zero
 * and increasing until TXTRecordGetItemAtIndex() returns kDNSServiceErr_Invalid.
 *
 * On return:
 * For keys with no value, *value is set to NULL and *valueLen is zero.
 * For keys with empty value, *value is non-NULL and *valueLen is zero.
 * For keys with non-empty value, *value is non-NULL and *valueLen is non-zero.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * itemIndex:       An index into the TXT Record.
 *
 * keyBufLen:       The size of the string buffer being supplied.
 *
 * key:             A string buffer used to store the key name.
 *                  On return, the buffer contains a null-terminated C string
 *                  giving the key name. DNS-SD TXT keys are usually
 *                  9 characters or fewer. To hold the maximum possible
 *                  key name, the buffer should be 256 bytes long.
 *
 * valueLen:        On output, will be set to the size of the "value" data.
 *
 * value:           On output, *value is set to point to location within TXT
 *                  Record bytes that holds the value data.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_NoMemory if keyBufLen is too short.
 *                  Returns kDNSServiceErr_Invalid if index is greater than
 *                  TXTRecordGetCount()-1.
 */

DNSServiceErrorType DNSSD_API TXTRecordGetItemAtIndex
(
    uint16_t txtLen,
    const void       *txtRecord,
    uint16_t itemIndex,
    uint16_t keyBufLen,
    char             *key,
    uint8_t          *valueLen,
    const void       **value
);

#if _DNS_SD_LIBDISPATCH
/*
 * DNSServiceSetDispatchQueue
 *
 * Allows you to schedule a DNSServiceRef on a serial dispatch queue for receiving asynchronous
 * callbacks.  It's the clients responsibility to ensure that the provided dispatch queue is running.
 *
 * A typical application that uses CFRunLoopRun or dispatch_main on its main thread will
 * usually schedule DNSServiceRefs on its main queue (which is always a serial queue)
 * using "DNSServiceSetDispatchQueue(sdref, dispatch_get_main_queue());"
 *
 * If there is any error during the processing of events, the application callback will
 * be called with an error code. For shared connections, each subordinate DNSServiceRef
 * will get its own error callback. Currently these error callbacks only happen
 * if the daemon is manually terminated or crashes, and the error
 * code in this case is kDNSServiceErr_ServiceNotRunning. The application must call
 * DNSServiceRefDeallocate to free the DNSServiceRef when it gets such an error code.
 * These error callbacks are rare and should not normally happen on customer machines,
 * but application code should be written defensively to handle such error callbacks
 * gracefully if they occur.
 *
 * After using DNSServiceSetDispatchQueue on a DNSServiceRef, calling DNSServiceProcessResult
 * on the same DNSServiceRef will result in undefined behavior and should be avoided.
 *
 * Once the application successfully schedules a DNSServiceRef on a serial dispatch queue using
 * DNSServiceSetDispatchQueue, it cannot remove the DNSServiceRef from the dispatch queue, or use
 * DNSServiceSetDispatchQueue a second time to schedule the DNSServiceRef onto a different serial dispatch
 * queue. Once scheduled onto a dispatch queue a DNSServiceRef will deliver events to that queue until
 * the application no longer requires that operation and terminates it using DNSServiceRefDeallocate.
 *
 * service:         DNSServiceRef that was allocated and returned to the application, when the
 *                  application calls one of the DNSService API.
 *
 * queue:           dispatch queue where the application callback will be scheduled
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_NoMemory if it cannot create a dispatch source
 *                  Returns kDNSServiceErr_BadParam if the service param is invalid or the
 *                  queue param is invalid
 */

DNSServiceErrorType DNSSD_API DNSServiceSetDispatchQueue
(
    DNSServiceRef service,
    dispatch_queue_t queue
);
#endif //_DNS_SD_LIBDISPATCH

#if !defined(_WIN32)
typedef void (DNSSD_API *DNSServiceSleepKeepaliveReply)
(
    DNSServiceRef sdRef,
    DNSServiceErrorType errorCode,
    void                                *context
);
DNSServiceErrorType DNSSD_API DNSServiceSleepKeepalive
(
    DNSServiceRef                       *sdRef,
    DNSServiceFlags flags,
    int fd,
    unsigned int timeout,
    DNSServiceSleepKeepaliveReply callBack,
    void                                *context
);
#endif

#ifdef APPLE_OSX_mDNSResponder
/* DNSServiceCreateDelegateConnection()
 *
 * Create a delegate connection to the daemon allowing efficient registration of
 * multiple individual records.
 *
 * Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. Deallocating
 *                  the reference (via DNSServiceRefDeallocate()) severs the
 *                  connection and deregisters all records registered on this connection.
 *
 * pid :            Process ID of the delegate
 *
 * uuid:            UUID of the delegate
 *
 *                  Note that only one of the two arguments (pid or uuid) can be specified. If pid
 *                  is zero, uuid will be assumed to be a valid value; otherwise pid will be used.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred (in which
 *                  case the DNSServiceRef is not initialized). kDNSServiceErr_NotAuth is
 *                  returned to indicate that the calling process does not have entitlements
 *                  to use this API.
 */
DNSServiceErrorType DNSSD_API DNSServiceCreateDelegateConnection(DNSServiceRef *sdRef, int32_t pid, uuid_t uuid);
#endif

#ifdef __APPLE_API_PRIVATE

#define kDNSServiceCompPrivateDNS   "PrivateDNS"
#define kDNSServiceCompMulticastDNS "MulticastDNS"

#endif //__APPLE_API_PRIVATE

/* Some C compiler cleverness. We can make the compiler check certain things for us,
 * and report errors at compile-time if anything is wrong. The usual way to do this would
 * be to use a run-time "if" statement or the conventional run-time "assert" mechanism, but
 * then you don't find out what's wrong until you run the software. This way, if the assertion
 * condition is false, the array size is negative, and the complier complains immediately.
 */

struct CompileTimeAssertionChecks_DNS_SD
{
    char assert0[(sizeof(union _TXTRecordRef_t) == 16) ? 1 : -1];
};

#ifdef  __cplusplus
}
#endif

#endif  /* _DNS_SD_H */
