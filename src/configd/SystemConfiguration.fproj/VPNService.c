/*
 * Copyright (c) 2012, 2013 Apple Inc. All rights reserved.
 */

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include "dy_framework.h"

static CFStringRef g_apple_app_prefix = CFSTR("com.apple.");

static struct {
	CFStringRef signing_id;
	Boolean domains_required;
} g_apple_app_exceptions[] = {
#if TARGET_OS_IPHONE
	{ CFSTR("com.apple.mobilesafari"),		TRUE },
	{ CFSTR("com.apple.webapp"),			TRUE },
#else
	{ CFSTR("com.apple.WebKit.NetworkProcess"),	TRUE },
#endif
};


static Boolean
isA_VPNService(CFTypeRef cf)
{
	if (isA_SCNetworkService(cf)) {
		SCNetworkInterfaceRef	interface = SCNetworkServiceGetInterface((SCNetworkServiceRef)cf);

		return (interface != NULL &&
			CFEqual(SCNetworkInterfaceGetInterfaceType(interface), kSCNetworkInterfaceTypeVPN));
	}

	return FALSE;
}


static CFArrayRef
copy_matching_services(SCPreferencesRef prefs, CFStringRef identifierDomain, CFStringRef identifier)
{
	CFMutableArrayRef	results		= NULL;
	CFArrayRef			services;

	services = SCNetworkServiceCopyAll(prefs);
	if (services != NULL) {
		CFIndex	idx;
		CFIndex	service_count = CFArrayGetCount(services);

		for (idx = 0; idx < service_count; idx++) {
			SCNetworkServiceRef		service = CFArrayGetValueAtIndex(services, idx);
			Boolean					matches	= FALSE;

			if (isA_VPNService(service)) {
				if (isA_CFString(identifierDomain) && isA_CFString(identifier)) {
					CFStringRef	ex_identifier = SCNetworkServiceCopyExternalID(service, identifierDomain);
					if (ex_identifier != NULL) {
						matches = CFEqual(ex_identifier, identifier);
						CFRelease(ex_identifier);
					}
				} else {
					matches = TRUE;
				}
			}

			if (matches) {
				if (results == NULL) {
					results = CFArrayCreateMutable(kCFAllocatorDefault, CFArrayGetCount(services) - idx, &kCFTypeArrayCallBacks);
				}
				CFArrayAppendValue(results, service);
			}
		}

		CFRelease(services);
	}

	return results;
}


static CFIndex
find_app_rule(CFDictionaryRef vpn_config, CFStringRef ruleIdentifier)
{
	CFArrayRef	app_rules = CFDictionaryGetValue(vpn_config, kSCPropNetVPNAppRules);
	CFIndex		idx;

	if (isA_CFArray(app_rules)) {
		CFIndex	rule_count = CFArrayGetCount(app_rules);

		for (idx = 0; idx < rule_count; idx++) {
			CFDictionaryRef	rule = CFArrayGetValueAtIndex(app_rules, idx);

			if (isA_CFDictionary(rule)) {
				CFStringRef	rule_id = CFDictionaryGetValue(rule, kSCValNetVPNAppRuleIdentifier);

				if (CFEqual(ruleIdentifier, rule_id)) {
					return idx;
				}
			}
		}
	}

	return -1;
}

