project(Accounts)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(Accounts
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/Accounts.m
        src/ACDManagedAccount.m
        src/ACDPropertyEncryption.m
        src/ACManagedDefaults.m
        src/ACProtobufVariableValue.m
        src/ACDManagedAccountType.m
        src/ACAccountStore.m
        src/ACAccount.m
        src/ACAccountType.m
        src/ACPersonaManager.m
        src/ACOAuthSigner.m
        src/ACProtobufVariableKeyValuePair.m
        src/ACAccountCredential.m
        src/ACProtobufDataclassAction.m
        src/ACRemoteAccountStoreSession.m
        src/ACAccountStoreClientSideListener.m
        src/ACSimpleRateLimiter.m
        src/ACRateLimiter.m
        src/ACDManagedAuthorization.m
        src/ACDManagedAccessOptionsKey.m
        src/ACProtobufVariableValueList.m
        src/ACProtobufAccount.m
        src/ACSystemConfigManager.m
        src/ACNotificationRebroadcaster.m
        src/ACDManagedDataclass.m
        src/ACProtobufCredentialItem.m
        src/ACRemoteAccountStoreInterface.m
        src/ACProtobufKeyValuePair.m
        src/ACDataclassAction.m
        src/ACDManagedCredentialItem.m
        src/ACProtobufVariableValueDictionary.m
        src/ACProtobufAccountType.m
        src/ACProtobufDate.m
        src/ACProtobufURL.m
        src/ACDManagedAccountProperty.m
        src/ACProtobufAccountCredential.m
        src/ACCredentialItem.m
        src/ACDispatchCerberus.m
        src/ACTrackedSet.m
        src/ACMutableTrackedSet.m
        src/ACZeroingString.m
        src/ACAccountMigrationLock.m
        src/ACProtobufUUID.m

    DEPENDENCIES
        system
        objc
        Foundation
)
