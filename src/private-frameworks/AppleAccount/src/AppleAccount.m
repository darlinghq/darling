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


void* const AAAbsintheSignerDefaultTimeout = (void*)0;
void* const AAAccountClassBasic = (void*)0;
void* const AAAccountClassFull = (void*)0;
void* const AAAccountClassPrimary = (void*)0;
void* const AAAccountConfigChangedNotification = (void*)0;
void* const AAAccountUserNotificationMachService = (void*)0;
void* const AAAppProvidedContextKey = (void*)0;
void* const AAAttestationSignerKeychainLabel = (void*)0;
void* const AAAttestationSignerValidity = (void*)0;
void* const AACloudKitMigrationStateDone = (void*)0;
void* const AACloudKitMigrationStateFailed = (void*)0;
void* const AACloudKitMigrationStateMigrating = (void*)0;
void* const AACloudKitMigrationStateNone = (void*)0;
void* const AACloudKitMigrationStateNotNeeded = (void*)0;
void* const AACloudKitMigrationStateStart = (void*)0;
void* const AACustodianErrorDomain = (void*)0;
void* const AACustodianMachService = (void*)0;
void* const AACustodianRecoveryErrorDomain = (void*)0;
void* const AADaemonMachService = (void*)0;
void* const AADataclassActionErrorDomain = (void*)0;
void* const AADataclassActionUserInfoLibrarySize = (void*)0;
void* const AADataclassActionsKey = (void*)0;
void* const AADataclassesKey = (void*)0;
void* const AADefaultGrandSlamTokenHeaderKey = (void*)0;
void* const AAErrorDomain = (void*)0;
void* const AAFamilyEligibilityKey = (void*)0;
void* const AAFamilyGrandSlamTokenHeaderKey = (void*)0;
void* AAFollowUpActionDismiss = (void*)0;
void* AAFollowUpActionPrimary = (void*)0;
void* AAFollowUpClientIdentifier = (void*)0;
void* AAFollowUpExtensionIdentifier = (void*)0;
void* AAFollowUpIdentifierBeneficiaryIneligible = (void*)0;
void* AAFollowUpIdentifierBeneficiaryRemoved = (void*)0;
void* AAFollowUpIdentifierCustodianEmbargo = (void*)0;
void* AAFollowUpIdentifierCustodianInvitationReminder = (void*)0;
void* AAFollowUpIdentifierCustodianRemoved = (void*)0;
void* AAFollowUpIdentifierCustodianWalrusNoLiveness = (void*)0;
void* AAFollowUpIdentifierCustodianWalrusRemoved = (void*)0;
void* AAFollowUpIdentifierRenewCredentials = (void*)0;
void* AAFollowUpIdentifierStartUsing = (void*)0;
void* AAFollowUpIdentifierUserInfoKey = (void*)0;
void* AAFollowUpIdentifierVerifyTerms = (void*)0;
void* AAFollowUpIdentifierVerifyTermsV2 = (void*)0;
void* AAFollowUpIdentifierWalrusUserRecoveryFactorsMissing = (void*)0;
void* const AAFollowUpMacOptionsButtonTitle = (void*)0;
void* AAFollowUpOpenPrefPaneActionAddBeneficiary = (void*)0;
void* AAFollowUpOpenPrefPaneActionAddCustodian = (void*)0;
void* const AAFollowUpOpenPrefPaneActionAddRecoveryKey = (void*)0;
void* AAFollowUpOpenPrefPaneActionBeneficiaryList = (void*)0;
void* AAFollowUpOpenPrefPaneActionCustodianRecovery = (void*)0;
void* AAFollowUpOpenPrefPaneActionKey = (void*)0;
void* AAFollowUpOpenPrefPaneActionRenewAuth = (void*)0;
void* AAFollowUpOpenPrefPaneActionShowAppleIDLogin = (void*)0;
void* AAFollowUpOpenPrefPaneActionShowDeviceList = (void*)0;
void* AAFollowUpOpenPrefPaneActionShowInitialPage = (void*)0;
void* AAFollowUpOpenPrefPaneActionShowTransparencyPane = (void*)0;
void* AAFollowUpOpenPrefPaneActionShowTrustedContacts = (void*)0;
void* AAFollowUpOpenPrefPaneActionTerms = (void*)0;
void* const AAFollowUpOpenPrefPaneActionWalrusAddRecoveryContact = (void*)0;
void* AAFollowUpOpenPrefPaneURL = (void*)0;
void* AAFollowUpTrustedContactFirstName = (void*)0;
void* AAFollowUpTrustedContactHandle = (void*)0;
void* AAFollowUpTrustedContactID = (void*)0;
void* AAFollowUpTrustedContactInvitationURL = (void*)0;
void* AAFollowUpTrustedContactName = (void*)0;
void* AAFollowUpUserInfoAccountIdentifier = (void*)0;
void* AAFollowUpUserInfoAltDSID = (void*)0;
void* AAFollowUpUserInfoCFUType = (void*)0;
void* AAFollowUpUserInfoClientName = (void*)0;
void* AAFollowUpUserInfoCustodianSessionID = (void*)0;
void* AAFollowUpUserInfoNotificationAltButtonTitle = (void*)0;
void* AAFollowUpUserInfoNotificationBody = (void*)0;
void* AAFollowUpUserInfoNotificationButtonTitle = (void*)0;
void* AAFollowUpUserInfoNotificationDelay = (void*)0;
void* AAFollowUpUserInfoNotificationFrequency = (void*)0;
void* AAFollowUpUserInfoNotificationInfo = (void*)0;
void* AAFollowUpUserInfoNotificationTimeLimit = (void*)0;
void* AAFollowUpUserInfoNotificationTitle = (void*)0;
void* AAFollowUpUserInfoProxiedDeviceData = (void*)0;
void* AAFollowUpUserInfoTermsCFUBody = (void*)0;
void* AAFollowUpUserInfoTermsCFUTitle = (void*)0;
void* AAFollowUpUserInfoTermsDependentAltDSID = (void*)0;
void* AAFollowUpUserInfoTermsFamilyAltDSIDValue = (void*)0;
void* AAFollowUpUserInfoTermsNotificationBody = (void*)0;
void* AAFollowUpUserInfoTermsNotificationTitle = (void*)0;
void* AAFollowUpUserInfoTermsServerProvidedContext = (void*)0;
void* const AAGrandSlamAppTokenIDAppleIDSettings = (void*)0;
void* const AAGrandSlamAppTokenIDFamilySharing = (void*)0;
void* const AAGrandSlamAppTokenIDiCloudSettings = (void*)0;
void* const AAHTTPHeaderDeviceUDID = (void*)0;
void* const AAHTTPHeaderMachineID = (void*)0;
void* const AAHTTPHeaderOneTimePassword = (void*)0;
void* const AAHTTPHeaderXAppleMDAction = (void*)0;
void* const AAHTTPHeaderXAppleMDData = (void*)0;
void* const AAIDSRegistrationPropertySupportsLegacyContact = (void*)0;
void* const AAInheritanceAccessKeyKey = (void*)0;
void* const AAInheritanceBenefactorAltDSIDKey = (void*)0;
void* const AAInheritanceBeneficiaryIDKey = (void*)0;
void* AAInheritanceConfigDurationBeforeNotSetupCFUKey = (void*)0;
void* AAInheritanceConfigStaleInviteDurationKey = (void*)0;
void* const AAInheritanceContactChangedNotification = (void*)0;
void* const AAInheritanceContactHandleKey = (void*)0;
void* const AAInheritanceErrorDomain = (void*)0;
void* const AAInheritanceMachService = (void*)0;
void* const AAInheritanceSetupHelpURL = (void*)0;
void* const AALoginDelegatesAppleIDKey = (void*)0;
void* const AALoginDelegatesDSIDKey = (void*)0;
void* const AALoginDelegatesDelegatesKey = (void*)0;
void* const AALoginDelegatesPasswordKey = (void*)0;
void* const AALoginDelegatesResponseParametersKey = (void*)0;
void* const AALoginDelegatesServiceDataKey = (void*)0;
void* const AALoginDelegatesStatusKey = (void*)0;
void* const AALoginDelegatesStatusMessageKey = (void*)0;
void* const AAMessagesInviteContextActionLabelKey = (void*)0;
void* const AAMessagesInviteContextInviteURLKey = (void*)0;
void* const AAMessagesInviteContextInviteesKey = (void*)0;
void* const AAMessagesInviteContextMessageBodyKey = (void*)0;
void* const AAMessagesInviteContextResponseSubtitleKey = (void*)0;
void* const AAMessagesInviteContextSubtitleKey = (void*)0;
void* const AAMessagesInviteContextTitleKey = (void*)0;
void* const AAMessagingErrorDomain = (void*)0;
void* const AAOBKProtocolPropertyBubbleTextKey = (void*)0;
void* const AAOBKProtocolPropertyCustodianshipInfo = (void*)0;
void* const AAOBKProtocolPropertyDetailTextKey = (void*)0;
void* const AAOBKProtocolPropertyHelpLinkTitleKey = (void*)0;
void* const AAOBKProtocolPropertyHelpLinkURLKey = (void*)0;
void* const AAOBKProtocolPropertyImageNameKey = (void*)0;
void* const AAOBKProtocolPropertyOwnerHandleKey = (void*)0;
void* const AAOBKProtocolPropertyPrimaryButtonKey = (void*)0;
void* const AAOBKProtocolPropertyRecipientHandleKey = (void*)0;
void* const AAOBKProtocolPropertySecondaryButtonKey = (void*)0;
void* const AAOBKProtocolPropertyTitleKey = (void*)0;
void* const AAPartialErrorsByNameKey = (void*)0;
void* const AAPrefsConfigurationDomain = (void*)0;
void* const AAPrefsConfigurationPlistURL = (void*)0;
void* const AAPrefsDomain = (void*)0;
void* const AAPrefsLocalPlistPath = (void*)0;
void* const AAPrefsUseLocalConfigPlist = (void*)0;
void* const AARecoveryContactChangedNotification = (void*)0;
void* const AAServiceTypeGameCenter = (void*)0;
void* AASetupAssistantAboutURLKey = (void*)0;
void* AASetupAssistantActiveEmailDomainKey = (void*)0;
void* const AASetupAssistantAppleIDKey = (void*)0;
void* AASetupAssistantAuthenticateURLKey = (void*)0;
void* const AASetupAssistantBirthdayKey = (void*)0;
void* AASetupAssistantCheckValidityURLKey = (void*)0;
void* const AASetupAssistantClientIDKey = (void*)0;
void* const AASetupAssistantCountryKey = (void*)0;
void* AASetupAssistantCreateAppleIDKey = (void*)0;
void* AASetupAssistantCreateAppleIDURLKey = (void*)0;
void* AASetupAssistantCreateDelegateAccountsURLKey = (void*)0;
void* AASetupAssistantCreateStatusKey = (void*)0;
void* AASetupAssistantCreateStatusMessageKey = (void*)0;
void* AASetupAssistantDSIDKey = (void*)0;
void* const AASetupAssistantDelegatesKey = (void*)0;
void* const AASetupAssistantEmailChoiceKey = (void*)0;
void* AASetupAssistantExistingAppleIDTermsUIURLKey = (void*)0;
void* const AASetupAssistantFirstNameKey = (void*)0;
void* AASetupAssistantIForgotURLKey = (void*)0;
void* const AASetupAssistantLanguageKey = (void*)0;
void* const AASetupAssistantLastNameKey = (void*)0;
void* AASetupAssistantLoginDelegatesURLKey = (void*)0;
void* const AASetupAssistantPasswordKey = (void*)0;
void* AASetupAssistantResponseParametersKey = (void*)0;
void* const AASetupAssistantSecurityAnswerKey = (void*)0;
void* const AASetupAssistantSecurityQuestionKey = (void*)0;
void* AASetupAssistantServerEnabledKey = (void*)0;
void* const AASetupAssistantTimezoneKey = (void*)0;
void* AASetupAssistantUpdateAppleIDURLKey = (void*)0;
void* AASetupAssistantUpgradeStatusKey = (void*)0;
void* AASetupAssistantUpgradeStatusURLKey = (void*)0;
void* AASetupAssistantUpgradeiOSTermsUIURLKey = (void*)0;
void* AASetupAssistantXMLUIKey = (void*)0;
void* AASetupAssistantiForgotUIURLKey = (void*)0;
void* const AASignInErrorDomain = (void*)0;
void* AASpyGlassOptionDependentAccount = (void*)0;
void* AASpyGlassOptionPrimaryAccount = (void*)0;
void* const AATermsEntryBeta = (void*)0;
void* const AATermsEntryDevice = (void*)0;
void* const AATermsEntryGameCenter = (void*)0;
void* const AATermsEntryPrivacy = (void*)0;
void* const AATermsEntryWarranty = (void*)0;
void* const AATermsEntryiCloud = (void*)0;
void* const AATermsEntryiTunes = (void*)0;
void* const AAURLEndpointAbout = (void*)0;
void* const AAURLEndpointAuthenticate = (void*)0;
void* const AAURLEndpointCreateDelegateAccounts = (void*)0;
void* const AAURLEndpointGetAccountSettings = (void*)0;
void* const AAURLEndpointLoginDelegates = (void*)0;
void* const AAURLEndpointLoginOrCreateAccount = (void*)0;
void* const AAURLEndpointQualifyCertificate = (void*)0;
void* const AAURLEndpointQualifySession = (void*)0;
void* const AAURLKeyCustomDomainState = (void*)0;
void* const AAURLKeyFlowType = (void*)0;
void* const AAURLKeyFlowTypeBeneficiary = (void*)0;
void* const AAURLKeyFlowTypeCustodian = (void*)0;
void* const AAURLKeyFlowTypeCustomDomain = (void*)0;
void* const AAURLKeyFlowTypeMailManage = (void*)0;
void* const AAURLKeyFlowTypePrivateEmailManage = (void*)0;
void* const AAURLKeyID = (void*)0;
void* const AAURLProtocolScheme = (void*)0;
void* const IDSRegistrationPropertySupportsCustodian = (void*)0;
void* const IDSRegistrationPropertyUnknown = (void*)0;
void* _AABeneficiaryClaimUIURLKey = (void*)0;
void* const _AAIDSServiceName = (void*)0;
void* const _AAMessageDestinationMailToPrefix = (void*)0;
void* const _AAMessageDestinationSelfTokenPrefix = (void*)0;
void* const _AAMessageDestinationTelToPrefix = (void*)0;
void* const _AAMessageDestinationTokenPrefix = (void*)0;
void* const _AAMessagesInviteContextURL = (void*)0;
void* const kAAAccountAlternateDSID = (void*)0;
void* const kAAAccountAppProvidedContextKey = (void*)0;
void* const kAAAccountAppleIDAliasesKey = (void*)0;
void* const kAAAccountAppleIDKey = (void*)0;
void* const kAAAccountBenefactorAccountKey = (void*)0;
void* const kAAAccountClassKey = (void*)0;
void* const kAAAccountCloudDocsMigrationComplete = (void*)0;
void* const kAAAccountDataclass = (void*)0;
void* const kAAAccountDataclassesDisabledForCellularKey = (void*)0;
void* const kAAAccountDataclassesPropertiesKey = (void*)0;
void* const kAAAccountEmailAddress = (void*)0;
void* const kAAAccountFirstNameKey = (void*)0;
void* const kAAAccountHasOptionalTerms = (void*)0;
void* const kAAAccountIsCloudSubscriber = (void*)0;
void* const kAAAccountIsFamilyEligible = (void*)0;
void* const kAAAccountIsManagedAppleID = (void*)0;
void* const kAAAccountIsSandboxAccount = (void*)0;
void* const kAAAccountLastAgreedTerms = (void*)0;
void* const kAAAccountLastKnownQuota = (void*)0;
void* const kAAAccountLastNameKey = (void*)0;
void* const kAAAccountMiddleNameKey = (void*)0;
void* const kAAAccountMobileMeStatusKey = (void*)0;
void* const kAAAccountNeedsToVerifyTerms = (void*)0;
void* const kAAAccountNotesMigrated = (void*)0;
void* const kAAAccountPasswordKey = (void*)0;
void* const kAAAccountPersonIDKey = (void*)0;
void* const kAAAccountPrimaryAccountKey = (void*)0;
void* const kAAAccountPrimaryAccountUpdaterVersion = (void*)0;
void* const kAAAccountPrimaryEmailKey = (void*)0;
void* const kAAAccountPrimaryEmailVerifiedKey = (void*)0;
void* const kAAAccountProtocolVersionKey = (void*)0;
void* const kAAAccountRegionInfoKey = (void*)0;
void* const kAAAccountRemindersAutoMigratableToCK = (void*)0;
void* const kAAAccountRemindersMigrated = (void*)0;
void* const kAAAccountRepairStateKey = (void*)0;
void* const kAAAccountRepairerPID = (void*)0;
void* const kAAAccountSyncedAccount = (void*)0;
void* const kAAAccountUbiquityPropertyiCloudHome = (void*)0;
void* const kAAAccountUsesCloudDocs = (void*)0;
void* const kAAAccountWarmingUp = (void*)0;
void* const kAAActionNameKey = (void*)0;
void* const kAAAltDSIDKey = (void*)0;
void* const kAACountryCodeKey = (void*)0;
void* const kAADeviceProvisioningMachineId = (void*)0;
void* const kAADeviceProvisioningOneTimePassword = (void*)0;
void* const kAAErrorDetailsActionButtonTitleKey = (void*)0;
void* const kAAErrorDetailsActionButtonTypeKey = (void*)0;
void* const kAAErrorDetailsActionButtonURLKey = (void*)0;
void* const kAAErrorDetailsDismissButtonTitleKey = (void*)0;
void* const kAAErrorDetailsMIDKey = (void*)0;
void* const kAAErrorDetailsMessageKey = (void*)0;
void* const kAAErrorDetailsOTPKey = (void*)0;
void* const kAAErrorDetailsProxiedDeviceDataKey = (void*)0;
void* const kAAErrorDetailsRoutingInfoKey = (void*)0;
void* const kAAErrorDetailsTitleKey = (void*)0;
void* const kAAMessagingServiceName = (void*)0;
void* const kAAMetadatanKey = (void*)0;
void* const kAAProcotolNewAccountUIURLKey = (void*)0;
void* const kAAProtocoEmailLookupURLKey = (void*)0;
void* const kAAProtocolAPSEnvKey = (void*)0;
void* const kAAProtocolAboutURLKey = (void*)0;
void* const kAAProtocolAcceptChildTransferURLKey = (void*)0;
void* const kAAProtocolAcceptFamilyInviteURLKey = (void*)0;
void* const kAAProtocolAcceptFamilyInviteV2URLKey = (void*)0;
void* const kAAProtocolAccountBetaKey = (void*)0;
void* const kAAProtocolAccountFirstDisplayAlertKey = (void*)0;
void* const kAAProtocolAccountFooterButtonKey = (void*)0;
void* const kAAProtocolAccountFooterTextKey = (void*)0;
void* const kAAProtocolAccountManagementUIURLKey = (void*)0;
void* const kAAProtocolAccountServiceTypeKey = (void*)0;
void* const kAAProtocolAccountUnavailableKey = (void*)0;
void* const kAAProtocolAddEmailURLKey = (void*)0;
void* const kAAProtocolAddFamilyMemberUIURLKey = (void*)0;
void* const kAAProtocolAltDSIDKey = (void*)0;
void* const kAAProtocolAppleAccountInfoKey = (void*)0;
void* const kAAProtocolAppleIDAliasesKey = (void*)0;
void* const kAAProtocolAppleIDEnteredByUserKey = (void*)0;
void* const kAAProtocolAppleIDKey = (void*)0;
void* const kAAProtocolAppleIDToken = (void*)0;
void* const kAAProtocolAuthTokenKey = (void*)0;
void* const kAAProtocolAuthenticateURLKey = (void*)0;
void* const kAAProtocolCheckiCloudMembershipURLKey = (void*)0;
void* const kAAProtocolChildAccountCreationUIURLKey = (void*)0;
void* const kAAProtocolClientIDKey = (void*)0;
void* const kAAProtocolCloudKitDevicesList = (void*)0;
void* const kAAProtocolCloudKitMigrationStateURLKey = (void*)0;
void* const kAAProtocolCloudKitStartMigrationURLKey = (void*)0;
void* const kAAProtocolCloudKitTokenKey = (void*)0;
void* const kAAProtocolCompleteValidateEmailURLKey = (void*)0;
void* const kAAProtocolCreateDelegatesURLKey = (void*)0;
void* const kAAProtocolDSIDKey = (void*)0;
void* const kAAProtocolDeclineFamilyInviteURLKey = (void*)0;
void* const kAAProtocolDelegatesKey = (void*)0;
void* const kAAProtocolDeviceHardwareModelKey = (void*)0;
void* const kAAProtocolDeviceInfoKey = (void*)0;
void* const kAAProtocolDeviceInfoSignatureKey = (void*)0;
void* const kAAProtocolDeviceListURLKey = (void*)0;
void* const kAAProtocolDeviceOSVersionKey = (void*)0;
void* const kAAProtocolDeviceQualificationURLKey = (void*)0;
void* const kAAProtocolDeviceRegionCode = (void*)0;
void* const kAAProtocolDeviceSerialNumberKey = (void*)0;
void* const kAAProtocolDeviceUDIDHeader = (void*)0;
void* const kAAProtocolDevicesUIURLKey = (void*)0;
void* const kAAProtocolDismissAlertURLKey = (void*)0;
void* const kAAProtocolDisplayedHostnameKey = (void*)0;
void* const kAAProtocolDomainURLsKey = (void*)0;
void* const kAAProtocolEmailToVet = (void*)0;
void* const kAAProtocolEmailVettingToken = (void*)0;
void* const kAAProtocolEnvironmentsKey = (void*)0;
void* const kAAProtocolErrorCancelButtonTitleKey = (void*)0;
void* const kAAProtocolErrorCommunicationsError = (void*)0;
void* const kAAProtocolErrorDescriptionKey = (void*)0;
void* const kAAProtocolErrorInvalidPassword = (void*)0;
void* const kAAProtocolErrorKey = (void*)0;
void* const kAAProtocolErrorMessageKey = (void*)0;
void* const kAAProtocolErrorMobileMeAccountDisabled = (void*)0;
void* const kAAProtocolErrorMobileMeAccountLapsed = (void*)0;
void* const kAAProtocolErrorMobileMeAccountMissingInfo = (void*)0;
void* const kAAProtocolErrorMobileMeCreateUnavailable = (void*)0;
void* const kAAProtocolErrorMobileMeInvalidSignature = (void*)0;
void* const kAAProtocolErrorMobileMeRegionDenied = (void*)0;
void* const kAAProtocolErrorMobileMeSuspendedMode = (void*)0;
void* const kAAProtocolErrorMobileMeTermsOfServiceUpdate = (void*)0;
void* const kAAProtocolErrorMobileMeUnsupportedDevice = (void*)0;
void* const kAAProtocolErrorResponseDictionaryKey = (void*)0;
void* const kAAProtocolErrorScheduledMaintenence = (void*)0;
void* const kAAProtocolErrorTitleKey = (void*)0;
void* const kAAProtocolExistingAppleIDTermsUIURLKey = (void*)0;
void* const kAAProtocolExperimentalModeHeaderKey = (void*)0;
void* const kAAProtocolFMFAppTokenKey = (void*)0;
void* const kAAProtocolFMFTokenKey = (void*)0;
void* const kAAProtocolFMIPAppTokenKey = (void*)0;
void* const kAAProtocolFMIPAuthenticateURLKey = (void*)0;
void* const kAAProtocolFMIPLostModeTokenKey = (void*)0;
void* const kAAProtocolFMIPSiriTokenKey = (void*)0;
void* const kAAProtocolFMIPTokenKey = (void*)0;
void* const kAAProtocolFMIPUrl = (void*)0;
void* const kAAProtocolFMIPWipeTokenKey = (void*)0;
void* const kAAProtocolFamilyEligibilityURLKey = (void*)0;
void* const kAAProtocolFamilyInviteSentV2URLKey = (void*)0;
void* const kAAProtocolFamilyLearnMoreURLKey = (void*)0;
void* const kAAProtocolFamilyPaymentCardUIURLKey = (void*)0;
void* const kAAProtocolFamilyPaymentInfoURLKey = (void*)0;
void* const kAAProtocolFamilySetupUIURLKey = (void*)0;
void* const kAAProtocolFamilyUIURLKey = (void*)0;
void* const kAAProtocolFetchFamilyInviteV2URLKey = (void*)0;
void* const kAAProtocolFinishProvisioningMachineKey = (void*)0;
void* const kAAProtocolFirstNameKey = (void*)0;
void* const kAAProtocolGameCenterDataKey = (void*)0;
void* const kAAProtocolGenericTermsUIBuddyMLKey = (void*)0;
void* const kAAProtocolGenericTermsUIPlistBuddyMLKey = (void*)0;
void* const kAAProtocolGenericTermsUIPlistTextKey = (void*)0;
void* const kAAProtocolGenericTermsURLKey = (void*)0;
void* const kAAProtocolGetAccountEventDetailsURLKey = (void*)0;
void* const kAAProtocolGetAccountSettingsURLKey = (void*)0;
void* const kAAProtocolGetDeviceListURLKey = (void*)0;
void* const kAAProtocolGetFamilyDetailsURLKey = (void*)0;
void* const kAAProtocolGetFamilyMemberPhotoURLKey = (void*)0;
void* const kAAProtocolGetFamilyPushDetailsURLKey = (void*)0;
void* const kAAProtocolGetMyPhotoURLKey = (void*)0;
void* const kAAProtocolGrandslamURLKey = (void*)0;
void* const kAAProtocolHasOptionalTermsKey = (void*)0;
void* const kAAProtocolIdentityServicesDataKey = (void*)0;
void* const kAAProtocolInitiateFamilyV2URLKey = (void*)0;
void* const kAAProtocolInitiateValidateEmailURLKey = (void*)0;
void* const kAAProtocolIsCloudDocsMigratedKey = (void*)0;
void* const kAAProtocolIsFamilyEligibleKey = (void*)0;
void* const kAAProtocolIsManagedAppleIDKey = (void*)0;
void* const kAAProtocolIsSandboxAccountKey = (void*)0;
void* const kAAProtocolKeyTransparencyTokenKey = (void*)0;
void* const kAAProtocolLanguageKey = (void*)0;
void* const kAAProtocolLastNameKey = (void*)0;
void* const kAAProtocolLoginAccountURLKey = (void*)0;
void* const kAAProtocolLoginDelegatesURLKey = (void*)0;
void* const kAAProtocolLoginOrCreateDelegatesURLKey = (void*)0;
void* const kAAProtocolLoginOrCreateURLKey = (void*)0;
void* const kAAProtocolMDMServerTokenKey = (void*)0;
void* const kAAProtocolMapsTokenKey = (void*)0;
void* const kAAProtocolMemberDetailsUIURLKey = (void*)0;
void* const kAAProtocolMobileMeDataKey = (void*)0;
void* const kAAProtocolMobileMeOfferAlertURLKey = (void*)0;
void* const kAAProtocolMobileMeOfferClassicMessageKey = (void*)0;
void* const kAAProtocolMobileMeOfferClassicTitleKey = (void*)0;
void* const kAAProtocolMobileMeOfferIdentifierKey = (void*)0;
void* const kAAProtocolMobileMeOfferMessageKey = (void*)0;
void* const kAAProtocolMobileMeOfferTitleKey = (void*)0;
void* const kAAProtocolMobileMeSetupTokenKey = (void*)0;
void* const kAAProtocolNewAccountURLKey = (void*)0;
void* const kAAProtocolNonQualifiedRegisterURLKey = (void*)0;
void* const kAAProtocolNotesMigratedKey = (void*)0;
void* const kAAProtocolPasswordKey = (void*)0;
void* const kAAProtocolPasswordSecurityUIURLKey = (void*)0;
void* const kAAProtocolPaymentSummaryUIURLKey = (void*)0;
void* const kAAProtocolPaymentUIURLKey = (void*)0;
void* const kAAProtocolPendingFamilyInvitesUIURLKey = (void*)0;
void* const kAAProtocolPersonIDKey = (void*)0;
void* const kAAProtocolPersonIDToken = (void*)0;
void* const kAAProtocolPersonalInfoUIURLKey = (void*)0;
void* const kAAProtocolPrefCustomHealthCheckIntervalEnabled = (void*)0;
void* const kAAProtocolPrefCustomHealthCheckIntervalMinutes = (void*)0;
void* const kAAProtocolPrefCustomHealthCheckReachabilityIntervalMinutes = (void*)0;
void* const kAAProtocolPrefCustomHealthCheckVersion = (void*)0;
void* const kAAProtocolPrefDisableHealthCheckTTRKey = (void*)0;
void* const kAAProtocolPrefExperimentalModeKey = (void*)0;
void* const kAAProtocolPrefMultipleFullAccounts = (void*)0;
void* const kAAProtocolPrefNeverSkipCustodianCheckKey = (void*)0;
void* const kAAProtocolPrefSendInternalDevHeaderKey = (void*)0;
void* const kAAProtocolPrefShowAccountContactsKey = (void*)0;
void* const kAAProtocolPrefSimulateUnhealthyCustodianKey = (void*)0;
void* const kAAProtocolPrefSuppressFireKey = (void*)0;
void* const kAAProtocolPrefUseLegacyLoginEndpoint = (void*)0;
void* const kAAProtocolPrimaryEmailKey = (void*)0;
void* const kAAProtocolPrimaryEmailVerifiedKey = (void*)0;
void* const kAAProtocolProvisionedDataclassesKey = (void*)0;
void* const kAAProtocolRegionInfoKey = (void*)0;
void* const kAAProtocolRegisterDeviceURLKey = (void*)0;
void* const kAAProtocolRegisterURLKey = (void*)0;
void* const kAAProtocolRejectChildTransferURLKey = (void*)0;
void* const kAAProtocolRemindersAutoMigratableToCKKey = (void*)0;
void* const kAAProtocolRemindersMigratedKey = (void*)0;
void* const kAAProtocolRepairCloudAccountUIURLKey = (void*)0;
void* const kAAProtocolSearchPartyTokenKey = (void*)0;
void* const kAAProtocolSecondaryAuthURLKey = (void*)0;
void* const kAAProtocolSendCodeURLKey = (void*)0;
void* const kAAProtocolServerResponseStatus = (void*)0;
void* const kAAProtocolServerResponseStatusMessage = (void*)0;
void* const kAAProtocolServiceDataKey = (void*)0;
void* const kAAProtocolSigningSessionCertificateURLKey = (void*)0;
void* const kAAProtocolSigningSessionURLKey = (void*)0;
void* const kAAProtocolStartFamilyInviteV2URLKey = (void*)0;
void* const kAAProtocolSuspendedInfoKey = (void*)0;
void* const kAAProtocolSuspensionFamilyKey = (void*)0;
void* const kAAProtocolSuspensioniCloudKey = (void*)0;
void* const kAAProtocolSyncMachineKey = (void*)0;
void* const kAAProtocolTermsReportUserActionURLKey = (void*)0;
void* const kAAProtocolTimezoneKey = (void*)0;
void* const kAAProtocolTokensKey = (void*)0;
void* const kAAProtocolUDIDKey = (void*)0;
void* const kAAProtocolUDIDToken = (void*)0;
void* const kAAProtocolURLsKey = (void*)0;
void* const kAAProtocolUnregisterDeviceURLKey = (void*)0;
void* const kAAProtocolUpdateAccountUIURLKey = (void*)0;
void* const kAAProtocolUpdateAccountURLKey = (void*)0;
void* const kAAProtocolUpdateMyPhotoURLKey = (void*)0;
void* const kAAProtocolUpdateNameURLKey = (void*)0;
void* const kAAProtocolUserInfoKey = (void*)0;
void* const kAAProtocolValidateURLKey = (void*)0;
void* const kAAProtocolVerifyCodeURLKey = (void*)0;
void* const kAAProtocolVersionKey = (void*)0;
void* const kAAProtocoliCloudAccountKey = (void*)0;
void* const kAAProtocoliCloudDataclassPrefix = (void*)0;
void* const kAAProtocoliForgotUIURLKey = (void*)0;
void* const kAASaveOptionAnisetteMIDKey = (void*)0;
void* const kAASaveOptionAnisetteOTPKey = (void*)0;
void* const kAASaveOptionAnisetteRoutingKey = (void*)0;
void* const kAASaveOptionCompanionDeviceClientInfoKey = (void*)0;
void* const kAASaveOptionCompanionDeviceUDIDKey = (void*)0;
void* const kAASaveOptionsTwoFactorLoginCookiesKey = (void*)0;
void* const kAASendWalrusWebAccessPCSAuthKey = (void*)0;
void* const kAATermsAgreeUrlKey = (void*)0;
void* const kAATermsFieldFormat = (void*)0;
void* const kAATermsFieldName = (void*)0;
void* const kAATermsFieldTerms = (void*)0;
void* const kAATermsVersionKey = (void*)0;
void* const kAATrueString = (void*)0;
void* const kAAiCloudVersionKey = (void*)0;
void* const kAppleIDMigratedAccountIdentifier = (void*)0;
void* const kLegacyAAAccountAppleIDAliasesKey = (void*)0;
void* const kLegacyAAAccountAuthTokenKey = (void*)0;
void* const kLegacyAAAccountDataclass = (void*)0;
void* const kLegacyAAAccountDataclassesDisabledForCellularKey = (void*)0;
void* const kLegacyAAAccountDataclassesPropertiesKey = (void*)0;
void* const kLegacyAAAccountFMIPTokenKey = (void*)0;
void* const kLegacyAAAccountFirstNameKey = (void*)0;
void* const kLegacyAAAccountLastNameKey = (void*)0;
void* const kLegacyAAAccountMobileMeStatusKey = (void*)0;
void* const kLegacyAAAccountPersonIDKey = (void*)0;
void* const kLegacyAAAccountPrimaryAccountKey = (void*)0;
void* const kLegacyAAAccountPrimaryEmailKey = (void*)0;
void* const kLegacyAAAccountPrimaryEmailVerifiedKey = (void*)0;
void* const kLegacyAAAccountProtocolVersionKey = (void*)0;

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

