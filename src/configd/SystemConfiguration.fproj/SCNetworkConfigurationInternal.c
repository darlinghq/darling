/*
 * Copyright (c) 2004-2007, 2009, 2010, 2012, 2013 Apple Inc. All rights reserved.
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
 * May 27, 2004		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/SCPrivate.h>

#include <sys/ioctl.h>
#include <net/if.h>


__private_extern__ CFDictionaryRef
__getPrefsConfiguration(SCPreferencesRef prefs, CFStringRef path)
{
	CFDictionaryRef config;
	CFIndex		n;

	config = SCPreferencesPathGetValue(prefs, path);

	n = isA_CFDictionary(config) ? CFDictionaryGetCount(config) : 0;
	switch (n) {
		case 0 :
			// ignore empty configuration entities
			config = NULL;
			break;
		case 1 :
			if (CFDictionaryContainsKey(config, kSCResvInactive)) {
				// ignore [effectively] empty configuration entities
				config = NULL;
			}
			break;
		default :
			break;
	}

	return config;
}


__private_extern__ Boolean
__setPrefsConfiguration(SCPreferencesRef	prefs,
			CFStringRef		path,
			CFDictionaryRef		config,
			Boolean			keepInactive)
{
	CFDictionaryRef		curConfig;
	CFMutableDictionaryRef	newConfig	= NULL;
	Boolean			ok;

	if ((config != NULL) && !isA_CFDictionary(config)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	curConfig = SCPreferencesPathGetValue(prefs, path);

	if (config != NULL) {
		newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
	}

	if (keepInactive) {
		if (config == NULL) {
			newConfig = CFDictionaryCreateMutable(NULL,
							      0,
							      &kCFTypeDictionaryKeyCallBacks,
							      &kCFTypeDictionaryValueCallBacks);
		}

		if (isA_CFDictionary(curConfig) && CFDictionaryContainsKey(curConfig, kSCResvInactive)) {
			// if currently disabled
			CFDictionarySetValue(newConfig, kSCResvInactive, kCFBooleanTrue);
		} else {
			// if currently enabled
			CFDictionaryRemoveValue(newConfig, kSCResvInactive);
		}
	}

	// set new configuration
	if (_SC_CFEqual(curConfig, newConfig)) {
		// if no change
		if (newConfig != NULL) CFRelease(newConfig);
		ok = TRUE;
	} else if (newConfig != NULL) {
		// if new configuration (or we are preserving a disabled state)
		ok = SCPreferencesPathSetValue(prefs, path, newConfig);
		CFRelease(newConfig);
	} else {
		ok = SCPreferencesPathRemoveValue(prefs, path);
		if (!ok && (SCError() == kSCStatusNoKey)) {
			ok = TRUE;
		}
	}

	return ok;
}


__private_extern__ Boolean
__getPrefsEnabled(SCPreferencesRef prefs, CFStringRef path)
{
	CFDictionaryRef config;

	config = SCPreferencesPathGetValue(prefs, path);
	if (isA_CFDictionary(config) && CFDictionaryContainsKey(config, kSCResvInactive)) {
		return FALSE;
	}

	return TRUE;
}


__private_extern__ Boolean
__setPrefsEnabled(SCPreferencesRef      prefs,
		  CFStringRef		path,
		  Boolean		enabled)
{
	CFDictionaryRef		curConfig;
	CFMutableDictionaryRef  newConfig       = NULL;
	Boolean			ok		= FALSE;

	// preserve current configuration
	curConfig = SCPreferencesPathGetValue(prefs, path);
	if (curConfig != NULL) {
		if (!isA_CFDictionary(curConfig)) {
			_SCErrorSet(kSCStatusFailed);
			return FALSE;
		}
		newConfig = CFDictionaryCreateMutableCopy(NULL, 0, curConfig);

		if (enabled) {
			// enable
			CFDictionaryRemoveValue(newConfig, kSCResvInactive);
		} else {
			// disable
			CFDictionarySetValue(newConfig, kSCResvInactive, kCFBooleanTrue);
		}
	} else {
		if (!enabled) {
			// disable
			newConfig = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
			CFDictionarySetValue(newConfig, kSCResvInactive, kCFBooleanTrue);
		}
	}

	// set new configuration
	if (_SC_CFEqual(curConfig, newConfig)) {
		// if no change
		if (newConfig != NULL) CFRelease(newConfig);
		ok = TRUE;
	} else if (newConfig != NULL) {
		// if updated configuration (or we are establishing as disabled)
		ok = SCPreferencesPathSetValue(prefs, path, newConfig);
		CFRelease(newConfig);
	} else {
		ok = SCPreferencesPathRemoveValue(prefs, path);
		if (!ok && (SCError() == kSCStatusNoKey)) {
			ok = TRUE;
		}
	}

	return ok;
}


static CFDictionaryRef
__copyTemplates()
{
	CFBundleRef     bundle;
	CFErrorRef	error		= NULL;
	Boolean		ok;
	CFDictionaryRef templates;
	CFURLRef	url;
	CFDataRef       xmlTemplates    = NULL;

	bundle = _SC_CFBundleGet();
	if (bundle == NULL) {
		return NULL;
	}

	url = CFBundleCopyResourceURL(bundle, CFSTR("NetworkConfiguration"), CFSTR("plist"), NULL);
	if (url == NULL) {
		return NULL;
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
	ok = CFURLCreateDataAndPropertiesFromResource(NULL, url, &xmlTemplates, NULL, NULL, NULL);
#pragma GCC diagnostic pop
	CFRelease(url);
	if (!ok || (xmlTemplates == NULL)) {
		return NULL;
	}

	// convert the XML data into a property list
	templates = CFPropertyListCreateWithData(NULL, xmlTemplates, kCFPropertyListImmutable, NULL, &error);
	CFRelease(xmlTemplates);
	if (templates == NULL) {
		if (error != NULL) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("could not load SCNetworkConfiguration templates: %@"), error);
			CFRelease(error);
		}
		return NULL;
	}

	if (!isA_CFDictionary(templates)) {
		CFRelease(templates);
		return NULL;
	}

	return templates;
}


__private_extern__ CFDictionaryRef
__copyInterfaceTemplate(CFStringRef      interfaceType,
			CFStringRef      childInterfaceType)
{
	CFDictionaryRef interface       = NULL;
	CFDictionaryRef interfaces;
	CFDictionaryRef templates;

	templates = __copyTemplates();
	if (templates == NULL) {
		return NULL;
	}

	interfaces = CFDictionaryGetValue(templates, CFSTR("Interface"));
	if (!isA_CFDictionary(interfaces)) {
		CFRelease(templates);
		return NULL;
	}

	if (childInterfaceType == NULL) {
		interface = CFDictionaryGetValue(interfaces, interfaceType);
	} else {
		CFStringRef     expandedType;

		if (CFStringFind(childInterfaceType, CFSTR("."), 0).location != kCFNotFound) {
			// if "vendor" type
			childInterfaceType = CFSTR("*");
		}

		expandedType = CFStringCreateWithFormat(NULL,
							NULL,
							CFSTR("%@-%@"),
							interfaceType,
							childInterfaceType);
		interface = CFDictionaryGetValue(interfaces, expandedType);
		CFRelease(expandedType);
	}

	if (isA_CFDictionary(interface) && (CFDictionaryGetCount(interface) > 0)) {
		CFRetain(interface);
	} else {
		interface = NULL;
	}

	CFRelease(templates);

	return interface;
}


__private_extern__ CFDictionaryRef
__copyProtocolTemplate(CFStringRef      interfaceType,
		       CFStringRef      childInterfaceType,
		       CFStringRef      protocolType)
{
	CFDictionaryRef interface       = NULL;
	CFDictionaryRef protocol	= NULL;
	CFDictionaryRef protocols;
	CFDictionaryRef templates;

	templates = __copyTemplates();
	if (templates == NULL) {
		return NULL;
	}

	protocols = CFDictionaryGetValue(templates, CFSTR("Protocol"));
	if (!isA_CFDictionary(protocols)) {
		CFRelease(templates);
		return NULL;
	}

	if (childInterfaceType == NULL) {
		interface = CFDictionaryGetValue(protocols, interfaceType);
	} else {
		CFStringRef     expandedType;

		if (CFStringFind(childInterfaceType, CFSTR("."), 0).location != kCFNotFound) {
			// if "vendor" type
			childInterfaceType = CFSTR("*");
		}

		expandedType = CFStringCreateWithFormat(NULL,
							NULL,
							CFSTR("%@-%@"),
							interfaceType,
							childInterfaceType);
		interface = CFDictionaryGetValue(protocols, expandedType);
		CFRelease(expandedType);
	}

	if (isA_CFDictionary(interface)) {
		protocol = CFDictionaryGetValue(interface, protocolType);
		if (isA_CFDictionary(protocol)) {
			CFRetain(protocol);
		} else {
			protocol = NULL;
		}
	}

	CFRelease(templates);

	return protocol;
}


__private_extern__ Boolean
__createInterface(int s, CFStringRef interface)
{
	struct ifreq	ifr;

	bzero(&ifr, sizeof(ifr));
	(void) _SC_cfstring_to_cstring(interface,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);

	if (ioctl(s, SIOCIFCREATE, &ifr) == -1) {
		SCLog(TRUE,
		      LOG_ERR,
		      CFSTR("could not create interface \"%@\": %s"),
		      interface,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}


__private_extern__ Boolean
__destroyInterface(int s, CFStringRef interface)
{
	struct ifreq	ifr;

	bzero(&ifr, sizeof(ifr));
	(void) _SC_cfstring_to_cstring(interface,
				       ifr.ifr_name,
				       sizeof(ifr.ifr_name),
				       kCFStringEncodingASCII);

	if (ioctl(s, SIOCIFDESTROY, &ifr) == -1) {
		SCLog(TRUE,
		      LOG_ERR,
		      CFSTR("could not destroy interface \"%@\": %s"),
		      interface,
		      strerror(errno));
		return FALSE;
	}

	return TRUE;
}


/*
 * For rdar://problem/4685223
 *
 * To keep MoreSCF happy we need to ensure that the first "Set" and
 * "NetworkService" have a [less than] unique identifier that can
 * be parsed as a numeric string.
 *
 * Note: this backwards compatibility code must be enabled using the
 *       following command:
 *
 *       sudo defaults write						\
 *       	/Library/Preferences/SystemConfiguration/preferences	\
 *       	MoreSCF							\
 *       	-bool true
 */
