/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#import <CoreGraphics/CGGeometry.h>

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

NSString *const IBCocoaFramework = @"IBCocoaFramework";
NSString *const NSAccessibilityActivationPointAttribute = @"NSAccessibilityActivationPointAttribute";
NSString *const NSAccessibilityAnnouncementKey = @"NSAccessibilityAnnouncementKey";
NSString *const NSAccessibilityAnnouncementRequestedNotification = @"NSAccessibilityAnnouncementRequestedNotification";
NSString *const NSAccessibilityBackgroundColorTextAttribute = @"NSAccessibilityBackgroundColorTextAttribute";
NSString *const NSAccessibilityButtonRole = @"NSAccessibilityButtonRole";
NSString *const NSAccessibilityCancelAction = @"NSAccessibilityCancelAction";
NSString *const NSAccessibilityCellRole = @"NSAccessibilityCellRole";
NSString *const NSAccessibilityCheckBoxRole = @"NSAccessibilityCheckBoxRole";
NSString *const NSAccessibilityChildrenAttribute = @"NSAccessibilityChildrenAttribute";
NSString *const NSAccessibilityComboBoxRole = @"NSAccessibilityComboBoxRole";
NSString *const NSAccessibilityCreatedNotification = @"NSAccessibilityCreatedNotification";
NSString *const NSAccessibilityDecrementAction = @"NSAccessibilityDecrementAction";
NSString *const NSAccessibilityDescriptionAttribute = @"NSAccessibilityDescriptionAttribute";
NSString *const NSAccessibilityDisclosureTriangleRole = @"NSAccessibilityDisclosureTriangleRole";
NSString *const NSAccessibilityEnabledAttribute = @"NSAccessibilityEnabledAttribute";
NSString *const NSAccessibilityEventProcessedNotification = @"NSAccessibilityEventProcessedNotification";
NSString *const NSAccessibilityEventSourceProcessIDKey = @"NSAccessibilityEventSourceProcessIDKey";
NSString *const NSAccessibilityEventSourceStateIDKey = @"NSAccessibilityEventSourceStateIDKey";
NSString *const NSAccessibilityEventSourceUserDataKey = @"NSAccessibilityEventSourceUserDataKey";
NSString *const NSAccessibilityFocusedAttribute = @"NSAccessibilityFocusedAttribute";
NSString *const NSAccessibilityFocusedUIElementAttribute = @"NSAccessibilityFocusedUIElementAttribute";
NSString *const NSAccessibilityFontTextAttribute = @"NSAccessibilityFontTextAttribute";
NSString *const NSAccessibilityForegroundColorTextAttribute = @"NSAccessibilityForegroundColorTextAttribute";
NSString *const NSAccessibilityGroupRole = @"NSAccessibilityGroupRole";
NSString *const NSAccessibilityHelpAttribute = @"NSAccessibilityHelpAttribute";
NSString *const NSAccessibilityHorizontalOrientationValue = @"NSAccessibilityHorizontalOrientationValue";
NSString *const NSAccessibilityIdentifierAttribute = @"NSAccessibilityIdentifierAttribute";
NSString *const NSAccessibilityImageRole = @"NSAccessibilityImageRole";
NSString *const NSAccessibilityIncrementAction = @"NSAccessibilityIncrementAction";
NSString *const NSAccessibilityIncrementorRole = @"NSAccessibilityIncrementorRole";
NSString *const NSAccessibilityLayoutAreaRole = @"NSAccessibilityLayoutAreaRole";
NSString *const NSAccessibilityLayoutChangedNotification = @"NSAccessibilityLayoutChangedNotification";
NSString *const NSAccessibilityLinkRole = @"NSAccessibilityLinkRole";
NSString *const NSAccessibilityLinkedUIElementsAttribute = @"NSAccessibilityLinkedUIElementsAttribute";
NSString *const NSAccessibilityMainThreadIdleNotification = @"NSAccessibilityMainThreadIdleNotification";
NSString *const NSAccessibilityMaxValueAttribute = @"NSAccessibilityMaxValueAttribute";
NSString *const NSAccessibilityMenuButtonRole = @"NSAccessibilityMenuButtonRole";
NSString *const NSAccessibilityMenuItemRole = @"NSAccessibilityMenuItemRole";
NSString *const NSAccessibilityMenuRole = @"NSAccessibilityMenuRole";
NSString *const NSAccessibilityMinValueAttribute = @"NSAccessibilityMinValueAttribute";
NSString *const NSAccessibilityOrientationAttribute = @"NSAccessibilityOrientationAttribute";
NSString *const NSAccessibilityOutlineRole = @"NSAccessibilityOutlineRole";
NSString *const NSAccessibilityOutlineRowSubrole = @"NSAccessibilityOutlineRowSubrole";
NSString *const NSAccessibilityParentAttribute = @"NSAccessibilityParentAttribute";
NSString *const NSAccessibilityPopUpButtonRole = @"NSAccessibilityPopUpButtonRole";
NSString *const NSAccessibilityPositionAttribute = @"NSAccessibilityPositionAttribute";
NSString *const NSAccessibilityPressAction = @"NSAccessibilityPressAction";
NSString *const NSAccessibilityPriorityKey = @"NSAccessibilityPriorityKey";
NSString *const NSAccessibilityProgressIndicatorRole = @"NSAccessibilityProgressIndicatorRole";
NSString *const NSAccessibilityRadioButtonRole = @"NSAccessibilityRadioButtonRole";
NSString *const NSAccessibilityRadioGroupRole = @"NSAccessibilityRadioGroupRole";
NSString *const NSAccessibilityRoleAttribute = @"NSAccessibilityRoleAttribute";
NSString *const NSAccessibilityRoleDescriptionAttribute = @"NSAccessibilityRoleDescriptionAttribute";
NSString *const NSAccessibilityRowCountChangedNotification = @"NSAccessibilityRowCountChangedNotification";
NSString *const NSAccessibilityRowRole = @"NSAccessibilityRowRole";
NSString *const NSAccessibilitySearchFieldSubrole = @"NSAccessibilitySearchFieldSubrole";
NSString *const NSAccessibilitySelectedAttribute = @"NSAccessibilitySelectedAttribute";
NSString *const NSAccessibilitySelectedRowsChangedNotification = @"NSAccessibilitySelectedRowsChangedNotification";
NSString *const NSAccessibilitySelectedTextChangedNotification = @"NSAccessibilitySelectedTextChangedNotification";
NSString *const NSAccessibilityShowMenuAction = @"NSAccessibilityShowMenuAction";
NSString *const NSAccessibilitySizeAttribute = @"NSAccessibilitySizeAttribute";
NSString *const NSAccessibilitySplitGroupRole = @"NSAccessibilitySplitGroupRole";
NSString *const NSAccessibilitySplitterRole = @"NSAccessibilitySplitterRole";
NSString *const NSAccessibilityStaticTextRole = @"NSAccessibilityStaticTextRole";
NSString *const NSAccessibilitySupportsMainThreadIdleNotificationAttribute = @"NSAccessibilitySupportsMainThreadIdleNotificationAttribute";
NSString *const NSAccessibilityTextAreaRole = @"NSAccessibilityTextAreaRole";
NSString *const NSAccessibilityTextFieldRole = @"NSAccessibilityTextFieldRole";
NSString *const NSAccessibilityTitleAttribute = @"NSAccessibilityTitleAttribute";
NSString *const NSAccessibilityTitleChangedNotification = @"NSAccessibilityTitleChangedNotification";
NSString *const NSAccessibilityTitleUIElementAttribute = @"NSAccessibilityTitleUIElementAttribute";
NSString *const NSAccessibilityToggleSubrole = @"NSAccessibilityToggleSubrole";
NSString *const NSAccessibilityTopLevelUIElementAttribute = @"NSAccessibilityTopLevelUIElementAttribute";
NSString *const NSAccessibilityUIElementDestroyedNotification = @"NSAccessibilityUIElementDestroyedNotification";
NSString *const NSAccessibilityUIElementsKey = @"NSAccessibilityUIElementsKey";
NSString *const NSAccessibilityUnknownRole = @"NSAccessibilityUnknownRole";
NSString *const NSAccessibilityUnknownSubrole = @"NSAccessibilityUnknownSubrole";
NSString *const NSAccessibilityValueAttribute = @"NSAccessibilityValueAttribute";
NSString *const NSAccessibilityValueChangedNotification = @"NSAccessibilityValueChangedNotification";
NSString *const NSAccessibilityVisibleChildrenAttribute = @"NSAccessibilityVisibleChildrenAttribute";
NSString *const NSAccessibilityWindowAttribute = @"NSAccessibilityWindowAttribute";
NSString *const NSAccessibilityWindowRole = @"NSAccessibilityWindowRole";
NSString *const NSAllRomanInputSourcesLocaleIdentifier = @"NSAllRomanInputSourcesLocaleIdentifier";
NSString *const NSAllowsNullArgumentBindingOption = @"NSAllowsNullArgumentBindingOption";
NSString *const NSAnimateBinding = @"NSAnimateBinding";
NSObject *NSApp = nil;
const double NSAppKitVersionNumber = 1504;
NSString *const NSAppearanceNameAccessibilityHighContrastAqua = @"NSAppearanceNameAccessibilityHighContrastAqua";
NSString *const NSAppearanceNameAccessibilityHighContrastDarkAqua = @"NSAppearanceNameAccessibilityHighContrastDarkAqua";
NSString *const NSAppearanceNameAccessibilityHighContrastSystem = @"NSAppearanceNameAccessibilityHighContrastSystem";
NSString *const NSAppearanceNameAccessibilityHighContrastVibrantDark = @"NSAppearanceNameAccessibilityHighContrastVibrantDark";
NSString *const NSAppearanceNameAccessibilityHighContrastVibrantLight = @"NSAppearanceNameAccessibilityHighContrastVibrantLight";
NSString *const NSAppearanceNameAqua = @"NSAppearanceNameAqua";
NSString *const NSAppearanceNameControlStrip = @"NSAppearanceNameControlStrip";
NSString *const NSAppearanceNameDarkAqua = @"NSAppearanceNameDarkAqua";
NSString *const NSAppearanceNameSystem = @"NSAppearanceNameSystem";
NSString *const NSAppearanceNameTouchBar = @"NSAppearanceNameTouchBar";
NSString *const NSAppearanceNameVibrantDark = @"NSAppearanceNameVibrantDark";
NSString *const NSAppearanceNameVibrantLight = @"NSAppearanceNameVibrantLight";
NSString *const NSApplicationDidBecomeActiveNotification = @"NSApplicationDidBecomeActiveNotification";
NSString *const NSApplicationDidResignActiveNotification = @"NSApplicationDidResignActiveNotification";
NSString *const NSApplicationWillHideNotification = @"NSApplicationWillHideNotification";
NSString *const NSApplicationWillResignActiveNotification = @"NSApplicationWillResignActiveNotification";
NSString *const NSApplicationWillTerminateNotification = @"NSApplicationWillTerminateNotification";
NSString *const NSApplicationWillUpdateNotification = @"NSApplicationWillUpdateNotification";
NSString *const NSAttachmentAttributeName = @"NSAttachmentAttributeName";
NSString *const NSAuthorDocumentAttribute = @"NSAuthorDocumentAttribute";
NSString *const NSBackgroundColorAttributeName = @"NSBackgroundColorAttributeName";
NSString *const NSBackgroundColorDocumentAttribute = @"NSBackgroundColorDocumentAttribute";
NSString *const NSBaseURLDocumentOption = @"NSBaseURLDocumentOption";
NSString *const NSBaselineOffsetAttributeName = @"NSBaselineOffsetAttributeName";
NSString *const NSBottomMarginDocumentAttribute = @"NSBottomMarginDocumentAttribute";
NSString *const NSCalibratedRGBColorSpace = @"NSCalibratedRGBColorSpace";
NSString *const NSCalibratedWhiteColorSpace = @"NSCalibratedWhiteColorSpace";
NSString *const NSCharacterEncodingDocumentAttribute = @"NSCharacterEncodingDocumentAttribute";
NSString *const NSCharacterEncodingDocumentOption = @"NSCharacterEncodingDocumentOption";
NSString *const NSCocoaVersionDocumentAttribute = @"NSCocoaVersionDocumentAttribute";
NSString *const NSCollectionElementKindSectionFooter = @"NSCollectionElementKindSectionFooter";
NSString *const NSCollectionElementKindSectionHeader = @"NSCollectionElementKindSectionHeader";
NSString *const NSColorPanelColorDidChangeNotification = @"NSColorPanelColorDidChangeNotification";
NSString *const NSCommentDocumentAttribute = @"NSCommentDocumentAttribute";
NSString *const NSCompanyDocumentAttribute = @"NSCompanyDocumentAttribute";
NSString *const NSConditionallySetsEditableBindingOption = @"NSConditionallySetsEditableBindingOption";
NSString *const NSConditionallySetsEnabledBindingOption = @"NSConditionallySetsEnabledBindingOption";
NSString *const NSContentArrayBinding = @"NSContentArrayBinding";
NSString *const NSContentArrayForMultipleSelectionBinding = @"NSContentArrayForMultipleSelectionBinding";
NSString *const NSContentBinding = @"NSContentBinding";
NSString *const NSContentObjectBinding = @"NSContentObjectBinding";
NSString *const NSContentObjectsBinding = @"NSContentObjectsBinding";
NSString *const NSContentSetBinding = @"NSContentSetBinding";
NSString *const NSContentValuesBinding = @"NSContentValuesBinding";
NSString *const NSContinuouslyUpdatesValueBindingOption = @"NSContinuouslyUpdatesValueBindingOption";
NSString *const NSControlTextDidBeginEditingNotification = @"NSControlTextDidBeginEditingNotification";
NSString *const NSControlTextDidChangeNotification = @"NSControlTextDidChangeNotification";
NSString *const NSControlTextDidEndEditingNotification = @"NSControlTextDidEndEditingNotification";
NSString *const NSConvertedDocumentAttribute = @"NSConvertedDocumentAttribute";
NSString *const NSCopyrightDocumentAttribute = @"NSCopyrightDocumentAttribute";
NSString *const NSCursorAttributeName = @"NSCursorAttributeName";
NSString *const NSCustomColorSpace = @"NSCustomColorSpace";
NSString *const NSDataBinding = @"NSDataBinding";
NSString *const NSDeviceCMYKColorSpace = @"NSDeviceCMYKColorSpace";
NSString *const NSDeviceRGBColorSpace = @"NSDeviceRGBColorSpace";
NSString *const NSDeviceWhiteColorSpace = @"NSDeviceWhiteColorSpace";
NSString *const NSDisplayNameBindingOption = @"NSDisplayNameBindingOption";
NSString *const NSDisplayPatternBindingOption = @"NSDisplayPatternBindingOption";
NSString *const NSDocFormatTextDocumentType = @"NSDocFormatTextDocumentType";
NSString *const NSDocumentTypeDocumentAttribute = @"NSDocumentTypeDocumentAttribute";
NSString *const NSDocumentTypeDocumentOption = @"NSDocumentTypeDocumentOption";
NSString *const NSDoubleClickTargetBinding = @"NSDoubleClickTargetBinding";
NSString *const NSEditableBinding = @"NSEditableBinding";
NSString *const NSEnabledBinding = @"NSEnabledBinding";
NSString *const NSEventTrackingRunLoopMode = @"NSEventTrackingRunLoopMode";
NSString *const NSExcludedElementsDocumentAttribute = @"NSExcludedElementsDocumentAttribute";
NSString *const NSExpansionAttributeName = @"NSExpansionAttributeName";
NSString *const NSFilenamesPboardType = @"NSFilenamesPboardType";
NSString *const NSFontAttributeName = @"NSFontAttributeName";
NSString *const NSFontFaceAttribute = @"NSFontFaceAttribute";
NSString *const NSFontFamilyAttribute = @"NSFontFamilyAttribute";
NSString *const NSFontNameAttribute = @"NSFontNameAttribute";
NSString *const NSFontSizeAttribute = @"NSFontSizeAttribute";
NSString *const NSFontSlantTrait = @"NSFontSlantTrait";
NSString *const NSFontTraitsAttribute = @"NSFontTraitsAttribute";
const CGFloat NSFontWeightBlack      = 0x3fe3d70a40000000;
const CGFloat NSFontWeightBold       = 0x3fd99999a0000000;
const CGFloat NSFontWeightHeavy      = 0x3fe1eb8520000000;
const CGFloat NSFontWeightLight      = 0xbfd99999a0000000;
const CGFloat NSFontWeightMedium     = 0x3fcd70a3e0000000;
const CGFloat NSFontWeightRegular    = 0x0000000000000000;
const CGFloat NSFontWeightSemibold   = 0x3fd3333340000000;
const CGFloat NSFontWeightThin       = 0xbfe3333340000000;
NSString *const NSFontWeightTrait = @"NSFontWeightTrait";
const CGFloat NSFontWeightUltraLight = 0xbfe99999a0000000;
NSString *const NSForegroundColorAttributeName = @"NSForegroundColorAttributeName";
NSString *const NSGlyphInfoAttributeName = @"NSGlyphInfoAttributeName";
NSString *const NSGridViewSizeForContent = @"NSGridViewSizeForContent";
NSString *const NSHTMLTextDocumentType = @"NSHTMLTextDocumentType";
NSString *const NSHeaderTitleBinding = @"NSHeaderTitleBinding";
NSString *const NSHiddenBinding = @"NSHiddenBinding";
NSString *const NSHyphenationFactorDocumentAttribute = @"NSHyphenationFactorDocumentAttribute";
NSString *const NSImageBinding = @"NSImageBinding";
NSString *const NSImageNameActionTemplate = @"NSImageNameActionTemplate";
NSString *const NSImageNameAddTemplate = @"NSImageNameAddTemplate";
NSString *const NSImageNameAdvanced = @"NSImageNameAdvanced";
NSString *const NSImageNameApplicationIcon = @"NSImageNameApplicationIcon";
NSString *const NSImageNameAudioOutputVolumeHighTemplate = @"NSImageNameAudioOutputVolumeHighTemplate";
NSString *const NSImageNameBluetoothTemplate = @"NSImageNameBluetoothTemplate";
NSString *const NSImageNameBonjour = @"NSImageNameBonjour";
NSString *const NSImageNameBookmarksTemplate = @"NSImageNameBookmarksTemplate";
NSString *const NSImageNameCaution = @"NSImageNameCaution";
NSString *const NSImageNameColorPanel = @"NSImageNameColorPanel";
NSString *const NSImageNameColumnViewTemplate = @"NSImageNameColumnViewTemplate";
NSString *const NSImageNameComputer = @"NSImageNameComputer";
NSString *const NSImageNameEnterFullScreenTemplate = @"NSImageNameEnterFullScreenTemplate";
NSString *const NSImageNameEveryone = @"NSImageNameEveryone";
NSString *const NSImageNameExitFullScreenTemplate = @"NSImageNameExitFullScreenTemplate";
NSString *const NSImageNameFlowViewTemplate = @"NSImageNameFlowViewTemplate";
NSString *const NSImageNameFolder = @"NSImageNameFolder";
NSString *const NSImageNameFolderBurnable = @"NSImageNameFolderBurnable";
NSString *const NSImageNameFolderSmart = @"NSImageNameFolderSmart";
NSString *const NSImageNameFollowLinkFreestandingTemplate = @"NSImageNameFollowLinkFreestandingTemplate";
NSString *const NSImageNameFontPanel = @"NSImageNameFontPanel";
NSString *const NSImageNameGoBackTemplate = @"NSImageNameGoBackTemplate";
NSString *const NSImageNameGoForwardTemplate = @"NSImageNameGoForwardTemplate";
NSString *const NSImageNameGoLeftTemplate = @"NSImageNameGoLeftTemplate";
NSString *const NSImageNameGoRightTemplate = @"NSImageNameGoRightTemplate";
NSString *const NSImageNameHomeTemplate = @"NSImageNameHomeTemplate";
NSString *const NSImageNameIChatTheaterTemplate = @"NSImageNameIChatTheaterTemplate";
NSString *const NSImageNameIconViewTemplate = @"NSImageNameIconViewTemplate";
NSString *const NSImageNameInfo = @"NSImageNameInfo";
NSString *const NSImageNameInvalidDataFreestandingTemplate = @"NSImageNameInvalidDataFreestandingTemplate";
NSString *const NSImageNameLeftFacingTriangleTemplate = @"NSImageNameLeftFacingTriangleTemplate";
NSString *const NSImageNameListViewTemplate = @"NSImageNameListViewTemplate";
NSString *const NSImageNameLockLockedTemplate = @"NSImageNameLockLockedTemplate";
NSString *const NSImageNameLockUnlockedTemplate = @"NSImageNameLockUnlockedTemplate";
NSString *const NSImageNameMenuMixedStateTemplate = @"NSImageNameMenuMixedStateTemplate";
NSString *const NSImageNameMenuOnStateTemplate = @"NSImageNameMenuOnStateTemplate";
NSString *const NSImageNameMobileMe = @"NSImageNameMobileMe";
NSString *const NSImageNameMultipleDocuments = @"NSImageNameMultipleDocuments";
NSString *const NSImageNameNetwork = @"NSImageNameNetwork";
NSString *const NSImageNamePathTemplate = @"NSImageNamePathTemplate";
NSString *const NSImageNamePreferencesGeneral = @"NSImageNamePreferencesGeneral";
NSString *const NSImageNameQuickLookTemplate = @"NSImageNameQuickLookTemplate";
NSString *const NSImageNameRefreshFreestandingTemplate = @"NSImageNameRefreshFreestandingTemplate";
NSString *const NSImageNameRefreshTemplate = @"NSImageNameRefreshTemplate";
NSString *const NSImageNameRemoveTemplate = @"NSImageNameRemoveTemplate";
NSString *const NSImageNameRevealFreestandingTemplate = @"NSImageNameRevealFreestandingTemplate";
NSString *const NSImageNameRightFacingTriangleTemplate = @"NSImageNameRightFacingTriangleTemplate";
NSString *const NSImageNameShareTemplate = @"NSImageNameShareTemplate";
NSString *const NSImageNameSlideshowTemplate = @"NSImageNameSlideshowTemplate";
NSString *const NSImageNameSmartBadgeTemplate = @"NSImageNameSmartBadgeTemplate";
NSString *const NSImageNameStatusAvailable = @"NSImageNameStatusAvailable";
NSString *const NSImageNameStatusNone = @"NSImageNameStatusNone";
NSString *const NSImageNameStatusPartiallyAvailable = @"NSImageNameStatusPartiallyAvailable";
NSString *const NSImageNameStatusUnavailable = @"NSImageNameStatusUnavailable";
NSString *const NSImageNameStopProgressFreestandingTemplate = @"NSImageNameStopProgressFreestandingTemplate";
NSString *const NSImageNameStopProgressTemplate = @"NSImageNameStopProgressTemplate";
NSString *const NSImageNameTouchBarAddDetailTemplate = @"NSImageNameTouchBarAddDetailTemplate";
NSString *const NSImageNameTouchBarAddTemplate = @"NSImageNameTouchBarAddTemplate";
NSString *const NSImageNameTouchBarAlarmTemplate = @"NSImageNameTouchBarAlarmTemplate";
NSString *const NSImageNameTouchBarAudioInputMuteTemplate = @"NSImageNameTouchBarAudioInputMuteTemplate";
NSString *const NSImageNameTouchBarAudioInputTemplate = @"NSImageNameTouchBarAudioInputTemplate";
NSString *const NSImageNameTouchBarAudioOutputMuteTemplate = @"NSImageNameTouchBarAudioOutputMuteTemplate";
NSString *const NSImageNameTouchBarAudioOutputVolumeHighTemplate = @"NSImageNameTouchBarAudioOutputVolumeHighTemplate";
NSString *const NSImageNameTouchBarAudioOutputVolumeLowTemplate = @"NSImageNameTouchBarAudioOutputVolumeLowTemplate";
NSString *const NSImageNameTouchBarAudioOutputVolumeMediumTemplate = @"NSImageNameTouchBarAudioOutputVolumeMediumTemplate";
NSString *const NSImageNameTouchBarAudioOutputVolumeOffTemplate = @"NSImageNameTouchBarAudioOutputVolumeOffTemplate";
NSString *const NSImageNameTouchBarBookmarksTemplate = @"NSImageNameTouchBarBookmarksTemplate";
NSString *const NSImageNameTouchBarColorPickerFill = @"NSImageNameTouchBarColorPickerFill";
NSString *const NSImageNameTouchBarColorPickerFont = @"NSImageNameTouchBarColorPickerFont";
NSString *const NSImageNameTouchBarColorPickerStroke = @"NSImageNameTouchBarColorPickerStroke";
NSString *const NSImageNameTouchBarCommunicationAudioTemplate = @"NSImageNameTouchBarCommunicationAudioTemplate";
NSString *const NSImageNameTouchBarCommunicationVideoTemplate = @"NSImageNameTouchBarCommunicationVideoTemplate";
NSString *const NSImageNameTouchBarComposeTemplate = @"NSImageNameTouchBarComposeTemplate";
NSString *const NSImageNameTouchBarDeleteTemplate = @"NSImageNameTouchBarDeleteTemplate";
NSString *const NSImageNameTouchBarDownloadTemplate = @"NSImageNameTouchBarDownloadTemplate";
NSString *const NSImageNameTouchBarEnterFullScreenTemplate = @"NSImageNameTouchBarEnterFullScreenTemplate";
NSString *const NSImageNameTouchBarExitFullScreenTemplate = @"NSImageNameTouchBarExitFullScreenTemplate";
NSString *const NSImageNameTouchBarFastForwardTemplate = @"NSImageNameTouchBarFastForwardTemplate";
NSString *const NSImageNameTouchBarFolderCopyToTemplate = @"NSImageNameTouchBarFolderCopyToTemplate";
NSString *const NSImageNameTouchBarFolderMoveToTemplate = @"NSImageNameTouchBarFolderMoveToTemplate";
NSString *const NSImageNameTouchBarFolderTemplate = @"NSImageNameTouchBarFolderTemplate";
NSString *const NSImageNameTouchBarGetInfoTemplate = @"NSImageNameTouchBarGetInfoTemplate";
NSString *const NSImageNameTouchBarGoBackTemplate = @"NSImageNameTouchBarGoBackTemplate";
NSString *const NSImageNameTouchBarGoDownTemplate = @"NSImageNameTouchBarGoDownTemplate";
NSString *const NSImageNameTouchBarGoForwardTemplate = @"NSImageNameTouchBarGoForwardTemplate";
NSString *const NSImageNameTouchBarGoUpTemplate = @"NSImageNameTouchBarGoUpTemplate";
NSString *const NSImageNameTouchBarHistoryTemplate = @"NSImageNameTouchBarHistoryTemplate";
NSString *const NSImageNameTouchBarIconViewTemplate = @"NSImageNameTouchBarIconViewTemplate";
NSString *const NSImageNameTouchBarListViewTemplate = @"NSImageNameTouchBarListViewTemplate";
NSString *const NSImageNameTouchBarMailTemplate = @"NSImageNameTouchBarMailTemplate";
NSString *const NSImageNameTouchBarNewFolderTemplate = @"NSImageNameTouchBarNewFolderTemplate";
NSString *const NSImageNameTouchBarNewMessageTemplate = @"NSImageNameTouchBarNewMessageTemplate";
NSString *const NSImageNameTouchBarOpenInBrowserTemplate = @"NSImageNameTouchBarOpenInBrowserTemplate";
NSString *const NSImageNameTouchBarPauseTemplate = @"NSImageNameTouchBarPauseTemplate";
NSString *const NSImageNameTouchBarPlayPauseTemplate = @"NSImageNameTouchBarPlayPauseTemplate";
NSString *const NSImageNameTouchBarPlayTemplate = @"NSImageNameTouchBarPlayTemplate";
NSString *const NSImageNameTouchBarPlayheadTemplate = @"NSImageNameTouchBarPlayheadTemplate";
NSString *const NSImageNameTouchBarQuickLookTemplate = @"NSImageNameTouchBarQuickLookTemplate";
NSString *const NSImageNameTouchBarRecordStartTemplate = @"NSImageNameTouchBarRecordStartTemplate";
NSString *const NSImageNameTouchBarRecordStopTemplate = @"NSImageNameTouchBarRecordStopTemplate";
NSString *const NSImageNameTouchBarRefreshTemplate = @"NSImageNameTouchBarRefreshTemplate";
NSString *const NSImageNameTouchBarRemoveTemplate = @"NSImageNameTouchBarRemoveTemplate";
NSString *const NSImageNameTouchBarRewindTemplate = @"NSImageNameTouchBarRewindTemplate";
NSString *const NSImageNameTouchBarRotateLeftTemplate = @"NSImageNameTouchBarRotateLeftTemplate";
NSString *const NSImageNameTouchBarRotateRightTemplate = @"NSImageNameTouchBarRotateRightTemplate";
NSString *const NSImageNameTouchBarSearchTemplate = @"NSImageNameTouchBarSearchTemplate";
NSString *const NSImageNameTouchBarShareTemplate = @"NSImageNameTouchBarShareTemplate";
NSString *const NSImageNameTouchBarSidebarTemplate = @"NSImageNameTouchBarSidebarTemplate";
NSString *const NSImageNameTouchBarSkipAhead15SecondsTemplate = @"NSImageNameTouchBarSkipAhead15SecondsTemplate";
NSString *const NSImageNameTouchBarSkipAhead30SecondsTemplate = @"NSImageNameTouchBarSkipAhead30SecondsTemplate";
NSString *const NSImageNameTouchBarSkipAheadTemplate = @"NSImageNameTouchBarSkipAheadTemplate";
NSString *const NSImageNameTouchBarSkipBack15SecondsTemplate = @"NSImageNameTouchBarSkipBack15SecondsTemplate";
NSString *const NSImageNameTouchBarSkipBack30SecondsTemplate = @"NSImageNameTouchBarSkipBack30SecondsTemplate";
NSString *const NSImageNameTouchBarSkipBackTemplate = @"NSImageNameTouchBarSkipBackTemplate";
NSString *const NSImageNameTouchBarSkipToEndTemplate = @"NSImageNameTouchBarSkipToEndTemplate";
NSString *const NSImageNameTouchBarSkipToStartTemplate = @"NSImageNameTouchBarSkipToStartTemplate";
NSString *const NSImageNameTouchBarSlideshowTemplate = @"NSImageNameTouchBarSlideshowTemplate";
NSString *const NSImageNameTouchBarTagIconTemplate = @"NSImageNameTouchBarTagIconTemplate";
NSString *const NSImageNameTouchBarTextBoldTemplate = @"NSImageNameTouchBarTextBoldTemplate";
NSString *const NSImageNameTouchBarTextBoxTemplate = @"NSImageNameTouchBarTextBoxTemplate";
NSString *const NSImageNameTouchBarTextCenterAlignTemplate = @"NSImageNameTouchBarTextCenterAlignTemplate";
NSString *const NSImageNameTouchBarTextItalicTemplate = @"NSImageNameTouchBarTextItalicTemplate";
NSString *const NSImageNameTouchBarTextJustifiedAlignTemplate = @"NSImageNameTouchBarTextJustifiedAlignTemplate";
NSString *const NSImageNameTouchBarTextLeftAlignTemplate = @"NSImageNameTouchBarTextLeftAlignTemplate";
NSString *const NSImageNameTouchBarTextListTemplate = @"NSImageNameTouchBarTextListTemplate";
NSString *const NSImageNameTouchBarTextRightAlignTemplate = @"NSImageNameTouchBarTextRightAlignTemplate";
NSString *const NSImageNameTouchBarTextStrikethroughTemplate = @"NSImageNameTouchBarTextStrikethroughTemplate";
NSString *const NSImageNameTouchBarTextUnderlineTemplate = @"NSImageNameTouchBarTextUnderlineTemplate";
NSString *const NSImageNameTouchBarUserAddTemplate = @"NSImageNameTouchBarUserAddTemplate";
NSString *const NSImageNameTouchBarUserGroupTemplate = @"NSImageNameTouchBarUserGroupTemplate";
NSString *const NSImageNameTouchBarUserTemplate = @"NSImageNameTouchBarUserTemplate";
NSString *const NSImageNameTouchBarVolumeDownTemplate = @"NSImageNameTouchBarVolumeDownTemplate";
NSString *const NSImageNameTouchBarVolumeUpTemplate = @"NSImageNameTouchBarVolumeUpTemplate";
NSString *const NSImageNameTrashEmpty = @"NSImageNameTrashEmpty";
NSString *const NSImageNameTrashFull = @"NSImageNameTrashFull";
NSString *const NSImageNameUser = @"NSImageNameUser";
NSString *const NSImageNameUserAccounts = @"NSImageNameUserAccounts";
NSString *const NSImageNameUserGroup = @"NSImageNameUserGroup";
NSString *const NSImageNameUserGuest = @"NSImageNameUserGuest";
NSString *const NSInsertsNullPlaceholderBindingOption = @"NSInsertsNullPlaceholderBindingOption";
NSString *const NSKernAttributeName = @"NSKernAttributeName";
NSString *const NSKeywordsDocumentAttribute = @"NSKeywordsDocumentAttribute";
NSString *const NSLeftMarginDocumentAttribute = @"NSLeftMarginDocumentAttribute";
NSString *const NSLigatureAttributeName = @"NSLigatureAttributeName";
NSString *const NSLinkAttributeName = @"NSLinkAttributeName";
NSString *const NSMacSimpleTextDocumentType = @"NSMacSimpleTextDocumentType";
NSString *const NSMarkedClauseSegmentAttributeName = @"NSMarkedClauseSegmentAttributeName";
NSString *const NSMaxValueBinding = @"NSMaxValueBinding";
NSString *const NSMinValueBinding = @"NSMinValueBinding";
NSString *const NSModalPanelRunLoopMode = @"NSModalPanelRunLoopMode";
NSString *const NSMultipleValuesMarker = @"NSMultipleValuesMarker";
NSString *const NSMultipleValuesPlaceholderBindingOption = @"NSMultipleValuesPlaceholderBindingOption";
NSString *const NSNamedColorSpace = @"NSNamedColorSpace";
NSString *const NSNoSelectionMarker = @"NSNoSelectionMarker";
NSString *const NSNoSelectionPlaceholderBindingOption = @"NSNoSelectionPlaceholderBindingOption";
NSString *const NSNotApplicableMarker = @"NSNotApplicableMarker";
NSString *const NSNotApplicablePlaceholderBindingOption = @"NSNotApplicablePlaceholderBindingOption";
NSString *const NSNullPlaceholderBindingOption = @"NSNullPlaceholderBindingOption";
NSString *const NSObliquenessAttributeName = @"NSObliquenessAttributeName";
NSString *const NSObservedKeyPathKey = @"NSObservedKeyPathKey";
NSString *const NSObservedObjectKey = @"NSObservedObjectKey";
NSString *const NSOfficeOpenXMLTextDocumentType = @"NSOfficeOpenXMLTextDocumentType";
NSString *const NSOpenDocumentTextDocumentType = @"NSOpenDocumentTextDocumentType";
NSString *const NSOptionsKey = @"NSOptionsKey";
NSString *const NSOutlineViewColumnDidResizeNotification = @"NSOutlineViewColumnDidResizeNotification";
NSString *const NSOutlineViewDisclosureButtonKey = @"NSOutlineViewDisclosureButtonKey";
NSString *const NSOutlineViewItemDidCollapseNotification = @"NSOutlineViewItemDidCollapseNotification";
NSString *const NSOutlineViewItemDidExpandNotification = @"NSOutlineViewItemDidExpandNotification";
NSString *const NSOutlineViewSelectionDidChangeNotification = @"NSOutlineViewSelectionDidChangeNotification";
NSString *const NSPaperSizeDocumentAttribute = @"NSPaperSizeDocumentAttribute";
NSString *const NSParagraphStyleAttributeName = @"NSParagraphStyleAttributeName";
NSString *const NSPasteboardNameDrag = @"NSPasteboardNameDrag";
NSString *const NSPasteboardNameFind = @"NSPasteboardNameFind";
NSString *const NSPasteboardNameGeneral = @"NSPasteboardNameGeneral";
NSString *const NSPasteboardTypeColor = @"NSPasteboardTypeColor";
NSString *const NSPasteboardTypeFileURL = @"NSPasteboardTypeFileURL";
NSString *const NSPasteboardTypeMultipleTextSelection = @"NSPasteboardTypeMultipleTextSelection";
NSString *const NSPasteboardTypePDF = @"NSPasteboardTypePDF";
NSString *const NSPasteboardTypePNG = @"NSPasteboardTypePNG";
NSString *const NSPasteboardTypeRTF = @"NSPasteboardTypeRTF";
NSString *const NSPasteboardTypeString = @"NSPasteboardTypeString";
NSString *const NSPasteboardTypeTIFF = @"NSPasteboardTypeTIFF";
NSString *const NSPasteboardTypeTabularText = @"NSPasteboardTypeTabularText";
NSString *const NSPasteboardTypeURL = @"NSPasteboardTypeURL";
NSString *const NSPasteboardURLReadingContentsConformToTypesKey = @"NSPasteboardURLReadingContentsConformToTypesKey";
NSString *const NSPasteboardURLReadingFileURLsOnlyKey = @"NSPasteboardURLReadingFileURLsOnlyKey";
NSString *const NSPatternColorSpace = @"NSPatternColorSpace";
NSString *const NSPlainTextDocumentType = @"NSPlainTextDocumentType";
NSString *const NSPopUpButtonWillPopUpNotification = @"NSPopUpButtonWillPopUpNotification";
NSString *const NSPopoverDidCloseNotification = @"NSPopoverDidCloseNotification";
NSString *const NSPopoverDidShowNotification = @"NSPopoverDidShowNotification";
NSString *const NSPopoverWillCloseNotification = @"NSPopoverWillCloseNotification";
NSString *const NSPreferredScrollerStyleDidChangeNotification = @"NSPreferredScrollerStyleDidChangeNotification";
NSString *const NSPrefixSpacesDocumentAttribute = @"NSPrefixSpacesDocumentAttribute";
NSString *const NSPrintHeaderAndFooter = @"NSPrintHeaderAndFooter";
NSString *const NSPrintPanelAccessorySummaryItemDescriptionKey = @"NSPrintPanelAccessorySummaryItemDescriptionKey";
NSString *const NSPrintPanelAccessorySummaryItemNameKey = @"NSPrintPanelAccessorySummaryItemNameKey";
NSString *const NSPrintScalingFactor = @"NSPrintScalingFactor";
NSString *const NSRTFDTextDocumentType = @"NSRTFDTextDocumentType";
NSString *const NSRTFPboardType = @"NSRTFPboardType";
NSString *const NSRTFTextDocumentType = @"NSRTFTextDocumentType";
NSString *const NSReadOnlyDocumentAttribute = @"NSReadOnlyDocumentAttribute";
NSString *const NSRightMarginDocumentAttribute = @"NSRightMarginDocumentAttribute";
NSString *const NSRuleEditorRowsDidChangeNotification = @"NSRuleEditorRowsDidChangeNotification";
NSString *const NSScrollViewDidEndLiveMagnifyNotification = @"NSScrollViewDidEndLiveMagnifyNotification";
NSString *const NSScrollViewDidEndLiveScrollNotification = @"NSScrollViewDidEndLiveScrollNotification";
NSString *const NSScrollViewDidLiveScrollNotification = @"NSScrollViewDidLiveScrollNotification";
NSString *const NSScrollViewWillStartLiveMagnifyNotification = @"NSScrollViewWillStartLiveMagnifyNotification";
NSString *const NSScrollViewWillStartLiveScrollNotification = @"NSScrollViewWillStartLiveScrollNotification";
NSString *const NSSelectedIndexBinding = @"NSSelectedIndexBinding";
NSString *const NSSelectedObjectBinding = @"NSSelectedObjectBinding";
NSString *const NSSelectedObjectsBinding = @"NSSelectedObjectsBinding";
NSString *const NSSelectedTagBinding = @"NSSelectedTagBinding";
NSString *const NSSelectionIndexPathsBinding = @"NSSelectionIndexPathsBinding";
NSString *const NSSelectionIndexesBinding = @"NSSelectionIndexesBinding";
NSString *const NSShadowAttributeName = @"NSShadowAttributeName";
NSString *const NSSharingServiceNameComposeEmail = @"NSSharingServiceNameComposeEmail";
NSString *const NSSliderAccessoryWidthWide = @"NSSliderAccessoryWidthWide";
NSString *const NSSortDescriptorsBinding = @"NSSortDescriptorsBinding";
NSString *const NSSplitViewDidResizeSubviewsNotification = @"NSSplitViewDidResizeSubviewsNotification";
NSString *const NSStrikethroughColorAttributeName = @"NSStrikethroughColorAttributeName";
NSString *const NSStrikethroughStyleAttributeName = @"NSStrikethroughStyleAttributeName";
NSString *const NSStringPboardType = @"NSStringPboardType";
NSString *const NSStrokeColorAttributeName = @"NSStrokeColorAttributeName";
NSString *const NSStrokeWidthAttributeName = @"NSStrokeWidthAttributeName";
NSString *const NSSubjectDocumentAttribute = @"NSSubjectDocumentAttribute";
NSString *const NSSystemColorsDidChangeNotification = @"NSSystemColorsDidChangeNotification";
NSString *const NSTableViewRowViewKey = @"NSTableViewRowViewKey";
NSString *const NSTableViewSelectionDidChangeNotification = @"NSTableViewSelectionDidChangeNotification";
NSString *const NSTextColorBinding = @"NSTextColorBinding";
NSString *const NSTextDidBeginEditingNotification = @"NSTextDidBeginEditingNotification";
NSString *const NSTextDidChangeNotification = @"NSTextDidChangeNotification";
NSString *const NSTextDidEndEditingNotification = @"NSTextDidEndEditingNotification";
NSString *const NSTextLayoutSectionsAttribute = @"NSTextLayoutSectionsAttribute";
NSString *const NSTextListMarkerDisc = @"NSTextListMarkerDisc";
NSString *const NSTextStorageDidProcessEditingNotification = @"NSTextStorageDidProcessEditingNotification";
NSString *const NSTextViewDidChangeSelectionNotification = @"NSTextViewDidChangeSelectionNotification";
NSString *const NSTitleBinding = @"NSTitleBinding";
NSString *const NSTitleDocumentAttribute = @"NSTitleDocumentAttribute";
NSString *const NSToolTipBinding = @"NSToolTipBinding";
NSString *const NSToolTipHelpKey = @"NSToolTipHelpKey";
NSString *const NSToolbarCustomizeToolbarItemIdentifier = @"NSToolbarCustomizeToolbarItemIdentifier";
NSString *const NSToolbarFlexibleSpaceItemIdentifier = @"NSToolbarFlexibleSpaceItemIdentifier";
NSString *const NSToolbarPrintItemIdentifier = @"NSToolbarPrintItemIdentifier";
NSString *const NSToolbarSeparatorItemIdentifier = @"NSToolbarSeparatorItemIdentifier";
NSString *const NSToolbarShowColorsItemIdentifier = @"NSToolbarShowColorsItemIdentifier";
NSString *const NSToolbarShowFontsItemIdentifier = @"NSToolbarShowFontsItemIdentifier";
NSString *const NSToolbarSpaceItemIdentifier = @"NSToolbarSpaceItemIdentifier";
NSString *const NSTopMarginDocumentAttribute = @"NSTopMarginDocumentAttribute";
NSString *const NSTouchBarItemIdentifierCharacterPicker = @"NSTouchBarItemIdentifierCharacterPicker";
NSString *const NSTouchBarItemIdentifierFixedSpaceLarge = @"NSTouchBarItemIdentifierFixedSpaceLarge";
NSString *const NSTouchBarItemIdentifierFixedSpaceSmall = @"NSTouchBarItemIdentifierFixedSpaceSmall";
NSString *const NSTouchBarItemIdentifierFlexibleSpace = @"NSTouchBarItemIdentifierFlexibleSpace";
NSString *const NSTouchBarItemIdentifierOtherItemsProxy = @"NSTouchBarItemIdentifierOtherItemsProxy";
NSString *const NSURLPboardType = @"NSURLPboardType";
NSUInteger NSUnderlineByWordMask = 0x8000;
NSString *const NSUnderlineColorAttributeName = @"NSUnderlineColorAttributeName";
NSString *const NSUnderlineStyleAttributeName = @"NSUnderlineStyleAttributeName";
NSString *const NSValidatesImmediatelyBindingOption = @"NSValidatesImmediatelyBindingOption";
NSString *const NSValueBinding = @"NSValueBinding";
NSString *const NSValueTransformerBindingOption = @"NSValueTransformerBindingOption";
NSString *const NSValueTransformerNameBindingOption = @"NSValueTransformerNameBindingOption";
NSString *const NSViewAnimationEffectKey = @"NSViewAnimationEffectKey";
NSString *const NSViewAnimationEndFrameKey = @"NSViewAnimationEndFrameKey";
NSString *const NSViewAnimationFadeInEffect = @"NSViewAnimationFadeInEffect";
NSString *const NSViewAnimationFadeOutEffect = @"NSViewAnimationFadeOutEffect";
NSString *const NSViewAnimationTargetKey = @"NSViewAnimationTargetKey";
NSString *const NSViewBoundsDidChangeNotification = @"NSViewBoundsDidChangeNotification";
NSString *const NSViewFrameDidChangeNotification = @"NSViewFrameDidChangeNotification";
NSString *const NSViewModeDocumentAttribute = @"NSViewModeDocumentAttribute";
const CGFloat NSViewNoIntrinsicMetric = 0xbff0000000000000;
NSString *const NSViewSizeDocumentAttribute = @"NSViewSizeDocumentAttribute";
NSString *const NSViewZoomDocumentAttribute = @"NSViewZoomDocumentAttribute";
NSString *const NSVisibleBinding = @"NSVisibleBinding";
NSString *const NSVoiceName = @"NSVoiceName";
NSString *const NSWebArchiveTextDocumentType = @"NSWebArchiveTextDocumentType";
NSString *const NSWindowDidBecomeKeyNotification = @"NSWindowDidBecomeKeyNotification";
NSString *const NSWindowDidBecomeMainNotification = @"NSWindowDidBecomeMainNotification";
NSString *const NSWindowDidChangeBackingPropertiesNotification = @"NSWindowDidChangeBackingPropertiesNotification";
NSString *const NSWindowDidChangeOcclusionStateNotification = @"NSWindowDidChangeOcclusionStateNotification";
NSString *const NSWindowDidChangeScreenNotification = @"NSWindowDidChangeScreenNotification";
NSString *const NSWindowDidDeminiaturizeNotification = @"NSWindowDidDeminiaturizeNotification";
NSString *const NSWindowDidEndSheetNotification = @"NSWindowDidEndSheetNotification";
NSString *const NSWindowDidEnterFullScreenNotification = @"NSWindowDidEnterFullScreenNotification";
NSString *const NSWindowDidExitFullScreenNotification = @"NSWindowDidExitFullScreenNotification";
NSString *const NSWindowDidMiniaturizeNotification = @"NSWindowDidMiniaturizeNotification";
NSString *const NSWindowDidMoveNotification = @"NSWindowDidMoveNotification";
NSString *const NSWindowDidOrderOffScreenNotification = @"NSWindowDidOrderOffScreenNotification";
NSString *const NSWindowDidOrderOnScreenNotification = @"NSWindowDidOrderOnScreenNotification";
NSString *const NSWindowDidResignKeyNotification = @"NSWindowDidResignKeyNotification";
NSString *const NSWindowDidResignMainNotification = @"NSWindowDidResignMainNotification";
NSString *const NSWindowDidResizeNotification = @"NSWindowDidResizeNotification";
NSString *const NSWindowDidUpdateNotification = @"NSWindowDidUpdateNotification";
NSString *const NSWindowWillBeginSheetNotification = @"NSWindowWillBeginSheetNotification";
NSString *const NSWindowWillCloseNotification = @"NSWindowWillCloseNotification";
NSString *const NSWindowWillEnterFullScreenNotification = @"NSWindowWillEnterFullScreenNotification";
NSString *const NSWindowWillExitFullScreenNotification = @"NSWindowWillExitFullScreenNotification";
NSString *const NSWindowWillStartLiveResizeNotification = @"NSWindowWillStartLiveResizeNotification";
NSString *const NSWordMLTextDocumentType = @"NSWordMLTextDocumentType";
NSString *const NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification = @"NSWorkspaceAccessibilityDisplayOptionsDidChangeNotification";
NSString *const NSWorkspaceActiveSpaceDidChangeNotification = @"NSWorkspaceActiveSpaceDidChangeNotification";
NSString *const NSWorkspaceLaunchConfigurationArguments = @"NSWorkspaceLaunchConfigurationArguments";
NSString *const NSWorkspaceLaunchConfigurationEnvironment = @"NSWorkspaceLaunchConfigurationEnvironment";
NSString *const NSWorkspaceSessionDidBecomeActiveNotification = @"NSWorkspaceSessionDidBecomeActiveNotification";
NSString *const NSWorkspaceSessionDidResignActiveNotification = @"NSWorkspaceSessionDidResignActiveNotification";
NSString *const NSWritingDirectionAttributeName = @"NSWritingDirectionAttributeName";
NSString *const _NSColorCoreUICatalogMainBundleID = @"_NSColorCoreUICatalogMainBundleID";
NSString *const _NSColorCoreUICatalogNamePrefix = @"_NSColorCoreUICatalogNamePrefix";
NSString *const _NSFontMenuName = @"_NSFontMenuName";
NSString *const _NSHelpMenuName = @"_NSHelpMenuName";
NSString *const _NSMainMenuName = @"_NSMainMenuName";
NSString *const _NSRecentDocumentsMenuName = @"_NSRecentDocumentsMenuName";
NSString *const _NSServicesMenuName = @"_NSServicesMenuName";
NSString *const _NSTabViewFloatingCell = @"_NSTabViewFloatingCell";
NSString *const _NSToolbarItemDragType = @"_NSToolbarItemDragType";
NSString *const _NSToolbarItemIdentifierPboardType = @"_NSToolbarItemIdentifierPboardType";
NSString *const _NSWindowsMenuName = @"_NSWindowsMenuName";

