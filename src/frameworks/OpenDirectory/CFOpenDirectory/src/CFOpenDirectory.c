/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "../include/generated-stubs.h"
#include <CFOpenDirectory/CFOpenDirectoryPriv.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

CFTypeID ODContextGetTypeID(void)
{
    if (verbose) puts("STUB: ODContextGetTypeID called");
    return 0;
}

tDirStatus ODConvertToLegacyErrorCode(CFIndex code)
{
    if (verbose) puts("STUB: ODConvertToLegacyErrorCode called");
    return eNotYetImplemented;
}

bool ODNodeAddAccountPolicy(ODNodeRef node, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeAddAccountPolicy called");
    return false;
}

CFDictionaryRef ODNodeCopyAccountPolicies(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyAccountPolicies called");
    return NULL;
}

bool ODNodeCopyCredentials(ODNodeRef node, ODRecordType *recordType, CFStringRef *username, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyCredentials called");
    return false;
}

CFDictionaryRef ODNodeCopyDetails(ODNodeRef node, CFArrayRef keys, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyDetails called");
    return NULL;
}

CFDictionaryRef ODNodeCopyPolicies(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyPolicies called");
    return NULL;
}

ODRecordRef ODNodeCopyRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFTypeRef attributes, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyRecord called");
    return NULL;
}

ODRecordRef ODNodeCopyRecordAuthenticationData(ODNodeRef node, ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyRecordAuthenticationData called");
    return NULL;
}

void* ODNodeCopySession(void)
{
    if (verbose) puts("STUB: ODNodeCopySession called");
    return NULL;
}

CFArrayRef ODNodeCopySubnodeNames(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopySubnodeNames called");
    return NULL;
}

CFArrayRef ODNodeCopySupportedAttributes(ODNodeRef node, ODRecordType recordType, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopySupportedAttributes called");
    return NULL;
}

CFDictionaryRef ODNodeCopySupportedPolicies(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopySupportedPolicies called");
    return NULL;
}

CFArrayRef ODNodeCopySupportedRecordTypes(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopySupportedRecordTypes called");
    return NULL;
}

CFArrayRef ODNodeCopyUnreachableSubnodeNames(ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCopyUnreachableSubnodeNames called");
    return NULL;
}

ODNodeRef ODNodeCreateCopy(CFAllocatorRef allocator, ODNodeRef node, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCreateCopy called");
    return NULL;
}

ODRecordRef ODNodeCreateRecord(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFDictionaryRef attributeDict, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCreateRecord called");
    return NULL;
}

ODNodeRef ODNodeCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, tDirNodeReference inNodeRef, bool inCloseOnRelease)
{
    if (verbose) puts("STUB: ODNodeCreateWithDSRef called");
    return NULL;
}

ODNodeRef ODNodeCreateWithName(CFAllocatorRef allocator, ODSessionRef session, CFStringRef nodeName, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCreateWithName called");
    return NULL;
}

void* ODNodeCreateWithNameAndOptions(void)
{
    if (verbose) puts("STUB: ODNodeCreateWithNameAndOptions called");
    return NULL;
}

ODNodeRef ODNodeCreateWithNodeType(CFAllocatorRef allocator, ODSessionRef session, ODNodeType nodeType, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCreateWithNodeType called");
    return NULL;
}

CFDataRef ODNodeCustomCall(ODNodeRef node, CFIndex customCode, CFDataRef data, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCustomCall called");
    return NULL;
}

CFTypeRef ODNodeCustomFunction(ODNodeRef node, CFStringRef function, CFTypeRef payload, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeCustomFunction called");
    return NULL;
}

tDirNodeReference ODNodeGetDSRef(ODNodeRef inNodeRef)
{
    if (verbose) puts("STUB: ODNodeGetDSRef called");
    return 0;
}

CFStringRef ODNodeGetName(ODNodeRef node)
{
    if (verbose) puts("STUB: ODNodeGetName called");
    return NULL;
}

void* ODNodeGetSecureTokenRecoveryType(void)
{
    if (verbose) puts("STUB: ODNodeGetSecureTokenRecoveryType called");
    return NULL;
}

CFTypeID ODNodeGetTypeID(void)
{
    if (verbose) puts("STUB: ODNodeGetTypeID called");
    return 0;
}

bool ODNodePasswordContentCheck(ODNodeRef node, CFStringRef password, CFStringRef recordName, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodePasswordContentCheck called");
    return false;
}

bool ODNodeRemoveAccountPolicy(ODNodeRef node, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeRemoveAccountPolicy called");
    return false;
}

bool ODNodeRemovePolicy(ODNodeRef node, ODPolicyType policyType, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeRemovePolicy called");
    return NULL;
}

