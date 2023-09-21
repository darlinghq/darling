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

#ifndef _APPLEACCOUNT_H_
#define _APPLEACCOUNT_H_

#import <Foundation/Foundation.h>

#import <AppleAccount/AAAbsintheContext.h>
#import <AppleAccount/AAAbsintheContextProtocol.h>
#import <AppleAccount/AAAbsintheSigner.h>
#import <AppleAccount/AAAbsintheSignerContextCache.h>
#import <AppleAccount/AAAcceptedTermsController.h>
#import <AppleAccount/AAAccount.h>
#import <AppleAccount/AAAccountBeneficiaryManagementViewModel.h>
#import <AppleAccount/AAAccountContactPromptModel.h>
#import <AppleAccount/AAAccountManagementUIResponse.h>
#import <AppleAccount/AAAccountManager.h>
#import <AppleAccount/AAAccountRecoveryManagementViewModel.h>
#import <AppleAccount/AAAccountUserNotificationContext.h>
#import <AppleAccount/AAAccountUserNotificationDaemonConnection.h>
#import <AppleAccount/AAAccountUserNotificationDaemonInterface.h>
#import <AppleAccount/AAAccountUserNotificationDaemonProtocol.h>
#import <AppleAccount/AAAccountUserNotificationPublisher.h>
#import <AppleAccount/AAAddEmailUIRequest.h>
#import <AppleAccount/AAAppleIDLoginPlugin.h>
#import <AppleAccount/AAAppleIDSettingsRequest.h>
#import <AppleAccount/AAAppleTVRequest.h>
#import <AppleAccount/AAAttestationSigner.h>
#import <AppleAccount/AAAuthenticateRequest.h>
#import <AppleAccount/AAAuthenticationResponse.h>
#import <AppleAccount/AAAutoAccountVerifier.h>
#import <AppleAccount/AAAvailabilityRequest.h>
#import <AppleAccount/AAAvailabilityResponse.h>
#import <AppleAccount/AABenefactorInfo.h>
#import <AppleAccount/AABeneficiary.h>
#import <AppleAccount/AABeneficiaryClaimUIRequest.h>
#import <AppleAccount/AABeneficiaryInfo.h>
#import <AppleAccount/AACNContactsManager.h>
#import <AppleAccount/AACertificatePinner.h>
#import <AppleAccount/AAChooseContactViewModel.h>
#import <AppleAccount/AACloudKitDevicesListRequest.h>
#import <AppleAccount/AACloudKitDevicesListResponse.h>
#import <AppleAccount/AACloudKitMigrationStateRequest.h>
#import <AppleAccount/AACloudKitMigrationStateResponse.h>
#import <AppleAccount/AACloudKitStartMigrationRequest.h>
#import <AppleAccount/AACloudKitStartMigrationResponse.h>
#import <AppleAccount/AACompleteEmailVettingRequest.h>
#import <AppleAccount/AACompleteEmailVettingResponse.h>
#import <AppleAccount/AAContactsManager.h>
#import <AppleAccount/AAContactsProvider.h>
#import <AppleAccount/AACustodianChooseContactViewModel.h>
#import <AppleAccount/AACustodianController.h>
#import <AppleAccount/AACustodianDaemonConnection.h>
#import <AppleAccount/AACustodianDaemonInterface.h>
#import <AppleAccount/AACustodianDaemonProtocol.h>
#import <AppleAccount/AACustodianDataRecoveryKeys.h>
#import <AppleAccount/AACustodianInvitationResponseContext.h>
#import <AppleAccount/AACustodianMessageInviteContext.h>
#import <AppleAccount/AACustodianPasswordResetInformation.h>
#import <AppleAccount/AACustodianRecoveryConfiguration.h>
#import <AppleAccount/AACustodianRecoveryRequestContext.h>
#import <AppleAccount/AACustodianSetupRequestContext.h>
#import <AppleAccount/AACustodianshipInfo.h>
#import <AppleAccount/AADaemonController.h>
#import <AppleAccount/AADaemonProtocol.h>
#import <AppleAccount/AADataclassManager.h>
#import <AppleAccount/AADataclassVersionUpdateProtocol.h>
#import <AppleAccount/AADependentAuthenticationUIRequest.h>
#import <AppleAccount/AADevice.h>
#import <AppleAccount/AADeviceInfo.h>
#import <AppleAccount/AADeviceList.h>
#import <AppleAccount/AADeviceListRequest.h>
#import <AppleAccount/AADeviceListResponse.h>
#import <AppleAccount/AADeviceProvisioningRequest.h>
#import <AppleAccount/AADeviceProvisioningResponse.h>
#import <AppleAccount/AADeviceProvisioningSession.h>
#import <AppleAccount/AAEmailVettingRequest.h>
#import <AppleAccount/AAFMIPAuthenticateRequest.h>
#import <AppleAccount/AAFMIPAuthenticateResponse.h>
#import <AppleAccount/AAFXPCSessionDelegate.h>
#import <AppleAccount/AAFamilyDetailsRequest.h>
#import <AppleAccount/AAFamilyDetailsResponse.h>
#import <AppleAccount/AAFamilyEligibilityRequest.h>
#import <AppleAccount/AAFamilyEligibilityResponse.h>
#import <AppleAccount/AAFamilyInvite.h>
#import <AppleAccount/AAFamilyMember.h>
#import <AppleAccount/AAFamilyRequest.h>
#import <AppleAccount/AAFlowPresenter.h>
#import <AppleAccount/AAFlowPresenterHostInterface.h>
#import <AppleAccount/AAFlowPresenterHostProtocol.h>
#import <AppleAccount/AAFlowPresenterProtocol.h>
#import <AppleAccount/AAFollowUpController.h>
#import <AppleAccount/AAFollowUpProtocol.h>
#import <AppleAccount/AAFollowUpUtilities.h>
#import <AppleAccount/AAGenericTermsUIRequest.h>
#import <AppleAccount/AAGenericTermsUIResponse.h>
#import <AppleAccount/AAGrandSlamSigner.h>
#import <AppleAccount/AAIDSMessagingService.h>
#import <AppleAccount/AAIDSMessagingServiceInternal.h>
#import <AppleAccount/AAInheritance.h>
#import <AppleAccount/AAInheritanceContact.h>
#import <AppleAccount/AAInheritanceContactController.h>
#import <AppleAccount/AAInheritanceContactInfo.h>
#import <AppleAccount/AAInheritanceContactsViewModel.h>
#import <AppleAccount/AAInheritanceController.h>
#import <AppleAccount/AAInheritanceDaemonProtocol.h>
#import <AppleAccount/AAInheritanceInvitation.h>
#import <AppleAccount/AAInheritanceMessageInviteContext.h>
#import <AppleAccount/AAInitiateEmailVettingRequest.h>
#import <AppleAccount/AAInviteCompletionInfo.h>
#import <AppleAccount/AAKeychainManager.h>
#import <AppleAccount/AALocalContactInfo.h>
#import <AppleAccount/AALocalization.h>
#import <AppleAccount/AALoginAccountRequest.h>
#import <AppleAccount/AALoginAccountResponse.h>
#import <AppleAccount/AALoginContextManager.h>
#import <AppleAccount/AALoginContextTransientStorage.h>
#import <AppleAccount/AALoginDelegatesRequest.h>
#import <AppleAccount/AALoginDelegatesResponse.h>
#import <AppleAccount/AALoginOrCreateDelegatesRequest.h>
#import <AppleAccount/AALoginPluginManager.h>
#import <AppleAccount/AALoginResponseAppleAccountInfo.h>
#import <AppleAccount/AALoginResponseDataclasses.h>
#import <AppleAccount/AALoginResponseiCloudTokens.h>
#import <AppleAccount/AAMessage.h>
#import <AppleAccount/AAMessagesInviteContext.h>
#import <AppleAccount/AAMessagingAccount.h>
#import <AppleAccount/AAMessagingCapability.h>
#import <AppleAccount/AAMessagingContext.h>
#import <AppleAccount/AAMessagingDestination.h>
#import <AppleAccount/AAMessagingDevice.h>
#import <AppleAccount/AAMessagingService.h>
#import <AppleAccount/AAMobileMeOfferResponse.h>
#import <AppleAccount/AAMyPhotoRequest.h>
#import <AppleAccount/AAOBAddTrustedContactsModel.h>
#import <AppleAccount/AAOBBulletPointModel.h>
#import <AppleAccount/AAOBBulletPointModelProtocol.h>
#import <AppleAccount/AAOBBulletedWelcomeControllerModelProtocol.h>
#import <AppleAccount/AAOBCustodianContactSelectorModel.h>
#import <AppleAccount/AAOBCustodianHelpNowModel.h>
#import <AppleAccount/AAOBCustodianInvitationModel.h>
#import <AppleAccount/AAOBCustodiansListViewModel.h>
#import <AppleAccount/AAOBInheritanceContactPickerModel.h>
#import <AppleAccount/AAOBInheritanceInvitationModel.h>
#import <AppleAccount/AAOBInheritanceInviteMessageModel.h>
#import <AppleAccount/AAOBInheritanceSetupCompleteModel.h>
#import <AppleAccount/AAOBInheritanceShareAccessKeyOptionsModel.h>
#import <AppleAccount/AAOBInheritanceShareDataModel.h>
#import <AppleAccount/AAOBInvitationSentModel.h>
#import <AppleAccount/AAOBModelHelper.h>
#import <AppleAccount/AAOBTrustedContactInviteMessageModel.h>
#import <AppleAccount/AAOBTrustedContactInviteMessageModelProtocol.h>
#import <AppleAccount/AAOBWelcomeControllerModelProtocol.h>
#import <AppleAccount/AAPasswordSecurityUIRequest.h>
#import <AppleAccount/AAPaymentSummaryRequest.h>
#import <AppleAccount/AAPaymentSummaryResponse.h>
#import <AppleAccount/AAPaymentUIRequest.h>
#import <AppleAccount/AAPendingIDSMessage.h>
#import <AppleAccount/AAPersonaUtility.h>
#import <AppleAccount/AAPersonalInfoUIRequest.h>
#import <AppleAccount/AAPhotoResponse.h>
#import <AppleAccount/AAPreferences.h>
#import <AppleAccount/AAPrimaryAccountUpdater.h>
#import <AppleAccount/AAProvisioningResponse.h>
#import <AppleAccount/AAQuotaDepletionAlert.h>
#import <AppleAccount/AAQuotaInfoRequest.h>
#import <AppleAccount/AAQuotaInfoResponse.h>
#import <AppleAccount/AARecoveryFactorController.h>
#import <AppleAccount/AARegionInfo.h>
#import <AppleAccount/AARegisterRequest.h>
#import <AppleAccount/AARemoteServer.h>
#import <AppleAccount/AARemoteServerConfigurationCache.h>
#import <AppleAccount/AARemoteServerProtocol.h>
#import <AppleAccount/AARequest.h>
#import <AppleAccount/AARequester.h>
#import <AppleAccount/AAResponse.h>
#import <AppleAccount/AASecondaryAuthenticationRequest.h>
#import <AppleAccount/AASecondaryAuthenticationResponse.h>
#import <AppleAccount/AASetupAssistantAuthenticateRequest.h>
#import <AppleAccount/AASetupAssistantConfigRequest.h>
#import <AppleAccount/AASetupAssistantConfigResponse.h>
#import <AppleAccount/AASetupAssistantCreateRequest.h>
#import <AppleAccount/AASetupAssistantCreateResponse.h>
#import <AppleAccount/AASetupAssistantService.h>
#import <AppleAccount/AASetupAssistantSetupDelegatesRequest.h>
#import <AppleAccount/AASetupAssistantSetupDelegatesResponse.h>
#import <AppleAccount/AASetupAssistantTermsFetchRequest.h>
#import <AppleAccount/AASetupAssistantUpdateRequest.h>
#import <AppleAccount/AASetupAssistantUpgradeStatusRequest.h>
#import <AppleAccount/AASetupAssistantUpgradeStatusResponse.h>
#import <AppleAccount/AASignInFlowController.h>
#import <AppleAccount/AASignInOperationHelper.h>
#import <AppleAccount/AASignOutFlowController.h>
#import <AppleAccount/AASigningSession.h>
#import <AppleAccount/AASigningSessionRequest.h>
#import <AppleAccount/AASigningSessionResponse.h>
#import <AppleAccount/AAStorableLoginContext.h>
#import <AppleAccount/AASuspensionInfo.h>
#import <AppleAccount/AATermsReportUserActionRequest.h>
#import <AppleAccount/AATermsReportUserActionResponse.h>
#import <AppleAccount/AATermsUserActionReporter.h>
#import <AppleAccount/AATrustedContact.h>
#import <AppleAccount/AATrustedContactDetailsViewModel.h>
#import <AppleAccount/AATrustedContactDetailsViewModelFactory.h>
#import <AppleAccount/AATrustedContactFlowPresenter.h>
#import <AppleAccount/AATrustedContactHealthInfo.h>
#import <AppleAccount/AATrustedContactNotificationPublisher.h>
#import <AppleAccount/AATrustedContactStringProvider.h>
#import <AppleAccount/AATrustedContactsCustodianSplashScreenModel.h>
#import <AppleAccount/AATrustedContactsInheritanceSplashScreenModel.h>
#import <AppleAccount/AATrustedDevice.h>
#import <AppleAccount/AATrustedDeviceListRequest.h>
#import <AppleAccount/AATrustedDeviceListResponse.h>
#import <AppleAccount/AAURLConfiguration.h>
#import <AppleAccount/AAURLConfigurationRequest.h>
#import <AppleAccount/AAURLProtocol.h>
#import <AppleAccount/AAURLSession.h>
#import <AppleAccount/AAURLSessionContext.h>
#import <AppleAccount/AAURLSessionTaskProtocol.h>
#import <AppleAccount/AAUniversalLinkHelper.h>
#import <AppleAccount/AAUpdateAccountUIRequest.h>
#import <AppleAccount/AAUpdateNameRequest.h>
#import <AppleAccount/AAUpdateProvisioningRequest.h>
#import <AppleAccount/AAUpgradeiOSTermsResponse.h>
#import <AppleAccount/AAUserNotification.h>
#import <AppleAccount/AAVersionUpdater.h>
#import <AppleAccount/AAVersionUpdaterProtocol.h>
#import <AppleAccount/AAVersionUpdaterProtocol_Internal.h>
#import <AppleAccount/AAWalrusPCSAuthRequest.h>
#import <AppleAccount/AAWalrusRecoveryContactRemovalAlertModel.h>
#import <AppleAccount/AAWalrusRecoveryContactRemovedScreenModel.h>
#import <AppleAccount/AAWalrusRecoveryKeyRemovalViewModel.h>
#import <AppleAccount/AAWalrusStringProvider.h>
#import <AppleAccount/AAiCloudLoginAccountRequester.h>
#import <AppleAccount/AAiCloudTermsAgreeRequest.h>
#import <AppleAccount/AAiCloudTermsAgreeResponse.h>
#import <AppleAccount/AAiCloudTermsStringRequest.h>
#import <AppleAccount/AAiCloudTermsStringResponse.h>
#import <AppleAccount/ACAccount+AppleID.h>
#import <AppleAccount/ACAccountStore+AppleID.h>
#import <AppleAccount/ACDataclassAction+AppleAccount.h>
#import <AppleAccount/ATVHighSecurityAccountDeviceList.h>
#import <AppleAccount/ATVHighSecurityAccountDeviceListResponse.h>
#import <AppleAccount/ATVHighSecurityAccountSendCode.h>
#import <AppleAccount/ATVHighSecurityAccountSendCodeResponse.h>
#import <AppleAccount/ATVHighSecurityAccountVerifyCode.h>
#import <AppleAccount/ATVHighSecurityAccountVerifyCodeResponse.h>
#import <AppleAccount/IDSServiceDelegate.h>
#import <AppleAccount/NSArray+AppleAccount.h>
#import <AppleAccount/NSData+AppleAccount.h>
#import <AppleAccount/NSDictionary+AppleAccount.h>
#import <AppleAccount/NSError+AppleAccount.h>
#import <AppleAccount/NSHTTPCookieStorage+AppleAccount.h>
#import <AppleAccount/NSMutableArray+AppleAccount.h>
#import <AppleAccount/NSMutableDictionary+AppleAccount.h>
#import <AppleAccount/NSMutableURLRequest+AppleAccount.h>
#import <AppleAccount/NSOperationQueue+AppleAccount.h>
#import <AppleAccount/NSSet+AppleAccount.h>
#import <AppleAccount/NSString+AAMessage.h>
#import <AppleAccount/NSURL+AppleAccount.h>
#import <AppleAccount/NSURLConnectionDelegate.h>
#import <AppleAccount/NSURLRequest+AppleAccount.h>
#import <AppleAccount/NSURLResponse+AppleAccount.h>
#import <AppleAccount/NSURLSessionAppleIDContext.h>
#import <AppleAccount/NSURLSessionConfiguration+AppleAccount.h>
#import <AppleAccount/NSURLSessionDataDelegate.h>
#import <AppleAccount/NSURLSessionDelegate.h>
#import <AppleAccount/NSURLSessionTaskDelegate.h>
#import <AppleAccount/NSXPCListenerDelegate.h>
#import <AppleAccount/TrustedContactFlowPresenterProtocol.h>
#import <AppleAccount/_AABasicGetRequest.h>
#import <AppleAccount/_AAURLSessionConfigurationTask.h>
#import <AppleAccount/_AAURLSessionDelegate.h>
#import <AppleAccount/_AAURLSessionOperation.h>


