/*
 * Copyright (c) 2001-2014 Apple Inc. All rights reserved.
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
 * May 20, 2006			Joe Liu <joe.liu@apple.com>
 *				Allan Nathanson <ajn@apple.com>
 * - register interface by entryID (and not path)
 *
 * November 6, 2006		Allan Nathanson <ajn@apple.com>
 *				Dan Markarian <markarian@apple.com>
 *				Dieter Siegmund <dieter@apple.com>
 * - updated code to name interfaces quicker (without need for
 *   calling IOKitWaitQuiet).
 *
 * October 3, 2003		Allan Nathanson <ajn@apple.com>
 * - sort new interfaces by IOKit path (rather than MAC address) to
 *   help facilitate a more predictable interface-->name mapping for
 *   like hardware configurations.
 *
 * June 23, 2001		Allan Nathanson <ajn@apple.com>
 * - update to public SystemConfiguration.framework APIs
 *
 * January 23, 2001		Dieter Siegmund <dieter@apple.com>
 * - initial revision
 */

/*
 * ifnamer.c
 * - module that receives IOKit Network Interface messages
 *   and names any interface that currently does not have a name
 * - uses Interface Type and MACAddress as the unique identifying
 *   keys; any interface that doesn't contain both of these properties
 *   is ignored and not processed
 * - stores the Interface Type, MACAddress, and Unit in permanent storage
 *   to give persistent interface names
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <mach/mach.h>
#include <net/ethernet.h>
#include <net/if_types.h>
#include <pthread.h>
#include <vproc.h>

#include <CommonCrypto/CommonDigest.h>

#include <CoreFoundation/CoreFoundation.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCDPlugin.h>
#include <SystemConfiguration/SCPrivate.h>	// for SCLog(), SCPrint()
#include <SystemConfiguration/SCValidation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitLibPrivate.h>
#include <IOKit/IOBSD.h>
#include <IOKit/IOMessage.h>
#include <IOKit/network/IONetworkController.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IONetworkStack.h>
#include <IOKit/usb/USB.h>

#ifdef kIONetworkStackUserCommandKey
#define	USE_REGISTRY_ENTRY_ID
#endif

#ifndef	USE_REGISTRY_ENTRY_ID
// from <IOKit/network/IONetworkStack.h>
#define kIONetworkStackUserCommandKey	"IONetworkStackUserCommand"
enum {
    kRegisterInterfaceWithFixedUnit = 0,
    kRegisterInterface,
    kRegisterAllInterfaces
};
#endif	// !USE_REGISTRY_ENTRY_ID

#define	kSCNetworkInterfaceInfo		"SCNetworkInterfaceInfo"
#define	kSCNetworkInterfaceType		"SCNetworkInterfaceType"
#define	kSCNetworkInterfaceActive	"Active"

#define MY_PLUGIN_NAME			"InterfaceNamer"
#define	MY_PLUGIN_ID			CFSTR("com.apple.SystemConfiguration." MY_PLUGIN_NAME)

#define WAIT_STACK_TIMEOUT_KEY		"WaitStackTimeout"
#define WAIT_STACK_TIMEOUT_DEFAULT	300.0

#define WAIT_QUIET_TIMEOUT_KEY		"WaitQuietTimeout"
#define WAIT_QUIET_TIMEOUT_DEFAULT	60.0

/*
 * S_connect
 *   "IONetworkStack" connect object used to "name" an interface.
 */
static io_connect_t		S_connect		= MACH_PORT_NULL;

/*
 * S_dblist
 *   An array of CFDictionary's representing the interfaces
 *   that have been identified and [need to be] named.
 */
static CFMutableArrayRef	S_dblist		= NULL;

/*
 * S_debug
 *   A boolean that enables additional logging.
 */
static boolean_t		S_debug			= FALSE;

/*
 * S_iflist
 *   An array of SCNetworkInterface's representing the
 *   interfaces that have been identified.
 */
static CFMutableArrayRef	S_iflist		= NULL;

/*
 * S_iter
 *   IOServiceAddMatchingNotification object used to watch for
 *   new network interfaces.
 */
static io_iterator_t		S_iter			= MACH_PORT_NULL;

/*
 * S_notify
 *   notification object for receiving IOKit notifications of
 *   new devices or state changes.
 */
static IONotificationPortRef	S_notify		= NULL;

/* S_prev_active_list
 *   An array of CFDictionary's representing the previously
 *   named interfaces.
 */
static CFMutableArrayRef	S_prev_active_list	= NULL;

/*
 * S_quiet
 *   IOServiceAddInterestNotification object used to watch for
 *   IOKit matching to quiesce.
 */
static io_object_t		S_quiet			= MACH_PORT_NULL;

/*
 * S_stack
 *   IOServiceAddMatchingNotification object used to watch for
 *   the availability of the "IONetworkStack" object.
 */
static io_iterator_t		S_stack			= MACH_PORT_NULL;

/*
 * S_state
 *   A dictionary containing Information about each network
 *   interface.  For now, the key is the BSD name and the
 *   value is a CFNumber noting how long (in milliseconds)
 *   it took for the interface to be recognized/named.
 */
static CFMutableDictionaryRef	S_state			= NULL;

/*
 * S_timer
 *   CFRunLoopTimer tracking how long we are willing to wait
 *   for IOKit matching to quiesce (IOKitWaitQuiet).
 *
 * S_stack_timeout
 *   time to wait for the IONetworkStack object to appear before timeout
 *
 * S_quiet_timeout
 *   time to wait for the IOKit to quiesce (after the IONetworkStack is
 *   has appeared.
 */
static CFRunLoopTimerRef	S_timer			= NULL;
static double			S_stack_timeout		= WAIT_STACK_TIMEOUT_DEFAULT;
static double			S_quiet_timeout		= WAIT_QUIET_TIMEOUT_DEFAULT;

#if	!TARGET_OS_EMBEDDED
/*
 * S_vproc_transaction
 *   The vproc transaction used to keep launchd from sending us
 *   a SIGKILL before we've had a chance to set the platform UUID
 */
vproc_transaction_t		S_vproc_transaction	= NULL;
#endif	// !TARGET_OS_EMBEDDED

/*
 * Virtual network interface configuration
 *   S_prefs   : SCPreferences to configuration
 *   S_bonds   : most recently actived Bond configuration
 *   S_bridges : most recently actived Bridge configuration
 *   S_vlans   : most recently actived VLAN configuration
 */
static SCPreferencesRef		S_prefs			= NULL;
static CFArrayRef		S_bonds			= NULL;
static CFArrayRef		S_bridges		= NULL;
static CFArrayRef		S_vlans			= NULL;

static void
addTimestamp(CFMutableDictionaryRef dict, CFStringRef key)
{
    CFAbsoluteTime	now;
    CFNumberRef		val;

    now = CFAbsoluteTimeGetCurrent();
    val = CFNumberCreate(NULL, kCFNumberDoubleType, &now);
    CFDictionaryAddValue(dict, key, val);
    CFRelease(val);
    return;
}

#define	INTERFACES			CFSTR("Interfaces")
#define	NETWORK_INTERFACES_PREFS	CFSTR("NetworkInterfaces.plist")

static CFComparisonResult
if_unit_compare(const void *val1, const void *val2, void *context)
{
    CFComparisonResult	res;
    CFNumberRef		type1;
    CFNumberRef		type2;
    CFNumberRef		unit1;
    CFNumberRef		unit2;

    type1 = CFDictionaryGetValue((CFDictionaryRef)val1,
				 CFSTR(kIOInterfaceType));
    type2 = CFDictionaryGetValue((CFDictionaryRef)val2,
				 CFSTR(kIOInterfaceType));
    res = CFNumberCompare(type1, type2, NULL);
    if (res != kCFCompareEqualTo) {
	return (res);
    }
    unit1 = CFDictionaryGetValue((CFDictionaryRef)val1,
				 CFSTR(kIOInterfaceUnit));
    unit2 = CFDictionaryGetValue((CFDictionaryRef)val2,
				 CFSTR(kIOInterfaceUnit));
    return (CFNumberCompare(unit1, unit2, NULL));
}

static void
reportIssue(const char *signature, CFStringRef issue)
{
    asl_object_t  m;

    m = asl_new(ASL_TYPE_MSG);
    asl_set(m, "com.apple.message.domain", "com.apple.SystemConfiguration." MY_PLUGIN_NAME);
    asl_set(m, "com.apple.message.signature", signature);
    asl_set(m, "com.apple.message.result", "failure");
    SCLOG(NULL, m, ~ASL_LEVEL_ERR, CFSTR("%s\n%@"), signature, issue);
    asl_release(m);

    return;
}

static void
writeInterfaceList(CFArrayRef if_list)
{
    CFArrayRef		cur_list;
    CFStringRef		new_model;
    CFStringRef		old_model;
    SCPreferencesRef	prefs;

    if (isA_CFArray(if_list) == NULL) {
	return;
    }

    prefs = SCPreferencesCreate(NULL, MY_PLUGIN_ID, NETWORK_INTERFACES_PREFS);
    if (prefs == NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesCreate failed, %s"),
	      SCErrorString(SCError()));
	return;
    }

    cur_list = SCPreferencesGetValue(prefs, INTERFACES);
    if (_SC_CFEqual(cur_list, if_list)) {
	goto done;
    }

    old_model = SCPreferencesGetValue(prefs, MODEL);
    new_model = _SC_hw_model(FALSE);
    if ((new_model != NULL) && !_SC_CFEqual(old_model, new_model)) {
	// if new hardware
	if ((old_model != NULL) && (cur_list != NULL)) {
	    CFStringRef history;
	    CFStringRef issue;

	    // if interface list was created on other hardware
	    history = CFStringCreateWithFormat(NULL, NULL,
					       CFSTR("%@:%@"),
					       INTERFACES,
					       old_model);
	    SCPreferencesSetValue(prefs, history, cur_list);
	    CFRelease(history);

	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": Hardware model changed\n"
			MY_PLUGIN_NAME ":   created on \"%@\"\n"
			MY_PLUGIN_NAME ":   now on     \"%@\""),
		  old_model,
		  new_model);

	    issue = CFStringCreateWithFormat(NULL, NULL,
					     CFSTR("%@ --> %@"),
					     old_model,
					     new_model);
	    reportIssue("Hardware model changed", issue);
	    CFRelease(issue);
	}

	if (!SCPreferencesSetValue(prefs, MODEL, new_model)) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": SCPreferencesSetValue failed, %s"),
		  SCErrorString(SCError()));
	    goto done;
	}
    }

    if (!SCPreferencesSetValue(prefs, INTERFACES, if_list)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesSetValue failed, %s"),
	      SCErrorString(SCError()));
	goto done;
    }

    if (!SCPreferencesCommitChanges(prefs)) {
	SCLog((SCError() != EROFS), LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesCommitChanges failed, %s"),
	      SCErrorString(SCError()));
	goto done;
    }

