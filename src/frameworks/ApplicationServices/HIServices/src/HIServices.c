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


#include <HIServices/HIServices.h>
#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <LaunchServices/LaunchServicesPriv.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>

const CFStringRef kAXUIElementCopyHierarchyArrayAttributesKey = CFSTR("AXCHAA");
const CFStringRef kAXUIElementCopyHierarchyMaxArrayCountKey = CFSTR("AXCHMAC");
const CFStringRef kAXUIElementCopyHierarchyResultValueKey = CFSTR("value");
const CFStringRef kAXUIElementCopyHierarchyReturnAttributeErrorsKey = CFSTR("AXCHRE");
const CFStringRef kAXUIElementCopyHierarchySkipInspectionForAttributesKey = CFSTR("AXCHSIA");
const CFStringRef kAXUIElementCopyHierarchyTruncateStringsKey = CFSTR("AXTRUNC");
/* Not const for some reason */
CFStringRef kAXTrustedCheckOptionPrompt = CFSTR("AXTrustedCheckOptionPrompt");

static int verbose = 0;
static Boolean g_isDaemon = FALSE;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

OSStatus SetApplicationIsDaemon(Boolean daemon)
{
    g_isDaemon = daemon;
    if (daemon)
        CGSSetDenyWindowServerConnections(TRUE);

    _LSSetApplicationLaunchServicesServerConnectionStatus(daemon ? 2 : 0, NULL);

    return noErr;
}

Boolean GetApplicationIsDaemon(void)
{
    return g_isDaemon;
}

void* AXAPIEnabled(void)
{
    if (verbose) puts("STUB: AXAPIEnabled called");
    return NULL;
}

void* AXIsProcessTrusted(void)
{
    if (verbose) puts("STUB: AXIsProcessTrusted called");
    return NULL;
}

void* AXIsProcessTrustedWithOptions(void)
{
    if (verbose) puts("STUB: AXIsProcessTrustedWithOptions called");
    return NULL;
}

void* AXMakeProcessTrusted(void)
{
    if (verbose) puts("STUB: AXMakeProcessTrusted called");
    return NULL;
}

void* AXObserverAddNotification(void)
{
    if (verbose) puts("STUB: AXObserverAddNotification called");
    return NULL;
}

void* AXObserverAddNotificationAsync(void)
{
    if (verbose) puts("STUB: AXObserverAddNotificationAsync called");
    return NULL;
}

void* AXObserverCreate(void)
{
    if (verbose) puts("STUB: AXObserverCreate called");
    return NULL;
}

void* AXObserverCreateWithInfoCallback(void)
{
    if (verbose) puts("STUB: AXObserverCreateWithInfoCallback called");
    return NULL;
}

void* AXObserverGetRunLoopSource(void)
{
    if (verbose) puts("STUB: AXObserverGetRunLoopSource called");
    return NULL;
}

void* AXObserverGetTypeID(void)
{
    if (verbose) puts("STUB: AXObserverGetTypeID called");
    return NULL;
}

void* AXObserverRemoveNotification(void)
{
    if (verbose) puts("STUB: AXObserverRemoveNotification called");
    return NULL;
}

void* AXObserverRemoveNotificationAsync(void)
{
    if (verbose) puts("STUB: AXObserverRemoveNotificationAsync called");
    return NULL;
}

void* AXSerializeCFType(void)
{
    if (verbose) puts("STUB: AXSerializeCFType called");
    return NULL;
}

void* AXTextMarkerCreate(void)
{
    if (verbose) puts("STUB: AXTextMarkerCreate called");
    return NULL;
}

void* AXTextMarkerGetBytePtr(void)
{
    if (verbose) puts("STUB: AXTextMarkerGetBytePtr called");
    return NULL;
}

void* AXTextMarkerGetLength(void)
{
    if (verbose) puts("STUB: AXTextMarkerGetLength called");
    return NULL;
}

void* AXTextMarkerGetTypeID(void)
{
    if (verbose) puts("STUB: AXTextMarkerGetTypeID called");
    return NULL;
}

void* AXTextMarkerRangeCopyEndMarker(void)
{
    if (verbose) puts("STUB: AXTextMarkerRangeCopyEndMarker called");
    return NULL;
}

void* AXTextMarkerRangeCopyStartMarker(void)
{
    if (verbose) puts("STUB: AXTextMarkerRangeCopyStartMarker called");
    return NULL;
}

void* AXTextMarkerRangeCreate(void)
{
    if (verbose) puts("STUB: AXTextMarkerRangeCreate called");
    return NULL;
}

void* AXTextMarkerRangeCreateWithBytes(void)
{
    if (verbose) puts("STUB: AXTextMarkerRangeCreateWithBytes called");
    return NULL;
}

void* AXTextMarkerRangeGetTypeID(void)
{
    if (verbose) puts("STUB: AXTextMarkerRangeGetTypeID called");
    return NULL;
}

void* AXUIElementCopyActionDescription(void)
{
    if (verbose) puts("STUB: AXUIElementCopyActionDescription called");
    return NULL;
}

void* AXUIElementCopyActionNames(void)
{
    if (verbose) puts("STUB: AXUIElementCopyActionNames called");
    return NULL;
}

void* AXUIElementCopyAttributeNames(void)
{
    if (verbose) puts("STUB: AXUIElementCopyAttributeNames called");
    return NULL;
}

void* AXUIElementCopyAttributeValue(void)
{
    if (verbose) puts("STUB: AXUIElementCopyAttributeValue called");
    return NULL;
}

void* AXUIElementCopyAttributeValues(void)
{
    if (verbose) puts("STUB: AXUIElementCopyAttributeValues called");
    return NULL;
}

void* AXUIElementCopyElementAtPosition(void)
{
    if (verbose) puts("STUB: AXUIElementCopyElementAtPosition called");
    return NULL;
}

void* AXUIElementCopyHierarchy(void)
{
    if (verbose) puts("STUB: AXUIElementCopyHierarchy called");
    return NULL;
}

void* AXUIElementCopyMultipleAttributeValues(void)
{
    if (verbose) puts("STUB: AXUIElementCopyMultipleAttributeValues called");
    return NULL;
}

void* AXUIElementCopyParameterizedAttributeNames(void)
{
    if (verbose) puts("STUB: AXUIElementCopyParameterizedAttributeNames called");
    return NULL;
}

void* AXUIElementCopyParameterizedAttributeValue(void)
{
    if (verbose) puts("STUB: AXUIElementCopyParameterizedAttributeValue called");
    return NULL;
}

void* AXUIElementCreateApplication(void)
{
    if (verbose) puts("STUB: AXUIElementCreateApplication called");
    return NULL;
}

void* AXUIElementCreateSystemWide(void)
{
    if (verbose) puts("STUB: AXUIElementCreateSystemWide called");
    return NULL;
}

void* AXUIElementGetAttributeValueCount(void)
{
    if (verbose) puts("STUB: AXUIElementGetAttributeValueCount called");
    return NULL;
}

void* AXUIElementGetPid(void)
{
    if (verbose) puts("STUB: AXUIElementGetPid called");
    return NULL;
}

void* AXUIElementGetTypeID(void)
{
    if (verbose) puts("STUB: AXUIElementGetTypeID called");
    return NULL;
}

void* AXUIElementIsAttributeSettable(void)
{
    if (verbose) puts("STUB: AXUIElementIsAttributeSettable called");
    return NULL;
}

void* AXUIElementPerformAction(void)
{
    if (verbose) puts("STUB: AXUIElementPerformAction called");
    return NULL;
}

void* AXUIElementPostKeyboardEvent(void)
{
    if (verbose) puts("STUB: AXUIElementPostKeyboardEvent called");
    return NULL;
}

void* AXUIElementSetAttributeValue(void)
{
    if (verbose) puts("STUB: AXUIElementSetAttributeValue called");
    return NULL;
}

void* AXUIElementSetMessagingTimeout(void)
{
    if (verbose) puts("STUB: AXUIElementSetMessagingTimeout called");
    return NULL;
}

void* AXUnserializeCFType(void)
{
    if (verbose) puts("STUB: AXUnserializeCFType called");
    return NULL;
}

void* AXValueCreate(void)
{
    if (verbose) puts("STUB: AXValueCreate called");
    return NULL;
}

void* AXValueGetType(void)
{
    if (verbose) puts("STUB: AXValueGetType called");
    return NULL;
}

void* AXValueGetTypeID(void)
{
    if (verbose) puts("STUB: AXValueGetTypeID called");
    return NULL;
}

void* AXValueGetValue(void)
{
    if (verbose) puts("STUB: AXValueGetValue called");
    return NULL;
}

void* ApplicationTypeGet(void)
{
    if (verbose) puts("STUB: ApplicationTypeGet called");
    return NULL;
}

