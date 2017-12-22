/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#include <DataDetectorsCore/DataDetectorsCore.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DDAddressBookAccessAllowed(void) {
    if (verbose) puts("STUB: DDAddressBookAccessAllowed called");
    return NULL;
}

void* DDAddressResultCanBeMergedWith(void) {
    if (verbose) puts("STUB: DDAddressResultCanBeMergedWith called");
    return NULL;
}

void* DDApplyOptions(void) {
    if (verbose) puts("STUB: DDApplyOptions called");
    return NULL;
}

void* DDCacheCopyStatsDescription(void) {
    if (verbose) puts("STUB: DDCacheCopyStatsDescription called");
    return NULL;
}

void* DDCacheCreateWithSubTable(void) {
    if (verbose) puts("STUB: DDCacheCreateWithSubTable called");
    return NULL;
}

void* DDCacheSetTemporaryDirectory(void) {
    if (verbose) puts("STUB: DDCacheSetTemporaryDirectory called");
    return NULL;
}

void* DDCacheWriteAtPath(void) {
    if (verbose) puts("STUB: DDCacheWriteAtPath called");
    return NULL;
}

void* DDCacheWriteWithSuffix(void) {
    if (verbose) puts("STUB: DDCacheWriteWithSuffix called");
    return NULL;
}

void* DDCalendarAccessAllowed(void) {
    if (verbose) puts("STUB: DDCalendarAccessAllowed called");
    return NULL;
}

void* DDCopyPunycodedWebURLString(void) {
    if (verbose) puts("STUB: DDCopyPunycodedWebURLString called");
    return NULL;
}

void* DDCrash(void) {
    if (verbose) puts("STUB: DDCrash called");
    return NULL;
}

void* DDCreateDataFromFileAtPath(void) {
    if (verbose) puts("STUB: DDCreateDataFromFileAtPath called");
    return NULL;
}

void* DDCreateDataFromFileAtUrl(void) {
    if (verbose) puts("STUB: DDCreateDataFromFileAtUrl called");
    return NULL;
}

void* DDCreateRelatedResultsArray(void) {
    if (verbose) puts("STUB: DDCreateRelatedResultsArray called");
    return NULL;
}

void* DDCreateTimeZoneValuesFromResultValue(void) {
    if (verbose) puts("STUB: DDCreateTimeZoneValuesFromResultValue called");
    return NULL;
}

void* DDDFACacheCreateFromBundle(void) {
    if (verbose) puts("STUB: DDDFACacheCreateFromBundle called");
    return NULL;
}

void* DDDFACacheCreateFromFramework(void) {
    if (verbose) puts("STUB: DDDFACacheCreateFromFramework called");
    return NULL;
}

void* DDDFAScannerCopyResults(void) {
    if (verbose) puts("STUB: DDDFAScannerCopyResults called");
    return NULL;
}

void* DDDFAScannerCreateFromCache(void) {
    if (verbose) puts("STUB: DDDFAScannerCreateFromCache called");
    return NULL;
}

void* DDDFAScannerEmitResult(void) {
    if (verbose) puts("STUB: DDDFAScannerEmitResult called");
    return NULL;
}

void* DDDFAScannerFirstResultInUnicharArray(void) {
    if (verbose) puts("STUB: DDDFAScannerFirstResultInUnicharArray called");
    return NULL;
}

void* DDDFAScannerGetCFTypeID(void) {
    if (verbose) puts("STUB: DDDFAScannerGetCFTypeID called");
    return NULL;
}

void* DDDFAScannerReset(void) {
    if (verbose) puts("STUB: DDDFAScannerReset called");
    return NULL;
}

void* DDDFAScannerScanQuery(void) {
    if (verbose) puts("STUB: DDDFAScannerScanQuery called");
    return NULL;
}

void* DDDFAScannerScanString(void) {
    if (verbose) puts("STUB: DDDFAScannerScanString called");
    return NULL;
}

void* DDDFAScannerScanStringWithRange(void) {
    if (verbose) puts("STUB: DDDFAScannerScanStringWithRange called");
    return NULL;
}

void* DDDFAScannerScanUnicharArray(void) {
    if (verbose) puts("STUB: DDDFAScannerScanUnicharArray called");
    return NULL;
}

void* DDDFAScannerScanUnicharArrayFragment(void) {
    if (verbose) puts("STUB: DDDFAScannerScanUnicharArrayFragment called");
    return NULL;
}