void* NSAccessibilityActionDescription(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityActionDescription called\n");
	return NULL;
};

void* NSAccessibilityFrameInView(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityFrameInView called\n");
	return NULL;
};

void* NSAccessibilityPointInView(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityPointInView called\n");
	return NULL;
};

void* NSAccessibilityPostNotification(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityPostNotification called\n");
	return NULL;
};

void* NSAccessibilityPostNotificationWithUserInfo(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityPostNotificationWithUserInfo called\n");
	return NULL;
};

void* NSAccessibilityRoleDescription(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityRoleDescription called\n");
	return NULL;
};

void* NSAccessibilityRoleDescriptionForUIElement(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityRoleDescriptionForUIElement called\n");
	return NULL;
};

void* NSAccessibilityUnignoredAncestor(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityUnignoredAncestor called\n");
	return NULL;
};

void* NSAccessibilityUnignoredChildren(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityUnignoredChildren called\n");
	return NULL;
};

void* NSAccessibilityUnignoredDescendant(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityUnignoredDescendant called\n");
	return NULL;
};

void* NSAccessibilityUnregisterUniqueIdForUIElement(void) {
	if (verbose) LOG_FUNC("STUB: NSAccessibilityUnregisterUniqueIdForUIElement called\n");
	return NULL;
};

