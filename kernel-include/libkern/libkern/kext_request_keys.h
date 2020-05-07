/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

#ifndef _LIBKERN_KEXT_REQUEST_KEYS_H
#define _LIBKERN_KEXT_REQUEST_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*********************************************************************
* This file defines keys (and values) for properties in kext_request
* collections and mkext archives used for loading in the kernel.
* An incoming request is always a serialized XML plist with at least
* a predicate, and optionally a dictionary of arguments.
*
* Some requests generate serialized XML plist responses, while others
* return raw data. See the predicate description for more.
*
* All of these keys are strictly for internal run-time communication
* between IOKitUser's OSKext module and xnu's OSKext class.
* Keys and values may change at any time without notice.
*********************************************************************/

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Top-Level Request Properties
/********************************************************************/
#endif

/* The Predicate Key
 * The value of this key indicates the operation to perform or the
 * information desired.
 */
#define kKextRequestPredicateKey                   "Kext Request Predicate"

/* The Arguments Key
 * The value of this key is a dictionary containing the arguments
 * for the request.
 */
#define kKextRequestArgumentsKey                   "Kext Request Arguments"

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Request Predicates - User-Space to Kernel
/********************************************************************/
#endif

/*********************************************************************
 * Nonprivileged requests from user -> kernel
 *
 * These requests do not require a privileged host port, as they just
 * return information about loaded kexts.
 **********/

/* Predicate: Get Loaded Kext Info
 * Argument:  (None)
 * Response:  An array of information about loaded kexts (see OSKextLib.h).
 * Op result: OSReturn indicating any errors in processing (see OSKextLib.h)
 *
 * Retrieves an array of dictionaries whose properties describe every kext
 * loaded at the time of the call.
 */
#define kKextRequestPredicateGetLoaded             "Get Loaded Kext Info"

/* Predicate: Get Loaded Kext Info By UUID
 * Argument:  (None)
 * Response:  An array of information about loaded kexts (see OSKextLib.h).
 * Op result: OSReturn indicating any errors in processing (see OSKextLib.h)
 *
 * Retrieves an array of dictionaries whose properties describe every kext
 * loaded at the time of the call.
 */
#define kKextRequestPredicateGetLoadedByUUID       "Get Loaded Kext Info By UUID"

/* Predicate: Get Loaded Kext UUID By Address
 * Argument:  An address to lookup
 * Response:  A UUID of the kext
 * Op result: OSReturn indicating any errors in processing (see OSKextLib.h)
 *
 * Retrieves the uuid of a loaded kext in whose address range the given
 * lookup address falls into.
 */
#define kKextRequestPredicateGetUUIDByAddress      "Get Kext UUID by Address"

/* Predicate: Get All Load Requests
 * Argument:  None
 * Response:  A set of bundle identifiers of all requested kext loads..
 * Op result: OSReturn indicating any errors in processing (see OSKextLib.h)
 *
 * Retrieves the bundle identifiers of all kexts that were requested to be
 * loaded since power on.
 *
 */
#define kKextRequestPredicateGetAllLoadRequests    "Get All Load Requests"


/*********************************************************************
 * Privileged requests from user -> kernel
 *
 * These requests all do something with kexts in the kernel or to
 * the OSKext system overall. The user-space caller of kext_request()
 * must have access to a privileged host port or these requests result
 * in an op_result of kOSKextReturnNotPrivileged.
 **********/

/* Predicate: Get Kernel Requests
 * Argument:  (None)
 * Response:  An array of kernel requests (see below).
 * Op result: OSReturn indicating any errors in processing (see OSKextLib.h)
 *
 * Retrieve the list of deferred load (and other) requests from OSKext.
 * This predicate is reserved for kextd, and we may be enforcing access
 * to the kextd process only.
 */
#define kKextRequestPredicateGetKernelRequests     "Get Kernel Requests"

/* Predicate: Load
 * Argument:  kKextRequestArgumentLoadRequestsKey
 * Response:  None (yet, may become an array of log message strings)
 * Op result: OSReturn indicating processing/load+start result (see OSKextLib.h)
 *
 * Load one or more kexts per the load requests in the arguments dict.
 * See kKextRequestArgumentLoadRequestsKey for more info.
 */
