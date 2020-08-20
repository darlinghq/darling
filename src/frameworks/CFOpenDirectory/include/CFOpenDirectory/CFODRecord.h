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

#if !defined(__OPENDIRECTORY_CFODRECORD__)
#define __OPENDIRECTORY_CFODRECORD__

#include <CFOpenDirectory/CFOpenDirectory.h>

__BEGIN_DECLS

/*!
    @function   ODRecordGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODRecord object
    @result     a valid CFTypeID for the ODRecord object
*/
CF_EXPORT
CFTypeID ODRecordGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetNodeCredentials
    @abstract   Similar to calling ODNodeSetCredentials except credentials are only set for this particular record's node
    @discussion Sets the credentials if necessary on the ODNodeRef referenced by this ODRecordRef.  Very similar to
                calling ODNodeSetCredentials except other records referencing the underlying ODNodeRef will not get
                authenticated, therefore inadvertant changes cannot occur.  If all records referencing a particular 
                ODNodeRef need to be updated, then use ODNodeSetCredentials on the original ODNodeRef instead.  If the
                ODNodeRef is already authenticated with the same name and password, this will be a NOOP call.  The original
                ODNodeRef held by an ODRecordRef will be released and a new ODNodeRef will be created when the credentials
                are set for this ODRecordRef.  Calling this on multiple records could result in multiple References into the 
                OpenDirectory daemon, which could cause errors logged into /var/log/system.log if a threshold is reached.
    @param      record an ODRecordRef to use
    @param      username a CFStringRef of the username used to authenticate
    @param      password a CFStringRef of the password used to authenticate
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  Upon failure the original node
                will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentials(ODRecordRef record, CFStringRef username, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetNodeCredentialsExtended
    @abstract   Similar to calling ODNodeSetCredentialsExtended except credentials are only set for this particular record's
                node
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.  Same behavior as ODRecordSetNodeCredentials.
    @param      record an ODRecordRef to use
    @param      recordType a ODRecordTypeRef of the type of record to do the authentication with
    @param      authType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      authItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Upon failure the original node will still be intact.
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsExtended(ODRecordRef record, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetNodeCredentialsUsingKerberosCache
    @abstract   Unsupported function.
    @discussion Unsupported function.
*/
CF_EXPORT
bool ODRecordSetNodeCredentialsUsingKerberosCache(ODRecordRef record, CFStringRef cacheName, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6, __MAC_10_7, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODRecordCopyPasswordPolicy
    @abstract   Returns a CFDictionaryRef of the effective policy for the user if available
    @discussion Returns a CFDictionaryRef of the effective policy for the user if available
    @param      allocator a CFAllocatorRef to use
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the password policies for the supplied record, or NULL if no policy set
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyPasswordPolicy(CFAllocatorRef allocator, ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_6, __MAC_10_9, __IPHONE_NA, __IPHONE_NA, "use ODRecordCopyEffectivePolicies");

/*!
    @function   ODRecordVerifyPassword
    @abstract   Verifies the password provided is valid for the record
    @discussion Verifies the password provided is valid for the record.
    @param      record an ODRecordRef to use
    @param      password a CFStringRef of the password that is being verified
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordVerifyPasswordExtended
    @abstract   Allows use of other Open Directory types of authentications to verify a record password
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may 
                require response-request loops, etc.
    @param      record an ODRecordRef to use
    @param      authType a ODAuthenticationTypeRef of the type of authentication to be used (e.g., kODAuthenticationTypeCRAM_MD5)
    @param      authItems a CFArrayRef of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems a pointer to CFArrayRef that will be assigned to a CFArrayRef of CFData items if the
                call returned any values followup values
    @param      outContext a pointer to ODContextRef if the call requires further calls for response-request auths.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.  Some ODNodes may not support the call so an error of
                eNotHandledByThisNode or eNotYetImplemented may be returned.
*/
CF_EXPORT
bool ODRecordVerifyPasswordExtended(ODRecordRef record, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordChangePassword
    @abstract   Changes the password of an ODRecord
    @discussion Changes the password of an ODRecord.  If NULL is passed into inOldPassword, then an attempt to set
                the password will be tried.  If changing a password, then both old and new passwords should be supplied.
    @param      record an ODRecordRef to use
    @param      oldPassword a CFString of the record's old password (NULL is optional).
    @param      newPassword a CFString of the record's new password
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordChangePassword(ODRecordRef record, CFStringRef oldPassword, CFStringRef newPassword, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordGetRecordType
    @abstract   Returns the record type of an ODRecordRef
    @discussion Returns the record type of an ODRecordRef
    @param      record an ODRecordRef to use
    @result     a CFStringRef of the record type for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordType(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordGetRecordName
    @abstract   Returns the official record name of an ODRecordRef
    @discussion Returns the official record name of an ODRecordRef which typically corresponds to the first value
                of the kODAttributeTypeRecordName attribute, but not always.  This name should be a valid name in either case.
    @param      record an ODRecordRef to use
    @result     a CFStringRef of the record name for this ODRecordRef
*/
CF_EXPORT
CFStringRef ODRecordGetRecordName(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordCopyValues
    @abstract   Returns the value of an attribute as an array of CFStringRef or CFDataRef types
    @discussion Returns the value of an attribute as an array of CFStringRef or CFDataRef, depending on
                whether the data is Binary or not.  If the value has been fetched from the directory previously
                a copy of the internal storage will be returned without going to the directory.  If it has not been fetched
                previously, then it will be fetched at that time.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef or ODAttributeType of the attribute (e.g., kODAttributeTypeRecordName, etc.)
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef of the attribute requested if possible, or NULL if the attribute doesn't exist
*/
CF_EXPORT
CFArrayRef ODRecordCopyValues(ODRecordRef record, ODAttributeType attribute, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSetValue
    @abstract   Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute
    @discussion Will take a CFDataRef or CFStringRef or a CFArrayRef of either type and set it for the attribute.
                Any mixture of the types CFData and CFString are accepted.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute for values to be added too
    @param      valueOrValues a CFArrayRef of CFStringRef or CFDataRef types or either of the individual types, passing
                an empty CFArray deletes the attribute.  The underlying implementation will do this in the most efficient manner,
                either by adding only new values or completely replacing the values depending on the capabilities of the
                particular plugin.
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordSetValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef valueOrValues, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordAddValue
    @abstract   Adds a value to an attribute
    @discussion Adds a value to an attribute.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute for values to be added too
    @param      value a CFTypeRef of the value to be added to the attribute, either CFStringRef or CFDataRef
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordRemoveValue
    @abstract   Removes a particular value from an attribute.
    @discussion Removes a particular value from an attribute.
    @param      record an ODRecordRef to use
    @param      attribute a CFStringRef of the attribute to remove the value from
    @param      value a CFTypeRef of the value to be removed from the attribute.  Either CFStringRef or CFDataRef.
                If the value does not exist, true will be returned and no error will be set.
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordCopyDetails
    @abstract   Returns the attributes and values in the form of a key-value pair set for this record.
    @discussion Returns the attributes and values in the form of a key-value pair set for this record.  The key is a 
                CFStringRef or ODAttributeType of the attribute name (e.g., kODAttributeTypeRecordName, etc.) and the 
                value is an CFArrayRef of either CFDataRef or CFStringRef depending on the type of data.  Binary data will
                be returned as CFDataRef.
    @param      record an ODRecordRef to use
    @param      attributes a CFArrayRef of attributes.  If an attribute has not been fetched previously, it will be
                fetched in order to return the value.  If this parameter is NULL then all currently fetched attributes 
                will be returned.
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef of the attributes for the record
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyDetails(ODRecordRef record, CFArrayRef attributes, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordSynchronize
    @abstract   Synchronizes the record from the Directory in order to get current data and commit pending changes
    @discussion Synchronizes the record from the Directory in order to get current data.  Any previously fetched attributes
                will be refetched from the Directory.  This will not refetch the entire record, unless the entire record
                has been accessed.  Additionally, any changes made to the record will be committed to the directory
                if the node does not do immediate commits.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
*/
CF_EXPORT
bool ODRecordSynchronize(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordDelete
    @abstract   Deletes the record from the node and invalidates the record.
    @discussion Deletes the record from the node and invalidates the record.  The ODRecordRef should be
                released after deletion.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordDelete(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordAddMember
    @abstract   Will add the record as a member of the group record that is provided
    @discussion Will add the record as a member of the group record that is provided in an appopriate manner 
                based on what the directory will store.  An error will be returned if the record is not a group record.  
                Additionally, if the member record is not an appropriate type allowed as part of a group an
                error will be returned.
    @param      group an ODRecordRef of the group record to modify
    @param      member an ODRecordRef of the record to add to the group record
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordAddMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordRemoveMember
    @abstract   Will remove the record as a member from the group record that is provided
    @discussion Will remove the record as a member from the group record that is provided.  If the record type
                of group is not a Group, false will be returned with an appropriate error.
    @param      group an ODRecordRef of the group record to modify
    @param      member an ODRecordRef of the record to remove from the group record
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details
*/
CF_EXPORT
bool ODRecordRemoveMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordContainsMember
    @abstract   Will use membership APIs to resolve group membership based on Group and Member record combination
    @discussion Will use membership APIs to resolve group membership based on Group and Member record combination.
                This API does not check attributes values directly, instead uses system APIs to deal with nested
                memberships.
    @param      group an ODRecordRef of the group to be checked for membership
    @param      member an ODRecordRef of the member to be checked against the group
    @param      error an optional CFErrorRef reference for error details
    @result     returns true or false depending on result
*/
CF_EXPORT
bool ODRecordContainsMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODRecordCopyPolicies
    @abstract   This will copy any policies configured for the record.
    @discussion This will copy any policies configured for the record.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing all currently configured policies
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyPolicies(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODRecordCopyAccountPolicies");

/*!
    @function   ODRecordCopyEffectivePolicies
    @abstract   This will copy the effective policies for the record (merging any node-level policies).
    @discussion This will copy the effective policies for the record (merging any node-level policies).
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing all currently configured policies (merging any node-level policies)
*/
CF_EXPORT
CFDictionaryRef ODRecordCopyEffectivePolicies(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODRecordAuthenticationAllowed and similar functions");

/*!
    @function   ODRecordCopySupportedPolicies
    @abstract   This will return a dictionary of supported policies.
    @discussion This will return a dictionary of supported policies, if appropriate, the value will be the maximum value allowed
                for the policy in question.  For example, if password history is available, it will state how much history is
                supported.
    @param      record an ODRecordRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing all currently supported policies
*/
CF_EXPORT
CFDictionaryRef ODRecordCopySupportedPolicies(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODRecordSetPolicies
    @abstract   This will set the policy for the record.
    @discussion This will set the policy for the record.  Policies are evaluated in combination with node-level policies.
    @param      record an ODRecordRef to use
    @param      policies a CFDictionary of policies to be set
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy set succeeded, otherwise error is set. 
*/
CF_EXPORT
bool ODRecordSetPolicies(ODRecordRef record, CFDictionaryRef policies, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODRecordSetAccountPolicies");

/*!
    @function   ODRecordSetPolicy
    @abstract   This will set a specific policy setting for the record.
    @discussion This will set a specific policy setting for the record.
    @param      record an ODRecordRef to use
    @param      policy a valid ODPolicyType
    @param      value a CFTypeRef to be set (should be of appropriate type for the policy)
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy set succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODRecordSetPolicy(ODRecordRef record, ODPolicyType policy, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODRecordAddAccountPolicy");

/*!
    @function   ODRecordRemovePolicy
    @abstract   This will remove a specific policy setting from the record.
    @discussion This will remove a specific policy setting from the record.
    @param      record an ODRecordRef to use
    @param      policy a valid ODPolicyType
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy removal succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODRecordRemovePolicy(ODRecordRef record, ODPolicyType policy, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODRecordRemoveAccountPolicy");

/*!
    @function   ODRecordAddAccountPolicy
    @abstract   This will add an account policy to the record for the specified category.
    @discussion This will add an account policy to the record for the specified category.
                The node-level and record-level policies will be combined and
				evaluated as appropriate, ensuring the strongest policy is enforced.
    @param      record an ODRecordRef to use.
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
    @param      error is an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the policy addition succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODRecordAddAccountPolicy(ODRecordRef record, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordRemoveAccountPolicy
    @abstract   This will remove an account policy from the record for the specified category.
    @discussion This will remove an account policy from the record for the specified category.
    @param      record an ODRecordRef to use.
    @param      policy a dictionary containing the specific policy to be
                removed, with the same format as described in ODRecordAddAccountPolicy.
    @param      category a valid ODPolicyCategoryType from which the policy will be removed.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the policy removal succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODRecordRemoveAccountPolicy(ODRecordRef record, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordSetAccountPolicies
    @abstract   This will set the policies for the record.
    @discussion This will set the policies for the record, replacing any
                existing policies.  All of the policies in the set will be
                applied to the record when policies are evaluated.
    @param      record an ODRecordRef to use.
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
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the policy set succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODRecordSetAccountPolicies(ODRecordRef record, CFDictionaryRef policies, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordCopyAccountPolicies
    @abstract   This will copy any policies configured for the record.
    @discussion This will copy any policies configured for the record.  Does not
                copy any policies set for the node.
    @param      record an ODRecordRef to use.
    @param      error an optional CFErrorRef reference for error details.
    @result     a CFDictionaryRef containing all currently set policies.  The
                format of the dictionary is the same as described in
                ODRecordSetAccountPolicies(). 
*/
CF_EXPORT CF_RETURNS_RETAINED
CFDictionaryRef ODRecordCopyAccountPolicies(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordAuthenticationAllowed
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

                It is not necessary to call this function when callingg
                ODRecordVerifyPassword or ODRecordVerifyPasswordExtended
                since those functions perform same policy evaluation.

    @param      record an ODRecordRef to use.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the authentication is allowed, otherwise error is set.
 */
CF_EXPORT
bool ODRecordAuthenticationAllowed(ODRecordRef record, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordPasswordChangeAllowed
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

    @param      record an ODRecordRef to use.
    @param      newPassword contains the password to be evaluated.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the password change is allowed, otherwise error is set.
 */
CF_EXPORT
bool ODRecordPasswordChangeAllowed(ODRecordRef record, CFStringRef newPassword, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordWillPasswordExpire
    @abstract   Determines if the password will expire within the specified time.
    @discussion Determines if the password will expire (i.e. need to be changed)
                between now and the specified number of seconds in the future. 
                Record-level and node-level policies are evaluated
                together, with record-level taking precedence over node-level
                policies.
    @param      record an ODRecordRef to use.
    @param      willExpireIn the number of seconds from the current time to be
                used as the upper-bound for the password expiration period.
    @result     a bool which signifies if the password will expire within the
                specified time.
 */
CF_EXPORT
bool ODRecordWillPasswordExpire(ODRecordRef record, uint64_t willExpireIn) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordWillAuthenticationsExpire
    @abstract   Determines if authentications will expire within the specified time.
    @discussion Determines if authentications will expire (i.e. session and/or
                account expires) between now and the specified number of seconds
                in the future.  Record-level and node-level policies are evaluated
                together, with record-level taking precedence over node-level
                policies. 
    @param      record an ODRecordRef to use.
    @param      willExpireIn the number of seconds from the current time to be
                used as the upper-bound for the authentication expiration period.
    @result     a bool which signifies if authentications will expire within the
                specified time.
 */
CF_EXPORT
bool ODRecordWillAuthenticationsExpire(ODRecordRef record, uint64_t willExpireIn) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordSecondsUntilPasswordExpires
    @abstract   Determines how many seconds until the password expires.
    @discussion Determines how many seconds until the password expires (i.e.
                needs changing).  Password change policies are evaluated.
                Record-level and node-level policies are evaluated in
                combination, with record-level taking precedence over node-level
                policies. 
    @param      record an ODRecordRef to use.
    @result     the number of seconds until the password expires.  If multiple
                policies will cause the password to expire, the soonest
                expiration time is returned.  If already expired,
                kODExpirationTimeExpired is returned.  If there are no password
                change policies, kODExpirationTimeNeverExpires is returned. 
 */
CF_EXPORT
int64_t ODRecordSecondsUntilPasswordExpires(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODRecordSecondsUntilAuthenticationsExpire
    @abstract   Determines how many seconds until authentications expire.
    @discussion Determines how many seconds until authentications expire (i.e.
                session and/or account expires). Record-level and node-level 
                policies are evaluated together, with record-level taking 
                precedence over node-level policies
    @param      record an ODRecordRef to use.
    @result     the number of seconds until authentications expire.  If multiple
                policies will cause authentications to expire, the soonest
                expiration time is returned. If already expired,
                kODExpirationTimeExpired is returned.  If there are no
                authentication policies controlling expiration,
                kODExpirationTimeNeverExpires is returned.  
 */
CF_EXPORT
int64_t ODRecordSecondsUntilAuthenticationsExpire(ODRecordRef record) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

__END_DECLS

#endif /* ! __OPENDIRECTORY_CFODRECORD__ */
