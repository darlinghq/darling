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


#ifndef _Accounts_H_
#define _Accounts_H_

#import <Foundation/Foundation.h>

#import <Accounts/ACAccountStoreProtocol.h>
#import <Accounts/ACDOAuthSignerProtocol.h>
#import <Accounts/ACProtobufCoding.h>
#import <Accounts/ACRemoteAccountStoreProtocol.h>
#import <Accounts/NSXPCProxyCreating.h>
#import <Accounts/UMUserPersonaUpdateObserver.h>
#import <Accounts/ACDManagedAccount.h>
#import <Accounts/ACDPropertyEncryption.h>
#import <Accounts/ACManagedDefaults.h>
#import <Accounts/ACProtobufVariableValue.h>
#import <Accounts/ACDManagedAccountType.h>
#import <Accounts/ACAccountStore.h>
#import <Accounts/ACAccount.h>
#import <Accounts/ACAccountType.h>
#import <Accounts/ACPersonaManager.h>
#import <Accounts/ACOAuthSigner.h>
#import <Accounts/ACProtobufVariableKeyValuePair.h>
#import <Accounts/ACAccountCredential.h>
#import <Accounts/ACProtobufDataclassAction.h>
#import <Accounts/ACRemoteAccountStoreSession.h>
#import <Accounts/ACAccountStoreClientSideListener.h>
#import <Accounts/ACSimpleRateLimiter.h>
#import <Accounts/ACRateLimiter.h>
#import <Accounts/ACDManagedAuthorization.h>
#import <Accounts/ACDManagedAccessOptionsKey.h>
#import <Accounts/ACProtobufVariableValueList.h>
#import <Accounts/ACProtobufAccount.h>
#import <Accounts/ACSystemConfigManager.h>
#import <Accounts/ACNotificationRebroadcaster.h>
#import <Accounts/ACDManagedDataclass.h>
#import <Accounts/ACProtobufCredentialItem.h>
#import <Accounts/ACRemoteAccountStoreInterface.h>
#import <Accounts/ACProtobufKeyValuePair.h>
#import <Accounts/ACDataclassAction.h>
#import <Accounts/ACDManagedCredentialItem.h>
#import <Accounts/ACProtobufVariableValueDictionary.h>
#import <Accounts/ACProtobufAccountType.h>
#import <Accounts/ACProtobufDate.h>
#import <Accounts/ACProtobufURL.h>
#import <Accounts/ACDManagedAccountProperty.h>
#import <Accounts/ACProtobufAccountCredential.h>
#import <Accounts/ACCredentialItem.h>
#import <Accounts/ACDispatchCerberus.h>
#import <Accounts/ACTrackedSet.h>
#import <Accounts/ACMutableTrackedSet.h>
#import <Accounts/ACZeroingString.h>
#import <Accounts/ACAccountMigrationLock.h>
#import <Accounts/ACProtobufUUID.h>

void* ACAccountTypeIdentifierForASSAccountType(void);
void* ACDGetAdamOrDisplayIDForPID(void);
void* ACDGetStoreOrBundleIDForPID(void);
void* ACDGetTeamIDForBundleURL(void);
void* ACDataclassForASSDataclass(void);
void* _ACLogSystem(void);
void* _ACPLogSystem(void);
void* _ACSignpostCreate(void);
void* _ACSignpostGetNanoseconds(void);
void* _ACSignpostLogSystem(void);
void* createACAccountWithASAccountProperties(void);
void* createACAccountWithASAccountPropertiesAndACAccountType(void);
void* serializeSecCertificates(void);
void* serializeSecTrust(void);
void* unserializeSecCertificates(void);
void* unserializeSecTrust(void);

#endif
