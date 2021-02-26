/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _AuthKitUI_H_
#define _AuthKitUI_H_

#import <Foundation/Foundation.h>

#import <AuthKitUI/AKAppleIDAuthenticationDelegate.h>
#import <AuthKitUI/AKAppleIDAuthenticationInAppContextAlertDelegate.h>
#import <AuthKitUI/AKAppleIDAuthenticationInAppContextPasswordDelegate.h>
#import <AuthKitUI/AKAppleIDAuthenticationLimitedUIProvider.h>
#import <AuthKitUI/AKAppleIDAuthenticationSurrogateUIProvider.h>
#import <AuthKitUI/AKAppleIDAuthenticationUIProvider.h>
#import <AuthKitUI/AKAppleIDServerAuthenticationUIProvider.h>
#import <AuthKitUI/AKAuthorizationEditableDataSources.h>
#import <AuthKitUI/AKAuthorizationEmailScopeViewControllerDelegate.h>
#import <AuthKitUI/AKAuthorizationNameScopeViewControllerDelegate.h>
#import <AuthKitUI/AKAuthorizationPaneAutomaticResizeDelegate.h>
#import <AuthKitUI/AKAuthorizationPaneViewControllerDelegate.h>
#import <AuthKitUI/AKAuthorizationPasswordAuthenticationDelegate.h>
#import <AuthKitUI/AKAuthorizationSubPaneAdding.h>
#import <AuthKitUI/AKAuthorizationSubPaneConfirmButtonDelegate.h>
#import <AuthKitUI/AKAuthorizationSubPaneLoginChoiceDelegate.h>
#import <AuthKitUI/AKAuthorizationUIProvider.h>
#import <AuthKitUI/AKAuthorizationViewHostDelegate.h>
#import <AuthKitUI/AKAuthorizationViewRemoteDelegate.h>
#import <AuthKitUI/AKBasicLoginAnimateProtocol.h>
#import <AuthKitUI/AKICAUIDelegate.h>
#import <AuthKitUI/AKICAWebKitViewControllerDelegate.h>
#import <AuthKitUI/AKIDPPresentationController.h>
#import <AuthKitUI/AKInAppAuthenticationUIProvider.h>
#import <AuthKitUI/AKInAssistantNavigationHandler.h>
#import <AuthKitUI/AKMBICAUIDelegate.h>
#import <AuthKitUI/AKNameTextFieldDelegate.h>
#import <AuthKitUI/AKPinFieldViewDelegate.h>
#import <AuthKitUI/AKSecondFactorCodeEntryDelegate.h>
#import <AuthKitUI/AKWebKitControllerDelegate.h>
#import <AuthKitUI/AKWebViewButtonBarDelegate.h>
#import <AuthKitUI/AKiCDPRecoveryKeyDelegate.h>
#import <AuthKitUI/AKiCDPValidationDelegate.h>
#import <AuthKitUI/AKiCSCEntryViewDelegate.h>
#import <AuthKitUI/AuthWebViewDelegate.h>
#import <AuthKitUI/InternetAccountExportsAuthKit.h>
#import <AuthKitUI/JSExport.h>
#import <AuthKitUI/MBUIDelegate.h>
#import <AuthKitUI/MBWebKitViewControllerDelegate.h>
#import <AuthKitUI/NSControlTextEditingDelegate.h>
#import <AuthKitUI/NSTableViewDelegate.h>
#import <AuthKitUI/NSTextFieldDelegate.h>
#import <AuthKitUI/NSTouchBarProvider.h>
#import <AuthKitUI/WKNavigationDelegate.h>
#import <AuthKitUI/WebFrameLoadDelegate.h>
#import <AuthKitUI/WebResourceLoadDelegate.h>
#import <AuthKitUI/WebUIDelegate.h>
#import <AuthKitUI/AKCurvePoint.h>
#import <AuthKitUI/AKRoundedPath.h>
#import <AuthKitUI/AKAuthorizationSubPaneLoginChoice.h>
#import <AuthKitUI/AKNavigationController.h>
#import <AuthKitUI/AKAppleIDAuthenticationUISurrogateContext.h>
#import <AuthKitUI/AKiCDPValidationDevice.h>
#import <AuthKitUI/AKAuthorizationPresenterViewController.h>
#import <AuthKitUI/AKAuthorizationSubPane.h>
#import <AuthKitUI/AKAppleIDAuthenticationMacOSExtenstionContext.h>
#import <AuthKitUI/AKAuthorizationRadioButtonView.h>
#import <AuthKitUI/AKIDPProvidedSignInViewController.h>
#import <AuthKitUI/AKIDPProvidedSignInWindow.h>
#import <AuthKitUI/AKAuthorizationSubPanePrivacyLink.h>
#import <AuthKitUI/AKWebViewButtonBar.h>
#import <AuthKitUI/AKDeviceInfo.h>
#import <AuthKitUI/AKURLRequestApprover.h>
#import <AuthKitUI/AKAuthorizationWindowController.h>
#import <AuthKitUI/AKAppleIDAuthenticationiCloudPrefPaneViewController.h>
#import <AuthKitUI/AKTrustedDeviceViewItem.h>
#import <AuthKitUI/AKBarButtonItem.h>
#import <AuthKitUI/AKAuthorizationFirstTimePaneViewController.h>
#import <AuthKitUI/AKAuthorizationEmailScopeViewController.h>
#import <AuthKitUI/AKICAWebKitViewController.h>
#import <AuthKitUI/AKiCSCEntryView.h>
#import <AuthKitUI/AKIcon.h>
#import <AuthKitUI/AKAuthorizationButton.h>
#import <AuthKitUI/AKAuthorizationPasswordAuthenticationViewController.h>
#import <AuthKitUI/AKAppleIDAuthenticationInAssistantContext.h>
#import <AuthKitUI/AKPaddedTextFieldCell.h>
#import <AuthKitUI/AKSecondFactorCodeEntryController.h>
#import <AuthKitUI/AKShortInlineSignInViewController.h>
#import <AuthKitUI/AKAuthorizationPasswordRequestViewController.h>
#import <AuthKitUI/AKGeneratedComplexCodeFormatter.h>
#import <AuthKitUI/AKPinFieldView.h>
#import <AuthKitUI/AKPinFieldBoxUIElement.h>
#import <AuthKitUI/AKKeepUsingController.h>
#import <AuthKitUI/AKAuthenticationPromptController.h>
#import <AuthKitUI/AKAuthorizationScopeChoices.h>
#import <AuthKitUI/AKAuthWebTabView.h>
#import <AuthKitUI/AKAuthorizationSubPaneSeparator.h>
#import <AuthKitUI/AKLegacyInlineSignInViewController.h>
#import <AuthKitUI/AKiCDPRecoveryKeyViewController.h>
#import <AuthKitUI/AKCAReporter.h>
#import <AuthKitUI/AKTiburonRequestReport.h>
#import <AuthKitUI/AKTiburonPasswordUIReport.h>
#import <AuthKitUI/AKTiburonAuthorizationUIReport.h>
#import <AuthKitUI/AKTiburonRequestAttemptReport.h>
#import <AuthKitUI/AKSatoriReport.h>
#import <AuthKitUI/AKAuthorizationConfirmPaneViewController.h>
#import <AuthKitUI/AKAuthorizationSubPaneConfirmButton.h>
#import <AuthKitUI/AKAppleIDAuthenticationiCloudPrefPaneContext.h>
#import <AuthKitUI/AKDateFormatter.h>
#import <AuthKitUI/AKTouchBarController.h>
#import <AuthKitUI/AKiCDPWaitForApprovalViewController.h>
#import <AuthKitUI/AKBaseSignInViewController.h>
#import <AuthKitUI/AKAuthorizationPaneContext.h>
#import <AuthKitUI/AKAuthorizationNavigationController.h>
#import <AuthKitUI/AKAppleIDServerUIContextController.h>
#import <AuthKitUI/AKInfoLabel.h>
#import <AuthKitUI/AKAuthorizationSubPaneInfoLabel.h>
#import <AuthKitUI/AKiCDPEnrollmentDeviceListViewController.h>
#import <AuthKitUI/AKAccessibleContainerView.h>
#import <AuthKitUI/AKAppleIDServerUIDataHarvester.h>
#import <AuthKitUI/AKAppleIDAuthenticationiCloudPrefPaneSecondFactorViewController.h>
#import <AuthKitUI/AKAuthorizationLoginChoiceView.h>
#import <AuthKitUI/AKWebKitController.h>
#import <AuthKitUI/AKNameTextField.h>
#import <AuthKitUI/AKAuthorizationNameScopeViewController.h>
#import <AuthKitUI/AKAuthorizationNameScopeView.h>
#import <AuthKitUI/AKNavigationItem.h>
#import <AuthKitUI/AKiCDPDeviceValidationFlowViewController.h>
#import <AuthKitUI/AKBasicLoginActions.h>
#import <AuthKitUI/AKBasicLoginSecondFactorActions.h>
#import <AuthKitUI/AKIDPHandler.h>
#import <AuthKitUI/AKInAppAuthenticationMacOSUIProvider.h>
#import <AuthKitUI/AKAppleIDAuthenticationInAppContext.h>
#import <AuthKitUI/AKInAssistantSecondFactorCodeEntry.h>
#import <AuthKitUI/AKNoCodeReceivedController.h>
#import <AuthKitUI/AKRoundLoginContainerView.h>
#import <AuthKitUI/AKAuthorizationViewController.h>
#import <AuthKitUI/AKAuthorizationFirstTimeView.h>
#import <AuthKitUI/AKSignInPromptController.h>
#import <AuthKitUI/AKAuthorizationPaneViewController.h>
#import <AuthKitUI/AKAuthorizationRightUI.h>
#import <AuthKitUI/AKAuthorizationUIProviderImp.h>
#import <AuthKitUI/AKiCDPEnrollmentDevicePasscodeViewController.h>
#import <AuthKitUI/AKTapToSignInViewController.h>
#import <AuthKitUI/AKAuthorizationNameFormatter.h>
#import <AuthKitUI/AKMaxLengthFormatter.h>
#import <AuthKitUI/AKTooManyAttemptsController.h>
#import <AuthKitUI/AKAuthorizationSubPaneImage.h>
#import <AuthKitUI/AKAuthorizationViewHelper.h>
#import <AuthKitUI/AKInlineSignInViewController.h>
#import <AuthKitUI/AKTextualLinkButton.h>

