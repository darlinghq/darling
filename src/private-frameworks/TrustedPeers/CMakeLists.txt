project(TrustedPeers)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(TrustedPeers
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

    SOURCES
        src/TrustedPeers.m
        src/TPCategoryRule.m
        src/TPPeerDynamicInfo.m
        src/TPPolicy.m
        src/TPPBDictionaryMatchingRuleFieldExists.m
        src/TPModel.m
        src/TPPolicyDocument.m
        src/TPMachineID.m
        src/TPMachineIDList.m
        src/TPHashBuilder.m
        src/TPPBDictionaryMatchingRuleFieldRegexMatch.m
        src/TPPBDictionaryMatchingRule.m
        src/TPPeerStableInfo.m
        src/TPECPublicKeyFactory.m
        src/TPPeerPermanentInfo.m
        src/TPPBPolicyKeyViewMapping.m
        src/TPRecoveryKeyPair.m
        src/TPCachedViableBottles.m
        src/TPPolicyVersion.m
        src/TPPeer.m
        src/TPPBDispositionDuplicateMachineID.m
        src/TPVoucher.m
        src/TPPBDispositionEntry.m
        src/TPPBDispositionDisallowedMachineID.m
        src/TPPBAncientEpoch.m
        src/TPPBPolicyProhibits.m
        src/TPPBUnknownMachineID.m
        src/TPPBDisposition.m
        src/TPPBPeerPermanentInfo.m
        src/TPDictionaryMatchingRule.m
        src/TPPBPeerDynamicInfo.m
        src/TPPBPolicySecret.m
        src/TPPBPeerStableInfo.m
        src/TPPBVoucher.m
        src/TPPBPolicyDocument.m
        src/TPPBPolicyCategoriesByView.m
        src/TPPBPolicyIntroducersByCategory.m
        src/TPStringTable.m
        src/TPPBPolicyModelToCategory.m
        src/TPPBPolicyRedaction.m

    DEPENDENCIES
        system
        objc
        Foundation
)