void* ApplicationTypeSet(void)
{
    if (verbose) puts("STUB: ApplicationTypeSet called");
    return NULL;
}

void* CGPointInIconRef(void)
{
    if (verbose) puts("STUB: CGPointInIconRef called");
    return NULL;
}

void* CGRectInIconRef(void)
{
    if (verbose) puts("STUB: CGRectInIconRef called");
    return NULL;
}

void* CopyLabelColorAndName(void)
{
    if (verbose) puts("STUB: CopyLabelColorAndName called");
    return NULL;
}

void* CoreAppearanceGetFontCGStyleRef(void)
{
    if (verbose) puts("STUB: CoreAppearanceGetFontCGStyleRef called");
    return NULL;
}

void* CoreAppearanceGetFontShadowOutsets(void)
{
    if (verbose) puts("STUB: CoreAppearanceGetFontShadowOutsets called");
    return NULL;
}

void* CoreAppearanceGetFontSize(void)
{
    if (verbose) puts("STUB: CoreAppearanceGetFontSize called");
    return NULL;
}

void* CoreAppearanceGetQDFontForScript(void)
{
    if (verbose) puts("STUB: CoreAppearanceGetQDFontForScript called");
    return NULL;
}

void* CoreCursorCopyImages(void)
{
    if (verbose) puts("STUB: CoreCursorCopyImages called");
    return NULL;
}

void* CoreCursorGetData(void)
{
    if (verbose) puts("STUB: CoreCursorGetData called");
    return NULL;
}

void* CoreCursorGetDataSize(void)
{
    if (verbose) puts("STUB: CoreCursorGetDataSize called");
    return NULL;
}

void* CoreCursorSet(void)
{
    if (verbose) puts("STUB: CoreCursorSet called");
    return NULL;
}

void* CoreCursorSetAndReturnSeed(void)
{
    if (verbose) puts("STUB: CoreCursorSetAndReturnSeed called");
    return NULL;
}

void* CoreCursorUnregisterAll(void)
{
    if (verbose) puts("STUB: CoreCursorUnregisterAll called");
    return NULL;
}

void* CoreDockAddFileToDock(void)
{
    if (verbose) puts("STUB: CoreDockAddFileToDock called");
    return NULL;
}

void* CoreDockBounceAppTile(void)
{
    if (verbose) puts("STUB: CoreDockBounceAppTile called");
    return NULL;
}

void* CoreDockCompositeProcessImage(void)
{
    if (verbose) puts("STUB: CoreDockCompositeProcessImage called");
    return NULL;
}

void* CoreDockCopyPreferences(void)
{
    if (verbose) puts("STUB: CoreDockCopyPreferences called");
    return NULL;
}

void* CoreDockCopyWorkspacesAppBindings(void)
{
    if (verbose) puts("STUB: CoreDockCopyWorkspacesAppBindings called");
    return NULL;
}

void* CoreDockCreateDragTrashContext(void)
{
    if (verbose) puts("STUB: CoreDockCreateDragTrashContext called");
    return NULL;
}

void* CoreDockDisableExposeKeysIfNecessary(void)
{
    if (verbose) puts("STUB: CoreDockDisableExposeKeysIfNecessary called");
    return NULL;
}

void* CoreDockGetAutoHideEnabled(void)
{
    if (verbose) puts("STUB: CoreDockGetAutoHideEnabled called");
    return NULL;
}

void* CoreDockGetContainerRect(void)
{
    if (verbose) puts("STUB: CoreDockGetContainerRect called");
    return NULL;
}

void* CoreDockGetDashboardInDock(void)
{
    if (verbose) puts("STUB: CoreDockGetDashboardInDock called");
    return NULL;
}

void* CoreDockGetEffect(void)
{
    if (verbose) puts("STUB: CoreDockGetEffect called");
    return NULL;
}

void* CoreDockGetExposeCornerActions(void)
{
    if (verbose) puts("STUB: CoreDockGetExposeCornerActions called");
    return NULL;
}

void* CoreDockGetExposeCornerActionsWithModifiers(void)
{
    if (verbose) puts("STUB: CoreDockGetExposeCornerActionsWithModifiers called");
    return NULL;
}

void* CoreDockGetItemDockContext(void)
{
    if (verbose) puts("STUB: CoreDockGetItemDockContext called");
    return NULL;
}

void* CoreDockGetItemDockWindow(void)
{
    if (verbose) puts("STUB: CoreDockGetItemDockWindow called");
    return NULL;
}

void* CoreDockGetMagnificationSize(void)
{
    if (verbose) puts("STUB: CoreDockGetMagnificationSize called");
    return NULL;
}

void* CoreDockGetMinimizeInPlace(void)
{
    if (verbose) puts("STUB: CoreDockGetMinimizeInPlace called");
    return NULL;
}

void* CoreDockGetOrientationAndPinning(void)
{
    if (verbose) puts("STUB: CoreDockGetOrientationAndPinning called");
    return NULL;
}

void* CoreDockGetProcessContext(void)
{
    if (verbose) puts("STUB: CoreDockGetProcessContext called");
    return NULL;
}

void* CoreDockGetProcessWindow(void)
{
    if (verbose) puts("STUB: CoreDockGetProcessWindow called");
    return NULL;
}

void* CoreDockGetRect(void)
{
    if (verbose) puts("STUB: CoreDockGetRect called");
    return NULL;
}

void* CoreDockGetRectAndReason(void)
{
    if (verbose) puts("STUB: CoreDockGetRectAndReason called");
    return NULL;
}

void* CoreDockGetSpringLoadingTime(void)
{
    if (verbose) puts("STUB: CoreDockGetSpringLoadingTime called");
    return NULL;
}

void* CoreDockGetTileSize(void)
{
    if (verbose) puts("STUB: CoreDockGetTileSize called");
    return NULL;
}

void* CoreDockGetTrashWindow(void)
{
    if (verbose) puts("STUB: CoreDockGetTrashWindow called");
    return NULL;
}

void* CoreDockGetWorkspacesCount(void)
{
    if (verbose) puts("STUB: CoreDockGetWorkspacesCount called");
    return NULL;
}

void* CoreDockGetWorkspacesEnabled(void)
{
    if (verbose) puts("STUB: CoreDockGetWorkspacesEnabled called");
    return NULL;
}

void* CoreDockGetWorkspacesKeyBindings(void)
{
    if (verbose) puts("STUB: CoreDockGetWorkspacesKeyBindings called");
    return NULL;
}

void* CoreDockIsDockRunning(void)
{
    if (verbose) puts("STUB: CoreDockIsDockRunning called");
    return NULL;
}

void* CoreDockIsLaunchAnimationsEnabled(void)
{
    if (verbose) puts("STUB: CoreDockIsLaunchAnimationsEnabled called");
    return NULL;
}

void* CoreDockIsMagnificationEnabled(void)
{
    if (verbose) puts("STUB: CoreDockIsMagnificationEnabled called");
    return NULL;
}

void* CoreDockIsSpringLoadingEnabled(void)
{
    if (verbose) puts("STUB: CoreDockIsSpringLoadingEnabled called");
    return NULL;
}

void* CoreDockMinimizeItemWithTitle(void)
{
    if (verbose) puts("STUB: CoreDockMinimizeItemWithTitle called");
    return NULL;
}

void* CoreDockMinimizeItemWithTitleAsync(void)
{
    if (verbose) puts("STUB: CoreDockMinimizeItemWithTitleAsync called");
    return NULL;
}

void* CoreDockMinimizeItemsWithTitle(void)
{
    if (verbose) puts("STUB: CoreDockMinimizeItemsWithTitle called");
    return NULL;
}

void* CoreDockMinimizeItemsWithTitleAsync(void)
{
    if (verbose) puts("STUB: CoreDockMinimizeItemsWithTitleAsync called");
    return NULL;
}

void* CoreDockPreventCommunicationWithDock(void)
{
    if (verbose) puts("STUB: CoreDockPreventCommunicationWithDock called");
    return NULL;
}

void* CoreDockRegisterClientWithRunLoop(void)
{
    if (verbose) puts("STUB: CoreDockRegisterClientWithRunLoop called");
    return NULL;
}

void* CoreDockReleaseDragTrashContext(void)
{
    if (verbose) puts("STUB: CoreDockReleaseDragTrashContext called");
    return NULL;
}

void* CoreDockReleaseItemDockContext(void)
{
    if (verbose) puts("STUB: CoreDockReleaseItemDockContext called");
    return NULL;
}

void* CoreDockReleaseItemDockWindow(void)
{
    if (verbose) puts("STUB: CoreDockReleaseItemDockWindow called");
    return NULL;
}

