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


#include <PrintCore/PrintCore.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CPLAutoSelectCancel(void)
{
    if (verbose) puts("STUB: CPLAutoSelectCancel called");
    return NULL;
}

void* CPLAutoSelectCopyLocation(void)
{
    if (verbose) puts("STUB: CPLAutoSelectCopyLocation called");
    return NULL;
}

void* CPLAutoSelectCreateContext(void)
{
    if (verbose) puts("STUB: CPLAutoSelectCreateContext called");
    return NULL;
}

void* CPLAutoSelectGetInfo(void)
{
    if (verbose) puts("STUB: CPLAutoSelectGetInfo called");
    return NULL;
}

void* CPLAutoSelectHasNewPPD(void)
{
    if (verbose) puts("STUB: CPLAutoSelectHasNewPPD called");
    return NULL;
}

void* CPLAutoSelectInstallableOptions(void)
{
    if (verbose) puts("STUB: CPLAutoSelectInstallableOptions called");
    return NULL;
}

void* CPLAutoSelectIsBinaryOK(void)
{
    if (verbose) puts("STUB: CPLAutoSelectIsBinaryOK called");
    return NULL;
}

void* CPLAutoSelectPPD(void)
{
    if (verbose) puts("STUB: CPLAutoSelectPPD called");
    return NULL;
}

void* CPLAutoSelectReleaseContext(void)
{
    if (verbose) puts("STUB: CPLAutoSelectReleaseContext called");
    return NULL;
}

void* CPLAutoSelectSetIsBinaryOK(void)
{
    if (verbose) puts("STUB: CPLAutoSelectSetIsBinaryOK called");
    return NULL;
}

void* CPLAutoSetupCanGetInstallableOptions(void)
{
    if (verbose) puts("STUB: CPLAutoSetupCanGetInstallableOptions called");
    return NULL;
}

void* CPLCopyDefaultPaperID(void)
{
    if (verbose) puts("STUB: CPLCopyDefaultPaperID called");
    return NULL;
}

void* CPLCopyDefaultPrinterName(void)
{
    if (verbose) puts("STUB: CPLCopyDefaultPrinterName called");
    return NULL;
}

void* CPLCopyGenericPPDURL(void)
{
    if (verbose) puts("STUB: CPLCopyGenericPPDURL called");
    return NULL;
}

void* CPLCopyLocalServerURL(void)
{
    if (verbose) puts("STUB: CPLCopyLocalServerURL called");
    return NULL;
}

void* CPLCopyM5DefaultPrinterName(void)
{
    if (verbose) puts("STUB: CPLCopyM5DefaultPrinterName called");
    return NULL;
}

void* CPLCopyPrinterNames(void)
{
    if (verbose) puts("STUB: CPLCopyPrinterNames called");
    return NULL;
}

void* CPLCreateOptions(void)
{
    if (verbose) puts("STUB: CPLCreateOptions called");
    return NULL;
}

void* CPLCreateOptionsStr(void)
{
    if (verbose) puts("STUB: CPLCreateOptionsStr called");
    return NULL;
}

void* CPLCreateSuggestedQueueName(void)
{
    if (verbose) puts("STUB: CPLCreateSuggestedQueueName called");
    return NULL;
}

void* CPLEmptyPPDCache(void)
{
    if (verbose) puts("STUB: CPLEmptyPPDCache called");
    return NULL;
}

void* CPLEmptyPrintersCache(void)
{
    if (verbose) puts("STUB: CPLEmptyPrintersCache called");
    return NULL;
}

void* CPLGetErrorString(void)
{
    if (verbose) puts("STUB: CPLGetErrorString called");
    return NULL;
}

void* CPLInstallIconFile(void)
{
    if (verbose) puts("STUB: CPLInstallIconFile called");
    return NULL;
}

void* CPLIsPrintAdmin(void)
{
    if (verbose) puts("STUB: CPLIsPrintAdmin called");
    return NULL;
}

void* CPLIsPrintingManaged(void)
{
    if (verbose) puts("STUB: CPLIsPrintingManaged called");
    return NULL;
}

void* CPLPrinterCopyPPD(void)
{
    if (verbose) puts("STUB: CPLPrinterCopyPPD called");
    return NULL;
}

void* CPLPrinterGetType(void)
{
    if (verbose) puts("STUB: CPLPrinterGetType called");
    return NULL;
}

void* CPLPrinterNameInUse(void)
{
    if (verbose) puts("STUB: CPLPrinterNameInUse called");
    return NULL;
}

void* CPLPrinterSetLocation(void)
{
    if (verbose) puts("STUB: CPLPrinterSetLocation called");
    return NULL;
}

void* CPLPrinterSetPPD(void)
{
    if (verbose) puts("STUB: CPLPrinterSetPPD called");
    return NULL;
}

void* CPLQueuePrintDocument(void)
{
    if (verbose) puts("STUB: CPLQueuePrintDocument called");
    return NULL;
}

void* CPLQueueResume(void)
{
    if (verbose) puts("STUB: CPLQueueResume called");
    return NULL;
}

void* CPLRemovePrinter(void)
{
    if (verbose) puts("STUB: CPLRemovePrinter called");
    return NULL;
}

void* CPLRemovePrinterKeepPreferences(void)
{
    if (verbose) puts("STUB: CPLRemovePrinterKeepPreferences called");
    return NULL;
}

void* CPLResetPrintingPermissions(void)
{
    if (verbose) puts("STUB: CPLResetPrintingPermissions called");
    return NULL;
}

void* CPLResetPrintingSystem(void)
{
    if (verbose) puts("STUB: CPLResetPrintingSystem called");
    return NULL;
}

void* CPLServerCreateClass(void)
{
    if (verbose) puts("STUB: CPLServerCreateClass called");
    return NULL;
}

void* CPLServerCreateQueue(void)
{
    if (verbose) puts("STUB: CPLServerCreateQueue called");
    return NULL;
}

void* CPLSetDefaultPaperID(void)
{
    if (verbose) puts("STUB: CPLSetDefaultPaperID called");
    return NULL;
}

void* CPLSetDefaultPrinterName(void)
{
    if (verbose) puts("STUB: CPLSetDefaultPrinterName called");
    return NULL;
}

void* CPLSetUseLastPrinterAsCurrentPrinter(void)
{
    if (verbose) puts("STUB: CPLSetUseLastPrinterAsCurrentPrinter called");
    return NULL;
}

void* CPLStartAirScanLegacyBrowsing(void)
{
    if (verbose) puts("STUB: CPLStartAirScanLegacyBrowsing called");
    return NULL;
}

void* CPLUseLastPrinterAsCurrentPrinter(void)
{
    if (verbose) puts("STUB: CPLUseLastPrinterAsCurrentPrinter called");
    return NULL;
}

void* CreateCStrFromEncodedValue(void)
{
    if (verbose) puts("STUB: CreateCStrFromEncodedValue called");
    return NULL;
}

void* FindBestMediaForSheetSize(void)
{
    if (verbose) puts("STUB: FindBestMediaForSheetSize called");
    return NULL;
}

void* GetPPDDefaultColorSpace(void)
{
    if (verbose) puts("STUB: GetPPDDefaultColorSpace called");
    return NULL;
}

void* GetPPDRefFromSession(void)
{
    if (verbose) puts("STUB: GetPPDRefFromSession called");
    return NULL;
}

void* MakePrinterDeviceIDFromStr(void)
{
    if (verbose) puts("STUB: MakePrinterDeviceIDFromStr called");
    return NULL;
}

void* PJCCopyArrayPresetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyArrayPresetPrintingPref called");
    return NULL;
}

void* PJCCopyArrayPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyArrayPrintingPref called");
    return NULL;
}

void* PJCCopyBooleanPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyBooleanPrintingPref called");
    return NULL;
}

void* PJCCopyDictionaryPresetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyDictionaryPresetPrintingPref called");
    return NULL;
}

void* PJCCopyDictionaryPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyDictionaryPrintingPref called");
    return NULL;
}

void* PJCCopyNumberPresetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyNumberPresetPrintingPref called");
    return NULL;
}

void* PJCCopyNumberPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyNumberPrintingPref called");
    return NULL;
}

void* PJCCopyPrinterPrimaryProfileFromSettings(void)
{
    if (verbose) puts("STUB: PJCCopyPrinterPrimaryProfileFromSettings called");
    return NULL;
}

void* PJCCopyStringPresetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyStringPresetPrintingPref called");
    return NULL;
}

void* PJCCopyStringPrintingPref(void)
{
    if (verbose) puts("STUB: PJCCopyStringPrintingPref called");
    return NULL;
}

void* PJCCreateAndLaunchPrinterProxy(void)
{
    if (verbose) puts("STUB: PJCCreateAndLaunchPrinterProxy called");
    return NULL;
}

void* PJCCreateDefaultSaveToFileName(void)
{
    if (verbose) puts("STUB: PJCCreateDefaultSaveToFileName called");
    return NULL;
}

