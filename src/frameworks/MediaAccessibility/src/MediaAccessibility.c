/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <MediaAccessibility/MediaAccessibility.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* MAAudibleMediaCopyPreferredCharacteristics(void)
{
    if (verbose) puts("STUB: MAAudibleMediaCopyPreferredCharacteristics called");
    return NULL;
}

void* MAAudibleMediaPrefCopyPreferDescriptiveVideo(void)
{
    if (verbose) puts("STUB: MAAudibleMediaPrefCopyPreferDescriptiveVideo called");
    return NULL;
}

void* MAAudibleMediaPrefResetAll(void)
{
    if (verbose) puts("STUB: MAAudibleMediaPrefResetAll called");
    return NULL;
}

void* MAAudibleMediaPrefSetPreferDescriptiveVideo(void)
{
    if (verbose) puts("STUB: MAAudibleMediaPrefSetPreferDescriptiveVideo called");
    return NULL;
}

void* MACaptionAppearanceAddPreferredLanguage(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceAddPreferredLanguage called");
    return NULL;
}

void* MACaptionAppearanceAddSelectedLanguage(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceAddSelectedLanguage called");
    return NULL;
}

void* MACaptionAppearanceCopyBackgroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyBackgroundColor called");
    return NULL;
}

void* MACaptionAppearanceCopyFontDescriptorForLanguage(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyFontDescriptorForLanguage called");
    return NULL;
}

void* MACaptionAppearanceCopyFontDescriptorForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyFontDescriptorForStyle called");
    return NULL;
}

void* MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyFontDescriptorWithStrokeForStyle called");
    return NULL;
}

void* MACaptionAppearanceCopyFontForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyFontForStyle called");
    return NULL;
}

void* MACaptionAppearanceCopyForegroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyForegroundColor called");
    return NULL;
}

void* MACaptionAppearanceCopyPreferredCaptioningMediaCharacteristics(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyPreferredCaptioningMediaCharacteristics called");
    return NULL;
}

void* MACaptionAppearanceCopyPreferredLanguages(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyPreferredLanguages called");
    return NULL;
}

void* MACaptionAppearanceCopySelectedLanguages(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopySelectedLanguages called");
    return NULL;
}

void* MACaptionAppearanceCopyStrokeColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyStrokeColor called");
    return NULL;
}

void* MACaptionAppearanceCopyWindowColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceCopyWindowColor called");
    return NULL;
}

void* MACaptionAppearanceDidDisplayCaptions(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceDidDisplayCaptions called");
    return NULL;
}

void* MACaptionAppearanceGetBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetBackgroundOpacity called");
    return NULL;
}

void* MACaptionAppearanceGetDisplayType(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetDisplayType called");
    return NULL;
}

void* MACaptionAppearanceGetForegroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetForegroundOpacity called");
    return NULL;
}

void* MACaptionAppearanceGetRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetRelativeCharacterSize called");
    return NULL;
}

void* MACaptionAppearanceGetShowCaptions(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetShowCaptions called");
    return NULL;
}

void* MACaptionAppearanceGetShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetShowCaptionsWhenAvailable called");
    return NULL;
}

void* MACaptionAppearanceGetStrokeWidth(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetStrokeWidth called");
    return NULL;
}

void* MACaptionAppearanceGetTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetTextEdgeStyle called");
    return NULL;
}

void* MACaptionAppearanceGetWindowOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetWindowOpacity called");
    return NULL;
}

void* MACaptionAppearanceGetWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceGetWindowRoundedCornerRadius called");
    return NULL;
}

void* MACaptionAppearancePrefCopyActiveProfileID(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyActiveProfileID called");
    return NULL;
}

void* MACaptionAppearancePrefCopyBackgroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyBackgroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyBackgroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyDisplayType(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyDisplayType called");
    return NULL;
}

void* MACaptionAppearancePrefCopyFontDescriptorForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyFontDescriptorForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyFontForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyFontForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyFontSpecifiedByUserForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyFontSpecifiedByUserForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyForegroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyForegroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyForegroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyForegroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyPreferAccessibleCaptions(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyPreferAccessibleCaptions called");
    return NULL;
}

void* MACaptionAppearancePrefCopyPreferredLanguages(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyPreferredLanguages called");
    return NULL;
}

void* MACaptionAppearancePrefCopyProfileIDs(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyProfileIDs called");
    return NULL;
}

void* MACaptionAppearancePrefCopyProfileName(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyProfileName called");
    return NULL;
}

void* MACaptionAppearancePrefCopyProfileOrder(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyProfileOrder called");
    return NULL;
}

void* MACaptionAppearancePrefCopyRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyRelativeCharacterSize called");
    return NULL;
}

void* MACaptionAppearancePrefCopyShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyShowCaptionsWhenAvailable called");
    return NULL;
}

void* MACaptionAppearancePrefCopySystemLanguage(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopySystemLanguage called");
    return NULL;
}

void* MACaptionAppearancePrefCopyTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyTextEdgeStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyUsesAppleLookAndFeel(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyUsesAppleLookAndFeel called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideBackgroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideBackgroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideBackgroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideFontForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideFontForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideForegroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideForegroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideForegroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideForegroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideRelativeCharSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideRelativeCharSize called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideRelativeCharacterSize called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideTextEdgeStyle called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideWindowColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideWindowColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyVideoOverrideWindowOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyVideoOverrideWindowOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyWindowColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyWindowColor called");
    return NULL;
}

void* MACaptionAppearancePrefCopyWindowOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyWindowOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefCopyWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCopyWindowRoundedCornerRadius called");
    return NULL;
}

void* MACaptionAppearancePrefCreateNewProfileFromProfile(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCreateNewProfileFromProfile called");
    return NULL;
}

void* MACaptionAppearancePrefCreateNewProfileID(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefCreateNewProfileID called");
    return NULL;
}

void* MACaptionAppearancePrefGetRelativeCharSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefGetRelativeCharSize called");
    return NULL;
}

void* MACaptionAppearancePrefIsProfileEditable(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefIsProfileEditable called");
    return NULL;
}

void* MACaptionAppearancePrefRemoveProfile(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefRemoveProfile called");
    return NULL;
}

void* MACaptionAppearancePrefResetAll(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefResetAll called");
    return NULL;
}

void* MACaptionAppearancePrefResetProfile(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefResetProfile called");
    return NULL;
}

void* MACaptionAppearancePrefRestoreState(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefRestoreState called");
    return NULL;
}

void* MACaptionAppearancePrefSaveState(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSaveState called");
    return NULL;
}

void* MACaptionAppearancePrefSetActiveProfileID(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetActiveProfileID called");
    return NULL;
}

void* MACaptionAppearancePrefSetAllowNullProfile(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetAllowNullProfile called");
    return NULL;
}

void* MACaptionAppearancePrefSetBackgroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetBackgroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetBackgroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetDisplayType(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetDisplayType called");
    return NULL;
}

void* MACaptionAppearancePrefSetFontDescriptorForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetFontDescriptorForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetFontForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetFontForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetFontSpecifiedByUserForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetFontSpecifiedByUserForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetForegroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetForegroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetForegroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetForegroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetPreferAccessibleCaptions(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetPreferAccessibleCaptions called");
    return NULL;
}

void* MACaptionAppearancePrefSetPreferredLanguages(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetPreferredLanguages called");
    return NULL;
}

void* MACaptionAppearancePrefSetProfileName(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetProfileName called");
    return NULL;
}

void* MACaptionAppearancePrefSetProfileOrder(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetProfileOrder called");
    return NULL;
}

void* MACaptionAppearancePrefSetRelativeCharSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetRelativeCharSize called");
    return NULL;
}

void* MACaptionAppearancePrefSetRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetRelativeCharacterSize called");
    return NULL;
}

void* MACaptionAppearancePrefSetShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetShowCaptionsWhenAvailable called");
    return NULL;
}

void* MACaptionAppearancePrefSetSystemLanguageOverride(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetSystemLanguageOverride called");
    return NULL;
}

void* MACaptionAppearancePrefSetTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetTextEdgeStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetUsesAppleLookAndFeel(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetUsesAppleLookAndFeel called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideBackgroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideBackgroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideBackgroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideFontForStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideFontForStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideForegroundColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideForegroundColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideForegroundOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideForegroundOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideRelativeCharSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideRelativeCharSize called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideRelativeCharacterSize called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideTextEdgeStyle called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideWindowColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideWindowColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetVideoOverrideWindowOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetVideoOverrideWindowOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetWindowColor(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetWindowColor called");
    return NULL;
}

void* MACaptionAppearancePrefSetWindowOpacity(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetWindowOpacity called");
    return NULL;
}

void* MACaptionAppearancePrefSetWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MACaptionAppearancePrefSetWindowRoundedCornerRadius called");
    return NULL;
}

void* MACaptionAppearanceSetDidDisplayCaptionsCallback(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceSetDidDisplayCaptionsCallback called");
    return NULL;
}

void* MACaptionAppearanceSetDisplayType(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceSetDisplayType called");
    return NULL;
}

void* MACaptionAppearanceSetShowCaptions(void)
{
    if (verbose) puts("STUB: MACaptionAppearanceSetShowCaptions called");
    return NULL;
}

void* MADisplayFilterCombine(void)
{
    if (verbose) puts("STUB: MADisplayFilterCombine called");
    return NULL;
}

void* MADisplayFilterCopySystemFilter(void)
{
    if (verbose) puts("STUB: MADisplayFilterCopySystemFilter called");
    return NULL;
}