void* NSApplicationLoad(void) {
	if (verbose) LOG_FUNC("STUB: NSApplicationLoad called\n");
	return NULL;
};

void* NSApplicationMain(void) {
	if (verbose) LOG_FUNC("STUB: NSApplicationMain called\n");
	return NULL;
};

void* NSAvailableWindowDepths(void) {
	if (verbose) LOG_FUNC("STUB: NSAvailableWindowDepths called\n");
	return NULL;
};

void* NSBeep(void) {
	if (verbose) LOG_FUNC("STUB: NSBeep called\n");
	return NULL;
};

void* NSBestDepth(void) {
	if (verbose) LOG_FUNC("STUB: NSBestDepth called\n");
	return NULL;
};

void* NSDisableScreenUpdates(void) {
	if (verbose) LOG_FUNC("STUB: NSDisableScreenUpdates called\n");
	return NULL;
};

void* NSDrawColorTiledRects(void) {
	if (verbose) LOG_FUNC("STUB: NSDrawColorTiledRects called\n");
	return NULL;
};

void* NSDrawNinePartImage(void) {
	if (verbose) LOG_FUNC("STUB: NSDrawNinePartImage called\n");
	return NULL;
};

void* NSDrawScopeBar(void) {
	if (verbose) LOG_FUNC("STUB: NSDrawScopeBar called\n");
	return NULL;
};

