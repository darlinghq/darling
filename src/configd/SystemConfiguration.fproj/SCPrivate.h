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

#ifndef _SCPRIVATE_H
#define _SCPRIVATE_H

#include <sys/cdefs.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <mach/message.h>
#include <os/log.h>
#include <os/variant_private.h>
#include <sys/sysctl.h>

#include <CoreFoundation/CoreFoundation.h>

/* SCDynamicStore SPIs */
#include <SystemConfiguration/SCDynamicStorePrivate.h>
#include <SystemConfiguration/SCDynamicStoreCopySpecificPrivate.h>
#include <SystemConfiguration/SCDynamicStoreSetSpecificPrivate.h>

/* SCPreferences SPIs */
#include <SystemConfiguration/SCPreferencesPrivate.h>
#include <SystemConfiguration/SCPreferencesGetSpecificPrivate.h>
#include <SystemConfiguration/SCPreferencesSetSpecificPrivate.h>

/* [private] Schema Definitions (for SCDynamicStore and SCPreferences) */
#include <SystemConfiguration/SCSchemaDefinitionsPrivate.h>

/* SCNetworkConfiguration SPIs */
#include <SystemConfiguration/SCNetworkConfigurationPrivate.h>

/* SCNetworkConnection SPIs */
#include <SystemConfiguration/SCNetworkConnectionPrivate.h>

/* Keychain SPIs */
#include <SystemConfiguration/SCPreferencesKeychainPrivate.h>

/*!
	@header SCPrivate
 */


/* "server" defines */
#if	!TARGET_OS_SIMULATOR
#define _SC_SERVER_PROG			"configd"
#else	// !TARGET_OS_SIMULATOR
#define _SC_SERVER_PROG			"configd_sim"
#endif	// !TARGET_OS_SIMULATOR


/* atomic operations */
#define _SC_ATOMIC_CMPXCHG(p, o, n)	__sync_bool_compare_and_swap((p), (o), (n))
#define _SC_ATOMIC_INC(p)		__sync_fetch_and_add((p), 1)		// return (n++);
#define _SC_ATOMIC_DEC(p)		__sync_sub_and_fetch((p), 1)		// return (--n);
#define _SC_ATOMIC_ZERO(p)		__sync_fetch_and_and((p), 0)		// old_n = n; n = 0; return(old_n);


/* framework path */
#if	!TARGET_OS_WATCH
#define	SYSTEMCONFIGURATION_FRAMEWORK_PATH	"/System/Library/Frameworks/SystemConfiguration.framework"
#else
#define	SYSTEMCONFIGURATION_FRAMEWORK_PATH	"/System/Library/PrivateFrameworks/SystemConfiguration.framework"
#endif


/* get-network-info script path */
#if	TARGET_OS_OSX
#define SYSTEMCONFIGURATION_GET_NETWORK_INFO_PATH	SYSTEMCONFIGURATION_FRAMEWORK_PATH "/Resources/get-network-info"
#else
#define SYSTEMCONFIGURATION_GET_NETWORK_INFO_PATH	SYSTEMCONFIGURATION_FRAMEWORK_PATH "/get-network-info"
#endif


/* crash report(s) directory path */
#if	TARGET_OS_OSX
#define	_SC_CRASH_DIR	"/Library/Logs/DiagnosticReports"
#else
#define	_SC_CRASH_DIR	"/Library/Logs/CrashReporter"
#endif


/* framework variables */
extern int	_sc_debug;	/* non-zero if debugging enabled */
extern int	_sc_verbose;	/* non-zero if verbose logging enabled */
extern int	_sc_log;	/* 0 if SC messages should be written to stdout/stderr,
				   1 if SC messages should be logged w/os_log(3),
				   2 if SC messages should be written to stdout/stderr AND logged */


/* notify(3) keys */

#define _SC_NOTIFY_PREFIX		"com.apple.system.config"
#define _SC_NOTIFY_NETWORK_CHANGE	_SC_NOTIFY_PREFIX ".network_change"
#define _SC_NOTIFY_NETWORK_CHANGE_DNS	_SC_NOTIFY_NETWORK_CHANGE ".dns"
#define _SC_NOTIFY_NETWORK_CHANGE_NWI	_SC_NOTIFY_NETWORK_CHANGE ".nwi"
#define _SC_NOTIFY_NETWORK_CHANGE_PROXY	_SC_NOTIFY_PREFIX ".proxy_change"


