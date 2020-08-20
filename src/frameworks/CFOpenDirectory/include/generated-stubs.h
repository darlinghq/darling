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


#ifndef _CFOpenDirectory_H_
#define _CFOpenDirectory_H_

void* ODContextGetTypeID(void);
void* ODConvertToLegacyErrorCode(void);
void* ODNodeAddAccountPolicy(void);
void* ODNodeCopyAccountPolicies(void);
void* ODNodeCopyCredentials(void);
void* ODNodeCopyDetails(void);
void* ODNodeCopyPolicies(void);
void* ODNodeCopyRecord(void);
void* ODNodeCopyRecordAuthenticationData(void);
void* ODNodeCopySession(void);
void* ODNodeCopySubnodeNames(void);
void* ODNodeCopySupportedAttributes(void);
void* ODNodeCopySupportedPolicies(void);
void* ODNodeCopySupportedRecordTypes(void);
void* ODNodeCopyUnreachableSubnodeNames(void);
void* ODNodeCreateCopy(void);
void* ODNodeCreateRecord(void);
void* ODNodeCreateWithDSRef(void);
void* ODNodeCreateWithName(void);
void* ODNodeCreateWithNameAndOptions(void);
void* ODNodeCreateWithNodeType(void);
void* ODNodeCustomCall(void);
void* ODNodeCustomFunction(void);
void* ODNodeGetDSRef(void);
void* ODNodeGetName(void);
void* ODNodeGetSecureTokenRecoveryType(void);
void* ODNodeGetTypeID(void);
void* ODNodePasswordContentCheck(void);
void* ODNodeRemoveAccountPolicy(void);
void* ODNodeRemovePolicy(void);
void* ODNodeReplaceUserRecord(void);
void* ODNodeSetAccountPolicies(void);
void* ODNodeSetCredentials(void);
void* ODNodeSetCredentialsExtended(void);
void* ODNodeSetCredentialsUsingKerberosCache(void);
void* ODNodeSetCredentialsWithBootstrapToken(void);
void* ODNodeSetPolicies(void);
void* ODNodeSetPolicy(void);
void* ODNodeSetSecureTokenRecoveryCloud(void);
void* ODNodeSetSecureTokenRecoveryCloudUsingIdentity(void);
void* ODNodeSetSecureTokenRecoveryMasterPassword(void);
void* ODNodeSetSecureTokenRecoveryPersonal(void);
void* ODNodeSetSecureTokenRecoveryPersonalUsingKey(void);
void* ODNodeVerifyCredentialsExtended(void);
void* ODQueryCopyResults(void);
void* ODQueryCreateWithNode(void);
void* ODQueryCreateWithNodeType(void);
void* ODQueryGetTypeID(void);
void* ODQueryScheduleWithRunLoop(void);
void* ODQuerySetCallback(void);
void* ODQuerySetDispatchQueue(void);
void* ODQuerySynchronize(void);
void* ODQueryUnscheduleFromRunLoop(void);
void* ODRecordAddAccountPolicy(void);
void* ODRecordAddMember(void);
void* ODRecordAddValue(void);
void* ODRecordAuthenticationAllowed(void);
void* ODRecordChangePassword(void);
void* ODRecordContainsMember(void);
void* ODRecordContainsMemberRefresh(void);
void* ODRecordCopyAccountPolicies(void);
void* ODRecordCopyDetails(void);
void* ODRecordCopyEffectivePolicies(void);
void* ODRecordCopyPasswordContentSummary(void);
void* ODRecordCopyPasswordPolicy(void);
void* ODRecordCopyPolicies(void);
void* ODRecordCopySupportedPolicies(void);
void* ODRecordCopyValues(void);
void* ODRecordDelete(void);
void* ODRecordEnumerateMembers(void);
void* ODRecordEnumerateMembership(void);
void* ODRecordGetRecordName(void);
void* ODRecordGetRecordType(void);
void* ODRecordGetTypeID(void);
void* ODRecordPasswordChangeAllowed(void);
void* ODRecordRemoveAccountPolicy(void);
void* ODRecordRemoveMember(void);
void* ODRecordRemovePolicy(void);
void* ODRecordRemoveValue(void);
void* ODRecordSecondsUntilAuthenticationsExpire(void);
void* ODRecordSecondsUntilPasswordExpires(void);
void* ODRecordSetAccountPolicies(void);
void* ODRecordSetNodeCredentials(void);
void* ODRecordSetNodeCredentialsExtended(void);
void* ODRecordSetNodeCredentialsUsingKerberosCache(void);
void* ODRecordSetNodeCredentialsWithBootstrapToken(void);
void* ODRecordSetPolicies(void);
void* ODRecordSetPolicy(void);
void* ODRecordSetValue(void);
void* ODRecordSynchronize(void);
void* ODRecordVerifyPassword(void);
void* ODRecordVerifyPasswordExtended(void);
void* ODRecordWillAuthenticationsExpire(void);
void* ODRecordWillPasswordExpire(void);
void* ODSessionCopyNodeNames(void);
void* ODSessionCopySessionKeySupport(void);
void* ODSessionCreate(void);
void* ODSessionCreateWithDSRef(void);
void* ODSessionGetDSRef(void);
void* ODSessionGetTypeID(void);
void* ODSessionIsProxy(void);
void* ODSessionNodeNameIsLocal(void);
void* ODSessionProxyAuthorizationForm(void);
void* ODTriggerCancel(void);
void* ODTriggerCreateForNodes(void);
void* ODTriggerCreateForRecordAttributes(void);
void* ODTriggerCreateForRecords(void);
void* ODTriggerCreateForSearch(void);
void* ODTriggerGetTypeID(void);
void* _NodeGetNodeTypeName(void);
void* _ODInitialize(void);
void* _ODNodeCreate(void);
void* _ODNodeInit(void);
void* _ODQueryCreate(void);
void* _ODQueryGetDelegate(void);
void* _ODQueryGetOperationQueue(void);
void* _ODQueryInit(void);
void* _ODQuerySetDelegate(void);
void* _ODQuerySetOperationQueue(void);
void* _ODSessionCreate(void);
void* _ODSessionGetShared(void);
void* _ODSessionInit(void);

#endif
