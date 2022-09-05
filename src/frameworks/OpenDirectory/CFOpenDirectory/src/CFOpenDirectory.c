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
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ODContextGetTypeID(void)
{
    if (verbose) puts("STUB: ODContextGetTypeID called");
    return NULL;
}

void* ODConvertToLegacyErrorCode(void)
{
    if (verbose) puts("STUB: ODConvertToLegacyErrorCode called");
    return NULL;
}

void* ODNodeAddAccountPolicy(void)
{
    if (verbose) puts("STUB: ODNodeAddAccountPolicy called");
    return NULL;
}

void* ODNodeCopyAccountPolicies(void)
{
    if (verbose) puts("STUB: ODNodeCopyAccountPolicies called");
    return NULL;
}

void* ODNodeCopyCredentials(void)
{
    if (verbose) puts("STUB: ODNodeCopyCredentials called");
    return NULL;
}

void* ODNodeCopyDetails(void)
{
    if (verbose) puts("STUB: ODNodeCopyDetails called");
    return NULL;
}

void* ODNodeCopyPolicies(void)
{
    if (verbose) puts("STUB: ODNodeCopyPolicies called");
    return NULL;
}

void* ODNodeCopyRecord(void)
{
    if (verbose) puts("STUB: ODNodeCopyRecord called");
    return NULL;
}

void* ODNodeCopyRecordAuthenticationData(void)
{
    if (verbose) puts("STUB: ODNodeCopyRecordAuthenticationData called");
    return NULL;
}

void* ODNodeCopySession(void)
{
    if (verbose) puts("STUB: ODNodeCopySession called");
    return NULL;
}

void* ODNodeCopySubnodeNames(void)
{
    if (verbose) puts("STUB: ODNodeCopySubnodeNames called");
    return NULL;
}

void* ODNodeCopySupportedAttributes(void)
{
    if (verbose) puts("STUB: ODNodeCopySupportedAttributes called");
    return NULL;
}

void* ODNodeCopySupportedPolicies(void)
{
    if (verbose) puts("STUB: ODNodeCopySupportedPolicies called");
    return NULL;
}

void* ODNodeCopySupportedRecordTypes(void)
{
    if (verbose) puts("STUB: ODNodeCopySupportedRecordTypes called");
    return NULL;
}

void* ODNodeCopyUnreachableSubnodeNames(void)
{
    if (verbose) puts("STUB: ODNodeCopyUnreachableSubnodeNames called");
    return NULL;
}

void* ODNodeCreateCopy(void)
{
    if (verbose) puts("STUB: ODNodeCreateCopy called");
    return NULL;
}

void* ODNodeCreateRecord(void)
{
    if (verbose) puts("STUB: ODNodeCreateRecord called");
    return NULL;
}

void* ODNodeCreateWithDSRef(void)
{
    if (verbose) puts("STUB: ODNodeCreateWithDSRef called");
    return NULL;
}

void* ODNodeCreateWithName(void)
{
    if (verbose) puts("STUB: ODNodeCreateWithName called");
    return NULL;
}

void* ODNodeCreateWithNameAndOptions(void)
{
    if (verbose) puts("STUB: ODNodeCreateWithNameAndOptions called");
    return NULL;
}

void* ODNodeCreateWithNodeType(void)
{
    if (verbose) puts("STUB: ODNodeCreateWithNodeType called");
    return NULL;
}

void* ODNodeCustomCall(void)
{
    if (verbose) puts("STUB: ODNodeCustomCall called");
    return NULL;
}

void* ODNodeCustomFunction(void)
{
    if (verbose) puts("STUB: ODNodeCustomFunction called");
    return NULL;
}

void* ODNodeGetDSRef(void)
{
    if (verbose) puts("STUB: ODNodeGetDSRef called");
    return NULL;
}

void* ODNodeGetName(void)
{
    if (verbose) puts("STUB: ODNodeGetName called");
    return NULL;
}

void* ODNodeGetSecureTokenRecoveryType(void)
{
    if (verbose) puts("STUB: ODNodeGetSecureTokenRecoveryType called");
    return NULL;
}

void* ODNodeGetTypeID(void)
{
    if (verbose) puts("STUB: ODNodeGetTypeID called");
    return NULL;
}

void* ODNodePasswordContentCheck(void)
{
    if (verbose) puts("STUB: ODNodePasswordContentCheck called");
    return NULL;
}

void* ODNodeRemoveAccountPolicy(void)
{
    if (verbose) puts("STUB: ODNodeRemoveAccountPolicy called");
    return NULL;
}

void* ODNodeRemovePolicy(void)
{
    if (verbose) puts("STUB: ODNodeRemovePolicy called");
    return NULL;
}

void* ODNodeReplaceUserRecord(void)
{
    if (verbose) puts("STUB: ODNodeReplaceUserRecord called");
    return NULL;
}

void* ODNodeSetAccountPolicies(void)
{
    if (verbose) puts("STUB: ODNodeSetAccountPolicies called");
    return NULL;
}

void* ODNodeSetCredentials(void)
{
    if (verbose) puts("STUB: ODNodeSetCredentials called");
    return NULL;
}