/*!
	@group SCNetworkReachabilityCreateWithOptions #defines
	@discussion The following defines the keys and values that can
		be passed to the SCNetworkReachabilityCreateWithOptions
		API.
 */

/*!
	@constant kSCNetworkReachabilityOptionNodeName
	@discussion A CFString that will be passed to getaddrinfo(3).  An acceptable
		value is either a valid host name or a numeric host address string
		consisting of a dotted decimal IPv4 address or an IPv6 address.
 */
#define kSCNetworkReachabilityOptionNodeName			CFSTR("nodename")

/*!
	@constant kSCNetworkReachabilityOptionLocalAddress
	@discussion A CFData wrapping a "struct sockaddr" that represents
		local address associated with a network connection.
 */
#define kSCNetworkReachabilityOptionLocalAddress		CFSTR("local-address")

/*!
	@constant kSCNetworkReachabilityOptionPTRAddress
	@discussion A CFData wrapping a "struct sockaddr" that represents
		the reverse-address to be queried.
 */
#define kSCNetworkReachabilityOptionPTRAddress			CFSTR("ptr-address")

/*!
	@constant kSCNetworkReachabilityOptionRemoteAddress
	@discussion A CFData wrapping a "struct sockaddr" that represents
		remote address associated with a network connection.
 */
#define kSCNetworkReachabilityOptionRemoteAddress		CFSTR("remote-address")

/*!
	@constant kSCNetworkReachabilityOptionInterface
	@discussion A CFString specifying that the reachability query should be
		limited to the provided network interface (e.g. "en0", "en1", ...).
 */
#define kSCNetworkReachabilityOptionInterface			CFSTR("interface")


/*!
	@constant kSCNetworkReachabilityOptionConnectionOnDemandBypass
	@discussion A CFBoolean that indicates if we should bypass the VPNOnDemand
		checks for this target.
 */
#define kSCNetworkReachabilityOptionConnectionOnDemandBypass	CFSTR("ConnectionOnDemandBypass")

/*!
	@constant kSCNetworkReachabilityOptionResolverBypass
	@discussion A CFBoolean that indicates if we should bypass resolving any
		node names.  Instead, the status of the DNS server configuration
		associated with the name will be returned. */
#define kSCNetworkReachabilityOptionResolverBypass		CFSTR("ResolverBypass")


/*!
	@constant kSCNetworkReachabilityOptionServerBypass
	@discussion A CFBoolean that indicates if we should bypass usage of the
		SCNetworkReachability "server" for this target.
 */
#define kSCNetworkReachabilityOptionServerBypass		CFSTR("ServerBypass")




/*!
	@group
 */

__BEGIN_DECLS


#pragma mark -
#pragma mark SCError()


/*!
	@function _SCErrorSet
	@discussion Sets the last SystemConfiguration.framework API error code.
	@param error The error encountered.
 */
void		_SCErrorSet			(int			error);


#pragma mark -
#pragma mark Serialization/Unserialization


/*!
	@function _SCSerialize
	@discussion Serialize a CFPropertyList object for passing
		to/from configd.
	@param obj CFPropertyList object to serialize
	@param xml A pointer to a CFDataRef, NULL if data should be
		vm_allocated.
	@param dataRef A pointer to the newly allocated/serialized data
	@param dataLen A pointer to the length in bytes of the newly
		allocated/serialized data
 */
Boolean		_SCSerialize			(CFPropertyListRef	obj,
						 CFDataRef		*xml,
						 void			**dataRef,
						 CFIndex		*dataLen);

/*!
	@function _SCUnserialize
	@discussion Unserialize a stream of bytes passed from/to configd
		into a CFPropertyList object.
	@param obj A pointer to memory that will be filled with the CFPropertyList
		associated with the stream of bytes.
	@param xml CFDataRef with the serialized data
	@param dataRef A pointer to the serialized data
	@param dataLen A pointer to the length of the serialized data

	Specify either "xml" or "data/dataLen".
 */
Boolean		_SCUnserialize			(CFPropertyListRef	*obj,
						 CFDataRef		xml,
						 void			*dataRef,
						 CFIndex		dataLen);

