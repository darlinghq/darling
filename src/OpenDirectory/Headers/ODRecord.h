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

#import <OpenDirectory/OpenDirectory.h>

/*!
    @class       ODRecord
    @abstract    This class is used to read, update and modify records within the directory
    @discussion  This class is used to read, update and modify records within the directory.  outError is optional parameter,
                 nil can be passed if error details are not needed.
*/
@interface ODRecord : NSObject

/*!
    @method     setNodeCredentials:password:error:
    @abstract   Similar to calling -[ODNode setCredentials:] except credentials are only set for this particular
                record's node
    @discussion Sets the credentials if necessary on the ODNode referenced by this ODRecord.  Very similar to
                calling -[ODNode setCredentials:] except other records referencing the underlying node will not get
                authenticated, therefore inadvertant changes cannot occur.  If all records referencing a particular 
                node need to be updated, then use -[ODNode setCredentials:] on the original node instead.  If the
                node is already authenticated with the same name and password, it will be a NOOP call.  The original
                ODNode held by an ODRecord will be released when the credentials are changed for the connection
                associated with the record.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setNodeCredentials:(NSString *)inUsername password:(NSString *)inPassword error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     setNodeCredentialsWithRecordType:authenticationType:authenticationItems:continueItems:context:error:
    @abstract   Similar to calling -[ODNode setCredentialsWithRecordType:] except credentials are only set for this particular record's
                node
    @discussion Allows the caller to use other types of authentications that are available in OpenDirectory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  Same behavior as ODRecordSetNodeCredentials.  outError 
				is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setNodeCredentialsWithRecordType:(ODRecordType)inRecordType authenticationType:(ODAuthenticationType)inType 
                     authenticationItems:(NSArray *)inItems continueItems:(NSArray **)outItems
                                 context:(id *)outContext error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     setNodeCredentialsUsingKerberosCache:error:
    @abstract   Unsupported method.
    @discussion Unsupported method.
*/
- (BOOL)setNodeCredentialsUsingKerberosCache:(NSString *)inCacheName error:(NSError **)outError NS_DEPRECATED_MAC(10_6, 10_7);

/*!
    @method     passwordPolicyAndReturnError:
    @abstract   Returns a dictionary containing the password policy for the record if available.
    @discussion Returns a dictionary containing the password policy for the record if available.  If no policy for record
                nil will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (NSDictionary *)passwordPolicyAndReturnError:(NSError **)outError __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_9, __IPHONE_NA, __IPHONE_NA, "use effectivePoliciesAndReturnError");

/*!
    @method     verifyPassword:error:
    @abstract   Verifies the password provided is valid for the record
    @discussion Verifies the password provided is valid for the record.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)verifyPassword:(NSString *)inPassword error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     verifyExtendedWithAuthenticationType:authenticationItems:continueItems:context:error:
    @abstract   Allows use of other OpenDirectory types of authentications
    @discussion Allows the caller to use other types of authentications that are available in OpenDirectory, that may 
                require response-request loops, etc.  A bool with the result of the operation.  
                If it fails, outError can be checked for more specific error.  Some ODNodes may not support the call
                so an error code of kODErrorCredentialsMethodNotSupported may be returned.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)verifyExtendedWithAuthenticationType:(ODAuthenticationType)inType authenticationItems:(NSArray *)inItems 
                               continueItems:(NSArray **)outItems context:(id *)outContext error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     changePassword:toPassword:error:
    @abstract   Changes the password for a record
    @discussion Changes the password for a record.  The oldPassword can be nil if password is being set assuming the appropriate
                privileges are in place.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)changePassword:(NSString *)oldPassword toPassword:(NSString *)newPassword error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     synchronizeAndReturnError:
    @abstract   Synchronizes the record from the Directory in order to get current data and/or commit pending changes
    @discussion Synchronizes the record from the Directory in order to get current data.  Any previously fetched attributes
                will be re-fetch from the Directory.  This will not re-fetch the entire record, unless the entire record
                has been accessed.  Additionally, any changes made to the record will be committed to the directory,
                if the node does not do immediate commits.  outError is optional parameter, nil can be passed if error details
                are not needed.
*/
- (BOOL)synchronizeAndReturnError:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @property   recordType
    @abstract   Type of the record.
    @discussion The record type.