void* DDDFAScannerSimulateState(void) {
    if (verbose) puts("STUB: DDDFAScannerSimulateState called");
    return NULL;
}

void* DDDataDetectorBuilderSetup(void) {
    if (verbose) puts("STUB: DDDataDetectorBuilderSetup called");
    return NULL;
}

void* DDDataDetectorSetUsesDebugSource(void) {
    if (verbose) puts("STUB: DDDataDetectorSetUsesDebugSource called");
    return NULL;
}

void* DDError(void) {
    if (verbose) puts("STUB: DDError called");
    return NULL;
}

void* DDFastIntegerExtraction(void) {
    if (verbose) puts("STUB: DDFastIntegerExtraction called");
    return NULL;
}

void* DDForce24HrsFormatProcessWideForExtraction(void) {
    if (verbose) puts("STUB: DDForce24HrsFormatProcessWideForExtraction called");
    return NULL;
}

void* DDFullLogFunction(void) {
    if (verbose) puts("STUB: DDFullLogFunction called");
    return NULL;
}

void* DDInitialSetup(void) {
    if (verbose) puts("STUB: DDInitialSetup called");
    return NULL;
}

void* DDLog(void) {
    if (verbose) puts("STUB: DDLog called");
    return NULL;
}

void* DDLogAssertionFailure(void) {
    if (verbose) puts("STUB: DDLogAssertionFailure called");
    return NULL;
}

void* DDLogErrD(void) {
    if (verbose) puts("STUB: DDLogErrD called");
    return NULL;
}

void* DDLogHandle(void) {
    if (verbose) puts("STUB: DDLogHandle called");
    return NULL;
}

void* DDLogWarnD(void) {
    if (verbose) puts("STUB: DDLogWarnD called");
    return NULL;
}

void* DDLogv(void) {
    if (verbose) puts("STUB: DDLogv called");
    return NULL;
}

void* DDPatternLoaderCopyEnabledCategories(void) {
    if (verbose) puts("STUB: DDPatternLoaderCopyEnabledCategories called");
    return NULL;
}

void* DDPatternLoaderCopyLoadedFileURLs(void) {
    if (verbose) puts("STUB: DDPatternLoaderCopyLoadedFileURLs called");
    return NULL;
}

void* DDPatternLoaderCreateAndLoadAllFrameworkPatterns(void) {
    if (verbose) puts("STUB: DDPatternLoaderCreateAndLoadAllFrameworkPatterns called");
    return NULL;
}

void* DDPatternLoaderCreateCache(void) {
    if (verbose) puts("STUB: DDPatternLoaderCreateCache called");
    return NULL;
}

void* DDPatternLoaderCreateFromPaths(void) {
    if (verbose) puts("STUB: DDPatternLoaderCreateFromPaths called");
    return NULL;
}

void* DDPatternLoaderCreateFromPathsWithOptions(void) {
    if (verbose) puts("STUB: DDPatternLoaderCreateFromPathsWithOptions called");
    return NULL;
}

void* DDPatternLoaderCreateScanner(void) {
    if (verbose) puts("STUB: DDPatternLoaderCreateScanner called");
    return NULL;
}

void* DDResultAddSubresult(void) {
    if (verbose) puts("STUB: DDResultAddSubresult called");
    return NULL;
}

void* DDResultAddSubresultSorted(void) {
    if (verbose) puts("STUB: DDResultAddSubresultSorted called");
    return NULL;
}

void* DDResultCopyExtractedDateFromReferenceDate(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedDateFromReferenceDate called");
    return NULL;
}

void* DDResultCopyExtractedDateFromReferenceDateWithLocale(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedDateFromReferenceDateWithLocale called");
    return NULL;
}

void* DDResultCopyExtractedStartDateEndDate(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedStartDateEndDate called");
    return NULL;
}

void* DDResultCopyExtractedStartDateEndDateWithLocale(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedStartDateEndDateWithLocale called");
    return NULL;
}

void* DDResultCopyExtractedURL(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedURL called");
    return NULL;
}

void* DDResultCopyExtractedURLWithOptions(void) {
    if (verbose) puts("STUB: DDResultCopyExtractedURLWithOptions called");
    return NULL;
}

void* DDResultCopyFullAddressComponents(void) {
    if (verbose) puts("STUB: DDResultCopyFullAddressComponents called");
    return NULL;
}