void* NSDrawThreePartImage(void) {
	if (verbose) LOG_FUNC("STUB: NSDrawThreePartImage called\n");
	return NULL;
};

void* NSEnableScreenUpdates(void) {
	if (verbose) LOG_FUNC("STUB: NSEnableScreenUpdates called\n");
	return NULL;
};

void* NSEraseRect(void) {
	if (verbose) LOG_FUNC("STUB: NSEraseRect called\n");
	return NULL;
};

void* NSFrameRect(void) {
	if (verbose) LOG_FUNC("STUB: NSFrameRect called\n");
	return NULL;
};

void* NSFrameRectWithWidth(void) {
	if (verbose) LOG_FUNC("STUB: NSFrameRectWithWidth called\n");
	return NULL;
};

void* NSFrameRectWithWidthUsingOperation(void) {
	if (verbose) LOG_FUNC("STUB: NSFrameRectWithWidthUsingOperation called\n");
	return NULL;
};

void* NSIsControllerMarker(void) {
	if (verbose) LOG_FUNC("STUB: NSIsControllerMarker called\n");
	return NULL;
};

void* NSOpenGLGetOption(void) {
	if (verbose) LOG_FUNC("STUB: NSOpenGLGetOption called\n");
	return NULL;
};

void* NSOpenGLGetVersion(void) {
	if (verbose) LOG_FUNC("STUB: NSOpenGLGetVersion called\n");
	return NULL;
};

