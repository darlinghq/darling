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


#include <JavaRuntimeSupport/JavaRuntimeSupport.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CopyCFStringToFilesystemRepresentation(void)
{
    if (verbose) puts("STUB: CopyCFStringToFilesystemRepresentation called");
    return NULL;
}

void* CopyCFStringToUTF8(void)
{
    if (verbose) puts("STUB: CopyCFStringToUTF8 called");
    return NULL;
}

void* GetHitPart(void)
{
    if (verbose) puts("STUB: GetHitPart called");
    return NULL;
}

void* GetScrollBarHitPart(void)
{
    if (verbose) puts("STUB: GetScrollBarHitPart called");
    return NULL;
}

void* JRSAccessibilityUnregisterUniqueIdForUIElement(void)
{
    if (verbose) puts("STUB: JRSAccessibilityUnregisterUniqueIdForUIElement called");
    return NULL;
}

void* JRSCopyCanonicalLanguageForPrimaryLanguage(void)
{
    if (verbose) puts("STUB: JRSCopyCanonicalLanguageForPrimaryLanguage called");
    return NULL;
}

void* JRSCopyOSJavaSupportVersion(void)
{
    if (verbose) puts("STUB: JRSCopyOSJavaSupportVersion called");
    return NULL;
}

void* JRSCopyOSName(void)
{
    if (verbose) puts("STUB: JRSCopyOSName called");
    return NULL;
}

void* JRSCopyOSVersion(void)
{
    if (verbose) puts("STUB: JRSCopyOSVersion called");
    return NULL;
}

void* JRSCopyPrimaryLanguage(void)
{
    if (verbose) puts("STUB: JRSCopyPrimaryLanguage called");
    return NULL;
}

void* JRSFontAlignStyleForFractionalMeasurement(void)
{
    if (verbose) puts("STUB: JRSFontAlignStyleForFractionalMeasurement called");
    return NULL;
}

void* JRSFontAlignStyleForIntegerMeasurement(void)
{
    if (verbose) puts("STUB: JRSFontAlignStyleForIntegerMeasurement called");
    return NULL;
}

void* JRSFontCreateFallbackFontForCharacters(void)
{
    if (verbose) puts("STUB: JRSFontCreateFallbackFontForCharacters called");
    return NULL;
}

void* JRSFontGetAdvancesForGlyphsAndStyle(void)
{
    if (verbose) puts("STUB: JRSFontGetAdvancesForGlyphsAndStyle called");
    return NULL;
}

void* JRSFontGetBoundingBoxesForGlyphsAndStyle(void)
{
    if (verbose) puts("STUB: JRSFontGetBoundingBoxesForGlyphsAndStyle called");
    return NULL;
}

void* JRSFontGetRenderingStyleForContext(void)
{
    if (verbose) puts("STUB: JRSFontGetRenderingStyleForContext called");
    return NULL;
}

void* JRSFontGetRenderingStyleForHints(void)
{
    if (verbose) puts("STUB: JRSFontGetRenderingStyleForHints called");
    return NULL;
}

void* JRSFontSetRenderingStyleOnContext(void)
{
    if (verbose) puts("STUB: JRSFontSetRenderingStyleOnContext called");
    return NULL;
}

void* JRSFontStyleIsAntialiased(void)
{
    if (verbose) puts("STUB: JRSFontStyleIsAntialiased called");
    return NULL;
}

void* JRSFontStyleUsesFractionalMetrics(void)
{
    if (verbose) puts("STUB: JRSFontStyleUsesFractionalMetrics called");
    return NULL;
}

void* JRSSetDefaultLocalization(void)
{
    if (verbose) puts("STUB: JRSSetDefaultLocalization called");
    return NULL;
}

void* JRSUIControlCreate(void)
{
    if (verbose) puts("STUB: JRSUIControlCreate called");
    return NULL;
}

void* JRSUIControlDraw(void)
{
    if (verbose) puts("STUB: JRSUIControlDraw called");
    return NULL;
}

void* JRSUIControlGetHitPart(void)
{
    if (verbose) puts("STUB: JRSUIControlGetHitPart called");
    return NULL;
}