void* DDResultCopyIMScreenNameValue(void) {
    if (verbose) puts("STUB: DDResultCopyIMScreenNameValue called");
    return NULL;
}

void* DDResultCopyMailValue(void) {
    if (verbose) puts("STUB: DDResultCopyMailValue called");
    return NULL;
}

void* DDResultCopyPhoneValue(void) {
    if (verbose) puts("STUB: DDResultCopyPhoneValue called");
    return NULL;
}

void* DDResultCreate(void) {
    if (verbose) puts("STUB: DDResultCreate called");
    return NULL;
}

void* DDResultCreateCopy(void) {
    if (verbose) puts("STUB: DDResultCreateCopy called");
    return NULL;
}

void* DDResultCreateEmpty(void) {
    if (verbose) puts("STUB: DDResultCreateEmpty called");
    return NULL;
}

void* DDResultCreateFromDateTimeResults(void) {
    if (verbose) puts("STUB: DDResultCreateFromDateTimeResults called");
    return NULL;
}

void* DDResultCreateFromXMLNode(void) {
    if (verbose) puts("STUB: DDResultCreateFromXMLNode called");
    return NULL;
}

void* DDResultCreateVerboseXMLDescription(void) {
    if (verbose) puts("STUB: DDResultCreateVerboseXMLDescription called");
    return NULL;
}

void* DDResultCreateVerboseXMLDescriptionFromResults(void) {
    if (verbose) puts("STUB: DDResultCreateVerboseXMLDescriptionFromResults called");
    return NULL;
}

void* DDResultCreateVerboseXMLRepresentation(void) {
    if (verbose) puts("STUB: DDResultCreateVerboseXMLRepresentation called");
    return NULL;
}

void* DDResultCreateXMLRepresentation(void) {
    if (verbose) puts("STUB: DDResultCreateXMLRepresentation called");
    return NULL;
}

void* DDResultCurrencyExtraction(void) {
    if (verbose) puts("STUB: DDResultCurrencyExtraction called");
    return NULL;
}

void* DDResultDateExtractionDependsOnContextTense(void) {
    if (verbose) puts("STUB: DDResultDateExtractionDependsOnContextTense called");
    return NULL;
}

void* DDResultExtractDayAndMonthWithSpecialDayIdentifierInYear(void) {
    if (verbose) puts("STUB: DDResultExtractDayAndMonthWithSpecialDayIdentifierInYear called");
    return NULL;
}

void* DDResultExtractHourMinuteSecondCopyTimeZone(void) {
    if (verbose) puts("STUB: DDResultExtractHourMinuteSecondCopyTimeZone called");
    return NULL;
}

void* DDResultGetCategory(void) {
    if (verbose) puts("STUB: DDResultGetCategory called");
    return NULL;
}

void* DDResultGetContextualData(void) {
    if (verbose) puts("STUB: DDResultGetContextualData called");
    return NULL;
}

void* DDResultGetContextualDataForKey(void) {
    if (verbose) puts("STUB: DDResultGetContextualDataForKey called");
    return NULL;
}

void* DDResultGetContextualFloat(void) {
    if (verbose) puts("STUB: DDResultGetContextualFloat called");
    return NULL;
}

void* DDResultGetCountryCode(void) {
    if (verbose) puts("STUB: DDResultGetCountryCode called");
    return NULL;
}

void* DDResultGetDuration(void) {
    if (verbose) puts("STUB: DDResultGetDuration called");
    return NULL;
}

void* DDResultGetMatchedString(void) {
    if (verbose) puts("STUB: DDResultGetMatchedString called");
    return NULL;
}

void* DDResultGetOptionalSourceTableIndex(void) {
    if (verbose) puts("STUB: DDResultGetOptionalSourceTableIndex called");
    return NULL;
}

void* DDResultGetParsecDomain(void) {
    if (verbose) puts("STUB: DDResultGetParsecDomain called");
    return NULL;
}

void* DDResultGetParsecRawDomain(void) {
    if (verbose) puts("STUB: DDResultGetParsecRawDomain called");
    return NULL;
}

void* DDResultGetParsecUUID(void) {
    if (verbose) puts("STUB: DDResultGetParsecUUID called");
    return NULL;
}

void* DDResultGetQueryRange(void) {
    if (verbose) puts("STUB: DDResultGetQueryRange called");
    return NULL;
}