#define kKextRequestPredicateLoad                  "Load"

/* Predicate: Start
 * Argument:  kKextRequestArgumentBundleIdentifierKey (CFBundleIdentifier)
 * Response:  None (yet, may become an array of log message strings)
 * Op result: OSReturn indicating start result (see OSKextLib.h)
 *
 * Start a kext by bundle id. If it's already started, returns success.
 * If a kext's dependencies need to be started, they are also started.
 */
#define kKextRequestPredicateStart                 "Start"

/* Predicate: Stop
 * Argument:  kKextRequestArgumentBundleIdentifierKey (CFBundleIdentifier)
 * Response:  None (yet, may become an array of log message strings)
 * Op result: OSReturn indicating stop result (see OSKextLib.h)
 *
 * Stop a kext by bundle id if it can be stoppoed.
 * If it's already stopped, returns success.
 * Does not attempt to stop dependents; that will return an error.
 */
#define kKextRequestPredicateStop                  "Stop"

/* Predicate: Unload
 * Argument:  kKextRequestArgumentBundleIdentifierKey (CFBundleIdentifier)
 * Response:  None (yet, may become an array of log message strings)
 * Op result: OSReturn indicating stop+unload result (see OSKextLib.h)
 *
 * Stop and unload a kext by bundle id if it can be.
 * Does not attempt to stop dependents; that will return an error.
 */
#define kKextRequestPredicateUnload                "Unload"

#if PRAGMA_MARK
/********************************************************************/
#pragma mark Requests Predicates - Kernel to User Space (kextd)
/********************************************************************/
#endif
/* Predicate: Send Resource
 * Argument:  kKextRequestArgumentRequestTagKey
 * Argument:  kKextRequestArgumentBundleIdentifierKey
 * Argument:  kKextRequestArgumentNameKey
 * Argument:  kKextRequestArgumentValueKey
 * Argument:  kKextRequestArgumentResult
 * Response:  None
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Retrieves a resource file from a kext bundle. The callback corresponding
 * to the request will be invoked.
 */
#define kKextRequestPredicateSendResource          "Send Resource"

/*********************************************************************
 * Kernel Requests: from the kernel or loaded kexts up to kextd
 *
 * These requests come from within the kernel, and kextd retrieves
 * them using kKextRequestPredicateGetKernelRequests.
 **********/

/* Predicate: Kext Load Request
 * Argument:  kKextRequestArgumentBundleIdentifierKey
 * Response:  Asynchronous via a kKextRequestPredicateLoad from kextd
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Requests that kextd load the kext with the given identifier.
 * When kexts loads the kext, it informs the IOCatalogue of the load.
 * If the kext cannot be loaded, kextd or OSKext removes its personalities
 * from the kernel.
 */
#define kKextRequestPredicateRequestLoad           "Kext Load Request"

/* Predicate: Kext Load Notification
 * Argument:  kext identifier
 * Response:  None
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Informs kextd that the kernel has successfully loaded and started
 * a kext.
 */
#define kKextRequestPredicateLoadNotification      "Kext Load Notification"

/* Predicate: Kext Unload Notification
 * Argument:  kext identifier
 * Response:  None
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Informs kextd that the kernel has successfully stopped and unloaded
 * a kext.
 */
#define kKextRequestPredicateUnloadNotification    "Kext Unload Notification"

/* Predicate: Prelinked Kernel Request
 * Argument:  None
 * Response:  None
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Notifies kextd that the kernel we booted from was not prelinked, therefore
 * that kextd should try to create a prelinked kernel now.
 */
#define kKextRequestPredicateRequestPrelink        "Kext Prelinked Kernel Request"

/* Predicate: Kext Resource Request
 * Argument:  kKextRequestArgumentRequestTagKey
 * Argument:  kKextRequestArgumentBundleIdentifierKey
 * Argument:  kKextRequestArgumentNameKey
 * Response:  Asynchronous via a kKextRequestPredicateSendResource from kextd
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Requests a resource file from a kext bundle by identifier + filename.
 */
#define kKextRequestPredicateRequestResource       "Kext Resource Request"

/* Predicate: Kext Kextd Exit Request
 * Argument:  None
 * Response:  None
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Requests kextd exit for system shutdown.
 */