void* MADisplayFilterCreateBlueColorCorrection(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateBlueColorCorrection called");
    return NULL;
}

void* MADisplayFilterCreateBrightnessAdjust(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateBrightnessAdjust called");
    return NULL;
}

void* MADisplayFilterCreateGrayscale(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateGrayscale called");
    return NULL;
}

void* MADisplayFilterCreateGreenColorCorrection(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateGreenColorCorrection called");
    return NULL;
}

void* MADisplayFilterCreateIdentity(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateIdentity called");
    return NULL;
}

void* MADisplayFilterCreateInterpolated(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateInterpolated called");
    return NULL;
}

void* MADisplayFilterCreateRedBlack(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateRedBlack called");
    return NULL;
}

void* MADisplayFilterCreateRedColorCorrection(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateRedColorCorrection called");
    return NULL;
}

void* MADisplayFilterCreateRedNightMode(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateRedNightMode called");
    return NULL;
}

void* MADisplayFilterCreateSaturation(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateSaturation called");
    return NULL;
}

void* MADisplayFilterCreateSingleColor(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateSingleColor called");
    return NULL;
}

void* MADisplayFilterCreateTwoColor(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateTwoColor called");
    return NULL;
}

void* MADisplayFilterCreateWarmthAdjust(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateWarmthAdjust called");
    return NULL;
}

void* MADisplayFilterCreateYellowBlack(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateYellowBlack called");
    return NULL;
}

void* MADisplayFilterCreateYellowBlue(void)
{
    if (verbose) puts("STUB: MADisplayFilterCreateYellowBlue called");
    return NULL;
}

void* MADisplayFilterGetGain(void)
{
    if (verbose) puts("STUB: MADisplayFilterGetGain called");
    return NULL;
}

void* MADisplayFilterGetMatrix(void)
{
    if (verbose) puts("STUB: MADisplayFilterGetMatrix called");
    return NULL;
}

void* MADisplayFilterGetPostOffsets(void)
{
    if (verbose) puts("STUB: MADisplayFilterGetPostOffsets called");
    return NULL;
}

void* MADisplayFilterGetReduceWhitePoint(void)
{
    if (verbose) puts("STUB: MADisplayFilterGetReduceWhitePoint called");
    return NULL;
}

void* MADisplayFilterGetTypeID(void)
{
    if (verbose) puts("STUB: MADisplayFilterGetTypeID called");
    return NULL;
}

void* MADisplayFilterInversionCompensationForNightShift(void)
{
    if (verbose) puts("STUB: MADisplayFilterInversionCompensationForNightShift called");
    return NULL;
}

void* MADisplayFilterPrefCopyCategoriesForCurrentPlatform(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefCopyCategoriesForCurrentPlatform called");
    return NULL;
}

void* MADisplayFilterPrefGetBlueColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetBlueColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetBrightnessIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetBrightnessIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetCategoryEnabled(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetCategoryEnabled called");
    return NULL;
}

void* MADisplayFilterPrefGetGreenColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetGreenColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetRedColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetRedColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetReduceWhitePointIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetReduceWhitePointIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetSingleColorHue(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetSingleColorHue called");
    return NULL;
}

void* MADisplayFilterPrefGetSingleColorIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetSingleColorIntensity called");
    return NULL;
}

void* MADisplayFilterPrefGetType(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetType called");
    return NULL;
}

void* MADisplayFilterPrefGetWarmthIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefGetWarmthIntensity called");
    return NULL;
}

void* MADisplayFilterPrefPossibleTypesForCategory(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefPossibleTypesForCategory called");
    return NULL;
}

void* MADisplayFilterPrefResetAll(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefResetAll called");
    return NULL;
}

void* MADisplayFilterPrefSetBlueColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetBlueColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetBrightnessIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetBrightnessIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetCategoryEnabled(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetCategoryEnabled called");
    return NULL;
}

void* MADisplayFilterPrefSetGreenColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetGreenColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetRedColorCorrectionIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetRedColorCorrectionIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetReduceWhitePointIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetReduceWhitePointIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetSingleColorHue(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetSingleColorHue called");
    return NULL;
}

void* MADisplayFilterPrefSetSingleColorIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetSingleColorIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSetType(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetType called");
    return NULL;
}

void* MADisplayFilterPrefSetWarmthIntensity(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSetWarmthIntensity called");
    return NULL;
}

void* MADisplayFilterPrefSuspendNotifications(void)
{
    if (verbose) puts("STUB: MADisplayFilterPrefSuspendNotifications called");
    return NULL;
}

void* MADisplayFilterSetGain(void)
{
    if (verbose) puts("STUB: MADisplayFilterSetGain called");
    return NULL;
}

void* MADisplayFilterSetMatrix(void)
{
    if (verbose) puts("STUB: MADisplayFilterSetMatrix called");
    return NULL;
}

