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

#if !defined(__OPENDIRECTORY_CFODNODE__)
#define __OPENDIRECTORY_CFODNODE__ 1

#include <CFOpenDirectory/CFOpenDirectory.h>

__BEGIN_DECLS

/*!
    @function   ODNodeGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODNode objects
    @result     a valid CFTypeID for the ODNode object
*/
CF_EXPORT
CFTypeID ODNodeGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateWithNodeType
    @abstract   Creates an ODNodeRef based on a specific node type
    @discussion Creates an ODNodeRef based on a specific node type
    @param      allocator a memory allocator to use for this object
    @param      session an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      nodeType an ODNodeType of the node to open
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL.  outError can be checked for details upon
                failure.
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session, ODNodeType nodeType, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateWithName
    @abstract   Creates an ODNodeRef based on a partciular node name
    @discussion Creates an ODNodeRef based on a particular node name
    @param      allocator a memory allocator to use for this object
    @param      session an ODSessionRef, either kODSessionDefault or a valid ODSessionRef can be passed
    @param      nodeName a CFStringRef of the name of the node to open
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL. outError can be checked for specific
                error
*/
CF_EXPORT
ODNodeRef ODNodeCreateWithName(CFAllocatorRef allocator, ODSessionRef session, CFStringRef nodeName, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCreateCopy
    @abstract   Creates a copy, including any remote credentials used for Proxy and/or Node authentication
    @discussion Creates a copy of the object including all credentials used for the original.  Can be used for future
                references to the same node setup.
    @param      allocator a memory allocator to use for this object
    @param      node an ODNodeRef to make a copy of
    @param      error an optional CFErrorRef reference for error details
    @result     a valid ODNodeRef if successful, otherwise returns NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
ODNodeRef ODNodeCreateCopy(CFAllocatorRef allocator, ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySubnodeNames
    @abstract   Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes
    @discussion Returns a CFArray of subnode names for this node, which may contain sub-nodes or search policy nodes.
                Commonly used with Search policy nodes.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of nodes, otherwise NULL, with outError set to a CFErrorRef
*/
CF_EXPORT
CFArrayRef ODNodeCopySubnodeNames(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyUnreachableSubnodeNames
    @abstract   Will return names of subnodes that are not currently reachable.
    @discussion Will return names of subnodes that are not currently reachable.  Commonly used with Search policy nodes
                to determine if any nodes are currently unreachable, but may also return other subnodes if the
                Open Directory plugin supports.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef with the list of unreachable nodes or NULL if no bad nodes
*/
CF_EXPORT
CFArrayRef ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeGetName
    @abstract   Returns the node name of the node that was opened
    @discussion Returns the node name of the node that was opened
    @param      node an ODNodeRef to use
    @result     a CFStringRef of the node name that is current or NULL if no open node
*/
CF_EXPORT
CFStringRef ODNodeGetName(ODNodeRef node) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyDetails
    @abstract   Returns a dictionary with details about the node in dictionary form
    @discussion Returns a dictionary with details about the node in dictionary form.
    @param      node an ODNodeRef to use
    @param      keys a CFArrayRef listing the keys the user wants returned, such as 
                kODAttributeTypeStreet
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing the requested key and values in form of a CFArray
*/
CF_EXPORT
CFDictionaryRef ODNodeCopyDetails(ODNodeRef node, CFArrayRef keys, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySupportedRecordTypes
    @abstract   Returns a CFArrayRef of the record types supported by this node.
    @discussion Returns a CFArrayRef of the record types supported by this node.  If node does not support the check
                then all possible types will be returned.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the supported Record types on this node.
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedRecordTypes(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopySupportedAttributes
    @abstract   Will return a list of attribute types supported for that attribute if possible
    @discussion Will return a list of attribute types supported for that attribute if possible.  If no specific
                types are available, then all possible values will be returned instead.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef with the type of record to check attribute types.  If NULL is passed it will
                return all possible attributes that are available.
    @param      error an optional CFErrorRef reference for error details
    @result     a valid CFArrayRef of CFStrings listing the attributes supported for the requested record type
*/
CF_EXPORT
CFArrayRef ODNodeCopySupportedAttributes(ODNodeRef node, ODRecordType recordType, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentials
    @abstract   Sets the credentials for interaction with the ODNode
    @discussion Sets the credentials for interaction with the ODNode.  Record references, etc. will use these credentials
                to query or change data.  Setting the credentials on a node referenced by other OD object types will
                change the credentials for all for all references.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the Record Type to use, if NULL is passed, defaults to a 
                kODRecordTypeUsers
    @param      recordName a CFString of the username to be used for this node authentication
    @param      password a CFString of the password to be used for this node authentication
    @param      error an optional CFErrorRef reference for error details
    @result     returns true on success, otherwise outError can be checked for details.  If the authentication failed, 
                the previous credentials are used.
*/
CF_EXPORT
bool ODNodeSetCredentials(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFStringRef password, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentialsExtended
    @abstract   Allows use of other Open Directory types of authentications to set the credentials for an ODNode
    @discussion Allows the caller to use other types of authentications that are available in Open Directory, that may
                require response-request loops, etc.  Not all OD plugins will support this call, look for 
                kODErrorCredentialsMethodNotSupported in outError.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordType of the type of record to do the authentication with
    @param      authType a ODAuthenticationType of the type of authentication to be used (e.g., kDSStdAuthNTLMv2)
    @param      authItems a CFArray of CFData or CFString items that will be sent in order to the auth process
    @param      outAuthItems will be assigned to a pointer of a CFArray of CFData items if there are returned values
    @param      outContext will return a pointer to a context if caller supplies a container, and the call requires a
                context.  If a non-NULL value is returned, then more calls must be made with the Context to continue
                the authorization.
    @param      error an optional CFErrorRef reference for error details
    @result     a bool will be returned with the result of the operation and outAuthItems set with response items
                and outContext set for any needed continuation.
*/
CF_EXPORT
bool ODNodeSetCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeSetCredentialsUsingKerberosCache
    @abstract   Unsupported function.
    @discussion Unsupported function.
*/
CF_EXPORT
bool ODNodeSetCredentialsUsingKerberosCache(ODNodeRef node, CFStringRef cacheName, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_6, __MAC_10_7, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODNodeCreateRecord
    @abstract   Takes a record and all of the provided attributes and creates the record in the node
    @discussion Takes all the provided attributes and type to create an entire record.  The function will assign a
                UUID to the record automatically.  This UUID can be overwritten by the client by passing with the
                other attributes.
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the type of record (e.g., kODRecordTypeUsers, etc.)
    @param      recordName a CFStringRef of the name of record
    @param      attributeDict a CFDictionaryRef of key-value pairs for attribute values.  The key is a CFStringRef of the
                attribute name or ODRecordType constant such as kODAttributeTypeRecordName.  The value must be a CFArrayRef of
                CFDataRef or CFStringRef.  If additional kODAttributeTypeRecordName are to be set, they can be passed in the 
                inAttributes list.  This parameter is optional and can be NULL.  If any of the attributes passed
                fail to be set, the record will be deleted and outError will be set with the appropriate error.
                If a password is not supplied with a user account, then a random password will be set automatically.  If
                an empty password is expected, then the kODAttributeTypePassword should be set to an empty CFArray.
    @param      error an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the add fails, outError can be checked for details.
*/
CF_EXPORT
ODRecordRef ODNodeCreateRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFDictionaryRef attributeDict, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCopyRecord
    @abstract   Simple API to open / create a references to a particular record on a Node
    @discussion Simple API to open / create a references to a particular record on a Node
    @param      node an ODNodeRef to use
    @param      recordType a ODRecordTypeRef of the record type to copy
    @param      recordName a CFStringRef of the record name to copy
    @param      attributes (optional) a CFArrayRef (or single ODAttributeType) of the attributes to copy from the directory.  Can be NULL when no 
                attributes are needed.  Any standard types can be passed, for example
                kODAttributeTypeAllAttributes will fetch all attributes up front.  If just standard attributes are needed, then
                kODAttributeTypeStandardOnly can be passed.
    @param      error an optional CFErrorRef reference for error details
    @result     returns a valid ODRecordRef.  If the record copy fails, the error can be checked for details.
                If the record is not found, will return NULL with a NULL error.
*/
CF_EXPORT
ODRecordRef ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFTypeRef attributes, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODNodeCustomCall
    @abstract   Sends a custom call to a node.
    @discussion This will send a custom call to a node along with the specified data, returning the result.
    @param      node an ODNodeRef to use
    @param      customCode the custom code to be sent to the node
    @param      data a data blob expected by the custom code, can be NULL of no send data
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDataRef with the result of the operation, otherwise outError can be checked for specific details
*/
CF_EXPORT CF_RETURNS_RETAINED
CFDataRef ODNodeCustomCall(ODNodeRef node, CFIndex customCode, CFDataRef data, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
 * @function ODNodeCustomCall
 *
 * @abstract
 * Sends a named custom function call to a node.
 *
 * @discussion
 * Sends a named custom function call to a node.  Custom functions are defined by the modules that implement them
 * and the parameter is defined by the module.
 *
 * @param node
 * An ODNodeRef to use
 *
 * @param function
 * A CFStringRef that specifies the name of the function
 *
 * @param payload
 * A CFType appropriate for the custom function.  The type is dictated by the module implementing the function.
 *
 * @param error
 * An optional CFErrorRef reference to receive any errors from the custom function call.
 *
 * @result
 * Returns a CFType appropriate for the function.
 */
CF_EXPORT CF_RETURNS_RETAINED
CFTypeRef
ODNodeCustomFunction(ODNodeRef node, CFStringRef function, CFTypeRef payload, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_NA);

/*!
    @function   ODNodeCopyPolicies
    @abstract   This will copy any policies configured for the node.
    @discussion This will copy any policies configured for the node.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing all currently set policies
*/
CF_EXPORT
CFDictionaryRef ODNodeCopyPolicies(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODNodeCopyAccountPolicies");

/*!
    @function   ODNodeCopySupportedPolicies
    @abstract   This will return a dictionary of supported policies.
    @discussion This will return a dictionary of supported policies, if appropriate, the value will be the maximum value allowed
                for the policy in question.  For example, if password history is available, it will state how much history is
                supported.
    @param      node an ODNodeRef to use
    @param      error an optional CFErrorRef reference for error details
    @result     a CFDictionaryRef containing all currently supported policies.  The values will be the maximum value allowed.
*/
CF_EXPORT
CFDictionaryRef ODNodeCopySupportedPolicies(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA);

/*!
    @function   ODNodeSetPolicies
    @abstract   This will set the policy for the node.
    @discussion This will set the policy for the node.  Policies are evaluated in combination with record-level policies.
    @param      node an ODNodeRef to use
    @param      policies a CFDictionary of policies to be set
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy set succeeded, otherwise error is set. 
*/
CF_EXPORT
bool ODNodeSetPolicies(ODNodeRef node, CFDictionaryRef policies, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODNodeSetAccountPolicies");

/*!
    @function   ODNodeSetPolicy
    @abstract   This will set a specific policy setting for the node.
    @discussion This will set a specific policy setting for the node.
    @param      node an ODNodeRef to use
    @param      policy a valid ODPolicyType
    @param      value a CFTypeRef to be set (should be of appropriate type for the policy)
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy set succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODNodeSetPolicy(ODNodeRef node, ODPolicyType policyType, CFTypeRef value, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODNodeAddAccountPolicy");

/*!
    @function   ODNodeRemovePolicy
    @abstract   This will remove a specific policy setting from the node.
    @discussion This will remove a specific policy setting from the node.
    @param      node an ODNodeRef to use
    @param      policy a valid ODPolicyType
    @param      error an optional CFErrorRef reference for error details
    @result     a bool which signifies if the policy removal succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODNodeRemovePolicy(ODNodeRef node, ODPolicyType policyType, CFErrorRef *error) __OSX_AVAILABLE_BUT_DEPRECATED_MSG(__MAC_10_9, __MAC_10_10, __IPHONE_NA, __IPHONE_NA, "use ODNodeRemoveAccountPolicy");

/*!
    @function   ODNodeAddAccountPolicy
    @abstract   This will add an account policy to the node for the specified category.
    @discussion This will add an account policy to the node for the specified category.
                The specified policy will be applied to all users in the
                specified node when policies are evaluated.
    @param      node an ODNodeRef to use.
    @param      policy a dictionary containing the specific policy to be added.
                The dictionary may contain the following keys:
                    kODPolicyKeyIdentifier a required key identifying the policy.
                    kODPolicyKeyParameters an optional key containing a dictionary of
                        parameters that can be used for informational purposes or in
                        the policy format string.
                    kODPolicyKeyContent a required key specifying the policy,
                        from which a predicate will be created for evaluating
                        the policy.
    @param      category a valid ODPolicyCategoryType to which the specified policy will be added.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the policy addition succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODNodeAddAccountPolicy(ODNodeRef node, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODNodeRemoveAccountPolicy
    @abstract   This will remove an account policy from the node for the specified category.
    @discussion This will remove an account policy from the node for the specified category.
    @param      node an ODNodeRef to use.
    @param      policy a dictionary containing the specific policy to be
                removed, with the same format as described in ODNodeAddAccountPolicy.
    @param      category a valid ODPolicyCategoryType from which the specified policy will be removed.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the policy removal succeeded, otherwise error is set.
*/
CF_EXPORT
bool ODNodeRemoveAccountPolicy(ODNodeRef node, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODNodeSetAccountPolicies
    @abstract   This will set the policies for the node.
    @discussion This will set the policies for the node, replacing any existing
                policies.
    @param      node an ODNodeRef to use.
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
bool ODNodeSetAccountPolicies(ODNodeRef node, CFDictionaryRef policies, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODNodeCopyAccountPolicies
    @abstract   This will copy any policies configured for the node.
    @discussion This will copy any policies configured for the node.
    @param      node an ODNodeRef to use.
    @param      error an optional CFErrorRef reference for error details.
    @result     a CFDictionaryRef containing all currently set policies.  The
                format of the dictionary is the same as described in
                ODNodeSetAccountPolicies(). 
*/
CF_EXPORT CF_RETURNS_RETAINED
CFDictionaryRef ODNodeCopyAccountPolicies(ODNodeRef node, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

/*!
    @function   ODNodePasswordContentCheck
    @abstract   Validates a password against the node's password content policies.
    @discussion Validates a password against the node's password content policies.
                The node's password content policies will be evaluated to
                determine if the password is acceptable.  May be used prior to
                creating the record.

                This check is only definitive at the time it was requested. The
                policy or the environment could change before the password change
                is actually requested.  Errors from the password change request
                should be consulted.

    @param      node an ODNodeRef to use.
    @param      password the password to be evaluated against the content policies.
    @param      recordName the name of the record.
    @param      error an optional CFErrorRef reference for error details.
    @result     a bool which signifies if the password passes all content policies, otherwise error is set.
 */
CF_EXPORT
bool ODNodePasswordContentCheck(ODNodeRef node, CFStringRef password, CFStringRef recordName, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_NA);

__END_DECLS

#endif /* ! __OPENDIRECTORY_CFODNODE__ */