/*!
	@function _SCSerializeString
	@discussion Serialize a CFString object for passing
		to/from configd.
	@param str CFString key to serialize
	@param data A pointer to a CFDataRef, NULL if storage should be
		vm_allocated.
	@param dataRef A pointer to the newly allocated/serialized data
	@param dataLen A pointer to the length in bytes of the newly
		allocated/serialized data
 */
Boolean		_SCSerializeString		(CFStringRef		str,
						 CFDataRef		*data,
						 void			**dataRef,
						 CFIndex		*dataLen);

/*!
	@function _SCUnserializeString
	@discussion Unserialize a stream of bytes passed from/to configd
		into a CFString object.
	@param str A pointer to memory that will be filled with the CFString
		associated with the stream of bytes.
	@param utf8 CFDataRef with the serialized data
	@param dataRef A pointer to the serialized data
	@param dataLen A pointer to the length of the serialized data

	Specify either "utf8" or "data/dataLen".
 */
Boolean		_SCUnserializeString		(CFStringRef		*str,
						 CFDataRef		utf8,
						 void			*dataRef,
						 CFIndex		dataLen);

/*!
	@function _SCSerializeData
	@discussion Serialize a CFData object for passing
		to/from configd.
	@param data CFData key to serialize
	@param dataRef A pointer to the newly allocated/serialized data
	@param dataLen A pointer to the length in bytes of the newly
		allocated/serialized data
 */
Boolean		_SCSerializeData		(CFDataRef		data,
						 void			**dataRef,
						 CFIndex		*dataLen);

/*!
	@function _SCUnserializeData
	@discussion Unserialize a stream of bytes passed from/to configd
		into a CFData object.
	@param data A pointer to memory that will be filled with the CFData
		associated with the stream of bytes.
	@param dataRef A pointer to the serialized data
	@param dataLen A pointer to the length of the serialized data
 */
Boolean		_SCUnserializeData		(CFDataRef		*data,
						 void			*dataRef,
						 CFIndex		dataLen);

/*!
	@function _SCSerializeMultiple
	@discussion Convert a CFDictionary containing a set of CFPropertlyList
		values into a CFDictionary containing a set of serialized CFData
		values.
	@param dict The CFDictionary with CFPropertyList values.
	@result The serialized CFDictionary with CFData values
 */
CF_RETURNS_RETAINED
CFDictionaryRef	_SCSerializeMultiple		(CFDictionaryRef	dict);

/*!
	@function _SCUnserializeMultiple
	@discussion Convert a CFDictionary containing a set of CFData
		values into a CFDictionary containing a set of serialized
		CFPropertlyList values.
	@param dict The CFDictionary with CFData values.
	@result The serialized CFDictionary with CFPropertyList values
 */
CF_RETURNS_RETAINED
CFDictionaryRef	_SCUnserializeMultiple		(CFDictionaryRef	dict);


#pragma mark -


/*!
	@function _SCCreatePropertyListFromResource
	@discussion Reads a property list referenced by a file URL.
	@param url The file URL.
	@result The CFPropertyList content of the URL.
 */
CFPropertyListRef
_SCCreatePropertyListFromResource		(CFURLRef		url);


#pragma mark -
#pragma mark String conversion


/*!
	@function _SC_cfstring_to_cstring
	@discussion Extracts a C-string from a CFString.
	@param cfstr The CFString to extract the data from.
	@param buf A user provided buffer of the specified length.  If NULL,
		a new buffer will be allocated to contain the C-string.  It
		is the responsiblity of the caller to free an allocated
		buffer.
	@param bufLen The size of the user provided buffer.
	@param encoding The string encoding
	@result If the extraction (conversion) is successful then a pointer
		to the user provided (or allocated) buffer is returned, NULL
		if the string could not be extracted.
 */
char *		_SC_cfstring_to_cstring		(CFStringRef		cfstr,
						 char			*buf,
						 CFIndex		bufLen,
						 CFStringEncoding	encoding);

/*!
 *      @function _SC_sockaddr_to_string
 *      @discussion Formats a "struct sockaddr" for reporting
 *      @param address The address to format
 *	@param buf A user provided buffer of the specified length.
 *	@param bufLen The size of the user provided buffer.
 */
void		_SC_sockaddr_to_string		(const struct sockaddr  *address,
						 char			*buf,
						 size_t			bufLen);