#define kKextRequestPredicateRequestKextdExit    "Kextd Exit"

/* Predicate: Dext Daemon Launch
 * Argument: kKextRequestArgumentBundleIdentifierKey
 * Argument: IOUserServerName
 * Response: Asynchronous via a DriverKit daemon checking in
 * Op result: OSReturn indicating result (see OSKextLib.h)
 *
 * Requests kextd to launch a driver extension userspace daemon.
 */
#define kKextRequestPredicateRequestDaemonLaunch "Dext Daemon Launch"

#if PRAGMA_MARK
/********************************************************************/
#pragma mark -
#pragma mark Generic Request Arguments
/********************************************************************/
#endif
/* Argument:  Kext Load Requests
 * Type:      Array of dictionaries (see Load Request Arguments below)
 * Used by:   kKextRequestPredicateLoad
 *
 * An array of dictionaries, each describing a single load operation to
 * be performed with its options. A kext load request is effectively a
 * nested series requests. Currently only one load request is embedded
 * in a user-space Load request, so the result is unambiguous. We might
 * change this, specifically for kextd, to allow all pending kernel
 * load requests to be rolled up into one blob. Might not be much win
 * in that, however. The nested logic makes the code difficult to read.
 */
#define kKextRequestArgumentLoadRequestsKey        "Kext Load Requests"

/* Argument:  CFBundleIdentifier
 * Type:      String
 * Used by:   several
 *
 * Any request that takes a bundle identifier uses this key.
 */
#define kKextRequestArgumentBundleIdentifierKey    "CFBundleIdentifier"

/* Argument:  OSReturn
 * Type:      Dictionary
 * Used by:   OSKext::copyInfo()
 *
 * Used to specify a subset of all possible info to be returned.
 */
#define kKextRequestArgumentInfoKeysKey          "Kext Request Info Keys"

/* Argument:  OSReturn
 * Type:      Number (OSReturn)
 * Used by:   several
 *
 * Contains the OSReturn/kern_return_t result of the request.
 */
#define kKextRequestArgumentResultKey              "Kext Request Result Code"

/* Argument:  Address
 * Type:      Number (OSReturn)
 * Used by:   OSKextGetUUIDByAddress
 *
 * Contains the address that needs to be looked up
 */
#define kKextRequestArgumentLookupAddressKey       "Kext Request Lookup Address"

/* Argument:  Value
 * Type:      Varies with the predicate
 * Used by:   several
 *
 * Used for all the Set-Enabled predicates, and also for Send Resource (OSData).
 */
#define kKextRequestArgumentValueKey               "Value"

/* Argument:  Filename
 * Type:      String
 * Used by:   kKextRequestPredicateSendResource
 *
 * Names the resource being sent to the kext
 */
#define kKextRequestArgumentNameKey                "Name"

/* Argument:  Filename
 * Type:      Data
 * Used by:   kKextRequestPredicateSendResource
 *
 * Contains the contents of the resource file being sent.
 */
#define kKextRequestArgumentFileContentsKey        "File Contents"

/* Argument:  Delay Autounload
 * Type:      Boolean
 * Default:   false
 *
 * Normally all kexts loaded are subject to normal autounload behavior:
 * when no OSMetaClass instances remain for a kext that defines an IOService
 * subclass, or when a non-IOService kext turns on autounload and its reference
 * count drops to zero (external) references.
 *
 * Setting this property to true in a load request makes the kext being loaded
 * skip ONE autounload pass, giving about an extra minute of time before the
 * kext is subject to autounload. This is how kextutil(8) to delays autounload
 * so that there's more time to set up a debug session.
 *
 * Setting this property in any other request causes OSKext::considerUnloads()
 * to be called before processing the request, ensuring a window where kexts
 * will not be unloaded. The user-space kext library uses this so that it can
 * avoid including kexts that are already loaded in a load request.
 */
#define kKextRequestArgumentDelayAutounloadKey         "Delay Autounload"

#if PRAGMA_MARK
#pragma mark Load Request Arguments
#endif