void* PJCCreateLocalizedNameFromPPDChoiceName(void)
{
    if (verbose) puts("STUB: PJCCreateLocalizedNameFromPPDChoiceName called");
    return NULL;
}

void* PJCCreateLocalizedPaperName2(void)
{
    if (verbose) puts("STUB: PJCCreateLocalizedPaperName2 called");
    return NULL;
}

void* PJCCreateLocalizedPaperNameWithPPD(void)
{
    if (verbose) puts("STUB: PJCCreateLocalizedPaperNameWithPPD called");
    return NULL;
}

void* PJCCreatePMPaperForPaperInfoEntry(void)
{
    if (verbose) puts("STUB: PJCCreatePMPaperForPaperInfoEntry called");
    return NULL;
}

void* PJCCreatePaperInfoListForMenuInternal(void)
{
    if (verbose) puts("STUB: PJCCreatePaperInfoListForMenuInternal called");
    return NULL;
}

void* PJCCreateSuggestedPaper(void)
{
    if (verbose) puts("STUB: PJCCreateSuggestedPaper called");
    return NULL;
}

void* PJCForceRelease(void)
{
    if (verbose) puts("STUB: PJCForceRelease called");
    return NULL;
}

void* PJCGetCurrentProfileIDFromPPD(void)
{
    if (verbose) puts("STUB: PJCGetCurrentProfileIDFromPPD called");
    return NULL;
}

void* PJCIsMetric(void)
{
    if (verbose) puts("STUB: PJCIsMetric called");
    return NULL;
}

void* PJCLaunchPrintCenter(void)
{
    if (verbose) puts("STUB: PJCLaunchPrintCenter called");
    return NULL;
}

void* PJCNewPageFormatWithPMPaper(void)
{
    if (verbose) puts("STUB: PJCNewPageFormatWithPMPaper called");
    return NULL;
}

void* PJCPrintSettingsCreateCustomProfileRef(void)
{
    if (verbose) puts("STUB: PJCPrintSettingsCreateCustomProfileRef called");
    return NULL;
}

void* PJCPrintSettingsSetCustomProfilePath(void)
{
    if (verbose) puts("STUB: PJCPrintSettingsSetCustomProfilePath called");
    return NULL;
}

void* PJCPrinterCopyReadyMediaList(void)
{
    if (verbose) puts("STUB: PJCPrinterCopyReadyMediaList called");
    return NULL;
}

void* PJCPrinterGetDeviceID(void)
{
    if (verbose) puts("STUB: PJCPrinterGetDeviceID called");
    return NULL;
}

void* PJCPrinterProxyCreateURL(void)
{
    if (verbose) puts("STUB: PJCPrinterProxyCreateURL called");
    return NULL;
}

void* PJCPrinterSupportsReadyMedia(void)
{
    if (verbose) puts("STUB: PJCPrinterSupportsReadyMedia called");
    return NULL;
}

void* PJCPrinterSupportsThisCustomPaper(void)
{
    if (verbose) puts("STUB: PJCPrinterSupportsThisCustomPaper called");
    return NULL;
}

void* PJCRunWorkflowWithRenamedPDF(void)
{
    if (verbose) puts("STUB: PJCRunWorkflowWithRenamedPDF called");
    return NULL;
}

void* PJCSessionCreateCopyForAppKit(void)
{
    if (verbose) puts("STUB: PJCSessionCreateCopyForAppKit called");
    return NULL;
}

void* PJCSessionHasApplicationSetPrinter(void)
{
    if (verbose) puts("STUB: PJCSessionHasApplicationSetPrinter called");
    return NULL;
}

void* PJCSessionSetHideFileExtension(void)
{
    if (verbose) puts("STUB: PJCSessionSetHideFileExtension called");
    return NULL;
}

void* PJCSetPresetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCSetPresetPrintingPref called");
    return NULL;
}

void* PJCSetPrintingPref(void)
{
    if (verbose) puts("STUB: PJCSetPrintingPref called");
    return NULL;
}

void* PJCTicketGetSrcImageableArea(void)
{
    if (verbose) puts("STUB: PJCTicketGetSrcImageableArea called");
    return NULL;
}

void* PJCTicketGetSrcOrientation(void)
{
    if (verbose) puts("STUB: PJCTicketGetSrcOrientation called");
    return NULL;
}

void* PJCTicketGetSrcPaperSize(void)
{
    if (verbose) puts("STUB: PJCTicketGetSrcPaperSize called");
    return NULL;
}

void* PJCTicketGetSrcResolution(void)
{
    if (verbose) puts("STUB: PJCTicketGetSrcResolution called");
    return NULL;
}

void* PJCTicketGetSrcScaling(void)
{
    if (verbose) puts("STUB: PJCTicketGetSrcScaling called");
    return NULL;
}

void* PMBackendExecIO(void)
{
    if (verbose) puts("STUB: PMBackendExecIO called");
    return NULL;
}

void* PMCFEqual(void)
{
    if (verbose) puts("STUB: PMCFEqual called");
    return NULL;
}

void* PMCGImageCreateWithEPSDataProvider(void)
{
    if (verbose) puts("STUB: PMCGImageCreateWithEPSDataProvider called");
    return NULL;
}

void* PMConvertFile(void)
{
    if (verbose) puts("STUB: PMConvertFile called");
    return NULL;
}

void* PMCopyAvailablePPDs(void)
{
    if (verbose) puts("STUB: PMCopyAvailablePPDs called");
    return NULL;
}

void* PMCopyCountry(void)
{
    if (verbose) puts("STUB: PMCopyCountry called");
    return NULL;
}

void* PMCopyLocalizedPPD(void)
{
    if (verbose) puts("STUB: PMCopyLocalizedPPD called");
    return NULL;
}

void* PMCopyPPDData(void)
{
    if (verbose) puts("STUB: PMCopyPPDData called");
    return NULL;
}

void* PMCopyPageFormat(void)
{
    if (verbose) puts("STUB: PMCopyPageFormat called");
    return NULL;
}

void* PMCopyPrintSettings(void)
{
    if (verbose) puts("STUB: PMCopyPrintSettings called");
    return NULL;
}

void* PMCopyUserDefaultPaperSize(void)
{
    if (verbose) puts("STUB: PMCopyUserDefaultPaperSize called");
    return NULL;
}

void* PMCreateCFArrayForPMObjects(void)
{
    if (verbose) puts("STUB: PMCreateCFArrayForPMObjects called");
    return NULL;
}

void* PMCreateGenericPrinter(void)
{
    if (verbose) puts("STUB: PMCreateGenericPrinter called");
    return NULL;
}

void* PMCreatePageFormat(void)
{
    if (verbose) puts("STUB: PMCreatePageFormat called");
    return NULL;
}

void* PMCreatePageFormatWithPMPaper(void)
{
    if (verbose) puts("STUB: PMCreatePageFormatWithPMPaper called");
    return NULL;
}

void* PMCreatePaperInfoTicketFromPPDPaperName(void)
{
    if (verbose) puts("STUB: PMCreatePaperInfoTicketFromPPDPaperName called");
    return NULL;
}

void* PMCreatePrintSettings(void)
{
    if (verbose) puts("STUB: PMCreatePrintSettings called");
    return NULL;
}

void* PMCreatePrinter(void)
{
    if (verbose) puts("STUB: PMCreatePrinter called");
    return NULL;
}

void* PMCreateProxy(void)
{
    if (verbose) puts("STUB: PMCreateProxy called");
    return NULL;
}

void* PMCreateSession(void)
{
    if (verbose) puts("STUB: PMCreateSession called");
    return NULL;
}

void* PMDebugSet_Boolean(void)
{
    if (verbose) puts("STUB: PMDebugSet_Boolean called");
    return NULL;
}

void* PMDebugSet_CFTypeRef(void)
{
    if (verbose) puts("STUB: PMDebugSet_CFTypeRef called");
    return NULL;
}

void* PMDebugSet_PMObject(void)
{
    if (verbose) puts("STUB: PMDebugSet_PMObject called");
    return NULL;
}

void* PMDebugSet_PMRect_p(void)
{
    if (verbose) puts("STUB: PMDebugSet_PMRect_p called");
    return NULL;
}

void* PMDebugSet_char_p(void)
{
    if (verbose) puts("STUB: PMDebugSet_char_p called");
    return NULL;
}

void* PMDebugSet_double(void)
{
    if (verbose) puts("STUB: PMDebugSet_double called");
    return NULL;
}

void* PMDebugSet_long(void)
{
    if (verbose) puts("STUB: PMDebugSet_long called");
    return NULL;
}

void* PMDebugSet_ulong(void)
{
    if (verbose) puts("STUB: PMDebugSet_ulong called");
    return NULL;
}

void* PMExecIO(void)
{
    if (verbose) puts("STUB: PMExecIO called");
    return NULL;
}

void* PMFindProxy(void)
{
    if (verbose) puts("STUB: PMFindProxy called");
    return NULL;
}

void* PMFlattenPageFormatToCFData(void)
{
    if (verbose) puts("STUB: PMFlattenPageFormatToCFData called");
    return NULL;
}

