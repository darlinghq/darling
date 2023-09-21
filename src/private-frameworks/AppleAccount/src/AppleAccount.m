/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <AppleAccount/AppleAccount.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* const AAAbsintheSignerDefaultTimeout = (void*)0;
extern void* const AAAccountClassBasic = (void*)0;
extern void* const AAAccountClassFull = (void*)0;
extern void* const AAAccountClassPrimary = (void*)0;
extern void* const AAAccountConfigChangedNotification = (void*)0;
extern void* const AAAccountUserNotificationMachService = (void*)0;
extern void* const AAAppProvidedContextKey = (void*)0;
extern void* const AAAttestationSignerKeychainLabel = (void*)0;
extern void* const AAAttestationSignerValidity = (void*)0;
extern void* const AACloudKitMigrationStateDone = (void*)0;
extern void* const AACloudKitMigrationStateFailed = (void*)0;
extern void* const AACloudKitMigrationStateMigrating = (void*)0;
extern void* const AACloudKitMigrationStateNone = (void*)0;
extern void* const AACloudKitMigrationStateNotNeeded = (void*)0;
extern void* const AACloudKitMigrationStateStart = (void*)0;
extern void* const AACustodianErrorDomain = (void*)0;
extern void* const AACustodianMachService = (void*)0;
extern void* const AACustodianRecoveryErrorDomain = (void*)0;
extern void* const AADaemonMachService = (void*)0;
extern void* const AADataclassActionErrorDomain = (void*)0;
extern void* const AADataclassActionUserInfoLibrarySize = (void*)0;
extern void* const AADataclassActionsKey = (void*)0;
extern void* const AADataclassesKey = (void*)0;
extern void* const AADefaultGrandSlamTokenHeaderKey = (void*)0;
extern void* const AAErrorDomain = (void*)0;
extern void* const AAFamilyEligibilityKey = (void*)0;
extern void* const AAFamilyGrandSlamTokenHeaderKey = (void*)0;
extern void* AAFollowUpActionDismiss = (void*)0;
extern void* AAFollowUpActionPrimary = (void*)0;
extern void* AAFollowUpClientIdentifier = (void*)0;
extern void* AAFollowUpExtensionIdentifier = (void*)0;
extern void* AAFollowUpIdentifierBeneficiaryIneligible = (void*)0;
extern void* AAFollowUpIdentifierBeneficiaryRemoved = (void*)0;
extern void* AAFollowUpIdentifierCustodianEmbargo = (void*)0;
extern void* AAFollowUpIdentifierCustodianInvitationReminder = (void*)0;
extern void* AAFollowUpIdentifierCustodianRemoved = (void*)0;
extern void* AAFollowUpIdentifierCustodianWalrusNoLiveness = (void*)0;
extern void* AAFollowUpIdentifierCustodianWalrusRemoved = (void*)0;
extern void* AAFollowUpIdentifierRenewCredentials = (void*)0;
extern void* AAFollowUpIdentifierStartUsing = (void*)0;
extern void* AAFollowUpIdentifierUserInfoKey = (void*)0;
extern void* AAFollowUpIdentifierVerifyTerms = (void*)0;
extern void* AAFollowUpIdentifierVerifyTermsV2 = (void*)0;
extern void* AAFollowUpIdentifierWalrusUserRecoveryFactorsMissing = (void*)0;
extern void* const AAFollowUpMacOptionsButtonTitle = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionAddBeneficiary = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionAddCustodian = (void*)0;
extern void* const AAFollowUpOpenPrefPaneActionAddRecoveryKey = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionBeneficiaryList = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionCustodianRecovery = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionKey = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionRenewAuth = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionShowAppleIDLogin = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionShowDeviceList = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionShowInitialPage = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionShowTransparencyPane = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionShowTrustedContacts = (void*)0;
extern void* AAFollowUpOpenPrefPaneActionTerms = (void*)0;
extern void* const AAFollowUpOpenPrefPaneActionWalrusAddRecoveryContact = (void*)0;
extern void* AAFollowUpOpenPrefPaneURL = (void*)0;
extern void* AAFollowUpTrustedContactFirstName = (void*)0;
extern void* AAFollowUpTrustedContactHandle = (void*)0;
extern void* AAFollowUpTrustedContactID = (void*)0;
extern void* AAFollowUpTrustedContactInvitationURL = (void*)0;
extern void* AAFollowUpTrustedContactName = (void*)0;
extern void* AAFollowUpUserInfoAccountIdentifier = (void*)0;
extern void* AAFollowUpUserInfoAltDSID = (void*)0;
extern void* AAFollowUpUserInfoCFUType = (void*)0;
extern void* AAFollowUpUserInfoClientName = (void*)0;
extern void* AAFollowUpUserInfoCustodianSessionID = (void*)0;
extern void* AAFollowUpUserInfoNotificationAltButtonTitle = (void*)0;
extern void* AAFollowUpUserInfoNotificationBody = (void*)0;
extern void* AAFollowUpUserInfoNotificationButtonTitle = (void*)0;
extern void* AAFollowUpUserInfoNotificationDelay = (void*)0;
extern void* AAFollowUpUserInfoNotificationFrequency = (void*)0;
extern void* AAFollowUpUserInfoNotificationInfo = (void*)0;
extern void* AAFollowUpUserInfoNotificationTimeLimit = (void*)0;
extern void* AAFollowUpUserInfoNotificationTitle = (void*)0;
extern void* AAFollowUpUserInfoProxiedDeviceData = (void*)0;
extern void* AAFollowUpUserInfoTermsCFUBody = (void*)0;
extern void* AAFollowUpUserInfoTermsCFUTitle = (void*)0;
extern void* AAFollowUpUserInfoTermsDependentAltDSID = (void*)0;
extern void* AAFollowUpUserInfoTermsFamilyAltDSIDValue = (void*)0;
extern void* AAFollowUpUserInfoTermsNotificationBody = (void*)0;
extern void* AAFollowUpUserInfoTermsNotificationTitle = (void*)0;
extern void* AAFollowUpUserInfoTermsServerProvidedContext = (void*)0;
extern void* const AAGrandSlamAppTokenIDAppleIDSettings = (void*)0;
extern void* const AAGrandSlamAppTokenIDFamilySharing = (void*)0;
extern void* const AAGrandSlamAppTokenIDiCloudSettings = (void*)0;
extern void* const AAHTTPHeaderDeviceUDID = (void*)0;
extern void* const AAHTTPHeaderMachineID = (void*)0;
extern void* const AAHTTPHeaderOneTimePassword = (void*)0;
extern void* const AAHTTPHeaderXAppleMDAction = (void*)0;
extern void* const AAHTTPHeaderXAppleMDData = (void*)0;
extern void* const AAIDSRegistrationPropertySupportsLegacyContact = (void*)0;
extern void* const AAInheritanceAccessKeyKey = (void*)0;
extern void* const AAInheritanceBenefactorAltDSIDKey = (void*)0;
extern void* const AAInheritanceBeneficiaryIDKey = (void*)0;
extern void* AAInheritanceConfigDurationBeforeNotSetupCFUKey = (void*)0;
extern void* AAInheritanceConfigStaleInviteDurationKey = (void*)0;
extern void* const AAInheritanceContactChangedNotification = (void*)0;
extern void* const AAInheritanceContactHandleKey = (void*)0;
extern void* const AAInheritanceErrorDomain = (void*)0;
extern void* const AAInheritanceMachService = (void*)0;
extern void* const AAInheritanceSetupHelpURL = (void*)0;
extern void* const AALoginDelegatesAppleIDKey = (void*)0;
extern void* const AALoginDelegatesDSIDKey = (void*)0;
extern void* const AALoginDelegatesDelegatesKey = (void*)0;
extern void* const AALoginDelegatesPasswordKey = (void*)0;
extern void* const AALoginDelegatesResponseParametersKey = (void*)0;
extern void* const AALoginDelegatesServiceDataKey = (void*)0;
extern void* const AALoginDelegatesStatusKey = (void*)0;
extern void* const AALoginDelegatesStatusMessageKey = (void*)0;
extern void* const AAMessagesInviteContextActionLabelKey = (void*)0;
extern void* const AAMessagesInviteContextInviteURLKey = (void*)0;
extern void* const AAMessagesInviteContextInviteesKey = (void*)0;
extern void* const AAMessagesInviteContextMessageBodyKey = (void*)0;
extern void* const AAMessagesInviteContextResponseSubtitleKey = (void*)0;
extern void* const AAMessagesInviteContextSubtitleKey = (void*)0;
extern void* const AAMessagesInviteContextTitleKey = (void*)0;
extern void* const AAMessagingErrorDomain = (void*)0;
extern void* const AAOBKProtocolPropertyBubbleTextKey = (void*)0;
extern void* const AAOBKProtocolPropertyCustodianshipInfo = (void*)0;
extern void* const AAOBKProtocolPropertyDetailTextKey = (void*)0;
extern void* const AAOBKProtocolPropertyHelpLinkTitleKey = (void*)0;
extern void* const AAOBKProtocolPropertyHelpLinkURLKey = (void*)0;
extern void* const AAOBKProtocolPropertyImageNameKey = (void*)0;
extern void* const AAOBKProtocolPropertyOwnerHandleKey = (void*)0;
extern void* const AAOBKProtocolPropertyPrimaryButtonKey = (void*)0;
extern void* const AAOBKProtocolPropertyRecipientHandleKey = (void*)0;
extern void* const AAOBKProtocolPropertySecondaryButtonKey = (void*)0;
extern void* const AAOBKProtocolPropertyTitleKey = (void*)0;
extern void* const AAPartialErrorsByNameKey = (void*)0;
extern void* const AAPrefsConfigurationDomain = (void*)0;
extern void* const AAPrefsConfigurationPlistURL = (void*)0;
extern void* const AAPrefsDomain = (void*)0;
extern void* const AAPrefsLocalPlistPath = (void*)0;
extern void* const AAPrefsUseLocalConfigPlist = (void*)0;
extern void* const AARecoveryContactChangedNotification = (void*)0;
extern void* const AAServiceTypeGameCenter = (void*)0;
extern void* AASetupAssistantAboutURLKey = (void*)0;
extern void* AASetupAssistantActiveEmailDomainKey = (void*)0;
extern void* const AASetupAssistantAppleIDKey = (void*)0;
extern void* AASetupAssistantAuthenticateURLKey = (void*)0;
extern void* const AASetupAssistantBirthdayKey = (void*)0;
extern void* AASetupAssistantCheckValidityURLKey = (void*)0;
extern void* const AASetupAssistantClientIDKey = (void*)0;
extern void* const AASetupAssistantCountryKey = (void*)0;
extern void* AASetupAssistantCreateAppleIDKey = (void*)0;
extern void* AASetupAssistantCreateAppleIDURLKey = (void*)0;
extern void* AASetupAssistantCreateDelegateAccountsURLKey = (void*)0;
extern void* AASetupAssistantCreateStatusKey = (void*)0;
extern void* AASetupAssistantCreateStatusMessageKey = (void*)0;
extern void* AASetupAssistantDSIDKey = (void*)0;
extern void* const AASetupAssistantDelegatesKey = (void*)0;
extern void* const AASetupAssistantEmailChoiceKey = (void*)0;
extern void* AASetupAssistantExistingAppleIDTermsUIURLKey = (void*)0;
extern void* const AASetupAssistantFirstNameKey = (void*)0;
extern void* AASetupAssistantIForgotURLKey = (void*)0;
extern void* const AASetupAssistantLanguageKey = (void*)0;
extern void* const AASetupAssistantLastNameKey = (void*)0;
extern void* AASetupAssistantLoginDelegatesURLKey = (void*)0;
extern void* const AASetupAssistantPasswordKey = (void*)0;
extern void* AASetupAssistantResponseParametersKey = (void*)0;
extern void* const AASetupAssistantSecurityAnswerKey = (void*)0;
extern void* const AASetupAssistantSecurityQuestionKey = (void*)0;
extern void* AASetupAssistantServerEnabledKey = (void*)0;
extern void* const AASetupAssistantTimezoneKey = (void*)0;
extern void* AASetupAssistantUpdateAppleIDURLKey = (void*)0;
extern void* AASetupAssistantUpgradeStatusKey = (void*)0;
extern void* AASetupAssistantUpgradeStatusURLKey = (void*)0;
extern void* AASetupAssistantUpgradeiOSTermsUIURLKey = (void*)0;
extern void* AASetupAssistantXMLUIKey = (void*)0;
extern void* AASetupAssistantiForgotUIURLKey = (void*)0;
extern void* const AASignInErrorDomain = (void*)0;
extern void* AASpyGlassOptionDependentAccount = (void*)0;
extern void* AASpyGlassOptionPrimaryAccount = (void*)0;
extern void* const AATermsEntryBeta = (void*)0;
extern void* const AATermsEntryDevice = (void*)0;
extern void* const AATermsEntryGameCenter = (void*)0;
extern void* const AATermsEntryPrivacy = (void*)0;
extern void* const AATermsEntryWarranty = (void*)0;
extern void* const AATermsEntryiCloud = (void*)0;
extern void* const AATermsEntryiTunes = (void*)0;
extern void* const AAURLEndpointAbout = (void*)0;
extern void* const AAURLEndpointAuthenticate = (void*)0;
extern void* const AAURLEndpointCreateDelegateAccounts = (void*)0;
extern void* const AAURLEndpointGetAccountSettings = (void*)0;
extern void* const AAURLEndpointLoginDelegates = (void*)0;
extern void* const AAURLEndpointLoginOrCreateAccount = (void*)0;
extern void* const AAURLEndpointQualifyCertificate = (void*)0;
extern void* const AAURLEndpointQualifySession = (void*)0;
extern void* const AAURLKeyCustomDomainState = (void*)0;
extern void* const AAURLKeyFlowType = (void*)0;
extern void* const AAURLKeyFlowTypeBeneficiary = (void*)0;
extern void* const AAURLKeyFlowTypeCustodian = (void*)0;
extern void* const AAURLKeyFlowTypeCustomDomain = (void*)0;
extern void* const AAURLKeyFlowTypeMailManage = (void*)0;
extern void* const AAURLKeyFlowTypePrivateEmailManage = (void*)0;
extern void* const AAURLKeyID = (void*)0;
extern void* const AAURLProtocolScheme = (void*)0;
extern void* const IDSRegistrationPropertySupportsCustodian = (void*)0;
extern void* const IDSRegistrationPropertyUnknown = (void*)0;
extern void* _AABeneficiaryClaimUIURLKey = (void*)0;
extern void* const _AAIDSServiceName = (void*)0;
extern void* const _AAMessageDestinationMailToPrefix = (void*)0;
extern void* const _AAMessageDestinationSelfTokenPrefix = (void*)0;
extern void* const _AAMessageDestinationTelToPrefix = (void*)0;
extern void* const _AAMessageDestinationTokenPrefix = (void*)0;
extern void* const _AAMessagesInviteContextURL = (void*)0;
extern void* const kAAAccountAlternateDSID = (void*)0;
extern void* const kAAAccountAppProvidedContextKey = (void*)0;
extern void* const kAAAccountAppleIDAliasesKey = (void*)0;
extern void* const kAAAccountAppleIDKey = (void*)0;
extern void* const kAAAccountBenefactorAccountKey = (void*)0;
extern void* const kAAAccountClassKey = (void*)0;
extern void* const kAAAccountCloudDocsMigrationComplete = (void*)0;
extern void* const kAAAccountDataclass = (void*)0;
extern void* const kAAAccountDataclassesDisabledForCellularKey = (void*)0;
extern void* const kAAAccountDataclassesPropertiesKey = (void*)0;
extern void* const kAAAccountEmailAddress = (void*)0;
extern void* const kAAAccountFirstNameKey = (void*)0;
extern void* const kAAAccountHasOptionalTerms = (void*)0;
extern void* const kAAAccountIsCloudSubscriber = (void*)0;
extern void* const kAAAccountIsFamilyEligible = (void*)0;
extern void* const kAAAccountIsManagedAppleID = (void*)0;
extern void* const kAAAccountIsSandboxAccount = (void*)0;
extern void* const kAAAccountLastAgreedTerms = (void*)0;
extern void* const kAAAccountLastKnownQuota = (void*)0;
extern void* const kAAAccountLastNameKey = (void*)0;
extern void* const kAAAccountMiddleNameKey = (void*)0;
extern void* const kAAAccountMobileMeStatusKey = (void*)0;
extern void* const kAAAccountNeedsToVerifyTerms = (void*)0;
extern void* const kAAAccountNotesMigrated = (void*)0;
extern void* const kAAAccountPasswordKey = (void*)0;
extern void* const kAAAccountPersonIDKey = (void*)0;
extern void* const kAAAccountPrimaryAccountKey = (void*)0;
extern void* const kAAAccountPrimaryAccountUpdaterVersion = (void*)0;
extern void* const kAAAccountPrimaryEmailKey = (void*)0;
extern void* const kAAAccountPrimaryEmailVerifiedKey = (void*)0;
extern void* const kAAAccountProtocolVersionKey = (void*)0;
extern void* const kAAAccountRegionInfoKey = (void*)0;
extern void* const kAAAccountRemindersAutoMigratableToCK = (void*)0;
extern void* const kAAAccountRemindersMigrated = (void*)0;
extern void* const kAAAccountRepairStateKey = (void*)0;
extern void* const kAAAccountRepairerPID = (void*)0;
extern void* const kAAAccountSyncedAccount = (void*)0;
extern void* const kAAAccountUbiquityPropertyiCloudHome = (void*)0;
extern void* const kAAAccountUsesCloudDocs = (void*)0;
extern void* const kAAAccountWarmingUp = (void*)0;
extern void* const kAAActionNameKey = (void*)0;
extern void* const kAAAltDSIDKey = (void*)0;
extern void* const kAACountryCodeKey = (void*)0;
extern void* const kAADeviceProvisioningMachineId = (void*)0;
extern void* const kAADeviceProvisioningOneTimePassword = (void*)0;
extern void* const kAAErrorDetailsActionButtonTitleKey = (void*)0;
extern void* const kAAErrorDetailsActionButtonTypeKey = (void*)0;
extern void* const kAAErrorDetailsActionButtonURLKey = (void*)0;
extern void* const kAAErrorDetailsDismissButtonTitleKey = (void*)0;
extern void* const kAAErrorDetailsMIDKey = (void*)0;
extern void* const kAAErrorDetailsMessageKey = (void*)0;
extern void* const kAAErrorDetailsOTPKey = (void*)0;
extern void* const kAAErrorDetailsProxiedDeviceDataKey = (void*)0;
extern void* const kAAErrorDetailsRoutingInfoKey = (void*)0;
extern void* const kAAErrorDetailsTitleKey = (void*)0;
extern void* const kAAMessagingServiceName = (void*)0;
extern void* const kAAMetadatanKey = (void*)0;
extern void* const kAAProcotolNewAccountUIURLKey = (void*)0;
extern void* const kAAProtocoEmailLookupURLKey = (void*)0;
extern void* const kAAProtocolAPSEnvKey = (void*)0;
extern void* const kAAProtocolAboutURLKey = (void*)0;
extern void* const kAAProtocolAcceptChildTransferURLKey = (void*)0;
extern void* const kAAProtocolAcceptFamilyInviteURLKey = (void*)0;
extern void* const kAAProtocolAcceptFamilyInviteV2URLKey = (void*)0;
extern void* const kAAProtocolAccountBetaKey = (void*)0;
extern void* const kAAProtocolAccountFirstDisplayAlertKey = (void*)0;
extern void* const kAAProtocolAccountFooterButtonKey = (void*)0;
extern void* const kAAProtocolAccountFooterTextKey = (void*)0;
extern void* const kAAProtocolAccountManagementUIURLKey = (void*)0;
extern void* const kAAProtocolAccountServiceTypeKey = (void*)0;
extern void* const kAAProtocolAccountUnavailableKey = (void*)0;
extern void* const kAAProtocolAddEmailURLKey = (void*)0;
extern void* const kAAProtocolAddFamilyMemberUIURLKey = (void*)0;
extern void* const kAAProtocolAltDSIDKey = (void*)0;
extern void* const kAAProtocolAppleAccountInfoKey = (void*)0;
extern void* const kAAProtocolAppleIDAliasesKey = (void*)0;
extern void* const kAAProtocolAppleIDEnteredByUserKey = (void*)0;
extern void* const kAAProtocolAppleIDKey = (void*)0;
extern void* const kAAProtocolAppleIDToken = (void*)0;
extern void* const kAAProtocolAuthTokenKey = (void*)0;
extern void* const kAAProtocolAuthenticateURLKey = (void*)0;
extern void* const kAAProtocolCheckiCloudMembershipURLKey = (void*)0;
extern void* const kAAProtocolChildAccountCreationUIURLKey = (void*)0;
extern void* const kAAProtocolClientIDKey = (void*)0;
extern void* const kAAProtocolCloudKitDevicesList = (void*)0;
extern void* const kAAProtocolCloudKitMigrationStateURLKey = (void*)0;
extern void* const kAAProtocolCloudKitStartMigrationURLKey = (void*)0;
extern void* const kAAProtocolCloudKitTokenKey = (void*)0;
extern void* const kAAProtocolCompleteValidateEmailURLKey = (void*)0;
extern void* const kAAProtocolCreateDelegatesURLKey = (void*)0;
extern void* const kAAProtocolDSIDKey = (void*)0;
extern void* const kAAProtocolDeclineFamilyInviteURLKey = (void*)0;
extern void* const kAAProtocolDelegatesKey = (void*)0;
extern void* const kAAProtocolDeviceHardwareModelKey = (void*)0;
extern void* const kAAProtocolDeviceInfoKey = (void*)0;
extern void* const kAAProtocolDeviceInfoSignatureKey = (void*)0;
extern void* const kAAProtocolDeviceListURLKey = (void*)0;
extern void* const kAAProtocolDeviceOSVersionKey = (void*)0;
extern void* const kAAProtocolDeviceQualificationURLKey = (void*)0;
extern void* const kAAProtocolDeviceRegionCode = (void*)0;
extern void* const kAAProtocolDeviceSerialNumberKey = (void*)0;
extern void* const kAAProtocolDeviceUDIDHeader = (void*)0;
extern void* const kAAProtocolDevicesUIURLKey = (void*)0;
extern void* const kAAProtocolDismissAlertURLKey = (void*)0;
extern void* const kAAProtocolDisplayedHostnameKey = (void*)0;
extern void* const kAAProtocolDomainURLsKey = (void*)0;
extern void* const kAAProtocolEmailToVet = (void*)0;
extern void* const kAAProtocolEmailVettingToken = (void*)0;
extern void* const kAAProtocolEnvironmentsKey = (void*)0;
extern void* const kAAProtocolErrorCancelButtonTitleKey = (void*)0;
extern void* const kAAProtocolErrorCommunicationsError = (void*)0;
extern void* const kAAProtocolErrorDescriptionKey = (void*)0;
extern void* const kAAProtocolErrorInvalidPassword = (void*)0;
extern void* const kAAProtocolErrorKey = (void*)0;
extern void* const kAAProtocolErrorMessageKey = (void*)0;
extern void* const kAAProtocolErrorMobileMeAccountDisabled = (void*)0;
extern void* const kAAProtocolErrorMobileMeAccountLapsed = (void*)0;
extern void* const kAAProtocolErrorMobileMeAccountMissingInfo = (void*)0;
extern void* const kAAProtocolErrorMobileMeCreateUnavailable = (void*)0;
extern void* const kAAProtocolErrorMobileMeInvalidSignature = (void*)0;
extern void* const kAAProtocolErrorMobileMeRegionDenied = (void*)0;
extern void* const kAAProtocolErrorMobileMeSuspendedMode = (void*)0;
extern void* const kAAProtocolErrorMobileMeTermsOfServiceUpdate = (void*)0;
extern void* const kAAProtocolErrorMobileMeUnsupportedDevice = (void*)0;
extern void* const kAAProtocolErrorResponseDictionaryKey = (void*)0;
extern void* const kAAProtocolErrorScheduledMaintenence = (void*)0;
extern void* const kAAProtocolErrorTitleKey = (void*)0;
extern void* const kAAProtocolExistingAppleIDTermsUIURLKey = (void*)0;
extern void* const kAAProtocolExperimentalModeHeaderKey = (void*)0;
extern void* const kAAProtocolFMFAppTokenKey = (void*)0;
extern void* const kAAProtocolFMFTokenKey = (void*)0;
extern void* const kAAProtocolFMIPAppTokenKey = (void*)0;
extern void* const kAAProtocolFMIPAuthenticateURLKey = (void*)0;
extern void* const kAAProtocolFMIPLostModeTokenKey = (void*)0;
extern void* const kAAProtocolFMIPSiriTokenKey = (void*)0;
extern void* const kAAProtocolFMIPTokenKey = (void*)0;
extern void* const kAAProtocolFMIPUrl = (void*)0;
extern void* const kAAProtocolFMIPWipeTokenKey = (void*)0;
extern void* const kAAProtocolFamilyEligibilityURLKey = (void*)0;
extern void* const kAAProtocolFamilyInviteSentV2URLKey = (void*)0;
extern void* const kAAProtocolFamilyLearnMoreURLKey = (void*)0;
extern void* const kAAProtocolFamilyPaymentCardUIURLKey = (void*)0;
extern void* const kAAProtocolFamilyPaymentInfoURLKey = (void*)0;
extern void* const kAAProtocolFamilySetupUIURLKey = (void*)0;
extern void* const kAAProtocolFamilyUIURLKey = (void*)0;
extern void* const kAAProtocolFetchFamilyInviteV2URLKey = (void*)0;
extern void* const kAAProtocolFinishProvisioningMachineKey = (void*)0;
extern void* const kAAProtocolFirstNameKey = (void*)0;
extern void* const kAAProtocolGameCenterDataKey = (void*)0;
extern void* const kAAProtocolGenericTermsUIBuddyMLKey = (void*)0;
extern void* const kAAProtocolGenericTermsUIPlistBuddyMLKey = (void*)0;
extern void* const kAAProtocolGenericTermsUIPlistTextKey = (void*)0;
extern void* const kAAProtocolGenericTermsURLKey = (void*)0;
extern void* const kAAProtocolGetAccountEventDetailsURLKey = (void*)0;
extern void* const kAAProtocolGetAccountSettingsURLKey = (void*)0;
extern void* const kAAProtocolGetDeviceListURLKey = (void*)0;
extern void* const kAAProtocolGetFamilyDetailsURLKey = (void*)0;
extern void* const kAAProtocolGetFamilyMemberPhotoURLKey = (void*)0;
extern void* const kAAProtocolGetFamilyPushDetailsURLKey = (void*)0;
extern void* const kAAProtocolGetMyPhotoURLKey = (void*)0;
extern void* const kAAProtocolGrandslamURLKey = (void*)0;
extern void* const kAAProtocolHasOptionalTermsKey = (void*)0;
extern void* const kAAProtocolIdentityServicesDataKey = (void*)0;
extern void* const kAAProtocolInitiateFamilyV2URLKey = (void*)0;
extern void* const kAAProtocolInitiateValidateEmailURLKey = (void*)0;
extern void* const kAAProtocolIsCloudDocsMigratedKey = (void*)0;
extern void* const kAAProtocolIsFamilyEligibleKey = (void*)0;
extern void* const kAAProtocolIsManagedAppleIDKey = (void*)0;
extern void* const kAAProtocolIsSandboxAccountKey = (void*)0;
extern void* const kAAProtocolKeyTransparencyTokenKey = (void*)0;
extern void* const kAAProtocolLanguageKey = (void*)0;
extern void* const kAAProtocolLastNameKey = (void*)0;
extern void* const kAAProtocolLoginAccountURLKey = (void*)0;
extern void* const kAAProtocolLoginDelegatesURLKey = (void*)0;
extern void* const kAAProtocolLoginOrCreateDelegatesURLKey = (void*)0;
extern void* const kAAProtocolLoginOrCreateURLKey = (void*)0;
extern void* const kAAProtocolMDMServerTokenKey = (void*)0;
extern void* const kAAProtocolMapsTokenKey = (void*)0;
extern void* const kAAProtocolMemberDetailsUIURLKey = (void*)0;
extern void* const kAAProtocolMobileMeDataKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferAlertURLKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferClassicMessageKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferClassicTitleKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferIdentifierKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferMessageKey = (void*)0;
extern void* const kAAProtocolMobileMeOfferTitleKey = (void*)0;
extern void* const kAAProtocolMobileMeSetupTokenKey = (void*)0;
extern void* const kAAProtocolNewAccountURLKey = (void*)0;
extern void* const kAAProtocolNonQualifiedRegisterURLKey = (void*)0;
extern void* const kAAProtocolNotesMigratedKey = (void*)0;
extern void* const kAAProtocolPasswordKey = (void*)0;
extern void* const kAAProtocolPasswordSecurityUIURLKey = (void*)0;
extern void* const kAAProtocolPaymentSummaryUIURLKey = (void*)0;
extern void* const kAAProtocolPaymentUIURLKey = (void*)0;
extern void* const kAAProtocolPendingFamilyInvitesUIURLKey = (void*)0;
extern void* const kAAProtocolPersonIDKey = (void*)0;
extern void* const kAAProtocolPersonIDToken = (void*)0;
extern void* const kAAProtocolPersonalInfoUIURLKey = (void*)0;
extern void* const kAAProtocolPrefCustomHealthCheckIntervalEnabled = (void*)0;
extern void* const kAAProtocolPrefCustomHealthCheckIntervalMinutes = (void*)0;
extern void* const kAAProtocolPrefCustomHealthCheckReachabilityIntervalMinutes = (void*)0;
extern void* const kAAProtocolPrefCustomHealthCheckVersion = (void*)0;
extern void* const kAAProtocolPrefDisableHealthCheckTTRKey = (void*)0;
extern void* const kAAProtocolPrefExperimentalModeKey = (void*)0;
extern void* const kAAProtocolPrefMultipleFullAccounts = (void*)0;
extern void* const kAAProtocolPrefNeverSkipCustodianCheckKey = (void*)0;
extern void* const kAAProtocolPrefSendInternalDevHeaderKey = (void*)0;
extern void* const kAAProtocolPrefShowAccountContactsKey = (void*)0;
extern void* const kAAProtocolPrefSimulateUnhealthyCustodianKey = (void*)0;
extern void* const kAAProtocolPrefSuppressFireKey = (void*)0;
extern void* const kAAProtocolPrefUseLegacyLoginEndpoint = (void*)0;
extern void* const kAAProtocolPrimaryEmailKey = (void*)0;
extern void* const kAAProtocolPrimaryEmailVerifiedKey = (void*)0;
extern void* const kAAProtocolProvisionedDataclassesKey = (void*)0;
extern void* const kAAProtocolRegionInfoKey = (void*)0;
extern void* const kAAProtocolRegisterDeviceURLKey = (void*)0;
extern void* const kAAProtocolRegisterURLKey = (void*)0;
extern void* const kAAProtocolRejectChildTransferURLKey = (void*)0;
extern void* const kAAProtocolRemindersAutoMigratableToCKKey = (void*)0;
extern void* const kAAProtocolRemindersMigratedKey = (void*)0;
extern void* const kAAProtocolRepairCloudAccountUIURLKey = (void*)0;
extern void* const kAAProtocolSearchPartyTokenKey = (void*)0;
extern void* const kAAProtocolSecondaryAuthURLKey = (void*)0;
extern void* const kAAProtocolSendCodeURLKey = (void*)0;
extern void* const kAAProtocolServerResponseStatus = (void*)0;
extern void* const kAAProtocolServerResponseStatusMessage = (void*)0;
extern void* const kAAProtocolServiceDataKey = (void*)0;
extern void* const kAAProtocolSigningSessionCertificateURLKey = (void*)0;
extern void* const kAAProtocolSigningSessionURLKey = (void*)0;
extern void* const kAAProtocolStartFamilyInviteV2URLKey = (void*)0;
extern void* const kAAProtocolSuspendedInfoKey = (void*)0;
extern void* const kAAProtocolSuspensionFamilyKey = (void*)0;
extern void* const kAAProtocolSuspensioniCloudKey = (void*)0;
extern void* const kAAProtocolSyncMachineKey = (void*)0;
extern void* const kAAProtocolTermsReportUserActionURLKey = (void*)0;
extern void* const kAAProtocolTimezoneKey = (void*)0;
extern void* const kAAProtocolTokensKey = (void*)0;
extern void* const kAAProtocolUDIDKey = (void*)0;
extern void* const kAAProtocolUDIDToken = (void*)0;
extern void* const kAAProtocolURLsKey = (void*)0;
extern void* const kAAProtocolUnregisterDeviceURLKey = (void*)0;
extern void* const kAAProtocolUpdateAccountUIURLKey = (void*)0;
extern void* const kAAProtocolUpdateAccountURLKey = (void*)0;
extern void* const kAAProtocolUpdateMyPhotoURLKey = (void*)0;
extern void* const kAAProtocolUpdateNameURLKey = (void*)0;
extern void* const kAAProtocolUserInfoKey = (void*)0;
extern void* const kAAProtocolValidateURLKey = (void*)0;
extern void* const kAAProtocolVerifyCodeURLKey = (void*)0;
extern void* const kAAProtocolVersionKey = (void*)0;
extern void* const kAAProtocoliCloudAccountKey = (void*)0;
extern void* const kAAProtocoliCloudDataclassPrefix = (void*)0;
extern void* const kAAProtocoliForgotUIURLKey = (void*)0;
extern void* const kAASaveOptionAnisetteMIDKey = (void*)0;
extern void* const kAASaveOptionAnisetteOTPKey = (void*)0;
extern void* const kAASaveOptionAnisetteRoutingKey = (void*)0;
extern void* const kAASaveOptionCompanionDeviceClientInfoKey = (void*)0;
extern void* const kAASaveOptionCompanionDeviceUDIDKey = (void*)0;
extern void* const kAASaveOptionsTwoFactorLoginCookiesKey = (void*)0;
extern void* const kAASendWalrusWebAccessPCSAuthKey = (void*)0;
extern void* const kAATermsAgreeUrlKey = (void*)0;
extern void* const kAATermsFieldFormat = (void*)0;
extern void* const kAATermsFieldName = (void*)0;
extern void* const kAATermsFieldTerms = (void*)0;
extern void* const kAATermsVersionKey = (void*)0;
extern void* const kAATrueString = (void*)0;
extern void* const kAAiCloudVersionKey = (void*)0;
extern void* const kAppleIDMigratedAccountIdentifier = (void*)0;
extern void* const kLegacyAAAccountAppleIDAliasesKey = (void*)0;
extern void* const kLegacyAAAccountAuthTokenKey = (void*)0;
extern void* const kLegacyAAAccountDataclass = (void*)0;
extern void* const kLegacyAAAccountDataclassesDisabledForCellularKey = (void*)0;
extern void* const kLegacyAAAccountDataclassesPropertiesKey = (void*)0;
extern void* const kLegacyAAAccountFMIPTokenKey = (void*)0;
extern void* const kLegacyAAAccountFirstNameKey = (void*)0;
extern void* const kLegacyAAAccountLastNameKey = (void*)0;
extern void* const kLegacyAAAccountMobileMeStatusKey = (void*)0;
extern void* const kLegacyAAAccountPersonIDKey = (void*)0;
extern void* const kLegacyAAAccountPrimaryAccountKey = (void*)0;
extern void* const kLegacyAAAccountPrimaryEmailKey = (void*)0;
extern void* const kLegacyAAAccountPrimaryEmailVerifiedKey = (void*)0;
extern void* const kLegacyAAAccountProtocolVersionKey = (void*)0;

