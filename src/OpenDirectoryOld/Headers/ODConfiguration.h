/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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

#import <Foundation/Foundation.h>
#import <OpenDirectory/OpenDirectory.h>

enum {
    ODPacketSigningDisabled = 0,
    ODPacketSigningAllow = 1,
    ODPacketSigningRequired = 2,
};

enum {
    ODPacketEncryptionDisabled = 0,
    ODPacketEncryptionAllow = 1,
    ODPacketEncryptionRequired = 2,
    ODPacketEncryptionSSL = 3,
};

@class SFAuthorization;
@class ODMappings;

/* Signifies that the configuration is "joined" to the directory (a.k.a., Authenticated binding) with it's own dedicated account (often a computer account) */
FOUNDATION_EXPORT NSString *const ODTrustTypeJoined NS_AVAILABLE_MAC(10_9);

/* Signfies that some form of credentials is being used to talk to this directory node */
FOUNDATION_EXPORT NSString *const ODTrustTypeUsingCredentials NS_AVAILABLE_MAC(10_9);

/* Signifies there is no account associated with this configuration */
FOUNDATION_EXPORT NSString *const ODTrustTypeAnonymous NS_AVAILABLE_MAC(10_9);

NS_CLASS_AVAILABLE(10_9, NA)
@interface ODConfiguration : NSObject {
	@protected
	ODSession *session;
	NSString *nodeName;
	NSString *comment;
	NSArray *defaultModuleEntries;
	NSArray *authenticationModuleEntries;
	NSArray *discoveryModuleEntries;
	NSArray *generalModuleEntries;
	ODMappings *defaultMappings;
	NSString *templateName;
	NSArray *virtualSubnodes;
	BOOL hideRegistration;
	NSString *preferredDestinationHostName;
	uint16_t preferredDestinationHostPort;
	NSString *trustAccount;
	NSString *trustType;
	NSString *trustKerberosPrincipal;
	NSString *trustMetaAccount;
	BOOL trustUsesMutualAuthentication;
	BOOL trustUsesKerberosKeytab;
	BOOL trustUsesSystemKeychain;
	NSInteger packetSigning;
	NSInteger packetEncryption;
	BOOL manInTheMiddleProtection;
	NSInteger queryTimeoutInSeconds;
	NSInteger connectionSetupTimeoutInSeconds;
	NSInteger connectionIdleTimeoutInSeconds;
}

/* the nodename associated with this configuration */
@property(copy) NSString *nodeName NS_AVAILABLE_MAC(10_9);

/* comment for this configuration */
@property(copy) NSString *comment NS_AVAILABLE_MAC(10_9);

/* default mappings for this configuration (used for all modules unless specific ones are set for the module) */
@property(strong) ODMappings *defaultMappings NS_AVAILABLE_MAC(10_9);

/*
 * Name of the template used for this configuration, specifically loaded from /System/Library/OpenDirectory/Templates or /Library/OpenDirectory/Templates.  These templates
 * specify the module layout, settings, etc. appropriate for that template.   Any settings in this configuration will override settings from the template
 */
@property(copy) NSString *templateName NS_AVAILABLE_MAC(10_9);

/*
 * A list of subnodenames to be registered on behalf of this configuration.  For example, "subnode1" would register "/Nodename/subnode1" automatically so it is visible
 * without loading the actual configuration/modules.
 */
@property(copy) NSArray *virtualSubnodes NS_AVAILABLE_MAC(10_9);

/* Hides the registration of this node so it is not visible to clients in the UI. */
@property(assign) BOOL hideRegistration NS_AVAILABLE_MAC(10_9);

/* The optional hostname to be used with this configuration */
@property(copy) NSString *preferredDestinationHostName NS_AVAILABLE_MAC(10_9);

/* An optional port to be used in conjunction with the preferred hostname for this configuration */
@property(assign) uint16_t preferredDestinationHostPort NS_AVAILABLE_MAC(10_9);

/* The current trust account used with this configuration */
@property(readonly, copy) NSString *trustAccount NS_AVAILABLE_MAC(10_9);

/* The current trust meta account used with this configuration */
@property(readonly, copy) NSString *trustMetaAccount NS_AVAILABLE_MAC(10_9);

/* The current trust kerberos account used with this configuration */
@property(readonly, copy) NSString *trustKerberosPrincipal NS_AVAILABLE_MAC(10_9);

/* Type of trust established for this configuration */
@property(readonly, copy) NSString *trustType NS_AVAILABLE_MAC(10_9);

/* Trust uses mutual authentication for security */
@property(readonly) BOOL trustUsesMutualAuthentication NS_AVAILABLE_MAC(10_9);