void* CoreDockReleaseProcessContext(void)
{
    if (verbose) puts("STUB: CoreDockReleaseProcessContext called");
    return NULL;
}

void* CoreDockReleaseProcessWindow(void)
{
    if (verbose) puts("STUB: CoreDockReleaseProcessWindow called");
    return NULL;
}

void* CoreDockRemoveItem(void)
{
    if (verbose) puts("STUB: CoreDockRemoveItem called");
    return NULL;
}

void* CoreDockRenderWindowIntoContext(void)
{
    if (verbose) puts("STUB: CoreDockRenderWindowIntoContext called");
    return NULL;
}

void* CoreDockRestoreItem(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItem called");
    return NULL;
}

void* CoreDockRestoreItemAsync(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItemAsync called");
    return NULL;
}

void* CoreDockRestoreItemWithOrder(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItemWithOrder called");
    return NULL;
}

void* CoreDockRestoreItemWithOrderAsync(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItemWithOrderAsync called");
    return NULL;
}

void* CoreDockRestoreItems(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItems called");
    return NULL;
}

void* CoreDockRestoreItemsAsync(void)
{
    if (verbose) puts("STUB: CoreDockRestoreItemsAsync called");
    return NULL;
}

void* CoreDockRestoreProcessImage(void)
{
    if (verbose) puts("STUB: CoreDockRestoreProcessImage called");
    return NULL;
}

void* CoreDockRevealWindowForShowDesktop(void)
{
    if (verbose) puts("STUB: CoreDockRevealWindowForShowDesktop called");
    return NULL;
}

void* CoreDockSendDragWindowMessage(void)
{
    if (verbose) puts("STUB: CoreDockSendDragWindowMessage called");
    return NULL;
}

void* CoreDockSendNotification(void)
{
    if (verbose) puts("STUB: CoreDockSendNotification called");
    return NULL;
}

void* CoreDockSetAutoHideEnabled(void)
{
    if (verbose) puts("STUB: CoreDockSetAutoHideEnabled called");
    return NULL;
}

void* CoreDockSetDashboardInDock(void)
{
    if (verbose) puts("STUB: CoreDockSetDashboardInDock called");
    return NULL;
}

void* CoreDockSetEffect(void)
{
    if (verbose) puts("STUB: CoreDockSetEffect called");
    return NULL;
}

void* CoreDockSetExposeCornerAction(void)
{
    if (verbose) puts("STUB: CoreDockSetExposeCornerAction called");
    return NULL;
}

void* CoreDockSetExposeCornerActionWithModifier(void)
{
    if (verbose) puts("STUB: CoreDockSetExposeCornerActionWithModifier called");
    return NULL;
}

void* CoreDockSetItemTitle(void)
{
    if (verbose) puts("STUB: CoreDockSetItemTitle called");
    return NULL;
}

void* CoreDockSetLaunchAnimationsEnabled(void)
{
    if (verbose) puts("STUB: CoreDockSetLaunchAnimationsEnabled called");
    return NULL;
}

void* CoreDockSetMagnificationEnabled(void)
{
    if (verbose) puts("STUB: CoreDockSetMagnificationEnabled called");
    return NULL;
}

void* CoreDockSetMagnificationSize(void)
{
    if (verbose) puts("STUB: CoreDockSetMagnificationSize called");
    return NULL;
}

void* CoreDockSetMiniView(void)
{
    if (verbose) puts("STUB: CoreDockSetMiniView called");
    return NULL;
}

void* CoreDockSetMinimizeInPlace(void)
{
    if (verbose) puts("STUB: CoreDockSetMinimizeInPlace called");
    return NULL;
}

void* CoreDockSetOrientationAndPinning(void)
{
    if (verbose) puts("STUB: CoreDockSetOrientationAndPinning called");
    return NULL;
}

void* CoreDockSetPreferences(void)
{
    if (verbose) puts("STUB: CoreDockSetPreferences called");
    return NULL;
}

void* CoreDockSetProcessImage(void)
{
    if (verbose) puts("STUB: CoreDockSetProcessImage called");
    return NULL;
}

void* CoreDockSetProcessLabel(void)
{
    if (verbose) puts("STUB: CoreDockSetProcessLabel called");
    return NULL;
}

void* CoreDockSetProcessOpenRecents(void)
{
    if (verbose) puts("STUB: CoreDockSetProcessOpenRecents called");
    return NULL;
}

void* CoreDockSetShowDesktopCallback(void)
{
    if (verbose) puts("STUB: CoreDockSetShowDesktopCallback called");
    return NULL;
}

void* CoreDockSetSpringLoadingEnabled(void)
{
    if (verbose) puts("STUB: CoreDockSetSpringLoadingEnabled called");
    return NULL;
}

void* CoreDockSetSpringLoadingTime(void)
{
    if (verbose) puts("STUB: CoreDockSetSpringLoadingTime called");
    return NULL;
}

void* CoreDockSetSpringWindowCallbacks(void)
{
    if (verbose) puts("STUB: CoreDockSetSpringWindowCallbacks called");
    return NULL;
}

void* CoreDockSetTileSize(void)
{
    if (verbose) puts("STUB: CoreDockSetTileSize called");
    return NULL;
}

void* CoreDockSetTrashFull(void)
{
    if (verbose) puts("STUB: CoreDockSetTrashFull called");
    return NULL;
}

void* CoreDockSetWindowLabel(void)
{
    if (verbose) puts("STUB: CoreDockSetWindowLabel called");
    return NULL;
}

void* CoreDockSetWorkspacesAppBindings(void)
{
    if (verbose) puts("STUB: CoreDockSetWorkspacesAppBindings called");
    return NULL;
}

void* CoreDockSetWorkspacesCount(void)
{
    if (verbose) puts("STUB: CoreDockSetWorkspacesCount called");
    return NULL;
}

void* CoreDockSetWorkspacesEnabled(void)
{
    if (verbose) puts("STUB: CoreDockSetWorkspacesEnabled called");
    return NULL;
}

void* CoreDockSetWorkspacesKeyBindings(void)
{
    if (verbose) puts("STUB: CoreDockSetWorkspacesKeyBindings called");
    return NULL;
}

void* CoreDockUpdateWindow(void)
{
    if (verbose) puts("STUB: CoreDockUpdateWindow called");
    return NULL;
}

void* CoreDragCancelDrag(void)
{
    if (verbose) puts("STUB: CoreDragCancelDrag called");
    return NULL;
}

void* CoreDragChangeBehaviors(void)
{
    if (verbose) puts("STUB: CoreDragChangeBehaviors called");
    return NULL;
}

void* CoreDragCleanDragState(void)
{
    if (verbose) puts("STUB: CoreDragCleanDragState called");
    return NULL;
}

void* CoreDragClearAllImageOverrides(void)
{
    if (verbose) puts("STUB: CoreDragClearAllImageOverrides called");
    return NULL;
}

void* CoreDragCopyTrashLabel(void)
{
    if (verbose) puts("STUB: CoreDragCopyTrashLabel called");
    return NULL;
}

void* CoreDragCreate(void)
{
    if (verbose) puts("STUB: CoreDragCreate called");
    return NULL;
}

void* CoreDragCreateWithPasteboard(void)
{
    if (verbose) puts("STUB: CoreDragCreateWithPasteboard called");
    return NULL;
}

void* CoreDragCreateWithPasteboardRef(void)
{
    if (verbose) puts("STUB: CoreDragCreateWithPasteboardRef called");
    return NULL;
}

void* CoreDragDispose(void)
{
    if (verbose) puts("STUB: CoreDragDispose called");
    return NULL;
}

void* CoreDragEnableSpringLoading(void)
{
    if (verbose) puts("STUB: CoreDragEnableSpringLoading called");
    return NULL;
}

void* CoreDragGetAllowableActions(void)
{
    if (verbose) puts("STUB: CoreDragGetAllowableActions called");
    return NULL;
}

void* CoreDragGetAttributes(void)
{
    if (verbose) puts("STUB: CoreDragGetAttributes called");
    return NULL;
}

void* CoreDragGetCurrentDrag(void)
{
    if (verbose) puts("STUB: CoreDragGetCurrentDrag called");
    return NULL;
}

void* CoreDragGetDragWindow(void)
{
    if (verbose) puts("STUB: CoreDragGetDragWindow called");
    return NULL;
}

void* CoreDragGetDropActions(void)
{
    if (verbose) puts("STUB: CoreDragGetDropActions called");
    return NULL;
}

void* CoreDragGetDropLocation(void)
{
    if (verbose) puts("STUB: CoreDragGetDropLocation called");
    return NULL;
}

void* CoreDragGetForce(void)
{
    if (verbose) puts("STUB: CoreDragGetForce called");
    return NULL;
}