done:

    CFRelease(prefs);
    return;
}

static CF_RETURNS_RETAINED CFMutableArrayRef
readInterfaceList()
{
    CFArrayRef		if_list;
    CFStringRef		old_model;
    CFMutableArrayRef 	plist	= NULL;
    SCPreferencesRef	prefs	= NULL;

    prefs = SCPreferencesCreate(NULL, MY_PLUGIN_ID, NETWORK_INTERFACES_PREFS);
    if (!prefs) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesCreate failed, %s"),
	      SCErrorString(SCError()));
	return (NULL);
    }

    if_list = SCPreferencesGetValue(prefs, INTERFACES);
    if_list = isA_CFArray(if_list);

    old_model = SCPreferencesGetValue(prefs, MODEL);
    if (old_model != NULL) {
	CFStringRef new_model;

	new_model = _SC_hw_model(FALSE);
	if (!_SC_CFEqual(old_model, new_model)) {
	    // if interface list was created on other hardware
	    if_list = NULL;
	}
    }

    if (if_list != NULL) {
	CFIndex	i;
	CFIndex	n	= CFArrayGetCount(if_list);

	plist = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	for (i = 0; i < n; i++) {
	    CFDictionaryRef	dict;

	    dict = CFArrayGetValueAtIndex(if_list, i);
	    if (isA_CFDictionary(dict) &&
		CFDictionaryContainsKey(dict, CFSTR(kIOInterfaceType)) &&
		CFDictionaryContainsKey(dict, CFSTR(kIOInterfaceUnit)) &&
		CFDictionaryContainsKey(dict, CFSTR(kIOMACAddress))) {
		    CFArrayAppendValue(plist, dict);
	    }
	}
    }

    if (prefs != NULL) {
	CFRelease(prefs);
    }
    return (plist);
}

static CF_RETURNS_RETAINED CFMutableArrayRef
previouslyActiveInterfaces()
{
    CFMutableArrayRef	active;
    CFIndex		i;
    CFIndex		n;

    if (S_dblist == NULL) {
	return NULL;
    }

    active = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

    n = CFArrayGetCount(S_dblist);
    for (i = 0; i < n; i++) {
	CFDictionaryRef	if_dict;

	if_dict = CFArrayGetValueAtIndex(S_dblist, i);
	if (CFDictionaryContainsKey(if_dict, CFSTR(kSCNetworkInterfaceActive))) {
	    CFMutableDictionaryRef	new_dict;

	    new_dict = CFDictionaryCreateMutableCopy(NULL, 0, if_dict);
	    CFDictionaryRemoveValue(new_dict, CFSTR(kSCNetworkInterfaceActive));
	    CFArraySetValueAtIndex(S_dblist, i, new_dict);
	    CFArrayAppendValue(active, new_dict);
	    CFRelease(new_dict);
	}
    }

    return active;
}

static void
updateStore(void)
{
    CFStringRef		key;

    key = SCDynamicStoreKeyCreate(NULL, CFSTR("%@" MY_PLUGIN_NAME),
				  kSCDynamicStoreDomainPlugin);
    (void)SCDynamicStoreSetValue(NULL, key, S_state);
    CFRelease(key);

    return;
}

#if	!TARGET_OS_IPHONE
static void
updateBondInterfaceConfiguration(SCPreferencesRef prefs)
{
    CFArrayRef	interfaces;

    interfaces = SCBondInterfaceCopyAll(prefs);
    if ((interfaces != NULL) && (CFArrayGetCount(interfaces) == 0)) {
	CFRelease(interfaces);
	interfaces = NULL;
    }

    if (_SC_CFEqual(S_bonds, interfaces)) {
	// if no change
	if (interfaces != NULL) CFRelease(interfaces);
	return;
    }

    if (S_bonds != NULL) CFRelease(S_bonds);
    S_bonds = interfaces;

    if (!_SCBondInterfaceUpdateConfiguration(prefs)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": _SCBondInterfaceUpdateConfiguration failed, %s"),
	      SCErrorString(SCError()));
    }

    return;
}
#endif	// !TARGET_OS_IPHONE

static void
updateBridgeInterfaceConfiguration(SCPreferencesRef prefs)
{
    CFArrayRef	interfaces;

    interfaces = SCBridgeInterfaceCopyAll(prefs);
    if ((interfaces != NULL) && (CFArrayGetCount(interfaces) == 0)) {
	CFRelease(interfaces);
	interfaces = NULL;
    }

    if (_SC_CFEqual(S_bridges, interfaces)) {
	// if no change
	if (interfaces != NULL) CFRelease(interfaces);
	return;
    }

    if (S_bridges != NULL) CFRelease(S_bridges);
    S_bridges = interfaces;

    if (!_SCBridgeInterfaceUpdateConfiguration(prefs)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": _SCBridgeInterfaceUpdateConfiguration failed, %s"),
	      SCErrorString(SCError()));
    }

    return;
}

static void
updateVLANInterfaceConfiguration(SCPreferencesRef prefs)
{
    CFArrayRef	interfaces;

    interfaces = SCVLANInterfaceCopyAll(prefs);
    if ((interfaces != NULL) && (CFArrayGetCount(interfaces) == 0)) {
	CFRelease(interfaces);
	interfaces = NULL;
    }

    if (_SC_CFEqual(S_vlans, interfaces)) {
	// if no change
	if (interfaces != NULL) CFRelease(interfaces);
	return;
    }

    if (S_vlans != NULL) CFRelease(S_vlans);
    S_vlans = interfaces;

    if (!_SCVLANInterfaceUpdateConfiguration(prefs)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": _SCVLANInterfaceUpdateConfiguration failed, %s"),
	      SCErrorString(SCError()));
    }

    return;
}

static void
updateVirtualNetworkInterfaceConfiguration(SCPreferencesRef		prefs,
					   SCPreferencesNotification   notificationType,
					   void				*info)
{
    if ((notificationType & kSCPreferencesNotificationApply) != kSCPreferencesNotificationApply) {
	return;
    }

    if (prefs == NULL) {
	// if a new interface has been "named"
	prefs = S_prefs;
	if (S_bonds != NULL) {
	    CFRelease(S_bonds);
	    S_bonds = NULL;
	}
	if (S_bridges != NULL) {
	    CFRelease(S_bridges);
	    S_bridges = NULL;
	}
	if (S_vlans != NULL) {
	    CFRelease(S_vlans);
	    S_vlans = NULL;
	}
    }

#if	!TARGET_OS_IPHONE
    updateBondInterfaceConfiguration  (prefs);
#endif	// !TARGET_OS_IPHONE
    updateBridgeInterfaceConfiguration(prefs);
    updateVLANInterfaceConfiguration  (prefs);

    // we are finished with current prefs, wait for changes
    SCPreferencesSynchronize(prefs);
    return;
}

#if	!TARGET_OS_EMBEDDED

#define	BT_PAN_NAME	"Bluetooth PAN"
#define	BT_PAN_MAC	BT_PAN_NAME " (MAC)"

static void
updateBTPANInformation(const void *value, void *context)
{
    CFDataRef		addr;
    CFDictionaryRef	dict    = (CFDictionaryRef)value;
    CFStringRef		if_name;
    CFDictionaryRef	info;
    CFStringRef		name;

    if_name = CFDictionaryGetValue(dict, CFSTR(kIOBSDNameKey));
    if (!isA_CFString(if_name)) {
	// if no BSD name
	return;
    }

    info = CFDictionaryGetValue(dict, CFSTR(kSCNetworkInterfaceInfo));
    if (!isA_CFDictionary(info)) {
	// if no SCNetworkInterface info
	return;
    }

    name = CFDictionaryGetValue(info, kSCPropUserDefinedName);
    if (!isA_CFString(name) || !CFEqual(name, CFSTR(BT_PAN_NAME))) {
	// if not BT-PAN interface
	return;
    }

    CFDictionaryAddValue(S_state, CFSTR("_" BT_PAN_NAME "_"), if_name);

    addr = CFDictionaryGetValue(dict, CFSTR(kIOMACAddress));
    if (isA_CFData(addr)) {
	CFDictionaryAddValue(S_state, CFSTR("_" BT_PAN_MAC "_"), addr);
    }

    return;
}
#endif	// !TARGET_OS_EMBEDDED

static CFDictionaryRef
createInterfaceDict(SCNetworkInterfaceRef interface)
{
    CFMutableDictionaryRef	new_if;
    CFTypeRef			val;

    new_if = CFDictionaryCreateMutable(NULL,
				       0,
				       &kCFTypeDictionaryKeyCallBacks,
				       &kCFTypeDictionaryValueCallBacks);

    val = _SCNetworkInterfaceCopyInterfaceInfo(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kSCNetworkInterfaceInfo), val);
	CFRelease(val);
    }

    val = _SCNetworkInterfaceGetIOPath(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOPathMatchKey), val);
    }

    val = _SCNetworkInterfaceGetIOInterfaceNamePrefix(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOInterfaceNamePrefix), val);
    }

    val = _SCNetworkInterfaceGetIOInterfaceType(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOInterfaceType), val);
    }

    val = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOInterfaceUnit), val);
    }

    val = _SCNetworkInterfaceGetHardwareAddress(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOMACAddress), val);
    }

    val = SCNetworkInterfaceGetBSDName(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kIOBSDNameKey), val);
    }

    val = SCNetworkInterfaceGetInterfaceType(interface);
    if (val != NULL) {
	CFDictionarySetValue(new_if, CFSTR(kSCNetworkInterfaceType), val);
    }

    CFDictionarySetValue(new_if,
			 CFSTR(kIOBuiltin),
			 _SCNetworkInterfaceIsBuiltin(interface) ? kCFBooleanTrue : kCFBooleanFalse);

    CFDictionarySetValue(new_if, CFSTR(kSCNetworkInterfaceActive), kCFBooleanTrue);

    return new_if;
}

static CFDictionaryRef
lookupInterfaceByAddress(CFArrayRef db_list, SCNetworkInterfaceRef interface, CFIndex * where)
{
    CFDataRef	addr;
    CFIndex	i;
    CFIndex	n;
    CFNumberRef	type;

    if (db_list == NULL) {
	return (NULL);
    }
    type = _SCNetworkInterfaceGetIOInterfaceType(interface);
    addr = _SCNetworkInterfaceGetHardwareAddress(interface);
    if (type == NULL || addr == NULL) {
	return (NULL);
    }

    n = CFArrayGetCount(db_list);
    for (i = 0; i < n; i++) {
	CFDataRef	a;
	CFDictionaryRef	dict = CFArrayGetValueAtIndex(db_list, i);
	CFNumberRef	t;

	t = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceType));
	a = CFDictionaryGetValue(dict, CFSTR(kIOMACAddress));
	if (t == NULL || a == NULL)
	    continue;

	if (CFEqual(type, t) && CFEqual(addr, a)) {
	    if (where) {
		*where = i;
	    }
	    return (dict);
	}
    }
    return (NULL);
}