void* MADisplayFilterSetPostOffsets(void)
{
    if (verbose) puts("STUB: MADisplayFilterSetPostOffsets called");
    return NULL;
}

void* MADisplayFilterSetReduceWhitePoint(void)
{
    if (verbose) puts("STUB: MADisplayFilterSetReduceWhitePoint called");
    return NULL;
}

void* MAImageCaptioningCopyCaption(void)
{
    if (verbose) puts("STUB: MAImageCaptioningCopyCaption called");
    return NULL;
}

void* MAImageCaptioningCopyCaptionWithSource(void)
{
    if (verbose) puts("STUB: MAImageCaptioningCopyCaptionWithSource called");
    return NULL;
}

void* MAImageCaptioningCopyMetadataTagPath(void)
{
    if (verbose) puts("STUB: MAImageCaptioningCopyMetadataTagPath called");
    return NULL;
}

void* MAImageCaptioningSetCaption(void)
{
    if (verbose) puts("STUB: MAImageCaptioningSetCaption called");
    return NULL;
}

void* MALegibleOutputDidOutputAttributedStrings(void)
{
    if (verbose) puts("STUB: MALegibleOutputDidOutputAttributedStrings called");
    return NULL;
}

void* MALegibleOutputDidOutputAttributedStringsRegisterCallback(void)
{
    if (verbose) puts("STUB: MALegibleOutputDidOutputAttributedStringsRegisterCallback called");
    return NULL;
}

void* MATestingCopyProfileIDs(void)
{
    if (verbose) puts("STUB: MATestingCopyProfileIDs called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyWindowColor called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorCopyWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorCopyWindowRoundedCornerRadius called");
    return NULL;
}

void* MAXCaptionAppearanceBehaviorGetTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceBehaviorGetTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearanceCopyBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceCopyBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearanceCopyFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceCopyFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearanceCopyForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceCopyForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearanceCopyPreferredLanguages(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceCopyPreferredLanguages called");
    return NULL;
}

void* MAXCaptionAppearanceCopyWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceCopyWindowColor called");
    return NULL;
}

void* MAXCaptionAppearanceGetBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceGetForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceGetRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearanceGetTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearanceGetWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearanceGetWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceGetWindowRoundedCornerRadius called");
    return NULL;
}

void* MAXCaptionAppearanceHasUserDefinedValues(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceHasUserDefinedValues called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyActiveProfileID(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyActiveProfileID called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyPreferredLanguages(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyPreferredLanguages called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyProfileIDs(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyProfileIDs called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyProfileName(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyProfileName called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyProfileOrder(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyProfileOrder called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyShowCaptionsWhenAvailable called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyUsesAppleLookAndFeel(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyUsesAppleLookAndFeel called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideWindowColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyVideoOverrideWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyVideoOverrideWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyWindowColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefCopyWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCopyWindowRoundedCornerRadius called");
    return NULL;
}

void* MAXCaptionAppearancePrefCreateNewProfileFromProfile(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCreateNewProfileFromProfile called");
    return NULL;
}

void* MAXCaptionAppearancePrefCreateNewProfileID(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefCreateNewProfileID called");
    return NULL;
}

void* MAXCaptionAppearancePrefIsProfileEditable(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefIsProfileEditable called");
    return NULL;
}

void* MAXCaptionAppearancePrefRemoveProfile(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefRemoveProfile called");
    return NULL;
}

void* MAXCaptionAppearancePrefResetAll(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefResetAll called");
    return NULL;
}

void* MAXCaptionAppearancePrefResetProfile(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefResetProfile called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetActiveProfileID(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetActiveProfileID called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetPreferredLanguages(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetPreferredLanguages called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetProfileName(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetProfileName called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetProfileOrder(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetProfileOrder called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetShowCaptionsWhenAvailable called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetUsesAppleLookAndFeel(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetUsesAppleLookAndFeel called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideBackgroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideBackgroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideBackgroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideBackgroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideFontForStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideFontForStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideForegroundColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideForegroundColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideForegroundOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideForegroundOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideRelativeCharacterSize(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideRelativeCharacterSize called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideTextEdgeStyle(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideTextEdgeStyle called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideWindowColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetVideoOverrideWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetVideoOverrideWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetWindowColor(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetWindowColor called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetWindowOpacity(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetWindowOpacity called");
    return NULL;
}

void* MAXCaptionAppearancePrefSetWindowRoundedCornerRadius(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearancePrefSetWindowRoundedCornerRadius called");
    return NULL;
}

void* MAXCaptionAppearanceSetPreferredLanguage(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceSetPreferredLanguage called");
    return NULL;
}

void* MAXCaptionAppearanceShowCaptionsWhenAvailable(void)
{
    if (verbose) puts("STUB: MAXCaptionAppearanceShowCaptionsWhenAvailable called");
    return NULL;
}