void* ODNodeReplaceUserRecord(void)
{
    if (verbose) puts("STUB: ODNodeReplaceUserRecord called");
    return NULL;
}

bool ODNodeSetAccountPolicies(ODNodeRef node, CFDictionaryRef policies, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetAccountPolicies called");
    return NULL;
}

bool ODNodeSetCredentials(ODNodeRef node, ODRecordType recordType, CFStringRef recordName, CFStringRef password, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetCredentials called");
    return NULL;
}

bool ODNodeSetCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsExtended called");
    return NULL;
}

bool ODNodeSetCredentialsUsingKerberosCache(ODNodeRef node, CFStringRef cacheName, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsUsingKerberosCache called");
    return false;
}

void* ODNodeSetCredentialsWithBootstrapToken(void)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsWithBootstrapToken called");
    return NULL;
}

bool ODNodeSetPolicies(ODNodeRef node, CFDictionaryRef policies, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetPolicies called");
    return false;
}

bool ODNodeSetPolicy(ODNodeRef node, ODPolicyType policyType, CFTypeRef value, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeSetPolicy called");
    return false;
}

void* ODNodeSetSecureTokenRecoveryCloud(void)
{
    if (verbose) puts("STUB: ODNodeSetSecureTokenRecoveryCloud called");
    return NULL;
}

void* ODNodeSetSecureTokenRecoveryCloudUsingIdentity(void)
{
    if (verbose) puts("STUB: ODNodeSetSecureTokenRecoveryCloudUsingIdentity called");
    return NULL;
}

void* ODNodeSetSecureTokenRecoveryMasterPassword(void)
{
    if (verbose) puts("STUB: ODNodeSetSecureTokenRecoveryMasterPassword called");
    return NULL;
}

void* ODNodeSetSecureTokenRecoveryPersonal(void)
{
    if (verbose) puts("STUB: ODNodeSetSecureTokenRecoveryPersonal called");
    return NULL;
}

void* ODNodeSetSecureTokenRecoveryPersonalUsingKey(void)
{
    if (verbose) puts("STUB: ODNodeSetSecureTokenRecoveryPersonalUsingKey called");
    return NULL;
}

bool ODNodeVerifyCredentialsExtended(ODNodeRef node, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *authItemsOut, ODContextRef *context, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODNodeVerifyCredentialsExtended called");
    return false;
}

CFArrayRef ODQueryCopyResults(ODQueryRef query, bool allowPartialResults, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODQueryCopyResults called");
    return NULL;
}

ODQueryRef ODQueryCreateWithNode(CFAllocatorRef allocator, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODQueryCreateWithNode called");
    return NULL;
}

ODQueryRef ODQueryCreateWithNodeType(CFAllocatorRef allocator, ODNodeType nodeType, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODQueryCreateWithNodeType called");
    return NULL;
}

CFTypeID ODQueryGetTypeID(void)
{
    if (verbose) puts("STUB: ODQueryGetTypeID called");
    return 0;
}

void ODQueryScheduleWithRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
    if (verbose) puts("STUB: ODQueryScheduleWithRunLoop called");
}

void ODQuerySetCallback(ODQueryRef query, ODQueryCallback callback, void *userInfo)
{
    if (verbose) puts("STUB: ODQuerySetCallback called");
}

void ODQuerySetDispatchQueue(ODQueryRef query, dispatch_queue_t queue)
{
    if (verbose) puts("STUB: ODQuerySetDispatchQueue called");
}

void ODQuerySynchronize(ODQueryRef query)
{
    if (verbose) puts("STUB: ODQuerySynchronize called");
}

void ODQueryUnscheduleFromRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode)
{
    if (verbose) puts("STUB: ODQueryUnscheduleFromRunLoop called");
}

bool ODRecordAddAccountPolicy(ODRecordRef record, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordAddAccountPolicy called");
    return false;
}

bool ODRecordAddMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordAddMember called");
    return false;
}

bool ODRecordAddValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordAddValue called");
    return false;
}

bool ODRecordAuthenticationAllowed(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordAuthenticationAllowed called");
    return false;
}

bool ODRecordChangePassword(ODRecordRef record, CFStringRef oldPassword, CFStringRef newPassword, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordChangePassword called");
    return false;
}

bool ODRecordContainsMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordContainsMember called");
    return false;
}

bool ODRecordContainsMemberRefresh(ODRecordRef inGroupRef, ODRecordRef inMemberRef, CFErrorRef *outError)
{
    if (verbose) puts("STUB: ODRecordContainsMemberRefresh called");
    return false;
}

CFDictionaryRef ODRecordCopyAccountPolicies(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyAccountPolicies called");
    return NULL;
}

CFDictionaryRef ODRecordCopyDetails(ODRecordRef record, CFArrayRef attributes, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyDetails called");
    return NULL;
}