void* PMFlattenPageFormatToURL(void)
{
    if (verbose) puts("STUB: PMFlattenPageFormatToURL called");
    return NULL;
}

void* PMFlattenPrintSettingsToCFData(void)
{
    if (verbose) puts("STUB: PMFlattenPrintSettingsToCFData called");
    return NULL;
}

void* PMFlattenPrintSettingsToURL(void)
{
    if (verbose) puts("STUB: PMFlattenPrintSettingsToURL called");
    return NULL;
}

void* PMGetAdjustedPageRect(void)
{
    if (verbose) puts("STUB: PMGetAdjustedPageRect called");
    return NULL;
}

void* PMGetAdjustedPaperRect(void)
{
    if (verbose) puts("STUB: PMGetAdjustedPaperRect called");
    return NULL;
}

void* PMGetCollate(void)
{
    if (verbose) puts("STUB: PMGetCollate called");
    return NULL;
}

void* PMGetCopies(void)
{
    if (verbose) puts("STUB: PMGetCopies called");
    return NULL;
}

void* PMGetDuplex(void)
{
    if (verbose) puts("STUB: PMGetDuplex called");
    return NULL;
}

void* PMGetFirstPage(void)
{
    if (verbose) puts("STUB: PMGetFirstPage called");
    return NULL;
}

void* PMGetLastPage(void)
{
    if (verbose) puts("STUB: PMGetLastPage called");
    return NULL;
}

void* PMGetOrientation(void)
{
    if (verbose) puts("STUB: PMGetOrientation called");
    return NULL;
}

void* PMGetPageFormatExtendedData(void)
{
    if (verbose) puts("STUB: PMGetPageFormatExtendedData called");
    return NULL;
}

void* PMGetPageFormatPaper(void)
{
    if (verbose) puts("STUB: PMGetPageFormatPaper called");
    return NULL;
}

void* PMGetPageRange(void)
{
    if (verbose) puts("STUB: PMGetPageRange called");
    return NULL;
}

void* PMGetPrintSettingsExtendedData(void)
{
    if (verbose) puts("STUB: PMGetPrintSettingsExtendedData called");
    return NULL;
}

void* PMGetPrinterModel(void)
{
    if (verbose) puts("STUB: PMGetPrinterModel called");
    return NULL;
}

void* PMGetReverseOrder(void)
{
    if (verbose) puts("STUB: PMGetReverseOrder called");
    return NULL;
}

void* PMGetScale(void)
{
    if (verbose) puts("STUB: PMGetScale called");
    return NULL;
}

void* PMGetUnadjustedPageRect(void)
{
    if (verbose) puts("STUB: PMGetUnadjustedPageRect called");
    return NULL;
}

void* PMGetUnadjustedPaperRect(void)
{
    if (verbose) puts("STUB: PMGetUnadjustedPaperRect called");
    return NULL;
}

void* PMInlineWorkflowCopyItems(void)
{
    if (verbose) puts("STUB: PMInlineWorkflowCopyItems called");
    return NULL;
}

void* PMInstallCoercionHandlers(void)
{
    if (verbose) puts("STUB: PMInstallCoercionHandlers called");
    return NULL;
}

void* PMLaunchProxy(void)
{
    if (verbose) puts("STUB: PMLaunchProxy called");
    return NULL;
}

void* PMMonitorOnlineOfflineState(void)
{
    if (verbose) puts("STUB: PMMonitorOnlineOfflineState called");
    return NULL;
}

void* PMPDEBundleNeedsDisabledSandbox(void)
{
    if (verbose) puts("STUB: PMPDEBundleNeedsDisabledSandbox called");
    return NULL;
}

void* PMPageFormatComputeAdjustedRects(void)
{
    if (verbose) puts("STUB: PMPageFormatComputeAdjustedRects called");
    return NULL;
}

void* PMPageFormatComputeAdjustedRectsWithPrinter(void)
{
    if (verbose) puts("STUB: PMPageFormatComputeAdjustedRectsWithPrinter called");
    return NULL;
}

void* PMPageFormatCreateDataRepresentation(void)
{
    if (verbose) puts("STUB: PMPageFormatCreateDataRepresentation called");
    return NULL;
}

void* PMPageFormatCreateWithDataRepresentation(void)
{
    if (verbose) puts("STUB: PMPageFormatCreateWithDataRepresentation called");
    return NULL;
}

void* PMPageFormatGetPrinterID(void)
{
    if (verbose) puts("STUB: PMPageFormatGetPrinterID called");
    return NULL;
}

void* PMPaperAddToUserPrefs(void)
{
    if (verbose) puts("STUB: PMPaperAddToUserPrefs called");
    return NULL;
}

void* PMPaperCopyUserPrefs(void)
{
    if (verbose) puts("STUB: PMPaperCopyUserPrefs called");
    return NULL;
}

void* PMPaperCreateCustom(void)
{
    if (verbose) puts("STUB: PMPaperCreateCustom called");
    return NULL;
}

void* PMPaperCreateFromPaperInfo(void)
{
    if (verbose) puts("STUB: PMPaperCreateFromPaperInfo called");
    return NULL;
}

void* PMPaperCreateInternal(void)
{
    if (verbose) puts("STUB: PMPaperCreateInternal called");
    return NULL;
}

void* PMPaperCreateLocalizedName(void)
{
    if (verbose) puts("STUB: PMPaperCreateLocalizedName called");
    return NULL;
}

void* PMPaperCreatePMTicketRef(void)
{
    if (verbose) puts("STUB: PMPaperCreatePMTicketRef called");
    return NULL;
}

void* PMPaperGetHeight(void)
{
    if (verbose) puts("STUB: PMPaperGetHeight called");
    return NULL;
}

void* PMPaperGetID(void)
{
    if (verbose) puts("STUB: PMPaperGetID called");
    return NULL;
}

void* PMPaperGetMargins(void)
{
    if (verbose) puts("STUB: PMPaperGetMargins called");
    return NULL;
}

void* PMPaperGetName(void)
{
    if (verbose) puts("STUB: PMPaperGetName called");
    return NULL;
}

void* PMPaperGetPPDPaperName(void)
{
    if (verbose) puts("STUB: PMPaperGetPPDPaperName called");
    return NULL;
}

void* PMPaperGetPrinterID(void)
{
    if (verbose) puts("STUB: PMPaperGetPrinterID called");
    return NULL;
}

void* PMPaperGetWidth(void)
{
    if (verbose) puts("STUB: PMPaperGetWidth called");
    return NULL;
}

void* PMPaperIsCustom(void)
{
    if (verbose) puts("STUB: PMPaperIsCustom called");
    return NULL;
}

void* PMPaperRemoveFromUserPrefs(void)
{
    if (verbose) puts("STUB: PMPaperRemoveFromUserPrefs called");
    return NULL;
}

void* PMPresetCopyName(void)
{
    if (verbose) puts("STUB: PMPresetCopyName called");
    return NULL;
}

void* PMPresetCreatePageFormat(void)
{
    if (verbose) puts("STUB: PMPresetCreatePageFormat called");
    return NULL;
}

void* PMPresetCreatePrintSettings(void)
{
    if (verbose) puts("STUB: PMPresetCreatePrintSettings called");
    return NULL;
}

void* PMPresetGetAttributes(void)
{
    if (verbose) puts("STUB: PMPresetGetAttributes called");
    return NULL;
}

void* PMPrintSettingsCopyAsDictionary(void)
{
    if (verbose) puts("STUB: PMPrintSettingsCopyAsDictionary called");
    return NULL;
}

void* PMPrintSettingsCopyKeys(void)
{
    if (verbose) puts("STUB: PMPrintSettingsCopyKeys called");
    return NULL;
}

void* PMPrintSettingsCreateDataRepresentation(void)
{
    if (verbose) puts("STUB: PMPrintSettingsCreateDataRepresentation called");
    return NULL;
}

void* PMPrintSettingsCreateWithDataRepresentation(void)
{
    if (verbose) puts("STUB: PMPrintSettingsCreateWithDataRepresentation called");
    return NULL;
}

void* PMPrintSettingsGetJobName(void)
{
    if (verbose) puts("STUB: PMPrintSettingsGetJobName called");
    return NULL;
}

void* PMPrintSettingsGetValue(void)
{
    if (verbose) puts("STUB: PMPrintSettingsGetValue called");
    return NULL;
}

void* PMPrintSettingsSetJobName(void)
{
    if (verbose) puts("STUB: PMPrintSettingsSetJobName called");
    return NULL;
}

void* PMPrintSettingsSetValue(void)
{
    if (verbose) puts("STUB: PMPrintSettingsSetValue called");
    return NULL;
}

void* PMPrintSettingsToOptions(void)
{
    if (verbose) puts("STUB: PMPrintSettingsToOptions called");
    return NULL;
}

void* PMPrintSettingsToOptionsWithPrinterAndPageFormat(void)
{
    if (verbose) puts("STUB: PMPrintSettingsToOptionsWithPrinterAndPageFormat called");
    return NULL;
}