static CFDictionaryRef
lookupInterfaceByUnit(CFArrayRef db_list, SCNetworkInterfaceRef interface, CFIndex * where)
{
    CFIndex 	i;
    CFIndex	n;
    CFNumberRef	type;
    CFNumberRef	unit;

    if (db_list == NULL) {
	return (NULL);
    }
    type = _SCNetworkInterfaceGetIOInterfaceType(interface);
    unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
    if (type == NULL || unit == NULL) {
	return (NULL);
    }

    n = CFArrayGetCount(db_list);
    for (i = 0; i < n; i++) {
	CFDictionaryRef	dict = CFArrayGetValueAtIndex(db_list, i);
	CFNumberRef	t;
	CFNumberRef	u;

	t = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceType));
	u = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceUnit));
	if (t == NULL || u == NULL) {
	    continue;
	}

	if (CFEqual(type, t) && CFEqual(unit, u)) {
	    if (where)
		*where = i;
	    return (dict);
	}
    }
    return (NULL);
}

typedef struct {
    CFDictionaryRef	    match_info;
    CFStringRef		    match_type;
    CFBooleanRef	    match_builtin;
    CFMutableArrayRef	    matches;
} matchContext, *matchContextRef;

static CF_RETURNS_RETAINED CFDictionaryRef
thinInterfaceInfo(CFDictionaryRef info)
{
    CFNumberRef	num;
    int		vid;

    if (CFDictionaryGetValueIfPresent(info, CFSTR(kUSBVendorID), (const void **)&num)
	&& isA_CFNumber(num)
	&& CFNumberGetValue(num, kCFNumberIntType, &vid)
	&& (vid == kIOUSBVendorIDAppleComputer)) {
	CFMutableDictionaryRef  thin;

	// if this is an Apple USB device than we trust that
	// the non-localized name will be correct.
	thin = CFDictionaryCreateMutableCopy(NULL, 0, info);
	CFDictionaryRemoveValue(thin, CFSTR(kUSBProductString));
	CFDictionaryRemoveValue(thin, CFSTR(kUSBVendorID));
	CFDictionaryRemoveValue(thin, CFSTR(kUSBProductID));
	return thin;
    }

    return CFRetain(info);
}

static Boolean
matchInterfaceInfo(CFDictionaryRef known_info, CFDictionaryRef match_info)
{
    Boolean match;

    match = _SC_CFEqual(known_info, match_info);
    if (!match &&
	isA_CFDictionary(known_info) &&
	isA_CFDictionary(match_info)) {

	// if not an exact match, try thinning
	known_info = thinInterfaceInfo(known_info);
	match_info = thinInterfaceInfo(match_info);
	match = _SC_CFEqual(known_info, match_info);
	if (known_info != NULL) CFRelease(known_info);
	if (match_info != NULL) CFRelease(match_info);
    }

    return match;
}

static void
matchKnown(const void *value, void *context)
{
    CFDictionaryRef	known_dict	= (CFDictionaryRef)value;
    matchContextRef	match_context	= (matchContextRef)context;

    // match interface type
    {
	CFStringRef	known_type;

	known_type = CFDictionaryGetValue(known_dict, CFSTR(kSCNetworkInterfaceType));
	if (!_SC_CFEqual(known_type, match_context->match_type)) {
	    return;
	}
    }

    // match SCNetworkInterfaceInfo
    {
	CFDictionaryRef	known_info;

	known_info = CFDictionaryGetValue(known_dict, CFSTR(kSCNetworkInterfaceInfo));
	if (!matchInterfaceInfo(known_info, match_context->match_info)) {
	    return;
	}
    }

    // if requested, match [non-]builtin
    if (match_context->match_builtin != NULL) {
	CFBooleanRef	known_builtin;

	known_builtin = CFDictionaryGetValue(known_dict, CFSTR(kIOBuiltin));
	if (!isA_CFBoolean(known_builtin)) {
	    known_builtin = kCFBooleanFalse;
	}
	if (!_SC_CFEqual(known_builtin, match_context->match_builtin)) {
	    return;
	}
    }

    // if we have a match
    if (match_context->matches == NULL) {
	match_context->matches = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    }
    CFArrayAppendValue(match_context->matches, known_dict);

    return;
}

static void
matchUnnamed(const void *value, void *context)
{
    SCNetworkInterfaceRef   known_if	    = (SCNetworkInterfaceRef)value;
    matchContextRef	    match_context   = (matchContextRef)context;

    if (match_context->matches == NULL) {
	return;
    }

    // match interface type
    {
	CFStringRef	known_type;

	known_type = SCNetworkInterfaceGetInterfaceType(known_if);
	if (!_SC_CFEqual(known_type, match_context->match_type)) {
	    return;
	}
    }

    // match SCNetworkInterfaceInfo
    {
	CFDictionaryRef	known_info;
	Boolean		match;

	known_info = _SCNetworkInterfaceCopyInterfaceInfo(known_if);
	match = matchInterfaceInfo(known_info, match_context->match_info);
	if (known_info != NULL) CFRelease(known_info);
	if (!match) {
	    return;
	}
    }

    // if requested, match [non-]builtin
    if (match_context->match_builtin != NULL) {
	CFBooleanRef	known_builtin;

	known_builtin = _SCNetworkInterfaceIsBuiltin(known_if) ? kCFBooleanTrue
							       : kCFBooleanFalse;
	if (!_SC_CFEqual(known_builtin, match_context->match_builtin)) {
	    return;
	}
    }

    // if we have a match
    CFRelease(match_context->matches);
    match_context->matches = NULL;

    return;
}

static Boolean
interfaceExists(CFStringRef prefix, CFNumberRef unit)
{
    Boolean		found	    = FALSE;
    CFDictionaryRef	match_dict;
    CFStringRef		match_keys[2];
    CFTypeRef		match_vals[2];
    CFDictionaryRef	matching;



    io_registry_entry_t	entry		= MACH_PORT_NULL;
    io_iterator_t		iterator	= MACH_PORT_NULL;
    kern_return_t		kr;
    mach_port_t			masterPort	= MACH_PORT_NULL;

    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOMasterPort returned 0x%x"),
	      kr);
	goto error;
    }

    // look for kIONetworkInterface with matching prefix and unit
    match_keys[0] = CFSTR(kIOInterfaceNamePrefix);
    match_vals[0] = prefix;
    match_keys[1] = CFSTR(kIOInterfaceUnit);
    match_vals[1] = unit;
    match_dict = CFDictionaryCreate(NULL,
				    (const void **)match_keys,
				    (const void **)match_vals,
				    2,
				    &kCFTypeDictionaryKeyCallBacks,
				    &kCFTypeDictionaryValueCallBacks);

    match_keys[0] = CFSTR(kIOProviderClassKey);
    match_vals[0] = CFSTR(kIONetworkInterfaceClass);
    match_keys[1] = CFSTR(kIOPropertyMatchKey);
    match_vals[1] = match_dict;
    matching = CFDictionaryCreate(NULL,
				  (const void **)match_keys,
				  (const void **)match_vals,
				  sizeof(match_keys)/sizeof(match_keys[0]),
				  &kCFTypeDictionaryKeyCallBacks,
				  &kCFTypeDictionaryValueCallBacks);
    CFRelease(match_dict);

    // note: the "matching" dictionary will be consumed by the following
    kr = IOServiceGetMatchingServices(masterPort, matching, &iterator);
    if ((kr != kIOReturnSuccess) || (iterator == MACH_PORT_NULL)) {
	// if no interface
	goto error;
    }

    entry = IOIteratorNext(iterator);
    if (entry == MACH_PORT_NULL) {
	// if no interface
	goto error;
    }

    found = TRUE;

error:
    if (masterPort != MACH_PORT_NULL) {
	mach_port_deallocate(mach_task_self(), masterPort);
    }
    if (entry != MACH_PORT_NULL) {
	IOObjectRelease(entry);
    }
    if (iterator != MACH_PORT_NULL) {
	IOObjectRelease(iterator);
    }

    return (found);
}

/*
 * lookupMatchingInterface
 *
 * Looks at the interfaces that have already been [or need to be] named with
 * the goal of allowing a system using a single network interface/adaptor of
 * a given type (vendor, model, ...) to not care about the specific adaptor
 * that is used (i.e. swapping dongle's is OK).  Once a system has had more
 * than one interface/adaptor connected at the same time than we assume that
 * the network configuration is being setup for multi-homing that should be
 * maintained.
 *
 * If no matches are found or if more than one match is found, return NULL.
 * If a single match is found, return the match.
 */
static CFDictionaryRef
lookupMatchingInterface(SCNetworkInterfaceRef	interface,
			CFArrayRef		db_list,	// already named
			CFArrayRef		if_list,	// to be named
			CFIndex			if_list_index,
			CFBooleanRef		builtin)
{
    CFStringRef	    if_type;
    CFDictionaryRef match	    = NULL;
    matchContext    match_context;

    if_type = SCNetworkInterfaceGetInterfaceType(interface);
    if (if_type == NULL) {
	return NULL;
    }

    match_context.match_type	= if_type;
    match_context.match_info	= _SCNetworkInterfaceCopyInterfaceInfo(interface);
    match_context.match_builtin	= builtin;
    match_context.matches	= NULL;

    // check for matches to interfaces that have already been named
    // ... and append each match that we find to match_context.matches
    if (db_list != NULL) {
	CFArrayApplyFunction(db_list,
			     CFRangeMake(0, CFArrayGetCount(db_list)),
			     matchKnown,
			     &match_context);
    }

    // check for matches to interfaces that will be named
    // ... and CFRelease match_context.matches if we find another network
    //     interface of the same type that also needs to be named
    if (if_list != NULL) {
	CFIndex	   if_list_count;

	if_list_count = CFArrayGetCount(if_list);
	if (if_list_index < if_list_count) {
	    CFArrayApplyFunction(if_list,
				 CFRangeMake(if_list_index, if_list_count - if_list_index),
				 matchUnnamed,
				 &match_context);
	}
    }

    // check if we have a single match
    if (match_context.matches != NULL) {
	if (CFArrayGetCount(match_context.matches) == 1) {
	    match = CFArrayGetValueAtIndex(match_context.matches, 0);
	}
	CFRelease(match_context.matches);
    }

    if (match != NULL) {
	Boolean		active	= TRUE;
	CFStringRef	name;

	name = CFDictionaryGetValue(match, CFSTR(kIOBSDNameKey));
	if (isA_CFString(name)) {
	    CFStringRef	    prefix;
	    CFNumberRef	    unit;

	    prefix = CFDictionaryGetValue(match, CFSTR(kIOInterfaceNamePrefix));
	    unit   = CFDictionaryGetValue(match, CFSTR(kIOInterfaceUnit));
	    if (isA_CFString(prefix) && isA_CFNumber(unit)) {
		if (!interfaceExists(prefix, unit)) {
		    active = FALSE;
		}
	    }
	}

	if (active) {
	    match = NULL;
	}
    }

    if (match_context.match_info != NULL) CFRelease(match_context.match_info);
    return match;
}

