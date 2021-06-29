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


#include <InputMethodKit/InputMethodKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* IMKCandidateControllerUITypeIsWindowBacked(void)
{
    if (verbose) puts("STUB: IMKCandidateControllerUITypeIsWindowBacked called");
    return NULL;
}

void* IMKContainsShortcutModifier(void)
{
    if (verbose) puts("STUB: IMKContainsShortcutModifier called");
    return NULL;
}

void* IMKIsHighSurrogate(void)
{
    if (verbose) puts("STUB: IMKIsHighSurrogate called");
    return NULL;
}

void* IMKIsLowSurrogate(void)
{
    if (verbose) puts("STUB: IMKIsLowSurrogate called");
    return NULL;
}

void* IMKIsNumber(void)
{
    if (verbose) puts("STUB: IMKIsNumber called");
    return NULL;
}

void* IMKIsOnBigEndianSystem(void)
{
    if (verbose) puts("STUB: IMKIsOnBigEndianSystem called");
    return NULL;
}

void* IMKIsSentenceEndingCharacter(void)
{
    if (verbose) puts("STUB: IMKIsSentenceEndingCharacter called");
    return NULL;
}

void* IMKIsSurrogate(void)
{
    if (verbose) puts("STUB: IMKIsSurrogate called");
    return NULL;
}

void* IMKKeyModifiersFromNSModifierFlags(void)
{
    if (verbose) puts("STUB: IMKKeyModifiersFromNSModifierFlags called");
    return NULL;
}

void* IMKLayoutCandidatePositionIsValid(void)
{
    if (verbose) puts("STUB: IMKLayoutCandidatePositionIsValid called");
    return NULL;
}

void* IMKSentenceEndingCharacterSet(void)
{
    if (verbose) puts("STUB: IMKSentenceEndingCharacterSet called");
    return NULL;
}

void* IMKSimulatorCandidateProxies(void)
{
    if (verbose) puts("STUB: IMKSimulatorCandidateProxies called");
    return NULL;
}

void* IMKSimulatorCandidateSurfaces(void)
{
    if (verbose) puts("STUB: IMKSimulatorCandidateSurfaces called");
    return NULL;
}

void* IMKSimulatorCandidateWindowIsVisible(void)
{
    if (verbose) puts("STUB: IMKSimulatorCandidateWindowIsVisible called");
    return NULL;
}

void* IMKSimulatorCandidateWindowSelectedSurface(void)
{
    if (verbose) puts("STUB: IMKSimulatorCandidateWindowSelectedSurface called");
    return NULL;
}

void* IMKSimulatorCandidateWindowTopVisibleLine(void)
{
    if (verbose) puts("STUB: IMKSimulatorCandidateWindowTopVisibleLine called");
    return NULL;
}

void* IMKSimulatorCarbonModifiersForNSModifiers(void)
{
    if (verbose) puts("STUB: IMKSimulatorCarbonModifiersForNSModifiers called");
    return NULL;
}

void* IMKSimulatorControlKeyModifiers(void)
{
    if (verbose) puts("STUB: IMKSimulatorControlKeyModifiers called");
    return NULL;
}

void* IMKSimulatorDictionaryFromAccessibilityString(void)
{
    if (verbose) puts("STUB: IMKSimulatorDictionaryFromAccessibilityString called");
    return NULL;
}

void* IMKSimulatorGroupedCandidateProxies(void)
{
    if (verbose) puts("STUB: IMKSimulatorGroupedCandidateProxies called");
    return NULL;
}

void* IMKSimulatorGroupedCandidateSurfaces(void)
{
    if (verbose) puts("STUB: IMKSimulatorGroupedCandidateSurfaces called");
    return NULL;
}

void* IMKSimulatorMainInit(void)
{
    if (verbose) puts("STUB: IMKSimulatorMainInit called");
    return NULL;
}

void* IMKSimulatorMarkedRange(void)
{
    if (verbose) puts("STUB: IMKSimulatorMarkedRange called");
    return NULL;
}

void* IMKSimulatorMarkedText(void)
{
    if (verbose) puts("STUB: IMKSimulatorMarkedText called");
    return NULL;
}

void* IMKSimulatorPreferencesClear(void)
{
    if (verbose) puts("STUB: IMKSimulatorPreferencesClear called");
    return NULL;
}

void* IMKSimulatorPreferencesSetValue(void)
{
    if (verbose) puts("STUB: IMKSimulatorPreferencesSetValue called");
    return NULL;
}

