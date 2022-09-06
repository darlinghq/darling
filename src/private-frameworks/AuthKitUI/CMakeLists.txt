project(AuthKitUI)

remove_sdk_framework(AuthKitUI
    PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(AuthKitUI
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/AuthKitUI"
	PRIVATE
)

add_framework(AuthKitUI
	FAT
	CURRENT_VERSION
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/AuthKitUI.m
		src/AKCurvePoint.m
		src/AKRoundedPath.m
		src/AKAuthorizationSubPaneLoginChoice.m
		src/AKNavigationController.m
		src/AKAppleIDAuthenticationUISurrogateContext.m
		src/AKiCDPValidationDevice.m
		src/AKAuthorizationPresenterViewController.m
		src/AKAuthorizationSubPane.m
		src/AKAppleIDAuthenticationMacOSExtenstionContext.m
		src/AKAuthorizationRadioButtonView.m
		src/AKIDPProvidedSignInViewController.m
		src/AKIDPProvidedSignInWindow.m
		src/AKAuthorizationSubPanePrivacyLink.m
		src/AKWebViewButtonBar.m
		src/AKDeviceInfo.m
		src/AKURLRequestApprover.m
		src/AKAuthorizationWindowController.m
		src/AKAppleIDAuthenticationiCloudPrefPaneViewController.m
		src/AKTrustedDeviceViewItem.m
		src/AKBarButtonItem.m
		src/AKAuthorizationFirstTimePaneViewController.m
		src/AKAuthorizationEmailScopeViewController.m
		src/AKICAWebKitViewController.m
		src/AKiCSCEntryView.m
		src/AKIcon.m
		src/AKAuthorizationButton.m
		src/AKAuthorizationPasswordAuthenticationViewController.m
		src/AKAppleIDAuthenticationInAssistantContext.m
		src/AKPaddedTextFieldCell.m
		src/AKSecondFactorCodeEntryController.m
		src/AKShortInlineSignInViewController.m
		src/AKAuthorizationPasswordRequestViewController.m
		src/AKGeneratedComplexCodeFormatter.m
		src/AKPinFieldView.m
		src/AKPinFieldBoxUIElement.m
		src/AKKeepUsingController.m
		src/AKAuthenticationPromptController.m
		src/AKAuthorizationScopeChoices.m
		src/AKAuthWebTabView.m
		src/AKAuthorizationSubPaneSeparator.m
		src/AKLegacyInlineSignInViewController.m
		src/AKiCDPRecoveryKeyViewController.m
		src/AKCAReporter.m
		src/AKTiburonRequestReport.m
		src/AKTiburonPasswordUIReport.m
		src/AKTiburonAuthorizationUIReport.m
		src/AKTiburonRequestAttemptReport.m
		src/AKSatoriReport.m
		src/AKAuthorizationConfirmPaneViewController.m
		src/AKAuthorizationSubPaneConfirmButton.m
		src/AKAppleIDAuthenticationiCloudPrefPaneContext.m
		src/AKDateFormatter.m
		src/AKTouchBarController.m
		src/AKiCDPWaitForApprovalViewController.m
		src/AKBaseSignInViewController.m
		src/AKAuthorizationPaneContext.m
		src/AKAuthorizationNavigationController.m
		src/AKAppleIDServerUIContextController.m
		src/AKInfoLabel.m
		src/AKAuthorizationSubPaneInfoLabel.m
		src/AKiCDPEnrollmentDeviceListViewController.m
		src/AKAccessibleContainerView.m
		src/AKAppleIDServerUIDataHarvester.m
		src/AKAppleIDAuthenticationiCloudPrefPaneSecondFactorViewController.m
		src/AKAuthorizationLoginChoiceView.m
		src/AKWebKitController.m
		src/AKNameTextField.m
		src/AKAuthorizationNameScopeViewController.m
		src/AKAuthorizationNameScopeView.m
		src/AKNavigationItem.m
		src/AKiCDPDeviceValidationFlowViewController.m
		src/AKBasicLoginActions.m
		src/AKBasicLoginSecondFactorActions.m
		src/AKIDPHandler.m
		src/AKInAppAuthenticationMacOSUIProvider.m
		src/AKAppleIDAuthenticationInAppContext.m
		src/AKInAssistantSecondFactorCodeEntry.m
		src/AKNoCodeReceivedController.m
		src/AKRoundLoginContainerView.m
		src/AKAuthorizationViewController.m
		src/AKAuthorizationFirstTimeView.m
		src/AKSignInPromptController.m
		src/AKAuthorizationPaneViewController.m
		src/AKAuthorizationRightUI.m
		src/AKAuthorizationUIProviderImp.m
		src/AKiCDPEnrollmentDevicePasscodeViewController.m
		src/AKTapToSignInViewController.m
		src/AKAuthorizationNameFormatter.m
		src/AKMaxLengthFormatter.m
		src/AKTooManyAttemptsController.m
		src/AKAuthorizationSubPaneImage.m
		src/AKAuthorizationViewHelper.m
		src/AKInlineSignInViewController.m
		src/AKTextualLinkButton.m

	DEPENDENCIES
		system
		objc
		Foundation
)