static void
insertInterface(CFMutableArrayRef db_list, SCNetworkInterfaceRef interface)
{
    CFIndex		i;
    CFDictionaryRef	if_dict;
    CFStringRef		if_name;
    CFNumberRef		if_type;
    CFNumberRef		if_unit;
    CFIndex		n	= CFArrayGetCount(db_list);
    CFComparisonResult	res;

    if_name = SCNetworkInterfaceGetBSDName(interface);
    if (if_name != NULL) {
	addTimestamp(S_state, if_name);
    }

    if_dict = createInterfaceDict(interface);
    if_type = _SCNetworkInterfaceGetIOInterfaceType(interface);
    if_unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
    if ((if_type == NULL) || (if_unit == NULL)) {
	CFRelease(if_dict);
	return;
    }

    for (i = 0; i < n; i++) {
	CFNumberRef	db_type;
	CFNumberRef	db_unit;
	CFDictionaryRef	dict	= CFArrayGetValueAtIndex(db_list, i);

	db_type = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceType));
	db_unit = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceUnit));
	res = CFNumberCompare(if_type, db_type, NULL);
	if (res == kCFCompareLessThan
	    || (res == kCFCompareEqualTo
		&& (CFNumberCompare(if_unit, db_unit, NULL)
		    == kCFCompareLessThan))) {
	    CFArrayInsertValueAtIndex(db_list, i, if_dict);
	    CFRelease(if_dict);
	    return;
	}
    }

    CFArrayAppendValue(S_dblist, if_dict);

#if	!TARGET_OS_EMBEDDED
    updateBTPANInformation(if_dict, NULL);
#endif	// !TARGET_OS_EMBEDDED

    CFRelease(if_dict);
    return;
}

static void
replaceInterface(SCNetworkInterfaceRef interface)
{
    int		n	= 0;
    CFIndex	where;

    if (S_dblist == NULL) {
	S_dblist = CFArrayCreateMutable(NULL, 0,
					&kCFTypeArrayCallBacks);
    }
    // remove any dict that has our type/addr
    while (lookupInterfaceByAddress(S_dblist, interface, &where) != NULL) {
	CFArrayRemoveValueAtIndex(S_dblist, where);
	n++;
    }
    // remove any dict that has the same type/unit
    while (lookupInterfaceByUnit(S_dblist, interface, &where) != NULL) {
	CFArrayRemoveValueAtIndex(S_dblist, where);
	n++;
    }
    insertInterface(S_dblist, interface);

    if (n > 1) {
	CFStringRef	issue;

	issue = CFStringCreateWithFormat(NULL, NULL,
					 CFSTR("n = %d, %@"),
					 n,
					 interface);
	reportIssue("Multiple interfaces updated", issue);
	CFRelease(issue);
    }

    return;
}

static CFNumberRef
getHighestUnitForType(CFNumberRef if_type)
{
    int 		i;
    CFIndex		n;
    CFNumberRef		ret_unit	= NULL;

    if (S_dblist == NULL) {
	return (NULL);
    }

    n = CFArrayGetCount(S_dblist);
    for (i = 0; i < n; i++) {
	CFDictionaryRef	dict = CFArrayGetValueAtIndex(S_dblist, i);
	CFNumberRef	type;

	type = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceType));
	if (CFEqual(type, if_type)) {
	    CFNumberRef	unit;

	    unit = CFDictionaryGetValue(dict, CFSTR(kIOInterfaceUnit));
	    if (ret_unit == NULL
		|| (CFNumberCompare(unit, ret_unit, NULL)
		    == kCFCompareGreaterThan)) {
		ret_unit = unit;
	    }
	}
    }

    return (ret_unit);
}

/*
 * Function: ensureInterfaceHasUnit
 * Purpose:
 *   Ensure that the SCNetworkInterfaceRef has a unit number.  If it doesn't,
 *   release the interface and return NULL.
 */
static SCNetworkInterfaceRef
ensureInterfaceHasUnit(SCNetworkInterfaceRef net_if)
{
    if (net_if != NULL
	&& _SCNetworkInterfaceGetIOInterfaceUnit(net_if) == NULL) {
	CFRelease(net_if);
	net_if = NULL;
    }
    return (net_if);
}

#ifdef	USE_REGISTRY_ENTRY_ID
static kern_return_t
registerInterfaceWithIORegistryEntryID(io_connect_t connect,
				       uint64_t	    entryID,
				       CFNumberRef  unit,
				       const int    command)
{
    CFDataRef			data;
    CFMutableDictionaryRef	dict;
    kern_return_t		kr;
    CFNumberRef			num;

    dict = CFDictionaryCreateMutable(NULL, 0,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
    num = CFNumberCreate(NULL, kCFNumberIntType, &command);
    CFDictionarySetValue(dict, CFSTR(kIONetworkStackUserCommandKey), num);
    CFRelease(num);
    data = CFDataCreate(NULL, (void *) &entryID, sizeof(entryID));
    CFDictionarySetValue(dict, CFSTR(kIORegistryEntryIDKey), data);
    CFRelease(data);
    CFDictionarySetValue(dict, CFSTR(kIOInterfaceUnit), unit);
    kr = IOConnectSetCFProperties(connect, dict);
    CFRelease(dict);
    return kr;
}

static SCNetworkInterfaceRef
copyInterfaceForIORegistryEntryID(uint64_t entryID)
{
    io_registry_entry_t		entry		= MACH_PORT_NULL;
    SCNetworkInterfaceRef	interface	= NULL;
    io_iterator_t		iterator	= MACH_PORT_NULL;
    kern_return_t		kr;
    mach_port_t			masterPort	= MACH_PORT_NULL;

    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOMasterPort returned 0x%x"),
	      kr);
	goto error;
    }

    kr = IOServiceGetMatchingServices(masterPort,
				      IORegistryEntryIDMatching(entryID),
				      &iterator);
    if ((kr != KERN_SUCCESS) || (iterator == MACH_PORT_NULL)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceGetMatchingServices(0x%llx) returned 0x%x/%d"),
	      entryID,
	      kr,
	      iterator);
	goto error;
    }

    entry = IOIteratorNext(iterator);
    if (entry == MACH_PORT_NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IORegistryEntryIDMatching(0x%llx) failed"),
	      entryID);
	goto error;
    }

    interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(entry);

 error:
    if (masterPort != MACH_PORT_NULL) {
	mach_port_deallocate(mach_task_self(), masterPort);
    }
    if (entry != MACH_PORT_NULL) {
	IOObjectRelease(entry);
    }
    if (iterator != MACH_PORT_NULL) {
	IOObjectRelease(iterator);
    }
    return (interface);
}

static SCNetworkInterfaceRef
copyNamedInterfaceForIORegistryEntryID(uint64_t entryID)
{
    SCNetworkInterfaceRef	net_if;

    net_if = copyInterfaceForIORegistryEntryID(entryID);
    return (ensureInterfaceHasUnit(net_if));
}

#else	// USE_REGISTRY_ENTRY_ID
/*
 * Function: registerInterface
 * Purpose:
 *   Register a single interface with the given service path to the
 *   data link layer (BSD), using the specified unit number.
 */
static kern_return_t
registerInterfaceWithIOServicePath(io_connect_t	connect,
				   CFStringRef	path,
				   CFNumberRef	unit,
				   const int	command)
{
    CFMutableDictionaryRef	dict;
    kern_return_t		kr;
    CFNumberRef			num;

    dict = CFDictionaryCreateMutable(NULL, 0,
				     &kCFTypeDictionaryKeyCallBacks,
				     &kCFTypeDictionaryValueCallBacks);
    num = CFNumberCreate(NULL, kCFNumberIntType, &command);
    CFDictionarySetValue(dict, CFSTR(kIONetworkStackUserCommandKey), num);
    CFRelease(num);
    CFDictionarySetValue(dict, CFSTR(kIOPathMatchKey), path);
    CFDictionarySetValue(dict, CFSTR(kIOInterfaceUnit), unit);
    kr = IOConnectSetCFProperties(connect, dict);
    CFRelease(dict);
    return kr;
}

static SCNetworkInterfaceRef
copyInterfaceForIOKitPath(CFStringRef if_path)
{
    io_registry_entry_t		entry		= MACH_PORT_NULL;
    SCNetworkInterfaceRef	interface	= NULL;
    kern_return_t		kr;
    mach_port_t			masterPort	= MACH_PORT_NULL;
    io_string_t			path;

    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOMasterPort returned 0x%x"),
	      kr);
	goto error;
    }
    _SC_cfstring_to_cstring(if_path, path, sizeof(path), kCFStringEncodingASCII);
    entry = IORegistryEntryFromPath(masterPort, path);
    if (entry == MACH_PORT_NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IORegistryEntryFromPath(%@) failed"),
	      if_path);
	goto error;
    }

    interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(entry);

 error:
    if (masterPort != MACH_PORT_NULL) {
	mach_port_deallocate(mach_task_self(), masterPort);
    }
    if (entry != MACH_PORT_NULL) {
	IOObjectRelease(entry);
    }
    return (interface);

}

static SCNetworkInterfaceRef
copyNamedInterfaceForIOKitPath(CFStringRef if_path)
{
    SCNetworkInterfaceRef	net_if;

    net_if = copyInterfaceForIOKitPath(if_path);
    return (ensureInterfaceHasUnit(net_if));
}

#endif	// USE_REGISTRY_ENTRY_ID

static void
displayInterface(SCNetworkInterfaceRef interface)
{
    CFStringRef		addr;
    CFStringRef		name;
    CFNumberRef		type;
    CFNumberRef		unit;

    name = SCNetworkInterfaceGetBSDName(interface);
    unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
    type = _SCNetworkInterfaceGetIOInterfaceType(interface);
    addr = SCNetworkInterfaceGetHardwareAddressString(interface);

    SCLog(TRUE, LOG_INFO,
	  CFSTR(MY_PLUGIN_NAME ":   %s%@%sType: %@, %s%@%sMAC address: %@"),
	  (name != NULL) ? "BSD Name: " : "",
	  (name != NULL) ? name : CFSTR(""),
	  (name != NULL) ? ", " : "",
	  type,
	  (unit != NULL) ? "Unit: " : "",
	  (unit != NULL) ? (CFTypeRef)unit : (CFTypeRef)CFSTR(""),
	  (unit != NULL) ? ", " : "",
	  addr);
}

