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


#include <CommonPanels/CommonPanels.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CCP_FontPanelClose(void)
{
    if (verbose) puts("STUB: CCP_FontPanelClose called");
    return NULL;
}

void* CCP_FontPanelGetSelection(void)
{
    if (verbose) puts("STUB: CCP_FontPanelGetSelection called");
    return NULL;
}

void* CCP_FontPanelIsVisible(void)
{
    if (verbose) puts("STUB: CCP_FontPanelIsVisible called");
    return NULL;
}

void* CCP_FontPanelOpen(void)
{
    if (verbose) puts("STUB: CCP_FontPanelOpen called");
    return NULL;
}

void* CCP_FontPanelSelectFonts(void)
{
    if (verbose) puts("STUB: CCP_FontPanelSelectFonts called");
    return NULL;
}

void* CCP_NPickColor(void)
{
    if (verbose) puts("STUB: CCP_NPickColor called");
    return NULL;
}

void* CMCalibrateDisplay(void)
{
    if (verbose) puts("STUB: CMCalibrateDisplay called");
    return NULL;
}

void* Call_CommonCocoaPanels_NPickColor(void)
{
    if (verbose) puts("STUB: Call_CommonCocoaPanels_NPickColor called");
    return NULL;
}

void* DisposeCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: DisposeCalibrateEventUPP called");
    return NULL;
}

void* DisposeCalibrateUPP(void)
{
    if (verbose) puts("STUB: DisposeCalibrateUPP called");
    return NULL;
}

void* DisposeCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: DisposeCanCalibrateUPP called");
    return NULL;
}

void* DisposeNColorChangedUPP(void)
{
    if (verbose) puts("STUB: DisposeNColorChangedUPP called");
    return NULL;
}

void* FCAddCollection(void)
{
    if (verbose) puts("STUB: FCAddCollection called");
    return NULL;
}

void* FCAddFontDescriptorToCollection(void)
{
    if (verbose) puts("STUB: FCAddFontDescriptorToCollection called");
    return NULL;
}

void* FCCopyCollectionNames(void)
{
    if (verbose) puts("STUB: FCCopyCollectionNames called");
    return NULL;
}

void* FCCopyFontDescriptorsInCollection(void)
{
    if (verbose) puts("STUB: FCCopyFontDescriptorsInCollection called");
    return NULL;
}

void* FCFontDescriptorCreateWithFontAttributes(void)
{
    if (verbose) puts("STUB: FCFontDescriptorCreateWithFontAttributes called");
    return NULL;
}

void* FCFontDescriptorCreateWithName(void)
{
    if (verbose) puts("STUB: FCFontDescriptorCreateWithName called");
    return NULL;
}

void* FCRemoveCollection(void)
{
    if (verbose) puts("STUB: FCRemoveCollection called");
    return NULL;
}

void* FCRemoveFontDescriptorFromCollection(void)
{
    if (verbose) puts("STUB: FCRemoveFontDescriptorFromCollection called");
    return NULL;
}

void* FPIsFontPanelVisible(void)
{
    if (verbose) puts("STUB: FPIsFontPanelVisible called");
    return NULL;
}

void* FPShowHideFontPanel(void)
{
    if (verbose) puts("STUB: FPShowHideFontPanel called");
    return NULL;
}

void* GetColor(void)
{
    if (verbose) puts("STUB: GetColor called");
    return NULL;
}

void* GetHandleApplicationEventsUPP(void)
{
    if (verbose) puts("STUB: GetHandleApplicationEventsUPP called");
    return NULL;
}

void* GetHandleWinEventsUPP(void)
{
    if (verbose) puts("STUB: GetHandleWinEventsUPP called");
    return NULL;
}

void* InvokeCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: InvokeCalibrateEventUPP called");
    return NULL;
}

void* InvokeCalibrateUPP(void)
{
    if (verbose) puts("STUB: InvokeCalibrateUPP called");
    return NULL;
}

void* InvokeCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: InvokeCanCalibrateUPP called");
    return NULL;
}

void* InvokeNColorChangedUPP(void)
{
    if (verbose) puts("STUB: InvokeNColorChangedUPP called");
    return NULL;
}

void* LoadNSTypographyBundle(void)
{
    if (verbose) puts("STUB: LoadNSTypographyBundle called");
    return NULL;
}

void* NPickColor(void)
{
    if (verbose) puts("STUB: NPickColor called");
    return NULL;
}

void* NewCalibrateEventUPP(void)
{
    if (verbose) puts("STUB: NewCalibrateEventUPP called");
    return NULL;
}

void* NewCalibrateUPP(void)
{
    if (verbose) puts("STUB: NewCalibrateUPP called");
    return NULL;
}

void* NewCanCalibrateUPP(void)
{
    if (verbose) puts("STUB: NewCanCalibrateUPP called");
    return NULL;
}

void* NewNColorChangedUPP(void)
{
    if (verbose) puts("STUB: NewNColorChangedUPP called");
    return NULL;
}

void* SetFontInfoForSelection(void)
{
    if (verbose) puts("STUB: SetFontInfoForSelection called");
    return NULL;
}

void* SoftLinkingGetFunctionPointerFromBundleInDirectory(void)
{
    if (verbose) puts("STUB: SoftLinkingGetFunctionPointerFromBundleInDirectory called");
    return NULL;
}

void* UserWantsColorPanelFromCommonCocoaPanels(void)
{
    if (verbose) puts("STUB: UserWantsColorPanelFromCommonCocoaPanels called");
    return NULL;
}

void* _SoftLinkingLoadBundleFromLocation(void)
{
    if (verbose) puts("STUB: _SoftLinkingLoadBundleFromLocation called");
    return NULL;
}