static Boolean
validate_app_rule(CFDictionaryRef ruleSettings, Boolean check_for_apple_apps)
{
	CFIndex		account_count = 0;
	CFArrayRef	accounts;
	CFIndex		exception_idx = -1;
	CFArrayRef	executables;
	CFIndex		executable_count = 0;
	Boolean		found_exception = FALSE;
	CFIndex		idx;
	CFArrayRef	match_domains;
	CFIndex		match_domain_count = 0;

	if (!isA_CFDictionary(ruleSettings)) {
		return FALSE;
	}

	/* Validate the executable array. It needs to have at least one value. */
	executables = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleExecutableMatch);
	if (isA_CFArray(executables) && (executable_count = CFArrayGetCount(executables)) > 0) {
		for (idx = 0; idx < executable_count; idx++) {
			CFDictionaryRef	executable	= CFArrayGetValueAtIndex(executables, idx);

			if (isA_CFDictionary(executable)) {
				CFStringRef		signingID	= CFDictionaryGetValue(executable, kSCValNetVPNAppRuleExecutableSigningIdentifier);
				CFStringRef		requirement	= CFDictionaryGetValue(executable, kSCValNetVPNAppRuleExecutableDesignatedRequirement);

				if (!isA_CFString(signingID) || CFStringGetLength(signingID) == 0) {
					return FALSE;
				}

				if (check_for_apple_apps && CFStringHasPrefix(signingID, g_apple_app_prefix)) {
					for (exception_idx = 0;
					     exception_idx < sizeof(g_apple_app_exceptions) / sizeof(g_apple_app_exceptions[0]);
					     exception_idx++)
					{
						if (CFStringCompare(signingID, g_apple_app_exceptions[exception_idx].signing_id, 0) == 0) {
							found_exception = TRUE;
							break;
						}
					}

					if (!found_exception) {
						Boolean can_set_apple_app_rules = FALSE;
						SecTaskRef current_task = SecTaskCreateFromSelf(kCFAllocatorDefault);
						if (current_task != NULL) {
							CFBooleanRef entitlement =
								SecTaskCopyValueForEntitlement(current_task,
											       CFSTR("com.apple.private.app-vpn-config"),
											       NULL);
							can_set_apple_app_rules = (isA_CFBoolean(entitlement) && CFBooleanGetValue(entitlement));
							if (entitlement != NULL) {
								CFRelease(entitlement);
							}
							CFRelease(current_task);
						}
						if (!can_set_apple_app_rules) {
							return FALSE;
						}
					}
				}

				if (requirement != NULL) {
					if (!isA_CFString(requirement) || CFStringGetLength(requirement) == 0) {
						return FALSE;
					}
#if !TARGET_OS_IPHONE
				} else {
					return FALSE;
#endif /* !TARGET_OS_IPHONE */
				}
			}
		}
	}

	/* Validate the accounts array. It needs to have at least one value. */
	accounts = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleAccountIdentifierMatch);
	if (isA_CFArray(accounts) && (account_count = CFArrayGetCount(accounts)) > 0) {
		for (idx = 0; idx < account_count; idx++) {
			CFStringRef	account	= CFArrayGetValueAtIndex(accounts, idx);
			if (!isA_CFString(account)) {
				return FALSE;
			}
		}
	}

	/* Either executables or accounts must be present */
	if (executable_count == 0 && account_count == 0) {
		return FALSE;
	}

	/* Validate the domains array. It's optional, so just make sure that it contains only strings if it's present. */
	match_domains = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleDNSDomainMatch);
	if (match_domains != NULL) {
		if (!isA_CFArray(match_domains)) {
			return FALSE;
		}

		match_domain_count = CFArrayGetCount(match_domains);
		for (idx = 0; idx < match_domain_count; idx++) {
			CFStringRef	domain	= CFArrayGetValueAtIndex(match_domains, idx);
			if (!isA_CFString(domain)) {
				return FALSE;
			}
		}
	}

	/* Require at least one match domain for some Apple apps (like Safari) */
	if (match_domain_count == 0 &&
	    found_exception &&
	    exception_idx >= 0 &&
	    g_apple_app_exceptions[exception_idx].domains_required)
	{
		return FALSE;
	}

	return TRUE;
}