*/
@property (nonatomic, readonly, copy) NSString *recordType NS_AVAILABLE(10_6, NA);

/*!
    @property   recordName
    @abstract   Name of the record.
    @discussion This is the official record name.
*/
@property (nonatomic, readonly, copy) NSString *recordName NS_AVAILABLE(10_6, NA);

/*!
    @method     recordDetailsForAttributes:error:
    @abstract   Returns the attributes and values in the form of a key-value pair set.
    @discussion Returns the attributes and values in the form of a key-value pair set for this record.  The key is a 
                NSString of the attribute name (e.g., kODAttributeTypeRecordName, etc.) and the value is an NSArray
                of either NSData or NSString depending on the type of data.  Binary data will be returned as NSData.
                If nil is passed, then all currently retrieved attributes will be returned.  outError is optional parameter, 
                nil can be passed if error details are not needed.
*/
- (NSDictionary *)recordDetailsForAttributes:(NSArray *)inAttributes error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     valuesForAttribute:error:
    @abstract   Returns an NSArray of NSString or NSData values of the attribute
    @discussion Returns an NSArray of NSString or NSData depending on the type of data.  Binary data will be 
                returned as NSData.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (NSArray *)valuesForAttribute:(ODAttributeType)inAttribute error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
	@method     setValue:forAttribute:error:
	@abstract   Will take a mixture of NSData or NSString or an NSArray of either type when setting the values of an attribute
	@discussion Will take a mixture of NSData or NSString or an NSArray of either type when setting the values of an attribute.
				outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)setValue:(id)inValueOrValues forAttribute:(ODAttributeType)inAttribute error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     removeValuesForAttribute:error:
    @abstract   Removes all the values for an attribute.
    @discussion Removes all the values for an attribute.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)removeValuesForAttribute:(ODAttributeType)inAttribute error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     addValue:toAttribute:error:
    @abstract   Will add a value to an attribute
    @discussion Will add a value to an attribute.  Should be either NSData or NSString type.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)addValue:(id)inValue toAttribute:(ODAttributeType)inAttribute error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     removeValue:fromAttribute:error:
    @abstract   Will remove a value from an attribute
    @discussion Will remove a value from an attribute.  Should be either NSData or NSString type.  outError is optional 
                parameter, nil can be passed if error details are not needed.
*/
- (BOOL)removeValue:(id)inValue fromAttribute:(ODAttributeType)inAttribute error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     deleteRecordAndReturnError:
    @abstract   Deletes the record from the node and invalidates the record.
    @discussion Deletes the record from the node and invalidates the record.  The ODRecord should be
                released after deletion.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)deleteRecordAndReturnError:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     policiesAndReturnError:
    @abstract   This will copy any policies configured for the record.
    @discussion This will copy any policies configured for the record.
*/
- (NSDictionary *)policiesAndReturnError:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use accountPoliciesAndReturnError:");

/*!
    @method     effectivePoliciesAndReturnError:
    @abstract   This will copy any policies configured for the record.
    @discussion This will copy any policies configured for the record.
*/
- (NSDictionary *)effectivePoliciesAndReturnError:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use authenticationAllowedAndReturnError: and similar methods");

/*!
    @function   supportedPoliciesAndReturnError:
    @abstract   This will return a dictionary of supported policies.
    @discussion This will return a dictionary of supported policies, if appropriate, the value will be the maximum value allowed
                for the policy in question.  For example, if password history is available, it will state how much history is
                supported.
*/
- (NSDictionary *)supportedPoliciesAndReturnError:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   setPolicies:error:
    @abstract   This will set the policy for the record.
    @discussion This will set the policy for the record.  Policies are evaluated in combination with node-level policies.
*/
- (BOOL)setPolicies:(NSDictionary *)policies error:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use setAccountPolicies:error:");

/*!
    @function   setPolicy:value:error:
    @abstract   This will set a specific policy setting for the record.
    @discussion This will set a specific policy setting for the record.
*/
- (BOOL)setPolicy:(ODPolicyType)policy value:(id)value error:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use addAccountPolicy:toCategory:error:");

/*!
    @function   removePolicy:error:
    @abstract   This will remove a specific policy setting from the record.
    @discussion This will remove a specific policy setting from the record.
*/
- (BOOL)removePolicy:(ODPolicyType)policy error:(NSError **)error __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use removeAccountPolicy:fromCategory:error:");