void* DDResultGetQueryRangeForURLification(void) {
    if (verbose) puts("STUB: DDResultGetQueryRangeForURLification called");
    return NULL;
}

void* DDResultGetRange(void) {
    if (verbose) puts("STUB: DDResultGetRange called");
    return NULL;
}

void* DDResultGetRangeForURLification(void) {
    if (verbose) puts("STUB: DDResultGetRangeForURLification called");
    return NULL;
}

void* DDResultGetRawValue(void) {
    if (verbose) puts("STUB: DDResultGetRawValue called");
    return NULL;
}

void* DDResultGetScore(void) {
    if (verbose) puts("STUB: DDResultGetScore called");
    return NULL;
}

void* DDResultGetSubResults(void) {
    if (verbose) puts("STUB: DDResultGetSubResults called");
    return NULL;
}

void* DDResultGetSubresultWithType(void) {
    if (verbose) puts("STUB: DDResultGetSubresultWithType called");
    return NULL;
}

void* DDResultGetSubresultWithTypePath(void) {
    if (verbose) puts("STUB: DDResultGetSubresultWithTypePath called");
    return NULL;
}

void* DDResultGetTimeOffset(void) {
    if (verbose) puts("STUB: DDResultGetTimeOffset called");
    return NULL;
}

void* DDResultGetType(void) {
    if (verbose) puts("STUB: DDResultGetType called");
    return NULL;
}

void* DDResultGetValue(void) {
    if (verbose) puts("STUB: DDResultGetValue called");
    return NULL;
}

void* DDResultHasContextualFloat(void) {
    if (verbose) puts("STUB: DDResultHasContextualFloat called");
    return NULL;
}

void* DDResultHasProperties(void) {
    if (verbose) puts("STUB: DDResultHasProperties called");
    return NULL;
}

void* DDResultHasType(void) {
    if (verbose) puts("STUB: DDResultHasType called");
    return NULL;
}

void* DDResultIsDateRangeOrTimeRange(void) {
    if (verbose) puts("STUB: DDResultIsDateRangeOrTimeRange called");
    return NULL;
}

void* DDResultIsEqualToUnitTestResult(void) {
    if (verbose) puts("STUB: DDResultIsEqualToUnitTestResult called");
    return NULL;
}

void* DDResultIsPartialDateRangeOrTimeRange(void) {
    if (verbose) puts("STUB: DDResultIsPartialDateRangeOrTimeRange called");
    return NULL;
}

void* DDResultIsPastDate(void) {
    if (verbose) puts("STUB: DDResultIsPastDate called");
    return NULL;
}

void* DDResultIsPastDateWithLocale(void) {
    if (verbose) puts("STUB: DDResultIsPastDateWithLocale called");
    return NULL;
}

void* DDResultNeedsMeridianGuess(void) {
    if (verbose) puts("STUB: DDResultNeedsMeridianGuess called");
    return NULL;
}

void* DDResultProximitySort(void) {
    if (verbose) puts("STUB: DDResultProximitySort called");
    return NULL;
}

void* DDResultSelectBest(void) {
    if (verbose) puts("STUB: DDResultSelectBest called");
    return NULL;
}

void* DDResultSetAbsoluteRange(void) {
    if (verbose) puts("STUB: DDResultSetAbsoluteRange called");
    return NULL;
}

void* DDResultSetContent(void) {
    if (verbose) puts("STUB: DDResultSetContent called");
    return NULL;
}

void* DDResultSetContextualData(void) {
    if (verbose) puts("STUB: DDResultSetContextualData called");
    return NULL;
}

void* DDResultSetContextualFloat(void) {
    if (verbose) puts("STUB: DDResultSetContextualFloat called");
    return NULL;
}

void* DDResultSetLocaleForExtraction(void) {
    if (verbose) puts("STUB: DDResultSetLocaleForExtraction called");
    return NULL;
}

void* DDResultSetMatchedString(void) {
    if (verbose) puts("STUB: DDResultSetMatchedString called");
    return NULL;
}

void* DDResultSetScore(void) {
    if (verbose) puts("STUB: DDResultSetScore called");
    return NULL;
}

void* DDResultSetSubResults(void) {
    if (verbose) puts("STUB: DDResultSetSubResults called");
    return NULL;
}