__private_extern__
CFStringRef
__SCPreferencesPathCreateUniqueChild_WithMoreSCFCompatibility(SCPreferencesRef prefs, CFStringRef prefix)
{
	static int	hack	= -1;
	CFStringRef	path	= NULL;

	if (hack < 0) {
		CFBooleanRef	enable;

		enable = SCPreferencesGetValue(prefs, CFSTR("MoreSCF"));
		hack = (isA_CFBoolean(enable) && CFBooleanGetValue(enable)) ? 1 : 0;
	}

	if (hack > 0) {
		CFDictionaryRef	dict;
		Boolean	ok;

		path = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@/%@"), prefix, CFSTR("0"));
		dict = SCPreferencesPathGetValue(prefs, path);
		if (dict != NULL) {
			// if path "0" exists
			CFRelease(path);
			return NULL;
		}

		// unique child with path "0" does not exist, create
		dict = CFDictionaryCreate(NULL,
					  NULL, NULL, 0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
		ok = SCPreferencesPathSetValue(prefs, path, dict);
		CFRelease(dict);
		if (!ok) {
			// if create failed
			CFRelease(path);
			return NULL;
		}
	}

	return path;
}


static CFDataRef
__copy_legacy_password(CFTypeRef password)
{
	if (password == NULL) {
		return NULL;
	}

	if (isA_CFData(password)) {
		CFIndex	n;

		n = CFDataGetLength(password);
		if ((n % sizeof(UniChar)) == 0) {
			CFStringEncoding	encoding;
			CFStringRef		str;

#if	__BIG_ENDIAN__
			encoding = (*(CFDataGetBytePtr(password) + 1) == 0x00) ? kCFStringEncodingUTF16LE : kCFStringEncodingUTF16BE;
#else	// __LITTLE_ENDIAN__
			encoding = (*(CFDataGetBytePtr(password)    ) == 0x00) ? kCFStringEncodingUTF16BE : kCFStringEncodingUTF16LE;
#endif
			str = CFStringCreateWithBytes(NULL,
						      (const UInt8 *)CFDataGetBytePtr(password),
						      n,
						      encoding,
						      FALSE);
			password = CFStringCreateExternalRepresentation(NULL,
									str,
									kCFStringEncodingUTF8,
									0);
			CFRelease(str);
		} else {
			password = NULL;
		}
	} else if (isA_CFString(password) && (CFStringGetLength(password) > 0)) {
		// convert password to CFData
		password = CFStringCreateExternalRepresentation(NULL,
								password,
								kCFStringEncodingUTF8,
								0);
	} else {
		password = NULL;
	}

	return password;
}


__private_extern__
Boolean
__extract_password(SCPreferencesRef	prefs,
		   CFDictionaryRef	config,
		   CFStringRef		passwordKey,
		   CFStringRef		encryptionKey,
		   CFStringRef		encryptionKeyChainValue,
		   CFStringRef		unique_id,
		   CFDataRef		*password)
{
	CFStringRef	encryption	= NULL;
	Boolean		exists		= FALSE;

	// check for keychain password
	if (config != NULL) {
		encryption = CFDictionaryGetValue(config, encryptionKey);
	}
	if ((encryption == NULL) ||
	    (isA_CFString(encryption) &&
	     CFEqual(encryption, encryptionKeyChainValue))) {
		// check password
		if (password != NULL) {
			if (prefs != NULL) {
				*password = _SCPreferencesSystemKeychainPasswordItemCopy(prefs, unique_id);
			} else {
				*password = _SCSecKeychainPasswordItemCopy(NULL, unique_id);
			}
			exists = (*password != NULL);
		} else {
			if (prefs != NULL) {
				exists = _SCPreferencesSystemKeychainPasswordItemExists(prefs, unique_id);
			} else {
				exists = _SCSecKeychainPasswordItemExists(NULL, unique_id);
			}
		}
	}

	// as needed, check for in-line password
	if (!exists && (encryption == NULL) && (config != NULL)) {
		CFDataRef	inline_password;

		inline_password = CFDictionaryGetValue(config, passwordKey);
		inline_password = __copy_legacy_password(inline_password);
		if (inline_password != NULL) {
			exists = TRUE;

			if (password != NULL) {
				*password = inline_password;
			} else {
				CFRelease(inline_password);
			}
		}
	}

	return exists;
}


__private_extern__
Boolean
__remove_password(SCPreferencesRef	prefs,
		  CFDictionaryRef	config,
		  CFStringRef		passwordKey,
		  CFStringRef		encryptionKey,
		  CFStringRef		encryptionKeyChainValue,
		  CFStringRef		unique_id,
		  CFDictionaryRef	*newConfig)
{
	CFStringRef		encryption	= NULL;
	Boolean			ok		= FALSE;

	// check for keychain password
	if (config != NULL) {
		encryption = CFDictionaryGetValue(config, encryptionKey);
	}
	if ((encryption == NULL) ||
	    (isA_CFString(encryption) &&
	     CFEqual(encryption, encryptionKeyChainValue))) {
		    // remove keychain password
		    if (prefs != NULL) {
			    ok = _SCPreferencesSystemKeychainPasswordItemRemove(prefs, unique_id);
		    } else {
			    ok = _SCSecKeychainPasswordItemRemove(NULL, unique_id);
		    }
	    }

	// as needed, check if we have an in-line password that we can remove
	if (!ok && (encryption == NULL) && (config != NULL)) {
		CFDataRef	inline_password;

		inline_password = CFDictionaryGetValue(config, passwordKey);
		inline_password = __copy_legacy_password(inline_password);
		if (inline_password != NULL) {
			CFRelease(inline_password);
			ok = TRUE;
		}
	}

	if (newConfig != NULL) {
		if (ok && (config != NULL)) {
			CFMutableDictionaryRef	temp;

			temp = CFDictionaryCreateMutableCopy(NULL, 0, config);
			CFDictionaryRemoveValue(temp, passwordKey);
			CFDictionaryRemoveValue(temp, encryptionKey);
			*newConfig = (CFDictionaryRef)temp;
		} else {
			*newConfig = NULL;
		}
	}

	return ok;
}


__private_extern__ Boolean
__rank_to_str(SCNetworkServicePrimaryRank rank, CFStringRef *rankStr)
{
	switch (rank) {
		case kSCNetworkServicePrimaryRankDefault :
			*rankStr = NULL;
			break;
		case kSCNetworkServicePrimaryRankFirst :
			*rankStr = kSCValNetServicePrimaryRankFirst;
			break;
		case kSCNetworkServicePrimaryRankLast :
			*rankStr = kSCValNetServicePrimaryRankLast;
			break;
		case kSCNetworkServicePrimaryRankNever :
			*rankStr = kSCValNetServicePrimaryRankNever;
			break;
		case kSCNetworkServicePrimaryRankScoped :
			*rankStr = kSCValNetServicePrimaryRankScoped;
			break;
		default :
			return FALSE;
	}

	return TRUE;
}


__private_extern__ Boolean
__str_to_rank(CFStringRef rankStr, SCNetworkServicePrimaryRank *rank)
{
	if (isA_CFString(rankStr)) {
		if (CFEqual(rankStr, kSCValNetServicePrimaryRankFirst)) {
			*rank = kSCNetworkServicePrimaryRankFirst;
		} else if (CFEqual(rankStr, kSCValNetServicePrimaryRankLast)) {
			*rank = kSCNetworkServicePrimaryRankLast;
		} else if (CFEqual(rankStr, kSCValNetServicePrimaryRankNever)) {
			*rank = kSCNetworkServicePrimaryRankNever;
		} else if (CFEqual(rankStr, kSCValNetServicePrimaryRankScoped)) {
			*rank = kSCNetworkServicePrimaryRankScoped;
		} else {
			return FALSE;
		}
	} else if (rankStr == NULL) {
		*rank = kSCNetworkServicePrimaryRankDefault;
	} else {
		return FALSE;
	}

	return TRUE;
}