/*!
    @method     addAccountPolicy:toCategory:error:
    @abstract   This will add a specific policy to the specific category for the record.
    @discussion This will add a specific policy to the specific category for the record.
                The specified policy will be applied, in combination with any
                node policies, to the specified record when policies are evaluated.
    @param      policy a dictionary containing the specific policy to be added.
                The dictionary may contain the following keys:
                    kODPolicyKeyIdentifier a required key identifying the policy.
                    kODPolicyKeyParameters an optional key containing a dictionary of
                        parameters that can be used for informational purposes or in
                        the policy format string.
                    kODPolicyKeyContent a required key specifying the policy,
                        from which a predicate will be created for evaluating
                        the policy.
    @param      category a valid ODPolicyCategoryType to which the policy will be added.
    @param      error an optional NSError reference for error details.
    @result     a BOOL which signifies if the policy addition succeeded, otherwise error is set.
*/
- (BOOL)addAccountPolicy:(NSDictionary *)policy toCategory:(ODPolicyCategoryType)category error:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     removeAccountPolicy:fromCategory:error:
    @abstract   This will remove a specific policy from the specific category for the record.
    @discussion This will remove a specific policy from the specific category for the record.
    @param      policy a dictionary containing the specific policy to be
                removed, with the same format as described in addAccountPolicy.
    @param      category a valid ODPolicyCategoryType from which the policy will be removed.
    @param      error an optional NSError reference for error details.
    @result     a BOOL which signifies if the policy removal succeeded, otherwise error is set.
*/
- (BOOL)removeAccountPolicy:(NSDictionary *)policy fromCategory:(ODPolicyCategoryType)category error:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     setAccountPolicies:error:
    @abstract   This will set the policies for the record.
    @discussion This will set the policies for the record, replacing any
                existing policies.  All of the policies in the set will be
                applied to the record when policies are evaluated.
    @param      policies a dictionary containing all of the policies to be set
                for the node.  The dictionary may contain the following keys:
                    kODPolicyCategoryAuthentication an optional key with a value
                        of an array of policy dictionaries that specify when
                        authentications should be allowed.
                    kODPolicyCategoryPasswordContent an optional key with a
                        value of an array of policy dictionaries the specify the
                        required content of passwords. 
                    kODPolicyCategoryPasswordChange an optional key with a value
                        of an array of policy dictionaries that specify when
                        passwords are required to be changed.
    @param      error an optional NSError reference for error details.
    @result     a BOOL which signifies if the policy set succeeded, otherwise error is set.
*/
- (BOOL)setAccountPolicies:(NSDictionary *)policies error:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     accountPoliciesAndReturnError:
    @abstract   Returns a dictionary containing any policies configured for the record.
    @discussion Returns a dictionary containing any policies configured for the record.
                Does not include any policies set for the node.
    @discussion Returns a dictionary containing any policies configured for the record.
    @param      error an optional NSError reference for error details.
    @result     a NSDictionary containing all currently set policies.  The
                format of the dictionary is the same as described in
                setAccountPolicies. 
*/
- (NSDictionary *)accountPoliciesAndReturnError:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   authenticationAllowedAndReturnError:
    @abstract   Determines if policies allow the account to authenticate.
    @discussion Determines if policies allow the account to authenticate.
                Authentication and password change policies are evaluated.
                Record-level and node-level policies are evaluated in
                combination, with record-level taking precedence over node-level
                policies.  The failure of any single policy will deny the
                authentication.
 
                This check is only definitive at the time it was requested. The
                policy or the environment could change before the authentication
                is actually requested.  Errors from the authentication request
                should be consulted.

                It is not necessary to call this function when calling
                verifyPassword or verifyPasswordExtended since those methods
                perform the same policy evaluation.

    @param      error an optional NSError reference for error details.
    @result     a bool which signifies if the authentication is allowed, otherwise error is set.
 */
- (BOOL)authenticationAllowedAndReturnError:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     passwordChangeAllowed:error:
    @abstract   Determines if policies allow the password change.
    @discussion Determines if policies allow the password change.  Password
                content policies are evaluated. Record-level and node-level
                policies are evaluated in combination, with record-level taking
                precedence over node-level policies.  The failure of any single
                policy will deny the password change.

                This check is only definitive at the time it was requested. The
                policy or the environment could change before the password change
                is actually requested.  Errors from the password change request
                should be consulted.

    @param      newPassword contains the password to be evaluated.
    @param      error an optional NSError reference for error details.
    @result     a BOOL which signifies if the password change is allowed, otherwise error is set.
 */
