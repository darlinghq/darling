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


#ifndef _TrustedPeers_H_
#define _TrustedPeers_H_

#import <Foundation/Foundation.h>

#import <TrustedPeers/TPKeyPair.h>
#import <TrustedPeers/TPPublicKey.h>
#import <TrustedPeers/TPPublicKeyFactory.h>
#import <TrustedPeers/TPCategoryRule.h>
#import <TrustedPeers/TPPeerDynamicInfo.h>
#import <TrustedPeers/TPPolicy.h>
#import <TrustedPeers/TPPBDictionaryMatchingRuleFieldExists.h>
#import <TrustedPeers/TPModel.h>
#import <TrustedPeers/TPPolicyDocument.h>
#import <TrustedPeers/TPMachineID.h>
#import <TrustedPeers/TPMachineIDList.h>
#import <TrustedPeers/TPHashBuilder.h>
#import <TrustedPeers/TPPBDictionaryMatchingRuleFieldRegexMatch.h>
#import <TrustedPeers/TPPBDictionaryMatchingRule.h>
#import <TrustedPeers/TPPeerStableInfo.h>
#import <TrustedPeers/TPECPublicKeyFactory.h>
#import <TrustedPeers/TPPeerPermanentInfo.h>
#import <TrustedPeers/TPPBPolicyKeyViewMapping.h>
#import <TrustedPeers/TPRecoveryKeyPair.h>
#import <TrustedPeers/TPCachedViableBottles.h>
#import <TrustedPeers/TPPolicyVersion.h>
#import <TrustedPeers/TPPeer.h>
#import <TrustedPeers/TPPBDispositionDuplicateMachineID.h>
#import <TrustedPeers/TPVoucher.h>
#import <TrustedPeers/TPPBDispositionEntry.h>
#import <TrustedPeers/TPPBDispositionDisallowedMachineID.h>
#import <TrustedPeers/TPPBAncientEpoch.h>
#import <TrustedPeers/TPPBPolicyProhibits.h>
#import <TrustedPeers/TPPBUnknownMachineID.h>
#import <TrustedPeers/TPPBDisposition.h>
#import <TrustedPeers/TPPBPeerPermanentInfo.h>
#import <TrustedPeers/TPDictionaryMatchingRule.h>
#import <TrustedPeers/TPPBPeerDynamicInfo.h>
#import <TrustedPeers/TPPBPolicySecret.h>
#import <TrustedPeers/TPPBPeerStableInfo.h>
#import <TrustedPeers/TPPBVoucher.h>
#import <TrustedPeers/TPPBPolicyDocument.h>
#import <TrustedPeers/TPPBPolicyCategoriesByView.h>
#import <TrustedPeers/TPPBPolicyIntroducersByCategory.h>
#import <TrustedPeers/TPStringTable.h>
#import <TrustedPeers/TPPBPolicyModelToCategory.h>
#import <TrustedPeers/TPPBPolicyRedaction.h>

void* TPMachineIDStatusToString(void);
void* TPModelLog(void);
void* TPPBAncientEpochReadFrom(void);
void* TPPBDictionaryMatchingRuleFieldExistsReadFrom(void);
void* TPPBDictionaryMatchingRuleFieldRegexMatchReadFrom(void);
void* TPPBDictionaryMatchingRuleReadFrom(void);
void* TPPBDispositionDisallowedMachineIDReadFrom(void);
void* TPPBDispositionDuplicateMachineIDReadFrom(void);
void* TPPBDispositionEntryReadFrom(void);
void* TPPBDispositionReadFrom(void);
void* TPPBPeerDynamicInfoReadFrom(void);
void* TPPBPeerPermanentInfoReadFrom(void);
void* TPPBPeerStableInfoReadFrom(void);
void* TPPBPolicyCategoriesByViewReadFrom(void);
void* TPPBPolicyDocumentReadFrom(void);
void* TPPBPolicyIntroducersByCategoryReadFrom(void);
void* TPPBPolicyKeyViewMappingReadFrom(void);
void* TPPBPolicyModelToCategoryReadFrom(void);
void* TPPBPolicyProhibitsReadFrom(void);
void* TPPBPolicyRedactionReadFrom(void);
void* TPPBPolicySecretReadFrom(void);
void* TPPBUnknownMachineIDReadFrom(void);
void* TPPBVoucherReadFrom(void);
void* TPPeerStatusToString(void);
void* checkTypesafeSignature(void);
void* typesafeSignature(void);

#endif