void *AALocalizedStringForKey(void) {
    if (verbose) puts("STUB: AALocalizedStringForKey called");
    return NULL;
}

void *AALocalizedStringForKeyInTable(void) {
    if (verbose) puts("STUB: AALocalizedStringForKeyInTable called");
    return NULL;
}

void *AALocalizedStringForServiceType(void) {
    if (verbose) puts("STUB: AALocalizedStringForServiceType called");
    return NULL;
}

void *AKTrustedContactStatusFromAATrustedContactStatus(void) {
    if (verbose) puts("STUB: AKTrustedContactStatusFromAATrustedContactStatus called");
    return NULL;
}

void *_AAErrorUnderlyingError(void) {
    if (verbose) puts("STUB: _AAErrorUnderlyingError called");
    return NULL;
}

void *_AALogSystem(void) {
    if (verbose) puts("STUB: _AALogSystem called");
    return NULL;
}

void *_AASignpostCreate(void) {
    if (verbose) puts("STUB: _AASignpostCreate called");
    return NULL;
}

void *_AASignpostGetNanoseconds(void) {
    if (verbose) puts("STUB: _AASignpostGetNanoseconds called");
    return NULL;
}

void *_AASignpostLogSystem(void) {
    if (verbose) puts("STUB: _AASignpostLogSystem called");
    return NULL;
}

