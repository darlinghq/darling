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

#include <ATS/ATS.h>
#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

const CFStringRef kATSAutoActivationConfirmDontShowAgainKey = CFSTR("DontShow");
const CFStringRef kATSAutoActivationConfirmResultKey = CFSTR("Result");
const CFStringRef kATSValidationActivationInfo = CFSTR("activationinfo");
const CFStringRef kATSValidationActivationInfoKeep = CFSTR("activationinforetain");
const CFStringRef kATSValidationDontRunHiLevelAPITests = CFSTR("hilevelapitests");
const CFStringRef kATSValidationDynamicActivationInfo = CFSTR("getactivationinto");
const CFStringRef kATSValidationDynamicGlyphTests = CFSTR("performglyphtests");
const CFStringRef kATSValidationFontMD5 = CFSTR("md5");
const CFStringRef kATSValidationFontSummaryOnly = CFSTR("fontsummary");
const CFStringRef kATSValidationGenerateFontChecksum = CFSTR("generatechecksum");
const CFStringRef kATSValidationIgnoreDumpFiles = CFSTR("ignoredumpfiles");
const CFStringRef kATSValidationIgnoreErrors = CFSTR("ignoreerrors");
const CFStringRef kATSValidationOverallResult = CFSTR("overallresult");
const CFStringRef kATSValidationPostScriptName = CFSTR("fontname");
const CFStringRef kATSValidationResults = CFSTR("results");
const CFStringRef kATSValidationRuleAuthor = CFSTR("author");
const CFStringRef kATSValidationRuleComponents = CFSTR("components");
const CFStringRef kATSValidationRuleCoverage = CFSTR("coverage");
const CFStringRef kATSValidationRuleDescription = CFSTR("kATSFontTestDescriptionKey");
const CFStringRef kATSValidationRuleFormat = CFSTR("format");
const CFStringRef kATSValidationRuleName = CFSTR("kATSFontTestNameKey");
const CFStringRef kATSValidationRulePriority = CFSTR("priority");
const CFStringRef kATSValidationRuleResult = CFSTR("result");
const CFStringRef kATSValidationRuleSeverity = CFSTR("severity");
const CFStringRef kATSValidationRuleVersion = CFSTR("version");
const CFStringRef kATSValidationRulesList = CFSTR("ruleslist");
const CFStringRef kATSValidationRunDynamic = CFSTR("dynamic");
const CFStringRef kATSValidationSkipDuplicateFontCheck = CFSTR("skipduplicates");
const CFStringRef kATSValidationTestsFailed = CFSTR("fail");
const CFStringRef kATSValidationTestsPassed = CFSTR("pass");
const CFStringRef kATSValidationTestsRun = CFSTR("run");
const CFStringRef kATSValidationTestsWarned = CFSTR("warn");

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

ATSFontRef ATSFontFindFromName(CFStringRef a, ATSOptionFlags b)
{
    if (verbose) puts("STUB: ATSFontFindFromName called");
	return 0;
}

OSStatus ATSFontGetHorizontalMetrics(ATSFontRef a, ATSOptionFlags b, ATSFontMetrics *c)
{
    if (verbose) puts("STUB: ATSFontGetHorizontalMetrics called");
	return 0;
}

OSStatus ATSUCreateStyle(ATSUStyle *a)
{
    if (verbose) puts("STUB: ATSUCreateStyle called");
	return 0;
}

OSStatus ATSUCreateTextLayout(ATSUTextLayout *a)
{
    if (verbose) puts("STUB: ATSUCreateTextLayout called");
	return 0;
}

OSStatus ATSUDisposeStyle(ATSUStyle *a)
{
    if (verbose) puts("STUB: ATSUDisposeStyle called");
	return 0;
}

OSStatus ATSUDisposeTextLayout(ATSUTextLayout *a)
{
    if (verbose) puts("STUB: ATSUDisposeTextLayout called");
	return 0;
}

OSStatus ATSUDrawText(ATSUTextLayout a, UniCharArrayOffset b, UniCharCount c, ATSUTextMeasurement d, ATSUTextMeasurement e)
{
    if (verbose) puts("STUB: ATSUDrawText called");
	return 0;
}

OSStatus ATSUGetUnjustifiedBounds(ATSUTextLayout a , UniCharArrayOffset b , UniCharCount c , ATSUTextMeasurement * d , ATSUTextMeasurement * e , ATSUTextMeasurement * f , ATSUTextMeasurement * g)
{
    if (verbose) puts("STUB: ATSUGetUnjustifiedBounds called");
	return 0;
}

OSStatus ATSUMeasureTextImage(ATSUTextLayout a, UniCharArrayOffset b, UniCharCount c, ATSUTextMeasurement d, ATSUTextMeasurement e, Rect *f)
{
    if (verbose) puts("STUB: ATSUMeasureTextImage called");
	return 0;
}

OSStatus ATSUSetAttributes(ATSUStyle a, ItemCount b, const ATSUAttributeTag *c, const ByteCount *d, const ATSUAttributeValuePtr *e)
{
    if (verbose) puts("STUB: ATSUSetAttributes called");
	return 0;
}

OSStatus ATSUSetLayoutControls(ATSUTextLayout a, ItemCount b, const ATSUAttributeTag *c, const ByteCount *d, const ATSUAttributeValuePtr *e)
{
    if (verbose) puts("STUB: ATSUSetLayoutControls called");
	return 0;
}

OSStatus ATSUSetRunStyle(ATSUTextLayout a, ATSUStyle b, UniCharArrayOffset c, UniCharCount d)
{
    if (verbose) puts("STUB: ATSUSetRunStyle called");
	return 0;
}

OSStatus ATSUSetTextPointerLocation(ATSUTextLayout a, ConstUniCharArrayPtr b, UniCharArrayOffset c, UniCharCount d, UniCharCount e)
{
    if (verbose) puts("STUB: ATSUSetTextPointerLocation called");
	return 0;
}

OSStatus ATSUSetTransientFontMatching(ATSUTextLayout a, Boolean b)
{
    if (verbose) puts("STUB: ATSUSetTransientFontMatching called");
	return 0;
}