/*!
 *	@function _SC_string_to_sockaddr
 *	@discussion Parses a string into a "struct sockaddr"
 *	@param str The address string to parse
 *	@param af Allowed address families (AF_UNSPEC, AF_INET, AF_INET6)
 *	@param buf A user provided buffer of the specified length; NULL
 *		if a new buffer should be allocated (and deallocated by the
 *		caller).
 *	@param bufLen The size of the user provided buffer.
 *	@result A pointer to the parsed "struct sockaddr"; NULL if
 *		the string could not be parsed as an IP[v6] address.
 */
struct sockaddr *
_SC_string_to_sockaddr				(const char		*str,
						 sa_family_t		af,
						 void			*buf,
						 size_t			bufLen);

/*!
 *	@function _SC_trimDomain
 *	@discussion Trims leading and trailing "."s from a domain or host name
 *	@param domain The domain name to trim
 *	@result The trimmed domain name.
 */
CF_RETURNS_RETAINED
CFStringRef	_SC_trimDomain			(CFStringRef		domain);


#pragma mark -
#pragma mark Mach IPC


/*!
	@function _SC_sendMachMessage
	@discussion Sends a trivial mach message (one with just a
		message ID) to the specified port.
	@param port The mach port.
	@param msg_id The message id.
 */
void		_SC_sendMachMessage		(mach_port_t		port,
						 mach_msg_id_t		msg_id);


#pragma mark -
#pragma mark Logging


/*!
	@function _SC_LOG_DEFAULT
	@discussion A function returning a default os_log_t object to be
		used for [SystemConfiguration] logging.
	@result The os_log_t object
 */
os_log_t	_SC_LOG_DEFAULT			(void);


/*!
	@function _SC_syslog_os_log_mapping
	@discussion Maps a syslog logging level to an os_log level.
	@param level The syslog logging level
	@result The os_log level
 */
os_log_type_t	_SC_syslog_os_log_mapping	(int			level);


/*!
	@function _SCCopyDescription
	@discussion Returns a formatted textual description of a CF object.
	@param cf The CFType object (a generic reference of type CFTypeRef) from
		which to derive a description.
	@param formatOptions A dictionary containing formatting options for the object.
	@result A string that contains a formatted description of cf.
 */
CFStringRef	_SCCopyDescription		(CFTypeRef		cf,
						 CFDictionaryRef	formatOptions);


/*!
	@function SCLog
	@discussion Conditionally issue a log message.
	@param condition A boolean value indicating if the message should be logged
	@param level A syslog(3) logging priority.
	@param formatString The format string
 */
void		SCLog				(Boolean		condition,
						 int			level,
						 CFStringRef		formatString,
						 ...)	CF_FORMAT_FUNCTION(3, 4);


/*!
	@function SC_log
	@discussion Issue an os_log() message.

	Note:	By default, the log messages will be associated with the
		"com.apple.SystemConfiguration" subsystem.  This behavior
		can be overwritten by #define'ing SC_LOG_HANDLE with the name
		of an os_log_t global (or a function that returns an os_log_t)
		*BEFORE* this header is #include'd. In that case, the noted
		log handle will be used.

		Also, by #define'ing SC_LOG_OR_PRINT, we will check the "_sc_log"
		global to see if the messages should [also] be directed to stdout/stderr.

	@param level The syslog(3 logging priority.
	@param __string The format string
	@result The specified message will be written to the unified logging system.
 */