- (BOOL)passwordChangeAllowed:(NSString *)newPassword error:(NSError **)error __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     willPasswordExpire:
    @abstract   Determines if the password will expire within the specified time.
    @discussion Determines if the password will expire (i.e. need to be changed)
                between now and the specified number of seconds in the future. 
                Password change policies are evaluated.  Record-level and
                node-level policies are evaluated in combination, with
                record-level taking precedence over node-level policies.
    @param      willExpireIn the number of seconds from the current time to be
                used as the upper-bound for the password expiration period.
    @result     a BOOL which signifies if the password will expire within the
                specified time.
 */
- (BOOL)willPasswordExpire:(uint64_t)willExpireIn __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     willAuthenticationsExpire:
    @abstract   Determines if authentications will expire within the specified time.
    @discussion Determines if authentications will expire (i.e. session and/or
                account expires) between now and the specified number of seconds
                in the future.  Authentication policies are evaluated.
                Record-level and node-level policies are evaluated in
                combination, with record-level taking precedence over node-level
                policies. 
    @param      willExpireIn the number of seconds from the current time to be
                used as the upper-bound for the authentication expiration period.
    @result     a BOOL which signifies if authentications will expire within the
                specified time.
 */
- (BOOL)willAuthenticationsExpire:(uint64_t)willExpireIn __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     secondsUntilPasswordExpires
    @abstract   Determines how many seconds until the password expires.
    @discussion Determines how many seconds until the password expires (i.e.
                needs changing).  Password change policies are evaluated.
                Record-level and node-level policies are evaluated in
                combination, with record-level taking precedence over node-level
                policies. 
    @result     the number of seconds until the password expires.  If multiple
                policies will cause the password to expire, the soonest
                expiration time is returned.  If already expired,
                kODExpirationTimeExpired is returned.  If there are no password
                change policies, kODExpirationTimeNeverExpires is returned. 
 */
@property (readonly) int64_t secondsUntilPasswordExpires __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @method     secondsUntilAuthenticationsExpire
    @abstract   Determines how many seconds until authentications expire.
    @discussion Determines how many seconds until authentications expire (i.e.
                session and/or account expires). Authentication policies are
                evaluated.   Record-level and node-level policies are evaluated
                in combination, with record-level taking precedence over
                node-level policies.
    @result     the number of seconds until authentications expire.  If multiple
                policies will cause authentications to expire, the soonest
                expiration time is returned. If already expired,
                kODExpirationTimeExpired is returned.  If there are no
                authentication policies controlling expiration,
                kODExpirationTimeNeverExpires is returned.  
 */
@property (readonly) int64_t secondsUntilAuthenticationsExpire __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

@end

/*!
    @category    ODRecord (ODRecordGroupExtensions)
    @discussion  Record extensions for checking and modifying group membership.
*/
@interface ODRecord (ODRecordGroupExtensions)

/*!
    @method     addMemberRecord:error:
    @abstract   Will add the record as a member of the group record
    @discussion Will add the record as a member of the group record.  An error will be returned if the record is not
                a group record.  Additionally, if the member record is not an appropriate type allowed as part of a group
                an error will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)addMemberRecord:(ODRecord *)inRecord error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     removeMemberRecord:error:
    @abstract   Will remove the record as a member from the group record
    @discussion Will remove the record as a member from the group record. An error will be returned if the record is not
                a group record.  Additionally, if the member record is not an appropriate type allowed as part of a group
                an error will be returned.  outError is optional parameter, nil can be passed if error details are not needed.
*/
- (BOOL)removeMemberRecord:(ODRecord *)inRecord error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     isMemberRecord:error:
    @abstract   Will use membership APIs to determine if inRecord is a member of the group
    @discussion Will use membership APIs to determine if inRecord is a member of the group.  If the receiving
                object is not a group then NO will still be returned.  outError is optional parameter, nil can be passed if 
                error details are not needed.
*/
- (BOOL)isMemberRecord:(ODRecord *)inRecord error:(NSError **)outError NS_AVAILABLE(10_6, NA);

@end