void* PMPrinterCanDuplex(void)
{
    if (verbose) puts("STUB: PMPrinterCanDuplex called");
    return NULL;
}

void* PMPrinterCanIdentify(void)
{
    if (verbose) puts("STUB: PMPrinterCanIdentify called");
    return NULL;
}

void* PMPrinterCanScan(void)
{
    if (verbose) puts("STUB: PMPrinterCanScan called");
    return NULL;
}

void* PMPrinterCancelConnect(void)
{
    if (verbose) puts("STUB: PMPrinterCancelConnect called");
    return NULL;
}

void* PMPrinterChargeInfoURI(void)
{
    if (verbose) puts("STUB: PMPrinterChargeInfoURI called");
    return NULL;
}

void* PMPrinterCopyACL(void)
{
    if (verbose) puts("STUB: PMPrinterCopyACL called");
    return NULL;
}

void* PMPrinterCopyDefaultUserCredentials(void)
{
    if (verbose) puts("STUB: PMPrinterCopyDefaultUserCredentials called");
    return NULL;
}

void* PMPrinterCopyDescriptionURL(void)
{
    if (verbose) puts("STUB: PMPrinterCopyDescriptionURL called");
    return NULL;
}

void* PMPrinterCopyDeviceURI(void)
{
    if (verbose) puts("STUB: PMPrinterCopyDeviceURI called");
    return NULL;
}

void* PMPrinterCopyFinalDeviceURI(void)
{
    if (verbose) puts("STUB: PMPrinterCopyFinalDeviceURI called");
    return NULL;
}

void* PMPrinterCopyHostName(void)
{
    if (verbose) puts("STUB: PMPrinterCopyHostName called");
    return NULL;
}

void* PMPrinterCopyMembers(void)
{
    if (verbose) puts("STUB: PMPrinterCopyMembers called");
    return NULL;
}

void* PMPrinterCopyPresets(void)
{
    if (verbose) puts("STUB: PMPrinterCopyPresets called");
    return NULL;
}

void* PMPrinterCopyRequiredUserCredentials(void)
{
    if (verbose) puts("STUB: PMPrinterCopyRequiredUserCredentials called");
    return NULL;
}

void* PMPrinterCopyScannerUUID(void)
{
    if (verbose) puts("STUB: PMPrinterCopyScannerUUID called");
    return NULL;
}

void* PMPrinterCopyScanningApp(void)
{
    if (verbose) puts("STUB: PMPrinterCopyScanningApp called");
    return NULL;
}

void* PMPrinterCopyScanningAppPath(void)
{
    if (verbose) puts("STUB: PMPrinterCopyScanningAppPath called");
    return NULL;
}

void* PMPrinterCopyState(void)
{
    if (verbose) puts("STUB: PMPrinterCopyState called");
    return NULL;
}

void* PMPrinterCopySuppliesURI(void)
{
    if (verbose) puts("STUB: PMPrinterCopySuppliesURI called");
    return NULL;
}

void* PMPrinterCopyTempPPDURL(void)
{
    if (verbose) puts("STUB: PMPrinterCopyTempPPDURL called");
    return NULL;
}

void* PMPrinterCreateFromPrinterID(void)
{
    if (verbose) puts("STUB: PMPrinterCreateFromPrinterID called");
    return NULL;
}

void* PMPrinterCreatePaperInfoListForMenu(void)
{
    if (verbose) puts("STUB: PMPrinterCreatePaperInfoListForMenu called");
    return NULL;
}

void* PMPrinterCreatePaperInfoListForMenuPriv(void)
{
    if (verbose) puts("STUB: PMPrinterCreatePaperInfoListForMenuPriv called");
    return NULL;
}

void* PMPrinterGetCMDeviceID(void)
{
    if (verbose) puts("STUB: PMPrinterGetCMDeviceID called");
    return NULL;
}

void* PMPrinterGetCommInfo(void)
{
    if (verbose) puts("STUB: PMPrinterGetCommInfo called");
    return NULL;
}

void* PMPrinterGetConsumables(void)
{
    if (verbose) puts("STUB: PMPrinterGetConsumables called");
    return NULL;
}

void* PMPrinterGetConsumablesDisclaimer(void)
{
    if (verbose) puts("STUB: PMPrinterGetConsumablesDisclaimer called");
    return NULL;
}

void* PMPrinterGetConsumablesOptionalText(void)
{
    if (verbose) puts("STUB: PMPrinterGetConsumablesOptionalText called");
    return NULL;
}

void* PMPrinterGetCustomPaperSizeHardwareMargins(void)
{
    if (verbose) puts("STUB: PMPrinterGetCustomPaperSizeHardwareMargins called");
    return NULL;
}

void* PMPrinterGetCustomPaperSizeHardwareSheetSize(void)
{
    if (verbose) puts("STUB: PMPrinterGetCustomPaperSizeHardwareSheetSize called");
    return NULL;
}

void* PMPrinterGetDriverCreator(void)
{
    if (verbose) puts("STUB: PMPrinterGetDriverCreator called");
    return NULL;
}

void* PMPrinterGetDriverReleaseInfo(void)
{
    if (verbose) puts("STUB: PMPrinterGetDriverReleaseInfo called");
    return NULL;
}

void* PMPrinterGetID(void)
{
    if (verbose) puts("STUB: PMPrinterGetID called");
    return NULL;
}

void* PMPrinterGetIconData(void)
{
    if (verbose) puts("STUB: PMPrinterGetIconData called");
    return NULL;
}

void* PMPrinterGetImage(void)
{
    if (verbose) puts("STUB: PMPrinterGetImage called");
    return NULL;
}

void* PMPrinterGetIndexedPrinterResolution(void)
{
    if (verbose) puts("STUB: PMPrinterGetIndexedPrinterResolution called");
    return NULL;
}

void* PMPrinterGetLanguageInfo(void)
{
    if (verbose) puts("STUB: PMPrinterGetLanguageInfo called");
    return NULL;
}

void* PMPrinterGetLocation(void)
{
    if (verbose) puts("STUB: PMPrinterGetLocation called");
    return NULL;
}

void* PMPrinterGetMakeAndModelName(void)
{
    if (verbose) puts("STUB: PMPrinterGetMakeAndModelName called");
    return NULL;
}

void* PMPrinterGetMarkerChangeTime(void)
{
    if (verbose) puts("STUB: PMPrinterGetMarkerChangeTime called");
    return NULL;
}

void* PMPrinterGetMimeTypes(void)
{
    if (verbose) puts("STUB: PMPrinterGetMimeTypes called");
    return NULL;
}

void* PMPrinterGetName(void)
{
    if (verbose) puts("STUB: PMPrinterGetName called");
    return NULL;
}

void* PMPrinterGetOutputResolution(void)
{
    if (verbose) puts("STUB: PMPrinterGetOutputResolution called");
    return NULL;
}

void* PMPrinterGetPaperList(void)
{
    if (verbose) puts("STUB: PMPrinterGetPaperList called");
    return NULL;
}

void* PMPrinterGetPrinterResolutionCount(void)
{
    if (verbose) puts("STUB: PMPrinterGetPrinterResolutionCount called");
    return NULL;
}

void* PMPrinterGetPrinterValue(void)
{
    if (verbose) puts("STUB: PMPrinterGetPrinterValue called");
    return NULL;
}

void* PMPrinterGetState(void)
{
    if (verbose) puts("STUB: PMPrinterGetState called");
    return NULL;
}

void* PMPrinterHasCustomPaperSizes(void)
{
    if (verbose) puts("STUB: PMPrinterHasCustomPaperSizes called");
    return NULL;
}

void* PMPrinterIdentify(void)
{
    if (verbose) puts("STUB: PMPrinterIdentify called");
    return NULL;
}

void* PMPrinterIsAirPrint(void)
{
    if (verbose) puts("STUB: PMPrinterIsAirPrint called");
    return NULL;
}

void* PMPrinterIsAllowed(void)
{
    if (verbose) puts("STUB: PMPrinterIsAllowed called");
    return NULL;
}

void* PMPrinterIsClass(void)
{
    if (verbose) puts("STUB: PMPrinterIsClass called");
    return NULL;
}

void* PMPrinterIsDefault(void)
{
    if (verbose) puts("STUB: PMPrinterIsDefault called");
    return NULL;
}

void* PMPrinterIsDirectConnect(void)
{
    if (verbose) puts("STUB: PMPrinterIsDirectConnect called");
    return NULL;
}

void* PMPrinterIsFavorite(void)
{
    if (verbose) puts("STUB: PMPrinterIsFavorite called");
    return NULL;
}

void* PMPrinterIsFax(void)
{
    if (verbose) puts("STUB: PMPrinterIsFax called");
    return NULL;
}

void* PMPrinterIsGenericPrinter(void)
{
    if (verbose) puts("STUB: PMPrinterIsGenericPrinter called");
    return NULL;
}