void* DDResultSetType(void) {
    if (verbose) puts("STUB: DDResultSetType called");
    return NULL;
}

void* DDResultSetValue(void) {
    if (verbose) puts("STUB: DDResultSetValue called");
    return NULL;
}

void* DDResultTimeIsApprox(void) {
    if (verbose) puts("STUB: DDResultTimeIsApprox called");
    return NULL;
}

void* DDResultTypesAreEqual(void) {
    if (verbose) puts("STUB: DDResultTypesAreEqual called");
    return NULL;
}

void* DDScanQueryAddLineBreak(void) {
    if (verbose) puts("STUB: DDScanQueryAddLineBreak called");
    return NULL;
}

void* DDScanQueryAddSeparator(void) {
    if (verbose) puts("STUB: DDScanQueryAddSeparator called");
    return NULL;
}

void* DDScanQueryAddTextFragment(void) {
    if (verbose) puts("STUB: DDScanQueryAddTextFragment called");
    return NULL;
}

void* DDScanQueryCopyRange(void) {
    if (verbose) puts("STUB: DDScanQueryCopyRange called");
    return NULL;
}

void* DDScanQueryCreate(void) {
    if (verbose) puts("STUB: DDScanQueryCreate called");
    return NULL;
}

void* DDScanQueryCreateFromString(void) {
    if (verbose) puts("STUB: DDScanQueryCreateFromString called");
    return NULL;
}

void* DDScanQueryGetFragmentMetaData(void) {
    if (verbose) puts("STUB: DDScanQueryGetFragmentMetaData called");
    return NULL;
}

void* DDScanQueryGetFragmentRange(void) {
    if (verbose) puts("STUB: DDScanQueryGetFragmentRange called");
    return NULL;
}

void* DDScanQueryGetNumberOfFragments(void) {
    if (verbose) puts("STUB: DDScanQueryGetNumberOfFragments called");
    return NULL;
}

void* DDScanQueryHasNoLetterBeforeNextLine(void) {
    if (verbose) puts("STUB: DDScanQueryHasNoLetterBeforeNextLine called");
    return NULL;
}

void* DDScanQuerySetMetadataReleaseCallback(void) {
    if (verbose) puts("STUB: DDScanQuerySetMetadataReleaseCallback called");
    return NULL;
}

void* DDScannerCancelScanning(void) {
    if (verbose) puts("STUB: DDScannerCancelScanning called");
    return NULL;
}

void* DDScannerCopyItemSetForStateAtIndex(void) {
    if (verbose) puts("STUB: DDScannerCopyItemSetForStateAtIndex called");
    return NULL;
}

void* DDScannerCopyResults(void) {
    if (verbose) puts("STUB: DDScannerCopyResults called");
    return NULL;
}

void* DDScannerCopyResultsCheckOverlap(void) {
    if (verbose) puts("STUB: DDScannerCopyResultsCheckOverlap called");
    return NULL;
}

void* DDScannerCopyResultsWithOptions(void) {
    if (verbose) puts("STUB: DDScannerCopyResultsWithOptions called");
    return NULL;
}

void* DDScannerCopyStatsDescription(void) {
    if (verbose) puts("STUB: DDScannerCopyStatsDescription called");
    return NULL;
}

void* DDScannerCreate(void) {
    if (verbose) puts("STUB: DDScannerCreate called");
    return NULL;
}

void* DDScannerCreateFromCache(void) {
    if (verbose) puts("STUB: DDScannerCreateFromCache called");
    return NULL;
}

void* DDScannerCreateFromCacheData(void) {
    if (verbose) puts("STUB: DDScannerCreateFromCacheData called");
    return NULL;
}

void* DDScannerCreateWithCacheFile(void) {
    if (verbose) puts("STUB: DDScannerCreateWithCacheFile called");
    return NULL;
}

void* DDScannerCreateWithFileURL(void) {
    if (verbose) puts("STUB: DDScannerCreateWithFileURL called");
    return NULL;
}

void* DDScannerCreateWithLocale(void) {
    if (verbose) puts("STUB: DDScannerCreateWithLocale called");
    return NULL;
}

void* DDScannerCreateWithType(void) {
    if (verbose) puts("STUB: DDScannerCreateWithType called");
    return NULL;
}

void* DDScannerEnableDPReporting(void) {
    if (verbose) puts("STUB: DDScannerEnableDPReporting called");
    return NULL;
}