CFDictionaryRef ODRecordCopyEffectivePolicies(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyEffectivePolicies called");
    return NULL;
}

void* ODRecordCopyPasswordContentSummary(void)
{
    if (verbose) puts("STUB: ODRecordCopyPasswordContentSummary called");
    return NULL;
}

CFDictionaryRef ODRecordCopyPasswordPolicy(CFAllocatorRef allocator, ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyPasswordPolicy called");
    return NULL;
}

CFDictionaryRef ODRecordCopyPolicies(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyPolicies called");
    return NULL;
}

CFDictionaryRef ODRecordCopySupportedPolicies(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopySupportedPolicies called");
    return NULL;
}

CFArrayRef ODRecordCopyValues(ODRecordRef record, ODAttributeType attribute, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordCopyValues called");
    return NULL;
}

bool ODRecordDelete(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordDelete called");
    return false;
}

void* ODRecordEnumerateMembers(void)
{
    if (verbose) puts("STUB: ODRecordEnumerateMembers called");
    return NULL;
}

void* ODRecordEnumerateMembership(void)
{
    if (verbose) puts("STUB: ODRecordEnumerateMembership called");
    return NULL;
}

CFStringRef ODRecordGetRecordName(ODRecordRef record)
{
    if (verbose) puts("STUB: ODRecordGetRecordName called");
    return NULL;
}

CFStringRef ODRecordGetRecordType(ODRecordRef record)
{
    if (verbose) puts("STUB: ODRecordGetRecordType called");
    return NULL;
}

CFTypeID ODRecordGetTypeID(void)
{
    if (verbose) puts("STUB: ODRecordGetTypeID called");
    return 0;
}

bool ODRecordPasswordChangeAllowed(ODRecordRef record, CFStringRef newPassword, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordPasswordChangeAllowed called");
    return false;
}
bool ODRecordRemoveAccountPolicy(ODRecordRef record, CFDictionaryRef policy, ODPolicyCategoryType category, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordRemoveAccountPolicy called");
    return false;
}

bool ODRecordRemoveMember(ODRecordRef group, ODRecordRef member, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordRemoveMember called");
    return false;
}

bool ODRecordRemovePolicy(ODRecordRef record, ODPolicyType policy, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordRemovePolicy called");
    return false;
}

bool ODRecordRemoveValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef value, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordRemoveValue called");
    return false;
}

int64_t ODRecordSecondsUntilAuthenticationsExpire(ODRecordRef record)
{
    if (verbose) puts("STUB: ODRecordSecondsUntilAuthenticationsExpire called");
    return 0;
}

int64_t ODRecordSecondsUntilPasswordExpires(ODRecordRef record)
{
    if (verbose) puts("STUB: ODRecordSecondsUntilPasswordExpires called");
    return 0;
}

bool ODRecordSetAccountPolicies(ODRecordRef record, CFDictionaryRef policies, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetAccountPolicies called");
    return false;
}

bool ODRecordSetNodeCredentials(ODRecordRef record, CFStringRef username, CFStringRef password, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentials called");
    return false;
}

bool ODRecordSetNodeCredentialsExtended(ODRecordRef record, ODRecordType recordType, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsExtended called");
    return false;
}

bool ODRecordSetNodeCredentialsUsingKerberosCache(ODRecordRef record, CFStringRef cacheName, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsUsingKerberosCache called");
    return false;
}

bool ODRecordSetNodeCredentialsWithBootstrapToken(ODRecordRef cfRecord, CFStringRef token, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsWithBootstrapToken called");
    return false;
}

bool ODRecordSetPolicies(ODRecordRef record, CFDictionaryRef policies, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetPolicies called");
    return false;
}

bool ODRecordSetPolicy(ODRecordRef record, ODPolicyType policy, CFTypeRef value, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetPolicy called");
    return false;
}

bool ODRecordSetValue(ODRecordRef record, ODAttributeType attribute, CFTypeRef valueOrValues, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSetValue called");
    return false;
}

bool ODRecordSynchronize(ODRecordRef record, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordSynchronize called");
    return false;
}

bool ODRecordVerifyPassword(ODRecordRef record, CFStringRef password, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordVerifyPassword called");
    return false;
}

bool ODRecordVerifyPasswordExtended(ODRecordRef record, ODAuthenticationType authType, CFArrayRef authItems, CFArrayRef *outAuthItems, ODContextRef *outContext, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODRecordVerifyPasswordExtended called");
    return false;
}

bool ODRecordWillAuthenticationsExpire(ODRecordRef record, uint64_t willExpireIn)
{
    if (verbose) puts("STUB: ODRecordWillAuthenticationsExpire called");
    return false;
}