void* ODNodeSetCredentialsExtended(void)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsExtended called");
    return NULL;
}

void* ODNodeSetCredentialsUsingKerberosCache(void)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsUsingKerberosCache called");
    return NULL;
}

void* ODNodeSetCredentialsWithBootstrapToken(void)
{
    if (verbose) puts("STUB: ODNodeSetCredentialsWithBootstrapToken called");
    return NULL;
}

void* ODNodeSetPolicies(void)
{
    if (verbose) puts("STUB: ODNodeSetPolicies called");
    return NULL;
}

void* ODNodeSetPolicy(void)
{
    if (verbose) puts("STUB: ODNodeSetPolicy called");
    return NULL;
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

void* ODNodeVerifyCredentialsExtended(void)
{
    if (verbose) puts("STUB: ODNodeVerifyCredentialsExtended called");
    return NULL;
}

void* ODQueryCopyResults(void)
{
    if (verbose) puts("STUB: ODQueryCopyResults called");
    return NULL;
}

void* ODQueryCreateWithNode(void)
{
    if (verbose) puts("STUB: ODQueryCreateWithNode called");
    return NULL;
}

void* ODQueryCreateWithNodeType(void)
{
    if (verbose) puts("STUB: ODQueryCreateWithNodeType called");
    return NULL;
}

void* ODQueryGetTypeID(void)
{
    if (verbose) puts("STUB: ODQueryGetTypeID called");
    return NULL;
}

void* ODQueryScheduleWithRunLoop(void)
{
    if (verbose) puts("STUB: ODQueryScheduleWithRunLoop called");
    return NULL;
}

void* ODQuerySetCallback(void)
{
    if (verbose) puts("STUB: ODQuerySetCallback called");
    return NULL;
}

void* ODQuerySetDispatchQueue(void)
{
    if (verbose) puts("STUB: ODQuerySetDispatchQueue called");
    return NULL;
}

void* ODQuerySynchronize(void)
{
    if (verbose) puts("STUB: ODQuerySynchronize called");
    return NULL;
}

void* ODQueryUnscheduleFromRunLoop(void)
{
    if (verbose) puts("STUB: ODQueryUnscheduleFromRunLoop called");
    return NULL;
}

void* ODRecordAddAccountPolicy(void)
{
    if (verbose) puts("STUB: ODRecordAddAccountPolicy called");
    return NULL;
}

void* ODRecordAddMember(void)
{
    if (verbose) puts("STUB: ODRecordAddMember called");
    return NULL;
}

void* ODRecordAddValue(void)
{
    if (verbose) puts("STUB: ODRecordAddValue called");
    return NULL;
}

void* ODRecordAuthenticationAllowed(void)
{
    if (verbose) puts("STUB: ODRecordAuthenticationAllowed called");
    return NULL;
}

void* ODRecordChangePassword(void)
{
    if (verbose) puts("STUB: ODRecordChangePassword called");
    return NULL;
}

void* ODRecordContainsMember(void)
{
    if (verbose) puts("STUB: ODRecordContainsMember called");
    return NULL;
}

void* ODRecordContainsMemberRefresh(void)
{
    if (verbose) puts("STUB: ODRecordContainsMemberRefresh called");
    return NULL;
}

void* ODRecordCopyAccountPolicies(void)
{
    if (verbose) puts("STUB: ODRecordCopyAccountPolicies called");
    return NULL;
}

void* ODRecordCopyDetails(void)
{
    if (verbose) puts("STUB: ODRecordCopyDetails called");
    return NULL;
}

void* ODRecordCopyEffectivePolicies(void)
{
    if (verbose) puts("STUB: ODRecordCopyEffectivePolicies called");
    return NULL;
}

void* ODRecordCopyPasswordContentSummary(void)
{
    if (verbose) puts("STUB: ODRecordCopyPasswordContentSummary called");
    return NULL;
}

void* ODRecordCopyPasswordPolicy(void)
{
    if (verbose) puts("STUB: ODRecordCopyPasswordPolicy called");
    return NULL;
}

void* ODRecordCopyPolicies(void)
{
    if (verbose) puts("STUB: ODRecordCopyPolicies called");
    return NULL;
}

void* ODRecordCopySupportedPolicies(void)
{
    if (verbose) puts("STUB: ODRecordCopySupportedPolicies called");
    return NULL;
}

void* ODRecordCopyValues(void)
{
    if (verbose) puts("STUB: ODRecordCopyValues called");
    return NULL;
}

void* ODRecordDelete(void)
{
    if (verbose) puts("STUB: ODRecordDelete called");
    return NULL;
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

void* ODRecordGetRecordName(void)
{
    if (verbose) puts("STUB: ODRecordGetRecordName called");
    return NULL;
}

void* ODRecordGetRecordType(void)
{
    if (verbose) puts("STUB: ODRecordGetRecordType called");
    return NULL;
}

void* ODRecordGetTypeID(void)
{
    if (verbose) puts("STUB: ODRecordGetTypeID called");
    return NULL;
}

void* ODRecordPasswordChangeAllowed(void)
{
    if (verbose) puts("STUB: ODRecordPasswordChangeAllowed called");
    return NULL;
}

void* ODRecordRemoveAccountPolicy(void)
{
    if (verbose) puts("STUB: ODRecordRemoveAccountPolicy called");
    return NULL;
}

void* ODRecordRemoveMember(void)
{
    if (verbose) puts("STUB: ODRecordRemoveMember called");
    return NULL;
}

void* ODRecordRemovePolicy(void)
{
    if (verbose) puts("STUB: ODRecordRemovePolicy called");
    return NULL;
}

void* ODRecordRemoveValue(void)
{
    if (verbose) puts("STUB: ODRecordRemoveValue called");
    return NULL;
}

void* ODRecordSecondsUntilAuthenticationsExpire(void)
{
    if (verbose) puts("STUB: ODRecordSecondsUntilAuthenticationsExpire called");
    return NULL;
}

void* ODRecordSecondsUntilPasswordExpires(void)
{
    if (verbose) puts("STUB: ODRecordSecondsUntilPasswordExpires called");
    return NULL;
}

void* ODRecordSetAccountPolicies(void)
{
    if (verbose) puts("STUB: ODRecordSetAccountPolicies called");
    return NULL;
}

void* ODRecordSetNodeCredentials(void)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentials called");
    return NULL;
}

void* ODRecordSetNodeCredentialsExtended(void)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsExtended called");
    return NULL;
}