void* DDScannerEnableOptionalSource(void) {
    if (verbose) puts("STUB: DDScannerEnableOptionalSource called");
    return NULL;
}

void* DDScannerGetCache(void) {
    if (verbose) puts("STUB: DDScannerGetCache called");
    return NULL;
}

void* DDScannerGetExtendedResults(void) {
    if (verbose) puts("STUB: DDScannerGetExtendedResults called");
    return NULL;
}

void* DDScannerGetMemoryUsed(void) {
    if (verbose) puts("STUB: DDScannerGetMemoryUsed called");
    return NULL;
}

void* DDScannerGetOptions(void) {
    if (verbose) puts("STUB: DDScannerGetOptions called");
    return NULL;
}

void* DDScannerHasCapabilities(void) {
    if (verbose) puts("STUB: DDScannerHasCapabilities called");
    return NULL;
}

void* DDScannerLoadAddressBookData(void) {
    if (verbose) puts("STUB: DDScannerLoadAddressBookData called");
    return NULL;
}

void* DDScannerLoaderGetIdentifierStringForScannerType(void) {
    if (verbose) puts("STUB: DDScannerLoaderGetIdentifierStringForScannerType called");
    return NULL;
}

void* DDScannerLoaderScannerGetTypeWithIdentifierString(void) {
    if (verbose) puts("STUB: DDScannerLoaderScannerGetTypeWithIdentifierString called");
    return NULL;
}

void* DDScannerPrepareDynamicSources(void) {
    if (verbose) puts("STUB: DDScannerPrepareDynamicSources called");
    return NULL;
}

void* DDScannerPurgeCaches(void) {
    if (verbose) puts("STUB: DDScannerPurgeCaches called");
    return NULL;
}

void* DDScannerReset(void) {
    if (verbose) puts("STUB: DDScannerReset called");
    return NULL;
}

void* DDScannerResultCreateXMLDescription(void) {
    if (verbose) puts("STUB: DDScannerResultCreateXMLDescription called");
    return NULL;
}

void* DDScannerScanQuery(void) {
    if (verbose) puts("STUB: DDScannerScanQuery called");
    return NULL;
}

void* DDScannerScanString(void) {
    if (verbose) puts("STUB: DDScannerScanString called");
    return NULL;
}

void* DDScannerScanStringFromConversation(void) {
    if (verbose) puts("STUB: DDScannerScanStringFromConversation called");
    return NULL;
}

void* DDScannerScanStringWithContextOffset(void) {
    if (verbose) puts("STUB: DDScannerScanStringWithContextOffset called");
    return NULL;
}

void* DDScannerScanStringWithRange(void) {
    if (verbose) puts("STUB: DDScannerScanStringWithRange called");
    return NULL;
}

void* DDScannerSetClientTables(void) {
    if (verbose) puts("STUB: DDScannerSetClientTables called");
    return NULL;
}

void* DDScannerSetLoadDebugInformation(void) {
    if (verbose) puts("STUB: DDScannerSetLoadDebugInformation called");
    return NULL;
}

void* DDScannerSetLocale(void) {
    if (verbose) puts("STUB: DDScannerSetLocale called");
    return NULL;
}

void* DDScannerSetOptions(void) {
    if (verbose) puts("STUB: DDScannerSetOptions called");
    return NULL;
}

void* DDScannerSetProgressReportingBlock(void) {
    if (verbose) puts("STUB: DDScannerSetProgressReportingBlock called");
    return NULL;
}

void* DDScannerSetScannerTimeout(void) {
    if (verbose) puts("STUB: DDScannerSetScannerTimeout called");
    return NULL;
}

void* DDScannerSetStatsBuilder(void) {
    if (verbose) puts("STUB: DDScannerSetStatsBuilder called");
    return NULL;
}

void* DDScannerShouldKeepParsecScoresBelowThreshold(void) {
    if (verbose) puts("STUB: DDScannerShouldKeepParsecScoresBelowThreshold called");
    return NULL;
}

void* DDScannerThreadDebug(void) {
    if (verbose) puts("STUB: DDScannerThreadDebug called");
    return NULL;
}

void* DDSetCrashHandler(void) {
    if (verbose) puts("STUB: DDSetCrashHandler called");
    return NULL;
}

void* DDShouldUseDebugHighlightForResult(void) {
    if (verbose) puts("STUB: DDShouldUseDebugHighlightForResult called");
    return NULL;
}