bool ODRecordWillPasswordExpire(ODRecordRef record, uint64_t willExpireIn)
{
    if (verbose) puts("STUB: ODRecordWillPasswordExpire called");
    return false;
}

CFArrayRef ODSessionCopyNodeNames(CFAllocatorRef allocator, ODSessionRef session, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODSessionCopyNodeNames called");
    return NULL;
}

void* ODSessionCopySessionKeySupport(void)
{
    if (verbose) puts("STUB: ODSessionCopySessionKeySupport called");
    return NULL;
}

ODSessionRef ODSessionCreate(CFAllocatorRef allocator, CFDictionaryRef options, CFErrorRef *error)
{
    if (verbose) puts("STUB: ODSessionCreate called");
    return NULL;
}

ODSessionRef ODSessionCreateWithDSRef(CFAllocatorRef inAllocator, tDirReference inDirRef, bool inCloseOnRelease)
{
    if (verbose) puts("STUB: ODSessionCreateWithDSRef called");
    return NULL;
}

tDirReference ODSessionGetDSRef(ODSessionRef inSessionRef) 
{
    if (verbose) puts("STUB: ODSessionGetDSRef called");
    return 0;
}

CFTypeID ODSessionGetTypeID(void)
{
    if (verbose) puts("STUB: ODSessionGetTypeID called");
    return 0;
}

void* ODSessionIsProxy(void)
{
    if (verbose) puts("STUB: ODSessionIsProxy called");
    return NULL;
}

bool ODSessionNodeNameIsLocal(ODSessionRef session, CFStringRef nodename)
{
    if (verbose) puts("STUB: ODSessionNodeNameIsLocal called");
    return false;
}

void* ODSessionProxyAuthorizationForm(void)
{
    if (verbose) puts("STUB: ODSessionProxyAuthorizationForm called");
    return NULL;
}

void ODTriggerCancel(ODTriggerRef trigger)
{
    if (verbose) puts("STUB: ODTriggerCancel called");
}

ODTriggerRef ODTriggerCreateForNodes(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
{
    if (verbose) puts("STUB: ODTriggerCreateForNodes called");
    return NULL;
}

ODTriggerRef ODTriggerCreateForRecordAttributes(CFAllocatorRef allocator, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames, CFTypeRef attributes,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef rec, CFStringRef attr))
{
    if (verbose) puts("STUB: ODTriggerCreateForRecordAttributes called");
    return NULL;
}

ODTriggerRef ODTriggerCreateForRecords(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef name))
{
    if (verbose) puts("STUB: ODTriggerCreateForRecords called");
    return NULL;
}

ODTriggerRef ODTriggerCreateForSearch(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
{
    if (verbose) puts("STUB: ODTriggerCreateForSearch called");
    return NULL;
}

CFTypeID ODTriggerGetTypeID(void)
{
    if (verbose) puts("STUB: ODTriggerGetTypeID called");
    return 0;
}

void* _NodeGetNodeTypeName(void)
{
    if (verbose) puts("STUB: _NodeGetNodeTypeName called");
    return NULL;
}

void* _ODInitialize(void)
{
    if (verbose) puts("STUB: _ODInitialize called");
    return NULL;
}

void* _ODNodeCreate(void)
{
    if (verbose) puts("STUB: _ODNodeCreate called");
    return NULL;
}

void* _ODNodeInit(void)
{
    if (verbose) puts("STUB: _ODNodeInit called");
    return NULL;
}

void* _ODQueryCreate(void)
{
    if (verbose) puts("STUB: _ODQueryCreate called");
    return NULL;
}

void* _ODQueryGetDelegate(void)
{
    if (verbose) puts("STUB: _ODQueryGetDelegate called");
    return NULL;
}

void* _ODQueryGetOperationQueue(void)
{
    if (verbose) puts("STUB: _ODQueryGetOperationQueue called");
    return NULL;
}

void* _ODQueryInit(void)
{
    if (verbose) puts("STUB: _ODQueryInit called");
    return NULL;
}

void* _ODQuerySetDelegate(void)
{
    if (verbose) puts("STUB: _ODQuerySetDelegate called");
    return NULL;
}

void* _ODQuerySetOperationQueue(void)
{
    if (verbose) puts("STUB: _ODQuerySetOperationQueue called");
    return NULL;
}

void* _ODSessionCreate(void)
{
    if (verbose) puts("STUB: _ODSessionCreate called");
    return NULL;
}

void* _ODSessionGetShared(void)
{
    if (verbose) puts("STUB: _ODSessionGetShared called");
    return NULL;
}

void* _ODSessionInit(void)
{
    if (verbose) puts("STUB: _ODSessionInit called");
    return NULL;
}

ODSessionRef kODSessionDefault = NULL;