static Boolean
builtinAvailable(SCNetworkInterfaceRef	interface,	// new interface
		 CFNumberRef		if_unit)	// desired unit
{
    CFIndex	i;
    CFNumberRef if_type	= _SCNetworkInterfaceGetIOInterfaceType(interface);
    CFIndex	n;

    n = (S_dblist != NULL) ? CFArrayGetCount(S_dblist) : 0;
    for (i = 0; i < n; i++) {
	CFStringRef	    if_path;
	CFDictionaryRef	    known_dict	    = CFArrayGetValueAtIndex(S_dblist, i);
	CFStringRef	    known_path;
	CFNumberRef	    known_type;
	CFNumberRef	    known_unit;

	known_type = CFDictionaryGetValue(known_dict, CFSTR(kIOInterfaceType));
	if (!_SC_CFEqual(if_type, known_type)) {
	    continue;	// if not the same interface type
	}

	known_unit = CFDictionaryGetValue(known_dict, CFSTR(kIOInterfaceUnit));
	if (!_SC_CFEqual(if_unit, known_unit)) {
	    continue;	// if not the same interface unit
	}

	if_path    = _SCNetworkInterfaceGetIOPath(interface);
	known_path = CFDictionaryGetValue(known_dict, CFSTR(kIOPathMatchKey));
	if (!_SC_CFEqual(if_path, known_path)) {
	    // if different IORegistry path
	    return FALSE;
	}

	// if same type, same unit, same path
	return TRUE;
    }

    // if interface type/unit not found
    return TRUE;
}

static int
builtinCount(CFArrayRef if_list, CFIndex last, CFNumberRef if_type)
{
    CFIndex	i;
    int		n	= 0;

    for (i = 0; i < last; i++) {
	SCNetworkInterfaceRef	builtin_if;
	CFNumberRef		builtin_type;

	builtin_if   = CFArrayGetValueAtIndex(if_list, i);
	builtin_type = _SCNetworkInterfaceGetIOInterfaceType(builtin_if);
	if (CFEqual(if_type, builtin_type)) {
	    if (_SCNetworkInterfaceIsBuiltin(builtin_if)) {
		n++;	// if built-in interface
	    }
	}
    }

    return n;
}

static __inline__ boolean_t
isQuiet(void)
{
    return (S_quiet == MACH_PORT_NULL);
}

static void
nameInterfaces(CFMutableArrayRef if_list)
{
    CFIndex	i;
    CFIndex	n	= CFArrayGetCount(if_list);

    for (i = 0; i < n; i++) {
	uint64_t		entryID;
	SCNetworkInterfaceRef	interface;
	SCNetworkInterfaceRef	new_interface;
	CFStringRef		path;
	CFStringRef		str;
	CFNumberRef		type;
	CFNumberRef		unit;
	CFIndex			where;

	interface = CFArrayGetValueAtIndex(if_list, i);
	path = _SCNetworkInterfaceGetIOPath(interface);
	type = _SCNetworkInterfaceGetIOInterfaceType(interface);
	unit = _SCNetworkInterfaceGetIOInterfaceUnit(interface);
	entryID = _SCNetworkInterfaceGetIORegistryEntryID(interface);

	if (unit != NULL) {
	    if (S_debug) {
		CFStringRef	if_name;

		if_name = SCNetworkInterfaceGetBSDName(interface);
		if ((if_name == NULL) || !CFDictionaryContainsKey(S_state, if_name)) {
			SCLog(TRUE, LOG_INFO,
			      CFSTR(MY_PLUGIN_NAME ": Interface already has a unit number"));
			displayInterface(interface);
		}
	    }

	    // update the list of interfaces that were previously named
	    if ((S_prev_active_list != NULL)
		&& lookupInterfaceByAddress(S_prev_active_list, interface, &where) != NULL) {
		CFArrayRemoveValueAtIndex(S_prev_active_list, where);
	    }

	    replaceInterface(interface);
	} else {
	    CFDictionaryRef 	dbdict;
	    boolean_t		is_builtin;
	    kern_return_t	kr;
	    int			retries	= 0;

	    dbdict = lookupInterfaceByAddress(S_dblist, interface, NULL);
	    if (dbdict != NULL) {
		unit = CFDictionaryGetValue(dbdict, CFSTR(kIOInterfaceUnit));
		CFRetain(unit);

		SCLog(S_debug, LOG_INFO,
		      CFSTR(MY_PLUGIN_NAME ": Interface assigned unit %@ (from database)"),
		      unit);
	    }

	    if ((dbdict == NULL) && !isQuiet()) {
		// if new interface, wait until quiet before naming
		addTimestamp(S_state, path);
		continue;
	    }

	    is_builtin = _SCNetworkInterfaceIsBuiltin(interface);

	    if (dbdict == NULL) {
		dbdict = lookupMatchingInterface(interface,
						 S_dblist,
						 if_list,
						 i + 1,
						 is_builtin ? kCFBooleanTrue : kCFBooleanFalse);
		if (dbdict != NULL) {
		    unit = CFDictionaryGetValue(dbdict, CFSTR(kIOInterfaceUnit));
		    CFRetain(unit);

		    SCLog(S_debug, LOG_INFO,
			  CFSTR(MY_PLUGIN_NAME ": Interface assigned unit %@ (updating database)"),
			  unit);
		}
	    }

	    if ((dbdict != NULL) && (S_prev_active_list != NULL)) {
		// update the list of interfaces that were previously named
		where = CFArrayGetFirstIndexOfValue(S_prev_active_list,
						    CFRangeMake(0, CFArrayGetCount(S_prev_active_list)),
						    dbdict);
		if (where != kCFNotFound) {
		    CFArrayRemoveValueAtIndex(S_prev_active_list, where);
		}
	    }

	    if (dbdict == NULL) {
		int 		next_unit	= 0;

		if (is_builtin) {
		    // built-in interface, try to use the reserved slots
		    next_unit = builtinCount(if_list, i, type);

		    // But, before claiming a reserved slot we check to see if the
		    // slot had previously been used.  If so, and if the slot had been
		    // assigned to the same type of interface, then we will perform a
		    // replacement (e.g. assume that this was a board swap).  But, if
		    // the new interface is a different type then we assume that the
		    // built-in configuration has changed and allocate a new unit from
		    // the non-reserved slots.

		    unit = CFNumberCreate(NULL, kCFNumberIntType, &next_unit);
		    if (!builtinAvailable(interface, unit)) {
			// if [built-in] unit not available
			SCLog(S_debug, LOG_INFO,
			      CFSTR(MY_PLUGIN_NAME ": Interface not assigned [built-in] unit %@"),
			      unit);
			CFRelease(unit);
			unit = NULL;
		    }
		}

		if (unit == NULL) {
		    // not built-in (or built-in unit not available), allocate from
		    // the non-reserved slots
		    next_unit = builtinCount(if_list, n, type);

		    unit = getHighestUnitForType(type);
		    if (unit != NULL) {
			int	high_unit;

			CFNumberGetValue(unit, kCFNumberIntType, &high_unit);
			if (high_unit >= next_unit) {
			    next_unit = high_unit + 1;
			}
		    }

		    unit = CFNumberCreate(NULL, kCFNumberIntType, &next_unit);
		}

		SCLog(S_debug, LOG_INFO,
		      CFSTR(MY_PLUGIN_NAME ": Interface assigned unit %@ (%s)"),
		      unit,
		      is_builtin ? "built-in" : "next available");
	    }

	retry :

#ifdef	USE_REGISTRY_ENTRY_ID
	    kr = registerInterfaceWithIORegistryEntryID(S_connect,
							entryID,
							unit,
							(dbdict == NULL) ? kIONetworkStackRegisterInterfaceWithLowestUnit
									 : kIONetworkStackRegisterInterfaceWithUnit);
	    new_interface = copyNamedInterfaceForIORegistryEntryID(entryID);
#else	// USE_REGISTRY_ENTRY_ID
	    kr = registerInterfaceWithIOServicePath(S_connect,
						    path,
						    unit,
						    (dbdict == NULL) ? kRegisterInterface
								     : kRegisterInterfaceWithFixedUnit);
	    new_interface = copyNamedInterfaceForIOKitPath(path);
#endif	// USE_REGISTRY_ENTRY_ID
	    if (new_interface == NULL) {
		const char  *signature;

		signature = (dbdict == NULL) ? "failed to name new interface"
					     : "failed to name known interface";

		SCLog(TRUE, LOG_ERR,
		      CFSTR(MY_PLUGIN_NAME ": %s, kr=0x%x\n"
			    MY_PLUGIN_NAME ":   path = %@\n"
			    MY_PLUGIN_NAME ":   id   = 0x%llx\n"
			    MY_PLUGIN_NAME ":   unit = %@"),
		      signature,
		      kr,
		      path,
		      entryID,
		      unit);

		if (S_debug) {
		    displayInterface(interface);
		}

		// report issue w/MessageTracer
		str = CFStringCreateWithFormat(NULL, NULL,
					       CFSTR("kr=0x%x, path=%@, unit=%@"),
					       kr,
					       path,
					       unit);
		reportIssue(signature, str);
		CFRelease(str);

		if ((dbdict != NULL) && (retries++ < 5)) {
		    usleep(50 * 1000);	// sleep 50ms between attempts
		    goto retry;
		}
	    }
	    else {
		CFNumberRef	new_unit;

		if (retries > 0) {
		    SCLog(TRUE, LOG_ERR,
			  CFSTR(MY_PLUGIN_NAME ": %s interface named after %d %s\n"
				MY_PLUGIN_NAME ":   path = %@\n"
				MY_PLUGIN_NAME ":   unit = %@"),
			  (dbdict == NULL) ? "New" : "Known",
			  retries,
			  (retries == 1) ? "try" : "tries",
			  path,
			  unit);

#ifdef	SHOW_NAMING_FAILURE
		    str = CFStringCreateWithFormat(NULL,
						   NULL,
						   CFSTR("\"%s\" interface named after %d %s, unit = %@"),
						   (dbdict == NULL) ? "New" : "Known",
						   retries,
						   (retries == 1) ? "try" : "tries",
						   unit);
		    CFUserNotificationDisplayNotice(0,
						    kCFUserNotificationStopAlertLevel,
						    NULL,
						    NULL,
						    NULL,
						    str,
						    CFSTR("Please report repeated failures."),
						    NULL);
		    CFRelease(str);
#endif	// SHOW_NAMING_FAILURE
		}

		new_unit = _SCNetworkInterfaceGetIOInterfaceUnit(new_interface);
		if (CFEqual(unit, new_unit) == FALSE) {
		    SCLog(S_debug, LOG_INFO,
			  CFSTR(MY_PLUGIN_NAME
				": interface type %@ assigned "
				"unit %@ instead of %@"),
			  type, new_unit, unit);
		}
		if (S_debug) {
		    displayInterface(new_interface);
		}

		// update if_list (with the interface name & unit)
		CFArraySetValueAtIndex(if_list, i, new_interface);
		CFRelease(new_interface);
		interface = new_interface;	// if_list holds the reference

		if (is_builtin && (S_prev_active_list != NULL)) {
		    CFIndex	where;

		    // update the list of [built-in] interfaces that were previously named
		    if (lookupInterfaceByUnit(S_prev_active_list, interface, &where) != NULL) {
			SCLog(S_debug, LOG_INFO,
			      CFSTR(MY_PLUGIN_NAME ":   and updated database (new address)"));
			CFArrayRemoveValueAtIndex(S_prev_active_list, where);
		    }
		}
		replaceInterface(interface);
	    }
	    CFRelease(unit);
	}
    }
    return;
}