void* PMPrinterIsLocked(void)
{
    if (verbose) puts("STUB: PMPrinterIsLocked called");
    return NULL;
}

void* PMPrinterIsPostScriptCapable(void)
{
    if (verbose) puts("STUB: PMPrinterIsPostScriptCapable called");
    return NULL;
}

void* PMPrinterIsPostScriptPrinter(void)
{
    if (verbose) puts("STUB: PMPrinterIsPostScriptPrinter called");
    return NULL;
}

void* PMPrinterIsRemote(void)
{
    if (verbose) puts("STUB: PMPrinterIsRemote called");
    return NULL;
}

void* PMPrinterIsScanner(void)
{
    if (verbose) puts("STUB: PMPrinterIsScanner called");
    return NULL;
}

void* PMPrinterIsShared(void)
{
    if (verbose) puts("STUB: PMPrinterIsShared called");
    return NULL;
}

void* PMPrinterOpenSuppliesURI(void)
{
    if (verbose) puts("STUB: PMPrinterOpenSuppliesURI called");
    return NULL;
}

void* PMPrinterPrintWithFile(void)
{
    if (verbose) puts("STUB: PMPrinterPrintWithFile called");
    return NULL;
}

void* PMPrinterPrintWithProvider(void)
{
    if (verbose) puts("STUB: PMPrinterPrintWithProvider called");
    return NULL;
}

void* PMPrinterRequiresAccountingInformation(void)
{
    if (verbose) puts("STUB: PMPrinterRequiresAccountingInformation called");
    return NULL;
}

void* PMPrinterRequiresUserCredentials(void)
{
    if (verbose) puts("STUB: PMPrinterRequiresUserCredentials called");
    return NULL;
}

void* PMPrinterSendCommand(void)
{
    if (verbose) puts("STUB: PMPrinterSendCommand called");
    return NULL;
}

void* PMPrinterSetACL(void)
{
    if (verbose) puts("STUB: PMPrinterSetACL called");
    return NULL;
}

void* PMPrinterSetConsumables(void)
{
    if (verbose) puts("STUB: PMPrinterSetConsumables called");
    return NULL;
}

void* PMPrinterSetConsumablesDisclaimer(void)
{
    if (verbose) puts("STUB: PMPrinterSetConsumablesDisclaimer called");
    return NULL;
}

void* PMPrinterSetConsumablesOptionalText(void)
{
    if (verbose) puts("STUB: PMPrinterSetConsumablesOptionalText called");
    return NULL;
}

void* PMPrinterSetDefault(void)
{
    if (verbose) puts("STUB: PMPrinterSetDefault called");
    return NULL;
}

void* PMPrinterSetFavorite(void)
{
    if (verbose) puts("STUB: PMPrinterSetFavorite called");
    return NULL;
}

void* PMPrinterSetLocation(void)
{
    if (verbose) puts("STUB: PMPrinterSetLocation called");
    return NULL;
}

void* PMPrinterSetMarkerChangeTime(void)
{
    if (verbose) puts("STUB: PMPrinterSetMarkerChangeTime called");
    return NULL;
}

void* PMPrinterSetModel(void)
{
    if (verbose) puts("STUB: PMPrinterSetModel called");
    return NULL;
}

void* PMPrinterSetName(void)
{
    if (verbose) puts("STUB: PMPrinterSetName called");
    return NULL;
}

void* PMPrinterSetOutputResolution(void)
{
    if (verbose) puts("STUB: PMPrinterSetOutputResolution called");
    return NULL;
}

void* PMPrinterSetShared(void)
{
    if (verbose) puts("STUB: PMPrinterSetShared called");
    return NULL;
}

void* PMPrinterStartConnect(void)
{
    if (verbose) puts("STUB: PMPrinterStartConnect called");
    return NULL;
}

void* PMPrinterStartMonitoring(void)
{
    if (verbose) puts("STUB: PMPrinterStartMonitoring called");
    return NULL;
}

void* PMPrinterStopMonitoring(void)
{
    if (verbose) puts("STUB: PMPrinterStopMonitoring called");
    return NULL;
}

void* PMPrinterSupportsMatchingMode(void)
{
    if (verbose) puts("STUB: PMPrinterSupportsMatchingMode called");
    return NULL;
}

void* PMPrinterUsesTCP(void)
{
    if (verbose) puts("STUB: PMPrinterUsesTCP called");
    return NULL;
}

void* PMPrinterWritePostScriptToURL(void)
{
    if (verbose) puts("STUB: PMPrinterWritePostScriptToURL called");
    return NULL;
}

void* PMRelease(void)
{
    if (verbose) puts("STUB: PMRelease called");
    return NULL;
}

void* PMRetain(void)
{
    if (verbose) puts("STUB: PMRetain called");
    return NULL;
}

void* PMServerCopyJobLog(void)
{
    if (verbose) puts("STUB: PMServerCopyJobLog called");
    return NULL;
}

void* PMServerCreateDeviceList(void)
{
    if (verbose) puts("STUB: PMServerCreateDeviceList called");
    return NULL;
}

void* PMServerCreatePrinterList(void)
{
    if (verbose) puts("STUB: PMServerCreatePrinterList called");
    return NULL;
}

void* PMServerFavoritesListHasChanged(void)
{
    if (verbose) puts("STUB: PMServerFavoritesListHasChanged called");
    return NULL;
}

void* PMServerLaunchPrinterBrowser(void)
{
    if (verbose) puts("STUB: PMServerLaunchPrinterBrowser called");
    return NULL;
}

void* PMServerPrinterListHasChanged(void)
{
    if (verbose) puts("STUB: PMServerPrinterListHasChanged called");
    return NULL;
}

void* PMSessionBeginCGDocumentNoDialog(void)
{
    if (verbose) puts("STUB: PMSessionBeginCGDocumentNoDialog called");
    return NULL;
}

void* PMSessionBeginDocumentNoDialogPrivate(void)
{
    if (verbose) puts("STUB: PMSessionBeginDocumentNoDialogPrivate called");
    return NULL;
}

void* PMSessionBeginPageNoDialog(void)
{
    if (verbose) puts("STUB: PMSessionBeginPageNoDialog called");
    return NULL;
}

void* PMSessionCopyApplicationOutputIntent(void)
{
    if (verbose) puts("STUB: PMSessionCopyApplicationOutputIntent called");
    return NULL;
}

void* PMSessionCopyApplicationOutputIntentWithColorSyncProfiles(void)
{
    if (verbose) puts("STUB: PMSessionCopyApplicationOutputIntentWithColorSyncProfiles called");
    return NULL;
}

void* PMSessionCopyCurrentPrinterColorSyncProfile(void)
{
    if (verbose) puts("STUB: PMSessionCopyCurrentPrinterColorSyncProfile called");
    return NULL;
}

void* PMSessionCopyCurrentPrinterProfile(void)
{
    if (verbose) puts("STUB: PMSessionCopyCurrentPrinterProfile called");
    return NULL;
}

void* PMSessionCopyDefaultOutputIntent(void)
{
    if (verbose) puts("STUB: PMSessionCopyDefaultOutputIntent called");
    return NULL;
}

void* PMSessionCopyDefaultOutputIntentWithColorSyncProfiles(void)
{
    if (verbose) puts("STUB: PMSessionCopyDefaultOutputIntentWithColorSyncProfiles called");
    return NULL;
}

void* PMSessionCopyDestinationFormat(void)
{
    if (verbose) puts("STUB: PMSessionCopyDestinationFormat called");
    return NULL;
}

void* PMSessionCopyDestinationLocation(void)
{
    if (verbose) puts("STUB: PMSessionCopyDestinationLocation called");
    return NULL;
}

void* PMSessionCopyOutputFormatList(void)
{
    if (verbose) puts("STUB: PMSessionCopyOutputFormatList called");
    return NULL;
}

void* PMSessionCreateCurrentPrinterColorSpace(void)
{
    if (verbose) puts("STUB: PMSessionCreateCurrentPrinterColorSpace called");
    return NULL;
}

void* PMSessionCreatePageFormatList(void)
{
    if (verbose) puts("STUB: PMSessionCreatePageFormatList called");
    return NULL;
}

void* PMSessionCreatePrinterList(void)
{
    if (verbose) puts("STUB: PMSessionCreatePrinterList called");
    return NULL;
}

void* PMSessionDefaultPageFormat(void)
{
    if (verbose) puts("STUB: PMSessionDefaultPageFormat called");
    return NULL;
}

void* PMSessionDefaultPrintSettings(void)
{
    if (verbose) puts("STUB: PMSessionDefaultPrintSettings called");
    return NULL;
}

void* PMSessionDisablePresetsPopUp(void)
{
    if (verbose) puts("STUB: PMSessionDisablePresetsPopUp called");
    return NULL;
}

void* PMSessionDisablePrinterPresetsPrivate(void)
{
    if (verbose) puts("STUB: PMSessionDisablePrinterPresetsPrivate called");
    return NULL;
}

void* PMSessionEnablePresetsPopUp(void)
{
    if (verbose) puts("STUB: PMSessionEnablePresetsPopUp called");
    return NULL;
}