CFArrayRef
VPNServiceCopyAllMatchingExternalID(SCPreferencesRef prefs, CFStringRef identifierDomain, CFStringRef identifier)
{
	CFArrayRef	services;

	if (prefs == NULL || !isA_CFString(identifierDomain) || !isA_CFString(identifier)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	services = copy_matching_services(prefs, identifierDomain, identifier);
	if (services == NULL) {
		_SCErrorSet(kSCStatusOK);
	}

	return services;
}


CFArrayRef
VPNServiceCopyAll(SCPreferencesRef prefs)
{
	CFArrayRef	services;

	if (prefs == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	services = copy_matching_services(prefs, NULL, NULL);
	if (services == NULL) {
		_SCErrorSet(kSCStatusOK);
	}

	return services;
}


CFArrayRef
VPNServiceCopyAppRuleIDs(VPNServiceRef service)
{
	CFMutableArrayRef		results		= NULL;
	CFDictionaryRef			vpn_config;

	if (!isA_VPNService(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	vpn_config = SCNetworkInterfaceGetConfiguration(SCNetworkServiceGetInterface(service));

	if (isA_CFDictionary(vpn_config)) {
		CFArrayRef	app_rules = CFDictionaryGetValue(vpn_config, kSCPropNetVPNAppRules);
		if (isA_CFArray(app_rules)) {
			CFIndex	app_rule_count = CFArrayGetCount(app_rules);
			CFIndex	idx;
			results = CFArrayCreateMutable(kCFAllocatorDefault, app_rule_count, &kCFTypeArrayCallBacks);
			for (idx = 0; idx < app_rule_count; idx++) {
				CFDictionaryRef	rule = CFArrayGetValueAtIndex(app_rules, idx);
				if (isA_CFDictionary(rule)) {
					CFStringRef	rule_ID = CFDictionaryGetValue(rule, kSCValNetVPNAppRuleIdentifier);
					if (isA_CFString(rule_ID)) {
						CFArrayAppendValue(results, CFDictionaryGetValue(rule, kSCValNetVPNAppRuleIdentifier));
					}
				}
			}
			if (CFArrayGetCount(results) == 0) {
				CFRelease(results);
				results = NULL;
			}
		}
	}

	if (results == NULL) {
		_SCErrorSet(kSCStatusOK);
	}

	return results;
}


Boolean
VPNServiceSetAppRule(VPNServiceRef service, CFStringRef ruleIdentifier, CFDictionaryRef ruleSettings)
{
	CFArrayRef				accounts;
	CFArrayRef				app_rules;
	CFArrayRef				executables;
	CFIndex					existing_idx		= -1;
	CFArrayRef				match_domains;
	CFMutableArrayRef		new_app_rules;
	CFMutableDictionaryRef	new_settings;
	CFMutableDictionaryRef	new_vpn_config;
	CFDictionaryRef			vpn_config;

	/* Basic parameter validation */

	if (!isA_VPNService(service) || !isA_CFString(ruleIdentifier)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!validate_app_rule(ruleSettings, TRUE)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	executables = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleExecutableMatch);
	match_domains = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleDNSDomainMatch);
	accounts = CFDictionaryGetValue(ruleSettings, kSCValNetVPNAppRuleAccountIdentifierMatch);

	/* Set the new rule config, replacing any existing rule */

	vpn_config = SCNetworkInterfaceGetConfiguration(SCNetworkServiceGetInterface(service));
	if (isA_CFDictionary(vpn_config)) {
		existing_idx = find_app_rule(vpn_config, ruleIdentifier);
		new_vpn_config = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, vpn_config);
	} else {
		new_vpn_config = CFDictionaryCreateMutable(kCFAllocatorDefault,
												   0,
												   &kCFTypeDictionaryKeyCallBacks,
												   &kCFTypeDictionaryValueCallBacks);
	}

	app_rules = CFDictionaryGetValue(new_vpn_config, kSCPropNetVPNAppRules);
	if (isA_CFArray(app_rules)) {
		new_app_rules = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, app_rules);
	} else {
		new_app_rules = CFArrayCreateMutable(kCFAllocatorDefault,
										     0,
										     &kCFTypeArrayCallBacks);
	}

	new_settings = CFDictionaryCreateMutable(kCFAllocatorDefault,
											 0,
											 &kCFTypeDictionaryKeyCallBacks,
											 &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue(new_settings, kSCValNetVPNAppRuleIdentifier, ruleIdentifier);
	if (executables != NULL && CFArrayGetCount(executables) > 0) {
		CFDictionarySetValue(new_settings, kSCValNetVPNAppRuleExecutableMatch, executables);
	}
	if (match_domains != NULL && CFArrayGetCount(match_domains) > 0) {
		CFDictionarySetValue(new_settings, kSCValNetVPNAppRuleDNSDomainMatch, match_domains);
	}
	if (accounts != NULL && CFArrayGetCount(accounts) > 0) {
		CFDictionarySetValue(new_settings, kSCValNetVPNAppRuleAccountIdentifierMatch, accounts);
	}

	if (existing_idx >= 0) {
		CFArraySetValueAtIndex(new_app_rules, existing_idx, new_settings);
	} else {
		CFArrayAppendValue(new_app_rules, new_settings);
	}

	CFDictionarySetValue(new_vpn_config, kSCPropNetVPNAppRules, new_app_rules);

	SCNetworkInterfaceSetConfiguration(SCNetworkServiceGetInterface(service), new_vpn_config);

	CFRelease(new_vpn_config);
	CFRelease(new_app_rules);
	CFRelease(new_settings);

	return TRUE;
}


CFDictionaryRef
VPNServiceCopyAppRule(VPNServiceRef service, CFStringRef ruleIdentifier)
{
	CFDictionaryRef	vpn_config;

	if (!isA_VPNService(service) || !isA_CFString(ruleIdentifier)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	vpn_config = SCNetworkInterfaceGetConfiguration(SCNetworkServiceGetInterface(service));
	if (isA_CFDictionary(vpn_config)) {
		CFIndex	idx = find_app_rule(vpn_config, ruleIdentifier);
		if (idx >= 0) {
			CFArrayRef		app_rules		= CFDictionaryGetValue(vpn_config, kSCPropNetVPNAppRules);
			CFDictionaryRef	ruleSettings	= CFArrayGetValueAtIndex(app_rules, idx);

			if (validate_app_rule(ruleSettings, FALSE)) {
				return (CFDictionaryRef)CFRetain(ruleSettings);
			} else {
				_SCErrorSet(kSCStatusFailed);
			}
		} else {
			_SCErrorSet(kSCStatusNoKey);
		}
	} else {
		_SCErrorSet(kSCStatusFailed);
	}

	return NULL;
}


Boolean
VPNServiceRemoveAppRule(VPNServiceRef service, CFStringRef ruleIdentifier)
{
	CFDictionaryRef	vpn_config;

	if (!isA_VPNService(service) || !isA_CFString(ruleIdentifier)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	vpn_config = SCNetworkInterfaceGetConfiguration(SCNetworkServiceGetInterface(service));
	if (isA_CFDictionary(vpn_config)) {
		CFIndex	idx = find_app_rule(vpn_config, ruleIdentifier);
		if (idx >= 0) {
			CFArrayRef				current_app_rules;
			current_app_rules = CFDictionaryGetValue(vpn_config, kSCPropNetVPNAppRules);
			if (isA_CFArray(current_app_rules)) {
				CFMutableDictionaryRef	new_vpn_config;
				CFMutableArrayRef		new_app_rules;

				new_vpn_config = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, vpn_config);
				new_app_rules = CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, current_app_rules);

				CFArrayRemoveValueAtIndex(new_app_rules, idx);
				if (CFArrayGetCount(new_app_rules) > 0) {
					CFDictionarySetValue(new_vpn_config, kSCPropNetVPNAppRules, new_app_rules);
				} else {
					CFDictionaryRemoveValue(new_vpn_config, kSCPropNetVPNAppRules);
				}

				SCNetworkInterfaceSetConfiguration(SCNetworkServiceGetInterface(service), new_vpn_config);

				CFRelease(new_vpn_config);
				CFRelease(new_app_rules);

				return TRUE;
			} else {
				_SCErrorSet(kSCStatusFailed);
			}
		} else {
			_SCErrorSet(kSCStatusNoKey);
		}
	} else {
		_SCErrorSet(kSCStatusFailed);
	}

	return FALSE;
}


Boolean
VPNServiceIsManagedAppVPN(VPNServiceRef service)
{
	Boolean result = FALSE;
	CFStringRef mc_external_id = SCNetworkServiceCopyExternalID(service, CFSTR("MCVPNUUID"));
	if (isA_CFString(mc_external_id)) {
		result = TRUE;
	}
	if (mc_external_id != NULL) {
		CFRelease(mc_external_id);
	}
	return result;
}