void* CoreDragGetForceStage(void)
{
    if (verbose) puts("STUB: CoreDragGetForceStage called");
    return NULL;
}

void* CoreDragGetItemBounds(void)
{
    if (verbose) puts("STUB: CoreDragGetItemBounds called");
    return NULL;
}

void* CoreDragGetModifiers(void)
{
    if (verbose) puts("STUB: CoreDragGetModifiers called");
    return NULL;
}

void* CoreDragGetMouseLocation(void)
{
    if (verbose) puts("STUB: CoreDragGetMouseLocation called");
    return NULL;
}

void* CoreDragGetOrigin(void)
{
    if (verbose) puts("STUB: CoreDragGetOrigin called");
    return NULL;
}

void* CoreDragGetPasteboard(void)
{
    if (verbose) puts("STUB: CoreDragGetPasteboard called");
    return NULL;
}

void* CoreDragGetPasteboardRef(void)
{
    if (verbose) puts("STUB: CoreDragGetPasteboardRef called");
    return NULL;
}

void* CoreDragGetSpringLoadingAttributes(void)
{
    if (verbose) puts("STUB: CoreDragGetSpringLoadingAttributes called");
    return NULL;
}

void* CoreDragGetStandardDropLocation(void)
{
    if (verbose) puts("STUB: CoreDragGetStandardDropLocation called");
    return NULL;
}

void* CoreDragGetValueForKey(void)
{
    if (verbose) puts("STUB: CoreDragGetValueForKey called");
    return NULL;
}

void* CoreDragHasImageOverrides(void)
{
    if (verbose) puts("STUB: CoreDragHasImageOverrides called");
    return NULL;
}

void* CoreDragInstallContextReceiveMessageHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragInstallContextReceiveMessageHandlerOnConnection called");
    return NULL;
}

void* CoreDragInstallContextTrackingHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragInstallContextTrackingHandlerOnConnection called");
    return NULL;
}

void* CoreDragInstallReceiveHandler(void)
{
    if (verbose) puts("STUB: CoreDragInstallReceiveHandler called");
    return NULL;
}

void* CoreDragInstallReceiveHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragInstallReceiveHandlerOnConnection called");
    return NULL;
}

void* CoreDragInstallReceiveMessageHandler(void)
{
    if (verbose) puts("STUB: CoreDragInstallReceiveMessageHandler called");
    return NULL;
}

void* CoreDragInstallReceiveMessageHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragInstallReceiveMessageHandlerOnConnection called");
    return NULL;
}

void* CoreDragInstallTrackingHandler(void)
{
    if (verbose) puts("STUB: CoreDragInstallTrackingHandler called");
    return NULL;
}

void* CoreDragInstallTrackingHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragInstallTrackingHandlerOnConnection called");
    return NULL;
}

void* CoreDragIsTrashLabelSet(void)
{
    if (verbose) puts("STUB: CoreDragIsTrashLabelSet called");
    return NULL;
}

void* CoreDragItemGetImageComponents(void)
{
    if (verbose) puts("STUB: CoreDragItemGetImageComponents called");
    return NULL;
}

void* CoreDragItemGetScreenFrame(void)
{
    if (verbose) puts("STUB: CoreDragItemGetScreenFrame called");
    return NULL;
}

void* CoreDragItemHasAlternate(void)
{
    if (verbose) puts("STUB: CoreDragItemHasAlternate called");
    return NULL;
}

void* CoreDragItemSetCGImage(void)
{
    if (verbose) puts("STUB: CoreDragItemSetCGImage called");
    return NULL;
}

void* CoreDragItemSetImage(void)
{
    if (verbose) puts("STUB: CoreDragItemSetImage called");
    return NULL;
}

void* CoreDragItemSetImageComponents(void)
{
    if (verbose) puts("STUB: CoreDragItemSetImageComponents called");
    return NULL;
}

void* CoreDragRefSetImageDataForItem(void)
{
    if (verbose) puts("STUB: CoreDragRefSetImageDataForItem called");
    return NULL;
}

void* CoreDragRegisterClient(void)
{
    if (verbose) puts("STUB: CoreDragRegisterClient called");
    return NULL;
}

void* CoreDragRegisterClientInModes(void)
{
    if (verbose) puts("STUB: CoreDragRegisterClientInModes called");
    return NULL;
}

void* CoreDragRegisterClientWithOptions(void)
{
    if (verbose) puts("STUB: CoreDragRegisterClientWithOptions called");
    return NULL;
}

void* CoreDragReleaseImageData(void)
{
    if (verbose) puts("STUB: CoreDragReleaseImageData called");
    return NULL;
}

void* CoreDragRemoveReceiveHandler(void)
{
    if (verbose) puts("STUB: CoreDragRemoveReceiveHandler called");
    return NULL;
}

void* CoreDragRemoveReceiveHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragRemoveReceiveHandlerOnConnection called");
    return NULL;
}

void* CoreDragRemoveReceiveMessageHandler(void)
{
    if (verbose) puts("STUB: CoreDragRemoveReceiveMessageHandler called");
    return NULL;
}

void* CoreDragRemoveReceiveMessageHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragRemoveReceiveMessageHandlerOnConnection called");
    return NULL;
}

void* CoreDragRemoveTrackingHandler(void)
{
    if (verbose) puts("STUB: CoreDragRemoveTrackingHandler called");
    return NULL;
}

void* CoreDragRemoveTrackingHandlerOnConnection(void)
{
    if (verbose) puts("STUB: CoreDragRemoveTrackingHandlerOnConnection called");
    return NULL;
}

void* CoreDragRequestDragCompleteMessage(void)
{
    if (verbose) puts("STUB: CoreDragRequestDragCompleteMessage called");
    return NULL;
}

void* CoreDragSetAllowableActions(void)
{
    if (verbose) puts("STUB: CoreDragSetAllowableActions called");
    return NULL;
}

void* CoreDragSetAttribute(void)
{
    if (verbose) puts("STUB: CoreDragSetAttribute called");
    return NULL;
}

void* CoreDragSetCGEventInputProc(void)
{
    if (verbose) puts("STUB: CoreDragSetCGEventInputProc called");
    return NULL;
}

void* CoreDragSetCGEventProcs(void)
{
    if (verbose) puts("STUB: CoreDragSetCGEventProcs called");
    return NULL;
}

void* CoreDragSetCGImage(void)
{
    if (verbose) puts("STUB: CoreDragSetCGImage called");
    return NULL;
}

void* CoreDragSetCGImageWithScale(void)
{
    if (verbose) puts("STUB: CoreDragSetCGImageWithScale called");
    return NULL;
}

void* CoreDragSetDestClippingRect(void)
{
    if (verbose) puts("STUB: CoreDragSetDestClippingRect called");
    return NULL;
}

void* CoreDragSetDragRegion(void)
{
    if (verbose) puts("STUB: CoreDragSetDragRegion called");
    return NULL;
}

void* CoreDragSetDragRegionWithScale(void)
{
    if (verbose) puts("STUB: CoreDragSetDragRegionWithScale called");
    return NULL;
}

void* CoreDragSetDragWindow(void)
{
    if (verbose) puts("STUB: CoreDragSetDragWindow called");
    return NULL;
}

void* CoreDragSetDropActions(void)
{
    if (verbose) puts("STUB: CoreDragSetDropActions called");
    return NULL;
}

void* CoreDragSetDropLocation(void)
{
    if (verbose) puts("STUB: CoreDragSetDropLocation called");
    return NULL;
}

void* CoreDragSetEventProc(void)
{
    if (verbose) puts("STUB: CoreDragSetEventProc called");
    return NULL;
}

void* CoreDragSetExtendedEventProc(void)
{
    if (verbose) puts("STUB: CoreDragSetExtendedEventProc called");
    return NULL;
}

void* CoreDragSetImage(void)
{
    if (verbose) puts("STUB: CoreDragSetImage called");
    return NULL;
}

void* CoreDragSetImageOptions(void)
{
    if (verbose) puts("STUB: CoreDragSetImageOptions called");
    return NULL;
}

void* CoreDragSetInputProc(void)
{
    if (verbose) puts("STUB: CoreDragSetInputProc called");
    return NULL;
}

void* CoreDragSetItemBounds(void)
{
    if (verbose) puts("STUB: CoreDragSetItemBounds called");
    return NULL;
}

void* CoreDragSetMouseLocation(void)
{
    if (verbose) puts("STUB: CoreDragSetMouseLocation called");
    return NULL;
}

void* CoreDragSetOrigin(void)
{
    if (verbose) puts("STUB: CoreDragSetOrigin called");
    return NULL;
}

void* CoreDragSetRootCALayer(void)
{
    if (verbose) puts("STUB: CoreDragSetRootCALayer called");
    return NULL;
}