void* PMSessionEnablePrinterPresetsPrivate(void)
{
    if (verbose) puts("STUB: PMSessionEnablePrinterPresetsPrivate called");
    return NULL;
}

void* PMSessionEndDocumentNoDialog(void)
{
    if (verbose) puts("STUB: PMSessionEndDocumentNoDialog called");
    return NULL;
}

void* PMSessionEndPageNoDialog(void)
{
    if (verbose) puts("STUB: PMSessionEndPageNoDialog called");
    return NULL;
}

void* PMSessionError(void)
{
    if (verbose) puts("STUB: PMSessionError called");
    return NULL;
}

void* PMSessionGetCGGraphicsContext(void)
{
    if (verbose) puts("STUB: PMSessionGetCGGraphicsContext called");
    return NULL;
}

void* PMSessionGetColorMatchingMode(void)
{
    if (verbose) puts("STUB: PMSessionGetColorMatchingMode called");
    return NULL;
}

void* PMSessionGetColorMatchingMode2(void)
{
    if (verbose) puts("STUB: PMSessionGetColorMatchingMode2 called");
    return NULL;
}

void* PMSessionGetColorMatchingModeLock(void)
{
    if (verbose) puts("STUB: PMSessionGetColorMatchingModeLock called");
    return NULL;
}

void* PMSessionGetCurrentPrinter(void)
{
    if (verbose) puts("STUB: PMSessionGetCurrentPrinter called");
    return NULL;
}

void* PMSessionGetDataFromSession(void)
{
    if (verbose) puts("STUB: PMSessionGetDataFromSession called");
    return NULL;
}

void* PMSessionGetDestinationType(void)
{
    if (verbose) puts("STUB: PMSessionGetDestinationType called");
    return NULL;
}

void* PMSessionGetDocumentTags(void)
{
    if (verbose) puts("STUB: PMSessionGetDocumentTags called");
    return NULL;
}

void* PMSessionGetGenericPrinter(void)
{
    if (verbose) puts("STUB: PMSessionGetGenericPrinter called");
    return NULL;
}

void* PMSessionGetHideFileExtension(void)
{
    if (verbose) puts("STUB: PMSessionGetHideFileExtension called");
    return NULL;
}

void* PMSessionSetApplicationOutputIntent(void)
{
    if (verbose) puts("STUB: PMSessionSetApplicationOutputIntent called");
    return NULL;
}

void* PMSessionSetApplicationOutputIntentWithColorSyncProfiles(void)
{
    if (verbose) puts("STUB: PMSessionSetApplicationOutputIntentWithColorSyncProfiles called");
    return NULL;
}

void* PMSessionSetColorMatchingMode(void)
{
    if (verbose) puts("STUB: PMSessionSetColorMatchingMode called");
    return NULL;
}

void* PMSessionSetColorMatchingModeLock(void)
{
    if (verbose) puts("STUB: PMSessionSetColorMatchingModeLock called");
    return NULL;
}

void* PMSessionSetColorMatchingModeNoLock(void)
{
    if (verbose) puts("STUB: PMSessionSetColorMatchingModeNoLock called");
    return NULL;
}

void* PMSessionSetCurrentPMPrinter(void)
{
    if (verbose) puts("STUB: PMSessionSetCurrentPMPrinter called");
    return NULL;
}

void* PMSessionSetDataInSession(void)
{
    if (verbose) puts("STUB: PMSessionSetDataInSession called");
    return NULL;
}

void* PMSessionSetDefaultButtonTitleCFString(void)
{
    if (verbose) puts("STUB: PMSessionSetDefaultButtonTitleCFString called");
    return NULL;
}

void* PMSessionSetDestination(void)
{
    if (verbose) puts("STUB: PMSessionSetDestination called");
    return NULL;
}

void* PMSessionSetDocumentTags(void)
{
    if (verbose) puts("STUB: PMSessionSetDocumentTags called");
    return NULL;
}

void* PMSessionSetError(void)
{
    if (verbose) puts("STUB: PMSessionSetError called");
    return NULL;
}

void* PMSessionSetHideFileExtension(void)
{
    if (verbose) puts("STUB: PMSessionSetHideFileExtension called");
    return NULL;
}

void* PMSessionSetPrintingDialogTitleCFString(void)
{
    if (verbose) puts("STUB: PMSessionSetPrintingDialogTitleCFString called");
    return NULL;
}

void* PMSessionValidatePageFormat(void)
{
    if (verbose) puts("STUB: PMSessionValidatePageFormat called");
    return NULL;
}

void* PMSessionValidatePrintSettings(void)
{
    if (verbose) puts("STUB: PMSessionValidatePrintSettings called");
    return NULL;
}

void* PMSetCollate(void)
{
    if (verbose) puts("STUB: PMSetCollate called");
    return NULL;
}

void* PMSetCopies(void)
{
    if (verbose) puts("STUB: PMSetCopies called");
    return NULL;
}

void* PMSetCopyCollate(void)
{
    if (verbose) puts("STUB: PMSetCopyCollate called");
    return NULL;
}

void* PMSetDuplex(void)
{
    if (verbose) puts("STUB: PMSetDuplex called");
    return NULL;
}

void* PMSetFirstPage(void)
{
    if (verbose) puts("STUB: PMSetFirstPage called");
    return NULL;
}

void* PMSetLastPage(void)
{
    if (verbose) puts("STUB: PMSetLastPage called");
    return NULL;
}

void* PMSetOrientation(void)
{
    if (verbose) puts("STUB: PMSetOrientation called");
    return NULL;
}

void* PMSetPageFormatExtendedData(void)
{
    if (verbose) puts("STUB: PMSetPageFormatExtendedData called");
    return NULL;
}

void* PMSetPageRange(void)
{
    if (verbose) puts("STUB: PMSetPageRange called");
    return NULL;
}

void* PMSetPrintSettingsExtendedData(void)
{
    if (verbose) puts("STUB: PMSetPrintSettingsExtendedData called");
    return NULL;
}

void* PMSetReverseOrder(void)
{
    if (verbose) puts("STUB: PMSetReverseOrder called");
    return NULL;
}

void* PMSetScale(void)
{
    if (verbose) puts("STUB: PMSetScale called");
    return NULL;
}

void* PMSetUserDefaultPaperSize(void)
{
    if (verbose) puts("STUB: PMSetUserDefaultPaperSize called");
    return NULL;
}

void* PMShouldEnableSupplies(void)
{
    if (verbose) puts("STUB: PMShouldEnableSupplies called");
    return NULL;
}

void* PMTicketConfirmTicket(void)
{
    if (verbose) puts("STUB: PMTicketConfirmTicket called");
    return NULL;
}

void* PMTicketContainsItem(void)
{
    if (verbose) puts("STUB: PMTicketContainsItem called");
    return NULL;
}

void* PMTicketContainsTicket(void)
{
    if (verbose) puts("STUB: PMTicketContainsTicket called");
    return NULL;
}

void* PMTicketCopy(void)
{
    if (verbose) puts("STUB: PMTicketCopy called");
    return NULL;
}

void* PMTicketCopyItem(void)
{
    if (verbose) puts("STUB: PMTicketCopyItem called");
    return NULL;
}

void* PMTicketCopyKeys(void)
{
    if (verbose) puts("STUB: PMTicketCopyKeys called");
    return NULL;
}

void* PMTicketCreate(void)
{
    if (verbose) puts("STUB: PMTicketCreate called");
    return NULL;
}

void* PMTicketCreateDict(void)
{
    if (verbose) puts("STUB: PMTicketCreateDict called");
    return NULL;
}

void* PMTicketCreateMutableDict(void)
{
    if (verbose) puts("STUB: PMTicketCreateMutableDict called");
    return NULL;
}

void* PMTicketCreateTemplate(void)
{
    if (verbose) puts("STUB: PMTicketCreateTemplate called");
    return NULL;
}

void* PMTicketDeleteItem(void)
{
    if (verbose) puts("STUB: PMTicketDeleteItem called");
    return NULL;
}

void* PMTicketFillFromArray(void)
{
    if (verbose) puts("STUB: PMTicketFillFromArray called");
    return NULL;
}

void* PMTicketGetAPIVersion(void)
{
    if (verbose) puts("STUB: PMTicketGetAPIVersion called");
    return NULL;
}

void* PMTicketGetAllocator(void)
{
    if (verbose) puts("STUB: PMTicketGetAllocator called");
    return NULL;
}

void* PMTicketGetBoolean(void)
{
    if (verbose) puts("STUB: PMTicketGetBoolean called");
    return NULL;
}

void* PMTicketGetBytes(void)
{
    if (verbose) puts("STUB: PMTicketGetBytes called");
    return NULL;
}

void* PMTicketGetCFArray(void)
{
    if (verbose) puts("STUB: PMTicketGetCFArray called");
    return NULL;
}

void* PMTicketGetCFBoolean(void)
{
    if (verbose) puts("STUB: PMTicketGetCFBoolean called");
    return NULL;
}