#ifndef SC_log
  #ifdef	SC_LOG_HANDLE
    #ifndef	SC_LOG_HANDLE_TYPE
    #define SC_LOG_HANDLE_TYPE
    #endif	// SC_LOG_HANDLE_TYPE
    SC_LOG_HANDLE_TYPE	os_log_t	SC_LOG_HANDLE(void);
  #else	// SC_LOG_HANDLE
    #define	SC_LOG_HANDLE	_SC_LOG_DEFAULT		// use [SC] default os_log handle
    #ifndef	SC_LOG_OR_PRINT
      #define	USE_SC_LOG_OR_PRINT	1		// and use '_sc_log' to control os_log, printf
    #endif	// !SC_LOG_OR_PRINT
  #endif	// !SC_LOG_HANDLE

  #if	USE_SC_LOG_OR_PRINT
    #define	SC_log(__level, __format, ...)						\
	do {										\
		os_log_t	__handle = SC_LOG_HANDLE();				\
		os_log_type_t	__type   = _SC_syslog_os_log_mapping(__level);		\
											\
		if (((_sc_log != 1) && ((__level > LOG_DEBUG) || _sc_debug)) ||		\
		    os_log_type_enabled(__handle, __type)) {				\
			__SC_Log(__level,						\
				 CFSTR( __format ),					\
				 __handle,						\
				 __type,						\
				 __format,						\
				 ## __VA_ARGS__);					\
		}									\
	} while (0)
  #else	// USE_SC_LOG_OR_PRINT
    #define	SC_log(__level, __format, ...)						\
	do {										\
		os_log_t	__handle = SC_LOG_HANDLE();				\
		os_log_type_t	__type = _SC_syslog_os_log_mapping(__level);		\
											\
		os_log_with_type(__handle, __type, __format, ## __VA_ARGS__);		\
	} while (0)
  #endif	// USE_SC_LOG_OR_PRINT
#endif	// !SC_log


/*!
	@function __SC_Log
	@discussion Issue a log message w/os_log(3) or printf(3).
		The specified message will be written to the system message
		logger.
	@param level A syslog(3) logging priority. If less than 0, log message is multi-line
	@param format_CF The format string (as a CFString for stdout/stderr)
	@param log The os_log_t handle (for logging)
	@param type The os_log_type_t type (for logging)
	@param format The format string (for logging)
 stream.
 */
void		__SC_Log			(int		level,
						 CFStringRef	format_CF,
						 os_log_t	log,
						 os_log_type_t	type,
						 const char	*format,
						 ...)	CF_FORMAT_FUNCTION(2, 6) __attribute__((format(os_log, 5, 6)));


/*!
	@function SCPrint
	@discussion Conditionally issue a debug message.
		The message will be written to the specified output stream.
	@param condition A boolean value indicating if the message should be written
	@param stream The output stream for the log message.
	@param formatString The format string
 */
void		SCPrint				(Boolean		condition,
						 FILE			*stream,
						 CFStringRef		formatString,
						 ...)	CF_FORMAT_FUNCTION(3, 4);


#pragma mark -
#pragma mark Proxies


/*!
	@function SCNetworkProxiesCopyMatching
	@discussion Return the proxy configurations matching a target host
		and/or one associated with a specific network interface.
	@param globalConfiguration the proxy dictionary currently returned
		by SCDynamicStoreCopyProxies().
	@param server A CFString specying the hostname of interest; NULL if
		no specific hostname should be used in selecting the proxy
		configurations.
	@param interface A CFString specifying that the proxy configuration
		for the provided network interface (e.g. "en0", "en1", ...)
		should be returned; NULL if proxy usage will not be scoped
		to an interface.
	@result A CFArray containing the proxy configurations associated
		with the requested server and/or network interface.

 */
CFArrayRef
SCNetworkProxiesCopyMatching			(CFDictionaryRef	globalConfiguration,
						 CFStringRef		server,
						 CFStringRef		interface)	API_AVAILABLE(macos(10.7)) SPI_AVAILABLE(ios(5.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

#define kSCProxiesMatchServer		CFSTR("Server")		/* CFString */
#define kSCProxiesMatchInterface	CFSTR("Interface")	/* CFString */
#define kSCProxiesMatchExecutableUUID	CFSTR("UUID")		/* CFUUID */

/*!
	@function SCNetworkProxiesCopyMatchingWithOptions
	@discussion Return the proxy configurations matching a target host
		and/or one associated with a specific network interface.
	@param globalConfiguration the proxy dictionary currently returned
		by SCDynamicStoreCopyProxies().
	@param options A dictionary containing any (or none) of the following:
		key				value		description
		------------------------------------------------------------------------------------------------
		kSCProxiesMatchServer		CFString	The hostname of interest; do not include if no
								specific hostname should be used in selecting the
								proxy configurations.
		kSCProxiesMatchInterface	CFString	If present, specifies the network interface
								(e.g. "en0", "en1", ...) whose proxy configuration
								should be returned. If not present, then proxy usage
								will not be scoped to an interface.
		kSCProxiesMatchExecutableUUID	CFUUID		If present, specifies the Mach-O UUID of the executable
								on whose behalf the match operation is being performed.
								If kSCProxiesMatchInterface is present then this option
								is ignored. If not present, then the Mach-O UUID of
								the current process is used. The Mach-O UUID is used
								to match application-specific proxy configurations
								(i.e., if per-app VPN rules are in effect).
	@result A CFArray containing the proxy configurations associated with the given options.
 */
CFArrayRef
SCNetworkProxiesCopyMatchingWithOptions		(CFDictionaryRef	globalConfiguration,
						 CFDictionaryRef	options)	API_AVAILABLE(macos(10.10)) SPI_AVAILABLE(ios(8.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

extern const CFStringRef	kSCProxiesNoGlobal;

/*!
	@function SCNetworkProxiesCreateProxyAgentData
	@discussion Returns a serialized representation of a proxy configuration.
	@param proxyConfig A dictionary representing a proxy configuration
	@result returns a CFData representing a proxy configuration. This data is
		readable by all "config-agents" (Agents with domain as "SystemConfig")
		via config_agent_copy_proxy_information().
		You must release the returned value.
 */
CFDataRef
SCNetworkProxiesCreateProxyAgentData		(CFDictionaryRef	proxyConfig)	API_AVAILABLE(macos(10.12)) SPI_AVAILABLE(ios(10.0), tvos(10.0), watchos(3.0), bridgeos(3.0));

/*!
	@function SCDynamicStoreCopyProxiesWithOptions
	@discussion Return the proxy configurations matching the provided
		criteria.
	@param	store An SCDynamicStoreRef representing the dynamic store
		session that should be used for communication with the server.
		If NULL, a temporary session will be used.
	@param	options  A dictionary of proxy options which can include 1 (or more) of the following :
		key				value		description
		---------------------------------------------------------------------------------------
		kSCProxiesNoGlobal		CFBoolean	Bypass any "global" proxy configuration
		...
	@result Returns a dictionary containing key-value pairs that represent
		the current internet proxy settings;
		NULL if no proxy settings have been defined or if an error
		was encountered.
		You must release the returned value.
*/
CFDictionaryRef
SCDynamicStoreCopyProxiesWithOptions		(SCDynamicStoreRef	store,
						 CFDictionaryRef	options)	API_AVAILABLE(macos(10.9)) SPI_AVAILABLE(ios(7.0), tvos(9.0), watchos(1.0), bridgeos(1.0));

#pragma mark -
#pragma mark Reachability


/*!
	@function SCNetworkReachabilityCopyResolvedAddress
	@discussion Return the resolved addresses associated with the
		target host.
	@result A CFArray[CFData], where each CFData is a (struct sockaddr)
 */
CFArrayRef
SCNetworkReachabilityCopyResolvedAddress	(SCNetworkReachabilityRef	target,
						 int				*error_num);

/*!
	@function SCNetworkReachabilityCreateWithOptions
	@discussion Creates a reference to a specified network host.  The
		options allow the caller to specify the node name and/or
		the service name.  This reference can be used later to
		monitor the reachability of the target host.
	@param allocator The CFAllocator that should be used to allocate
		memory for the SCNetworkReachability object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param options A CFDictionary containing options specifying the
		network host.  The options reflect the arguments that would
		be passed to getaddrinfo().
  */
SCNetworkReachabilityRef
SCNetworkReachabilityCreateWithOptions		(CFAllocatorRef		allocator,
						 CFDictionaryRef	options);

/*!
	@function _SC_checkResolverReachabilityByAddress
	@discussion Check the reachability of a reverse DNS query
 */
Boolean
_SC_checkResolverReachabilityByAddress		(SCDynamicStoreRef		*storeP,
						 SCNetworkReachabilityFlags	*flags,
						 Boolean			*haveDNS,
						 struct sockaddr		*sa);

/*!
	@function SCNetworkReachabilityGetInterfaceIndex
	@discussion Returns the interface index associated with network interface that will
		be used to interact with the target host.
	@param target The SCNetworkReachability reference associated with the address or
		name to be checked for reachability.
	@result Returns the interface index associated with the target.  Returning -1 means that
		the target is not reachable.
 */
int
SCNetworkReachabilityGetInterfaceIndex		(SCNetworkReachabilityRef	target);


#pragma mark -
#pragma mark Domain
/*!
	@function    _SC_domainEndsWithDomain
	@discussion  Checks if one domain is a subset of another
	@param compare_domain The domain to be compared.
	@param match_domain The domain to be matched.
	@return TRUE if the match_domain is contained in the compare_domain.
		 FLASE otherwise.
 */
Boolean
_SC_domainEndsWithDomain			(CFStringRef			compare_domain,
						 CFStringRef 			match_domain);

/*!
 @function    _SC_hostMatchesDomain
 @discussion  Checks if a hostname matches a domain. "*" not accepted as a domain. Top-level domain matching not supported.
 The algorithm is as follows:

 1. Trim .’s and *’s from the front and back of hostname and domain.
 2. If the number of .’s left in the hostname and domain are equal, require an exact match.
 3. Else, if the number of .’s in the hostname is greater than the number of .’s in the domain, and the number of .’s in the domain is greater than zero, append a . to the front of the domain and do a suffix match on the hostname.
 4. Else, fail.

 Examples
 www.apple.com > * : NO
 www.apple.com > apple.com : YES
 www.badapple.com > apple.com : NO
 www.apple.com > .com : NO
 foobar > foobar : YES
 www.apple.com > www.apple.com : YES
 www.apple.com... > .*.apple.com. : YES

 @param hostname The specific hostname to check.
 @param domain The domain to be matched.
 @return TRUE if the hostname matches the domain. FALSE otherwise.
 */
Boolean
_SC_hostMatchesDomain				(CFStringRef			hostname,
						 CFStringRef			domain);

#pragma mark -
#pragma mark NetBIOS


#if	!TARGET_OS_IPHONE
/*
 * DOS encoding/codepage
 */
void
_SC_dos_encoding_and_codepage			(CFStringEncoding	macEncoding,
						 UInt32			macRegion,
						 CFStringEncoding	*dosEncoding,
						 UInt32			*dosCodepage);
#endif	// !TARGET_OS_IPHONE

#pragma mark -
#pragma mark Bundle


/*
 * bundle access
 */
CFBundleRef
_SC_CFBundleGet					(void);

CFStringRef
_SC_CFBundleCopyNonLocalizedString		(CFBundleRef		bundle,
						 CFStringRef		key,
						 CFStringRef		value,
						 CFStringRef		tableName);

#pragma mark -
#pragma mark Misc


/*
 * mach port access
 */
CFMachPortRef
_SC_CFMachPortCreateWithPort			(const char *		portDescription,
						 mach_port_t		portNum,
						 CFMachPortCallBack	callout,
						 CFMachPortContext	*context);

/*
 * misc
 */
static __inline__ Boolean
_SC_CFEqual(CFTypeRef val1, CFTypeRef val2)
{
	if (val1 == val2) {
	    return TRUE;
	}
	if (val1 != NULL && val2 != NULL) {
		return CFEqual(val1, val2);
	}
	return FALSE;
}

static __inline__ Boolean
_SC_isAppleInternal()
{
	static Boolean		isInternal;
	static dispatch_once_t	once;

	dispatch_once(&once, ^{
		isInternal = os_variant_has_internal_content("com.apple.SystemConfiguration");
	});

	return isInternal;
}

Boolean
_SC_isInstallEnvironment			(void);

#define	MODEL			CFSTR("Model")

CFStringRef
_SC_hw_model					(Boolean		trim);

void *
_SC_dlopen					(const char		*framework);

/*
 * debugging
 */

#ifdef	DEBUG_MACH_PORT_ALLOCATIONS
	#define __MACH_PORT_DEBUG(cond, str, port)		\
	do {							\
		if (cond) _SC_logMachPortReferences(str, port);	\
	} while (0)
#else	// DEBUG_MACH_PORT_ALLOCATIONS
	#define __MACH_PORT_DEBUG(cond, str, port)
#endif	// DEBUG_MACH_PORT_ALLOCATIONS

void
_SC_logMachPortStatus				(void);

void
_SC_logMachPortReferences			(const char		*str,
						 mach_port_t		port);

CFStringRef
_SC_copyBacktrace				(void);

void
_SC_crash					(const char		*crash_info,
						 CFStringRef		notifyHeader,
						 CFStringRef		notifyMessage);

static __inline__ void
_SC_crash_once					(const char		*crash_info,
						 CFStringRef		notifyHeader,
						 CFStringRef		notifyMessage)
{
	static dispatch_once_t	once;

	_dispatch_once(&once, ^{
		_SC_crash(crash_info, notifyHeader, notifyMessage);
	});
}

Boolean
_SC_getconninfo					(int				socket,
						 struct sockaddr_storage	*src_addr,
						 struct sockaddr_storage	*dest_addr,
						 int				*if_index,
						 uint32_t			*flags);

__END_DECLS

#endif	/* _SCPRIVATE_H */
