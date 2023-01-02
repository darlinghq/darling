project(AuthenticationServices)

remove_sdk_framework(AuthenticationServices)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "614.3.7")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AuthenticationServices
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/AuthenticationServices"
)


add_framework(AuthenticationServices
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/AKAuthorizationCredential+AuthenticationServicesExtras.m
		src/ASAuthorization.m
		src/ASAuthorizationAppleIDButton.m
		src/ASAuthorizationAppleIDCredential.m
		src/ASAuthorizationAppleIDProvider.m
		src/ASAuthorizationAppleIDRequest.m
		src/ASAuthorizationController.m
		src/ASAuthorizationOpenIDRequest.m
		src/ASAuthorizationPasswordProvider.m
		src/ASAuthorizationPasswordRequest.m
		src/ASAuthorizationPlatformPublicKeyCredentialAssertion.m
		src/ASAuthorizationPlatformPublicKeyCredentialAssertionRequest.m
		src/ASAuthorizationPlatformPublicKeyCredentialDescriptor.m
		src/ASAuthorizationPlatformPublicKeyCredentialProvider.m
		src/ASAuthorizationPlatformPublicKeyCredentialRegistration.m
		src/ASAuthorizationPlatformPublicKeyCredentialRegistrationRequest.m
		src/ASAuthorizationProviderExtensionAuthorizationRequest.m
		src/ASAuthorizationProviderExtensionAuthorizationResult.m
		src/ASAuthorizationProviderExtensionKerberosMapping.m
		src/ASAuthorizationProviderExtensionLoginConfiguration.m
		src/ASAuthorizationProviderExtensionLoginManager.m
		src/ASAuthorizationPublicKeyCredentialParameters.m
		src/ASAuthorizationRequest.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialAssertion.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialAssertionRequest.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialDescriptor.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialProvider.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialRegistration.m
		src/ASAuthorizationSecurityKeyPublicKeyCredentialRegistrationRequest.m
		src/ASAuthorizationServiceViewController.m
		src/ASAuthorizationSingleSignOnCredential.m
		src/ASAuthorizationSingleSignOnProvider.m
		src/ASAuthorizationSingleSignOnRequest.m
		src/ASAuthorizationViewController.m
		src/ASBarButtonItem.m
		src/ASCredentialIdentityStore.m
		src/ASCredentialIdentityStoreState.m
		src/ASCredentialPickerPaneViewController.m
		src/ASCredentialProviderExtensionContext.m
		src/ASCredentialProviderViewController.m
		src/ASCredentialRequestBasicPaneViewController.m
		src/ASCredentialRequestButton.m
		src/ASCredentialRequestButtonContinue.m
		src/ASCredentialRequestCABLEClientViewController.m
		src/ASCredentialRequestCABLEConnectingViewController.m
		src/ASCredentialRequestConfirmButtonSubPane.m
		src/ASCredentialRequestContainerViewController.m
		src/ASCredentialRequestEnableBluetoothViewController.m
		src/ASCredentialRequestIconGenerator.m
		src/ASCredentialRequestImageSubPane.m
		src/ASCredentialRequestInfoLabelSubPane.m
		src/ASCredentialRequestLoginChoiceCell.m
		src/ASCredentialRequestPaneContext.m
		src/ASCredentialRequestPaneViewController.m
		src/ASCredentialRequestSecondaryButton.m
		src/ASCredentialRequestSecurityKeyStringUtilities.m
		src/ASCredentialRequestSubPane.m
		src/ASCredentialServiceIdentifier.m
		src/ASExtensionRemoteViewController.m
		src/ASExtensionServiceViewController.m
		src/ASNavigationController.m
		src/ASNavigationItem.m
		src/ASPasswordAuthenticationPaneViewController.m
		src/ASPasswordCredential.m
		src/ASPasswordCredentialIdentity.m
		src/ASPublicKeyCredentialBase.m
		src/ASPublicKeyCredentialManager.m
		src/ASPublicKeyCredentialOperation.m
		src/ASViewServiceInterfaceUtilities.m
		src/ASWebAuthenticationSession.m
		src/ASWebAuthenticationSessionController.m
		src/ASWebAuthenticationSessionLaunchAgentProxy.m
		src/ASWebAuthenticationSessionRequest.m
		src/ASWebAuthenticationSessionRequestClient.m
		src/ASWebAuthenticationSessionWebBrowserSessionManager.m
		src/LAUIAuthenticationViewController+AuthenticationServicesExtras.m
		src/NSBundle+ASNSBundleExtras.m
		src/NSExtension+AuthenticationServicesExtras.m
		src/NSImage+ASImageExtras.m
		src/NSLayoutConstraint+ASNSLayoutConstraintExtras.m
		src/NSPasteboard+AuthenticationServicesExtras.m
		src/NSStackView+ASCredentialPickerPaneViewController.m
		src/NSViewController+AuthenticationServicesExtras.m
		src/NSXPCInterface+AuthenticationServicesExtras.m
		src/WBSSavedAccountStore+PasskeyExtras.m
		src/_ASCABLEQRCodeBadgePlatterView.m
		src/_ASCABLEQRCodeBadgeView.m
		src/_ASCABLEQRCodeImageView.m
		src/_ASCABLEQRCodeView.m
		src/_ASCredentialListViewController.m
		src/_ASCredentialProviderExtensionConfigurationViewController.m
		src/_ASCredentialProviderExtensionHostContext.m
		src/_ASExtensionViewController.m
		src/_ASIncomingCallObserver.m
		src/_ASPasskeysLearnMoreContent.m
		src/_ASPasswordCredentialAuthenticationViewController.m
		src/_ASPasswordManagerDeleteAlertConfiguration.m
		src/_ASPasswordManagerIconController.m
		src/_ASPasswordManagerMacFrameworksTrampoline.m
		src/_ASStackSpacerView.m
		src/_ASWebAuthenticationSessionRequestServer.m

	DEPENDENCIES
		system
		objc
		Foundation
		AppKit
)