void* DDShouldUseLightLinksForResult(void) {
    if (verbose) puts("STUB: DDShouldUseLightLinksForResult called");
    return NULL;
}

void* DDSourceClear(void) {
    if (verbose) puts("STUB: DDSourceClear called");
    return NULL;
}

void* DDSourceGetClientSignature(void) {
    if (verbose) puts("STUB: DDSourceGetClientSignature called");
    return NULL;
}

void* DDSourcePushFile(void) {
    if (verbose) puts("STUB: DDSourcePushFile called");
    return NULL;
}

void* DDSourcePushSharedContent(void) {
    if (verbose) puts("STUB: DDSourcePushSharedContent called");
    return NULL;
}

void* DDSourcePushSharedData(void) {
    if (verbose) puts("STUB: DDSourcePushSharedData called");
    return NULL;
}

void* DDSourcePushSharedDataWithSignature(void) {
    if (verbose) puts("STUB: DDSourcePushSharedDataWithSignature called");
    return NULL;
}

void* DDSourceSetStoragePath(void) {
    if (verbose) puts("STUB: DDSourceSetStoragePath called");
    return NULL;
}

void* DDStampContextDestroy(void) {
    if (verbose) puts("STUB: DDStampContextDestroy called");
    return NULL;
}

void* DDStampContextFillInWithTypeString(void) {
    if (verbose) puts("STUB: DDStampContextFillInWithTypeString called");
    return NULL;
}

void* DDSuffixCreateForString(void) {
    if (verbose) puts("STUB: DDSuffixCreateForString called");
    return NULL;
}

void* DDTrieLookupNormalizedStringCopy(void) {
    if (verbose) puts("STUB: DDTrieLookupNormalizedStringCopy called");
    return NULL;
}

void* DDURLWithPotentiallyInvalidURLString(void) {
    if (verbose) puts("STUB: DDURLWithPotentiallyInvalidURLString called");
    return NULL;
}

void* DDWriteFileAtPath(void) {
    if (verbose) puts("STUB: DDWriteFileAtPath called");
    return NULL;
}

void* DataDetectorsProximitySort(void) {
    if (verbose) puts("STUB: DataDetectorsProximitySort called");
    return NULL;
}

void* _DDDumpTrie(void) {
    if (verbose) puts("STUB: _DDDumpTrie called");
    return NULL;
}

void* _DDDumpTrieCells(void) {
    if (verbose) puts("STUB: _DDDumpTrieCells called");
    return NULL;
}

void* _ZN17dd_icu_3_6__5_0_2eqERKNS_11StringPieceES2_(void) {
    if (verbose) puts("STUB: _ZN17dd_icu_3_6__5_0_2eqERKNS_11StringPieceES2_ called");
    return NULL;
}

void* u_countChar32(void) {
    if (verbose) puts("STUB: u_countChar32 called");
    return NULL;
}

void* u_memchr(void) {
    if (verbose) puts("STUB: u_memchr called");
    return NULL;
}

void* u_memchr32(void) {
    if (verbose) puts("STUB: u_memchr32 called");
    return NULL;
}

void* u_memcmp(void) {
    if (verbose) puts("STUB: u_memcmp called");
    return NULL;
}

void* u_memcmpCodePointOrder(void) {
    if (verbose) puts("STUB: u_memcmpCodePointOrder called");
    return NULL;
}

void* u_memcpy(void) {
    if (verbose) puts("STUB: u_memcpy called");
    return NULL;
}

void* u_memmove(void) {
    if (verbose) puts("STUB: u_memmove called");
    return NULL;
}

void* u_memrchr(void) {
    if (verbose) puts("STUB: u_memrchr called");
    return NULL;
}

void* u_memrchr32(void) {
    if (verbose) puts("STUB: u_memrchr32 called");
    return NULL;
}

void* u_memset(void) {
    if (verbose) puts("STUB: u_memset called");
    return NULL;
}

void* u_setAtomicIncDecFunctions(void) {
    if (verbose) puts("STUB: u_setAtomicIncDecFunctions called");
    return NULL;
}

void* u_setMutexFunctions(void) {
    if (verbose) puts("STUB: u_setMutexFunctions called");
    return NULL;
}

void* u_strCompare(void) {
    if (verbose) puts("STUB: u_strCompare called");
    return NULL;
}

