/*
 This file is part of Darling.

 Copyright (C) 2022 Darling Team

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

#ifndef _AUTHENTICATIONSERVICES_H_
#define _AUTHENTICATIONSERVICES_H_

#import <Foundation/Foundation.h>

#import <AuthenticationServices/AKAdaptiveServiceInterface.h>
#import <AuthenticationServices/AKAuthorizationCredential+AuthenticationServicesExtras.h>
#import <AuthenticationServices/AKAuthorizationNotificationHandler.h>
#import <AuthenticationServices/ASAuthorization.h>
#import <AuthenticationServices/ASAuthorizationAppleIDButton.h>
#import <AuthenticationServices/ASAuthorizationAppleIDCredential.h>
#import <AuthenticationServices/ASAuthorizationAppleIDProvider.h>
#import <AuthenticationServices/ASAuthorizationAppleIDRequest.h>
#import <AuthenticationServices/ASAuthorizationController.h>
#import <AuthenticationServices/ASAuthorizationControllerDelegate.h>
#import <AuthenticationServices/ASAuthorizationControllerPresentationContextProviding.h>
#import <AuthenticationServices/ASAuthorizationCredential.h>
#import <AuthenticationServices/ASAuthorizationOpenIDRequest.h>
#import <AuthenticationServices/ASAuthorizationPasswordProvider.h>
#import <AuthenticationServices/ASAuthorizationPasswordRequest.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialAssertion.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialAssertionRequest.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialDescriptor.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialProvider.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialRegistration.h>
#import <AuthenticationServices/ASAuthorizationPlatformPublicKeyCredentialRegistrationRequest.h>
#import <AuthenticationServices/ASAuthorizationProvider.h>
#import <AuthenticationServices/ASAuthorizationProviderExtensionAuthorizationRequest.h>
#import <AuthenticationServices/ASAuthorizationProviderExtensionAuthorizationResult.h>
#import <AuthenticationServices/ASAuthorizationProviderExtensionKerberosMapping.h>
#import <AuthenticationServices/ASAuthorizationProviderExtensionLoginConfiguration.h>
#import <AuthenticationServices/ASAuthorizationProviderExtensionLoginManager.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialAssertion.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialAssertionRequest.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialDescriptor.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialParameters.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialRegistration.h>
#import <AuthenticationServices/ASAuthorizationPublicKeyCredentialRegistrationRequest.h>
#import <AuthenticationServices/ASAuthorizationRequest.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialAssertion.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialAssertionRequest.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialDescriptor.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialProvider.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialRegistration.h>
#import <AuthenticationServices/ASAuthorizationSecurityKeyPublicKeyCredentialRegistrationRequest.h>
#import <AuthenticationServices/ASAuthorizationServiceViewController.h>
#import <AuthenticationServices/ASAuthorizationSingleSignOnCredential.h>
#import <AuthenticationServices/ASAuthorizationSingleSignOnProvider.h>
#import <AuthenticationServices/ASAuthorizationSingleSignOnRequest.h>
#import <AuthenticationServices/ASAuthorizationViewController.h>
#import <AuthenticationServices/ASAuthorizationViewControllerDelegate.h>
#import <AuthenticationServices/ASBarButtonItem.h>
#import <AuthenticationServices/ASCAuthorizationPresenterHostProtocol.h>
#import <AuthenticationServices/ASCViewServiceProtocol.h>
#import <AuthenticationServices/ASCredentialIdentityStore.h>
#import <AuthenticationServices/ASCredentialIdentityStoreState.h>
#import <AuthenticationServices/ASCredentialPickerPaneViewController.h>
#import <AuthenticationServices/ASCredentialProviderExtensionContext.h>
#import <AuthenticationServices/ASCredentialProviderViewController.h>
#import <AuthenticationServices/ASCredentialRequestBasicPaneViewController.h>
#import <AuthenticationServices/ASCredentialRequestButton.h>
#import <AuthenticationServices/ASCredentialRequestButtonContinue.h>
#import <AuthenticationServices/ASCredentialRequestCABLEClientViewController.h>
#import <AuthenticationServices/ASCredentialRequestCABLEConnectingViewController.h>
#import <AuthenticationServices/ASCredentialRequestConfirmButtonSubPane.h>
#import <AuthenticationServices/ASCredentialRequestContainerViewController.h>
#import <AuthenticationServices/ASCredentialRequestContainerViewControllerDelegate.h>
#import <AuthenticationServices/ASCredentialRequestEnableBluetoothViewController.h>
#import <AuthenticationServices/ASCredentialRequestIconGenerator.h>
#import <AuthenticationServices/ASCredentialRequestImageSubPane.h>
#import <AuthenticationServices/ASCredentialRequestInfoLabelSubPane.h>
#import <AuthenticationServices/ASCredentialRequestLoginChoiceCell.h>
#import <AuthenticationServices/ASCredentialRequestPaneContext.h>
#import <AuthenticationServices/ASCredentialRequestPaneViewController.h>
#import <AuthenticationServices/ASCredentialRequestPaneViewControllerDelegate.h>
#import <AuthenticationServices/ASCredentialRequestSecondaryButton.h>
#import <AuthenticationServices/ASCredentialRequestSecurityKeyStringUtilities.h>
#import <AuthenticationServices/ASCredentialRequestSubPane.h>
#import <AuthenticationServices/ASCredentialRequestSubPaneConfirmButtonDelegate.h>
#import <AuthenticationServices/ASCredentialServiceIdentifier.h>
#import <AuthenticationServices/ASExtensionRemoteViewController.h>
#import <AuthenticationServices/ASExtensionServiceProtocol.h>
#import <AuthenticationServices/ASExtensionServiceViewController.h>
#import <AuthenticationServices/ASNavigationController.h>
#import <AuthenticationServices/ASNavigationControllerDelegate.h>
#import <AuthenticationServices/ASNavigationItem.h>
#import <AuthenticationServices/ASPasswordAuthenticationPaneViewController.h>
#import <AuthenticationServices/ASPasswordAuthenticationPaneViewControllerDelegate.h>
#import <AuthenticationServices/ASPasswordCredential.h>
#import <AuthenticationServices/ASPasswordCredentialIdentity.h>
#import <AuthenticationServices/ASPublicKeyCredential.h>
#import <AuthenticationServices/ASPublicKeyCredentialBase.h>
#import <AuthenticationServices/ASPublicKeyCredentialManager.h>
#import <AuthenticationServices/ASPublicKeyCredentialManagerInterface.h>
#import <AuthenticationServices/ASPublicKeyCredentialOperation.h>
#import <AuthenticationServices/ASViewServiceInterfaceUtilities.h>
#import <AuthenticationServices/ASWebAuthenticationSession.h>
#import <AuthenticationServices/ASWebAuthenticationSessionController.h>
#import <AuthenticationServices/ASWebAuthenticationSessionLaunchAgentProxy.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequest.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequestClient.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequestDelegate.h>
#import <AuthenticationServices/ASWebAuthenticationSessionRequestServing.h>
#import <AuthenticationServices/ASWebAuthenticationSessionWebBrowserSessionManager.h>
#import <AuthenticationServices/AuthenticationServicesHelperProtocol.h>
#import <AuthenticationServices/LAUIAuthenticationDelegate.h>
#import <AuthenticationServices/LAUIAuthenticationViewController+AuthenticationServicesExtras.h>
#import <AuthenticationServices/NSAccessibilityButton.h>
#import <AuthenticationServices/NSAccessibilityElement.h>
#import <AuthenticationServices/NSBundle+ASNSBundleExtras.h>
#import <AuthenticationServices/NSControlTextEditingDelegate.h>
#import <AuthenticationServices/NSExtension+AuthenticationServicesExtras.h>
#import <AuthenticationServices/NSImage+ASImageExtras.h>
#import <AuthenticationServices/NSLayoutConstraint+ASNSLayoutConstraintExtras.h>
#import <AuthenticationServices/NSPasteboard+AuthenticationServicesExtras.h>
#import <AuthenticationServices/NSStackView+ASCredentialPickerPaneViewController.h>
#import <AuthenticationServices/NSTableViewDataSource.h>
#import <AuthenticationServices/NSTableViewDelegate.h>
#import <AuthenticationServices/NSViewController+AuthenticationServicesExtras.h>
#import <AuthenticationServices/NSXPCInterface+AuthenticationServicesExtras.h>
#import <AuthenticationServices/NSXPCListenerDelegate.h>
#import <AuthenticationServices/SOAuthorizationDelegate.h>
#import <AuthenticationServices/WBSSavedAccountStore+PasskeyExtras.h>
#import <AuthenticationServices/WBSSavedAccountStorePasskeyExtras.h>
#import <AuthenticationServices/_ASAuthenticationPresentationProvider.h>
#import <AuthenticationServices/_ASCABLEQRCodeBadgePlatterView.h>
#import <AuthenticationServices/_ASCABLEQRCodeBadgeView.h>
#import <AuthenticationServices/_ASCABLEQRCodeImageView.h>
#import <AuthenticationServices/_ASCABLEQRCodeView.h>
#import <AuthenticationServices/_ASCredentialListViewController.h>
#import <AuthenticationServices/_ASCredentialProviderExtensionConfigurationViewController.h>
#import <AuthenticationServices/_ASCredentialProviderExtensionContextProtocol.h>
#import <AuthenticationServices/_ASCredentialProviderExtensionHostContext.h>
#import <AuthenticationServices/_ASCredentialProviderExtensionHostContextDelegate.h>
#import <AuthenticationServices/_ASCredentialProviderExtensionHostContextProtocol.h>
#import <AuthenticationServices/_ASExtensionHostContext.h>
#import <AuthenticationServices/_ASExtensionHostContextDelegate.h>
#import <AuthenticationServices/_ASExtensionViewController.h>
#import <AuthenticationServices/_ASIncomingCallObserver.h>
#import <AuthenticationServices/_ASPasskeysLearnMoreContent.h>
#import <AuthenticationServices/_ASPasswordCredentialAuthenticationViewController.h>
#import <AuthenticationServices/_ASPasswordManagerDeleteAlertConfiguration.h>
#import <AuthenticationServices/_ASPasswordManagerIconController.h>
#import <AuthenticationServices/_ASPasswordManagerMacFrameworksTrampoline.h>
#import <AuthenticationServices/_ASStackSpacerView.h>
#import <AuthenticationServices/_ASWebAuthenticationSessionRequestHandling.h>
#import <AuthenticationServices/_ASWebAuthenticationSessionRequestServer.h>
#import <AuthenticationServices/_SafariLaunchAgentProtocol.h>
#import <AuthenticationServices/_WKWebAuthenticationPanelDelegate.h>

#endif