void* JRSUIControlGetScrollBarOffsetFor(void)
{
    if (verbose) puts("STUB: JRSUIControlGetScrollBarOffsetFor called");
    return NULL;
}

void* JRSUIControlGetScrollBarPartBounds(void)
{
    if (verbose) puts("STUB: JRSUIControlGetScrollBarPartBounds called");
    return NULL;
}

void* JRSUIControlRelease(void)
{
    if (verbose) puts("STUB: JRSUIControlRelease called");
    return NULL;
}

void* JRSUIControlSetAlignmentHorizontal(void)
{
    if (verbose) puts("STUB: JRSUIControlSetAlignmentHorizontal called");
    return NULL;
}

void* JRSUIControlSetAlignmentVertical(void)
{
    if (verbose) puts("STUB: JRSUIControlSetAlignmentVertical called");
    return NULL;
}

void* JRSUIControlSetAnimating(void)
{
    if (verbose) puts("STUB: JRSUIControlSetAnimating called");
    return NULL;
}

void* JRSUIControlSetDirection(void)
{
    if (verbose) puts("STUB: JRSUIControlSetDirection called");
    return NULL;
}

void* JRSUIControlSetOrientation(void)
{
    if (verbose) puts("STUB: JRSUIControlSetOrientation called");
    return NULL;
}

void* JRSUIControlSetPresentationState(void)
{
    if (verbose) puts("STUB: JRSUIControlSetPresentationState called");
    return NULL;
}

void* JRSUIControlSetScrollBarPart(void)
{
    if (verbose) puts("STUB: JRSUIControlSetScrollBarPart called");
    return NULL;
}

void* JRSUIControlSetSegmentPosition(void)
{
    if (verbose) puts("STUB: JRSUIControlSetSegmentPosition called");
    return NULL;
}

void* JRSUIControlSetShowArrows(void)
{
    if (verbose) puts("STUB: JRSUIControlSetShowArrows called");
    return NULL;
}

void* JRSUIControlSetSize(void)
{
    if (verbose) puts("STUB: JRSUIControlSetSize called");
    return NULL;
}

void* JRSUIControlSetState(void)
{
    if (verbose) puts("STUB: JRSUIControlSetState called");
    return NULL;
}

void* JRSUIControlSetUserInterfaceLayoutDirection(void)
{
    if (verbose) puts("STUB: JRSUIControlSetUserInterfaceLayoutDirection called");
    return NULL;
}

void* JRSUIControlSetValueByKey(void)
{
    if (verbose) puts("STUB: JRSUIControlSetValueByKey called");
    return NULL;
}

void* JRSUIControlSetVariant(void)
{
    if (verbose) puts("STUB: JRSUIControlSetVariant called");
    return NULL;
}

void* JRSUIControlSetWidget(void)
{
    if (verbose) puts("STUB: JRSUIControlSetWidget called");
    return NULL;
}

void* JRSUIControlSetWindowType(void)
{
    if (verbose) puts("STUB: JRSUIControlSetWindowType called");
    return NULL;
}

void* JRSUIControlShouldScrollToClick(void)
{
    if (verbose) puts("STUB: JRSUIControlShouldScrollToClick called");
    return NULL;
}

void* JRSUIGetKey(void)
{
    if (verbose) puts("STUB: JRSUIGetKey called");
    return NULL;
}

void* JRSUIRendererCreate(void)
{
    if (verbose) puts("STUB: JRSUIRendererCreate called");
    return NULL;
}

void* JRSUIRendererRelease(void)
{
    if (verbose) puts("STUB: JRSUIRendererRelease called");
    return NULL;
}

void* do_transfer_port_send_right(void)
{
    if (verbose) puts("STUB: do_transfer_port_send_right called");
    return NULL;
}

void* port_send_right_transfer_server(void)
{
    if (verbose) puts("STUB: port_send_right_transfer_server called");
    return NULL;
}

void* port_send_right_transfer_server_routine(void)
{
    if (verbose) puts("STUB: port_send_right_transfer_server_routine called");
    return NULL;
}

void* transfer_port_send_right(void)
{
    if (verbose) puts("STUB: transfer_port_send_right called");
    return NULL;
}
