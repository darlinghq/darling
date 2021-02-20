/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#import <Foundation/NSObjCRuntime.h> // FOUNDATION_EXPORT
#import <objc/NSObject.h> // NSObject

@class SFAuthorization;
@class NSDictionary, NSError, NSArray;

/*!
    @const      ODSessionProxyAddress
    @abstract   the address to connect to via proxy, used when making the options dictionary
    @discussion the address to connect to via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyAddress NS_AVAILABLE(10_6, NA);

/*!
    @const      ODSessionProxyPort
    @abstract   the port to connect to via proxy, used when making the options dictionary
    @discussion the port to connect to via proxy, used when making the options dictionary.  This parameter
                is optional and should not be passed normally.
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyPort NS_AVAILABLE(10_6, NA);

/*!
    @const      ODSessionProxyUsername
    @abstract   the username to connect with via proxy, used when making the options dictionary
    @discussion the username to connect with via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyUsername NS_AVAILABLE(10_6, NA);

/*!
    @const      ODSessionProxyPassword
    @abstract   the password to connect with via proxy, used when making the options dictionary
    @discussion the password to connect with via proxy, used when making the options dictionary
*/
FOUNDATION_EXPORT NSString *const ODSessionProxyPassword NS_AVAILABLE(10_6, NA);

@class ODConfiguration;

/*!
    @class       ODSession
    @abstract    Class for working with OpenDirectory sessions.
    @discussion  Class for working with OpenDirectory sessions.
*/
@interface ODSession : NSObject {
	@private
	void *_internal;
}

/*!
    @method     defaultSession
    @abstract   Returns a shared instance of a local ODSession
    @discussion Returns a shared instance of a local ODSession.  This can be used for most situations unless
                more control is needed over the session.
*/
+ (ODSession *)defaultSession NS_AVAILABLE(10_6, NA);

/*!
    @method     sessionWithOptions:error:
    @abstract   Creates an autoreleased instance of ODSession directed over Proxy to another host
    @discussion Creates an autoreleased instance of ODSession directed over Proxy to another host.  nil
                can be passed for no options. outError is optional parameter, nil can be passed if error
                details are not needed.  Options include:

                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    ODSessionProxyAddress        NSString(hostname or IP)
                    ODSessionProxyPort           NSNumber(IP port, should not be set as it will default)
                    ODSessionProxyUsername       NSString(username)
                    ODSessionProxyPassword       NSString(password)
*/
+ (instancetype)sessionWithOptions:(NSDictionary *)inOptions error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     initWithOptions:error:
    @abstract   Creates an instance of ODSession directed over Proxy to another host
    @discussion Creates an instance of ODSession directed over Proxy to another host.  nil can be
                passed for no options. outError is optional parameter, nil can be passed if error
                details are not needed. Options include:
     
                If proxy is required then a dictionary with keys should be:
                            Key                             Value
                    ODSessionProxyAddress        NSString(hostname or IP)
                    ODSessionProxyPort           NSNumber(IP port, should not be set as it will default)
                    ODSessionProxyUsername       NSString(username)
                    ODSessionProxyPassword       NSString(password)
*/
- (instancetype)initWithOptions:(NSDictionary *)inOptions error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     nodeNamesAndReturnError:
    @abstract   Returns the node names that are registered on this ODSession
    @discussion Returns the node names that are registered on this ODSession.  outError can be nil if
                error details are not needed.
*/
- (NSArray *)nodeNamesAndReturnError:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
 * @method configurationTemplateNames
 *
 * @abstract
 * Returns a list of names as NSStrings for all available configuration templates.
 *
 * @discussion
 * Returns a list of names as NSStrings for all available configuration templates.  Configuration templates
 * have pre-configured modules and/or mappings.  Useful for re-using existing configurations
 * that may change with operating system without changing the actual configuration.
 */
@property (readonly, copy) NSArray *configurationTemplateNames NS_AVAILABLE_MAC(10_9);

/*!
 * @method mappingTemplateNames
 *
 * @abstract
 * Returns a list names as NSStrings for all available mapping templates.
 *
 * @discussion
 * Returns a list names as NSStrings for all available mapping templates.  Mapping templates have pre-configured
 * record/attribute mappings.  Useful if a configuration uses a common layout of mappings for a type of server.
 */
@property (readonly, copy) NSArray *mappingTemplateNames NS_AVAILABLE_MAC(10_9);

/*!
 * @method configurationAuthorizationAllowingUserInteraction:
 *
 * @abstract
 * Returns an authorization appropriate for managing configurations.
 *
 * @discussion
 * Returns an authorization appropriate for managing configurations.  If a proxy session is in use this method will return
 * nil and no error.
 */
- (SFAuthorization *)configurationAuthorizationAllowingUserInteraction:(BOOL)allowInteraction error:(NSError **)error NS_AVAILABLE_MAC(10_9);

/*!
 * @method configurationForNodename:
 *
 * @abstract
 * Reads the configuration for a given nodename.
 *
 * @discussion
 * Reads the configuration for a given nodename.
 */
- (ODConfiguration *)configurationForNodename:(NSString *)nodename NS_AVAILABLE_MAC(10_9);

/*!
 * @method addConfiguration:authorization:error:
 *
 * @abstract
 * Adds a new configuration to the existing ODSession.
 *
 * @discussion
 * Adds a new configuration to the existing ODSession.  An SFAuthorization can be provided if necessary.
 */
- (BOOL) addConfiguration:(ODConfiguration *)configuration authorization:(SFAuthorization *)authorization error:(NSError **)error NS_AVAILABLE_MAC(10_9);

/*!
 * @method deleteConfiguration:authorization:error:
 *
 * @abstract
 * Deletes an existing configuration from the ODSession.
 *
 * @discussion
 * Deletes an existing configuration from the ODSession.  An authorization can be provided if necessary.
 */
- (BOOL) deleteConfiguration:(ODConfiguration *)configuration authorization:(SFAuthorization *)authorization error:(NSError **)error NS_AVAILABLE_MAC(10_9);

/*!
 * @method deleteConfigurationWithNodename:authorization:error:
 *
 * @abstract
 * Deletes an existing configuration from the ODSession.
 *
 * @discussion
 * Deletes an existing configuration from the ODSession.  An authorization can be provided if necessary.
 */
- (BOOL) deleteConfigurationWithNodename:(NSString *)nodename authorization:(SFAuthorization *)authorization error:(NSError **)error NS_AVAILABLE_MAC(10_9);

@end