void* PMTicketGetCFData(void)
{
    if (verbose) puts("STUB: PMTicketGetCFData called");
    return NULL;
}

void* PMTicketGetCFDate(void)
{
    if (verbose) puts("STUB: PMTicketGetCFDate called");
    return NULL;
}

void* PMTicketGetCFDictionary(void)
{
    if (verbose) puts("STUB: PMTicketGetCFDictionary called");
    return NULL;
}

void* PMTicketGetCFNumber(void)
{
    if (verbose) puts("STUB: PMTicketGetCFNumber called");
    return NULL;
}

void* PMTicketGetCFString(void)
{
    if (verbose) puts("STUB: PMTicketGetCFString called");
    return NULL;
}

void* PMTicketGetCString(void)
{
    if (verbose) puts("STUB: PMTicketGetCString called");
    return NULL;
}

void* PMTicketGetDouble(void)
{
    if (verbose) puts("STUB: PMTicketGetDouble called");
    return NULL;
}

void* PMTicketGetEnumType(void)
{
    if (verbose) puts("STUB: PMTicketGetEnumType called");
    return NULL;
}

void* PMTicketGetItem(void)
{
    if (verbose) puts("STUB: PMTicketGetItem called");
    return NULL;
}

void* PMTicketGetPMRect(void)
{
    if (verbose) puts("STUB: PMTicketGetPMRect called");
    return NULL;
}

void* PMTicketGetPMResolution(void)
{
    if (verbose) puts("STUB: PMTicketGetPMResolution called");
    return NULL;
}

void* PMTicketGetPString(void)
{
    if (verbose) puts("STUB: PMTicketGetPString called");
    return NULL;
}

void* PMTicketGetRetainCount(void)
{
    if (verbose) puts("STUB: PMTicketGetRetainCount called");
    return NULL;
}

void* PMTicketGetSInt32(void)
{
    if (verbose) puts("STUB: PMTicketGetSInt32 called");
    return NULL;
}

void* PMTicketGetThisBoolean(void)
{
    if (verbose) puts("STUB: PMTicketGetThisBoolean called");
    return NULL;
}

void* PMTicketGetThisCString(void)
{
    if (verbose) puts("STUB: PMTicketGetThisCString called");
    return NULL;
}

void* PMTicketGetThisItem(void)
{
    if (verbose) puts("STUB: PMTicketGetThisItem called");
    return NULL;
}

void* PMTicketGetThisPMRect(void)
{
    if (verbose) puts("STUB: PMTicketGetThisPMRect called");
    return NULL;
}

void* PMTicketGetThisSInt32(void)
{
    if (verbose) puts("STUB: PMTicketGetThisSInt32 called");
    return NULL;
}

void* PMTicketGetThisTypedItem(void)
{
    if (verbose) puts("STUB: PMTicketGetThisTypedItem called");
    return NULL;
}

void* PMTicketGetTicket(void)
{
    if (verbose) puts("STUB: PMTicketGetTicket called");
    return NULL;
}

void* PMTicketGetType(void)
{
    if (verbose) puts("STUB: PMTicketGetType called");
    return NULL;
}

void* PMTicketGetUInt32(void)
{
    if (verbose) puts("STUB: PMTicketGetUInt32 called");
    return NULL;
}

void* PMTicketGetUTF8String(void)
{
    if (verbose) puts("STUB: PMTicketGetUTF8String called");
    return NULL;
}

void* PMTicketReadXMLFromCFURLRef(void)
{
    if (verbose) puts("STUB: PMTicketReadXMLFromCFURLRef called");
    return NULL;
}

void* PMTicketReadXMLFromFile(void)
{
    if (verbose) puts("STUB: PMTicketReadXMLFromFile called");
    return NULL;
}

void* PMTicketRelease(void)
{
    if (verbose) puts("STUB: PMTicketRelease called");
    return NULL;
}

void* PMTicketReleaseAndClear(void)
{
    if (verbose) puts("STUB: PMTicketReleaseAndClear called");
    return NULL;
}

void* PMTicketReleaseItem(void)
{
    if (verbose) puts("STUB: PMTicketReleaseItem called");
    return NULL;
}

void* PMTicketRemoveTicket(void)
{
    if (verbose) puts("STUB: PMTicketRemoveTicket called");
    return NULL;
}

void* PMTicketRetain(void)
{
    if (verbose) puts("STUB: PMTicketRetain called");
    return NULL;
}

void* PMTicketSetBoolean(void)
{
    if (verbose) puts("STUB: PMTicketSetBoolean called");
    return NULL;
}

void* PMTicketSetBytes(void)
{
    if (verbose) puts("STUB: PMTicketSetBytes called");
    return NULL;
}

void* PMTicketSetCFArray(void)
{
    if (verbose) puts("STUB: PMTicketSetCFArray called");
    return NULL;
}

void* PMTicketSetCFBoolean(void)
{
    if (verbose) puts("STUB: PMTicketSetCFBoolean called");
    return NULL;
}

void* PMTicketSetCFData(void)
{
    if (verbose) puts("STUB: PMTicketSetCFData called");
    return NULL;
}

void* PMTicketSetCFDate(void)
{
    if (verbose) puts("STUB: PMTicketSetCFDate called");
    return NULL;
}

void* PMTicketSetCFDictionary(void)
{
    if (verbose) puts("STUB: PMTicketSetCFDictionary called");
    return NULL;
}

void* PMTicketSetCFNumber(void)
{
    if (verbose) puts("STUB: PMTicketSetCFNumber called");
    return NULL;
}

void* PMTicketSetCFString(void)
{
    if (verbose) puts("STUB: PMTicketSetCFString called");
    return NULL;
}

void* PMTicketSetCString(void)
{
    if (verbose) puts("STUB: PMTicketSetCString called");
    return NULL;
}

void* PMTicketSetCStringArray(void)
{
    if (verbose) puts("STUB: PMTicketSetCStringArray called");
    return NULL;
}

void* PMTicketSetDouble(void)
{
    if (verbose) puts("STUB: PMTicketSetDouble called");
    return NULL;
}

void* PMTicketSetDoubleArray(void)
{
    if (verbose) puts("STUB: PMTicketSetDoubleArray called");
    return NULL;
}

void* PMTicketSetItem(void)
{
    if (verbose) puts("STUB: PMTicketSetItem called");
    return NULL;
}

void* PMTicketSetPMRect(void)
{
    if (verbose) puts("STUB: PMTicketSetPMRect called");
    return NULL;
}

void* PMTicketSetPMRectArray(void)
{
    if (verbose) puts("STUB: PMTicketSetPMRectArray called");
    return NULL;
}

void* PMTicketSetPMResolution(void)
{
    if (verbose) puts("STUB: PMTicketSetPMResolution called");
    return NULL;
}

void* PMTicketSetPMResolutionArray(void)
{
    if (verbose) puts("STUB: PMTicketSetPMResolutionArray called");
    return NULL;
}

void* PMTicketSetPString(void)
{
    if (verbose) puts("STUB: PMTicketSetPString called");
    return NULL;
}

void* PMTicketSetSInt32(void)
{
    if (verbose) puts("STUB: PMTicketSetSInt32 called");
    return NULL;
}

void* PMTicketSetSInt32Array(void)
{
    if (verbose) puts("STUB: PMTicketSetSInt32Array called");
    return NULL;
}

void* PMTicketSetTemplate(void)
{
    if (verbose) puts("STUB: PMTicketSetTemplate called");
    return NULL;
}

void* PMTicketSetTicket(void)
{
    if (verbose) puts("STUB: PMTicketSetTicket called");
    return NULL;
}

void* PMTicketSetUInt32(void)
{
    if (verbose) puts("STUB: PMTicketSetUInt32 called");
    return NULL;
}

void* PMTicketSetUInt32Array(void)
{
    if (verbose) puts("STUB: PMTicketSetUInt32Array called");
    return NULL;
}

void* PMTicketToLightXML(void)
{
    if (verbose) puts("STUB: PMTicketToLightXML called");
    return NULL;
}

void* PMTicketToXML(void)
{
    if (verbose) puts("STUB: PMTicketToXML called");
    return NULL;
}

void* PMTicketValidate(void)
{
    if (verbose) puts("STUB: PMTicketValidate called");
    return NULL;
}

void* PMTicketWriteXML(void)
{
    if (verbose) puts("STUB: PMTicketWriteXML called");
    return NULL;
}

void* PMTicketWriteXMLToFile(void)
{
    if (verbose) puts("STUB: PMTicketWriteXMLToFile called");
    return NULL;
}

void* PMUnflattenPageFormatWithCFData(void)
{
    if (verbose) puts("STUB: PMUnflattenPageFormatWithCFData called");
    return NULL;
}

void* PMUnflattenPageFormatWithURL(void)
{
    if (verbose) puts("STUB: PMUnflattenPageFormatWithURL called");
    return NULL;
}

void* PMUnflattenPrintSettingsWithCFData(void)
{
    if (verbose) puts("STUB: PMUnflattenPrintSettingsWithCFData called");
    return NULL;
}