void* AKColorSpaceStandardRGB(void);
void* AKContentAlignmentMake(void);
void* AKCreateAppleIDButtonImage(void);
void* AKCreateAppleIDButtonImageWithCornerRadius(void);
void* AKDrawAppleIDButton(void);
void* AKDrawAppleIDButtonWithCornerRadius(void);
void* AKFloatRoundToPixelWithScale(void);
void* AKPointRoundToPixelWithScale(void);
void* AKRectRoundToPixelWithScale(void);
void* AKSizeAlignedInRectWithScale(void);
void* AKSizeRoundToPixelWithScale(void);
void* AKSizeScaleAspectFit(void);
void* AddGlobalObject(void);
void* CallJSFunctionWithParameters(void);
void* EvaluateJavaScript(void);
void* JSCopyStringValue(void);
void* JSValueFromNSString(void);
void* JSValueRefsAreEqual(void);
void* NSObjectFromJSValue(void);
void* NSStringFromJSString(void);
void* NSStringFromJSValue(void);
void* _AKUIMainBundle(void);
void* _CreateAttributedStringWithFontSize(void);
void* _DrawBorder(void);
void* _DrawLogoPDFIntoContext(void);
void* _IsJSValueAnArray(void);
void* _MaximumFontSizeForStringInRect(void);
void* _NSDictionaryFromJSObject(void);
void* vsCallJSFunction(void);

#endif
