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

void* ODNodeCopySession(void);
void* ODNodeCreateWithNameAndOptions(void);
void* ODNodeGetSecureTokenRecoveryType(void);
void* ODNodeReplaceUserRecord(void);
void* ODNodeSetCredentialsWithBootstrapToken(void);
void* ODNodeSetSecureTokenRecoveryCloud(void);
void* ODNodeSetSecureTokenRecoveryCloudUsingIdentity(void);
void* ODNodeSetSecureTokenRecoveryMasterPassword(void);
void* ODNodeSetSecureTokenRecoveryPersonal(void);
void* ODNodeSetSecureTokenRecoveryPersonalUsingKey(void);
void* ODRecordCopyPasswordContentSummary(void);
void* ODRecordEnumerateMembers(void);
void* ODRecordEnumerateMembership(void);
void* ODSessionCopySessionKeySupport(void);
void* ODSessionIsProxy(void);
void* ODSessionProxyAuthorizationForm(void);
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
void* _ODSessionGetShared(void);
void* _ODSessionInit(void);

#endif