#if	!TARGET_OS_IPHONE
static void
updateNetworkConfiguration(CFArrayRef if_list)
{
    Boolean		do_commit	= FALSE;
    CFIndex		i;
    CFIndex		n;
    SCPreferencesRef	prefs		= NULL;
    SCNetworkSetRef	set		= NULL;

    prefs = SCPreferencesCreate(NULL, CFSTR("InterfaceNamer:updateNetworkConfiguration"), NULL);

    set = SCNetworkSetCopyCurrent(prefs);
    if (set == NULL) {
	SCLog(TRUE, LOG_ERR, CFSTR(MY_PLUGIN_NAME ": No current set"));
	goto done;
    }

    n = CFArrayGetCount(if_list);
    for (i = 0; i < n; i++) {
	SCNetworkInterfaceRef	interface;

	interface = CFArrayGetValueAtIndex(if_list, i);
	if (SCNetworkSetEstablishDefaultInterfaceConfiguration(set, interface)) {
	    SCLog(TRUE, LOG_INFO,
		  CFSTR(MY_PLUGIN_NAME ": adding default configuration for %@"),
		  SCNetworkInterfaceGetBSDName(interface));
	    do_commit = TRUE;
	}
    }

    if (do_commit) {
	Boolean	ok;

	ok = SCPreferencesCommitChanges(prefs);
	if (!ok) {
	    SCLog(TRUE, LOG_INFO,
		  CFSTR(MY_PLUGIN_NAME ": updateNetworkConfiguration: SCPreferencesCommitChanges() failed: %s"),
		  SCErrorString(SCError()));
	    goto done;
	}

	ok = SCPreferencesApplyChanges(prefs);
	if (!ok) {
	    SCLog(TRUE, LOG_INFO,
		  CFSTR(MY_PLUGIN_NAME ": updateNetworkConfiguration: SCPreferencesApplyChanges() failed: %s"),
		  SCErrorString(SCError()));
	    goto done;
	}
    }

  done :

    if (set != NULL) {
	CFRelease(set);
	set = NULL;
    }

    if (prefs != NULL) {
	CFRelease(prefs);
	prefs = NULL;
    }

    return;
}
#endif	// !TARGET_OS_IPHONE

static void
updateInterfaces()
{
    if (S_connect == MACH_PORT_NULL) {
	// if we don't have the "IONetworkStack" connect object
	return;
    }

    if (S_iflist != NULL) {
	CFIndex	n;

	n = CFArrayGetCount(S_iflist);
	if (n > 1) {
	    CFArraySortValues(S_iflist, CFRangeMake(0, n), _SCNetworkInterfaceCompare, NULL);
	}
	nameInterfaces(S_iflist);
    }

    if (isQuiet()) {
	/*
	 * The registry [matching] has quiesced so let's
	 * - save the DB with the interfaces that have been named
	 * - update the VLAN/BOND configuration
	 * - tell everyone that we've finished (at least for now)
	 * - log those interfaces which are no longer present
	 *   in the HW config (or have yet to show up).
	 */
	writeInterfaceList(S_dblist);
	updateVirtualNetworkInterfaceConfiguration(NULL, kSCPreferencesNotificationApply, NULL);

#if	!TARGET_OS_IPHONE
	if (access("/usr/libexec/UserEventAgent",  X_OK) == -1
	    && errno == ENOENT) {
	    /*
	     * We are most likely booted into the Recovery OS with no "SCMonitor"
	     * UserEventAgent plugin running so let's make sure we update the
	     * network configuration for new interfaces.
	     */
	    updateNetworkConfiguration(S_iflist);
	}
#endif	// !TARGET_OS_IPHONE

	updateStore();

	if (S_iflist != NULL) {
	    CFRelease(S_iflist);
	    S_iflist = NULL;
	}

	if (S_prev_active_list != NULL) {
	    if (S_debug) {
		CFIndex	i;
		CFIndex	n;

		n = CFArrayGetCount(S_prev_active_list);
		if (n > 0) {
		    SCLog(TRUE, LOG_INFO,
			  CFSTR(MY_PLUGIN_NAME ": Interface%s not [yet] active"),
			  (n > 1) ? "s" : "");
		}
		for (i = 0; i < n; i++) {
		    CFDictionaryRef	if_dict;
		    CFStringRef		name;
		    CFNumberRef		type;
		    CFNumberRef		unit;

		    if_dict = CFArrayGetValueAtIndex(S_prev_active_list, i);
		    name = CFDictionaryGetValue(if_dict, CFSTR(kIOBSDNameKey));
		    type = CFDictionaryGetValue(if_dict, CFSTR(kIOInterfaceType));
		    unit = CFDictionaryGetValue(if_dict, CFSTR(kIOInterfaceUnit));
		    SCLog(TRUE, LOG_INFO,
			  CFSTR(MY_PLUGIN_NAME ":   %s%@%sType: %@, Unit: %@"),
			  (name != NULL) ? "BSD Name: " : "",
			  (name != NULL) ? name : CFSTR(""),
			  (name != NULL) ? ", " : "",
			  type,
			  unit);
		}
	    }
	    CFRelease(S_prev_active_list);
	    S_prev_active_list = NULL;
	}
    } else {
	if ((S_prev_active_list != NULL) && (CFArrayGetCount(S_prev_active_list) == 0)) {
	    /*
	     * if we've named all of the interfaces that
	     * were used during the previous boot.
	     */
	    addTimestamp(S_state, CFSTR("*RELEASE*"));
	    SCLog(S_debug, LOG_INFO,
		  CFSTR(MY_PLUGIN_NAME ": last boot interfaces have been named"));
	    updateStore();
	    CFRelease(S_prev_active_list);
	    S_prev_active_list = NULL;
	}
    }

    return;
}

#if	!TARGET_OS_EMBEDDED
static CFComparisonResult
compareMacAddress(const void *val1, const void *val2, void *context)
{
    CFDataRef		mac1	= (CFDataRef)val1;
    CFDataRef		mac2	= (CFDataRef)val2;
    CFIndex		n1;
    CFIndex		n2;
    CFComparisonResult	res;

    n1 = CFDataGetLength(mac1);
    n2 = CFDataGetLength(mac2);
    if (n1 < n2) {
	res = kCFCompareLessThan;
     } else if (n2 > n1) {
	res = kCFCompareGreaterThan;
     } else {
	res = bcmp(CFDataGetBytePtr(mac1), CFDataGetBytePtr(mac2), n1);
     }

     return res;
}

static CFStringRef
copyEthernetUUID()
{
    CFDataRef		addr;
    CFMutableArrayRef	addrs	= NULL;
    CFStringRef		guid	= NULL;
    CFIndex		i;
    CFIndex		n;

    addrs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    n = (S_dblist != NULL) ? CFArrayGetCount(S_dblist) : 0;
    for (i = 0; i < n; i++) {
	CFBooleanRef	builtin;
	CFDictionaryRef	dict;
	CFStringRef	type;

	dict = CFArrayGetValueAtIndex(S_dblist, i);
	type = CFDictionaryGetValue(dict, CFSTR(kSCNetworkInterfaceType));
	if (!isA_CFString(type) || !CFEqual(type, kSCNetworkInterfaceTypeEthernet)) {
	    continue;
	}
	builtin = CFDictionaryGetValue(dict, CFSTR(kIOBuiltin));
	if (!isA_CFBoolean(builtin) || !CFBooleanGetValue(builtin)) {
	    continue;
	}
	addr = CFDictionaryGetValue(dict, CFSTR(kIOMACAddress));
	if (!isA_CFData(addr) || (CFDataGetLength(addr) != ETHER_ADDR_LEN)) {
	    continue;
	}
	CFArrayAppendValue(addrs, addr);
    }

    if (CFArrayGetCount(addrs) == 0) {
	// if no ethernet interfaces, look for wireless
	for (i = 0; i < n; i++) {
	    CFDictionaryRef	dict;
	    CFStringRef		type;

	    dict = CFArrayGetValueAtIndex(S_dblist, i);
	    type = CFDictionaryGetValue(dict, CFSTR(kSCNetworkInterfaceType));
	    if (!isA_CFString(type) || !CFEqual(type, kSCNetworkInterfaceTypeIEEE80211)) {
		continue;
	    }
	    addr = CFDictionaryGetValue(dict, CFSTR(kIOMACAddress));
	    if (!isA_CFData(addr) || (CFDataGetLength(addr) != ETHER_ADDR_LEN)) {
		continue;
	    }
	    CFArrayAppendValue(addrs, addr);
	}
    }

    n = CFArrayGetCount(addrs);
    switch (n) {
	case 0 :
	    // if no network interfaces
	    break;
	default :
	    // sort by MAC address
	    CFArraySortValues(addrs, CFRangeMake(0, n), compareMacAddress, NULL);

	    // fall through
	case 1 : {
	    CFUUIDBytes		bytes	= { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
					    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	    CFUUIDRef		uuid;

	    // set GUID
	    addr = CFArrayGetValueAtIndex(addrs, 0);
	    bcopy(CFDataGetBytePtr(addr),
		  (void *)&bytes + sizeof(bytes) - ETHER_ADDR_LEN,
		  ETHER_ADDR_LEN);
	    uuid = CFUUIDCreateFromUUIDBytes(NULL, bytes);
	    guid = CFUUIDCreateString(NULL, uuid);
	    CFRelease(uuid);

	    SCLog(TRUE, LOG_INFO,
		  CFSTR(MY_PLUGIN_NAME ": setting platform UUID [MAC] = %@"),
		  guid);
	    break;
	}
    }

    if (addrs != NULL) CFRelease(addrs);
    return guid;
}

#ifndef kIOPlatformUUIDKey
#define kIOPlatformUUIDKey "IOPlatformUUID"
#endif
static void
updatePlatformUUID()
{
    CFStringRef		guid	= NULL;
    kern_return_t	kr;
    io_registry_entry_t	platform;

    platform = IORegistryEntryFromPath(kIOMasterPortDefault, kIODeviceTreePlane ":/");
    if (platform == MACH_PORT_NULL) {
	goto done;
    }

    guid = IORegistryEntryCreateCFProperty(platform, CFSTR(kIOPlatformUUIDKey), NULL, 0);
    if (guid != NULL) {
	// if GUID already defined
	goto done;
    }

    guid = copyEthernetUUID();
    if (guid == NULL) {
	CFUUIDRef   uuid;

	uuid = CFUUIDCreate(NULL);
	guid = CFUUIDCreateString(NULL, uuid);
	CFRelease(uuid);

	SCLog(TRUE, LOG_INFO,
	      CFSTR(MY_PLUGIN_NAME ": setting platform UUID [random] = %@"),
	      guid);
    }

if (getenv("DO_NOT_SET_PLATFORM_UUID") == NULL) {
    kr = IORegistryEntrySetCFProperty(platform, CFSTR(kIOPlatformUUIDKey), guid);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IORegistryEntrySetCFProperty(platform UUID) failed, kr=0x%x"),
	      kr);
    }
}

    addTimestamp(S_state, CFSTR("*PLATFORM-UUID*"));
    updateStore();

  done :

    if (S_vproc_transaction != NULL) {
	vproc_transaction_end(NULL, S_vproc_transaction);
	S_vproc_transaction = NULL;
    }

    if (platform != MACH_PORT_NULL) IOObjectRelease(platform);
    if (guid != NULL) CFRelease(guid);
    return;
}
#endif	// !TARGET_OS_EMBEDDED