void* IMKSimulatorSelectedRange(void)
{
    if (verbose) puts("STUB: IMKSimulatorSelectedRange called");
    return NULL;
}

void* IMKSimulatorSelectedText(void)
{
    if (verbose) puts("STUB: IMKSimulatorSelectedText called");
    return NULL;
}

void* IMKSimulatorStringValue(void)
{
    if (verbose) puts("STUB: IMKSimulatorStringValue called");
    return NULL;
}

void* IMKSimulatorTouchBarCandidateSurfaces(void)
{
    if (verbose) puts("STUB: IMKSimulatorTouchBarCandidateSurfaces called");
    return NULL;
}

void* IMKSimulatorTouchBarCandidateWindowSelectedSurface(void)
{
    if (verbose) puts("STUB: IMKSimulatorTouchBarCandidateWindowSelectedSurface called");
    return NULL;
}

void* IMKUIBackingAwareCeil(void)
{
    if (verbose) puts("STUB: IMKUIBackingAwareCeil called");
    return NULL;
}

void* IMKUIBackingAwareFloor(void)
{
    if (verbose) puts("STUB: IMKUIBackingAwareFloor called");
    return NULL;
}

void* IMKUICandidatePanelTypeSupportsAlignment(void)
{
    if (verbose) puts("STUB: IMKUICandidatePanelTypeSupportsAlignment called");
    return NULL;
}

void* IMKUILocationHintString(void)
{
    if (verbose) puts("STUB: IMKUILocationHintString called");
    return NULL;
}

void* IMKUIOppositeLocationHint(void)
{
    if (verbose) puts("STUB: IMKUIOppositeLocationHint called");
    return NULL;
}

void* IMKUIPanelTypeIsScrubber(void)
{
    if (verbose) puts("STUB: IMKUIPanelTypeIsScrubber called");
    return NULL;
}

void* IMKUIPanelTypeIsTouchBar(void)
{
    if (verbose) puts("STUB: IMKUIPanelTypeIsTouchBar called");
    return NULL;
}

void* IMKUIPanelTypeToString(void)
{
    if (verbose) puts("STUB: IMKUIPanelTypeToString called");
    return NULL;
}

void* IMKUTF32StringEncoding(void)
{
    if (verbose) puts("STUB: IMKUTF32StringEncoding called");
    return NULL;
}

void* IMKUnsignedSubtract(void)
{
    if (verbose) puts("STUB: IMKUnsignedSubtract called");
    return NULL;
}

void* IMKWindowCategoryFromType(void)
{
    if (verbose) puts("STUB: IMKWindowCategoryFromType called");
    return NULL;
}

void* NSModifierFlagsFromIMKKeyModifiers(void)
{
    if (verbose) puts("STUB: NSModifierFlagsFromIMKKeyModifiers called");
    return NULL;
}

void* NSRectRoundedSize(void)
{
    if (verbose) puts("STUB: NSRectRoundedSize called");
    return NULL;
}

void* NSSizeAddSize(void)
{
    if (verbose) puts("STUB: NSSizeAddSize called");
    return NULL;
}

void* NSStringFromCandidateArray(void)
{
    if (verbose) puts("STUB: NSStringFromCandidateArray called");
    return NULL;
}

void* NSStringFromIMKCandidateWindowType(void)
{
    if (verbose) puts("STUB: NSStringFromIMKCandidateWindowType called");
    return NULL;
}

void* NSZeroOriginRect(void)
{
    if (verbose) puts("STUB: NSZeroOriginRect called");
    return NULL;
}

void* NSZeroOriginRectFromSize(void)
{
    if (verbose) puts("STUB: NSZeroOriginRectFromSize called");
    return NULL;
}

void* asl_init(void)
{
    if (verbose) puts("STUB: asl_init called");
    return NULL;
}

void* asl_stop(void)
{
    if (verbose) puts("STUB: asl_stop called");
    return NULL;
}

void* isEqualWithNil(void)
{
    if (verbose) puts("STUB: isEqualWithNil called");
    return NULL;
}

void* nameFor(void)
{
    if (verbose) puts("STUB: nameFor called");
    return NULL;
}

void* server_client_ipc_signpost_log(void)
{
    if (verbose) puts("STUB: server_client_ipc_signpost_log called");
    return NULL;
}

void* server_perf_signpost_log(void)
{
    if (verbose) puts("STUB: server_perf_signpost_log called");
    return NULL;
}

void* server_ui_signpost_log(void)
{
    if (verbose) puts("STUB: server_ui_signpost_log called");
    return NULL;
}