void* ODRecordSetNodeCredentialsUsingKerberosCache(void)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsUsingKerberosCache called");
    return NULL;
}

void* ODRecordSetNodeCredentialsWithBootstrapToken(void)
{
    if (verbose) puts("STUB: ODRecordSetNodeCredentialsWithBootstrapToken called");
    return NULL;
}

void* ODRecordSetPolicies(void)
{
    if (verbose) puts("STUB: ODRecordSetPolicies called");
    return NULL;
}

void* ODRecordSetPolicy(void)
{
    if (verbose) puts("STUB: ODRecordSetPolicy called");
    return NULL;
}

void* ODRecordSetValue(void)
{
    if (verbose) puts("STUB: ODRecordSetValue called");
    return NULL;
}

void* ODRecordSynchronize(void)
{
    if (verbose) puts("STUB: ODRecordSynchronize called");
    return NULL;
}

void* ODRecordVerifyPassword(void)
{
    if (verbose) puts("STUB: ODRecordVerifyPassword called");
    return NULL;
}

void* ODRecordVerifyPasswordExtended(void)
{
    if (verbose) puts("STUB: ODRecordVerifyPasswordExtended called");
    return NULL;
}

void* ODRecordWillAuthenticationsExpire(void)
{
    if (verbose) puts("STUB: ODRecordWillAuthenticationsExpire called");
    return NULL;
}

void* ODRecordWillPasswordExpire(void)
{
    if (verbose) puts("STUB: ODRecordWillPasswordExpire called");
    return NULL;
}

void* ODSessionCopyNodeNames(void)
{
    if (verbose) puts("STUB: ODSessionCopyNodeNames called");
    return NULL;
}

void* ODSessionCopySessionKeySupport(void)
{
    if (verbose) puts("STUB: ODSessionCopySessionKeySupport called");
    return NULL;
}

void* ODSessionCreate(void)
{
    if (verbose) puts("STUB: ODSessionCreate called");
    return NULL;
}

void* ODSessionCreateWithDSRef(void)
{
    if (verbose) puts("STUB: ODSessionCreateWithDSRef called");
    return NULL;
}

void* ODSessionGetDSRef(void)
{
    if (verbose) puts("STUB: ODSessionGetDSRef called");
    return NULL;
}

void* ODSessionGetTypeID(void)
{
    if (verbose) puts("STUB: ODSessionGetTypeID called");
    return NULL;
}

void* ODSessionIsProxy(void)
{
    if (verbose) puts("STUB: ODSessionIsProxy called");
    return NULL;
}

void* ODSessionNodeNameIsLocal(void)
{
    if (verbose) puts("STUB: ODSessionNodeNameIsLocal called");
    return NULL;
}

void* ODSessionProxyAuthorizationForm(void)
{
    if (verbose) puts("STUB: ODSessionProxyAuthorizationForm called");
    return NULL;
}

void* ODTriggerCancel(void)
{
    if (verbose) puts("STUB: ODTriggerCancel called");
    return NULL;
}

void* ODTriggerCreateForNodes(void)
{
    if (verbose) puts("STUB: ODTriggerCreateForNodes called");
    return NULL;
}

void* ODTriggerCreateForRecordAttributes(void)
{
    if (verbose) puts("STUB: ODTriggerCreateForRecordAttributes called");
    return NULL;
}

void* ODTriggerCreateForRecords(void)
{
    if (verbose) puts("STUB: ODTriggerCreateForRecords called");
    return NULL;
}

void* ODTriggerCreateForSearch(void)
{
    if (verbose) puts("STUB: ODTriggerCreateForSearch called");
    return NULL;
}

void* ODTriggerGetTypeID(void)
{
    if (verbose) puts("STUB: ODTriggerGetTypeID called");
    return NULL;
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

void* kODSessionDefault = NULL;