static void
interfaceArrivalCallback(void *refcon, io_iterator_t iter)
{
    io_object_t	obj;

    while ((obj = IOIteratorNext(iter)) != MACH_PORT_NULL) {
	SCNetworkInterfaceRef	interface;

	interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(obj);
	if (interface != NULL) {
	    if (S_iflist == NULL) {
		S_iflist = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	    }
	    CFArrayAppendValue(S_iflist, interface);
	    CFRelease(interface);
	}
	IOObjectRelease(obj);
    }

    updateInterfaces();
    return;
}

/*
 * Function: stackCallback
 * Purpose:
 *   Get a reference to the single IONetworkStack object instance in
 *   the kernel. Naming requests must be sent to this object, which is
 *   attached as a client to all network interface objects in the system.
 * Note:
 *   Call IOObjectRelease on the returned object.
 */
static void
stackCallback(void *refcon, io_iterator_t iter)
{
    kern_return_t	kr;
    io_object_t		stack;

    stack = IOIteratorNext(iter);
    if (stack == MACH_PORT_NULL) {
	goto error;
    }

    kr = IOServiceOpen(stack, mach_task_self(), 0, &S_connect);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceOpen returned 0x%x"),
	      kr);
	goto error;
    }

    addTimestamp(S_state, CFSTR("*STACK*"));
    SCLog(S_debug, LOG_INFO,
	  CFSTR(MY_PLUGIN_NAME ": IONetworkStack found"));

    if (S_stack != MACH_PORT_NULL) {
	IOObjectRelease(S_stack);
	S_stack = MACH_PORT_NULL;
    }

    if ((S_timer != NULL) && CFRunLoopTimerIsValid(S_timer)) {
	// With the IONetworkStack object now available we can
	// reset (shorten?) the time we are willing to wait for
	// IOKit to quiesce.
	CFRunLoopTimerSetNextFireDate(S_timer,
				      CFAbsoluteTimeGetCurrent() + S_quiet_timeout);
    }

    updateInterfaces();

 error:
    if (stack != MACH_PORT_NULL) {
	IOObjectRelease(stack);
    }

    return;
}

static void
quietCallback(void		*refcon,
	      io_service_t	service,
	      natural_t		messageType,
	      void		*messageArgument)
{
    if (messageArgument != NULL) {
	// if not yet quiet
	return;
    }

    if (messageType == kIOMessageServiceBusyStateChange) {
	addTimestamp(S_state, CFSTR("*QUIET*"));
	SCLog(S_debug, LOG_INFO,
	      CFSTR(MY_PLUGIN_NAME ": IOKit quiet"));
    }

    if (S_connect == MACH_PORT_NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": No network stack object"));
	return;
    }

    if (S_quiet != MACH_PORT_NULL) {
	IOObjectRelease(S_quiet);
	S_quiet = MACH_PORT_NULL;
    }

    if (S_timer != NULL) {
	CFRunLoopTimerInvalidate(S_timer);
	CFRelease(S_timer);
	S_timer = NULL;
    }

    // grab (and name) any additional interfaces.
    interfaceArrivalCallback((void *)S_notify, S_iter);

#if	!TARGET_OS_EMBEDDED
    updatePlatformUUID();
#endif	// !TARGET_OS_EMBEDDED

    return;
}

static void
iterateRegistryBusy(io_iterator_t iterator, CFArrayRef nodes, CFMutableStringRef snapshot, int *count)
{
    kern_return_t	kr  = kIOReturnSuccess;;
    io_object_t		obj;

    while ((kr == kIOReturnSuccess) &&
	   ((obj = IOIteratorNext(iterator)) != MACH_PORT_NULL)) {
	uint64_t		accumulated_busy_time;
	uint32_t		busy_state;
	io_name_t		location;
	io_name_t		name;
	CFMutableArrayRef	newNodes;
	uint64_t		state;
	CFMutableStringRef	str	= NULL;

	if (nodes == NULL) {
	    newNodes = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	} else {
	    newNodes = CFArrayCreateMutableCopy(NULL, 0, nodes);
	}
	assert(newNodes != NULL);

	kr = IORegistryEntryGetName(obj, name);
	if (kr != kIOReturnSuccess) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": captureBusy IORegistryEntryGetName returned 0x%x"),
		  kr);
	    goto next;
	}

	str = CFStringCreateMutable(NULL, 0);
	CFStringAppendCString(str, name, kCFStringEncodingUTF8);

	kr = IORegistryEntryGetLocationInPlane(obj, kIOServicePlane, location);
	switch (kr) {
	    case kIOReturnSuccess :
		CFStringAppendCString(str, "@", kCFStringEncodingUTF8);
		CFStringAppendCString(str, location, kCFStringEncodingUTF8);
		break;
	    case kIOReturnNotFound :
		break;
	    default :
		SCLog(TRUE, LOG_ERR,
		      CFSTR(MY_PLUGIN_NAME ": captureBusy IORegistryEntryGetLocationInPlane returned 0x%x"),
		      kr);
		CFRelease(str);
		goto next;
	}

	CFArrayAppendValue(newNodes, str);
	CFRelease(str);

	kr = IOServiceGetBusyStateAndTime(obj, &state, &busy_state, &accumulated_busy_time);
	if (kr != kIOReturnSuccess) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": captureBusy IOServiceGetBusyStateAndTime returned 0x%x"),
		  kr);
	    goto next;
	}

#ifdef	TEST_SNAPSHOT
	// report all nodes
	busy_state = 1;
#endif	// TEST_SNAPSHOT

	if (busy_state != 0) {
	    CFStringRef	path;

	    if ((*count)++ == 0) {
		CFStringAppend(snapshot, CFSTR("Busy services :"));
	    }

	    path = CFStringCreateByCombiningStrings(NULL, newNodes, CFSTR("/"));
	    CFStringAppendFormat(snapshot, NULL,
				 CFSTR("\n  %@ [%s%s%s%d, %lld ms]"),
				 path,
				 (state & kIOServiceRegisteredState) ? "" : "!registered, ",
				 (state & kIOServiceMatchedState)    ? "" : "!matched, ",
				 (state & kIOServiceInactiveState)   ? "inactive, " : "",
				 busy_state,
				 accumulated_busy_time / kMillisecondScale);
	    CFRelease(path);
	}

	kr = IORegistryIteratorEnterEntry(iterator);
	if (kr != kIOReturnSuccess) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": captureBusy IORegistryIteratorEnterEntry returned 0x%x"),
		  kr);
	    goto next;
	}

	iterateRegistryBusy(iterator, newNodes, snapshot, count);

	kr = IORegistryIteratorExitEntry(iterator);
	if (kr != kIOReturnSuccess) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": captureBusy IORegistryIteratorExitEntry returned 0x%x"),
		  kr);
	}

      next :

	CFRelease(newNodes);
	IOObjectRelease(obj);
    }

    return;
}

static CF_RETURNS_RETAINED CFStringRef
captureBusy()
{
    int			count		= 0;
    io_iterator_t	iterator	= MACH_PORT_NULL;
    kern_return_t	kr;
    CFMutableStringRef	snapshot;

    snapshot = CFStringCreateMutable(NULL, 0);

    kr = IORegistryCreateIterator(kIOMasterPortDefault,
				  kIOServicePlane,
				  0,
				  &iterator);
    if (kr != kIOReturnSuccess) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": captureBusy IORegistryCreateIterator returned 0x%x"),
	      kr);
	return snapshot;
    }

    iterateRegistryBusy(iterator, NULL, snapshot, &count);
    if (count == 0) {
	CFStringAppend(snapshot, CFSTR("w/no busy services"));
    }

    IOObjectRelease(iterator);
    return snapshot;
}

static void
timerCallback(CFRunLoopTimerRef	timer, void *info)
{
    CFStringRef	snapshot;

    // We've been waiting for IOKit to quiesce and it just
    // hasn't happenned.  Time to just move on!
    addTimestamp(S_state, CFSTR("*TIMEOUT*"));

    // log busy nodes
    snapshot = captureBusy();
    SCLog(TRUE, LOG_ERR,
	  CFSTR(MY_PLUGIN_NAME ": timed out waiting for IOKit to quiesce\n%@"),
	  snapshot);
    reportIssue("timed out waiting for IOKit to quiesce", snapshot);
    CFRelease(snapshot);

    quietCallback((void *)S_notify, MACH_PORT_NULL, 0, NULL);
    return;
}