void* CoreDragSetSourceClippingRect(void)
{
    if (verbose) puts("STUB: CoreDragSetSourceClippingRect called");
    return NULL;
}

void* CoreDragSetStandardDropLocation(void)
{
    if (verbose) puts("STUB: CoreDragSetStandardDropLocation called");
    return NULL;
}

void* CoreDragSetTrashDrawProc(void)
{
    if (verbose) puts("STUB: CoreDragSetTrashDrawProc called");
    return NULL;
}

void* CoreDragSetTrashLabel(void)
{
    if (verbose) puts("STUB: CoreDragSetTrashLabel called");
    return NULL;
}

void* CoreDragSetValueForKey(void)
{
    if (verbose) puts("STUB: CoreDragSetValueForKey called");
    return NULL;
}

void* CoreDragStartDragging(void)
{
    if (verbose) puts("STUB: CoreDragStartDragging called");
    return NULL;
}

void* CoreDragUpdatesBegin(void)
{
    if (verbose) puts("STUB: CoreDragUpdatesBegin called");
    return NULL;
}

void* CoreDragUpdatesCommit(void)
{
    if (verbose) puts("STUB: CoreDragUpdatesCommit called");
    return NULL;
}

void* CoreGetDragInfo(void)
{
    if (verbose) puts("STUB: CoreGetDragInfo called");
    return NULL;
}

void* CoreImagingCreateImage(void)
{
    if (verbose) puts("STUB: CoreImagingCreateImage called");
    return NULL;
}

void* CoreMenuCreateKeyEquivalentString(void)
{
    if (verbose) puts("STUB: CoreMenuCreateKeyEquivalentString called");
    return NULL;
}

void* CoreMenuCreateVirtualKeyString(void)
{
    if (verbose) puts("STUB: CoreMenuCreateVirtualKeyString called");
    return NULL;
}

void* CoreMenuExtraAddMenuExtra(void)
{
    if (verbose) puts("STUB: CoreMenuExtraAddMenuExtra called");
    return NULL;
}

void* CoreMenuExtraGetMenuExtra(void)
{
    if (verbose) puts("STUB: CoreMenuExtraGetMenuExtra called");
    return NULL;
}

void* CoreMenuExtraRemoveMenuExtra(void)
{
    if (verbose) puts("STUB: CoreMenuExtraRemoveMenuExtra called");
    return NULL;
}

void* CoreMenuGetVirtualKeyMap(void)
{
    if (verbose) puts("STUB: CoreMenuGetVirtualKeyMap called");
    return NULL;
}

void* CreatePasteboardFlavorTypeName(void)
{
    if (verbose) puts("STUB: CreatePasteboardFlavorTypeName called");
    return NULL;
}

void* DesktopPictureCopyDisplay(void)
{
    if (verbose) puts("STUB: DesktopPictureCopyDisplay called");
    return NULL;
}

void* DesktopPictureCopyDisplayForSpace(void)
{
    if (verbose) puts("STUB: DesktopPictureCopyDisplayForSpace called");
    return NULL;
}

void* DesktopPictureCopyValue(void)
{
    if (verbose) puts("STUB: DesktopPictureCopyValue called");
    return NULL;
}

void* DesktopPictureSetDisplay(void)
{
    if (verbose) puts("STUB: DesktopPictureSetDisplay called");
    return NULL;
}

void* DesktopPictureSetDisplayForSpace(void)
{
    if (verbose) puts("STUB: DesktopPictureSetDisplayForSpace called");
    return NULL;
}

void* DesktopPictureSetValue(void)
{
    if (verbose) puts("STUB: DesktopPictureSetValue called");
    return NULL;
}

void* GetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void)
{
    if (verbose) puts("STUB: GetGlobalIconImagesCacheMaxEntriesAndMaxDataSize called");
    return NULL;
}

void* GetIconFamilyData(void)
{
    if (verbose) puts("STUB: GetIconFamilyData called");
    return NULL;
}

void* GetIconRefVariant(void)
{
    if (verbose) puts("STUB: GetIconRefVariant called");
    return NULL;
}

void* HIS_XPC_CFNotificationCenterPostNotification(void)
{
    if (verbose) puts("STUB: HIS_XPC_CFNotificationCenterPostNotification called");
    return NULL;
}

void* HIS_XPC_CFPreferencesCopyValue(void)
{
    if (verbose) puts("STUB: HIS_XPC_CFPreferencesCopyValue called");
    return NULL;
}

void* HIS_XPC_CFPreferencesSetValue(void)
{
    if (verbose) puts("STUB: HIS_XPC_CFPreferencesSetValue called");
    return NULL;
}

void* HIS_XPC_CFPreferencesSynchronize(void)
{
    if (verbose) puts("STUB: HIS_XPC_CFPreferencesSynchronize called");
    return NULL;
}

void* HIS_XPC_CopyMacManagerPrefs(void)
{
    if (verbose) puts("STUB: HIS_XPC_CopyMacManagerPrefs called");
    return NULL;
}

void* HIS_XPC_GetCapsLockModifierState(void)
{
    if (verbose) puts("STUB: HIS_XPC_GetCapsLockModifierState called");
    return NULL;
}

void* HIS_XPC_PostDeleteKeyEvent(void)
{
    if (verbose) puts("STUB: HIS_XPC_PostDeleteKeyEvent called");
    return NULL;
}

void* HIS_XPC_RevealFileInFinder(void)
{
    if (verbose) puts("STUB: HIS_XPC_RevealFileInFinder called");
    return NULL;
}

void* HIS_XPC_SendAppleEventToSystemProcess(void)
{
    if (verbose) puts("STUB: HIS_XPC_SendAppleEventToSystemProcess called");
    return NULL;
}

void* HIS_XPC_SetCapsLockDelayOverride(void)
{
    if (verbose) puts("STUB: HIS_XPC_SetCapsLockDelayOverride called");
    return NULL;
}

void* HIS_XPC_SetCapsLockLED(void)
{
    if (verbose) puts("STUB: HIS_XPC_SetCapsLockLED called");
    return NULL;
}

void* HIS_XPC_SetCapsLockLEDInhibit(void)
{
    if (verbose) puts("STUB: HIS_XPC_SetCapsLockLEDInhibit called");
    return NULL;
}

void* HIS_XPC_SetCapsLockModifierState(void)
{
    if (verbose) puts("STUB: HIS_XPC_SetCapsLockModifierState called");
    return NULL;
}

void* HIS_XPC_SetNetworkLocation(void)
{
    if (verbose) puts("STUB: HIS_XPC_SetNetworkLocation called");
    return NULL;
}

void* HIShapeContainsPoint(void)
{
    if (verbose) puts("STUB: HIShapeContainsPoint called");
    return NULL;
}

void* HIShapeCreateCopy(void)
{
    if (verbose) puts("STUB: HIShapeCreateCopy called");
    return NULL;
}

void* HIShapeCreateCopyAsQDRgn(void)
{
    if (verbose) puts("STUB: HIShapeCreateCopyAsQDRgn called");
    return NULL;
}

void* HIShapeCreateDifference(void)
{
    if (verbose) puts("STUB: HIShapeCreateDifference called");
    return NULL;
}

void* HIShapeCreateEmpty(void)
{
    if (verbose) puts("STUB: HIShapeCreateEmpty called");
    return NULL;
}

void* HIShapeCreateIntersection(void)
{
    if (verbose) puts("STUB: HIShapeCreateIntersection called");
    return NULL;
}

void* HIShapeCreateMutable(void)
{
    if (verbose) puts("STUB: HIShapeCreateMutable called");
    return NULL;
}

void* HIShapeCreateMutableCopy(void)
{
    if (verbose) puts("STUB: HIShapeCreateMutableCopy called");
    return NULL;
}

void* HIShapeCreateMutableWithRect(void)
{
    if (verbose) puts("STUB: HIShapeCreateMutableWithRect called");
    return NULL;
}

void* HIShapeCreateUnion(void)
{
    if (verbose) puts("STUB: HIShapeCreateUnion called");
    return NULL;
}

void* HIShapeCreateWithQDRgn(void)
{
    if (verbose) puts("STUB: HIShapeCreateWithQDRgn called");
    return NULL;
}

void* HIShapeCreateWithRect(void)
{
    if (verbose) puts("STUB: HIShapeCreateWithRect called");
    return NULL;
}

void* HIShapeCreateXor(void)
{
    if (verbose) puts("STUB: HIShapeCreateXor called");
    return NULL;
}

void* HIShapeDifference(void)
{
    if (verbose) puts("STUB: HIShapeDifference called");
    return NULL;
}

void* HIShapeEnumerate(void)
{
    if (verbose) puts("STUB: HIShapeEnumerate called");
    return NULL;
}