void* PMUnflattenPrintSettingsWithURL(void)
{
    if (verbose) puts("STUB: PMUnflattenPrintSettingsWithURL called");
    return NULL;
}

void* PMUpdateProxy(void)
{
    if (verbose) puts("STUB: PMUpdateProxy called");
    return NULL;
}

void* PMWorkflowCopyItems(void)
{
    if (verbose) puts("STUB: PMWorkflowCopyItems called");
    return NULL;
}

void* PMWorkflowSubmitPDFWithOptions(void)
{
    if (verbose) puts("STUB: PMWorkflowSubmitPDFWithOptions called");
    return NULL;
}

void* PMWorkflowSubmitPDFWithSettings(void)
{
    if (verbose) puts("STUB: PMWorkflowSubmitPDFWithSettings called");
    return NULL;
}

void* PMWriteCFDataToFile(void)
{
    if (verbose) puts("STUB: PMWriteCFDataToFile called");
    return NULL;
}

void* PMXMLToTicket(void)
{
    if (verbose) puts("STUB: PMXMLToTicket called");
    return NULL;
}

void* PMfprintf(void)
{
    if (verbose) puts("STUB: PMfprintf called");
    return NULL;
}

void* _ZN12PMBaseObject14SetAppReadOnlyEh(void)
{
    if (verbose) puts("STUB: _ZN12PMBaseObject14SetAppReadOnlyEh called");
    return NULL;
}

void* _ZN12PMBaseObject15CopyObjectStateEPS_(void)
{
    if (verbose) puts("STUB: _ZN12PMBaseObject15CopyObjectStateEPS_ called");
    return NULL;
}

void* _ZN12PMBaseObject16CreateEmptyArrayEv(void)
{
    if (verbose) puts("STUB: _ZN12PMBaseObject16CreateEmptyArrayEv called");
    return NULL;
}

void* _ZN14OpaquePMPresetC1EPK14__CFDictionary(void)
{
    if (verbose) puts("STUB: _ZN14OpaquePMPresetC1EPK14__CFDictionary called");
    return NULL;
}

void* _ZN14OpaquePMPresetC2EPK14__CFDictionary(void)
{
    if (verbose) puts("STUB: _ZN14OpaquePMPresetC2EPK14__CFDictionary called");
    return NULL;
}

void* _ZN15OpaquePMPrinter14GetPrinterNameEv(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter14GetPrinterNameEv called");
    return NULL;
}

void* _ZN15OpaquePMPrinter14IsQueueStoppedEv(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter14IsQueueStoppedEv called");
    return NULL;
}

void* _ZN15OpaquePMPrinter16IsGenericPrinterEv(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter16IsGenericPrinterEv called");
    return NULL;
}

void* _ZN15OpaquePMPrinter19GetPaperListForMenuEv(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter19GetPaperListForMenuEv called");
    return NULL;
}

void* _ZN15OpaquePMPrinter19SetPaperListForMenuEPK9__CFArray(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter19SetPaperListForMenuEPK9__CFArray called");
    return NULL;
}

void* _ZN15OpaquePMPrinter22IsPrinterDirectConnectEv(void)
{
    if (verbose) puts("STUB: _ZN15OpaquePMPrinter22IsPrinterDirectConnectEv called");
    return NULL;
}

void* _ZN18OpaquePMPageFormat21PJCValidPageFormatKeyEPS_(void)
{
    if (verbose) puts("STUB: _ZN18OpaquePMPageFormat21PJCValidPageFormatKeyEPS_ called");
    return NULL;
}

void* _ZN18PMTicketBaseObject12GetTicketRefEv(void)
{
    if (verbose) puts("STUB: _ZN18PMTicketBaseObject12GetTicketRefEv called");
    return NULL;
}

void* _ZN18PMTicketBaseObject12SetTicketRefEP17OpaquePMTicketRef(void)
{
    if (verbose) puts("STUB: _ZN18PMTicketBaseObject12SetTicketRefEP17OpaquePMTicketRef called");
    return NULL;
}

void* _ZN18PMTicketBaseObject23ReleaseAndReplaceTicketEP17OpaquePMTicketRef(void)
{
    if (verbose) puts("STUB: _ZN18PMTicketBaseObject23ReleaseAndReplaceTicketEP17OpaquePMTicketRef called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession10ClearErrorEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession10ClearErrorEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession10SetPreviewEhj(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession10SetPreviewEhj called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession12GetJobTicketEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession12GetJobTicketEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession16GetSheetDoneProcEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession16GetSheetDoneProcEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession16GetXPCConnectionEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession16GetXPCConnectionEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession16SetSheetDoneProcEPFvPS_P15OpaqueWindowPtrhE(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession16SetSheetDoneProcEPFvPS_P15OpaqueWindowPtrhE called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession16SetXPCConnectionEPU24objcproto13OS_xpc_object8NSObject(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession16SetXPCConnectionEPU24objcproto13OS_xpc_object8NSObject called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession17GetCurrentPrinterEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession17GetCurrentPrinterEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession17GetDocumentWindowEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession17GetDocumentWindowEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession17SetCurrentPrinterEP15OpaquePMPrinter(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession17SetCurrentPrinterEP15OpaquePMPrinter called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession17SetCurrentPrinterEP15OpaquePMPrinterh(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession17SetCurrentPrinterEP15OpaquePMPrinterh called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession17SetDocumentWindowEP15OpaqueWindowPtr(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession17SetDocumentWindowEP15OpaqueWindowPtr called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession19SetDataInDictionaryEPK10__CFStringPKv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession19SetDataInDictionaryEPK10__CFStringPKv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession21GetEnablePresetsPopUpEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession21GetEnablePresetsPopUpEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession21GetPresetGraphicsTypeEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession21GetPresetGraphicsTypeEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession21SetPresetGraphicsTypeEPK10__CFString(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession21SetPresetGraphicsTypeEPK10__CFString called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession23PJCValidPrintSessionKeyEPS_(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession23PJCValidPrintSessionKeyEPS_ called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession24GetDefaultButtonTitleRefEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession24GetDefaultButtonTitleRefEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession25GetPrintingDialogTitleRefEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession25GetPrintingDialogTitleRefEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession7PreviewEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession7PreviewEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession8GetErrorEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession8GetErrorEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession8GetStateEv(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession8GetStateEv called");
    return NULL;
}

void* _ZN20OpaquePMPrintSession8SetErrorEi(void)
{
    if (verbose) puts("STUB: _ZN20OpaquePMPrintSession8SetErrorEi called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings14PJCGetLastPageEPj(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings14PJCGetLastPageEPj called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings14PJCSetLastPageEP17OpaquePMTicketRefjhh(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings14PJCSetLastPageEP17OpaquePMTicketRefjhh called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings14PJCSetLastPageEjhh(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings14PJCSetLastPageEjhh called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings15PJCGetFirstPageEPj(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings15PJCGetFirstPageEPj called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings17PJCGetDestinationEPtPPK7__CFURL(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings17PJCGetDestinationEPtPPK7__CFURL called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings17PJCSetDestinationEtPK7__CFURL(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings17PJCSetDestinationEtPK7__CFURL called");
    return NULL;
}

void* _ZN21OpaquePMPrintSettings24PJCValidPrintSettingsKeyEPS_(void)
{
    if (verbose) puts("STUB: _ZN21OpaquePMPrintSettings24PJCValidPrintSettingsKeyEPS_ called");
    return NULL;
}

void* getCurrentMonochromeProfileID(void)
{
    if (verbose) puts("STUB: getCurrentMonochromeProfileID called");
    return NULL;
}

void* getMatchedMediaNameFromPrinter(void)
{
    if (verbose) puts("STUB: getMatchedMediaNameFromPrinter called");
    return NULL;
}

void* getMediaNameFromPaperInfo(void)
{
    if (verbose) puts("STUB: getMediaNameFromPaperInfo called");
    return NULL;
}

void* getPDFSpoolingFunctionCallbacks(void)
{
    if (verbose) puts("STUB: getPDFSpoolingFunctionCallbacks called");
    return NULL;
}

void* parsePostScriptStatus(void)
{
    if (verbose) puts("STUB: parsePostScriptStatus called");
    return NULL;
}

void* printUIToolAuthenticateJob(void)
{
    if (verbose) puts("STUB: printUIToolAuthenticateJob called");
    return NULL;
}

void* printUIToolCreateConnection(void)
{
    if (verbose) puts("STUB: printUIToolCreateConnection called");
    return NULL;
}

void* printUIToolGetAuthenticationInfo(void)
{
    if (verbose) puts("STUB: printUIToolGetAuthenticationInfo called");
    return NULL;
}

void* printUIToolPrintFD(void)
{
    if (verbose) puts("STUB: printUIToolPrintFD called");
    return NULL;
}