void *AALocalizedStringForKey(void);
void *AALocalizedStringForKeyInTable(void);
void *AALocalizedStringForServiceType(void);
void *AKTrustedContactStatusFromAATrustedContactStatus(void);
void *_AAErrorUnderlyingError(void);
void *_AALogSystem(void);
void *_AASignpostCreate(void);
void *_AASignpostGetNanoseconds(void);
void *_AASignpostLogSystem(void);


extern void* const AAAbsintheSignerDefaultTimeout;
extern void* const AAAccountClassBasic;
extern void* const AAAccountClassFull;
extern void* const AAAccountClassPrimary;
extern void* const AAAccountConfigChangedNotification;
extern void* const AAAccountUserNotificationMachService;
extern void* const AAAppProvidedContextKey;
extern void* const AAAttestationSignerKeychainLabel;
extern void* const AAAttestationSignerValidity;
extern void* const AACloudKitMigrationStateDone;
extern void* const AACloudKitMigrationStateFailed;
extern void* const AACloudKitMigrationStateMigrating;
extern void* const AACloudKitMigrationStateNone;
extern void* const AACloudKitMigrationStateNotNeeded;
extern void* const AACloudKitMigrationStateStart;
extern void* const AACustodianErrorDomain;
extern void* const AACustodianMachService;
extern void* const AACustodianRecoveryErrorDomain;
extern void* const AADaemonMachService;
extern void* const AADataclassActionErrorDomain;
extern void* const AADataclassActionUserInfoLibrarySize;
extern void* const AADataclassActionsKey;
extern void* const AADataclassesKey;
extern void* const AADefaultGrandSlamTokenHeaderKey;
extern void* const AAErrorDomain;
extern void* const AAFamilyEligibilityKey;
extern void* const AAFamilyGrandSlamTokenHeaderKey;
extern void* AAFollowUpActionDismiss;
extern void* AAFollowUpActionPrimary;
extern void* AAFollowUpClientIdentifier;
extern void* AAFollowUpExtensionIdentifier;
extern void* AAFollowUpIdentifierBeneficiaryIneligible;
extern void* AAFollowUpIdentifierBeneficiaryRemoved;
extern void* AAFollowUpIdentifierCustodianEmbargo;
extern void* AAFollowUpIdentifierCustodianInvitationReminder;
extern void* AAFollowUpIdentifierCustodianRemoved;
extern void* AAFollowUpIdentifierCustodianWalrusNoLiveness;
extern void* AAFollowUpIdentifierCustodianWalrusRemoved;
extern void* AAFollowUpIdentifierRenewCredentials;
extern void* AAFollowUpIdentifierStartUsing;
extern void* AAFollowUpIdentifierUserInfoKey;
extern void* AAFollowUpIdentifierVerifyTerms;
extern void* AAFollowUpIdentifierVerifyTermsV2;
extern void* AAFollowUpIdentifierWalrusUserRecoveryFactorsMissing;
extern void* const AAFollowUpMacOptionsButtonTitle;
extern void* AAFollowUpOpenPrefPaneActionAddBeneficiary;
extern void* AAFollowUpOpenPrefPaneActionAddCustodian;
extern void* const AAFollowUpOpenPrefPaneActionAddRecoveryKey;
extern void* AAFollowUpOpenPrefPaneActionBeneficiaryList;
extern void* AAFollowUpOpenPrefPaneActionCustodianRecovery;
extern void* AAFollowUpOpenPrefPaneActionKey;
extern void* AAFollowUpOpenPrefPaneActionRenewAuth;
extern void* AAFollowUpOpenPrefPaneActionShowAppleIDLogin;
extern void* AAFollowUpOpenPrefPaneActionShowDeviceList;
extern void* AAFollowUpOpenPrefPaneActionShowInitialPage;
extern void* AAFollowUpOpenPrefPaneActionShowTransparencyPane;
extern void* AAFollowUpOpenPrefPaneActionShowTrustedContacts;
extern void* AAFollowUpOpenPrefPaneActionTerms;
extern void* const AAFollowUpOpenPrefPaneActionWalrusAddRecoveryContact;
extern void* AAFollowUpOpenPrefPaneURL;
extern void* AAFollowUpTrustedContactFirstName;
extern void* AAFollowUpTrustedContactHandle;
extern void* AAFollowUpTrustedContactID;
extern void* AAFollowUpTrustedContactInvitationURL;
extern void* AAFollowUpTrustedContactName;
extern void* AAFollowUpUserInfoAccountIdentifier;
extern void* AAFollowUpUserInfoAltDSID;
extern void* AAFollowUpUserInfoCFUType;
extern void* AAFollowUpUserInfoClientName;
extern void* AAFollowUpUserInfoCustodianSessionID;
extern void* AAFollowUpUserInfoNotificationAltButtonTitle;
extern void* AAFollowUpUserInfoNotificationBody;
extern void* AAFollowUpUserInfoNotificationButtonTitle;
extern void* AAFollowUpUserInfoNotificationDelay;
extern void* AAFollowUpUserInfoNotificationFrequency;
extern void* AAFollowUpUserInfoNotificationInfo;
extern void* AAFollowUpUserInfoNotificationTimeLimit;
extern void* AAFollowUpUserInfoNotificationTitle;
extern void* AAFollowUpUserInfoProxiedDeviceData;
extern void* AAFollowUpUserInfoTermsCFUBody;
extern void* AAFollowUpUserInfoTermsCFUTitle;
extern void* AAFollowUpUserInfoTermsDependentAltDSID;
extern void* AAFollowUpUserInfoTermsFamilyAltDSIDValue;
extern void* AAFollowUpUserInfoTermsNotificationBody;
extern void* AAFollowUpUserInfoTermsNotificationTitle;
extern void* AAFollowUpUserInfoTermsServerProvidedContext;
extern void* const AAGrandSlamAppTokenIDAppleIDSettings;
extern void* const AAGrandSlamAppTokenIDFamilySharing;
extern void* const AAGrandSlamAppTokenIDiCloudSettings;
extern void* const AAHTTPHeaderDeviceUDID;
extern void* const AAHTTPHeaderMachineID;
extern void* const AAHTTPHeaderOneTimePassword;
extern void* const AAHTTPHeaderXAppleMDAction;
extern void* const AAHTTPHeaderXAppleMDData;
extern void* const AAIDSRegistrationPropertySupportsLegacyContact;
extern void* const AAInheritanceAccessKeyKey;
extern void* const AAInheritanceBenefactorAltDSIDKey;
extern void* const AAInheritanceBeneficiaryIDKey;
extern void* AAInheritanceConfigDurationBeforeNotSetupCFUKey;
extern void* AAInheritanceConfigStaleInviteDurationKey;
extern void* const AAInheritanceContactChangedNotification;
extern void* const AAInheritanceContactHandleKey;
extern void* const AAInheritanceErrorDomain;
extern void* const AAInheritanceMachService;
extern void* const AAInheritanceSetupHelpURL;
extern void* const AALoginDelegatesAppleIDKey;
extern void* const AALoginDelegatesDSIDKey;
extern void* const AALoginDelegatesDelegatesKey;
extern void* const AALoginDelegatesPasswordKey;
extern void* const AALoginDelegatesResponseParametersKey;
extern void* const AALoginDelegatesServiceDataKey;
extern void* const AALoginDelegatesStatusKey;
extern void* const AALoginDelegatesStatusMessageKey;
extern void* const AAMessagesInviteContextActionLabelKey;
extern void* const AAMessagesInviteContextInviteURLKey;
extern void* const AAMessagesInviteContextInviteesKey;
extern void* const AAMessagesInviteContextMessageBodyKey;
extern void* const AAMessagesInviteContextResponseSubtitleKey;
extern void* const AAMessagesInviteContextSubtitleKey;
extern void* const AAMessagesInviteContextTitleKey;
extern void* const AAMessagingErrorDomain;
extern void* const AAOBKProtocolPropertyBubbleTextKey;
extern void* const AAOBKProtocolPropertyCustodianshipInfo;
extern void* const AAOBKProtocolPropertyDetailTextKey;
extern void* const AAOBKProtocolPropertyHelpLinkTitleKey;
extern void* const AAOBKProtocolPropertyHelpLinkURLKey;
extern void* const AAOBKProtocolPropertyImageNameKey;
extern void* const AAOBKProtocolPropertyOwnerHandleKey;
extern void* const AAOBKProtocolPropertyPrimaryButtonKey;
extern void* const AAOBKProtocolPropertyRecipientHandleKey;
extern void* const AAOBKProtocolPropertySecondaryButtonKey;
extern void* const AAOBKProtocolPropertyTitleKey;
extern void* const AAPartialErrorsByNameKey;
extern void* const AAPrefsConfigurationDomain;
extern void* const AAPrefsConfigurationPlistURL;
extern void* const AAPrefsDomain;
extern void* const AAPrefsLocalPlistPath;
extern void* const AAPrefsUseLocalConfigPlist;
extern void* const AARecoveryContactChangedNotification;
extern void* const AAServiceTypeGameCenter;
extern void* AASetupAssistantAboutURLKey;
extern void* AASetupAssistantActiveEmailDomainKey;
extern void* const AASetupAssistantAppleIDKey;
extern void* AASetupAssistantAuthenticateURLKey;
extern void* const AASetupAssistantBirthdayKey;
extern void* AASetupAssistantCheckValidityURLKey;
extern void* const AASetupAssistantClientIDKey;
extern void* const AASetupAssistantCountryKey;
extern void* AASetupAssistantCreateAppleIDKey;
extern void* AASetupAssistantCreateAppleIDURLKey;
extern void* AASetupAssistantCreateDelegateAccountsURLKey;
extern void* AASetupAssistantCreateStatusKey;
extern void* AASetupAssistantCreateStatusMessageKey;
extern void* AASetupAssistantDSIDKey;
extern void* const AASetupAssistantDelegatesKey;
extern void* const AASetupAssistantEmailChoiceKey;
extern void* AASetupAssistantExistingAppleIDTermsUIURLKey;
extern void* const AASetupAssistantFirstNameKey;
extern void* AASetupAssistantIForgotURLKey;
extern void* const AASetupAssistantLanguageKey;
extern void* const AASetupAssistantLastNameKey;
extern void* AASetupAssistantLoginDelegatesURLKey;
extern void* const AASetupAssistantPasswordKey;
extern void* AASetupAssistantResponseParametersKey;
extern void* const AASetupAssistantSecurityAnswerKey;
extern void* const AASetupAssistantSecurityQuestionKey;
extern void* AASetupAssistantServerEnabledKey;
extern void* const AASetupAssistantTimezoneKey;
extern void* AASetupAssistantUpdateAppleIDURLKey;
extern void* AASetupAssistantUpgradeStatusKey;
extern void* AASetupAssistantUpgradeStatusURLKey;
extern void* AASetupAssistantUpgradeiOSTermsUIURLKey;
extern void* AASetupAssistantXMLUIKey;
extern void* AASetupAssistantiForgotUIURLKey;
extern void* const AASignInErrorDomain;
extern void* AASpyGlassOptionDependentAccount;
extern void* AASpyGlassOptionPrimaryAccount;
extern void* const AATermsEntryBeta;
extern void* const AATermsEntryDevice;
extern void* const AATermsEntryGameCenter;
extern void* const AATermsEntryPrivacy;
extern void* const AATermsEntryWarranty;
extern void* const AATermsEntryiCloud;
extern void* const AATermsEntryiTunes;
extern void* const AAURLEndpointAbout;
extern void* const AAURLEndpointAuthenticate;
extern void* const AAURLEndpointCreateDelegateAccounts;
extern void* const AAURLEndpointGetAccountSettings;
extern void* const AAURLEndpointLoginDelegates;
extern void* const AAURLEndpointLoginOrCreateAccount;
extern void* const AAURLEndpointQualifyCertificate;
extern void* const AAURLEndpointQualifySession;
extern void* const AAURLKeyCustomDomainState;
extern void* const AAURLKeyFlowType;
extern void* const AAURLKeyFlowTypeBeneficiary;
extern void* const AAURLKeyFlowTypeCustodian;
extern void* const AAURLKeyFlowTypeCustomDomain;
extern void* const AAURLKeyFlowTypeMailManage;
extern void* const AAURLKeyFlowTypePrivateEmailManage;
extern void* const AAURLKeyID;
extern void* const AAURLProtocolScheme;
extern void* const IDSRegistrationPropertySupportsCustodian;
extern void* const IDSRegistrationPropertyUnknown;
extern void* _AABeneficiaryClaimUIURLKey;
extern void* const _AAIDSServiceName;
extern void* const _AAMessageDestinationMailToPrefix;
extern void* const _AAMessageDestinationSelfTokenPrefix;
extern void* const _AAMessageDestinationTelToPrefix;
extern void* const _AAMessageDestinationTokenPrefix;
extern void* const _AAMessagesInviteContextURL;
extern void* const kAAAccountAlternateDSID;
extern void* const kAAAccountAppProvidedContextKey;
extern void* const kAAAccountAppleIDAliasesKey;
extern void* const kAAAccountAppleIDKey;
extern void* const kAAAccountBenefactorAccountKey;
extern void* const kAAAccountClassKey;
extern void* const kAAAccountCloudDocsMigrationComplete;
extern void* const kAAAccountDataclass;
extern void* const kAAAccountDataclassesDisabledForCellularKey;
extern void* const kAAAccountDataclassesPropertiesKey;
extern void* const kAAAccountEmailAddress;
extern void* const kAAAccountFirstNameKey;
extern void* const kAAAccountHasOptionalTerms;
extern void* const kAAAccountIsCloudSubscriber;
extern void* const kAAAccountIsFamilyEligible;
extern void* const kAAAccountIsManagedAppleID;
extern void* const kAAAccountIsSandboxAccount;
extern void* const kAAAccountLastAgreedTerms;
extern void* const kAAAccountLastKnownQuota;
extern void* const kAAAccountLastNameKey;
extern void* const kAAAccountMiddleNameKey;
extern void* const kAAAccountMobileMeStatusKey;
extern void* const kAAAccountNeedsToVerifyTerms;
extern void* const kAAAccountNotesMigrated;
extern void* const kAAAccountPasswordKey;
extern void* const kAAAccountPersonIDKey;
extern void* const kAAAccountPrimaryAccountKey;
extern void* const kAAAccountPrimaryAccountUpdaterVersion;
extern void* const kAAAccountPrimaryEmailKey;
extern void* const kAAAccountPrimaryEmailVerifiedKey;
extern void* const kAAAccountProtocolVersionKey;
extern void* const kAAAccountRegionInfoKey;
extern void* const kAAAccountRemindersAutoMigratableToCK;
extern void* const kAAAccountRemindersMigrated;
extern void* const kAAAccountRepairStateKey;
extern void* const kAAAccountRepairerPID;
extern void* const kAAAccountSyncedAccount;
extern void* const kAAAccountUbiquityPropertyiCloudHome;
extern void* const kAAAccountUsesCloudDocs;
extern void* const kAAAccountWarmingUp;
extern void* const kAAActionNameKey;
extern void* const kAAAltDSIDKey;
extern void* const kAACountryCodeKey;
extern void* const kAADeviceProvisioningMachineId;
extern void* const kAADeviceProvisioningOneTimePassword;
extern void* const kAAErrorDetailsActionButtonTitleKey;
extern void* const kAAErrorDetailsActionButtonTypeKey;
extern void* const kAAErrorDetailsActionButtonURLKey;
extern void* const kAAErrorDetailsDismissButtonTitleKey;
extern void* const kAAErrorDetailsMIDKey;
extern void* const kAAErrorDetailsMessageKey;
extern void* const kAAErrorDetailsOTPKey;
extern void* const kAAErrorDetailsProxiedDeviceDataKey;
extern void* const kAAErrorDetailsRoutingInfoKey;
extern void* const kAAErrorDetailsTitleKey;
extern void* const kAAMessagingServiceName;
extern void* const kAAMetadatanKey;
extern void* const kAAProcotolNewAccountUIURLKey;
extern void* const kAAProtocoEmailLookupURLKey;
extern void* const kAAProtocolAPSEnvKey;
extern void* const kAAProtocolAboutURLKey;
extern void* const kAAProtocolAcceptChildTransferURLKey;
extern void* const kAAProtocolAcceptFamilyInviteURLKey;
extern void* const kAAProtocolAcceptFamilyInviteV2URLKey;
extern void* const kAAProtocolAccountBetaKey;
extern void* const kAAProtocolAccountFirstDisplayAlertKey;
extern void* const kAAProtocolAccountFooterButtonKey;
extern void* const kAAProtocolAccountFooterTextKey;
extern void* const kAAProtocolAccountManagementUIURLKey;
extern void* const kAAProtocolAccountServiceTypeKey;
extern void* const kAAProtocolAccountUnavailableKey;
extern void* const kAAProtocolAddEmailURLKey;
extern void* const kAAProtocolAddFamilyMemberUIURLKey;
extern void* const kAAProtocolAltDSIDKey;
extern void* const kAAProtocolAppleAccountInfoKey;
extern void* const kAAProtocolAppleIDAliasesKey;
extern void* const kAAProtocolAppleIDEnteredByUserKey;
extern void* const kAAProtocolAppleIDKey;
extern void* const kAAProtocolAppleIDToken;
extern void* const kAAProtocolAuthTokenKey;
extern void* const kAAProtocolAuthenticateURLKey;
extern void* const kAAProtocolCheckiCloudMembershipURLKey;
extern void* const kAAProtocolChildAccountCreationUIURLKey;
extern void* const kAAProtocolClientIDKey;
extern void* const kAAProtocolCloudKitDevicesList;
extern void* const kAAProtocolCloudKitMigrationStateURLKey;
extern void* const kAAProtocolCloudKitStartMigrationURLKey;
extern void* const kAAProtocolCloudKitTokenKey;
extern void* const kAAProtocolCompleteValidateEmailURLKey;
extern void* const kAAProtocolCreateDelegatesURLKey;
extern void* const kAAProtocolDSIDKey;
extern void* const kAAProtocolDeclineFamilyInviteURLKey;
extern void* const kAAProtocolDelegatesKey;
extern void* const kAAProtocolDeviceHardwareModelKey;
extern void* const kAAProtocolDeviceInfoKey;
extern void* const kAAProtocolDeviceInfoSignatureKey;
extern void* const kAAProtocolDeviceListURLKey;
extern void* const kAAProtocolDeviceOSVersionKey;
extern void* const kAAProtocolDeviceQualificationURLKey;
extern void* const kAAProtocolDeviceRegionCode;
extern void* const kAAProtocolDeviceSerialNumberKey;
extern void* const kAAProtocolDeviceUDIDHeader;
extern void* const kAAProtocolDevicesUIURLKey;
extern void* const kAAProtocolDismissAlertURLKey;
extern void* const kAAProtocolDisplayedHostnameKey;
extern void* const kAAProtocolDomainURLsKey;
extern void* const kAAProtocolEmailToVet;
extern void* const kAAProtocolEmailVettingToken;
extern void* const kAAProtocolEnvironmentsKey;
extern void* const kAAProtocolErrorCancelButtonTitleKey;
extern void* const kAAProtocolErrorCommunicationsError;
extern void* const kAAProtocolErrorDescriptionKey;
extern void* const kAAProtocolErrorInvalidPassword;
extern void* const kAAProtocolErrorKey;
extern void* const kAAProtocolErrorMessageKey;
extern void* const kAAProtocolErrorMobileMeAccountDisabled;
extern void* const kAAProtocolErrorMobileMeAccountLapsed;
extern void* const kAAProtocolErrorMobileMeAccountMissingInfo;
extern void* const kAAProtocolErrorMobileMeCreateUnavailable;
extern void* const kAAProtocolErrorMobileMeInvalidSignature;
extern void* const kAAProtocolErrorMobileMeRegionDenied;
extern void* const kAAProtocolErrorMobileMeSuspendedMode;
extern void* const kAAProtocolErrorMobileMeTermsOfServiceUpdate;
extern void* const kAAProtocolErrorMobileMeUnsupportedDevice;
extern void* const kAAProtocolErrorResponseDictionaryKey;
extern void* const kAAProtocolErrorScheduledMaintenence;
extern void* const kAAProtocolErrorTitleKey;
extern void* const kAAProtocolExistingAppleIDTermsUIURLKey;
extern void* const kAAProtocolExperimentalModeHeaderKey;
extern void* const kAAProtocolFMFAppTokenKey;
extern void* const kAAProtocolFMFTokenKey;
extern void* const kAAProtocolFMIPAppTokenKey;
extern void* const kAAProtocolFMIPAuthenticateURLKey;
extern void* const kAAProtocolFMIPLostModeTokenKey;
extern void* const kAAProtocolFMIPSiriTokenKey;
extern void* const kAAProtocolFMIPTokenKey;
extern void* const kAAProtocolFMIPUrl;
extern void* const kAAProtocolFMIPWipeTokenKey;
extern void* const kAAProtocolFamilyEligibilityURLKey;
extern void* const kAAProtocolFamilyInviteSentV2URLKey;
extern void* const kAAProtocolFamilyLearnMoreURLKey;
extern void* const kAAProtocolFamilyPaymentCardUIURLKey;
extern void* const kAAProtocolFamilyPaymentInfoURLKey;
extern void* const kAAProtocolFamilySetupUIURLKey;
extern void* const kAAProtocolFamilyUIURLKey;
extern void* const kAAProtocolFetchFamilyInviteV2URLKey;
extern void* const kAAProtocolFinishProvisioningMachineKey;
extern void* const kAAProtocolFirstNameKey;
extern void* const kAAProtocolGameCenterDataKey;
extern void* const kAAProtocolGenericTermsUIBuddyMLKey;
extern void* const kAAProtocolGenericTermsUIPlistBuddyMLKey;
extern void* const kAAProtocolGenericTermsUIPlistTextKey;
extern void* const kAAProtocolGenericTermsURLKey;
extern void* const kAAProtocolGetAccountEventDetailsURLKey;
extern void* const kAAProtocolGetAccountSettingsURLKey;
extern void* const kAAProtocolGetDeviceListURLKey;
extern void* const kAAProtocolGetFamilyDetailsURLKey;
extern void* const kAAProtocolGetFamilyMemberPhotoURLKey;
extern void* const kAAProtocolGetFamilyPushDetailsURLKey;
extern void* const kAAProtocolGetMyPhotoURLKey;
extern void* const kAAProtocolGrandslamURLKey;
extern void* const kAAProtocolHasOptionalTermsKey;
extern void* const kAAProtocolIdentityServicesDataKey;
extern void* const kAAProtocolInitiateFamilyV2URLKey;
extern void* const kAAProtocolInitiateValidateEmailURLKey;
extern void* const kAAProtocolIsCloudDocsMigratedKey;
extern void* const kAAProtocolIsFamilyEligibleKey;
extern void* const kAAProtocolIsManagedAppleIDKey;
extern void* const kAAProtocolIsSandboxAccountKey;
extern void* const kAAProtocolKeyTransparencyTokenKey;
extern void* const kAAProtocolLanguageKey;
extern void* const kAAProtocolLastNameKey;
extern void* const kAAProtocolLoginAccountURLKey;
extern void* const kAAProtocolLoginDelegatesURLKey;
extern void* const kAAProtocolLoginOrCreateDelegatesURLKey;
extern void* const kAAProtocolLoginOrCreateURLKey;
extern void* const kAAProtocolMDMServerTokenKey;
extern void* const kAAProtocolMapsTokenKey;
extern void* const kAAProtocolMemberDetailsUIURLKey;
extern void* const kAAProtocolMobileMeDataKey;
extern void* const kAAProtocolMobileMeOfferAlertURLKey;
extern void* const kAAProtocolMobileMeOfferClassicMessageKey;
extern void* const kAAProtocolMobileMeOfferClassicTitleKey;
extern void* const kAAProtocolMobileMeOfferIdentifierKey;
extern void* const kAAProtocolMobileMeOfferMessageKey;
extern void* const kAAProtocolMobileMeOfferTitleKey;
extern void* const kAAProtocolMobileMeSetupTokenKey;
extern void* const kAAProtocolNewAccountURLKey;
extern void* const kAAProtocolNonQualifiedRegisterURLKey;
extern void* const kAAProtocolNotesMigratedKey;
extern void* const kAAProtocolPasswordKey;
extern void* const kAAProtocolPasswordSecurityUIURLKey;
extern void* const kAAProtocolPaymentSummaryUIURLKey;
extern void* const kAAProtocolPaymentUIURLKey;
extern void* const kAAProtocolPendingFamilyInvitesUIURLKey;
extern void* const kAAProtocolPersonIDKey;
extern void* const kAAProtocolPersonIDToken;
extern void* const kAAProtocolPersonalInfoUIURLKey;
extern void* const kAAProtocolPrefCustomHealthCheckIntervalEnabled;
extern void* const kAAProtocolPrefCustomHealthCheckIntervalMinutes;
extern void* const kAAProtocolPrefCustomHealthCheckReachabilityIntervalMinutes;
extern void* const kAAProtocolPrefCustomHealthCheckVersion;
extern void* const kAAProtocolPrefDisableHealthCheckTTRKey;
extern void* const kAAProtocolPrefExperimentalModeKey;
extern void* const kAAProtocolPrefMultipleFullAccounts;
extern void* const kAAProtocolPrefNeverSkipCustodianCheckKey;
extern void* const kAAProtocolPrefSendInternalDevHeaderKey;
extern void* const kAAProtocolPrefShowAccountContactsKey;
extern void* const kAAProtocolPrefSimulateUnhealthyCustodianKey;
extern void* const kAAProtocolPrefSuppressFireKey;
extern void* const kAAProtocolPrefUseLegacyLoginEndpoint;
extern void* const kAAProtocolPrimaryEmailKey;
extern void* const kAAProtocolPrimaryEmailVerifiedKey;
extern void* const kAAProtocolProvisionedDataclassesKey;
extern void* const kAAProtocolRegionInfoKey;
extern void* const kAAProtocolRegisterDeviceURLKey;
extern void* const kAAProtocolRegisterURLKey;
extern void* const kAAProtocolRejectChildTransferURLKey;
extern void* const kAAProtocolRemindersAutoMigratableToCKKey;
extern void* const kAAProtocolRemindersMigratedKey;
extern void* const kAAProtocolRepairCloudAccountUIURLKey;
extern void* const kAAProtocolSearchPartyTokenKey;
extern void* const kAAProtocolSecondaryAuthURLKey;
extern void* const kAAProtocolSendCodeURLKey;
extern void* const kAAProtocolServerResponseStatus;
extern void* const kAAProtocolServerResponseStatusMessage;
extern void* const kAAProtocolServiceDataKey;
extern void* const kAAProtocolSigningSessionCertificateURLKey;
extern void* const kAAProtocolSigningSessionURLKey;
extern void* const kAAProtocolStartFamilyInviteV2URLKey;
extern void* const kAAProtocolSuspendedInfoKey;
extern void* const kAAProtocolSuspensionFamilyKey;
extern void* const kAAProtocolSuspensioniCloudKey;
extern void* const kAAProtocolSyncMachineKey;
extern void* const kAAProtocolTermsReportUserActionURLKey;
extern void* const kAAProtocolTimezoneKey;
extern void* const kAAProtocolTokensKey;
extern void* const kAAProtocolUDIDKey;
extern void* const kAAProtocolUDIDToken;
extern void* const kAAProtocolURLsKey;
extern void* const kAAProtocolUnregisterDeviceURLKey;
extern void* const kAAProtocolUpdateAccountUIURLKey;
extern void* const kAAProtocolUpdateAccountURLKey;
extern void* const kAAProtocolUpdateMyPhotoURLKey;
extern void* const kAAProtocolUpdateNameURLKey;
extern void* const kAAProtocolUserInfoKey;
extern void* const kAAProtocolValidateURLKey;
extern void* const kAAProtocolVerifyCodeURLKey;
extern void* const kAAProtocolVersionKey;
extern void* const kAAProtocoliCloudAccountKey;
extern void* const kAAProtocoliCloudDataclassPrefix;
extern void* const kAAProtocoliForgotUIURLKey;
extern void* const kAASaveOptionAnisetteMIDKey;
extern void* const kAASaveOptionAnisetteOTPKey;
extern void* const kAASaveOptionAnisetteRoutingKey;
extern void* const kAASaveOptionCompanionDeviceClientInfoKey;
extern void* const kAASaveOptionCompanionDeviceUDIDKey;
extern void* const kAASaveOptionsTwoFactorLoginCookiesKey;
extern void* const kAASendWalrusWebAccessPCSAuthKey;
extern void* const kAATermsAgreeUrlKey;
extern void* const kAATermsFieldFormat;
extern void* const kAATermsFieldName;
extern void* const kAATermsFieldTerms;
extern void* const kAATermsVersionKey;
extern void* const kAATrueString;
extern void* const kAAiCloudVersionKey;
extern void* const kAppleIDMigratedAccountIdentifier;
extern void* const kLegacyAAAccountAppleIDAliasesKey;
extern void* const kLegacyAAAccountAuthTokenKey;
extern void* const kLegacyAAAccountDataclass;
extern void* const kLegacyAAAccountDataclassesDisabledForCellularKey;
extern void* const kLegacyAAAccountDataclassesPropertiesKey;
extern void* const kLegacyAAAccountFMIPTokenKey;
extern void* const kLegacyAAAccountFirstNameKey;
extern void* const kLegacyAAAccountLastNameKey;
extern void* const kLegacyAAAccountMobileMeStatusKey;
extern void* const kLegacyAAAccountPersonIDKey;
extern void* const kLegacyAAAccountPrimaryAccountKey;
extern void* const kLegacyAAAccountPrimaryEmailKey;
extern void* const kLegacyAAAccountPrimaryEmailVerifiedKey;
extern void* const kLegacyAAAccountProtocolVersionKey;

#endif