void* NSOpenGLSetOption(void) {
	if (verbose) LOG_FUNC("STUB: NSOpenGLSetOption called\n");
	return NULL;
};

void* NSRectClip(void) {
	if (verbose) LOG_FUNC("STUB: NSRectClip called\n");
	return NULL;
};

void* NSRectClipList(void) {
	if (verbose) LOG_FUNC("STUB: NSRectClipList called\n");
	return NULL;
};

void* NSRectFill(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFill called\n");
	return NULL;
};

void* NSRectFillList(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillList called\n");
	return NULL;
};

void* NSRectFillListUsingOperation(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillListUsingOperation called\n");
	return NULL;
};

void* NSRectFillListWithColors(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillListWithColors called\n");
	return NULL;
};

void* NSRectFillListWithColorsUsingOperation(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillListWithColorsUsingOperation called\n");
	return NULL;
};

void* NSRectFillListWithGrays(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillListWithGrays called\n");
	return NULL;
};

void* NSRectFillUsingOperation(void) {
	if (verbose) LOG_FUNC("STUB: NSRectFillUsingOperation called\n");
	return NULL;
};

void* NSRunAlertPanel(void) {
	if (verbose) LOG_FUNC("STUB: NSRunAlertPanel called\n");
	return NULL;
};

void* NSSetFocusRingStyle(void) {
	if (verbose) LOG_FUNC("STUB: NSSetFocusRingStyle called\n");
	return NULL;
};

void* NSShowAnimationEffect(void) {
	if (verbose) LOG_FUNC("STUB: NSShowAnimationEffect called\n");
	return NULL;
};

void* NSUpdateDynamicServices(void) {
	if (verbose) LOG_FUNC("STUB: NSUpdateDynamicServices called\n");
	return NULL;
};

void* _NSColorSpaceNameFromNum(void) {
	if (verbose) LOG_FUNC("STUB: _NSColorSpaceNameFromNum called\n");
	return NULL;
};

void* _NSNameFromKeyEquivalentString(void) {
	if (verbose) LOG_FUNC("STUB: _NSNameFromKeyEquivalentString called\n");
	return NULL;
};