void* u_strCompareIter(void) {
    if (verbose) puts("STUB: u_strCompareIter called");
    return NULL;
}

void* u_strFindFirst(void) {
    if (verbose) puts("STUB: u_strFindFirst called");
    return NULL;
}

void* u_strFindLast(void) {
    if (verbose) puts("STUB: u_strFindLast called");
    return NULL;
}

void* u_strHasMoreChar32Than(void) {
    if (verbose) puts("STUB: u_strHasMoreChar32Than called");
    return NULL;
}

void* u_strcat(void) {
    if (verbose) puts("STUB: u_strcat called");
    return NULL;
}

void* u_strchr(void) {
    if (verbose) puts("STUB: u_strchr called");
    return NULL;
}

void* u_strchr32(void) {
    if (verbose) puts("STUB: u_strchr32 called");
    return NULL;
}

void* u_strcmp(void) {
    if (verbose) puts("STUB: u_strcmp called");
    return NULL;
}

void* u_strcmpCodePointOrder(void) {
    if (verbose) puts("STUB: u_strcmpCodePointOrder called");
    return NULL;
}

void* u_strcpy(void) {
    if (verbose) puts("STUB: u_strcpy called");
    return NULL;
}

void* u_strcspn(void) {
    if (verbose) puts("STUB: u_strcspn called");
    return NULL;
}

void* u_strlen(void) {
    if (verbose) puts("STUB: u_strlen called");
    return NULL;
}

void* u_strncat(void) {
    if (verbose) puts("STUB: u_strncat called");
    return NULL;
}

void* u_strncmp(void) {
    if (verbose) puts("STUB: u_strncmp called");
    return NULL;
}

void* u_strncmpCodePointOrder(void) {
    if (verbose) puts("STUB: u_strncmpCodePointOrder called");
    return NULL;
}

void* u_strncpy(void) {
    if (verbose) puts("STUB: u_strncpy called");
    return NULL;
}

void* u_strpbrk(void) {
    if (verbose) puts("STUB: u_strpbrk called");
    return NULL;
}

void* u_strrchr(void) {
    if (verbose) puts("STUB: u_strrchr called");
    return NULL;
}

void* u_strrchr32(void) {
    if (verbose) puts("STUB: u_strrchr32 called");
    return NULL;
}

void* u_strrstr(void) {
    if (verbose) puts("STUB: u_strrstr called");
    return NULL;
}

void* u_strspn(void) {
    if (verbose) puts("STUB: u_strspn called");
    return NULL;
}

void* u_strstr(void) {
    if (verbose) puts("STUB: u_strstr called");
    return NULL;
}

void* u_strtok_r(void) {
    if (verbose) puts("STUB: u_strtok_r called");
    return NULL;
}

void* u_terminateChars(void) {
    if (verbose) puts("STUB: u_terminateChars called");
    return NULL;
}

void* u_terminateUChar32s(void) {
    if (verbose) puts("STUB: u_terminateUChar32s called");
    return NULL;
}

void* u_terminateUChars(void) {
    if (verbose) puts("STUB: u_terminateUChars called");
    return NULL;
}

void* u_terminateWChars(void) {
    if (verbose) puts("STUB: u_terminateWChars called");
    return NULL;
}

void* u_unescape(void) {
    if (verbose) puts("STUB: u_unescape called");
    return NULL;
}

void* u_unescapeAt(void) {
    if (verbose) puts("STUB: u_unescapeAt called");
    return NULL;
}

void* umtx_atomic_dec(void) {
    if (verbose) puts("STUB: umtx_atomic_dec called");
    return NULL;
}

void* umtx_atomic_inc(void) {
    if (verbose) puts("STUB: umtx_atomic_inc called");
    return NULL;
}

void* umtx_lock(void) {
    if (verbose) puts("STUB: umtx_lock called");
    return NULL;
}

void* umtx_unlock(void) {
    if (verbose) puts("STUB: umtx_unlock called");
    return NULL;
}

void* ustr_hashCharsN(void) {
    if (verbose) puts("STUB: ustr_hashCharsN called");
    return NULL;
}

void* ustr_hashICharsN(void) {
    if (verbose) puts("STUB: ustr_hashICharsN called");
    return NULL;
}

void* ustr_hashUCharsN(void) {
    if (verbose) puts("STUB: ustr_hashUCharsN called");
    return NULL;
}