void* HIShapeGetAsQDRgn(void)
{
    if (verbose) puts("STUB: HIShapeGetAsQDRgn called");
    return NULL;
}

void* HIShapeGetBounds(void)
{
    if (verbose) puts("STUB: HIShapeGetBounds called");
    return NULL;
}

void* HIShapeGetTypeID(void)
{
    if (verbose) puts("STUB: HIShapeGetTypeID called");
    return NULL;
}

void* HIShapeInset(void)
{
    if (verbose) puts("STUB: HIShapeInset called");
    return NULL;
}

void* HIShapeIntersect(void)
{
    if (verbose) puts("STUB: HIShapeIntersect called");
    return NULL;
}

void* HIShapeIntersectsRect(void)
{
    if (verbose) puts("STUB: HIShapeIntersectsRect called");
    return NULL;
}

void* HIShapeIsEmpty(void)
{
    if (verbose) puts("STUB: HIShapeIsEmpty called");
    return NULL;
}

void* HIShapeIsRectangular(void)
{
    if (verbose) puts("STUB: HIShapeIsRectangular called");
    return NULL;
}

void* HIShapeOffset(void)
{
    if (verbose) puts("STUB: HIShapeOffset called");
    return NULL;
}

void* HIShapeReplacePathInCGContext(void)
{
    if (verbose) puts("STUB: HIShapeReplacePathInCGContext called");
    return NULL;
}

void* HIShapeSetEmpty(void)
{
    if (verbose) puts("STUB: HIShapeSetEmpty called");
    return NULL;
}

void* HIShapeSetWithShape(void)
{
    if (verbose) puts("STUB: HIShapeSetWithShape called");
    return NULL;
}

void* HIShapeUnion(void)
{
    if (verbose) puts("STUB: HIShapeUnion called");
    return NULL;
}

void* HIShapeUnionWithRect(void)
{
    if (verbose) puts("STUB: HIShapeUnionWithRect called");
    return NULL;
}

void* HIShapeXor(void)
{
    if (verbose) puts("STUB: HIShapeXor called");
    return NULL;
}

void* ICAddMapEntry(void)
{
    if (verbose) puts("STUB: ICAddMapEntry called");
    return NULL;
}

void* ICAddProfile(void)
{
    if (verbose) puts("STUB: ICAddProfile called");
    return NULL;
}

void* ICBegin(void)
{
    if (verbose) puts("STUB: ICBegin called");
    return NULL;
}

void* ICCountMapEntries(void)
{
    if (verbose) puts("STUB: ICCountMapEntries called");
    return NULL;
}

void* ICCountPref(void)
{
    if (verbose) puts("STUB: ICCountPref called");
    return NULL;
}

void* ICCountProfiles(void)
{
    if (verbose) puts("STUB: ICCountProfiles called");
    return NULL;
}

void* ICCreateGURLEvent(void)
{
    if (verbose) puts("STUB: ICCreateGURLEvent called");
    return NULL;
}

void* ICDeleteMapEntry(void)
{
    if (verbose) puts("STUB: ICDeleteMapEntry called");
    return NULL;
}

void* ICDeletePref(void)
{
    if (verbose) puts("STUB: ICDeletePref called");
    return NULL;
}

void* ICDeleteProfile(void)
{
    if (verbose) puts("STUB: ICDeleteProfile called");
    return NULL;
}

void* ICEditPreferences(void)
{
    if (verbose) puts("STUB: ICEditPreferences called");
    return NULL;
}

void* ICEnd(void)
{
    if (verbose) puts("STUB: ICEnd called");
    return NULL;
}

void* ICFindPrefHandle(void)
{
    if (verbose) puts("STUB: ICFindPrefHandle called");
    return NULL;
}

void* ICGetConfigName(void)
{
    if (verbose) puts("STUB: ICGetConfigName called");
    return NULL;
}

void* ICGetCurrentProfile(void)
{
    if (verbose) puts("STUB: ICGetCurrentProfile called");
    return NULL;
}

void* ICGetDefaultPref(void)
{
    if (verbose) puts("STUB: ICGetDefaultPref called");
    return NULL;
}

void* ICGetIndMapEntry(void)
{
    if (verbose) puts("STUB: ICGetIndMapEntry called");
    return NULL;
}

void* ICGetIndPref(void)
{
    if (verbose) puts("STUB: ICGetIndPref called");
    return NULL;
}

void* ICGetIndProfile(void)
{
    if (verbose) puts("STUB: ICGetIndProfile called");
    return NULL;
}

void* ICGetMapEntry(void)
{
    if (verbose) puts("STUB: ICGetMapEntry called");
    return NULL;
}

void* ICGetPerm(void)
{
    if (verbose) puts("STUB: ICGetPerm called");
    return NULL;
}

void* ICGetPref(void)
{
    if (verbose) puts("STUB: ICGetPref called");
    return NULL;
}

void* ICGetPrefHandle(void)
{
    if (verbose) puts("STUB: ICGetPrefHandle called");
    return NULL;
}

void* ICGetProfileName(void)
{
    if (verbose) puts("STUB: ICGetProfileName called");
    return NULL;
}

void* ICGetSeed(void)
{
    if (verbose) puts("STUB: ICGetSeed called");
    return NULL;
}

void* ICGetVersion(void)
{
    if (verbose) puts("STUB: ICGetVersion called");
    return NULL;
}

void* ICLaunchURL(void)
{
    if (verbose) puts("STUB: ICLaunchURL called");
    return NULL;
}

void* ICMapEntriesFilename(void)
{
    if (verbose) puts("STUB: ICMapEntriesFilename called");
    return NULL;
}

void* ICMapEntriesTypeCreator(void)
{
    if (verbose) puts("STUB: ICMapEntriesTypeCreator called");
    return NULL;
}

void* ICMapFilename(void)
{
    if (verbose) puts("STUB: ICMapFilename called");
    return NULL;
}

void* ICMapTypeCreator(void)
{
    if (verbose) puts("STUB: ICMapTypeCreator called");
    return NULL;
}

void* ICParseURL(void)
{
    if (verbose) puts("STUB: ICParseURL called");
    return NULL;
}

void* ICSendGURLEvent(void)
{
    if (verbose) puts("STUB: ICSendGURLEvent called");
    return NULL;
}

void* ICSetCurrentProfile(void)
{
    if (verbose) puts("STUB: ICSetCurrentProfile called");
    return NULL;
}

void* ICSetMapEntry(void)
{
    if (verbose) puts("STUB: ICSetMapEntry called");
    return NULL;
}

void* ICSetPref(void)
{
    if (verbose) puts("STUB: ICSetPref called");
    return NULL;
}

void* ICSetPrefHandle(void)
{
    if (verbose) puts("STUB: ICSetPrefHandle called");
    return NULL;
}

void* ICSetProfileName(void)
{
    if (verbose) puts("STUB: ICSetProfileName called");
    return NULL;
}

void* ICStart(void)
{
    if (verbose) puts("STUB: ICStart called");
    return NULL;
}

void* ICStop(void)
{
    if (verbose) puts("STUB: ICStop called");
    return NULL;
}

void* IconRefContainsCGPoint(void)
{
    if (verbose) puts("STUB: IconRefContainsCGPoint called");
    return NULL;
}

void* IconRefIntersectsCGRect(void)
{
    if (verbose) puts("STUB: IconRefIntersectsCGRect called");
    return NULL;
}

void* IconRefToHIShape(void)
{
    if (verbose) puts("STUB: IconRefToHIShape called");
    return NULL;
}

void* IconRefToIconFamily(void)
{
    if (verbose) puts("STUB: IconRefToIconFamily called");
    return NULL;
}

void* IsIconRefMaskEmpty(void)
{
    if (verbose) puts("STUB: IsIconRefMaskEmpty called");
    return NULL;
}

void* IsProcessManagerInitialized(void)
{
    if (verbose) puts("STUB: IsProcessManagerInitialized called");
    return NULL;
}

void* LaunchProcess(void)
{
    if (verbose) puts("STUB: LaunchProcess called");
    return NULL;
}

void* LaunchProcessAsync(void)
{
    if (verbose) puts("STUB: LaunchProcessAsync called");
    return NULL;
}

void* MSHCreateMIGServerSource(void)
{
    if (verbose) puts("STUB: MSHCreateMIGServerSource called");
    return NULL;
}

void* MSHCreateMachServerSource(void)
{
    if (verbose) puts("STUB: MSHCreateMachServerSource called");
    return NULL;
}

void* MSHGetMachPortFromSource(void)
{
    if (verbose) puts("STUB: MSHGetMachPortFromSource called");
    return NULL;
}