static Boolean
setup_IOKit(CFBundleRef bundle)
{
    uint32_t		busy;
    kern_return_t	kr;
    mach_port_t		masterPort	= MACH_PORT_NULL;
    Boolean		ok		= FALSE;
    io_object_t		root		= MACH_PORT_NULL;

    // read DB of previously named network interfaces
    S_dblist = readInterfaceList();
    if (S_dblist != NULL) {
	CFIndex	n;

	n = CFArrayGetCount(S_dblist);
	if (n > 1) {
	    CFArraySortValues(S_dblist, CFRangeMake(0, n), if_unit_compare, NULL);
	}
    }

    // get interfaces that were named during the last boot
    S_prev_active_list = previouslyActiveInterfaces();

    // track how long we've waited to see each interface.
    S_state = CFDictionaryCreateMutable(NULL,
					0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    addTimestamp(S_state, CFSTR("*START*"));

    // Creates and returns a notification object for receiving IOKit
    // notifications of new devices or state changes.
    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOMasterPort returned 0x%x"),
	      kr);
	goto done;
    }

    S_notify = IONotificationPortCreate(masterPort);
    if (S_notify == NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IONotificationPortCreate failed"));
	goto done;
    }

    // watch IOKit matching activity
    root = IORegistryEntryFromPath(masterPort, kIOServicePlane ":/");
    if (root == MACH_PORT_NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IORegistryEntryFromPath failed"));
	goto done;
    }

    kr = IOServiceAddInterestNotification(S_notify,
					  root,
					  kIOBusyInterest,
					  &quietCallback,
					  (void *)S_notify,	// refCon
					  &S_quiet);		// notification
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceAddInterestNotification returned 0x%x"),
	      kr);
	goto done;
    }

    kr = IOServiceGetBusyState(root, &busy);
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceGetBusyState returned 0x%x"),
	      kr);
	goto done;
    }

    // add a timer so we don't wait forever for IOKit to quiesce
    S_timer = CFRunLoopTimerCreate(NULL,
				   CFAbsoluteTimeGetCurrent() + S_stack_timeout,
				   0,
				   0,
				   0,
				   timerCallback,
				   NULL);
    if (S_timer == NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": CFRunLoopTimerCreate failed"));
	goto done;
    }

    CFRunLoopAddTimer(CFRunLoopGetCurrent(), S_timer, kCFRunLoopDefaultMode);

    // watch for the introduction of the IONetworkStack
    kr = IOServiceAddMatchingNotification(S_notify,
					  kIOFirstMatchNotification,
					  IOServiceMatching("IONetworkStack"),
					  &stackCallback,
					  (void *)S_notify,	// refCon
					  &S_stack);		// notification
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceAddMatchingNotification returned 0x%x"),
	      kr);
	goto done;
    }

    // check and see if the stack is already available and arm the
    // notification for its introduction.
    stackCallback((void *)S_notify, S_stack);

    // watch for the introduction of new network interfaces
    kr = IOServiceAddMatchingNotification(S_notify,
					  kIOFirstMatchNotification,
					  IOServiceMatching("IONetworkInterface"),
					  &interfaceArrivalCallback,
					  (void *)S_notify,	// refCon
					  &S_iter);		// notification
    if (kr != KERN_SUCCESS) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": IOServiceAddMatchingNotification returned 0x%x"),
	      kr);
	goto done;
    }

    // Get the current list of matches and arm the notification for
    // future interface arrivals.
    interfaceArrivalCallback((void *)S_notify, S_iter);

    // Check if IOKit has already quiesced.
    quietCallback((void *)S_notify,
		  MACH_PORT_NULL,
		  kIOMessageServiceBusyStateChange,
		  (void *)(uintptr_t)busy);

    CFRunLoopAddSource(CFRunLoopGetCurrent(),
		       IONotificationPortGetRunLoopSource(S_notify),
		       kCFRunLoopDefaultMode);

#ifdef	WAIT_PREVIOUS_BOOT_INTERFACES_OR_QUIET
    /*
     * Start the wheels turning until we've named all of
     * the interfaces that were used during the previous
     * boot, until IOKit [matching] has quiesced, or
     * until we've waited long enough.
     */
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), S_timer, MY_PLUGIN_ID);
    CFRunLoopAddSource(CFRunLoopGetCurrent(),
		       IONotificationPortGetRunLoopSource(S_notify),
		       MY_PLUGIN_ID);
    while (S_prev_active_list != NULL) {
	int	rlStatus;

	rlStatus = CFRunLoopRunInMode(MY_PLUGIN_ID, 1.0e10, TRUE);
    }
#endif	/* WAIT_PREVIOUS_BOOT_INTERFACES_OR_QUIET */

#if	!TARGET_OS_EMBEDDED
    if (S_dblist != NULL) {
	// apply special handling for the BT-PAN interface (if present)
	CFArrayApplyFunction(S_dblist,
			     CFRangeMake(0, CFArrayGetCount(S_dblist)),
			     updateBTPANInformation,
			     NULL);
    }
#endif	// !TARGET_OS_EMBEDDED

    ok = TRUE;

 done:
    if (root != MACH_PORT_NULL) {
	IOObjectRelease(root);
    }
    if (masterPort != MACH_PORT_NULL) {
	mach_port_deallocate(mach_task_self(), masterPort);
    }

    return ok;
}

static Boolean
setup_Virtual(CFBundleRef bundle)
{
    // open a SCPreferences session
    S_prefs = SCPreferencesCreate(NULL, CFSTR(MY_PLUGIN_NAME), NULL);
    if (S_prefs == NULL) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesCreate() failed: %s"),
	      SCErrorString(SCError()));
	return FALSE;
    }

    // register for change notifications.
    if (!SCPreferencesSetCallback(S_prefs, updateVirtualNetworkInterfaceConfiguration, NULL)) {
	SCLog(TRUE, LOG_ERR,
	      CFSTR(MY_PLUGIN_NAME ": SCPreferencesSetCallBack() failed: %s"),
	      SCErrorString(SCError()));
	CFRelease(S_prefs);
	return FALSE;
    }

    // schedule
    if (!SCPreferencesScheduleWithRunLoop(S_prefs, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
	if (SCError() != kSCStatusNoStoreServer) {
	    SCLog(TRUE, LOG_ERR,
		  CFSTR(MY_PLUGIN_NAME ": SCPreferencesScheduleWithRunLoop() failed: %s"),
			SCErrorString(SCError()));
	    CFRelease(S_prefs);
	    return FALSE;
	}
    }

    return TRUE;
}

static void *
exec_InterfaceNamer(void *arg)
{
    CFBundleRef		bundle  = (CFBundleRef)arg;
    CFDictionaryRef	dict;

    pthread_setname_np(MY_PLUGIN_NAME " thread");

    dict = CFBundleGetInfoDictionary(bundle);
    if (isA_CFDictionary(dict)) {
	CFNumberRef	num;

	num = CFDictionaryGetValue(dict, CFSTR(WAIT_STACK_TIMEOUT_KEY));
	if (num != NULL) {
	    if (!isA_CFNumber(num) ||
		!CFNumberGetValue(num, kCFNumberDoubleType, &S_stack_timeout) ||
		(S_stack_timeout <= 0.0)) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR(MY_PLUGIN_NAME ": " WAIT_STACK_TIMEOUT_KEY " value error"));
		S_stack_timeout = WAIT_STACK_TIMEOUT_DEFAULT;
	    }
	}

	num = CFDictionaryGetValue(dict, CFSTR(WAIT_QUIET_TIMEOUT_KEY));
	if (num != NULL) {
	    if (!isA_CFNumber(num) ||
		!CFNumberGetValue(num, kCFNumberDoubleType, &S_quiet_timeout) ||
		(S_quiet_timeout <= 0.0)) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR(MY_PLUGIN_NAME ": " WAIT_QUIET_TIMEOUT_KEY " value error"));
		S_quiet_timeout = WAIT_QUIET_TIMEOUT_DEFAULT;
	    }
	}
    }

    // setup virtual network interface monitoring
    if (!setup_Virtual(bundle)) {
	goto error;
    }

    // setup [IOKit] network interface monitoring
    if (!setup_IOKit(bundle)) {
	goto error;
    }

#if	!TARGET_OS_EMBEDDED
    // keep launchd from SIGKILL'ing us until after the platform-uuid has
    // been updated
    S_vproc_transaction = vproc_transaction_begin(NULL);
#endif	// !TARGET_OS_EMBEDDED

    goto done;

  error :
    if (S_connect != MACH_PORT_NULL) {
	IOServiceClose(S_connect);
	S_connect = MACH_PORT_NULL;
    }
    if (S_dblist != NULL) {
	CFRelease(S_dblist);
	S_dblist = NULL;
    }
    if (S_iter != MACH_PORT_NULL) {
	IOObjectRelease(S_iter);
	S_iter = MACH_PORT_NULL;
    }
    if (S_notify != MACH_PORT_NULL) {
	IONotificationPortDestroy(S_notify);
    }
    if (S_quiet != MACH_PORT_NULL) {
	IOObjectRelease(S_quiet);
	S_quiet = MACH_PORT_NULL;
    }
    if (S_stack != MACH_PORT_NULL) {
	IOObjectRelease(S_stack);
	S_stack = MACH_PORT_NULL;
    }
    if (S_state != NULL) {
	CFRelease(S_state);
	S_state = NULL;
    }
    if (S_timer != NULL) {
	CFRunLoopTimerInvalidate(S_timer);
	CFRelease(S_timer);
	S_timer = NULL;
    }

  done :
    CFRelease(bundle);
    CFRunLoopRun();

    return NULL;
}

__private_extern__
void
load_InterfaceNamer(CFBundleRef bundle, Boolean bundleVerbose)
{
    pthread_attr_t  tattr;
    pthread_t	    tid;

    if (bundleVerbose) {
	S_debug = TRUE;
    }

    CFRetain(bundle);	// released in exec_InterfaceNamer

    pthread_attr_init(&tattr);
    pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
//  pthread_attr_setstacksize(&tattr, 96 * 1024); // each thread gets a 96K stack
    pthread_create(&tid, &tattr, exec_InterfaceNamer, bundle);
    pthread_attr_destroy(&tattr);

    return;
}

//------------------------------------------------------------------------
// Main function.
#ifdef MAIN
int
main(int argc, char ** argv)
{
    CFBundleRef bundle;

    _sc_log     = FALSE;
    _sc_verbose = (argc > 1) ? TRUE : FALSE;

    S_debug = _sc_verbose;

    bundle = CFBundleGetMainBundle();
    CFRetain(bundle);	// released in exec_InterfaceNamer

    (void)exec_InterfaceNamer();

    /* not reached */
    exit(0);
    return 0;
}
#endif /* MAIN */

#ifdef	TEST_PLATFORM_UUID
int
main(int argc, char ** argv)
{
    CFStringRef	guid;
    CFArrayRef	interfaces;

    _sc_log     = FALSE;
    _sc_verbose = (argc > 1) ? TRUE : FALSE;

    S_dblist = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    interfaces = SCNetworkInterfaceCopyAll();
    if (interfaces != NULL) {
	CFIndex	i;
	CFIndex	n;

	n = CFArrayGetCount(interfaces);
	for (i = 0; i < n; i++) {
	    CFDictionaryRef		dict;
	    SCNetworkInterfaceRef	interface;

	    interface = CFArrayGetValueAtIndex(interfaces, i);
	    dict = createInterfaceDict(interface);
	    CFArrayAppendValue(S_dblist, dict);
	    CFRelease(dict);
	}
	CFRelease(interfaces);
    }

    guid = copyEthernetUUID();
    SCPrint(TRUE, stdout, CFSTR("copyEthernetUUID()  = %@\n"), (guid != NULL) ? guid : CFSTR("NULL"));
    if (guid != NULL) CFRelease(guid);

    updatePlatformUUID();
    CFRelease(S_dblist);
    exit(0);
    return 0;
}
#endif	/* TEST_PLATFORM_UUID */

#ifdef	TEST_SNAPSHOT
int
main(int argc, char ** argv)
{
    CFStringRef	snapshot;

    _sc_log     = FALSE;
    _sc_verbose = (argc > 1) ? TRUE : FALSE;

    snapshot = captureBusy();
    SCPrint(TRUE, stdout, CFSTR("%@\n"), snapshot);
    CFRelease(snapshot);

    exit(0);
    return 0;
}
#endif	/* TEST_SNAPSHOT */