/* Trust uses keytab for password storage */
@property(readonly) BOOL trustUsesKerberosKeytab NS_AVAILABLE_MAC(10_9);

/* Trust uses the system keychain to store password */
@property(readonly) BOOL trustUsesSystemKeychain NS_AVAILABLE_MAC(10_9);

/* Determines if packet signing is used for this configuration, should be one of the ODPacketSigning values */
@property(assign) NSInteger packetSigning NS_AVAILABLE_MAC(10_9);

/* Determins if packet encryption is used for this configuration, should be one of the ODPacketEncryption values */
@property(assign) NSInteger packetEncryption NS_AVAILABLE_MAC(10_9);

/* States if some man-in-the-middle protection is required */
@property(assign) BOOL manInTheMiddleProtection NS_AVAILABLE_MAC(10_9);

/* Timeout in seconds for all queries issued for this configuration */
@property(assign) NSInteger queryTimeoutInSeconds NS_AVAILABLE_MAC(10_9);

/* Timeout in seconds for connection setup for this configuration */
@property(assign) NSInteger connectionSetupTimeoutInSeconds NS_AVAILABLE_MAC(10_9);

/* Default timeout in seconds for all connections associated with this configuration */
@property(assign) NSInteger connectionIdleTimeoutInSeconds NS_AVAILABLE_MAC(10_9);

/* A list of default ODModuleEntry objects.  Default modules are used in all categories, after all "specific" modules */
@property(copy) NSArray *defaultModuleEntries NS_AVAILABLE_MAC(10_9);

/* A list of authentication ODModuleEntry objects */
@property(copy) NSArray *authenticationModuleEntries NS_AVAILABLE_MAC(10_9);

/* A list of discovery ODModuleEntry objects */
@property(copy) NSArray *discoveryModuleEntries NS_AVAILABLE_MAC(10_9);

/* A list of general ODModuleEntry objects used for all other APIs (Queries, modifications, etc.) */
@property(copy) NSArray *generalModuleEntries NS_AVAILABLE_MAC(10_9);

/*!
 * @method configuration
 *
 * @abstract
 * Returns an initialized and autoreleased ODConfiguration object.
 *
 * @discussion
 * Returns an initialized and autoreleased ODConfiguration object.
 */
+ (instancetype)configuration;

/*!
 * @method suggestedTrustAccount:
 *
 * @abstract
 * Returns a suggested name to use for the trust account.
 *
 * @discussion
 * Returns a suggested name to use for a trust account.  This name will be derived from the hostname
 * (if provided), otherwise it will be derived from the local hostname removing special characters
 * that may not be allowed by many systems.
 */
+ (NSString *) suggestedTrustAccount:(NSString *)hostname NS_AVAILABLE_MAC(10_9);

/*!
 * @method suggestedTrustPassword:
 *
 * @abstract
 * Returns a suggested password to be used for trust account with the requested length.
 *
 * @discussion
 * Returns a suggested password to be used for trust account with the requested length.
 */
+ (NSString *) suggestedTrustPassword:(size_t)length NS_AVAILABLE_MAC(10_9);

/*!
 * @method saveUsingAuthorization:error:
 *
 * @abstract
 * Saves the configuration using the provided authorization.
 *
 * @discussion
 * Saves the configuration using the provided authorization.
 */
- (BOOL) saveUsingAuthorization:(SFAuthorization *)authorization error:(NSError **)error NS_AVAILABLE_MAC(10_9);

/*!
 * @method addTrustType:trustAccount:trustPassword:username:password:joinExisting:error:
 *
 * @abstract
 * Adds a trust account with the provided name and password using the credentials provided by the user.
 *
 * @discussion
 * Adds a trust account with the provided name and password using the credentials provided by the user.  User can
 * request that the trust be forcibly created (replacing existing trust if found in directory).  A trust should be
 * established only after enough configuration is available and the configuration been saved.  If the trust is
 * required, then the configuration can be deleted if necessary upon failure.
 */
- (BOOL) addTrustType:(NSString *)trustType trustAccount:(NSString *)account trustPassword:(NSString *)accountPassword username:(NSString *)username password:(NSString *)password joinExisting:(BOOL)join error:(NSError **)error NS_AVAILABLE_MAC(10_9);

/*!
 * @method removeTrustUsingUsername:password:deleteTrustAccount:error:
 *
 * @abstract
 * Removes trust using the provided username and password.
 *
 * @discussion
 * Removes trust using the provided username and password.  The trust account will be removed from the directory only if requested.
 */
- (BOOL) removeTrustUsingUsername:(NSString *)username password:(NSString *)password deleteTrustAccount:(BOOL)deleteAccount error:(NSError **)error NS_AVAILABLE_MAC(10_9);

@end