/*********************************************************************
 * Kext Load Request Properties
 *
 * In addition to a bundle identifier, load requests can contain
 * these optional keys.
 *
 * These properties are used primarily by kextutil(8) to alter default
 * load behavior, but the OSKext user-level library makes them all
 * available in OSKextLoadWithOptions().
 **********/

/* Argument:  StartExclude
 * Type:      Integer, corresponding to OSKextExcludeLevel
 * Default:   kOSKextExcludeNone if not specified
 *
 * Normally all kexts in the load list for a load request are started.
 * This property is used primarily by kextutil(8) to delay start of
 * either the primary kext, or the whole load list (any that weren't
 * already loaded & started).
 */
#define kKextRequestArgumentStartExcludeKey        "Start Exclude Level"

/* Argument:  Start Matching Exclude Level
 * Type:      Integer, corresponding to OSKextExcludeLevel
 * Default:   kOSKextExcludeAll if not specified
 *
 * Normally no personalities are sent to the IOCatalogue for a regular
 * kext load; the assumption is that they are already there and triggered
 * the load request in the first place.
 *
 * This property is used primarily by kextutil(8) to delay matching for
 * either the primary kext, or the whole load list (any that didn't
 * already have personalities in the IOCatalogue).
 */
#define kKextRequestArgumentStartMatchingExcludeKey    "Start Matching Exclude Level"

// see also Delay Autounload

/* Argument:  Personality Names
 * Type:      Array of strings
 * Default:   All personalities are used
 *
 * Normally when personalities are sent to the IOCatalogue, they are all sent.
 * This property restricts the personalities sent, for the primary kext
 * being loaded, to those named. Personalities for dependencies are all sent,
 * and there is currently no mechanism to filter them.
 *
 * This property is used primarily by kextutil(8) to help debug matching
 * problems.
 */
#define kKextRequestArgumentPersonalityNamesKey        "Personality Names"

#if PRAGMA_MARK
#pragma mark Unload Request Arguments
#endif

/* Argument:  Terminate
 * Type:      Boolean
 * Default:   false
 *
 * An unload request may optionally specify via this key that all IOService
 * objects are to be terminated before attempting to unload. Kexts with
 * dependents will not attempt to terminate and will return kOSKextReturnInUse.
 */
#define kKextRequestArgumentTerminateIOServicesKey     "Terminate IOServices"

#if PRAGMA_MARK
#pragma mark Daemon Launch Request Arguments
#endif

/* Argument: Server tag
 * Type:     Integer
 * Default:  N/A
 *
 * A DriverKit daemon launch request must include a "server tag" that
 * is unique to every launch request. Userspace daemons include this
 * tag in their messages when attempting to rendez-vous with IOKit.
 */
#define kKextRequestArgumentDriverExtensionServerTag   "Driver Extension Server Tag"

/* Argument: Server name
 * Type:     String
 * Default:  N/A
 *
 * A DriverKit daemon launch request must include a "server name" that
 * can be used to identify what personality the driver is matching on.
 * This name is also used for the launchd service name of the daemon.
 */
#define kKextRequestArgumentDriverExtensionServerName  "Driver Extension Server Name"

#if PRAGMA_MARK
#pragma mark Internal Tracking Properties
#endif
/*********************************************************************
 * Internal Tracking Properties
 **********/

/* Argument:  Request Tag
 * Type:      Number (uint32_t)
 * Used by:   internal tracking for requests with callbacks
 *
 * Most requests to get resources (files) use this.
 */
#define kKextRequestArgumentRequestTagKey              "Request Tag"

/* Argument:  Request Callback
 * Type:      Data (pointer)
 * Used by:   internal tracking
 *
 * Most requests to get resources (files) use this.
 */
#define kKextRequestArgumentCallbackKey                "Request Callback"

/* Argument:  Request context.
 * Type:      OSData (wraps a void *)
 * Used by:   several
 */
#define kKextRequestArgumentContextKey                 "Context"

/* Argument:  Request Stale
 * Type:      Boolean
 * Used by:   internal tracking
 *
 * _OSKextConsiderUnloads sets this on any callback record lacking
 * it, and deletes any callback record that has it.
 */
#define kKextRequestStaleKey                           "Request Stale"

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* _LIBKERN_KEXT_REQUEST_KEYS_H */