void* MSHMIGSourceSetNoSendersCallback(void)
{
    if (verbose) puts("STUB: MSHMIGSourceSetNoSendersCallback called");
    return NULL;
}

void* MSHMIGSourceSetSendOnceCallback(void)
{
    if (verbose) puts("STUB: MSHMIGSourceSetSendOnceCallback called");
    return NULL;
}

void* PasteboardClear(void)
{
    if (verbose) puts("STUB: PasteboardClear called");
    return NULL;
}

void* PasteboardCopyItemFlavorData(void)
{
    if (verbose) puts("STUB: PasteboardCopyItemFlavorData called");
    return NULL;
}

void* PasteboardCopyItemFlavors(void)
{
    if (verbose) puts("STUB: PasteboardCopyItemFlavors called");
    return NULL;
}

void* PasteboardCopyName(void)
{
    if (verbose) puts("STUB: PasteboardCopyName called");
    return NULL;
}

void* PasteboardCopyPasteLocation(void)
{
    if (verbose) puts("STUB: PasteboardCopyPasteLocation called");
    return NULL;
}

void* PasteboardCreate(void)
{
    if (verbose) puts("STUB: PasteboardCreate called");
    return NULL;
}

void* PasteboardGetCFPasteboard(void)
{
    if (verbose) puts("STUB: PasteboardGetCFPasteboard called");
    return NULL;
}

void* PasteboardGetItemCount(void)
{
    if (verbose) puts("STUB: PasteboardGetItemCount called");
    return NULL;
}

void* PasteboardGetItemFlavorFlags(void)
{
    if (verbose) puts("STUB: PasteboardGetItemFlavorFlags called");
    return NULL;
}

void* PasteboardGetItemIdentifier(void)
{
    if (verbose) puts("STUB: PasteboardGetItemIdentifier called");
    return NULL;
}

void* PasteboardGetTypeID(void)
{
    if (verbose) puts("STUB: PasteboardGetTypeID called");
    return NULL;
}

void* PasteboardPutItemFlavor(void)
{
    if (verbose) puts("STUB: PasteboardPutItemFlavor called");
    return NULL;
}

void* PasteboardResolvePromises(void)
{
    if (verbose) puts("STUB: PasteboardResolvePromises called");
    return NULL;
}

void* PasteboardSetPasteLocation(void)
{
    if (verbose) puts("STUB: PasteboardSetPasteLocation called");
    return NULL;
}

void* PasteboardSetPromiseKeeper(void)
{
    if (verbose) puts("STUB: PasteboardSetPromiseKeeper called");
    return NULL;
}

void* PasteboardSynchronize(void)
{
    if (verbose) puts("STUB: PasteboardSynchronize called");
    return NULL;
}

void* PasteboardToggleDuplicateFlavorCheck(void)
{
    if (verbose) puts("STUB: PasteboardToggleDuplicateFlavorCheck called");
    return NULL;
}

void* PlotIconRefInContext(void)
{
    if (verbose) puts("STUB: PlotIconRefInContext called");
    return NULL;
}

void* SXArbitrationAddQueuedOutputRequest(void)
{
    if (verbose) puts("STUB: SXArbitrationAddQueuedOutputRequest called");
    return NULL;
}

void* SXArbitrationCancelQueuedRequest(void)
{
    if (verbose) puts("STUB: SXArbitrationCancelQueuedRequest called");
    return NULL;
}

void* SXArbitrationCreateServerSource(void)
{
    if (verbose) puts("STUB: SXArbitrationCreateServerSource called");
    return NULL;
}

void* SXArbitrationIsQueuedRequestPending(void)
{
    if (verbose) puts("STUB: SXArbitrationIsQueuedRequestPending called");
    return NULL;
}

void* SXArbitrationRegisterOutputStarting(void)
{
    if (verbose) puts("STUB: SXArbitrationRegisterOutputStarting called");
    return NULL;
}

void* SXArbitrationRegisterOutputStopped(void)
{
    if (verbose) puts("STUB: SXArbitrationRegisterOutputStopped called");
    return NULL;
}

void* SerializeCFType(void)
{
    if (verbose) puts("STUB: SerializeCFType called");
    return NULL;
}

void* SetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void)
{
    if (verbose) puts("STUB: SetGlobalIconImagesCacheMaxEntriesAndMaxDataSize called");
    return NULL;
}

void* SetIconFamilyData(void)
{
    if (verbose) puts("STUB: SetIconFamilyData called");
    return NULL;
}

void* SetLabelColorAndName(void)
{
    if (verbose) puts("STUB: SetLabelColorAndName called");
    return NULL;
}

void* ShowHideDrag(void)
{
    if (verbose) puts("STUB: ShowHideDrag called");
    return NULL;
}

void* StartIPCPing(void)
{
    if (verbose) puts("STUB: StartIPCPing called");
    return NULL;
}

void* TranslationCopyDestinationType(void)
{
    if (verbose) puts("STUB: TranslationCopyDestinationType called");
    return NULL;
}

void* TranslationCopySourceType(void)
{
    if (verbose) puts("STUB: TranslationCopySourceType called");
    return NULL;
}

void* TranslationCreate(void)
{
    if (verbose) puts("STUB: TranslationCreate called");
    return NULL;
}

void* TranslationCreateWithSourceArray(void)
{
    if (verbose) puts("STUB: TranslationCreateWithSourceArray called");
    return NULL;
}

void* TranslationGetTranslationFlags(void)
{
    if (verbose) puts("STUB: TranslationGetTranslationFlags called");
    return NULL;
}

void* TranslationGetTypeID(void)
{
    if (verbose) puts("STUB: TranslationGetTypeID called");
    return NULL;
}

void* TranslationPerformForData(void)
{
    if (verbose) puts("STUB: TranslationPerformForData called");
    return NULL;
}

void* TranslationPerformForFile(void)
{
    if (verbose) puts("STUB: TranslationPerformForFile called");
    return NULL;
}

void* TranslationPerformForURL(void)
{
    if (verbose) puts("STUB: TranslationPerformForURL called");
    return NULL;
}

void* UAZoomChangeFocus(void)
{
    if (verbose) puts("STUB: UAZoomChangeFocus called");
    return NULL;
}

void* UAZoomEnabled(void)
{
    if (verbose) puts("STUB: UAZoomEnabled called");
    return NULL;
}

void* UnserializeCFType(void)
{
    if (verbose) puts("STUB: UnserializeCFType called");
    return NULL;
}

void* _AXCopyActionDescription(void)
{
    if (verbose) puts("STUB: _AXCopyActionDescription called");
    return NULL;
}

void* _AXCopyChildrenHash(void)
{
    if (verbose) puts("STUB: _AXCopyChildrenHash called");
    return NULL;
}

void* _AXCopyRoleDescription(void)
{
    if (verbose) puts("STUB: _AXCopyRoleDescription called");
    return NULL;
}

void* _AXCopyRoleDescriptionWithSubrole(void)
{
    if (verbose) puts("STUB: _AXCopyRoleDescriptionWithSubrole called");
    return NULL;
}

void* _AXCopyTitle(void)
{
    if (verbose) puts("STUB: _AXCopyTitle called");
    return NULL;
}

void* _AXCreateElementOrdering(void)
{
    if (verbose) puts("STUB: _AXCreateElementOrdering called");
    return NULL;
}

void* _AXCurrentRequestCanReturnInspectionContent(void)
{
    if (verbose) puts("STUB: _AXCurrentRequestCanReturnInspectionContent called");
    return NULL;
}

void* _AXCurrentRequestCanReturnProtectedContent(void)
{
    if (verbose) puts("STUB: _AXCurrentRequestCanReturnProtectedContent called");
    return NULL;
}

void* _AXInterfaceGetBristolEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetBristolEnabled called");
    return NULL;
}

void* _AXInterfaceGetDifferentiateWithoutColorEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetDifferentiateWithoutColorEnabled called");
    return NULL;
}

void* _AXInterfaceGetIncreaseContrastEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetIncreaseContrastEnabled called");
    return NULL;
}

void* _AXInterfaceGetReduceMotionEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetReduceMotionEnabled called");
    return NULL;
}

void* _AXInterfaceGetReduceTransparencyEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetReduceTransparencyEnabled called");
    return NULL;
}

void* _AXInterfaceGetRichmondEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceGetRichmondEnabled called");
    return NULL;
}

void* _AXInterfaceSetBristolEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetBristolEnabled called");
    return NULL;
}

void* _AXInterfaceSetDifferentiateWithoutColorEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetDifferentiateWithoutColorEnabled called");
    return NULL;
}

void* _AXInterfaceSetIncreaseContrastEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetIncreaseContrastEnabled called");
    return NULL;
}

void* _AXInterfaceSetIncreaseContrastEnabledOverride(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetIncreaseContrastEnabledOverride called");
    return NULL;
}

void* _AXInterfaceSetReduceMotionEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetReduceMotionEnabled called");
    return NULL;
}

void* _AXInterfaceSetReduceTransparencyEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetReduceTransparencyEnabled called");
    return NULL;
}

void* _AXInterfaceSetRichmondEnabled(void)
{
    if (verbose) puts("STUB: _AXInterfaceSetRichmondEnabled called");
    return NULL;
}

void* _AXRegisterControlComputerAccess(void)
{
    if (verbose) puts("STUB: _AXRegisterControlComputerAccess called");
    return NULL;
}

void* _AXShouldElementBeIgnoredForNavigation(void)
{
    if (verbose) puts("STUB: _AXShouldElementBeIgnoredForNavigation called");
    return NULL;
}

void* _AXUIElementCopyElementAtPositionIncludeIgnored(void)
{
    if (verbose) puts("STUB: _AXUIElementCopyElementAtPositionIncludeIgnored called");
    return NULL;
}

void* _AXUIElementCreateApplicationWithPresenterPid(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateApplicationWithPresenterPid called");
    return NULL;
}

void* _AXUIElementCreateWithData(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateWithData called");
    return NULL;
}

void* _AXUIElementCreateWithDataAndPid(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateWithDataAndPid called");
    return NULL;
}

void* _AXUIElementCreateWithDataAndPresenterPid(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateWithDataAndPresenterPid called");
    return NULL;
}

void* _AXUIElementCreateWithPtr(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateWithPtr called");
    return NULL;
}

void* _AXUIElementCreateWithRemoteToken(void)
{
    if (verbose) puts("STUB: _AXUIElementCreateWithRemoteToken called");
    return NULL;
}

void* _AXUIElementGetActualPid(void)
{
    if (verbose) puts("STUB: _AXUIElementGetActualPid called");
    return NULL;
}

void* _AXUIElementGetData(void)
{
    if (verbose) puts("STUB: _AXUIElementGetData called");
    return NULL;
}

void* _AXUIElementGetIsProcessSuspended(void)
{
    if (verbose) puts("STUB: _AXUIElementGetIsProcessSuspended called");
    return NULL;
}

void* _AXUIElementGetWindow(void)
{
    if (verbose) puts("STUB: _AXUIElementGetWindow called");
    return NULL;
}

void* _AXUIElementNotifyProcessSuspendStatus(void)
{
    if (verbose) puts("STUB: _AXUIElementNotifyProcessSuspendStatus called");
    return NULL;
}

void* _AXUIElementPostNotification(void)
{
    if (verbose) puts("STUB: _AXUIElementPostNotification called");
    return NULL;
}

void* _AXUIElementPostNotificationForObservedElement(void)
{
    if (verbose) puts("STUB: _AXUIElementPostNotificationForObservedElement called");
    return NULL;
}

void* _AXUIElementPostNotificationWithInfo(void)
{
    if (verbose) puts("STUB: _AXUIElementPostNotificationWithInfo called");
    return NULL;
}

void* _AXUIElementRegisterServerWithRunLoop(void)
{
    if (verbose) puts("STUB: _AXUIElementRegisterServerWithRunLoop called");
    return NULL;
}

void* _AXUIElementRemoteTokenCreate(void)
{
    if (verbose) puts("STUB: _AXUIElementRemoteTokenCreate called");
    return NULL;
}

void* _AXUIElementUnregisterServer(void)
{
    if (verbose) puts("STUB: _AXUIElementUnregisterServer called");
    return NULL;
}

void* _AddLabelsChangedCallback(void)
{
    if (verbose) puts("STUB: _AddLabelsChangedCallback called");
    return NULL;
}

void* _CopyProcessBundleLocationURL(void)
{
    if (verbose) puts("STUB: _CopyProcessBundleLocationURL called");
    return NULL;
}

void* _GDBIconsCGCacheList(void)
{
    if (verbose) puts("STUB: _GDBIconsCGCacheList called");
    return NULL;
}

void* _GetApplicationDesiresAttention(void)
{
    if (verbose) puts("STUB: _GetApplicationDesiresAttention called");
    return NULL;
}

void* _GetFrontUIProcess(void)
{
    if (verbose) puts("STUB: _GetFrontUIProcess called");
    return NULL;
}

void* _HIShapeCreateWithCGImage(void)
{
    if (verbose) puts("STUB: _HIShapeCreateWithCGImage called");
    return NULL;
}

void* _HIShapeCreateWithCGSRegionObj(void)
{
    if (verbose) puts("STUB: _HIShapeCreateWithCGSRegionObj called");
    return NULL;
}

void* _HIShapeGetNative(void)
{
    if (verbose) puts("STUB: _HIShapeGetNative called");
    return NULL;
}

void* _HIShapeOutsetToPixelBoundary(void)
{
    if (verbose) puts("STUB: _HIShapeOutsetToPixelBoundary called");
    return NULL;
}

void* _HIShapeSetImmutable(void)
{
    if (verbose) puts("STUB: _HIShapeSetImmutable called");
    return NULL;
}

void* _HIShapeSetShapeWithOffset(void)
{
    if (verbose) puts("STUB: _HIShapeSetShapeWithOffset called");
    return NULL;
}

void* _HideOtherApplications(void)
{
    if (verbose) puts("STUB: _HideOtherApplications called");
    return NULL;
}

void* _ICCopyMailHostName(void)
{
    if (verbose) puts("STUB: _ICCopyMailHostName called");
    return NULL;
}

void* _ICCopyMailUserName(void)
{
    if (verbose) puts("STUB: _ICCopyMailUserName called");
    return NULL;
}

void* _ISCreateCGImageForType(void)
{
    if (verbose) puts("STUB: _ISCreateCGImageForType called");
    return NULL;
}

void* _ISCreateCGImageForTypeAtScale(void)
{
    if (verbose) puts("STUB: _ISCreateCGImageForTypeAtScale called");
    return NULL;
}

void* _IconServicesGetCGImageRefFromIconRef(void)
{
    if (verbose) puts("STUB: _IconServicesGetCGImageRefFromIconRef called");
    return NULL;
}

void* _IconServicesGetCGImageRefFromURL(void)
{
    if (verbose) puts("STUB: _IconServicesGetCGImageRefFromURL called");
    return NULL;
}

void* _InstallGURLEventHandler(void)
{
    if (verbose) puts("STUB: _InstallGURLEventHandler called");
    return NULL;
}

void* _PIPZoomingEnabled(void)
{
    if (verbose) puts("STUB: _PIPZoomingEnabled called");
    return NULL;
}

void* _RegisterApplication(void)
{
    if (verbose) puts("STUB: _RegisterApplication called");
    return NULL;
}

void* _RegisterAsSessionLauncherApplication(void)
{
    if (verbose) puts("STUB: _RegisterAsSessionLauncherApplication called");
    return NULL;
}

void* _RemoveLabelsChangedCallback(void)
{
    if (verbose) puts("STUB: _RemoveLabelsChangedCallback called");
    return NULL;
}

void* _SetApplicationDesiresAttention(void)
{
    if (verbose) puts("STUB: _SetApplicationDesiresAttention called");
    return NULL;
}

void* _SetFrontProcessWithOptions(void)
{
    if (verbose) puts("STUB: _SetFrontProcessWithOptions called");
    return NULL;
}

void* _SetHLTBWakeUpHook(void)
{
    if (verbose) puts("STUB: _SetHLTBWakeUpHook called");
    return NULL;
}

void* _ShowAllApplications(void)
{
    if (verbose) puts("STUB: _ShowAllApplications called");
    return NULL;
}

void* _SignalApplicationReady(void)
{
    if (verbose) puts("STUB: _SignalApplicationReady called");
    return NULL;
}

void* _UAZoomFocusChange(void)
{
    if (verbose) puts("STUB: _UAZoomFocusChange called");
    return NULL;
}

void* _UAZoomFocusChangeAnchored(void)
{
    if (verbose) puts("STUB: _UAZoomFocusChangeAnchored called");
    return NULL;
}

void* _UAZoomFocusChangeHighlightRect(void)
{
    if (verbose) puts("STUB: _UAZoomFocusChangeHighlightRect called");
    return NULL;
}

void* _UAZoomFocusChangeHighlightRectAnchored(void)
{
    if (verbose) puts("STUB: _UAZoomFocusChangeHighlightRectAnchored called");
    return NULL;
}

void* _UAZoomingEnabled(void)
{
    if (verbose) puts("STUB: _UAZoomingEnabled called");
    return NULL;
}

void* _UnregisterAsSessionLauncherApplication(void)
{
    if (verbose) puts("STUB: _UnregisterAsSessionLauncherApplication called");
    return NULL;
}
