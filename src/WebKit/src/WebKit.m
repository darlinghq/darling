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


#include <WebKit/WebKit.h>
#include <stdlib.h>
#include <stdio.h>

@interface WebView : NSObject
@end
@implementation WebView
@end

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DatabaseServiceInitializer(void) {
    if (verbose) puts("STUB: DatabaseServiceInitializer called");
    return NULL;
}

void* NetworkServiceInitializer(void) {
    if (verbose) puts("STUB: NetworkServiceInitializer called");
    return NULL;
}

void* PluginServiceInitializer(void) {
    if (verbose) puts("STUB: PluginServiceInitializer called");
    return NULL;
}

void* WKAccessibilityEnableEnhancedAccessibility(void) {
    if (verbose) puts("STUB: WKAccessibilityEnableEnhancedAccessibility called");
    return NULL;
}

void* WKAccessibilityEnhancedAccessibilityEnabled(void) {
    if (verbose) puts("STUB: WKAccessibilityEnhancedAccessibilityEnabled called");
    return NULL;
}

void* WKAccessibilityFocusedObject(void) {
    if (verbose) puts("STUB: WKAccessibilityFocusedObject called");
    return NULL;
}

void* WKAccessibilityRootObject(void) {
    if (verbose) puts("STUB: WKAccessibilityRootObject called");
    return NULL;
}

void* WKApplicationCacheManagerDeleteAllEntries(void) {
    if (verbose) puts("STUB: WKApplicationCacheManagerDeleteAllEntries called");
    return NULL;
}

void* WKApplicationCacheManagerDeleteEntriesForOrigin(void) {
    if (verbose) puts("STUB: WKApplicationCacheManagerDeleteEntriesForOrigin called");
    return NULL;
}

void* WKApplicationCacheManagerGetApplicationCacheOrigins(void) {
    if (verbose) puts("STUB: WKApplicationCacheManagerGetApplicationCacheOrigins called");
    return NULL;
}

void* WKApplicationCacheManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKApplicationCacheManagerGetTypeID called");
    return NULL;
}

void* WKArrayAppendItem(void) {
    if (verbose) puts("STUB: WKArrayAppendItem called");
    return NULL;
}

void* WKArrayCreate(void) {
    if (verbose) puts("STUB: WKArrayCreate called");
    return NULL;
}

void* WKArrayCreateAdoptingValues(void) {
    if (verbose) puts("STUB: WKArrayCreateAdoptingValues called");
    return NULL;
}

void* WKArrayGetItemAtIndex(void) {
    if (verbose) puts("STUB: WKArrayGetItemAtIndex called");
    return NULL;
}

void* WKArrayGetSize(void) {
    if (verbose) puts("STUB: WKArrayGetSize called");
    return NULL;
}

void* WKArrayGetTypeID(void) {
    if (verbose) puts("STUB: WKArrayGetTypeID called");
    return NULL;
}

void* WKArrayRemoveItemAtIndex(void) {
    if (verbose) puts("STUB: WKArrayRemoveItemAtIndex called");
    return NULL;
}

void* WKAuthenticationChallengeGetDecisionListener(void) {
    if (verbose) puts("STUB: WKAuthenticationChallengeGetDecisionListener called");
    return NULL;
}

void* WKAuthenticationChallengeGetPreviousFailureCount(void) {
    if (verbose) puts("STUB: WKAuthenticationChallengeGetPreviousFailureCount called");
    return NULL;
}

void* WKAuthenticationChallengeGetProposedCredential(void) {
    if (verbose) puts("STUB: WKAuthenticationChallengeGetProposedCredential called");
    return NULL;
}

void* WKAuthenticationChallengeGetProtectionSpace(void) {
    if (verbose) puts("STUB: WKAuthenticationChallengeGetProtectionSpace called");
    return NULL;
}

void* WKAuthenticationChallengeGetTypeID(void) {
    if (verbose) puts("STUB: WKAuthenticationChallengeGetTypeID called");
    return NULL;
}

void* WKAuthenticationDecisionListenerCancel(void) {
    if (verbose) puts("STUB: WKAuthenticationDecisionListenerCancel called");
    return NULL;
}

void* WKAuthenticationDecisionListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKAuthenticationDecisionListenerGetTypeID called");
    return NULL;
}

void* WKAuthenticationDecisionListenerRejectProtectionSpaceAndContinue(void) {
    if (verbose) puts("STUB: WKAuthenticationDecisionListenerRejectProtectionSpaceAndContinue called");
    return NULL;
}

void* WKAuthenticationDecisionListenerUseCredential(void) {
    if (verbose) puts("STUB: WKAuthenticationDecisionListenerUseCredential called");
    return NULL;
}

void* WKBackForwardListClear(void) {
    if (verbose) puts("STUB: WKBackForwardListClear called");
    return NULL;
}

void* WKBackForwardListCopyBackListWithLimit(void) {
    if (verbose) puts("STUB: WKBackForwardListCopyBackListWithLimit called");
    return NULL;
}

void* WKBackForwardListCopyForwardListWithLimit(void) {
    if (verbose) puts("STUB: WKBackForwardListCopyForwardListWithLimit called");
    return NULL;
}

void* WKBackForwardListGetBackItem(void) {
    if (verbose) puts("STUB: WKBackForwardListGetBackItem called");
    return NULL;
}

void* WKBackForwardListGetBackListCount(void) {
    if (verbose) puts("STUB: WKBackForwardListGetBackListCount called");
    return NULL;
}

void* WKBackForwardListGetCurrentItem(void) {
    if (verbose) puts("STUB: WKBackForwardListGetCurrentItem called");
    return NULL;
}

void* WKBackForwardListGetForwardItem(void) {
    if (verbose) puts("STUB: WKBackForwardListGetForwardItem called");
    return NULL;
}

void* WKBackForwardListGetForwardListCount(void) {
    if (verbose) puts("STUB: WKBackForwardListGetForwardListCount called");
    return NULL;
}

void* WKBackForwardListGetItemAtIndex(void) {
    if (verbose) puts("STUB: WKBackForwardListGetItemAtIndex called");
    return NULL;
}

void* WKBackForwardListGetTypeID(void) {
    if (verbose) puts("STUB: WKBackForwardListGetTypeID called");
    return NULL;
}

void* WKBackForwardListItemCopyOriginalURL(void) {
    if (verbose) puts("STUB: WKBackForwardListItemCopyOriginalURL called");
    return NULL;
}

void* WKBackForwardListItemCopyTitle(void) {
    if (verbose) puts("STUB: WKBackForwardListItemCopyTitle called");
    return NULL;
}

void* WKBackForwardListItemCopyURL(void) {
    if (verbose) puts("STUB: WKBackForwardListItemCopyURL called");
    return NULL;
}

void* WKBackForwardListItemGetTypeID(void) {
    if (verbose) puts("STUB: WKBackForwardListItemGetTypeID called");
    return NULL;
}

void* WKBooleanCreate(void) {
    if (verbose) puts("STUB: WKBooleanCreate called");
    return NULL;
}

void* WKBooleanGetTypeID(void) {
    if (verbose) puts("STUB: WKBooleanGetTypeID called");
    return NULL;
}

void* WKBooleanGetValue(void) {
    if (verbose) puts("STUB: WKBooleanGetValue called");
    return NULL;
}

void* WKBundleAddOriginAccessWhitelistEntry(void) {
    if (verbose) puts("STUB: WKBundleAddOriginAccessWhitelistEntry called");
    return NULL;
}

void* WKBundleAddUserScript(void) {
    if (verbose) puts("STUB: WKBundleAddUserScript called");
    return NULL;
}

void* WKBundleAddUserStyleSheet(void) {
    if (verbose) puts("STUB: WKBundleAddUserStyleSheet called");
    return NULL;
}

void* WKBundleBackForwardListClear(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListClear called");
    return NULL;
}

void* WKBundleBackForwardListCopyItemAtIndex(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListCopyItemAtIndex called");
    return NULL;
}

void* WKBundleBackForwardListGetBackListCount(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListGetBackListCount called");
    return NULL;
}

void* WKBundleBackForwardListGetForwardListCount(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListGetForwardListCount called");
    return NULL;
}

void* WKBundleBackForwardListGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListGetTypeID called");
    return NULL;
}

void* WKBundleBackForwardListItemCopyChildren(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemCopyChildren called");
    return NULL;
}

void* WKBundleBackForwardListItemCopyOriginalURL(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemCopyOriginalURL called");
    return NULL;
}

void* WKBundleBackForwardListItemCopyTarget(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemCopyTarget called");
    return NULL;
}

void* WKBundleBackForwardListItemCopyTitle(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemCopyTitle called");
    return NULL;
}

void* WKBundleBackForwardListItemCopyURL(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemCopyURL called");
    return NULL;
}

void* WKBundleBackForwardListItemGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemGetTypeID called");
    return NULL;
}

void* WKBundleBackForwardListItemHasCachedPageExpired(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemHasCachedPageExpired called");
    return NULL;
}

void* WKBundleBackForwardListItemIsInPageCache(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemIsInPageCache called");
    return NULL;
}

void* WKBundleBackForwardListItemIsSame(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemIsSame called");
    return NULL;
}

void* WKBundleBackForwardListItemIsTargetItem(void) {
    if (verbose) puts("STUB: WKBundleBackForwardListItemIsTargetItem called");
    return NULL;
}

void* WKBundleClearAllDatabases(void) {
    if (verbose) puts("STUB: WKBundleClearAllDatabases called");
    return NULL;
}

void* WKBundleClearResourceLoadStatistics(void) {
    if (verbose) puts("STUB: WKBundleClearResourceLoadStatistics called");
    return NULL;
}

void* WKBundleCreateWKDataFromUInt8Array(void) {
    if (verbose) puts("STUB: WKBundleCreateWKDataFromUInt8Array called");
    return NULL;
}

void* WKBundleDOMWindowExtensionCreate(void) {
    if (verbose) puts("STUB: WKBundleDOMWindowExtensionCreate called");
    return NULL;
}

void* WKBundleDOMWindowExtensionGetFrame(void) {
    if (verbose) puts("STUB: WKBundleDOMWindowExtensionGetFrame called");
    return NULL;
}

void* WKBundleDOMWindowExtensionGetScriptWorld(void) {
    if (verbose) puts("STUB: WKBundleDOMWindowExtensionGetScriptWorld called");
    return NULL;
}

void* WKBundleDOMWindowExtensionGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleDOMWindowExtensionGetTypeID called");
    return NULL;
}

void* WKBundleFileHandleCreateWithPath(void) {
    if (verbose) puts("STUB: WKBundleFileHandleCreateWithPath called");
    return NULL;
}

void* WKBundleFileHandleGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleFileHandleGetTypeID called");
    return NULL;
}

void* WKBundleFrameAllowsFollowingLink(void) {
    if (verbose) puts("STUB: WKBundleFrameAllowsFollowingLink called");
    return NULL;
}

void* WKBundleFrameCallShouldCloseOnWebView(void) {
    if (verbose) puts("STUB: WKBundleFrameCallShouldCloseOnWebView called");
    return NULL;
}

void* WKBundleFrameClearOpener(void) {
    if (verbose) puts("STUB: WKBundleFrameClearOpener called");
    return NULL;
}

void* WKBundleFrameContainsAnyFormControls(void) {
    if (verbose) puts("STUB: WKBundleFrameContainsAnyFormControls called");
    return NULL;
}

void* WKBundleFrameContainsAnyFormElements(void) {
    if (verbose) puts("STUB: WKBundleFrameContainsAnyFormElements called");
    return NULL;
}

void* WKBundleFrameCopyChildFrames(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyChildFrames called");
    return NULL;
}

void* WKBundleFrameCopyCounterValue(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyCounterValue called");
    return NULL;
}

void* WKBundleFrameCopyInnerText(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyInnerText called");
    return NULL;
}

void* WKBundleFrameCopyLayerTreeAsText(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyLayerTreeAsText called");
    return NULL;
}

void* WKBundleFrameCopyMIMETypeForResourceWithURL(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyMIMETypeForResourceWithURL called");
    return NULL;
}

void* WKBundleFrameCopyName(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyName called");
    return NULL;
}

void* WKBundleFrameCopyProvisionalURL(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyProvisionalURL called");
    return NULL;
}

void* WKBundleFrameCopySecurityOrigin(void) {
    if (verbose) puts("STUB: WKBundleFrameCopySecurityOrigin called");
    return NULL;
}

void* WKBundleFrameCopySuggestedFilenameForResourceWithURL(void) {
    if (verbose) puts("STUB: WKBundleFrameCopySuggestedFilenameForResourceWithURL called");
    return NULL;
}

void* WKBundleFrameCopyURL(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyURL called");
    return NULL;
}

void* WKBundleFrameCopyWebArchive(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyWebArchive called");
    return NULL;
}

void* WKBundleFrameCopyWebArchiveFilteringSubframes(void) {
    if (verbose) puts("STUB: WKBundleFrameCopyWebArchiveFilteringSubframes called");
    return NULL;
}

void* WKBundleFrameCreateFrameHandle(void) {
    if (verbose) puts("STUB: WKBundleFrameCreateFrameHandle called");
    return NULL;
}

void* WKBundleFrameCreateHitTestResult(void) {
    if (verbose) puts("STUB: WKBundleFrameCreateHitTestResult called");
    return NULL;
}

void* WKBundleFrameFocus(void) {
    if (verbose) puts("STUB: WKBundleFrameFocus called");
    return NULL;
}

void* WKBundleFrameForJavaScriptContext(void) {
    if (verbose) puts("STUB: WKBundleFrameForJavaScriptContext called");
    return NULL;
}

void* WKBundleFrameGetContentBounds(void) {
    if (verbose) puts("STUB: WKBundleFrameGetContentBounds called");
    return NULL;
}

void* WKBundleFrameGetDocumentBackgroundColor(void) {
    if (verbose) puts("STUB: WKBundleFrameGetDocumentBackgroundColor called");
    return NULL;
}

void* WKBundleFrameGetFrameLoadState(void) {
    if (verbose) puts("STUB: WKBundleFrameGetFrameLoadState called");
    return NULL;
}

void* WKBundleFrameGetJavaScriptContext(void) {
    if (verbose) puts("STUB: WKBundleFrameGetJavaScriptContext called");
    return NULL;
}

void* WKBundleFrameGetJavaScriptContextForWorld(void) {
    if (verbose) puts("STUB: WKBundleFrameGetJavaScriptContextForWorld called");
    return NULL;
}

void* WKBundleFrameGetJavaScriptWrapperForFileForWorld(void) {
    if (verbose) puts("STUB: WKBundleFrameGetJavaScriptWrapperForFileForWorld called");
    return NULL;
}

void* WKBundleFrameGetJavaScriptWrapperForNodeForWorld(void) {
    if (verbose) puts("STUB: WKBundleFrameGetJavaScriptWrapperForNodeForWorld called");
    return NULL;
}

void* WKBundleFrameGetJavaScriptWrapperForRangeForWorld(void) {
    if (verbose) puts("STUB: WKBundleFrameGetJavaScriptWrapperForRangeForWorld called");
    return NULL;
}

void* WKBundleFrameGetPage(void) {
    if (verbose) puts("STUB: WKBundleFrameGetPage called");
    return NULL;
}

void* WKBundleFrameGetParentFrame(void) {
    if (verbose) puts("STUB: WKBundleFrameGetParentFrame called");
    return NULL;
}

void* WKBundleFrameGetPendingUnloadCount(void) {
    if (verbose) puts("STUB: WKBundleFrameGetPendingUnloadCount called");
    return NULL;
}

void* WKBundleFrameGetScrollOffset(void) {
    if (verbose) puts("STUB: WKBundleFrameGetScrollOffset called");
    return NULL;
}

void* WKBundleFrameGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleFrameGetTypeID called");
    return NULL;
}

void* WKBundleFrameGetVisibleContentBounds(void) {
    if (verbose) puts("STUB: WKBundleFrameGetVisibleContentBounds called");
    return NULL;
}

void* WKBundleFrameGetVisibleContentBoundsExcludingScrollbars(void) {
    if (verbose) puts("STUB: WKBundleFrameGetVisibleContentBoundsExcludingScrollbars called");
    return NULL;
}

void* WKBundleFrameHandlesPageScaleGesture(void) {
    if (verbose) puts("STUB: WKBundleFrameHandlesPageScaleGesture called");
    return NULL;
}

void* WKBundleFrameHasHorizontalScrollbar(void) {
    if (verbose) puts("STUB: WKBundleFrameHasHorizontalScrollbar called");
    return NULL;
}

void* WKBundleFrameHasVerticalScrollbar(void) {
    if (verbose) puts("STUB: WKBundleFrameHasVerticalScrollbar called");
    return NULL;
}

void* WKBundleFrameIsMainFrame(void) {
    if (verbose) puts("STUB: WKBundleFrameIsMainFrame called");
    return NULL;
}

void* WKBundleFrameSetAccessibleName(void) {
    if (verbose) puts("STUB: WKBundleFrameSetAccessibleName called");
    return NULL;
}

void* WKBundleFrameSetTextDirection(void) {
    if (verbose) puts("STUB: WKBundleFrameSetTextDirection called");
    return NULL;
}

void* WKBundleFrameStopLoading(void) {
    if (verbose) puts("STUB: WKBundleFrameStopLoading called");
    return NULL;
}

void* WKBundleGarbageCollectJavaScriptObjects(void) {
    if (verbose) puts("STUB: WKBundleGarbageCollectJavaScriptObjects called");
    return NULL;
}

void* WKBundleGarbageCollectJavaScriptObjectsOnAlternateThreadForDebugging(void) {
    if (verbose) puts("STUB: WKBundleGarbageCollectJavaScriptObjectsOnAlternateThreadForDebugging called");
    return NULL;
}

void* WKBundleGetApplicationConnection(void) {
    if (verbose) puts("STUB: WKBundleGetApplicationConnection called");
    return NULL;
}

void* WKBundleGetJavaScriptObjectsCount(void) {
    if (verbose) puts("STUB: WKBundleGetJavaScriptObjectsCount called");
    return NULL;
}

void* WKBundleGetParameters(void) {
    if (verbose) puts("STUB: WKBundleGetParameters called");
    return NULL;
}

void* WKBundleGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleGetTypeID called");
    return NULL;
}

void* WKBundleGetWebNotificationID(void) {
    if (verbose) puts("STUB: WKBundleGetWebNotificationID called");
    return NULL;
}

void* WKBundleHitTestResultCopyAbsoluteImageURL(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyAbsoluteImageURL called");
    return NULL;
}

void* WKBundleHitTestResultCopyAbsoluteLinkURL(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyAbsoluteLinkURL called");
    return NULL;
}

void* WKBundleHitTestResultCopyAbsoluteMediaURL(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyAbsoluteMediaURL called");
    return NULL;
}

void* WKBundleHitTestResultCopyAbsolutePDFURL(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyAbsolutePDFURL called");
    return NULL;
}

void* WKBundleHitTestResultCopyImage(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyImage called");
    return NULL;
}

void* WKBundleHitTestResultCopyLinkLabel(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyLinkLabel called");
    return NULL;
}

void* WKBundleHitTestResultCopyLinkSuggestedFilename(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyLinkSuggestedFilename called");
    return NULL;
}

void* WKBundleHitTestResultCopyLinkTitle(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyLinkTitle called");
    return NULL;
}

void* WKBundleHitTestResultCopyNodeHandle(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyNodeHandle called");
    return NULL;
}

void* WKBundleHitTestResultCopyURLElementHandle(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultCopyURLElementHandle called");
    return NULL;
}

void* WKBundleHitTestResultGetFrame(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetFrame called");
    return NULL;
}

void* WKBundleHitTestResultGetImageRect(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetImageRect called");
    return NULL;
}

void* WKBundleHitTestResultGetIsSelected(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetIsSelected called");
    return NULL;
}

void* WKBundleHitTestResultGetMediaType(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetMediaType called");
    return NULL;
}

void* WKBundleHitTestResultGetTargetFrame(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetTargetFrame called");
    return NULL;
}

void* WKBundleHitTestResultGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultGetTypeID called");
    return NULL;
}

void* WKBundleHitTestResultIsDownloadableMedia(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultIsDownloadableMedia called");
    return NULL;
}

void* WKBundleHitTestResultMediaHasAudio(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultMediaHasAudio called");
    return NULL;
}

void* WKBundleHitTestResultMediaIsInFullscreen(void) {
    if (verbose) puts("STUB: WKBundleHitTestResultMediaIsInFullscreen called");
    return NULL;
}

void* WKBundleInspectorClose(void) {
    if (verbose) puts("STUB: WKBundleInspectorClose called");
    return NULL;
}

void* WKBundleInspectorEvaluateScriptForTest(void) {
    if (verbose) puts("STUB: WKBundleInspectorEvaluateScriptForTest called");
    return NULL;
}

void* WKBundleInspectorGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleInspectorGetTypeID called");
    return NULL;
}

void* WKBundleInspectorSetPageProfilingEnabled(void) {
    if (verbose) puts("STUB: WKBundleInspectorSetPageProfilingEnabled called");
    return NULL;
}

void* WKBundleInspectorShow(void) {
    if (verbose) puts("STUB: WKBundleInspectorShow called");
    return NULL;
}

void* WKBundleIsPageBoxVisible(void) {
    if (verbose) puts("STUB: WKBundleIsPageBoxVisible called");
    return NULL;
}

void* WKBundleIsProcessingUserGesture(void) {
    if (verbose) puts("STUB: WKBundleIsProcessingUserGesture called");
    return NULL;
}

void* WKBundleNavigationActionCopyDownloadAttribute(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionCopyDownloadAttribute called");
    return NULL;
}

void* WKBundleNavigationActionCopyFormElement(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionCopyFormElement called");
    return NULL;
}

void* WKBundleNavigationActionCopyHitTestResult(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionCopyHitTestResult called");
    return NULL;
}

void* WKBundleNavigationActionGetEventModifiers(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetEventModifiers called");
    return NULL;
}

void* WKBundleNavigationActionGetEventMouseButton(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetEventMouseButton called");
    return NULL;
}

void* WKBundleNavigationActionGetNavigationType(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetNavigationType called");
    return NULL;
}

void* WKBundleNavigationActionGetShouldOpenExternalURLs(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetShouldOpenExternalURLs called");
    return NULL;
}

void* WKBundleNavigationActionGetShouldTryAppLinks(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetShouldTryAppLinks called");
    return NULL;
}

void* WKBundleNavigationActionGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleNavigationActionGetTypeID called");
    return NULL;
}

void* WKBundleNodeHandleCopyDocument(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyDocument called");
    return NULL;
}

void* WKBundleNodeHandleCopyDocumentFrame(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyDocumentFrame called");
    return NULL;
}

void* WKBundleNodeHandleCopyHTMLFrameElementContentFrame(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyHTMLFrameElementContentFrame called");
    return NULL;
}

void* WKBundleNodeHandleCopyHTMLIFrameElementContentFrame(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyHTMLIFrameElementContentFrame called");
    return NULL;
}

void* WKBundleNodeHandleCopyHTMLTableCellElementCellAbove(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyHTMLTableCellElementCellAbove called");
    return NULL;
}

void* WKBundleNodeHandleCopySnapshotWithOptions(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopySnapshotWithOptions called");
    return NULL;
}

void* WKBundleNodeHandleCopyVisibleRange(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCopyVisibleRange called");
    return NULL;
}

void* WKBundleNodeHandleCreate(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleCreate called");
    return NULL;
}

void* WKBundleNodeHandleGetElementBounds(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetElementBounds called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutoFillAvailable(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutoFillAvailable called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutoFillButtonBounds(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutoFillButtonBounds called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutoFillButtonEnabled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutoFillButtonEnabled called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutoFillButtonType(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutoFillButtonType called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutoFilled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutoFilled called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementAutofilled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementAutofilled called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementLastAutoFillButtonType(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementLastAutoFillButtonType called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLInputElementLastChangeWasUserEdit(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLInputElementLastChangeWasUserEdit called");
    return NULL;
}

void* WKBundleNodeHandleGetHTMLTextAreaElementLastChangeWasUserEdit(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetHTMLTextAreaElementLastChangeWasUserEdit called");
    return NULL;
}

void* WKBundleNodeHandleGetRenderRect(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetRenderRect called");
    return NULL;
}

void* WKBundleNodeHandleGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleGetTypeID called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementAutoFillAvailable(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementAutoFillAvailable called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementAutoFillButtonEnabled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementAutoFillButtonEnabled called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementAutoFillButtonEnabledWithButtonType(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementAutoFillButtonEnabledWithButtonType called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementAutoFilled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementAutoFilled called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementAutofilled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementAutofilled called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementSpellcheckEnabled(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementSpellcheckEnabled called");
    return NULL;
}

void* WKBundleNodeHandleSetHTMLInputElementValueForUser(void) {
    if (verbose) puts("STUB: WKBundleNodeHandleSetHTMLInputElementValueForUser called");
    return NULL;
}

void* WKBundleNumberOfPages(void) {
    if (verbose) puts("STUB: WKBundleNumberOfPages called");
    return NULL;
}

void* WKBundleOverrideBoolPreferenceForTestRunner(void) {
    if (verbose) puts("STUB: WKBundleOverrideBoolPreferenceForTestRunner called");
    return NULL;
}

void* WKBundlePageAddUserScript(void) {
    if (verbose) puts("STUB: WKBundlePageAddUserScript called");
    return NULL;
}

void* WKBundlePageAddUserStyleSheet(void) {
    if (verbose) puts("STUB: WKBundlePageAddUserStyleSheet called");
    return NULL;
}

void* WKBundlePageBannerCreateBannerWithCALayer(void) {
    if (verbose) puts("STUB: WKBundlePageBannerCreateBannerWithCALayer called");
    return NULL;
}

void* WKBundlePageBannerGetLayer(void) {
    if (verbose) puts("STUB: WKBundlePageBannerGetLayer called");
    return NULL;
}

void* WKBundlePageBannerGetTypeID(void) {
    if (verbose) puts("STUB: WKBundlePageBannerGetTypeID called");
    return NULL;
}

void* WKBundlePageCanHandleRequest(void) {
    if (verbose) puts("STUB: WKBundlePageCanHandleRequest called");
    return NULL;
}

void* WKBundlePageCanShowMIMEType(void) {
    if (verbose) puts("STUB: WKBundlePageCanShowMIMEType called");
    return NULL;
}

void* WKBundlePageClearApplicationCache(void) {
    if (verbose) puts("STUB: WKBundlePageClearApplicationCache called");
    return NULL;
}

void* WKBundlePageClearApplicationCacheForOrigin(void) {
    if (verbose) puts("STUB: WKBundlePageClearApplicationCacheForOrigin called");
    return NULL;
}

void* WKBundlePageClearMainFrameName(void) {
    if (verbose) puts("STUB: WKBundlePageClearMainFrameName called");
    return NULL;
}

void* WKBundlePageClickMenuItem(void) {
    if (verbose) puts("STUB: WKBundlePageClickMenuItem called");
    return NULL;
}

void* WKBundlePageClose(void) {
    if (verbose) puts("STUB: WKBundlePageClose called");
    return NULL;
}

void* WKBundlePageConfirmComposition(void) {
    if (verbose) puts("STUB: WKBundlePageConfirmComposition called");
    return NULL;
}

void* WKBundlePageConfirmCompositionWithText(void) {
    if (verbose) puts("STUB: WKBundlePageConfirmCompositionWithText called");
    return NULL;
}

void* WKBundlePageCopyContextMenuAtPointInWindow(void) {
    if (verbose) puts("STUB: WKBundlePageCopyContextMenuAtPointInWindow called");
    return NULL;
}

void* WKBundlePageCopyContextMenuItems(void) {
    if (verbose) puts("STUB: WKBundlePageCopyContextMenuItems called");
    return NULL;
}

void* WKBundlePageCopyGroupIdentifier(void) {
    if (verbose) puts("STUB: WKBundlePageCopyGroupIdentifier called");
    return NULL;
}

void* WKBundlePageCopyOriginsWithApplicationCache(void) {
    if (verbose) puts("STUB: WKBundlePageCopyOriginsWithApplicationCache called");
    return NULL;
}

void* WKBundlePageCopyRenderLayerTree(void) {
    if (verbose) puts("STUB: WKBundlePageCopyRenderLayerTree called");
    return NULL;
}

void* WKBundlePageCopyRenderTree(void) {
    if (verbose) puts("STUB: WKBundlePageCopyRenderTree called");
    return NULL;
}

void* WKBundlePageCopyRenderTreeExternalRepresentation(void) {
    if (verbose) puts("STUB: WKBundlePageCopyRenderTreeExternalRepresentation called");
    return NULL;
}

void* WKBundlePageCopyRenderTreeExternalRepresentationForPrinting(void) {
    if (verbose) puts("STUB: WKBundlePageCopyRenderTreeExternalRepresentationForPrinting called");
    return NULL;
}

void* WKBundlePageCopyTrackedRepaintRects(void) {
    if (verbose) puts("STUB: WKBundlePageCopyTrackedRepaintRects called");
    return NULL;
}

void* WKBundlePageCreateScaledSnapshotInDocumentCoordinates(void) {
    if (verbose) puts("STUB: WKBundlePageCreateScaledSnapshotInDocumentCoordinates called");
    return NULL;
}

void* WKBundlePageCreateSnapshotInDocumentCoordinates(void) {
    if (verbose) puts("STUB: WKBundlePageCreateSnapshotInDocumentCoordinates called");
    return NULL;
}

void* WKBundlePageCreateSnapshotInViewCoordinates(void) {
    if (verbose) puts("STUB: WKBundlePageCreateSnapshotInViewCoordinates called");
    return NULL;
}

void* WKBundlePageCreateSnapshotWithOptions(void) {
    if (verbose) puts("STUB: WKBundlePageCreateSnapshotWithOptions called");
    return NULL;
}

void* WKBundlePageDidEnterFullScreen(void) {
    if (verbose) puts("STUB: WKBundlePageDidEnterFullScreen called");
    return NULL;
}

void* WKBundlePageDidExitFullScreen(void) {
    if (verbose) puts("STUB: WKBundlePageDidExitFullScreen called");
    return NULL;
}

void* WKBundlePageExecuteEditingCommand(void) {
    if (verbose) puts("STUB: WKBundlePageExecuteEditingCommand called");
    return NULL;
}

void* WKBundlePageExtendIncrementalRenderingSuppression(void) {
    if (verbose) puts("STUB: WKBundlePageExtendIncrementalRenderingSuppression called");
    return NULL;
}

void* WKBundlePageFindString(void) {
    if (verbose) puts("STUB: WKBundlePageFindString called");
    return NULL;
}

void* WKBundlePageForceRepaint(void) {
    if (verbose) puts("STUB: WKBundlePageForceRepaint called");
    return NULL;
}

void* WKBundlePageGetAppCacheUsageForOrigin(void) {
    if (verbose) puts("STUB: WKBundlePageGetAppCacheUsageForOrigin called");
    return NULL;
}

void* WKBundlePageGetBackForwardList(void) {
    if (verbose) puts("STUB: WKBundlePageGetBackForwardList called");
    return NULL;
}

void* WKBundlePageGetBackingScaleFactor(void) {
    if (verbose) puts("STUB: WKBundlePageGetBackingScaleFactor called");
    return NULL;
}

void* WKBundlePageGetInspector(void) {
    if (verbose) puts("STUB: WKBundlePageGetInspector called");
    return NULL;
}

void* WKBundlePageGetMainFrame(void) {
    if (verbose) puts("STUB: WKBundlePageGetMainFrame called");
    return NULL;
}

void* WKBundlePageGetPageGroup(void) {
    if (verbose) puts("STUB: WKBundlePageGetPageGroup called");
    return NULL;
}

void* WKBundlePageGetPageZoomFactor(void) {
    if (verbose) puts("STUB: WKBundlePageGetPageZoomFactor called");
    return NULL;
}

void* WKBundlePageGetRenderTreeSize(void) {
    if (verbose) puts("STUB: WKBundlePageGetRenderTreeSize called");
    return NULL;
}

void* WKBundlePageGetTextZoomFactor(void) {
    if (verbose) puts("STUB: WKBundlePageGetTextZoomFactor called");
    return NULL;
}

void* WKBundlePageGetTypeID(void) {
    if (verbose) puts("STUB: WKBundlePageGetTypeID called");
    return NULL;
}

void* WKBundlePageGroupCopyIdentifier(void) {
    if (verbose) puts("STUB: WKBundlePageGroupCopyIdentifier called");
    return NULL;
}

void* WKBundlePageGroupGetTypeID(void) {
    if (verbose) puts("STUB: WKBundlePageGroupGetTypeID called");
    return NULL;
}

void* WKBundlePageHasComposition(void) {
    if (verbose) puts("STUB: WKBundlePageHasComposition called");
    return NULL;
}

void* WKBundlePageHasLocalDataForURL(void) {
    if (verbose) puts("STUB: WKBundlePageHasLocalDataForURL called");
    return NULL;
}

void* WKBundlePageInsertNewlineInQuotedContent(void) {
    if (verbose) puts("STUB: WKBundlePageInsertNewlineInQuotedContent called");
    return NULL;
}

void* WKBundlePageInstallPageOverlay(void) {
    if (verbose) puts("STUB: WKBundlePageInstallPageOverlay called");
    return NULL;
}

void* WKBundlePageInstallPageOverlayWithAnimation(void) {
    if (verbose) puts("STUB: WKBundlePageInstallPageOverlayWithAnimation called");
    return NULL;
}

void* WKBundlePageIsControlledByAutomation(void) {
    if (verbose) puts("STUB: WKBundlePageIsControlledByAutomation called");
    return NULL;
}

void* WKBundlePageIsEditingCommandEnabled(void) {
    if (verbose) puts("STUB: WKBundlePageIsEditingCommandEnabled called");
    return NULL;
}

void* WKBundlePageIsTrackingRepaints(void) {
    if (verbose) puts("STUB: WKBundlePageIsTrackingRepaints called");
    return NULL;
}

void* WKBundlePageIsUsingEphemeralSession(void) {
    if (verbose) puts("STUB: WKBundlePageIsUsingEphemeralSession called");
    return NULL;
}

void* WKBundlePageListenForLayoutMilestones(void) {
    if (verbose) puts("STUB: WKBundlePageListenForLayoutMilestones called");
    return NULL;
}

void* WKBundlePageNumberForElementById(void) {
    if (verbose) puts("STUB: WKBundlePageNumberForElementById called");
    return NULL;
}

void* WKBundlePageOverlayClear(void) {
    if (verbose) puts("STUB: WKBundlePageOverlayClear called");
    return NULL;
}

void* WKBundlePageOverlayCreate(void) {
    if (verbose) puts("STUB: WKBundlePageOverlayCreate called");
    return NULL;
}

void* WKBundlePageOverlayFractionFadedIn(void) {
    if (verbose) puts("STUB: WKBundlePageOverlayFractionFadedIn called");
    return NULL;
}

void* WKBundlePageOverlayGetTypeID(void) {
    if (verbose) puts("STUB: WKBundlePageOverlayGetTypeID called");
    return NULL;
}

void* WKBundlePageOverlaySetAccessibilityClient(void) {
    if (verbose) puts("STUB: WKBundlePageOverlaySetAccessibilityClient called");
    return NULL;
}

void* WKBundlePageOverlaySetNeedsDisplay(void) {
    if (verbose) puts("STUB: WKBundlePageOverlaySetNeedsDisplay called");
    return NULL;
}

void* WKBundlePagePostMessage(void) {
    if (verbose) puts("STUB: WKBundlePagePostMessage called");
    return NULL;
}

void* WKBundlePagePostSynchronousMessageForTesting(void) {
    if (verbose) puts("STUB: WKBundlePagePostSynchronousMessageForTesting called");
    return NULL;
}

void* WKBundlePageRegisterScrollOperationCompletionCallback(void) {
    if (verbose) puts("STUB: WKBundlePageRegisterScrollOperationCompletionCallback called");
    return NULL;
}

void* WKBundlePageRemoveAllUserContent(void) {
    if (verbose) puts("STUB: WKBundlePageRemoveAllUserContent called");
    return NULL;
}

void* WKBundlePageResetApplicationCacheOriginQuota(void) {
    if (verbose) puts("STUB: WKBundlePageResetApplicationCacheOriginQuota called");
    return NULL;
}

void* WKBundlePageResetTrackedRepaints(void) {
    if (verbose) puts("STUB: WKBundlePageResetTrackedRepaints called");
    return NULL;
}

void* WKBundlePageSetAppCacheMaximumSize(void) {
    if (verbose) puts("STUB: WKBundlePageSetAppCacheMaximumSize called");
    return NULL;
}

void* WKBundlePageSetApplicationCacheOriginQuota(void) {
    if (verbose) puts("STUB: WKBundlePageSetApplicationCacheOriginQuota called");
    return NULL;
}

void* WKBundlePageSetBottomOverhangImage(void) {
    if (verbose) puts("STUB: WKBundlePageSetBottomOverhangImage called");
    return NULL;
}

void* WKBundlePageSetComposition(void) {
    if (verbose) puts("STUB: WKBundlePageSetComposition called");
    return NULL;
}

void* WKBundlePageSetContextMenuClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetContextMenuClient called");
    return NULL;
}

void* WKBundlePageSetDefersLoading(void) {
    if (verbose) puts("STUB: WKBundlePageSetDefersLoading called");
    return NULL;
}

void* WKBundlePageSetEditorClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetEditorClient called");
    return NULL;
}

void* WKBundlePageSetEventThrottlingBehaviorOverride(void) {
    if (verbose) puts("STUB: WKBundlePageSetEventThrottlingBehaviorOverride called");
    return NULL;
}

void* WKBundlePageSetFooterBanner(void) {
    if (verbose) puts("STUB: WKBundlePageSetFooterBanner called");
    return NULL;
}

void* WKBundlePageSetFormClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetFormClient called");
    return NULL;
}

void* WKBundlePageSetFullScreenClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetFullScreenClient called");
    return NULL;
}

void* WKBundlePageSetHeaderBanner(void) {
    if (verbose) puts("STUB: WKBundlePageSetHeaderBanner called");
    return NULL;
}

void* WKBundlePageSetPageLoaderClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetPageLoaderClient called");
    return NULL;
}

void* WKBundlePageSetPageZoomFactor(void) {
    if (verbose) puts("STUB: WKBundlePageSetPageZoomFactor called");
    return NULL;
}

void* WKBundlePageSetPaintedObjectsCounterThreshold(void) {
    if (verbose) puts("STUB: WKBundlePageSetPaintedObjectsCounterThreshold called");
    return NULL;
}

void* WKBundlePageSetPolicyClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetPolicyClient called");
    return NULL;
}

void* WKBundlePageSetResourceLoadClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetResourceLoadClient called");
    return NULL;
}

void* WKBundlePageSetScaleAtOrigin(void) {
    if (verbose) puts("STUB: WKBundlePageSetScaleAtOrigin called");
    return NULL;
}

void* WKBundlePageSetTextZoomFactor(void) {
    if (verbose) puts("STUB: WKBundlePageSetTextZoomFactor called");
    return NULL;
}

void* WKBundlePageSetTopOverhangImage(void) {
    if (verbose) puts("STUB: WKBundlePageSetTopOverhangImage called");
    return NULL;
}

void* WKBundlePageSetTracksRepaints(void) {
    if (verbose) puts("STUB: WKBundlePageSetTracksRepaints called");
    return NULL;
}

void* WKBundlePageSetUIClient(void) {
    if (verbose) puts("STUB: WKBundlePageSetUIClient called");
    return NULL;
}

void* WKBundlePageSimulateMouseDown(void) {
    if (verbose) puts("STUB: WKBundlePageSimulateMouseDown called");
    return NULL;
}

void* WKBundlePageSimulateMouseMotion(void) {
    if (verbose) puts("STUB: WKBundlePageSimulateMouseMotion called");
    return NULL;
}

void* WKBundlePageSimulateMouseUp(void) {
    if (verbose) puts("STUB: WKBundlePageSimulateMouseUp called");
    return NULL;
}

void* WKBundlePageSizeAndMarginsInPixels(void) {
    if (verbose) puts("STUB: WKBundlePageSizeAndMarginsInPixels called");
    return NULL;
}

void* WKBundlePageStartMonitoringScrollOperations(void) {
    if (verbose) puts("STUB: WKBundlePageStartMonitoringScrollOperations called");
    return NULL;
}

void* WKBundlePageStopExtendingIncrementalRenderingSuppression(void) {
    if (verbose) puts("STUB: WKBundlePageStopExtendingIncrementalRenderingSuppression called");
    return NULL;
}

void* WKBundlePageStopLoading(void) {
    if (verbose) puts("STUB: WKBundlePageStopLoading called");
    return NULL;
}

void* WKBundlePageUninstallPageOverlay(void) {
    if (verbose) puts("STUB: WKBundlePageUninstallPageOverlay called");
    return NULL;
}

void* WKBundlePageUninstallPageOverlayWithAnimation(void) {
    if (verbose) puts("STUB: WKBundlePageUninstallPageOverlayWithAnimation called");
    return NULL;
}

void* WKBundlePageWillEnterFullScreen(void) {
    if (verbose) puts("STUB: WKBundlePageWillEnterFullScreen called");
    return NULL;
}

void* WKBundlePageWillExitFullScreen(void) {
    if (verbose) puts("STUB: WKBundlePageWillExitFullScreen called");
    return NULL;
}

void* WKBundlePostMessage(void) {
    if (verbose) puts("STUB: WKBundlePostMessage called");
    return NULL;
}

void* WKBundlePostSynchronousMessage(void) {
    if (verbose) puts("STUB: WKBundlePostSynchronousMessage called");
    return NULL;
}

void* WKBundleRangeHandleCopySnapshotWithOptions(void) {
    if (verbose) puts("STUB: WKBundleRangeHandleCopySnapshotWithOptions called");
    return NULL;
}

void* WKBundleRangeHandleCreate(void) {
    if (verbose) puts("STUB: WKBundleRangeHandleCreate called");
    return NULL;
}

void* WKBundleRangeHandleGetBoundingRectInWindowCoordinates(void) {
    if (verbose) puts("STUB: WKBundleRangeHandleGetBoundingRectInWindowCoordinates called");
    return NULL;
}

void* WKBundleRangeHandleGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleRangeHandleGetTypeID called");
    return NULL;
}

void* WKBundleRemoveAllUserContent(void) {
    if (verbose) puts("STUB: WKBundleRemoveAllUserContent called");
    return NULL;
}

void* WKBundleRemoveAllWebNotificationPermissions(void) {
    if (verbose) puts("STUB: WKBundleRemoveAllWebNotificationPermissions called");
    return NULL;
}

void* WKBundleRemoveOriginAccessWhitelistEntry(void) {
    if (verbose) puts("STUB: WKBundleRemoveOriginAccessWhitelistEntry called");
    return NULL;
}

void* WKBundleRemoveUserScript(void) {
    if (verbose) puts("STUB: WKBundleRemoveUserScript called");
    return NULL;
}

void* WKBundleRemoveUserScripts(void) {
    if (verbose) puts("STUB: WKBundleRemoveUserScripts called");
    return NULL;
}

void* WKBundleRemoveUserStyleSheet(void) {
    if (verbose) puts("STUB: WKBundleRemoveUserStyleSheet called");
    return NULL;
}

void* WKBundleRemoveUserStyleSheets(void) {
    if (verbose) puts("STUB: WKBundleRemoveUserStyleSheets called");
    return NULL;
}

void* WKBundleReportException(void) {
    if (verbose) puts("STUB: WKBundleReportException called");
    return NULL;
}

void* WKBundleResetOriginAccessWhitelists(void) {
    if (verbose) puts("STUB: WKBundleResetOriginAccessWhitelists called");
    return NULL;
}

void* WKBundleResourceLoadStatisticsNotifyObserver(void) {
    if (verbose) puts("STUB: WKBundleResourceLoadStatisticsNotifyObserver called");
    return NULL;
}

void* WKBundleScriptWorldClearWrappers(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldClearWrappers called");
    return NULL;
}

void* WKBundleScriptWorldCopyName(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldCopyName called");
    return NULL;
}

void* WKBundleScriptWorldCreateWorld(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldCreateWorld called");
    return NULL;
}

void* WKBundleScriptWorldGetTypeID(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldGetTypeID called");
    return NULL;
}

void* WKBundleScriptWorldMakeAllShadowRootsOpen(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldMakeAllShadowRootsOpen called");
    return NULL;
}

void* WKBundleScriptWorldNormalWorld(void) {
    if (verbose) puts("STUB: WKBundleScriptWorldNormalWorld called");
    return NULL;
}

void* WKBundleSetAllowFileAccessFromFileURLs(void) {
    if (verbose) puts("STUB: WKBundleSetAllowFileAccessFromFileURLs called");
    return NULL;
}

void* WKBundleSetAllowStorageAccessFromFileURLS(void) {
    if (verbose) puts("STUB: WKBundleSetAllowStorageAccessFromFileURLS called");
    return NULL;
}

void* WKBundleSetAllowUniversalAccessFromFileURLs(void) {
    if (verbose) puts("STUB: WKBundleSetAllowUniversalAccessFromFileURLs called");
    return NULL;
}

void* WKBundleSetAsyncFrameScrollingEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetAsyncFrameScrollingEnabled called");
    return NULL;
}

void* WKBundleSetAsynchronousSpellCheckingEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetAsynchronousSpellCheckingEnabled called");
    return NULL;
}

void* WKBundleSetAuthorAndUserStylesEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetAuthorAndUserStylesEnabled called");
    return NULL;
}

void* WKBundleSetClient(void) {
    if (verbose) puts("STUB: WKBundleSetClient called");
    return NULL;
}

void* WKBundleSetDatabaseQuota(void) {
    if (verbose) puts("STUB: WKBundleSetDatabaseQuota called");
    return NULL;
}

void* WKBundleSetFrameFlatteningEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetFrameFlatteningEnabled called");
    return NULL;
}

void* WKBundleSetJavaScriptCanAccessClipboard(void) {
    if (verbose) puts("STUB: WKBundleSetJavaScriptCanAccessClipboard called");
    return NULL;
}

void* WKBundleSetMinimumLogicalFontSize(void) {
    if (verbose) puts("STUB: WKBundleSetMinimumLogicalFontSize called");
    return NULL;
}

void* WKBundleSetPopupBlockingEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetPopupBlockingEnabled called");
    return NULL;
}

void* WKBundleSetPrivateBrowsingEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetPrivateBrowsingEnabled called");
    return NULL;
}

void* WKBundleSetServiceWorkerProxyCreationCallback(void) {
    if (verbose) puts("STUB: WKBundleSetServiceWorkerProxyCreationCallback called");
    return NULL;
}

void* WKBundleSetSpatialNavigationEnabled(void) {
    if (verbose) puts("STUB: WKBundleSetSpatialNavigationEnabled called");
    return NULL;
}

void* WKBundleSetTabKeyCyclesThroughElements(void) {
    if (verbose) puts("STUB: WKBundleSetTabKeyCyclesThroughElements called");
    return NULL;
}

void* WKBundleSetUseDashboardCompatibilityMode(void) {
    if (verbose) puts("STUB: WKBundleSetUseDashboardCompatibilityMode called");
    return NULL;
}

void* WKBundleSetUserStyleSheetLocation(void) {
    if (verbose) puts("STUB: WKBundleSetUserStyleSheetLocation called");
    return NULL;
}

void* WKBundleSetWebNotificationPermission(void) {
    if (verbose) puts("STUB: WKBundleSetWebNotificationPermission called");
    return NULL;
}

void* WKCertificateInfoCreateWithCertficateChain(void) {
    if (verbose) puts("STUB: WKCertificateInfoCreateWithCertficateChain called");
    return NULL;
}

void* WKCertificateInfoCreateWithServerTrust(void) {
    if (verbose) puts("STUB: WKCertificateInfoCreateWithServerTrust called");
    return NULL;
}

void* WKCertificateInfoGetCertificateChain(void) {
    if (verbose) puts("STUB: WKCertificateInfoGetCertificateChain called");
    return NULL;
}

void* WKCertificateInfoGetServerTrust(void) {
    if (verbose) puts("STUB: WKCertificateInfoGetServerTrust called");
    return NULL;
}

void* WKCertificateInfoGetTypeID(void) {
    if (verbose) puts("STUB: WKCertificateInfoGetTypeID called");
    return NULL;
}

void* WKConnectionGetTypeID(void) {
    if (verbose) puts("STUB: WKConnectionGetTypeID called");
    return NULL;
}

void* WKConnectionPostMessage(void) {
    if (verbose) puts("STUB: WKConnectionPostMessage called");
    return NULL;
}

void* WKConnectionSetConnectionClient(void) {
    if (verbose) puts("STUB: WKConnectionSetConnectionClient called");
    return NULL;
}

void* WKContextAddVisitedLink(void) {
    if (verbose) puts("STUB: WKContextAddVisitedLink called");
    return NULL;
}

void* WKContextAllowSpecificHTTPSCertificateForHost(void) {
    if (verbose) puts("STUB: WKContextAllowSpecificHTTPSCertificateForHost called");
    return NULL;
}

void* WKContextClearCachedCredentials(void) {
    if (verbose) puts("STUB: WKContextClearCachedCredentials called");
    return NULL;
}

void* WKContextClearPluginClientPolicies(void) {
    if (verbose) puts("STUB: WKContextClearPluginClientPolicies called");
    return NULL;
}

void* WKContextClearVisitedLinks(void) {
    if (verbose) puts("STUB: WKContextClearVisitedLinks called");
    return NULL;
}

void* WKContextConfigurationCopyApplicationCacheDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyApplicationCacheDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyDiskCacheDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyDiskCacheDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyIndexedDBDatabaseDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyIndexedDBDatabaseDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyInjectedBundlePath(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyInjectedBundlePath called");
    return NULL;
}

void* WKContextConfigurationCopyLocalStorageDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyLocalStorageDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyMediaKeysStorageDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyMediaKeysStorageDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyOverrideLanguages(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyOverrideLanguages called");
    return NULL;
}

void* WKContextConfigurationCopyResourceLoadStatisticsDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyResourceLoadStatisticsDirectory called");
    return NULL;
}

void* WKContextConfigurationCopyWebSQLDatabaseDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationCopyWebSQLDatabaseDirectory called");
    return NULL;
}

void* WKContextConfigurationCreate(void) {
    if (verbose) puts("STUB: WKContextConfigurationCreate called");
    return NULL;
}

void* WKContextConfigurationCreateWithLegacyOptions(void) {
    if (verbose) puts("STUB: WKContextConfigurationCreateWithLegacyOptions called");
    return NULL;
}

void* WKContextConfigurationFullySynchronousModeIsAllowedForTesting(void) {
    if (verbose) puts("STUB: WKContextConfigurationFullySynchronousModeIsAllowedForTesting called");
    return NULL;
}

void* WKContextConfigurationSetApplicationCacheDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetApplicationCacheDirectory called");
    return NULL;
}

void* WKContextConfigurationSetDiskCacheDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetDiskCacheDirectory called");
    return NULL;
}

void* WKContextConfigurationSetFullySynchronousModeIsAllowedForTesting(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetFullySynchronousModeIsAllowedForTesting called");
    return NULL;
}

void* WKContextConfigurationSetIndexedDBDatabaseDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetIndexedDBDatabaseDirectory called");
    return NULL;
}

void* WKContextConfigurationSetInjectedBundlePath(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetInjectedBundlePath called");
    return NULL;
}

void* WKContextConfigurationSetLocalStorageDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetLocalStorageDirectory called");
    return NULL;
}

void* WKContextConfigurationSetMediaKeysStorageDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetMediaKeysStorageDirectory called");
    return NULL;
}

void* WKContextConfigurationSetOverrideLanguages(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetOverrideLanguages called");
    return NULL;
}

void* WKContextConfigurationSetResourceLoadStatisticsDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetResourceLoadStatisticsDirectory called");
    return NULL;
}

void* WKContextConfigurationSetShouldCaptureAudioInUIProcess(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetShouldCaptureAudioInUIProcess called");
    return NULL;
}

void* WKContextConfigurationSetWebSQLDatabaseDirectory(void) {
    if (verbose) puts("STUB: WKContextConfigurationSetWebSQLDatabaseDirectory called");
    return NULL;
}

void* WKContextConfigurationShouldCaptureAudioInUIProcess(void) {
    if (verbose) puts("STUB: WKContextConfigurationShouldCaptureAudioInUIProcess called");
    return NULL;
}

void* WKContextCopyPlugInAutoStartOriginHashes(void) {
    if (verbose) puts("STUB: WKContextCopyPlugInAutoStartOriginHashes called");
    return NULL;
}

void* WKContextCopyPlugInInfoForBundleIdentifier(void) {
    if (verbose) puts("STUB: WKContextCopyPlugInInfoForBundleIdentifier called");
    return NULL;
}

void* WKContextCreate(void) {
    if (verbose) puts("STUB: WKContextCreate called");
    return NULL;
}

void* WKContextCreateWithConfiguration(void) {
    if (verbose) puts("STUB: WKContextCreateWithConfiguration called");
    return NULL;
}

void* WKContextCreateWithInjectedBundlePath(void) {
    if (verbose) puts("STUB: WKContextCreateWithInjectedBundlePath called");
    return NULL;
}

void* WKContextDisableProcessTermination(void) {
    if (verbose) puts("STUB: WKContextDisableProcessTermination called");
    return NULL;
}

void* WKContextDownloadURLRequest(void) {
    if (verbose) puts("STUB: WKContextDownloadURLRequest called");
    return NULL;
}

void* WKContextEnableProcessTermination(void) {
    if (verbose) puts("STUB: WKContextEnableProcessTermination called");
    return NULL;
}

void* WKContextGarbageCollectJavaScriptObjects(void) {
    if (verbose) puts("STUB: WKContextGarbageCollectJavaScriptObjects called");
    return NULL;
}

void* WKContextGetApplicationCacheManager(void) {
    if (verbose) puts("STUB: WKContextGetApplicationCacheManager called");
    return NULL;
}

void* WKContextGetCacheModel(void) {
    if (verbose) puts("STUB: WKContextGetCacheModel called");
    return NULL;
}

void* WKContextGetCookieManager(void) {
    if (verbose) puts("STUB: WKContextGetCookieManager called");
    return NULL;
}

void* WKContextGetDatabaseProcessIdentifier(void) {
    if (verbose) puts("STUB: WKContextGetDatabaseProcessIdentifier called");
    return NULL;
}

void* WKContextGetGeolocationManager(void) {
    if (verbose) puts("STUB: WKContextGetGeolocationManager called");
    return NULL;
}

void* WKContextGetGlobalStatistics(void) {
    if (verbose) puts("STUB: WKContextGetGlobalStatistics called");
    return NULL;
}

void* WKContextGetIconDatabase(void) {
    if (verbose) puts("STUB: WKContextGetIconDatabase called");
    return NULL;
}

void* WKContextGetInfoForInstalledPlugIns(void) {
    if (verbose) puts("STUB: WKContextGetInfoForInstalledPlugIns called");
    return NULL;
}

void* WKContextGetKeyValueStorageManager(void) {
    if (verbose) puts("STUB: WKContextGetKeyValueStorageManager called");
    return NULL;
}

void* WKContextGetMaximumNumberOfProcesses(void) {
    if (verbose) puts("STUB: WKContextGetMaximumNumberOfProcesses called");
    return NULL;
}

void* WKContextGetMediaSessionFocusManager(void) {
    if (verbose) puts("STUB: WKContextGetMediaSessionFocusManager called");
    return NULL;
}

void* WKContextGetNetworkProcessIdentifier(void) {
    if (verbose) puts("STUB: WKContextGetNetworkProcessIdentifier called");
    return NULL;
}

void* WKContextGetNotificationManager(void) {
    if (verbose) puts("STUB: WKContextGetNotificationManager called");
    return NULL;
}

void* WKContextGetResourceCacheManager(void) {
    if (verbose) puts("STUB: WKContextGetResourceCacheManager called");
    return NULL;
}

void* WKContextGetStatistics(void) {
    if (verbose) puts("STUB: WKContextGetStatistics called");
    return NULL;
}

void* WKContextGetStatisticsWithOptions(void) {
    if (verbose) puts("STUB: WKContextGetStatisticsWithOptions called");
    return NULL;
}

void* WKContextGetTypeID(void) {
    if (verbose) puts("STUB: WKContextGetTypeID called");
    return NULL;
}

void* WKContextGetWebsiteDataStore(void) {
    if (verbose) puts("STUB: WKContextGetWebsiteDataStore called");
    return NULL;
}

void* WKContextIsPlugInUpdateAvailable(void) {
    if (verbose) puts("STUB: WKContextIsPlugInUpdateAvailable called");
    return NULL;
}

void* WKContextJavaScriptConfigurationFileEnabled(void) {
    if (verbose) puts("STUB: WKContextJavaScriptConfigurationFileEnabled called");
    return NULL;
}

void* WKContextMenuCopySubmenuItems(void) {
    if (verbose) puts("STUB: WKContextMenuCopySubmenuItems called");
    return NULL;
}

void* WKContextMenuItemCopyTitle(void) {
    if (verbose) puts("STUB: WKContextMenuItemCopyTitle called");
    return NULL;
}

void* WKContextMenuItemCreateAsAction(void) {
    if (verbose) puts("STUB: WKContextMenuItemCreateAsAction called");
    return NULL;
}

void* WKContextMenuItemCreateAsCheckableAction(void) {
    if (verbose) puts("STUB: WKContextMenuItemCreateAsCheckableAction called");
    return NULL;
}

void* WKContextMenuItemCreateAsSubmenu(void) {
    if (verbose) puts("STUB: WKContextMenuItemCreateAsSubmenu called");
    return NULL;
}

void* WKContextMenuItemGetChecked(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetChecked called");
    return NULL;
}

void* WKContextMenuItemGetEnabled(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetEnabled called");
    return NULL;
}

void* WKContextMenuItemGetTag(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetTag called");
    return NULL;
}

void* WKContextMenuItemGetType(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetType called");
    return NULL;
}

void* WKContextMenuItemGetTypeID(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetTypeID called");
    return NULL;
}

void* WKContextMenuItemGetUserData(void) {
    if (verbose) puts("STUB: WKContextMenuItemGetUserData called");
    return NULL;
}

void* WKContextMenuItemSeparatorItem(void) {
    if (verbose) puts("STUB: WKContextMenuItemSeparatorItem called");
    return NULL;
}

void* WKContextMenuItemSetUserData(void) {
    if (verbose) puts("STUB: WKContextMenuItemSetUserData called");
    return NULL;
}

void* WKContextMenuListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKContextMenuListenerGetTypeID called");
    return NULL;
}

void* WKContextMenuListenerUseContextMenuItems(void) {
    if (verbose) puts("STUB: WKContextMenuListenerUseContextMenuItems called");
    return NULL;
}

void* WKContextPostMessageToInjectedBundle(void) {
    if (verbose) puts("STUB: WKContextPostMessageToInjectedBundle called");
    return NULL;
}

void* WKContextPreconnectToServer(void) {
    if (verbose) puts("STUB: WKContextPreconnectToServer called");
    return NULL;
}

void* WKContextRefreshPlugIns(void) {
    if (verbose) puts("STUB: WKContextRefreshPlugIns called");
    return NULL;
}

void* WKContextRegisterSchemeForCustomProtocol(void) {
    if (verbose) puts("STUB: WKContextRegisterSchemeForCustomProtocol called");
    return NULL;
}

void* WKContextRegisterURLSchemeAsBypassingContentSecurityPolicy(void) {
    if (verbose) puts("STUB: WKContextRegisterURLSchemeAsBypassingContentSecurityPolicy called");
    return NULL;
}

void* WKContextRegisterURLSchemeAsCachePartitioned(void) {
    if (verbose) puts("STUB: WKContextRegisterURLSchemeAsCachePartitioned called");
    return NULL;
}

void* WKContextRegisterURLSchemeAsEmptyDocument(void) {
    if (verbose) puts("STUB: WKContextRegisterURLSchemeAsEmptyDocument called");
    return NULL;
}

void* WKContextRegisterURLSchemeAsSecure(void) {
    if (verbose) puts("STUB: WKContextRegisterURLSchemeAsSecure called");
    return NULL;
}

void* WKContextResetHSTSHosts(void) {
    if (verbose) puts("STUB: WKContextResetHSTSHosts called");
    return NULL;
}

void* WKContextResetHSTSHostsAddedAfterDate(void) {
    if (verbose) puts("STUB: WKContextResetHSTSHostsAddedAfterDate called");
    return NULL;
}

void* WKContextResumeDownload(void) {
    if (verbose) puts("STUB: WKContextResumeDownload called");
    return NULL;
}

void* WKContextSetAdditionalPluginsDirectory(void) {
    if (verbose) puts("STUB: WKContextSetAdditionalPluginsDirectory called");
    return NULL;
}

void* WKContextSetAllowsAnySSLCertificateForServiceWorkerTesting(void) {
    if (verbose) puts("STUB: WKContextSetAllowsAnySSLCertificateForServiceWorkerTesting called");
    return NULL;
}

void* WKContextSetAllowsAnySSLCertificateForWebSocketTesting(void) {
    if (verbose) puts("STUB: WKContextSetAllowsAnySSLCertificateForWebSocketTesting called");
    return NULL;
}

void* WKContextSetAlwaysUsesComplexTextCodePath(void) {
    if (verbose) puts("STUB: WKContextSetAlwaysUsesComplexTextCodePath called");
    return NULL;
}

void* WKContextSetCacheModel(void) {
    if (verbose) puts("STUB: WKContextSetCacheModel called");
    return NULL;
}

void* WKContextSetCanHandleHTTPSServerTrustEvaluation(void) {
    if (verbose) puts("STUB: WKContextSetCanHandleHTTPSServerTrustEvaluation called");
    return NULL;
}

void* WKContextSetClient(void) {
    if (verbose) puts("STUB: WKContextSetClient called");
    return NULL;
}

void* WKContextSetConnectionClient(void) {
    if (verbose) puts("STUB: WKContextSetConnectionClient called");
    return NULL;
}

void* WKContextSetCookieStorageDirectory(void) {
    if (verbose) puts("STUB: WKContextSetCookieStorageDirectory called");
    return NULL;
}

void* WKContextSetDiskCacheSpeculativeValidationEnabled(void) {
    if (verbose) puts("STUB: WKContextSetDiskCacheSpeculativeValidationEnabled called");
    return NULL;
}

void* WKContextSetDomainRelaxationForbiddenForURLScheme(void) {
    if (verbose) puts("STUB: WKContextSetDomainRelaxationForbiddenForURLScheme called");
    return NULL;
}

void* WKContextSetDownloadClient(void) {
    if (verbose) puts("STUB: WKContextSetDownloadClient called");
    return NULL;
}

void* WKContextSetFontWhitelist(void) {
    if (verbose) puts("STUB: WKContextSetFontWhitelist called");
    return NULL;
}

void* WKContextSetHTTPPipeliningEnabled(void) {
    if (verbose) puts("STUB: WKContextSetHTTPPipeliningEnabled called");
    return NULL;
}

void* WKContextSetHistoryClient(void) {
    if (verbose) puts("STUB: WKContextSetHistoryClient called");
    return NULL;
}

void* WKContextSetIconDatabasePath(void) {
    if (verbose) puts("STUB: WKContextSetIconDatabasePath called");
    return NULL;
}

void* WKContextSetInitializationUserDataForInjectedBundle(void) {
    if (verbose) puts("STUB: WKContextSetInitializationUserDataForInjectedBundle called");
    return NULL;
}

void* WKContextSetInjectedBundleClient(void) {
    if (verbose) puts("STUB: WKContextSetInjectedBundleClient called");
    return NULL;
}

void* WKContextSetInvalidMessageFunction(void) {
    if (verbose) puts("STUB: WKContextSetInvalidMessageFunction called");
    return NULL;
}

void* WKContextSetJavaScriptConfigurationFileEnabled(void) {
    if (verbose) puts("STUB: WKContextSetJavaScriptConfigurationFileEnabled called");
    return NULL;
}

void* WKContextSetJavaScriptGarbageCollectorTimerEnabled(void) {
    if (verbose) puts("STUB: WKContextSetJavaScriptGarbageCollectorTimerEnabled called");
    return NULL;
}

void* WKContextSetMaximumNumberOfProcesses(void) {
    if (verbose) puts("STUB: WKContextSetMaximumNumberOfProcesses called");
    return NULL;
}

void* WKContextSetMemoryCacheDisabled(void) {
    if (verbose) puts("STUB: WKContextSetMemoryCacheDisabled called");
    return NULL;
}

void* WKContextSetPlugInAutoStartOriginHashes(void) {
    if (verbose) puts("STUB: WKContextSetPlugInAutoStartOriginHashes called");
    return NULL;
}

void* WKContextSetPlugInAutoStartOrigins(void) {
    if (verbose) puts("STUB: WKContextSetPlugInAutoStartOrigins called");
    return NULL;
}

void* WKContextSetPlugInAutoStartOriginsFilteringOutEntriesAddedAfterTime(void) {
    if (verbose) puts("STUB: WKContextSetPlugInAutoStartOriginsFilteringOutEntriesAddedAfterTime called");
    return NULL;
}

void* WKContextSetPluginLoadClientPolicy(void) {
    if (verbose) puts("STUB: WKContextSetPluginLoadClientPolicy called");
    return NULL;
}

void* WKContextSetProcessModel(void) {
    if (verbose) puts("STUB: WKContextSetProcessModel called");
    return NULL;
}

void* WKContextSetShouldUseFontSmoothing(void) {
    if (verbose) puts("STUB: WKContextSetShouldUseFontSmoothing called");
    return NULL;
}

void* WKContextSetUsesNetworkProcess(void) {
    if (verbose) puts("STUB: WKContextSetUsesNetworkProcess called");
    return NULL;
}

void* WKContextShouldBlockWebGL(void) {
    if (verbose) puts("STUB: WKContextShouldBlockWebGL called");
    return NULL;
}

void* WKContextShouldSuggestBlockWebGL(void) {
    if (verbose) puts("STUB: WKContextShouldSuggestBlockWebGL called");
    return NULL;
}

void* WKContextStartMemorySampler(void) {
    if (verbose) puts("STUB: WKContextStartMemorySampler called");
    return NULL;
}

void* WKContextStopMemorySampler(void) {
    if (verbose) puts("STUB: WKContextStopMemorySampler called");
    return NULL;
}

void* WKContextTerminateNetworkProcess(void) {
    if (verbose) puts("STUB: WKContextTerminateNetworkProcess called");
    return NULL;
}

void* WKContextTerminateServiceWorkerProcess(void) {
    if (verbose) puts("STUB: WKContextTerminateServiceWorkerProcess called");
    return NULL;
}

void* WKContextUnregisterSchemeForCustomProtocol(void) {
    if (verbose) puts("STUB: WKContextUnregisterSchemeForCustomProtocol called");
    return NULL;
}

void* WKContextUseTestingNetworkSession(void) {
    if (verbose) puts("STUB: WKContextUseTestingNetworkSession called");
    return NULL;
}

void* WKContextWarmInitialProcess(void) {
    if (verbose) puts("STUB: WKContextWarmInitialProcess called");
    return NULL;
}

void* WKCookieManagerDeleteAllCookies(void) {
    if (verbose) puts("STUB: WKCookieManagerDeleteAllCookies called");
    return NULL;
}

void* WKCookieManagerDeleteAllCookiesModifiedAfterDate(void) {
    if (verbose) puts("STUB: WKCookieManagerDeleteAllCookiesModifiedAfterDate called");
    return NULL;
}

void* WKCookieManagerDeleteCookiesForHostname(void) {
    if (verbose) puts("STUB: WKCookieManagerDeleteCookiesForHostname called");
    return NULL;
}

void* WKCookieManagerGetHTTPCookieAcceptPolicy(void) {
    if (verbose) puts("STUB: WKCookieManagerGetHTTPCookieAcceptPolicy called");
    return NULL;
}

void* WKCookieManagerGetHostnamesWithCookies(void) {
    if (verbose) puts("STUB: WKCookieManagerGetHostnamesWithCookies called");
    return NULL;
}

void* WKCookieManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKCookieManagerGetTypeID called");
    return NULL;
}

void* WKCookieManagerSetClient(void) {
    if (verbose) puts("STUB: WKCookieManagerSetClient called");
    return NULL;
}

void* WKCookieManagerSetCookieStoragePartitioningEnabled(void) {
    if (verbose) puts("STUB: WKCookieManagerSetCookieStoragePartitioningEnabled called");
    return NULL;
}

void* WKCookieManagerSetHTTPCookieAcceptPolicy(void) {
    if (verbose) puts("STUB: WKCookieManagerSetHTTPCookieAcceptPolicy called");
    return NULL;
}

void* WKCookieManagerSetStorageAccessAPIEnabled(void) {
    if (verbose) puts("STUB: WKCookieManagerSetStorageAccessAPIEnabled called");
    return NULL;
}

void* WKCookieManagerStartObservingCookieChanges(void) {
    if (verbose) puts("STUB: WKCookieManagerStartObservingCookieChanges called");
    return NULL;
}

void* WKCookieManagerStopObservingCookieChanges(void) {
    if (verbose) puts("STUB: WKCookieManagerStopObservingCookieChanges called");
    return NULL;
}

void* WKCredentialCopyUser(void) {
    if (verbose) puts("STUB: WKCredentialCopyUser called");
    return NULL;
}

void* WKCredentialCreate(void) {
    if (verbose) puts("STUB: WKCredentialCreate called");
    return NULL;
}

void* WKCredentialCreateWithCertificateInfo(void) {
    if (verbose) puts("STUB: WKCredentialCreateWithCertificateInfo called");
    return NULL;
}

void* WKCredentialGetTypeID(void) {
    if (verbose) puts("STUB: WKCredentialGetTypeID called");
    return NULL;
}

void* WKDataCreate(void) {
    if (verbose) puts("STUB: WKDataCreate called");
    return NULL;
}

void* WKDataGetBytes(void) {
    if (verbose) puts("STUB: WKDataGetBytes called");
    return NULL;
}

void* WKDataGetSize(void) {
    if (verbose) puts("STUB: WKDataGetSize called");
    return NULL;
}

void* WKDataGetTypeID(void) {
    if (verbose) puts("STUB: WKDataGetTypeID called");
    return NULL;
}

void* WKDictionaryCopyKeys(void) {
    if (verbose) puts("STUB: WKDictionaryCopyKeys called");
    return NULL;
}

void* WKDictionaryCreate(void) {
    if (verbose) puts("STUB: WKDictionaryCreate called");
    return NULL;
}

void* WKDictionaryGetItemForKey(void) {
    if (verbose) puts("STUB: WKDictionaryGetItemForKey called");
    return NULL;
}

void* WKDictionaryGetSize(void) {
    if (verbose) puts("STUB: WKDictionaryGetSize called");
    return NULL;
}

void* WKDictionaryGetTypeID(void) {
    if (verbose) puts("STUB: WKDictionaryGetTypeID called");
    return NULL;
}

void* WKDictionarySetItem(void) {
    if (verbose) puts("STUB: WKDictionarySetItem called");
    return NULL;
}

void* WKDoubleCreate(void) {
    if (verbose) puts("STUB: WKDoubleCreate called");
    return NULL;
}

void* WKDoubleGetTypeID(void) {
    if (verbose) puts("STUB: WKDoubleGetTypeID called");
    return NULL;
}

void* WKDoubleGetValue(void) {
    if (verbose) puts("STUB: WKDoubleGetValue called");
    return NULL;
}

void* WKDownloadCancel(void) {
    if (verbose) puts("STUB: WKDownloadCancel called");
    return NULL;
}

void* WKDownloadCopyRedirectChain(void) {
    if (verbose) puts("STUB: WKDownloadCopyRedirectChain called");
    return NULL;
}

void* WKDownloadCopyRequest(void) {
    if (verbose) puts("STUB: WKDownloadCopyRequest called");
    return NULL;
}

void* WKDownloadGetID(void) {
    if (verbose) puts("STUB: WKDownloadGetID called");
    return NULL;
}

void* WKDownloadGetOriginatingPage(void) {
    if (verbose) puts("STUB: WKDownloadGetOriginatingPage called");
    return NULL;
}

void* WKDownloadGetResumeData(void) {
    if (verbose) puts("STUB: WKDownloadGetResumeData called");
    return NULL;
}

void* WKDownloadGetTypeID(void) {
    if (verbose) puts("STUB: WKDownloadGetTypeID called");
    return NULL;
}

void* WKDownloadGetWasUserInitiated(void) {
    if (verbose) puts("STUB: WKDownloadGetWasUserInitiated called");
    return NULL;
}

void* WKErrorCopyCFError(void) {
    if (verbose) puts("STUB: WKErrorCopyCFError called");
    return NULL;
}

void* WKErrorCopyDomain(void) {
    if (verbose) puts("STUB: WKErrorCopyDomain called");
    return NULL;
}

void* WKErrorCopyFailingURL(void) {
    if (verbose) puts("STUB: WKErrorCopyFailingURL called");
    return NULL;
}

void* WKErrorCopyLocalizedDescription(void) {
    if (verbose) puts("STUB: WKErrorCopyLocalizedDescription called");
    return NULL;
}

void* WKErrorCopyWKErrorDomain(void) {
    if (verbose) puts("STUB: WKErrorCopyWKErrorDomain called");
    return NULL;
}

void* WKErrorCreateWithCFError(void) {
    if (verbose) puts("STUB: WKErrorCreateWithCFError called");
    return NULL;
}

void* WKErrorGetErrorCode(void) {
    if (verbose) puts("STUB: WKErrorGetErrorCode called");
    return NULL;
}

void* WKErrorGetTypeID(void) {
    if (verbose) puts("STUB: WKErrorGetTypeID called");
    return NULL;
}

void* WKFormSubmissionListenerContinue(void) {
    if (verbose) puts("STUB: WKFormSubmissionListenerContinue called");
    return NULL;
}

void* WKFormSubmissionListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKFormSubmissionListenerGetTypeID called");
    return NULL;
}

void* WKFrameCanProvideSource(void) {
    if (verbose) puts("STUB: WKFrameCanProvideSource called");
    return NULL;
}

void* WKFrameCanShowMIMEType(void) {
    if (verbose) puts("STUB: WKFrameCanShowMIMEType called");
    return NULL;
}

void* WKFrameCopyMIMEType(void) {
    if (verbose) puts("STUB: WKFrameCopyMIMEType called");
    return NULL;
}

void* WKFrameCopyProvisionalURL(void) {
    if (verbose) puts("STUB: WKFrameCopyProvisionalURL called");
    return NULL;
}

void* WKFrameCopyTitle(void) {
    if (verbose) puts("STUB: WKFrameCopyTitle called");
    return NULL;
}

void* WKFrameCopyURL(void) {
    if (verbose) puts("STUB: WKFrameCopyURL called");
    return NULL;
}

void* WKFrameCopyUnreachableURL(void) {
    if (verbose) puts("STUB: WKFrameCopyUnreachableURL called");
    return NULL;
}

void* WKFrameCreateFrameHandle(void) {
    if (verbose) puts("STUB: WKFrameCreateFrameHandle called");
    return NULL;
}

void* WKFrameCreateFrameInfo(void) {
    if (verbose) puts("STUB: WKFrameCreateFrameInfo called");
    return NULL;
}

void* WKFrameGetCertificateInfo(void) {
    if (verbose) puts("STUB: WKFrameGetCertificateInfo called");
    return NULL;
}

void* WKFrameGetFrameLoadState(void) {
    if (verbose) puts("STUB: WKFrameGetFrameLoadState called");
    return NULL;
}

void* WKFrameGetMainResourceData(void) {
    if (verbose) puts("STUB: WKFrameGetMainResourceData called");
    return NULL;
}

void* WKFrameGetPage(void) {
    if (verbose) puts("STUB: WKFrameGetPage called");
    return NULL;
}

void* WKFrameGetResourceData(void) {
    if (verbose) puts("STUB: WKFrameGetResourceData called");
    return NULL;
}

void* WKFrameGetTypeID(void) {
    if (verbose) puts("STUB: WKFrameGetTypeID called");
    return NULL;
}

void* WKFrameGetWebArchive(void) {
    if (verbose) puts("STUB: WKFrameGetWebArchive called");
    return NULL;
}

void* WKFrameHandleGetFrameID(void) {
    if (verbose) puts("STUB: WKFrameHandleGetFrameID called");
    return NULL;
}

void* WKFrameHandleGetTypeID(void) {
    if (verbose) puts("STUB: WKFrameHandleGetTypeID called");
    return NULL;
}

void* WKFrameInfoGetFrameHandleRef(void) {
    if (verbose) puts("STUB: WKFrameInfoGetFrameHandleRef called");
    return NULL;
}

void* WKFrameInfoGetTypeID(void) {
    if (verbose) puts("STUB: WKFrameInfoGetTypeID called");
    return NULL;
}

void* WKFrameIsDisplayingMarkupDocument(void) {
    if (verbose) puts("STUB: WKFrameIsDisplayingMarkupDocument called");
    return NULL;
}

void* WKFrameIsDisplayingStandaloneImageDocument(void) {
    if (verbose) puts("STUB: WKFrameIsDisplayingStandaloneImageDocument called");
    return NULL;
}

void* WKFrameIsFrameSet(void) {
    if (verbose) puts("STUB: WKFrameIsFrameSet called");
    return NULL;
}

void* WKFrameIsMainFrame(void) {
    if (verbose) puts("STUB: WKFrameIsMainFrame called");
    return NULL;
}

void* WKFramePolicyListenerDownload(void) {
    if (verbose) puts("STUB: WKFramePolicyListenerDownload called");
    return NULL;
}

void* WKFramePolicyListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKFramePolicyListenerGetTypeID called");
    return NULL;
}

void* WKFramePolicyListenerIgnore(void) {
    if (verbose) puts("STUB: WKFramePolicyListenerIgnore called");
    return NULL;
}

void* WKFramePolicyListenerUse(void) {
    if (verbose) puts("STUB: WKFramePolicyListenerUse called");
    return NULL;
}

void* WKFramePolicyListenerUseWithPolicies(void) {
    if (verbose) puts("STUB: WKFramePolicyListenerUseWithPolicies called");
    return NULL;
}

void* WKFrameStopLoading(void) {
    if (verbose) puts("STUB: WKFrameStopLoading called");
    return NULL;
}

void* WKGeolocationManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKGeolocationManagerGetTypeID called");
    return NULL;
}

void* WKGeolocationManagerProviderDidChangePosition(void) {
    if (verbose) puts("STUB: WKGeolocationManagerProviderDidChangePosition called");
    return NULL;
}

void* WKGeolocationManagerProviderDidFailToDeterminePosition(void) {
    if (verbose) puts("STUB: WKGeolocationManagerProviderDidFailToDeterminePosition called");
    return NULL;
}

void* WKGeolocationManagerProviderDidFailToDeterminePositionWithErrorMessage(void) {
    if (verbose) puts("STUB: WKGeolocationManagerProviderDidFailToDeterminePositionWithErrorMessage called");
    return NULL;
}

void* WKGeolocationManagerSetProvider(void) {
    if (verbose) puts("STUB: WKGeolocationManagerSetProvider called");
    return NULL;
}

void* WKGeolocationPermissionRequestAllow(void) {
    if (verbose) puts("STUB: WKGeolocationPermissionRequestAllow called");
    return NULL;
}

void* WKGeolocationPermissionRequestDeny(void) {
    if (verbose) puts("STUB: WKGeolocationPermissionRequestDeny called");
    return NULL;
}

void* WKGeolocationPermissionRequestGetTypeID(void) {
    if (verbose) puts("STUB: WKGeolocationPermissionRequestGetTypeID called");
    return NULL;
}

void* WKGeolocationPositionCreate(void) {
    if (verbose) puts("STUB: WKGeolocationPositionCreate called");
    return NULL;
}

void* WKGeolocationPositionCreate_b(void) {
    if (verbose) puts("STUB: WKGeolocationPositionCreate_b called");
    return NULL;
}

void* WKGeolocationPositionCreate_c(void) {
    if (verbose) puts("STUB: WKGeolocationPositionCreate_c called");
    return NULL;
}

void* WKGeolocationPositionGetTypeID(void) {
    if (verbose) puts("STUB: WKGeolocationPositionGetTypeID called");
    return NULL;
}

void* WKGetAPIURLSchemeTaskInstanceCount(void) {
    if (verbose) puts("STUB: WKGetAPIURLSchemeTaskInstanceCount called");
    return NULL;
}

void* WKGetTypeID(void) {
    if (verbose) puts("STUB: WKGetTypeID called");
    return NULL;
}

void* WKGetWebURLSchemeTaskInstanceCount(void) {
    if (verbose) puts("STUB: WKGetWebURLSchemeTaskInstanceCount called");
    return NULL;
}

void* WKHitTestResultCopyAbsoluteImageURL(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyAbsoluteImageURL called");
    return NULL;
}

void* WKHitTestResultCopyAbsoluteLinkURL(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyAbsoluteLinkURL called");
    return NULL;
}

void* WKHitTestResultCopyAbsoluteMediaURL(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyAbsoluteMediaURL called");
    return NULL;
}

void* WKHitTestResultCopyAbsolutePDFURL(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyAbsolutePDFURL called");
    return NULL;
}

void* WKHitTestResultCopyLinkLabel(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyLinkLabel called");
    return NULL;
}

void* WKHitTestResultCopyLinkTitle(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyLinkTitle called");
    return NULL;
}

void* WKHitTestResultCopyLookupText(void) {
    if (verbose) puts("STUB: WKHitTestResultCopyLookupText called");
    return NULL;
}

void* WKHitTestResultGetElementBoundingBox(void) {
    if (verbose) puts("STUB: WKHitTestResultGetElementBoundingBox called");
    return NULL;
}

void* WKHitTestResultGetTypeID(void) {
    if (verbose) puts("STUB: WKHitTestResultGetTypeID called");
    return NULL;
}

void* WKHitTestResultIsContentEditable(void) {
    if (verbose) puts("STUB: WKHitTestResultIsContentEditable called");
    return NULL;
}

void* WKIconDatabaseCheckIntegrityBeforeOpening(void) {
    if (verbose) puts("STUB: WKIconDatabaseCheckIntegrityBeforeOpening called");
    return NULL;
}

void* WKIconDatabaseClose(void) {
    if (verbose) puts("STUB: WKIconDatabaseClose called");
    return NULL;
}

void* WKIconDatabaseCopyIconDataForPageURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseCopyIconDataForPageURL called");
    return NULL;
}

void* WKIconDatabaseCopyIconURLForPageURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseCopyIconURLForPageURL called");
    return NULL;
}

void* WKIconDatabaseEnableDatabaseCleanup(void) {
    if (verbose) puts("STUB: WKIconDatabaseEnableDatabaseCleanup called");
    return NULL;
}

void* WKIconDatabaseGetTypeID(void) {
    if (verbose) puts("STUB: WKIconDatabaseGetTypeID called");
    return NULL;
}

void* WKIconDatabaseReleaseIconForURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseReleaseIconForURL called");
    return NULL;
}

void* WKIconDatabaseRemoveAllIcons(void) {
    if (verbose) puts("STUB: WKIconDatabaseRemoveAllIcons called");
    return NULL;
}

void* WKIconDatabaseRetainIconForURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseRetainIconForURL called");
    return NULL;
}

void* WKIconDatabaseSetIconDataForIconURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseSetIconDataForIconURL called");
    return NULL;
}

void* WKIconDatabaseSetIconDatabaseClient(void) {
    if (verbose) puts("STUB: WKIconDatabaseSetIconDatabaseClient called");
    return NULL;
}

void* WKIconDatabaseSetIconURLForPageURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseSetIconURLForPageURL called");
    return NULL;
}

void* WKIconDatabaseTryCopyCGImageArrayForURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseTryCopyCGImageArrayForURL called");
    return NULL;
}

void* WKIconDatabaseTryGetCGImageForURL(void) {
    if (verbose) puts("STUB: WKIconDatabaseTryGetCGImageForURL called");
    return NULL;
}

void* WKImageCreate(void) {
    if (verbose) puts("STUB: WKImageCreate called");
    return NULL;
}

void* WKImageCreateCGImage(void) {
    if (verbose) puts("STUB: WKImageCreateCGImage called");
    return NULL;
}

void* WKImageCreateFromCGImage(void) {
    if (verbose) puts("STUB: WKImageCreateFromCGImage called");
    return NULL;
}

void* WKImageGetSize(void) {
    if (verbose) puts("STUB: WKImageGetSize called");
    return NULL;
}

void* WKImageGetTypeID(void) {
    if (verbose) puts("STUB: WKImageGetTypeID called");
    return NULL;
}

void* WKInspectorAttach(void) {
    if (verbose) puts("STUB: WKInspectorAttach called");
    return NULL;
}

void* WKInspectorClose(void) {
    if (verbose) puts("STUB: WKInspectorClose called");
    return NULL;
}

void* WKInspectorConnect(void) {
    if (verbose) puts("STUB: WKInspectorConnect called");
    return NULL;
}

void* WKInspectorDetach(void) {
    if (verbose) puts("STUB: WKInspectorDetach called");
    return NULL;
}

void* WKInspectorGetPage(void) {
    if (verbose) puts("STUB: WKInspectorGetPage called");
    return NULL;
}

void* WKInspectorGetTypeID(void) {
    if (verbose) puts("STUB: WKInspectorGetTypeID called");
    return NULL;
}

void* WKInspectorHide(void) {
    if (verbose) puts("STUB: WKInspectorHide called");
    return NULL;
}

void* WKInspectorIsAttached(void) {
    if (verbose) puts("STUB: WKInspectorIsAttached called");
    return NULL;
}

void* WKInspectorIsConnected(void) {
    if (verbose) puts("STUB: WKInspectorIsConnected called");
    return NULL;
}

void* WKInspectorIsElementSelectionActive(void) {
    if (verbose) puts("STUB: WKInspectorIsElementSelectionActive called");
    return NULL;
}

void* WKInspectorIsFront(void) {
    if (verbose) puts("STUB: WKInspectorIsFront called");
    return NULL;
}

void* WKInspectorIsProfilingPage(void) {
    if (verbose) puts("STUB: WKInspectorIsProfilingPage called");
    return NULL;
}

void* WKInspectorIsVisible(void) {
    if (verbose) puts("STUB: WKInspectorIsVisible called");
    return NULL;
}

void* WKInspectorShow(void) {
    if (verbose) puts("STUB: WKInspectorShow called");
    return NULL;
}

void* WKInspectorShowConsole(void) {
    if (verbose) puts("STUB: WKInspectorShowConsole called");
    return NULL;
}

void* WKInspectorShowMainResourceForFrame(void) {
    if (verbose) puts("STUB: WKInspectorShowMainResourceForFrame called");
    return NULL;
}

void* WKInspectorShowResources(void) {
    if (verbose) puts("STUB: WKInspectorShowResources called");
    return NULL;
}

void* WKInspectorToggleElementSelection(void) {
    if (verbose) puts("STUB: WKInspectorToggleElementSelection called");
    return NULL;
}

void* WKInspectorTogglePageProfiling(void) {
    if (verbose) puts("STUB: WKInspectorTogglePageProfiling called");
    return NULL;
}

void* WKKeyValueStorageManagerDeleteAllEntries(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerDeleteAllEntries called");
    return NULL;
}

void* WKKeyValueStorageManagerDeleteEntriesForOrigin(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerDeleteEntriesForOrigin called");
    return NULL;
}

void* WKKeyValueStorageManagerGetCreationTimeKey(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetCreationTimeKey called");
    return NULL;
}

void* WKKeyValueStorageManagerGetKeyValueStorageOrigins(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetKeyValueStorageOrigins called");
    return NULL;
}

void* WKKeyValueStorageManagerGetModificationTimeKey(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetModificationTimeKey called");
    return NULL;
}

void* WKKeyValueStorageManagerGetOriginKey(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetOriginKey called");
    return NULL;
}

void* WKKeyValueStorageManagerGetStorageDetailsByOrigin(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetStorageDetailsByOrigin called");
    return NULL;
}

void* WKKeyValueStorageManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKKeyValueStorageManagerGetTypeID called");
    return NULL;
}

void* WKMediaSessionFocusManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKMediaSessionFocusManagerGetTypeID called");
    return NULL;
}

void* WKMediaSessionFocusManagerSetClient(void) {
    if (verbose) puts("STUB: WKMediaSessionFocusManagerSetClient called");
    return NULL;
}

void* WKMediaSessionFocusManagerSetVolumeOfFocusedMediaElement(void) {
    if (verbose) puts("STUB: WKMediaSessionFocusManagerSetVolumeOfFocusedMediaElement called");
    return NULL;
}

void* WKMediaSessionFocusManagerValueForPlaybackAttribute(void) {
    if (verbose) puts("STUB: WKMediaSessionFocusManagerValueForPlaybackAttribute called");
    return NULL;
}

void* WKMediaSessionMetadataCopyAlbum(void) {
    if (verbose) puts("STUB: WKMediaSessionMetadataCopyAlbum called");
    return NULL;
}

void* WKMediaSessionMetadataCopyArtist(void) {
    if (verbose) puts("STUB: WKMediaSessionMetadataCopyArtist called");
    return NULL;
}

void* WKMediaSessionMetadataCopyArtworkURL(void) {
    if (verbose) puts("STUB: WKMediaSessionMetadataCopyArtworkURL called");
    return NULL;
}

void* WKMediaSessionMetadataCopyTitle(void) {
    if (verbose) puts("STUB: WKMediaSessionMetadataCopyTitle called");
    return NULL;
}

void* WKMediaSessionMetadataGetTypeID(void) {
    if (verbose) puts("STUB: WKMediaSessionMetadataGetTypeID called");
    return NULL;
}

void* WKMutableArrayCreate(void) {
    if (verbose) puts("STUB: WKMutableArrayCreate called");
    return NULL;
}

void* WKMutableDictionaryCreate(void) {
    if (verbose) puts("STUB: WKMutableDictionaryCreate called");
    return NULL;
}

void* WKNavigationActionGetTypeID(void) {
    if (verbose) puts("STUB: WKNavigationActionGetTypeID called");
    return NULL;
}

void* WKNavigationDataCopyNavigationDestinationURL(void) {
    if (verbose) puts("STUB: WKNavigationDataCopyNavigationDestinationURL called");
    return NULL;
}

void* WKNavigationDataCopyOriginalRequest(void) {
    if (verbose) puts("STUB: WKNavigationDataCopyOriginalRequest called");
    return NULL;
}

void* WKNavigationDataCopyTitle(void) {
    if (verbose) puts("STUB: WKNavigationDataCopyTitle called");
    return NULL;
}

void* WKNavigationDataCopyURL(void) {
    if (verbose) puts("STUB: WKNavigationDataCopyURL called");
    return NULL;
}

void* WKNavigationDataGetTypeID(void) {
    if (verbose) puts("STUB: WKNavigationDataGetTypeID called");
    return NULL;
}

void* WKNavigationGetTypeID(void) {
    if (verbose) puts("STUB: WKNavigationGetTypeID called");
    return NULL;
}

void* WKNavigationResponseCanShowMIMEType(void) {
    if (verbose) puts("STUB: WKNavigationResponseCanShowMIMEType called");
    return NULL;
}

void* WKNavigationResponseGetTypeID(void) {
    if (verbose) puts("STUB: WKNavigationResponseGetTypeID called");
    return NULL;
}

void* WKNotificationCopyBody(void) {
    if (verbose) puts("STUB: WKNotificationCopyBody called");
    return NULL;
}

void* WKNotificationCopyDir(void) {
    if (verbose) puts("STUB: WKNotificationCopyDir called");
    return NULL;
}

void* WKNotificationCopyIconURL(void) {
    if (verbose) puts("STUB: WKNotificationCopyIconURL called");
    return NULL;
}

void* WKNotificationCopyLang(void) {
    if (verbose) puts("STUB: WKNotificationCopyLang called");
    return NULL;
}

void* WKNotificationCopyTag(void) {
    if (verbose) puts("STUB: WKNotificationCopyTag called");
    return NULL;
}

void* WKNotificationCopyTitle(void) {
    if (verbose) puts("STUB: WKNotificationCopyTitle called");
    return NULL;
}

void* WKNotificationGetID(void) {
    if (verbose) puts("STUB: WKNotificationGetID called");
    return NULL;
}

void* WKNotificationGetSecurityOrigin(void) {
    if (verbose) puts("STUB: WKNotificationGetSecurityOrigin called");
    return NULL;
}

void* WKNotificationGetTypeID(void) {
    if (verbose) puts("STUB: WKNotificationGetTypeID called");
    return NULL;
}

void* WKNotificationManagerGetLocalIDForTesting(void) {
    if (verbose) puts("STUB: WKNotificationManagerGetLocalIDForTesting called");
    return NULL;
}

void* WKNotificationManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKNotificationManagerGetTypeID called");
    return NULL;
}

void* WKNotificationManagerProviderDidClickNotification(void) {
    if (verbose) puts("STUB: WKNotificationManagerProviderDidClickNotification called");
    return NULL;
}

void* WKNotificationManagerProviderDidCloseNotifications(void) {
    if (verbose) puts("STUB: WKNotificationManagerProviderDidCloseNotifications called");
    return NULL;
}

void* WKNotificationManagerProviderDidRemoveNotificationPolicies(void) {
    if (verbose) puts("STUB: WKNotificationManagerProviderDidRemoveNotificationPolicies called");
    return NULL;
}

void* WKNotificationManagerProviderDidShowNotification(void) {
    if (verbose) puts("STUB: WKNotificationManagerProviderDidShowNotification called");
    return NULL;
}

void* WKNotificationManagerProviderDidUpdateNotificationPolicy(void) {
    if (verbose) puts("STUB: WKNotificationManagerProviderDidUpdateNotificationPolicy called");
    return NULL;
}

void* WKNotificationManagerSetProvider(void) {
    if (verbose) puts("STUB: WKNotificationManagerSetProvider called");
    return NULL;
}

void* WKNotificationPermissionRequestAllow(void) {
    if (verbose) puts("STUB: WKNotificationPermissionRequestAllow called");
    return NULL;
}

void* WKNotificationPermissionRequestDeny(void) {
    if (verbose) puts("STUB: WKNotificationPermissionRequestDeny called");
    return NULL;
}

void* WKNotificationPermissionRequestGetTypeID(void) {
    if (verbose) puts("STUB: WKNotificationPermissionRequestGetTypeID called");
    return NULL;
}

void* WKObjCTypeWrapperCreate(void) {
    if (verbose) puts("STUB: WKObjCTypeWrapperCreate called");
    return NULL;
}

void* WKObjCTypeWrapperGetObject(void) {
    if (verbose) puts("STUB: WKObjCTypeWrapperGetObject called");
    return NULL;
}

void* WKObjCTypeWrapperGetTypeID(void) {
    if (verbose) puts("STUB: WKObjCTypeWrapperGetTypeID called");
    return NULL;
}

void* WKOpenPanelParametersCopyAcceptedFileExtensions(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersCopyAcceptedFileExtensions called");
    return NULL;
}

void* WKOpenPanelParametersCopyAcceptedMIMETypes(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersCopyAcceptedMIMETypes called");
    return NULL;
}

void* WKOpenPanelParametersCopyCapture(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersCopyCapture called");
    return NULL;
}

void* WKOpenPanelParametersCopySelectedFileNames(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersCopySelectedFileNames called");
    return NULL;
}

void* WKOpenPanelParametersGetAllowsDirectories(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersGetAllowsDirectories called");
    return NULL;
}

void* WKOpenPanelParametersGetAllowsMultipleFiles(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersGetAllowsMultipleFiles called");
    return NULL;
}

void* WKOpenPanelParametersGetMediaCaptureType(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersGetMediaCaptureType called");
    return NULL;
}

void* WKOpenPanelParametersGetTypeID(void) {
    if (verbose) puts("STUB: WKOpenPanelParametersGetTypeID called");
    return NULL;
}

void* WKOpenPanelResultListenerCancel(void) {
    if (verbose) puts("STUB: WKOpenPanelResultListenerCancel called");
    return NULL;
}

void* WKOpenPanelResultListenerChooseFiles(void) {
    if (verbose) puts("STUB: WKOpenPanelResultListenerChooseFiles called");
    return NULL;
}

void* WKOpenPanelResultListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKOpenPanelResultListenerGetTypeID called");
    return NULL;
}

void* WKPageAreScrollbarAnimationsSuppressed(void) {
    if (verbose) puts("STUB: WKPageAreScrollbarAnimationsSuppressed called");
    return NULL;
}

void* WKPageBackgroundExtendsBeyondPage(void) {
    if (verbose) puts("STUB: WKPageBackgroundExtendsBeyondPage called");
    return NULL;
}

void* WKPageBeginPrinting(void) {
    if (verbose) puts("STUB: WKPageBeginPrinting called");
    return NULL;
}

void* WKPageCallAfterNextPresentationUpdate(void) {
    if (verbose) puts("STUB: WKPageCallAfterNextPresentationUpdate called");
    return NULL;
}

void* WKPageCanDelete(void) {
    if (verbose) puts("STUB: WKPageCanDelete called");
    return NULL;
}

void* WKPageCanGoBack(void) {
    if (verbose) puts("STUB: WKPageCanGoBack called");
    return NULL;
}

void* WKPageCanGoForward(void) {
    if (verbose) puts("STUB: WKPageCanGoForward called");
    return NULL;
}

void* WKPageCenterSelectionInVisibleArea(void) {
    if (verbose) puts("STUB: WKPageCenterSelectionInVisibleArea called");
    return NULL;
}

void* WKPageClearUserMediaState(void) {
    if (verbose) puts("STUB: WKPageClearUserMediaState called");
    return NULL;
}

void* WKPageClearWheelEventTestTrigger(void) {
    if (verbose) puts("STUB: WKPageClearWheelEventTestTrigger called");
    return NULL;
}

void* WKPageClose(void) {
    if (verbose) puts("STUB: WKPageClose called");
    return NULL;
}

void* WKPageComputePagesForPrinting(void) {
    if (verbose) puts("STUB: WKPageComputePagesForPrinting called");
    return NULL;
}

void* WKPageConfigurationCreate(void) {
    if (verbose) puts("STUB: WKPageConfigurationCreate called");
    return NULL;
}

void* WKPageConfigurationGetContext(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetContext called");
    return NULL;
}

void* WKPageConfigurationGetPageGroup(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetPageGroup called");
    return NULL;
}

void* WKPageConfigurationGetPreferences(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetPreferences called");
    return NULL;
}

void* WKPageConfigurationGetRelatedPage(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetRelatedPage called");
    return NULL;
}

void* WKPageConfigurationGetTypeID(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetTypeID called");
    return NULL;
}

void* WKPageConfigurationGetUserContentController(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetUserContentController called");
    return NULL;
}

void* WKPageConfigurationGetWebsiteDataStore(void) {
    if (verbose) puts("STUB: WKPageConfigurationGetWebsiteDataStore called");
    return NULL;
}

void* WKPageConfigurationSetBackgroundCPULimit(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetBackgroundCPULimit called");
    return NULL;
}

void* WKPageConfigurationSetContext(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetContext called");
    return NULL;
}

void* WKPageConfigurationSetInitialCapitalizationEnabled(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetInitialCapitalizationEnabled called");
    return NULL;
}

void* WKPageConfigurationSetPageGroup(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetPageGroup called");
    return NULL;
}

void* WKPageConfigurationSetPreferences(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetPreferences called");
    return NULL;
}

void* WKPageConfigurationSetRelatedPage(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetRelatedPage called");
    return NULL;
}

void* WKPageConfigurationSetUserContentController(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetUserContentController called");
    return NULL;
}

void* WKPageConfigurationSetWebsiteDataStore(void) {
    if (verbose) puts("STUB: WKPageConfigurationSetWebsiteDataStore called");
    return NULL;
}

void* WKPageCopyActiveURL(void) {
    if (verbose) puts("STUB: WKPageCopyActiveURL called");
    return NULL;
}

void* WKPageCopyApplicationNameForUserAgent(void) {
    if (verbose) puts("STUB: WKPageCopyApplicationNameForUserAgent called");
    return NULL;
}

void* WKPageCopyCommittedURL(void) {
    if (verbose) puts("STUB: WKPageCopyCommittedURL called");
    return NULL;
}

void* WKPageCopyCustomTextEncodingName(void) {
    if (verbose) puts("STUB: WKPageCopyCustomTextEncodingName called");
    return NULL;
}

void* WKPageCopyCustomUserAgent(void) {
    if (verbose) puts("STUB: WKPageCopyCustomUserAgent called");
    return NULL;
}

void* WKPageCopyPageConfiguration(void) {
    if (verbose) puts("STUB: WKPageCopyPageConfiguration called");
    return NULL;
}

void* WKPageCopyPendingAPIRequestURL(void) {
    if (verbose) puts("STUB: WKPageCopyPendingAPIRequestURL called");
    return NULL;
}

void* WKPageCopyProvisionalURL(void) {
    if (verbose) puts("STUB: WKPageCopyProvisionalURL called");
    return NULL;
}

void* WKPageCopyRelatedPages(void) {
    if (verbose) puts("STUB: WKPageCopyRelatedPages called");
    return NULL;
}

void* WKPageCopySessionState(void) {
    if (verbose) puts("STUB: WKPageCopySessionState called");
    return NULL;
}

void* WKPageCopyStandardUserAgentWithApplicationName(void) {
    if (verbose) puts("STUB: WKPageCopyStandardUserAgentWithApplicationName called");
    return NULL;
}

void* WKPageCopyTitle(void) {
    if (verbose) puts("STUB: WKPageCopyTitle called");
    return NULL;
}

void* WKPageCopyUserAgent(void) {
    if (verbose) puts("STUB: WKPageCopyUserAgent called");
    return NULL;
}

void* WKPageCountStringMatches(void) {
    if (verbose) puts("STUB: WKPageCountStringMatches called");
    return NULL;
}

void* WKPageCreateObservableState(void) {
    if (verbose) puts("STUB: WKPageCreateObservableState called");
    return NULL;
}

void* WKPageDidAllowPointerLock(void) {
    if (verbose) puts("STUB: WKPageDidAllowPointerLock called");
    return NULL;
}

void* WKPageDidDenyPointerLock(void) {
    if (verbose) puts("STUB: WKPageDidDenyPointerLock called");
    return NULL;
}

void* WKPageDrawPagesToPDF(void) {
    if (verbose) puts("STUB: WKPageDrawPagesToPDF called");
    return NULL;
}

void* WKPageEndPrinting(void) {
    if (verbose) puts("STUB: WKPageEndPrinting called");
    return NULL;
}

void* WKPageExecuteCommand(void) {
    if (verbose) puts("STUB: WKPageExecuteCommand called");
    return NULL;
}

void* WKPageFindString(void) {
    if (verbose) puts("STUB: WKPageFindString called");
    return NULL;
}

void* WKPageFindStringMatches(void) {
    if (verbose) puts("STUB: WKPageFindStringMatches called");
    return NULL;
}

void* WKPageFixedLayoutSize(void) {
    if (verbose) puts("STUB: WKPageFixedLayoutSize called");
    return NULL;
}

void* WKPageForceRepaint(void) {
    if (verbose) puts("STUB: WKPageForceRepaint called");
    return NULL;
}

void* WKPageGetAddsVisitedLinks(void) {
    if (verbose) puts("STUB: WKPageGetAddsVisitedLinks called");
    return NULL;
}

void* WKPageGetAllowsRemoteInspection(void) {
    if (verbose) puts("STUB: WKPageGetAllowsRemoteInspection called");
    return NULL;
}

void* WKPageGetApplicationManifest_b(void) {
    if (verbose) puts("STUB: WKPageGetApplicationManifest_b called");
    return NULL;
}

void* WKPageGetBackForwardList(void) {
    if (verbose) puts("STUB: WKPageGetBackForwardList called");
    return NULL;
}

void* WKPageGetBackingScaleFactor(void) {
    if (verbose) puts("STUB: WKPageGetBackingScaleFactor called");
    return NULL;
}

void* WKPageGetBytecodeProfile(void) {
    if (verbose) puts("STUB: WKPageGetBytecodeProfile called");
    return NULL;
}

void* WKPageGetContentsAsMHTMLData(void) {
    if (verbose) puts("STUB: WKPageGetContentsAsMHTMLData called");
    return NULL;
}

void* WKPageGetContentsAsString(void) {
    if (verbose) puts("STUB: WKPageGetContentsAsString called");
    return NULL;
}

void* WKPageGetContext(void) {
    if (verbose) puts("STUB: WKPageGetContext called");
    return NULL;
}

void* WKPageGetEstimatedProgress(void) {
    if (verbose) puts("STUB: WKPageGetEstimatedProgress called");
    return NULL;
}

void* WKPageGetFocusedFrame(void) {
    if (verbose) puts("STUB: WKPageGetFocusedFrame called");
    return NULL;
}

void* WKPageGetFrameSetLargestFrame(void) {
    if (verbose) puts("STUB: WKPageGetFrameSetLargestFrame called");
    return NULL;
}

void* WKPageGetFullscreenDelegate(void) {
    if (verbose) puts("STUB: WKPageGetFullscreenDelegate called");
    return NULL;
}

void* WKPageGetGapBetweenPages(void) {
    if (verbose) puts("STUB: WKPageGetGapBetweenPages called");
    return NULL;
}

void* WKPageGetImageForFindMatch(void) {
    if (verbose) puts("STUB: WKPageGetImageForFindMatch called");
    return NULL;
}

void* WKPageGetInspector(void) {
    if (verbose) puts("STUB: WKPageGetInspector called");
    return NULL;
}

void* WKPageGetIsControlledByAutomation(void) {
    if (verbose) puts("STUB: WKPageGetIsControlledByAutomation called");
    return NULL;
}

void* WKPageGetMainFrame(void) {
    if (verbose) puts("STUB: WKPageGetMainFrame called");
    return NULL;
}

void* WKPageGetMediaCaptureEnabled(void) {
    if (verbose) puts("STUB: WKPageGetMediaCaptureEnabled called");
    return NULL;
}

void* WKPageGetMediaState(void) {
    if (verbose) puts("STUB: WKPageGetMediaState called");
    return NULL;
}

void* WKPageGetObjectRegistry(void) {
    if (verbose) puts("STUB: WKPageGetObjectRegistry called");
    return NULL;
}

void* WKPageGetPageCount(void) {
    if (verbose) puts("STUB: WKPageGetPageCount called");
    return NULL;
}

void* WKPageGetPageGroup(void) {
    if (verbose) puts("STUB: WKPageGetPageGroup called");
    return NULL;
}

void* WKPageGetPageLength(void) {
    if (verbose) puts("STUB: WKPageGetPageLength called");
    return NULL;
}

void* WKPageGetPageZoomFactor(void) {
    if (verbose) puts("STUB: WKPageGetPageZoomFactor called");
    return NULL;
}

void* WKPageGetPaginationBehavesLikeColumns(void) {
    if (verbose) puts("STUB: WKPageGetPaginationBehavesLikeColumns called");
    return NULL;
}

void* WKPageGetPaginationLineGridEnabled(void) {
    if (verbose) puts("STUB: WKPageGetPaginationLineGridEnabled called");
    return NULL;
}

void* WKPageGetPaginationMode(void) {
    if (verbose) puts("STUB: WKPageGetPaginationMode called");
    return NULL;
}

void* WKPageGetProcessIdentifier(void) {
    if (verbose) puts("STUB: WKPageGetProcessIdentifier called");
    return NULL;
}

void* WKPageGetRenderTreeSize(void) {
    if (verbose) puts("STUB: WKPageGetRenderTreeSize called");
    return NULL;
}

void* WKPageGetResourceCachingDisabled(void) {
    if (verbose) puts("STUB: WKPageGetResourceCachingDisabled called");
    return NULL;
}

void* WKPageGetSamplingProfilerOutput(void) {
    if (verbose) puts("STUB: WKPageGetSamplingProfilerOutput called");
    return NULL;
}

void* WKPageGetScaleFactor(void) {
    if (verbose) puts("STUB: WKPageGetScaleFactor called");
    return NULL;
}

void* WKPageGetScrollPinningBehavior(void) {
    if (verbose) puts("STUB: WKPageGetScrollPinningBehavior called");
    return NULL;
}

void* WKPageGetSelectionAsWebArchiveData(void) {
    if (verbose) puts("STUB: WKPageGetSelectionAsWebArchiveData called");
    return NULL;
}

void* WKPageGetSessionBackForwardListItemValueType(void) {
    if (verbose) puts("STUB: WKPageGetSessionBackForwardListItemValueType called");
    return NULL;
}

void* WKPageGetSessionHistoryURLValueType(void) {
    if (verbose) puts("STUB: WKPageGetSessionHistoryURLValueType called");
    return NULL;
}

void* WKPageGetSourceForFrame(void) {
    if (verbose) puts("STUB: WKPageGetSourceForFrame called");
    return NULL;
}

void* WKPageGetTextZoomFactor(void) {
    if (verbose) puts("STUB: WKPageGetTextZoomFactor called");
    return NULL;
}

void* WKPageGetTypeID(void) {
    if (verbose) puts("STUB: WKPageGetTypeID called");
    return NULL;
}

void* WKPageGoBack(void) {
    if (verbose) puts("STUB: WKPageGoBack called");
    return NULL;
}

void* WKPageGoForward(void) {
    if (verbose) puts("STUB: WKPageGoForward called");
    return NULL;
}

void* WKPageGoToBackForwardListItem(void) {
    if (verbose) puts("STUB: WKPageGoToBackForwardListItem called");
    return NULL;
}

void* WKPageGroupAddUserScript(void) {
    if (verbose) puts("STUB: WKPageGroupAddUserScript called");
    return NULL;
}

void* WKPageGroupAddUserStyleSheet(void) {
    if (verbose) puts("STUB: WKPageGroupAddUserStyleSheet called");
    return NULL;
}

void* WKPageGroupCreateWithIdentifier(void) {
    if (verbose) puts("STUB: WKPageGroupCreateWithIdentifier called");
    return NULL;
}

void* WKPageGroupGetPreferences(void) {
    if (verbose) puts("STUB: WKPageGroupGetPreferences called");
    return NULL;
}

void* WKPageGroupGetTypeID(void) {
    if (verbose) puts("STUB: WKPageGroupGetTypeID called");
    return NULL;
}

void* WKPageGroupGetUserContentController(void) {
    if (verbose) puts("STUB: WKPageGroupGetUserContentController called");
    return NULL;
}

void* WKPageGroupRemoveAllUserScripts(void) {
    if (verbose) puts("STUB: WKPageGroupRemoveAllUserScripts called");
    return NULL;
}

void* WKPageGroupRemoveAllUserStyleSheets(void) {
    if (verbose) puts("STUB: WKPageGroupRemoveAllUserStyleSheets called");
    return NULL;
}

void* WKPageGroupSetPreferences(void) {
    if (verbose) puts("STUB: WKPageGroupSetPreferences called");
    return NULL;
}

void* WKPageHandleMediaEvent(void) {
    if (verbose) puts("STUB: WKPageHandleMediaEvent called");
    return NULL;
}

void* WKPageHasHorizontalScrollbar(void) {
    if (verbose) puts("STUB: WKPageHasHorizontalScrollbar called");
    return NULL;
}

void* WKPageHasMediaSessionWithActiveMediaElements(void) {
    if (verbose) puts("STUB: WKPageHasMediaSessionWithActiveMediaElements called");
    return NULL;
}

void* WKPageHasSelectedRange(void) {
    if (verbose) puts("STUB: WKPageHasSelectedRange called");
    return NULL;
}

void* WKPageHasVerticalScrollbar(void) {
    if (verbose) puts("STUB: WKPageHasVerticalScrollbar called");
    return NULL;
}

void* WKPageHideFindUI(void) {
    if (verbose) puts("STUB: WKPageHideFindUI called");
    return NULL;
}

void* WKPageHorizontalRubberBandingIsEnabled(void) {
    if (verbose) puts("STUB: WKPageHorizontalRubberBandingIsEnabled called");
    return NULL;
}

void* WKPageIsClosed(void) {
    if (verbose) puts("STUB: WKPageIsClosed called");
    return NULL;
}

void* WKPageIsContentEditable(void) {
    if (verbose) puts("STUB: WKPageIsContentEditable called");
    return NULL;
}

void* WKPageIsPinnedToBottomSide(void) {
    if (verbose) puts("STUB: WKPageIsPinnedToBottomSide called");
    return NULL;
}

void* WKPageIsPinnedToLeftSide(void) {
    if (verbose) puts("STUB: WKPageIsPinnedToLeftSide called");
    return NULL;
}

void* WKPageIsPinnedToRightSide(void) {
    if (verbose) puts("STUB: WKPageIsPinnedToRightSide called");
    return NULL;
}

void* WKPageIsPinnedToTopSide(void) {
    if (verbose) puts("STUB: WKPageIsPinnedToTopSide called");
    return NULL;
}

void* WKPageIsPlayingAudio(void) {
    if (verbose) puts("STUB: WKPageIsPlayingAudio called");
    return NULL;
}

void* WKPageIsPlayingVideoInEnhancedFullscreen(void) {
    if (verbose) puts("STUB: WKPageIsPlayingVideoInEnhancedFullscreen called");
    return NULL;
}

void* WKPageIsURLKnownHSTSHost(void) {
    if (verbose) puts("STUB: WKPageIsURLKnownHSTSHost called");
    return NULL;
}

void* WKPageIsWebProcessResponsive(void) {
    if (verbose) puts("STUB: WKPageIsWebProcessResponsive called");
    return NULL;
}

void* WKPageListenForLayoutMilestones(void) {
    if (verbose) puts("STUB: WKPageListenForLayoutMilestones called");
    return NULL;
}

void* WKPageLoadAlternateHTMLString(void) {
    if (verbose) puts("STUB: WKPageLoadAlternateHTMLString called");
    return NULL;
}

void* WKPageLoadAlternateHTMLStringWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadAlternateHTMLStringWithUserData called");
    return NULL;
}

void* WKPageLoadData(void) {
    if (verbose) puts("STUB: WKPageLoadData called");
    return NULL;
}

void* WKPageLoadDataWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadDataWithUserData called");
    return NULL;
}

void* WKPageLoadFile(void) {
    if (verbose) puts("STUB: WKPageLoadFile called");
    return NULL;
}

void* WKPageLoadFileWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadFileWithUserData called");
    return NULL;
}

void* WKPageLoadHTMLString(void) {
    if (verbose) puts("STUB: WKPageLoadHTMLString called");
    return NULL;
}

void* WKPageLoadHTMLStringWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadHTMLStringWithUserData called");
    return NULL;
}

void* WKPageLoadPlainTextString(void) {
    if (verbose) puts("STUB: WKPageLoadPlainTextString called");
    return NULL;
}

void* WKPageLoadPlainTextStringWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadPlainTextStringWithUserData called");
    return NULL;
}

void* WKPageLoadURL(void) {
    if (verbose) puts("STUB: WKPageLoadURL called");
    return NULL;
}

void* WKPageLoadURLRequest(void) {
    if (verbose) puts("STUB: WKPageLoadURLRequest called");
    return NULL;
}

void* WKPageLoadURLRequestWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadURLRequestWithUserData called");
    return NULL;
}

void* WKPageLoadURLWithShouldOpenExternalURLsPolicy(void) {
    if (verbose) puts("STUB: WKPageLoadURLWithShouldOpenExternalURLsPolicy called");
    return NULL;
}

void* WKPageLoadURLWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadURLWithUserData called");
    return NULL;
}

void* WKPageLoadWebArchiveData(void) {
    if (verbose) puts("STUB: WKPageLoadWebArchiveData called");
    return NULL;
}

void* WKPageLoadWebArchiveDataWithUserData(void) {
    if (verbose) puts("STUB: WKPageLoadWebArchiveDataWithUserData called");
    return NULL;
}

void* WKPageLookUpFrameFromHandle(void) {
    if (verbose) puts("STUB: WKPageLookUpFrameFromHandle called");
    return NULL;
}

void* WKPagePostMessageToInjectedBundle(void) {
    if (verbose) puts("STUB: WKPagePostMessageToInjectedBundle called");
    return NULL;
}

void* WKPageReload(void) {
    if (verbose) puts("STUB: WKPageReload called");
    return NULL;
}

void* WKPageReloadExpiredOnly(void) {
    if (verbose) puts("STUB: WKPageReloadExpiredOnly called");
    return NULL;
}

void* WKPageReloadFromOrigin(void) {
    if (verbose) puts("STUB: WKPageReloadFromOrigin called");
    return NULL;
}

void* WKPageReloadWithoutContentBlockers(void) {
    if (verbose) puts("STUB: WKPageReloadWithoutContentBlockers called");
    return NULL;
}

void* WKPageRenderTreeExternalRepresentation(void) {
    if (verbose) puts("STUB: WKPageRenderTreeExternalRepresentation called");
    return NULL;
}

void* WKPageRestoreFromSessionState(void) {
    if (verbose) puts("STUB: WKPageRestoreFromSessionState called");
    return NULL;
}

void* WKPageRestoreFromSessionStateWithoutNavigation(void) {
    if (verbose) puts("STUB: WKPageRestoreFromSessionStateWithoutNavigation called");
    return NULL;
}

void* WKPageRubberBandsAtBottom(void) {
    if (verbose) puts("STUB: WKPageRubberBandsAtBottom called");
    return NULL;
}

void* WKPageRubberBandsAtLeft(void) {
    if (verbose) puts("STUB: WKPageRubberBandsAtLeft called");
    return NULL;
}

void* WKPageRubberBandsAtRight(void) {
    if (verbose) puts("STUB: WKPageRubberBandsAtRight called");
    return NULL;
}

void* WKPageRubberBandsAtTop(void) {
    if (verbose) puts("STUB: WKPageRubberBandsAtTop called");
    return NULL;
}

void* WKPageRunBeforeUnloadConfirmPanelResultListenerCall(void) {
    if (verbose) puts("STUB: WKPageRunBeforeUnloadConfirmPanelResultListenerCall called");
    return NULL;
}

void* WKPageRunBeforeUnloadConfirmPanelResultListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKPageRunBeforeUnloadConfirmPanelResultListenerGetTypeID called");
    return NULL;
}

void* WKPageRunJavaScriptAlertResultListenerCall(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptAlertResultListenerCall called");
    return NULL;
}

void* WKPageRunJavaScriptAlertResultListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptAlertResultListenerGetTypeID called");
    return NULL;
}

void* WKPageRunJavaScriptConfirmResultListenerCall(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptConfirmResultListenerCall called");
    return NULL;
}

void* WKPageRunJavaScriptConfirmResultListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptConfirmResultListenerGetTypeID called");
    return NULL;
}

void* WKPageRunJavaScriptInMainFrame(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptInMainFrame called");
    return NULL;
}

void* WKPageRunJavaScriptInMainFrame_b(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptInMainFrame_b called");
    return NULL;
}

void* WKPageRunJavaScriptPromptResultListenerCall(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptPromptResultListenerCall called");
    return NULL;
}

void* WKPageRunJavaScriptPromptResultListenerGetTypeID(void) {
    if (verbose) puts("STUB: WKPageRunJavaScriptPromptResultListenerGetTypeID called");
    return NULL;
}

void* WKPageSelectContextMenuItem(void) {
    if (verbose) puts("STUB: WKPageSelectContextMenuItem called");
    return NULL;
}

void* WKPageSelectFindMatch(void) {
    if (verbose) puts("STUB: WKPageSelectFindMatch called");
    return NULL;
}

void* WKPageSetAddsVisitedLinks(void) {
    if (verbose) puts("STUB: WKPageSetAddsVisitedLinks called");
    return NULL;
}

void* WKPageSetAllowsRemoteInspection(void) {
    if (verbose) puts("STUB: WKPageSetAllowsRemoteInspection called");
    return NULL;
}

void* WKPageSetApplicationNameForUserAgent(void) {
    if (verbose) puts("STUB: WKPageSetApplicationNameForUserAgent called");
    return NULL;
}

void* WKPageSetBackgroundExtendsBeyondPage(void) {
    if (verbose) puts("STUB: WKPageSetBackgroundExtendsBeyondPage called");
    return NULL;
}

void* WKPageSetControlledByAutomation(void) {
    if (verbose) puts("STUB: WKPageSetControlledByAutomation called");
    return NULL;
}

void* WKPageSetCustomBackingScaleFactor(void) {
    if (verbose) puts("STUB: WKPageSetCustomBackingScaleFactor called");
    return NULL;
}

void* WKPageSetCustomTextEncodingName(void) {
    if (verbose) puts("STUB: WKPageSetCustomTextEncodingName called");
    return NULL;
}

void* WKPageSetCustomUserAgent(void) {
    if (verbose) puts("STUB: WKPageSetCustomUserAgent called");
    return NULL;
}

void* WKPageSetEnableHorizontalRubberBanding(void) {
    if (verbose) puts("STUB: WKPageSetEnableHorizontalRubberBanding called");
    return NULL;
}

void* WKPageSetEnableVerticalRubberBanding(void) {
    if (verbose) puts("STUB: WKPageSetEnableVerticalRubberBanding called");
    return NULL;
}

void* WKPageSetFixedLayoutSize(void) {
    if (verbose) puts("STUB: WKPageSetFixedLayoutSize called");
    return NULL;
}

void* WKPageSetFullscreenDelegate(void) {
    if (verbose) puts("STUB: WKPageSetFullscreenDelegate called");
    return NULL;
}

void* WKPageSetGapBetweenPages(void) {
    if (verbose) puts("STUB: WKPageSetGapBetweenPages called");
    return NULL;
}

void* WKPageSetIgnoresViewportScaleLimits(void) {
    if (verbose) puts("STUB: WKPageSetIgnoresViewportScaleLimits called");
    return NULL;
}

void* WKPageSetMaintainsInactiveSelection(void) {
    if (verbose) puts("STUB: WKPageSetMaintainsInactiveSelection called");
    return NULL;
}

void* WKPageSetMayStartMediaWhenInWindow(void) {
    if (verbose) puts("STUB: WKPageSetMayStartMediaWhenInWindow called");
    return NULL;
}

void* WKPageSetMediaCaptureEnabled(void) {
    if (verbose) puts("STUB: WKPageSetMediaCaptureEnabled called");
    return NULL;
}

void* WKPageSetMediaVolume(void) {
    if (verbose) puts("STUB: WKPageSetMediaVolume called");
    return NULL;
}

void* WKPageSetMuted(void) {
    if (verbose) puts("STUB: WKPageSetMuted called");
    return NULL;
}

void* WKPageSetPageAndTextZoomFactors(void) {
    if (verbose) puts("STUB: WKPageSetPageAndTextZoomFactors called");
    return NULL;
}

void* WKPageSetPageContextMenuClient(void) {
    if (verbose) puts("STUB: WKPageSetPageContextMenuClient called");
    return NULL;
}

void* WKPageSetPageDiagnosticLoggingClient(void) {
    if (verbose) puts("STUB: WKPageSetPageDiagnosticLoggingClient called");
    return NULL;
}

void* WKPageSetPageFindClient(void) {
    if (verbose) puts("STUB: WKPageSetPageFindClient called");
    return NULL;
}

void* WKPageSetPageFindMatchesClient(void) {
    if (verbose) puts("STUB: WKPageSetPageFindMatchesClient called");
    return NULL;
}

void* WKPageSetPageFormClient(void) {
    if (verbose) puts("STUB: WKPageSetPageFormClient called");
    return NULL;
}

void* WKPageSetPageInjectedBundleClient(void) {
    if (verbose) puts("STUB: WKPageSetPageInjectedBundleClient called");
    return NULL;
}

void* WKPageSetPageLength(void) {
    if (verbose) puts("STUB: WKPageSetPageLength called");
    return NULL;
}

void* WKPageSetPageLoaderClient(void) {
    if (verbose) puts("STUB: WKPageSetPageLoaderClient called");
    return NULL;
}

void* WKPageSetPageNavigationClient(void) {
    if (verbose) puts("STUB: WKPageSetPageNavigationClient called");
    return NULL;
}

void* WKPageSetPagePolicyClient(void) {
    if (verbose) puts("STUB: WKPageSetPagePolicyClient called");
    return NULL;
}

void* WKPageSetPageUIClient(void) {
    if (verbose) puts("STUB: WKPageSetPageUIClient called");
    return NULL;
}

void* WKPageSetPageZoomFactor(void) {
    if (verbose) puts("STUB: WKPageSetPageZoomFactor called");
    return NULL;
}

void* WKPageSetPaginationBehavesLikeColumns(void) {
    if (verbose) puts("STUB: WKPageSetPaginationBehavesLikeColumns called");
    return NULL;
}

void* WKPageSetPaginationLineGridEnabled(void) {
    if (verbose) puts("STUB: WKPageSetPaginationLineGridEnabled called");
    return NULL;
}

void* WKPageSetPaginationMode(void) {
    if (verbose) puts("STUB: WKPageSetPaginationMode called");
    return NULL;
}

void* WKPageSetResourceCachingDisabled(void) {
    if (verbose) puts("STUB: WKPageSetResourceCachingDisabled called");
    return NULL;
}

void* WKPageSetRubberBandsAtBottom(void) {
    if (verbose) puts("STUB: WKPageSetRubberBandsAtBottom called");
    return NULL;
}

void* WKPageSetRubberBandsAtLeft(void) {
    if (verbose) puts("STUB: WKPageSetRubberBandsAtLeft called");
    return NULL;
}

void* WKPageSetRubberBandsAtRight(void) {
    if (verbose) puts("STUB: WKPageSetRubberBandsAtRight called");
    return NULL;
}

void* WKPageSetRubberBandsAtTop(void) {
    if (verbose) puts("STUB: WKPageSetRubberBandsAtTop called");
    return NULL;
}

void* WKPageSetScaleFactor(void) {
    if (verbose) puts("STUB: WKPageSetScaleFactor called");
    return NULL;
}

void* WKPageSetScrollPinningBehavior(void) {
    if (verbose) puts("STUB: WKPageSetScrollPinningBehavior called");
    return NULL;
}

void* WKPageSetSuppressScrollbarAnimations(void) {
    if (verbose) puts("STUB: WKPageSetSuppressScrollbarAnimations called");
    return NULL;
}

void* WKPageSetTextZoomFactor(void) {
    if (verbose) puts("STUB: WKPageSetTextZoomFactor called");
    return NULL;
}

void* WKPageSetUseFixedLayout(void) {
    if (verbose) puts("STUB: WKPageSetUseFixedLayout called");
    return NULL;
}

void* WKPageSetUserContentExtensionsEnabled(void) {
    if (verbose) puts("STUB: WKPageSetUserContentExtensionsEnabled called");
    return NULL;
}

void* WKPageStopLoading(void) {
    if (verbose) puts("STUB: WKPageStopLoading called");
    return NULL;
}

void* WKPageSupportsTextEncoding(void) {
    if (verbose) puts("STUB: WKPageSupportsTextEncoding called");
    return NULL;
}

void* WKPageSupportsTextZoom(void) {
    if (verbose) puts("STUB: WKPageSupportsTextZoom called");
    return NULL;
}

void* WKPageTerminate(void) {
    if (verbose) puts("STUB: WKPageTerminate called");
    return NULL;
}

void* WKPageTryClose(void) {
    if (verbose) puts("STUB: WKPageTryClose called");
    return NULL;
}

void* WKPageTryRestoreScrollPosition(void) {
    if (verbose) puts("STUB: WKPageTryRestoreScrollPosition called");
    return NULL;
}

void* WKPageUpdateWebsitePolicies(void) {
    if (verbose) puts("STUB: WKPageUpdateWebsitePolicies called");
    return NULL;
}

void* WKPageUseFixedLayout(void) {
    if (verbose) puts("STUB: WKPageUseFixedLayout called");
    return NULL;
}

void* WKPageValidateCommand(void) {
    if (verbose) puts("STUB: WKPageValidateCommand called");
    return NULL;
}

void* WKPageVerticalRubberBandingIsEnabled(void) {
    if (verbose) puts("STUB: WKPageVerticalRubberBandingIsEnabled called");
    return NULL;
}

void* WKPageWillHandleHorizontalScrollEvents(void) {
    if (verbose) puts("STUB: WKPageWillHandleHorizontalScrollEvents called");
    return NULL;
}

void* WKPlugInInfoBundleIdentifierKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoBundleIdentifierKey called");
    return NULL;
}

void* WKPlugInInfoIsSandboxedKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoIsSandboxedKey called");
    return NULL;
}

void* WKPlugInInfoLoadPolicyKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoLoadPolicyKey called");
    return NULL;
}

void* WKPlugInInfoPathKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoPathKey called");
    return NULL;
}

void* WKPlugInInfoUpdatePastLastBlockedVersionIsKnownAvailableKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoUpdatePastLastBlockedVersionIsKnownAvailableKey called");
    return NULL;
}

void* WKPlugInInfoVersionKey(void) {
    if (verbose) puts("STUB: WKPlugInInfoVersionKey called");
    return NULL;
}

void* WKPlugInInformationReplacementObscuredKey(void) {
    if (verbose) puts("STUB: WKPlugInInformationReplacementObscuredKey called");
    return NULL;
}

void* WKPluginInformationBundleIdentifierKey(void) {
    if (verbose) puts("STUB: WKPluginInformationBundleIdentifierKey called");
    return NULL;
}

void* WKPluginInformationBundleShortVersionKey(void) {
    if (verbose) puts("STUB: WKPluginInformationBundleShortVersionKey called");
    return NULL;
}

void* WKPluginInformationBundleVersionKey(void) {
    if (verbose) puts("STUB: WKPluginInformationBundleVersionKey called");
    return NULL;
}

void* WKPluginInformationDefaultLoadPolicyKey(void) {
    if (verbose) puts("STUB: WKPluginInformationDefaultLoadPolicyKey called");
    return NULL;
}

void* WKPluginInformationDisplayNameKey(void) {
    if (verbose) puts("STUB: WKPluginInformationDisplayNameKey called");
    return NULL;
}

void* WKPluginInformationFrameURLKey(void) {
    if (verbose) puts("STUB: WKPluginInformationFrameURLKey called");
    return NULL;
}

void* WKPluginInformationHasSandboxProfileKey(void) {
    if (verbose) puts("STUB: WKPluginInformationHasSandboxProfileKey called");
    return NULL;
}

void* WKPluginInformationMIMETypeKey(void) {
    if (verbose) puts("STUB: WKPluginInformationMIMETypeKey called");
    return NULL;
}

void* WKPluginInformationPageURLKey(void) {
    if (verbose) puts("STUB: WKPluginInformationPageURLKey called");
    return NULL;
}

void* WKPluginInformationPathKey(void) {
    if (verbose) puts("STUB: WKPluginInformationPathKey called");
    return NULL;
}

void* WKPluginInformationPluginURLKey(void) {
    if (verbose) puts("STUB: WKPluginInformationPluginURLKey called");
    return NULL;
}

void* WKPluginInformationPluginspageAttributeURLKey(void) {
    if (verbose) puts("STUB: WKPluginInformationPluginspageAttributeURLKey called");
    return NULL;
}

void* WKPluginInformationUpdatePastLastBlockedVersionIsKnownAvailableKey(void) {
    if (verbose) puts("STUB: WKPluginInformationUpdatePastLastBlockedVersionIsKnownAvailableKey called");
    return NULL;
}

void* WKPointCreate(void) {
    if (verbose) puts("STUB: WKPointCreate called");
    return NULL;
}

void* WKPointGetTypeID(void) {
    if (verbose) puts("STUB: WKPointGetTypeID called");
    return NULL;
}

void* WKPointGetValue(void) {
    if (verbose) puts("STUB: WKPointGetValue called");
    return NULL;
}

void* WKPreferencesCopyCursiveFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopyCursiveFontFamily called");
    return NULL;
}

void* WKPreferencesCopyDefaultTextEncodingName(void) {
    if (verbose) puts("STUB: WKPreferencesCopyDefaultTextEncodingName called");
    return NULL;
}

void* WKPreferencesCopyFTPDirectoryTemplatePath(void) {
    if (verbose) puts("STUB: WKPreferencesCopyFTPDirectoryTemplatePath called");
    return NULL;
}

void* WKPreferencesCopyFantasyFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopyFantasyFontFamily called");
    return NULL;
}

void* WKPreferencesCopyFixedFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopyFixedFontFamily called");
    return NULL;
}

void* WKPreferencesCopyMediaContentTypesRequiringHardwareSupport(void) {
    if (verbose) puts("STUB: WKPreferencesCopyMediaContentTypesRequiringHardwareSupport called");
    return NULL;
}

void* WKPreferencesCopyPictographFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopyPictographFontFamily called");
    return NULL;
}

void* WKPreferencesCopySansSerifFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopySansSerifFontFamily called");
    return NULL;
}

void* WKPreferencesCopySerifFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopySerifFontFamily called");
    return NULL;
}

void* WKPreferencesCopyStandardFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesCopyStandardFontFamily called");
    return NULL;
}

void* WKPreferencesCreate(void) {
    if (verbose) puts("STUB: WKPreferencesCreate called");
    return NULL;
}

void* WKPreferencesCreateCopy(void) {
    if (verbose) puts("STUB: WKPreferencesCreateCopy called");
    return NULL;
}

void* WKPreferencesCreateWithIdentifier(void) {
    if (verbose) puts("STUB: WKPreferencesCreateWithIdentifier called");
    return NULL;
}

void* WKPreferencesEnableAllExperimentalFeatures(void) {
    if (verbose) puts("STUB: WKPreferencesEnableAllExperimentalFeatures called");
    return NULL;
}

void* WKPreferencesGetAVFoundationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAVFoundationEnabled called");
    return NULL;
}

void* WKPreferencesGetAVFoundationNSURLSessionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAVFoundationNSURLSessionEnabled called");
    return NULL;
}

void* WKPreferencesGetAccelerated2DCanvasEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAccelerated2DCanvasEnabled called");
    return NULL;
}

void* WKPreferencesGetAcceleratedCompositingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAcceleratedCompositingEnabled called");
    return NULL;
}

void* WKPreferencesGetAcceleratedCompositingForOverflowScrollEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAcceleratedCompositingForOverflowScrollEnabled called");
    return NULL;
}

void* WKPreferencesGetAcceleratedDrawingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAcceleratedDrawingEnabled called");
    return NULL;
}

void* WKPreferencesGetAccessibilityObjectModelEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAccessibilityObjectModelEnabled called");
    return NULL;
}

void* WKPreferencesGetAggressiveTileRetentionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAggressiveTileRetentionEnabled called");
    return NULL;
}

void* WKPreferencesGetAllowMediaContentTypesRequiringHardwareSupportAsFallback(void) {
    if (verbose) puts("STUB: WKPreferencesGetAllowMediaContentTypesRequiringHardwareSupportAsFallback called");
    return NULL;
}

void* WKPreferencesGetAllowsAirPlayForMediaPlayback(void) {
    if (verbose) puts("STUB: WKPreferencesGetAllowsAirPlayForMediaPlayback called");
    return NULL;
}

void* WKPreferencesGetAllowsPictureInPictureMediaPlayback(void) {
    if (verbose) puts("STUB: WKPreferencesGetAllowsPictureInPictureMediaPlayback called");
    return NULL;
}

void* WKPreferencesGetAnimatedImageAsyncDecodingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAnimatedImageAsyncDecodingEnabled called");
    return NULL;
}

void* WKPreferencesGetAntialiasedFontDilationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAntialiasedFontDilationEnabled called");
    return NULL;
}

void* WKPreferencesGetApplePayCapabilityDisclosureAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesGetApplePayCapabilityDisclosureAllowed called");
    return NULL;
}

void* WKPreferencesGetApplePayEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetApplePayEnabled called");
    return NULL;
}

void* WKPreferencesGetApplicationChromeModeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetApplicationChromeModeEnabled called");
    return NULL;
}

void* WKPreferencesGetArtificialPluginInitializationDelayEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetArtificialPluginInitializationDelayEnabled called");
    return NULL;
}

void* WKPreferencesGetAsynchronousPluginInitializationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAsynchronousPluginInitializationEnabled called");
    return NULL;
}

void* WKPreferencesGetAsynchronousPluginInitializationEnabledForAllPlugins(void) {
    if (verbose) puts("STUB: WKPreferencesGetAsynchronousPluginInitializationEnabledForAllPlugins called");
    return NULL;
}

void* WKPreferencesGetAsynchronousSpellCheckingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAsynchronousSpellCheckingEnabled called");
    return NULL;
}

void* WKPreferencesGetAttachmentElementEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAttachmentElementEnabled called");
    return NULL;
}

void* WKPreferencesGetAudioPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesGetAudioPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesGetAuthorAndUserStylesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAuthorAndUserStylesEnabled called");
    return NULL;
}

void* WKPreferencesGetAutostartOriginPlugInSnapshottingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetAutostartOriginPlugInSnapshottingEnabled called");
    return NULL;
}

void* WKPreferencesGetBackspaceKeyNavigationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetBackspaceKeyNavigationEnabled called");
    return NULL;
}

void* WKPreferencesGetBeaconAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetBeaconAPIEnabled called");
    return NULL;
}

void* WKPreferencesGetCSSAnimationTriggersEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetCSSAnimationTriggersEnabled called");
    return NULL;
}

void* WKPreferencesGetCanvasUsesAcceleratedDrawing(void) {
    if (verbose) puts("STUB: WKPreferencesGetCanvasUsesAcceleratedDrawing called");
    return NULL;
}

void* WKPreferencesGetCaretBrowsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetCaretBrowsingEnabled called");
    return NULL;
}

void* WKPreferencesGetCompositingBordersVisible(void) {
    if (verbose) puts("STUB: WKPreferencesGetCompositingBordersVisible called");
    return NULL;
}

void* WKPreferencesGetCompositingRepaintCountersVisible(void) {
    if (verbose) puts("STUB: WKPreferencesGetCompositingRepaintCountersVisible called");
    return NULL;
}

void* WKPreferencesGetCookieEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetCookieEnabled called");
    return NULL;
}

void* WKPreferencesGetCustomPasteboardDataEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetCustomPasteboardDataEnabled called");
    return NULL;
}

void* WKPreferencesGetDNSPrefetchingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDNSPrefetchingEnabled called");
    return NULL;
}

void* WKPreferencesGetDOMPasteAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesGetDOMPasteAllowed called");
    return NULL;
}

void* WKPreferencesGetDOMTimersThrottlingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDOMTimersThrottlingEnabled called");
    return NULL;
}

void* WKPreferencesGetDataTransferItemsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDataTransferItemsEnabled called");
    return NULL;
}

void* WKPreferencesGetDatabasesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDatabasesEnabled called");
    return NULL;
}

void* WKPreferencesGetDefaultFixedFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesGetDefaultFixedFontSize called");
    return NULL;
}

void* WKPreferencesGetDefaultFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesGetDefaultFontSize called");
    return NULL;
}

void* WKPreferencesGetDeferredCSSParserEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDeferredCSSParserEnabled called");
    return NULL;
}

void* WKPreferencesGetDeveloperExtrasEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDeveloperExtrasEnabled called");
    return NULL;
}

void* WKPreferencesGetDiagnosticLoggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDiagnosticLoggingEnabled called");
    return NULL;
}

void* WKPreferencesGetDirectoryUploadEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDirectoryUploadEnabled called");
    return NULL;
}

void* WKPreferencesGetDisplayContentsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDisplayContentsEnabled called");
    return NULL;
}

void* WKPreferencesGetDownloadAttributeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetDownloadAttributeEnabled called");
    return NULL;
}

void* WKPreferencesGetEditableLinkBehavior(void) {
    if (verbose) puts("STUB: WKPreferencesGetEditableLinkBehavior called");
    return NULL;
}

void* WKPreferencesGetEnableInheritURIQueryComponent(void) {
    if (verbose) puts("STUB: WKPreferencesGetEnableInheritURIQueryComponent called");
    return NULL;
}

void* WKPreferencesGetEncodingDetectorEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetEncodingDetectorEnabled called");
    return NULL;
}

void* WKPreferencesGetEnumeratingAllNetworkInterfacesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetEnumeratingAllNetworkInterfacesEnabled called");
    return NULL;
}

void* WKPreferencesGetFetchAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetFetchAPIEnabled called");
    return NULL;
}

void* WKPreferencesGetFetchAPIKeepAliveEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetFetchAPIKeepAliveEnabled called");
    return NULL;
}

void* WKPreferencesGetFileAccessFromFileURLsAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesGetFileAccessFromFileURLsAllowed called");
    return NULL;
}

void* WKPreferencesGetFontSmoothingLevel(void) {
    if (verbose) puts("STUB: WKPreferencesGetFontSmoothingLevel called");
    return NULL;
}

void* WKPreferencesGetForceFTPDirectoryListings(void) {
    if (verbose) puts("STUB: WKPreferencesGetForceFTPDirectoryListings called");
    return NULL;
}

void* WKPreferencesGetForceSoftwareWebGLRendering(void) {
    if (verbose) puts("STUB: WKPreferencesGetForceSoftwareWebGLRendering called");
    return NULL;
}

void* WKPreferencesGetFrameFlatteningEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetFrameFlatteningEnabled called");
    return NULL;
}

void* WKPreferencesGetFullScreenEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetFullScreenEnabled called");
    return NULL;
}

void* WKPreferencesGetGamepadsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetGamepadsEnabled called");
    return NULL;
}

void* WKPreferencesGetHTTPEquivEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetHTTPEquivEnabled called");
    return NULL;
}

void* WKPreferencesGetHiddenPageCSSAnimationSuspensionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetHiddenPageCSSAnimationSuspensionEnabled called");
    return NULL;
}

void* WKPreferencesGetHiddenPageDOMTimerThrottlingAutoIncreases(void) {
    if (verbose) puts("STUB: WKPreferencesGetHiddenPageDOMTimerThrottlingAutoIncreases called");
    return NULL;
}

void* WKPreferencesGetHiddenPageDOMTimerThrottlingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetHiddenPageDOMTimerThrottlingEnabled called");
    return NULL;
}

void* WKPreferencesGetHixie76WebSocketProtocolEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetHixie76WebSocketProtocolEnabled called");
    return NULL;
}

void* WKPreferencesGetHyperlinkAuditingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetHyperlinkAuditingEnabled called");
    return NULL;
}

void* WKPreferencesGetICECandidateFilteringEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetICECandidateFilteringEnabled called");
    return NULL;
}

void* WKPreferencesGetIgnoreViewportScalingConstraints(void) {
    if (verbose) puts("STUB: WKPreferencesGetIgnoreViewportScalingConstraints called");
    return NULL;
}

void* WKPreferencesGetImageControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetImageControlsEnabled called");
    return NULL;
}

void* WKPreferencesGetInactiveMediaCaptureSteamRepromptIntervalInMinutes(void) {
    if (verbose) puts("STUB: WKPreferencesGetInactiveMediaCaptureSteamRepromptIntervalInMinutes called");
    return NULL;
}

void* WKPreferencesGetIncrementalRenderingSuppressionTimeout(void) {
    if (verbose) puts("STUB: WKPreferencesGetIncrementalRenderingSuppressionTimeout called");
    return NULL;
}

void* WKPreferencesGetInlineMediaPlaybackRequiresPlaysInlineAttribute(void) {
    if (verbose) puts("STUB: WKPreferencesGetInlineMediaPlaybackRequiresPlaysInlineAttribute called");
    return NULL;
}

void* WKPreferencesGetInspectorAdditionsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetInspectorAdditionsEnabled called");
    return NULL;
}

void* WKPreferencesGetInspectorUsesWebKitUserInterface(void) {
    if (verbose) puts("STUB: WKPreferencesGetInspectorUsesWebKitUserInterface called");
    return NULL;
}

void* WKPreferencesGetInteractiveFormValidationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetInteractiveFormValidationEnabled called");
    return NULL;
}

void* WKPreferencesGetIntersectionObserverEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetIntersectionObserverEnabled called");
    return NULL;
}

void* WKPreferencesGetInvisibleMediaAutoplayPermitted(void) {
    if (verbose) puts("STUB: WKPreferencesGetInvisibleMediaAutoplayPermitted called");
    return NULL;
}

void* WKPreferencesGetIsSecureContextAttributeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetIsSecureContextAttributeEnabled called");
    return NULL;
}

void* WKPreferencesGetJavaEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaEnabled called");
    return NULL;
}

void* WKPreferencesGetJavaEnabledForLocalFiles(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaEnabledForLocalFiles called");
    return NULL;
}

void* WKPreferencesGetJavaScriptCanAccessClipboard(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaScriptCanAccessClipboard called");
    return NULL;
}

void* WKPreferencesGetJavaScriptCanOpenWindowsAutomatically(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaScriptCanOpenWindowsAutomatically called");
    return NULL;
}

void* WKPreferencesGetJavaScriptEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaScriptEnabled called");
    return NULL;
}

void* WKPreferencesGetJavaScriptMarkupEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaScriptMarkupEnabled called");
    return NULL;
}

void* WKPreferencesGetJavaScriptRuntimeFlags(void) {
    if (verbose) puts("STUB: WKPreferencesGetJavaScriptRuntimeFlags called");
    return NULL;
}

void* WKPreferencesGetLargeImageAsyncDecodingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLargeImageAsyncDecodingEnabled called");
    return NULL;
}

void* WKPreferencesGetLegacyEncryptedMediaAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLegacyEncryptedMediaAPIEnabled called");
    return NULL;
}

void* WKPreferencesGetLinkPreloadEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLinkPreloadEnabled called");
    return NULL;
}

void* WKPreferencesGetLoadsImagesAutomatically(void) {
    if (verbose) puts("STUB: WKPreferencesGetLoadsImagesAutomatically called");
    return NULL;
}

void* WKPreferencesGetLoadsSiteIconsIgnoringImageLoadingPreference(void) {
    if (verbose) puts("STUB: WKPreferencesGetLoadsSiteIconsIgnoringImageLoadingPreference called");
    return NULL;
}

void* WKPreferencesGetLocalFileContentSniffingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLocalFileContentSniffingEnabled called");
    return NULL;
}

void* WKPreferencesGetLocalStorageEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLocalStorageEnabled called");
    return NULL;
}

void* WKPreferencesGetLogsPageMessagesToSystemConsoleEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLogsPageMessagesToSystemConsoleEnabled called");
    return NULL;
}

void* WKPreferencesGetLongMousePressEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetLongMousePressEnabled called");
    return NULL;
}

void* WKPreferencesGetMainContentUserGestureOverrideEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMainContentUserGestureOverrideEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaCapabilitiesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaCapabilitiesEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaCaptureRequiresSecureConnection(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaCaptureRequiresSecureConnection called");
    return NULL;
}

void* WKPreferencesGetMediaControlsScaleWithPageZoom(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaControlsScaleWithPageZoom called");
    return NULL;
}

void* WKPreferencesGetMediaDevicesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaDevicesEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaPlaybackAllowsInline(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaPlaybackAllowsInline called");
    return NULL;
}

void* WKPreferencesGetMediaPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesGetMediaPreloadingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaPreloadingEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaSourceEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaSourceEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaStreamEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaStreamEnabled called");
    return NULL;
}

void* WKPreferencesGetMediaUserGestureInheritsFromDocument(void) {
    if (verbose) puts("STUB: WKPreferencesGetMediaUserGestureInheritsFromDocument called");
    return NULL;
}

void* WKPreferencesGetMenuItemElementEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMenuItemElementEnabled called");
    return NULL;
}

void* WKPreferencesGetMetaRefreshEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMetaRefreshEnabled called");
    return NULL;
}

void* WKPreferencesGetMinimumFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesGetMinimumFontSize called");
    return NULL;
}

void* WKPreferencesGetMinimumZoomFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesGetMinimumZoomFontSize called");
    return NULL;
}

void* WKPreferencesGetMockCaptureDevicesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMockCaptureDevicesEnabled called");
    return NULL;
}

void* WKPreferencesGetMockScrollbarsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetMockScrollbarsEnabled called");
    return NULL;
}

void* WKPreferencesGetModernMediaControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetModernMediaControlsEnabled called");
    return NULL;
}

void* WKPreferencesGetNeedsSiteSpecificQuirks(void) {
    if (verbose) puts("STUB: WKPreferencesGetNeedsSiteSpecificQuirks called");
    return NULL;
}

void* WKPreferencesGetNeedsStorageAccessFromFileURLsQuirk(void) {
    if (verbose) puts("STUB: WKPreferencesGetNeedsStorageAccessFromFileURLsQuirk called");
    return NULL;
}

void* WKPreferencesGetNewBlockInsideInlineModelEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetNewBlockInsideInlineModelEnabled called");
    return NULL;
}

void* WKPreferencesGetNotificationsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetNotificationsEnabled called");
    return NULL;
}

void* WKPreferencesGetOfflineWebApplicationCacheEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetOfflineWebApplicationCacheEnabled called");
    return NULL;
}

void* WKPreferencesGetPDFPluginEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPDFPluginEnabled called");
    return NULL;
}

void* WKPreferencesGetPageCacheEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPageCacheEnabled called");
    return NULL;
}

void* WKPreferencesGetPageCacheSupportsPlugins(void) {
    if (verbose) puts("STUB: WKPreferencesGetPageCacheSupportsPlugins called");
    return NULL;
}

void* WKPreferencesGetPageVisibilityBasedProcessSuppressionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPageVisibilityBasedProcessSuppressionEnabled called");
    return NULL;
}

void* WKPreferencesGetPaginateDuringLayoutEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPaginateDuringLayoutEnabled called");
    return NULL;
}

void* WKPreferencesGetPeerConnectionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPeerConnectionEnabled called");
    return NULL;
}

void* WKPreferencesGetPlugInSnapshottingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPlugInSnapshottingEnabled called");
    return NULL;
}

void* WKPreferencesGetPluginsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPluginsEnabled called");
    return NULL;
}

void* WKPreferencesGetPrimaryPlugInSnapshotDetectionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPrimaryPlugInSnapshotDetectionEnabled called");
    return NULL;
}

void* WKPreferencesGetPrivateBrowsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetPrivateBrowsingEnabled called");
    return NULL;
}

void* WKPreferencesGetQTKitEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetQTKitEnabled called");
    return NULL;
}

void* WKPreferencesGetRequestAnimationFrameEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetRequestAnimationFrameEnabled called");
    return NULL;
}

void* WKPreferencesGetResourceTimingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetResourceTimingEnabled called");
    return NULL;
}

void* WKPreferencesGetResourceUsageOverlayVisible(void) {
    if (verbose) puts("STUB: WKPreferencesGetResourceUsageOverlayVisible called");
    return NULL;
}

void* WKPreferencesGetScrollingPerformanceLoggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetScrollingPerformanceLoggingEnabled called");
    return NULL;
}

void* WKPreferencesGetSelectTrailingWhitespaceEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSelectTrailingWhitespaceEnabled called");
    return NULL;
}

void* WKPreferencesGetSelectionPaintingWithoutSelectionGapsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSelectionPaintingWithoutSelectionGapsEnabled called");
    return NULL;
}

void* WKPreferencesGetServiceControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetServiceControlsEnabled called");
    return NULL;
}

void* WKPreferencesGetShouldAllowUserInstalledFonts(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldAllowUserInstalledFonts called");
    return NULL;
}

void* WKPreferencesGetShouldConvertPositionStyleOnCopy(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldConvertPositionStyleOnCopy called");
    return NULL;
}

void* WKPreferencesGetShouldDisplayCaptions(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldDisplayCaptions called");
    return NULL;
}

void* WKPreferencesGetShouldDisplaySubtitles(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldDisplaySubtitles called");
    return NULL;
}

void* WKPreferencesGetShouldDisplayTextDescriptions(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldDisplayTextDescriptions called");
    return NULL;
}

void* WKPreferencesGetShouldPrintBackgrounds(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldPrintBackgrounds called");
    return NULL;
}

void* WKPreferencesGetShouldRespectImageOrientation(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldRespectImageOrientation called");
    return NULL;
}

void* WKPreferencesGetShouldSuppressKeyboardInputDuringProvisionalNavigation(void) {
    if (verbose) puts("STUB: WKPreferencesGetShouldSuppressKeyboardInputDuringProvisionalNavigation called");
    return NULL;
}

void* WKPreferencesGetShowsToolTipOverTruncatedText(void) {
    if (verbose) puts("STUB: WKPreferencesGetShowsToolTipOverTruncatedText called");
    return NULL;
}

void* WKPreferencesGetShowsURLsInToolTipsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetShowsURLsInToolTipsEnabled called");
    return NULL;
}

void* WKPreferencesGetSimpleLineLayoutDebugBordersEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSimpleLineLayoutDebugBordersEnabled called");
    return NULL;
}

void* WKPreferencesGetSimpleLineLayoutEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSimpleLineLayoutEnabled called");
    return NULL;
}

void* WKPreferencesGetSmartInsertDeleteEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSmartInsertDeleteEnabled called");
    return NULL;
}

void* WKPreferencesGetSnapshotAllPlugIns(void) {
    if (verbose) puts("STUB: WKPreferencesGetSnapshotAllPlugIns called");
    return NULL;
}

void* WKPreferencesGetSpatialNavigationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSpatialNavigationEnabled called");
    return NULL;
}

void* WKPreferencesGetStorageAccessAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetStorageAccessAPIEnabled called");
    return NULL;
}

void* WKPreferencesGetStorageBlockingPolicy(void) {
    if (verbose) puts("STUB: WKPreferencesGetStorageBlockingPolicy called");
    return NULL;
}

void* WKPreferencesGetSubpixelAntialiasedLayerTextEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSubpixelAntialiasedLayerTextEnabled called");
    return NULL;
}

void* WKPreferencesGetSubpixelCSSOMElementMetricsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetSubpixelCSSOMElementMetricsEnabled called");
    return NULL;
}

void* WKPreferencesGetSuppressesIncrementalRendering(void) {
    if (verbose) puts("STUB: WKPreferencesGetSuppressesIncrementalRendering called");
    return NULL;
}

void* WKPreferencesGetTabToLinksEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetTabToLinksEnabled called");
    return NULL;
}

void* WKPreferencesGetTabsToLinks(void) {
    if (verbose) puts("STUB: WKPreferencesGetTabsToLinks called");
    return NULL;
}

void* WKPreferencesGetTelephoneNumberParsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetTelephoneNumberParsingEnabled called");
    return NULL;
}

void* WKPreferencesGetTextAreasAreResizable(void) {
    if (verbose) puts("STUB: WKPreferencesGetTextAreasAreResizable called");
    return NULL;
}

void* WKPreferencesGetTextAutosizingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetTextAutosizingEnabled called");
    return NULL;
}

void* WKPreferencesGetThreadedScrollingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetThreadedScrollingEnabled called");
    return NULL;
}

void* WKPreferencesGetTiledScrollingIndicatorVisible(void) {
    if (verbose) puts("STUB: WKPreferencesGetTiledScrollingIndicatorVisible called");
    return NULL;
}

void* WKPreferencesGetTypeID(void) {
    if (verbose) puts("STUB: WKPreferencesGetTypeID called");
    return NULL;
}

void* WKPreferencesGetUniversalAccessFromFileURLsAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesGetUniversalAccessFromFileURLsAllowed called");
    return NULL;
}

void* WKPreferencesGetUseGiantTiles(void) {
    if (verbose) puts("STUB: WKPreferencesGetUseGiantTiles called");
    return NULL;
}

void* WKPreferencesGetUserInterfaceDirectionPolicy(void) {
    if (verbose) puts("STUB: WKPreferencesGetUserInterfaceDirectionPolicy called");
    return NULL;
}

void* WKPreferencesGetUserTimingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetUserTimingEnabled called");
    return NULL;
}

void* WKPreferencesGetVideoPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesGetVideoPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesGetViewGestureDebuggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetViewGestureDebuggingEnabled called");
    return NULL;
}

void* WKPreferencesGetVisibleDebugOverlayRegions(void) {
    if (verbose) puts("STUB: WKPreferencesGetVisibleDebugOverlayRegions called");
    return NULL;
}

void* WKPreferencesGetWebAnimationsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebAnimationsEnabled called");
    return NULL;
}

void* WKPreferencesGetWebArchiveDebugModeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebArchiveDebugModeEnabled called");
    return NULL;
}

void* WKPreferencesGetWebAudioEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebAudioEnabled called");
    return NULL;
}

void* WKPreferencesGetWebAuthenticationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebAuthenticationEnabled called");
    return NULL;
}

void* WKPreferencesGetWebGLEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebGLEnabled called");
    return NULL;
}

void* WKPreferencesGetWebRTCLegacyAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebRTCLegacyAPIEnabled called");
    return NULL;
}

void* WKPreferencesGetWebSecurityEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetWebSecurityEnabled called");
    return NULL;
}

void* WKPreferencesGetXSSAuditorEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesGetXSSAuditorEnabled called");
    return NULL;
}

void* WKPreferencesResetTestRunnerOverrides(void) {
    if (verbose) puts("STUB: WKPreferencesResetTestRunnerOverrides called");
    return NULL;
}

void* WKPreferencesSetAVFoundationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAVFoundationEnabled called");
    return NULL;
}

void* WKPreferencesSetAVFoundationNSURLSessionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAVFoundationNSURLSessionEnabled called");
    return NULL;
}

void* WKPreferencesSetAccelerated2DCanvasEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAccelerated2DCanvasEnabled called");
    return NULL;
}

void* WKPreferencesSetAcceleratedCompositingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAcceleratedCompositingEnabled called");
    return NULL;
}

void* WKPreferencesSetAcceleratedCompositingForOverflowScrollEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAcceleratedCompositingForOverflowScrollEnabled called");
    return NULL;
}

void* WKPreferencesSetAcceleratedDrawingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAcceleratedDrawingEnabled called");
    return NULL;
}

void* WKPreferencesSetAccessibilityObjectModelEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAccessibilityObjectModelEnabled called");
    return NULL;
}

void* WKPreferencesSetAggressiveTileRetentionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAggressiveTileRetentionEnabled called");
    return NULL;
}

void* WKPreferencesSetAllowMediaContentTypesRequiringHardwareSupportAsFallback(void) {
    if (verbose) puts("STUB: WKPreferencesSetAllowMediaContentTypesRequiringHardwareSupportAsFallback called");
    return NULL;
}

void* WKPreferencesSetAllowsAirPlayForMediaPlayback(void) {
    if (verbose) puts("STUB: WKPreferencesSetAllowsAirPlayForMediaPlayback called");
    return NULL;
}

void* WKPreferencesSetAllowsPictureInPictureMediaPlayback(void) {
    if (verbose) puts("STUB: WKPreferencesSetAllowsPictureInPictureMediaPlayback called");
    return NULL;
}

void* WKPreferencesSetAnimatedImageAsyncDecodingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAnimatedImageAsyncDecodingEnabled called");
    return NULL;
}

void* WKPreferencesSetAntialiasedFontDilationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAntialiasedFontDilationEnabled called");
    return NULL;
}

void* WKPreferencesSetApplePayCapabilityDisclosureAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesSetApplePayCapabilityDisclosureAllowed called");
    return NULL;
}

void* WKPreferencesSetApplePayEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetApplePayEnabled called");
    return NULL;
}

void* WKPreferencesSetApplicationChromeModeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetApplicationChromeModeEnabled called");
    return NULL;
}

void* WKPreferencesSetArtificialPluginInitializationDelayEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetArtificialPluginInitializationDelayEnabled called");
    return NULL;
}

void* WKPreferencesSetAsynchronousPluginInitializationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAsynchronousPluginInitializationEnabled called");
    return NULL;
}

void* WKPreferencesSetAsynchronousPluginInitializationEnabledForAllPlugins(void) {
    if (verbose) puts("STUB: WKPreferencesSetAsynchronousPluginInitializationEnabledForAllPlugins called");
    return NULL;
}

void* WKPreferencesSetAsynchronousSpellCheckingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAsynchronousSpellCheckingEnabled called");
    return NULL;
}

void* WKPreferencesSetAttachmentElementEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAttachmentElementEnabled called");
    return NULL;
}

void* WKPreferencesSetAudioPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesSetAudioPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesSetAuthorAndUserStylesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAuthorAndUserStylesEnabled called");
    return NULL;
}

void* WKPreferencesSetAutostartOriginPlugInSnapshottingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetAutostartOriginPlugInSnapshottingEnabled called");
    return NULL;
}

void* WKPreferencesSetBackspaceKeyNavigationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetBackspaceKeyNavigationEnabled called");
    return NULL;
}

void* WKPreferencesSetBeaconAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetBeaconAPIEnabled called");
    return NULL;
}

void* WKPreferencesSetCSSAnimationTriggersEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetCSSAnimationTriggersEnabled called");
    return NULL;
}

void* WKPreferencesSetCanvasUsesAcceleratedDrawing(void) {
    if (verbose) puts("STUB: WKPreferencesSetCanvasUsesAcceleratedDrawing called");
    return NULL;
}

void* WKPreferencesSetCaretBrowsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetCaretBrowsingEnabled called");
    return NULL;
}

void* WKPreferencesSetCompositingBordersVisible(void) {
    if (verbose) puts("STUB: WKPreferencesSetCompositingBordersVisible called");
    return NULL;
}

void* WKPreferencesSetCompositingRepaintCountersVisible(void) {
    if (verbose) puts("STUB: WKPreferencesSetCompositingRepaintCountersVisible called");
    return NULL;
}

void* WKPreferencesSetCookieEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetCookieEnabled called");
    return NULL;
}

void* WKPreferencesSetCursiveFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetCursiveFontFamily called");
    return NULL;
}

void* WKPreferencesSetCustomPasteboardDataEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetCustomPasteboardDataEnabled called");
    return NULL;
}

void* WKPreferencesSetDNSPrefetchingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDNSPrefetchingEnabled called");
    return NULL;
}

void* WKPreferencesSetDOMPasteAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesSetDOMPasteAllowed called");
    return NULL;
}

void* WKPreferencesSetDOMTimersThrottlingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDOMTimersThrottlingEnabled called");
    return NULL;
}

void* WKPreferencesSetDataTransferItemsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDataTransferItemsEnabled called");
    return NULL;
}

void* WKPreferencesSetDatabasesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDatabasesEnabled called");
    return NULL;
}

void* WKPreferencesSetDefaultFixedFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesSetDefaultFixedFontSize called");
    return NULL;
}

void* WKPreferencesSetDefaultFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesSetDefaultFontSize called");
    return NULL;
}

void* WKPreferencesSetDefaultTextEncodingName(void) {
    if (verbose) puts("STUB: WKPreferencesSetDefaultTextEncodingName called");
    return NULL;
}

void* WKPreferencesSetDeferredCSSParserEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDeferredCSSParserEnabled called");
    return NULL;
}

void* WKPreferencesSetDeveloperExtrasEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDeveloperExtrasEnabled called");
    return NULL;
}

void* WKPreferencesSetDiagnosticLoggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDiagnosticLoggingEnabled called");
    return NULL;
}

void* WKPreferencesSetDirectoryUploadEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDirectoryUploadEnabled called");
    return NULL;
}

void* WKPreferencesSetDisplayContentsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDisplayContentsEnabled called");
    return NULL;
}

void* WKPreferencesSetDownloadAttributeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetDownloadAttributeEnabled called");
    return NULL;
}

void* WKPreferencesSetEditableLinkBehavior(void) {
    if (verbose) puts("STUB: WKPreferencesSetEditableLinkBehavior called");
    return NULL;
}

void* WKPreferencesSetEnableInheritURIQueryComponent(void) {
    if (verbose) puts("STUB: WKPreferencesSetEnableInheritURIQueryComponent called");
    return NULL;
}

void* WKPreferencesSetEncodingDetectorEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetEncodingDetectorEnabled called");
    return NULL;
}

void* WKPreferencesSetEnumeratingAllNetworkInterfacesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetEnumeratingAllNetworkInterfacesEnabled called");
    return NULL;
}

void* WKPreferencesSetFTPDirectoryTemplatePath(void) {
    if (verbose) puts("STUB: WKPreferencesSetFTPDirectoryTemplatePath called");
    return NULL;
}

void* WKPreferencesSetFantasyFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetFantasyFontFamily called");
    return NULL;
}

void* WKPreferencesSetFetchAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetFetchAPIEnabled called");
    return NULL;
}

void* WKPreferencesSetFetchAPIKeepAliveEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetFetchAPIKeepAliveEnabled called");
    return NULL;
}

void* WKPreferencesSetFileAccessFromFileURLsAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesSetFileAccessFromFileURLsAllowed called");
    return NULL;
}

void* WKPreferencesSetFixedFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetFixedFontFamily called");
    return NULL;
}

void* WKPreferencesSetFontSmoothingLevel(void) {
    if (verbose) puts("STUB: WKPreferencesSetFontSmoothingLevel called");
    return NULL;
}

void* WKPreferencesSetForceFTPDirectoryListings(void) {
    if (verbose) puts("STUB: WKPreferencesSetForceFTPDirectoryListings called");
    return NULL;
}

void* WKPreferencesSetForceSoftwareWebGLRendering(void) {
    if (verbose) puts("STUB: WKPreferencesSetForceSoftwareWebGLRendering called");
    return NULL;
}

void* WKPreferencesSetFrameFlatteningEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetFrameFlatteningEnabled called");
    return NULL;
}

void* WKPreferencesSetFullScreenEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetFullScreenEnabled called");
    return NULL;
}

void* WKPreferencesSetGamepadsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetGamepadsEnabled called");
    return NULL;
}

void* WKPreferencesSetHTTPEquivEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetHTTPEquivEnabled called");
    return NULL;
}

void* WKPreferencesSetHiddenPageCSSAnimationSuspensionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetHiddenPageCSSAnimationSuspensionEnabled called");
    return NULL;
}

void* WKPreferencesSetHiddenPageDOMTimerThrottlingAutoIncreases(void) {
    if (verbose) puts("STUB: WKPreferencesSetHiddenPageDOMTimerThrottlingAutoIncreases called");
    return NULL;
}

void* WKPreferencesSetHiddenPageDOMTimerThrottlingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetHiddenPageDOMTimerThrottlingEnabled called");
    return NULL;
}

void* WKPreferencesSetHixie76WebSocketProtocolEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetHixie76WebSocketProtocolEnabled called");
    return NULL;
}

void* WKPreferencesSetHyperlinkAuditingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetHyperlinkAuditingEnabled called");
    return NULL;
}

void* WKPreferencesSetICECandidateFilteringEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetICECandidateFilteringEnabled called");
    return NULL;
}

void* WKPreferencesSetIgnoreViewportScalingConstraints(void) {
    if (verbose) puts("STUB: WKPreferencesSetIgnoreViewportScalingConstraints called");
    return NULL;
}

void* WKPreferencesSetImageControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetImageControlsEnabled called");
    return NULL;
}

void* WKPreferencesSetInactiveMediaCaptureSteamRepromptIntervalInMinutes(void) {
    if (verbose) puts("STUB: WKPreferencesSetInactiveMediaCaptureSteamRepromptIntervalInMinutes called");
    return NULL;
}

void* WKPreferencesSetIncrementalRenderingSuppressionTimeout(void) {
    if (verbose) puts("STUB: WKPreferencesSetIncrementalRenderingSuppressionTimeout called");
    return NULL;
}

void* WKPreferencesSetInlineMediaPlaybackRequiresPlaysInlineAttribute(void) {
    if (verbose) puts("STUB: WKPreferencesSetInlineMediaPlaybackRequiresPlaysInlineAttribute called");
    return NULL;
}

void* WKPreferencesSetInspectorAdditionsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetInspectorAdditionsEnabled called");
    return NULL;
}

void* WKPreferencesSetInspectorUsesWebKitUserInterface(void) {
    if (verbose) puts("STUB: WKPreferencesSetInspectorUsesWebKitUserInterface called");
    return NULL;
}

void* WKPreferencesSetInteractiveFormValidationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetInteractiveFormValidationEnabled called");
    return NULL;
}

void* WKPreferencesSetIntersectionObserverEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetIntersectionObserverEnabled called");
    return NULL;
}

void* WKPreferencesSetInvisibleMediaAutoplayPermitted(void) {
    if (verbose) puts("STUB: WKPreferencesSetInvisibleMediaAutoplayPermitted called");
    return NULL;
}

void* WKPreferencesSetIsSecureContextAttributeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetIsSecureContextAttributeEnabled called");
    return NULL;
}

void* WKPreferencesSetJavaEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaEnabled called");
    return NULL;
}

void* WKPreferencesSetJavaEnabledForLocalFiles(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaEnabledForLocalFiles called");
    return NULL;
}

void* WKPreferencesSetJavaScriptCanAccessClipboard(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaScriptCanAccessClipboard called");
    return NULL;
}

void* WKPreferencesSetJavaScriptCanOpenWindowsAutomatically(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaScriptCanOpenWindowsAutomatically called");
    return NULL;
}

void* WKPreferencesSetJavaScriptEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaScriptEnabled called");
    return NULL;
}

void* WKPreferencesSetJavaScriptMarkupEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaScriptMarkupEnabled called");
    return NULL;
}

void* WKPreferencesSetJavaScriptRuntimeFlags(void) {
    if (verbose) puts("STUB: WKPreferencesSetJavaScriptRuntimeFlags called");
    return NULL;
}

void* WKPreferencesSetLargeImageAsyncDecodingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLargeImageAsyncDecodingEnabled called");
    return NULL;
}

void* WKPreferencesSetLegacyEncryptedMediaAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLegacyEncryptedMediaAPIEnabled called");
    return NULL;
}

void* WKPreferencesSetLinkPreloadEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLinkPreloadEnabled called");
    return NULL;
}

void* WKPreferencesSetLoadsImagesAutomatically(void) {
    if (verbose) puts("STUB: WKPreferencesSetLoadsImagesAutomatically called");
    return NULL;
}

void* WKPreferencesSetLoadsSiteIconsIgnoringImageLoadingPreference(void) {
    if (verbose) puts("STUB: WKPreferencesSetLoadsSiteIconsIgnoringImageLoadingPreference called");
    return NULL;
}

void* WKPreferencesSetLocalFileContentSniffingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLocalFileContentSniffingEnabled called");
    return NULL;
}

void* WKPreferencesSetLocalStorageEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLocalStorageEnabled called");
    return NULL;
}

void* WKPreferencesSetLogsPageMessagesToSystemConsoleEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLogsPageMessagesToSystemConsoleEnabled called");
    return NULL;
}

void* WKPreferencesSetLongMousePressEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetLongMousePressEnabled called");
    return NULL;
}

void* WKPreferencesSetMainContentUserGestureOverrideEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMainContentUserGestureOverrideEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaCapabilitiesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaCapabilitiesEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaCaptureRequiresSecureConnection(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaCaptureRequiresSecureConnection called");
    return NULL;
}

void* WKPreferencesSetMediaContentTypesRequiringHardwareSupport(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaContentTypesRequiringHardwareSupport called");
    return NULL;
}

void* WKPreferencesSetMediaControlsScaleWithPageZoom(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaControlsScaleWithPageZoom called");
    return NULL;
}

void* WKPreferencesSetMediaDevicesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaDevicesEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaPlaybackAllowsInline(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaPlaybackAllowsInline called");
    return NULL;
}

void* WKPreferencesSetMediaPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesSetMediaPreloadingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaPreloadingEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaSourceEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaSourceEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaStreamEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaStreamEnabled called");
    return NULL;
}

void* WKPreferencesSetMediaUserGestureInheritsFromDocument(void) {
    if (verbose) puts("STUB: WKPreferencesSetMediaUserGestureInheritsFromDocument called");
    return NULL;
}

void* WKPreferencesSetMenuItemElementEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMenuItemElementEnabled called");
    return NULL;
}

void* WKPreferencesSetMetaRefreshEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMetaRefreshEnabled called");
    return NULL;
}

void* WKPreferencesSetMinimumFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesSetMinimumFontSize called");
    return NULL;
}

void* WKPreferencesSetMinimumZoomFontSize(void) {
    if (verbose) puts("STUB: WKPreferencesSetMinimumZoomFontSize called");
    return NULL;
}

void* WKPreferencesSetMockCaptureDevicesEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMockCaptureDevicesEnabled called");
    return NULL;
}

void* WKPreferencesSetMockScrollbarsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetMockScrollbarsEnabled called");
    return NULL;
}

void* WKPreferencesSetModernMediaControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetModernMediaControlsEnabled called");
    return NULL;
}

void* WKPreferencesSetNeedsSiteSpecificQuirks(void) {
    if (verbose) puts("STUB: WKPreferencesSetNeedsSiteSpecificQuirks called");
    return NULL;
}

void* WKPreferencesSetNeedsStorageAccessFromFileURLsQuirk(void) {
    if (verbose) puts("STUB: WKPreferencesSetNeedsStorageAccessFromFileURLsQuirk called");
    return NULL;
}

void* WKPreferencesSetNewBlockInsideInlineModelEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetNewBlockInsideInlineModelEnabled called");
    return NULL;
}

void* WKPreferencesSetNotificationsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetNotificationsEnabled called");
    return NULL;
}

void* WKPreferencesSetOfflineWebApplicationCacheEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetOfflineWebApplicationCacheEnabled called");
    return NULL;
}

void* WKPreferencesSetPDFPluginEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPDFPluginEnabled called");
    return NULL;
}

void* WKPreferencesSetPageCacheEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPageCacheEnabled called");
    return NULL;
}

void* WKPreferencesSetPageCacheSupportsPlugins(void) {
    if (verbose) puts("STUB: WKPreferencesSetPageCacheSupportsPlugins called");
    return NULL;
}

void* WKPreferencesSetPageVisibilityBasedProcessSuppressionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPageVisibilityBasedProcessSuppressionEnabled called");
    return NULL;
}

void* WKPreferencesSetPaginateDuringLayoutEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPaginateDuringLayoutEnabled called");
    return NULL;
}

void* WKPreferencesSetPeerConnectionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPeerConnectionEnabled called");
    return NULL;
}

void* WKPreferencesSetPictographFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetPictographFontFamily called");
    return NULL;
}

void* WKPreferencesSetPlugInSnapshottingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPlugInSnapshottingEnabled called");
    return NULL;
}

void* WKPreferencesSetPluginsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPluginsEnabled called");
    return NULL;
}

void* WKPreferencesSetPrimaryPlugInSnapshotDetectionEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPrimaryPlugInSnapshotDetectionEnabled called");
    return NULL;
}

void* WKPreferencesSetPrivateBrowsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetPrivateBrowsingEnabled called");
    return NULL;
}

void* WKPreferencesSetQTKitEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetQTKitEnabled called");
    return NULL;
}

void* WKPreferencesSetRequestAnimationFrameEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetRequestAnimationFrameEnabled called");
    return NULL;
}

void* WKPreferencesSetResourceTimingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetResourceTimingEnabled called");
    return NULL;
}

void* WKPreferencesSetResourceUsageOverlayVisible(void) {
    if (verbose) puts("STUB: WKPreferencesSetResourceUsageOverlayVisible called");
    return NULL;
}

void* WKPreferencesSetSansSerifFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetSansSerifFontFamily called");
    return NULL;
}

void* WKPreferencesSetScrollingPerformanceLoggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetScrollingPerformanceLoggingEnabled called");
    return NULL;
}

void* WKPreferencesSetSelectTrailingWhitespaceEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSelectTrailingWhitespaceEnabled called");
    return NULL;
}

void* WKPreferencesSetSelectionPaintingWithoutSelectionGapsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSelectionPaintingWithoutSelectionGapsEnabled called");
    return NULL;
}

void* WKPreferencesSetSerifFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetSerifFontFamily called");
    return NULL;
}

void* WKPreferencesSetServiceControlsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetServiceControlsEnabled called");
    return NULL;
}

void* WKPreferencesSetShouldAllowUserInstalledFonts(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldAllowUserInstalledFonts called");
    return NULL;
}

void* WKPreferencesSetShouldConvertPositionStyleOnCopy(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldConvertPositionStyleOnCopy called");
    return NULL;
}

void* WKPreferencesSetShouldDisplayCaptions(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldDisplayCaptions called");
    return NULL;
}

void* WKPreferencesSetShouldDisplaySubtitles(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldDisplaySubtitles called");
    return NULL;
}

void* WKPreferencesSetShouldDisplayTextDescriptions(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldDisplayTextDescriptions called");
    return NULL;
}

void* WKPreferencesSetShouldPrintBackgrounds(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldPrintBackgrounds called");
    return NULL;
}

void* WKPreferencesSetShouldRespectImageOrientation(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldRespectImageOrientation called");
    return NULL;
}

void* WKPreferencesSetShouldSuppressKeyboardInputDuringProvisionalNavigation(void) {
    if (verbose) puts("STUB: WKPreferencesSetShouldSuppressKeyboardInputDuringProvisionalNavigation called");
    return NULL;
}

void* WKPreferencesSetShowsToolTipOverTruncatedText(void) {
    if (verbose) puts("STUB: WKPreferencesSetShowsToolTipOverTruncatedText called");
    return NULL;
}

void* WKPreferencesSetShowsURLsInToolTipsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetShowsURLsInToolTipsEnabled called");
    return NULL;
}

void* WKPreferencesSetSimpleLineLayoutDebugBordersEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSimpleLineLayoutDebugBordersEnabled called");
    return NULL;
}

void* WKPreferencesSetSimpleLineLayoutEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSimpleLineLayoutEnabled called");
    return NULL;
}

void* WKPreferencesSetSmartInsertDeleteEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSmartInsertDeleteEnabled called");
    return NULL;
}

void* WKPreferencesSetSnapshotAllPlugIns(void) {
    if (verbose) puts("STUB: WKPreferencesSetSnapshotAllPlugIns called");
    return NULL;
}

void* WKPreferencesSetSpatialNavigationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSpatialNavigationEnabled called");
    return NULL;
}

void* WKPreferencesSetStandardFontFamily(void) {
    if (verbose) puts("STUB: WKPreferencesSetStandardFontFamily called");
    return NULL;
}

void* WKPreferencesSetStorageAccessAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetStorageAccessAPIEnabled called");
    return NULL;
}

void* WKPreferencesSetStorageBlockingPolicy(void) {
    if (verbose) puts("STUB: WKPreferencesSetStorageBlockingPolicy called");
    return NULL;
}

void* WKPreferencesSetSubpixelAntialiasedLayerTextEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSubpixelAntialiasedLayerTextEnabled called");
    return NULL;
}

void* WKPreferencesSetSubpixelCSSOMElementMetricsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetSubpixelCSSOMElementMetricsEnabled called");
    return NULL;
}

void* WKPreferencesSetSuppressesIncrementalRendering(void) {
    if (verbose) puts("STUB: WKPreferencesSetSuppressesIncrementalRendering called");
    return NULL;
}

void* WKPreferencesSetTabToLinksEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetTabToLinksEnabled called");
    return NULL;
}

void* WKPreferencesSetTabsToLinks(void) {
    if (verbose) puts("STUB: WKPreferencesSetTabsToLinks called");
    return NULL;
}

void* WKPreferencesSetTelephoneNumberParsingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetTelephoneNumberParsingEnabled called");
    return NULL;
}

void* WKPreferencesSetTextAreasAreResizable(void) {
    if (verbose) puts("STUB: WKPreferencesSetTextAreasAreResizable called");
    return NULL;
}

void* WKPreferencesSetTextAutosizingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetTextAutosizingEnabled called");
    return NULL;
}

void* WKPreferencesSetThreadedScrollingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetThreadedScrollingEnabled called");
    return NULL;
}

void* WKPreferencesSetTiledScrollingIndicatorVisible(void) {
    if (verbose) puts("STUB: WKPreferencesSetTiledScrollingIndicatorVisible called");
    return NULL;
}

void* WKPreferencesSetUniversalAccessFromFileURLsAllowed(void) {
    if (verbose) puts("STUB: WKPreferencesSetUniversalAccessFromFileURLsAllowed called");
    return NULL;
}

void* WKPreferencesSetUseGiantTiles(void) {
    if (verbose) puts("STUB: WKPreferencesSetUseGiantTiles called");
    return NULL;
}

void* WKPreferencesSetUseLegacyTextAlignPositionedElementBehavior(void) {
    if (verbose) puts("STUB: WKPreferencesSetUseLegacyTextAlignPositionedElementBehavior called");
    return NULL;
}

void* WKPreferencesSetUserInterfaceDirectionPolicy(void) {
    if (verbose) puts("STUB: WKPreferencesSetUserInterfaceDirectionPolicy called");
    return NULL;
}

void* WKPreferencesSetUserTimingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetUserTimingEnabled called");
    return NULL;
}

void* WKPreferencesSetVideoPlaybackRequiresUserGesture(void) {
    if (verbose) puts("STUB: WKPreferencesSetVideoPlaybackRequiresUserGesture called");
    return NULL;
}

void* WKPreferencesSetViewGestureDebuggingEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetViewGestureDebuggingEnabled called");
    return NULL;
}

void* WKPreferencesSetVisibleDebugOverlayRegions(void) {
    if (verbose) puts("STUB: WKPreferencesSetVisibleDebugOverlayRegions called");
    return NULL;
}

void* WKPreferencesSetWebAnimationsEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebAnimationsEnabled called");
    return NULL;
}

void* WKPreferencesSetWebArchiveDebugModeEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebArchiveDebugModeEnabled called");
    return NULL;
}

void* WKPreferencesSetWebAudioEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebAudioEnabled called");
    return NULL;
}

void* WKPreferencesSetWebAuthenticationEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebAuthenticationEnabled called");
    return NULL;
}

void* WKPreferencesSetWebGLEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebGLEnabled called");
    return NULL;
}

void* WKPreferencesSetWebRTCLegacyAPIEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebRTCLegacyAPIEnabled called");
    return NULL;
}

void* WKPreferencesSetWebSecurityEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetWebSecurityEnabled called");
    return NULL;
}

void* WKPreferencesSetXSSAuditorEnabled(void) {
    if (verbose) puts("STUB: WKPreferencesSetXSSAuditorEnabled called");
    return NULL;
}

void* WKPreferencesUseLegacyTextAlignPositionedElementBehavior(void) {
    if (verbose) puts("STUB: WKPreferencesUseLegacyTextAlignPositionedElementBehavior called");
    return NULL;
}

void* WKProtectionSpaceCopyHost(void) {
    if (verbose) puts("STUB: WKProtectionSpaceCopyHost called");
    return NULL;
}

void* WKProtectionSpaceCopyNSURLProtectionSpace(void) {
    if (verbose) puts("STUB: WKProtectionSpaceCopyNSURLProtectionSpace called");
    return NULL;
}

void* WKProtectionSpaceCopyRealm(void) {
    if (verbose) puts("STUB: WKProtectionSpaceCopyRealm called");
    return NULL;
}

void* WKProtectionSpaceGetAuthenticationScheme(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetAuthenticationScheme called");
    return NULL;
}

void* WKProtectionSpaceGetIsProxy(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetIsProxy called");
    return NULL;
}

void* WKProtectionSpaceGetPort(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetPort called");
    return NULL;
}

void* WKProtectionSpaceGetReceivesCredentialSecurely(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetReceivesCredentialSecurely called");
    return NULL;
}

void* WKProtectionSpaceGetServerType(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetServerType called");
    return NULL;
}

void* WKProtectionSpaceGetTypeID(void) {
    if (verbose) puts("STUB: WKProtectionSpaceGetTypeID called");
    return NULL;
}

void* WKRectCreate(void) {
    if (verbose) puts("STUB: WKRectCreate called");
    return NULL;
}

void* WKRectGetTypeID(void) {
    if (verbose) puts("STUB: WKRectGetTypeID called");
    return NULL;
}

void* WKRectGetValue(void) {
    if (verbose) puts("STUB: WKRectGetValue called");
    return NULL;
}

void* WKRelease(void) {
    if (verbose) puts("STUB: WKRelease called");
    return NULL;
}

void* WKRenderLayerCopyElementID(void) {
    if (verbose) puts("STUB: WKRenderLayerCopyElementID called");
    return NULL;
}

void* WKRenderLayerCopyElementTagName(void) {
    if (verbose) puts("STUB: WKRenderLayerCopyElementTagName called");
    return NULL;
}

void* WKRenderLayerCopyRendererName(void) {
    if (verbose) puts("STUB: WKRenderLayerCopyRendererName called");
    return NULL;
}

void* WKRenderLayerGetAbsoluteBounds(void) {
    if (verbose) puts("STUB: WKRenderLayerGetAbsoluteBounds called");
    return NULL;
}

void* WKRenderLayerGetBackingStoreMemoryEstimate(void) {
    if (verbose) puts("STUB: WKRenderLayerGetBackingStoreMemoryEstimate called");
    return NULL;
}

void* WKRenderLayerGetCompositingLayerType(void) {
    if (verbose) puts("STUB: WKRenderLayerGetCompositingLayerType called");
    return NULL;
}

void* WKRenderLayerGetElementClassNames(void) {
    if (verbose) puts("STUB: WKRenderLayerGetElementClassNames called");
    return NULL;
}

void* WKRenderLayerGetFrameContentsLayer(void) {
    if (verbose) puts("STUB: WKRenderLayerGetFrameContentsLayer called");
    return NULL;
}

void* WKRenderLayerGetNegativeZOrderList(void) {
    if (verbose) puts("STUB: WKRenderLayerGetNegativeZOrderList called");
    return NULL;
}

void* WKRenderLayerGetNormalFlowList(void) {
    if (verbose) puts("STUB: WKRenderLayerGetNormalFlowList called");
    return NULL;
}

void* WKRenderLayerGetPositiveZOrderList(void) {
    if (verbose) puts("STUB: WKRenderLayerGetPositiveZOrderList called");
    return NULL;
}

void* WKRenderLayerGetRenderer(void) {
    if (verbose) puts("STUB: WKRenderLayerGetRenderer called");
    return NULL;
}

void* WKRenderLayerGetTypeID(void) {
    if (verbose) puts("STUB: WKRenderLayerGetTypeID called");
    return NULL;
}

void* WKRenderLayerIsClipped(void) {
    if (verbose) puts("STUB: WKRenderLayerIsClipped called");
    return NULL;
}

void* WKRenderLayerIsClipping(void) {
    if (verbose) puts("STUB: WKRenderLayerIsClipping called");
    return NULL;
}

void* WKRenderLayerIsReflection(void) {
    if (verbose) puts("STUB: WKRenderLayerIsReflection called");
    return NULL;
}

void* WKRenderObjectCopyElementID(void) {
    if (verbose) puts("STUB: WKRenderObjectCopyElementID called");
    return NULL;
}

void* WKRenderObjectCopyElementTagName(void) {
    if (verbose) puts("STUB: WKRenderObjectCopyElementTagName called");
    return NULL;
}

void* WKRenderObjectCopyName(void) {
    if (verbose) puts("STUB: WKRenderObjectCopyName called");
    return NULL;
}

void* WKRenderObjectCopyTextSnippet(void) {
    if (verbose) puts("STUB: WKRenderObjectCopyTextSnippet called");
    return NULL;
}

void* WKRenderObjectGetAbsolutePosition(void) {
    if (verbose) puts("STUB: WKRenderObjectGetAbsolutePosition called");
    return NULL;
}

void* WKRenderObjectGetChildren(void) {
    if (verbose) puts("STUB: WKRenderObjectGetChildren called");
    return NULL;
}

void* WKRenderObjectGetElementClassNames(void) {
    if (verbose) puts("STUB: WKRenderObjectGetElementClassNames called");
    return NULL;
}

void* WKRenderObjectGetFrameRect(void) {
    if (verbose) puts("STUB: WKRenderObjectGetFrameRect called");
    return NULL;
}

void* WKRenderObjectGetTextLength(void) {
    if (verbose) puts("STUB: WKRenderObjectGetTextLength called");
    return NULL;
}

void* WKRenderObjectGetTypeID(void) {
    if (verbose) puts("STUB: WKRenderObjectGetTypeID called");
    return NULL;
}

void* WKResourceCacheManagerClearCacheForAllOrigins(void) {
    if (verbose) puts("STUB: WKResourceCacheManagerClearCacheForAllOrigins called");
    return NULL;
}

void* WKResourceCacheManagerClearCacheForOrigin(void) {
    if (verbose) puts("STUB: WKResourceCacheManagerClearCacheForOrigin called");
    return NULL;
}

void* WKResourceCacheManagerGetCacheOrigins(void) {
    if (verbose) puts("STUB: WKResourceCacheManagerGetCacheOrigins called");
    return NULL;
}

void* WKResourceCacheManagerGetTypeID(void) {
    if (verbose) puts("STUB: WKResourceCacheManagerGetTypeID called");
    return NULL;
}

void* WKRetain(void) {
    if (verbose) puts("STUB: WKRetain called");
    return NULL;
}

void* WKSecurityOriginCopyDatabaseIdentifier(void) {
    if (verbose) puts("STUB: WKSecurityOriginCopyDatabaseIdentifier called");
    return NULL;
}

void* WKSecurityOriginCopyHost(void) {
    if (verbose) puts("STUB: WKSecurityOriginCopyHost called");
    return NULL;
}

void* WKSecurityOriginCopyProtocol(void) {
    if (verbose) puts("STUB: WKSecurityOriginCopyProtocol called");
    return NULL;
}

void* WKSecurityOriginCopyToString(void) {
    if (verbose) puts("STUB: WKSecurityOriginCopyToString called");
    return NULL;
}

void* WKSecurityOriginCreate(void) {
    if (verbose) puts("STUB: WKSecurityOriginCreate called");
    return NULL;
}

void* WKSecurityOriginCreateFromDatabaseIdentifier(void) {
    if (verbose) puts("STUB: WKSecurityOriginCreateFromDatabaseIdentifier called");
    return NULL;
}

void* WKSecurityOriginCreateFromString(void) {
    if (verbose) puts("STUB: WKSecurityOriginCreateFromString called");
    return NULL;
}

void* WKSecurityOriginGetPort(void) {
    if (verbose) puts("STUB: WKSecurityOriginGetPort called");
    return NULL;
}

void* WKSecurityOriginGetTypeID(void) {
    if (verbose) puts("STUB: WKSecurityOriginGetTypeID called");
    return NULL;
}

void* WKSerializedScriptValueCreate(void) {
    if (verbose) puts("STUB: WKSerializedScriptValueCreate called");
    return NULL;
}

void* WKSerializedScriptValueDeserialize(void) {
    if (verbose) puts("STUB: WKSerializedScriptValueDeserialize called");
    return NULL;
}

void* WKSerializedScriptValueGetTypeID(void) {
    if (verbose) puts("STUB: WKSerializedScriptValueGetTypeID called");
    return NULL;
}

void* WKSessionStateCopyData(void) {
    if (verbose) puts("STUB: WKSessionStateCopyData called");
    return NULL;
}

void* WKSessionStateCreateFromData(void) {
    if (verbose) puts("STUB: WKSessionStateCreateFromData called");
    return NULL;
}

void* WKSessionStateGetTypeID(void) {
    if (verbose) puts("STUB: WKSessionStateGetTypeID called");
    return NULL;
}

void* WKSizeCreate(void) {
    if (verbose) puts("STUB: WKSizeCreate called");
    return NULL;
}

void* WKSizeGetTypeID(void) {
    if (verbose) puts("STUB: WKSizeGetTypeID called");
    return NULL;
}

void* WKSizeGetValue(void) {
    if (verbose) puts("STUB: WKSizeGetValue called");
    return NULL;
}

void* WKStringCopyCFString(void) {
    if (verbose) puts("STUB: WKStringCopyCFString called");
    return NULL;
}

void* WKStringCopyJSString(void) {
    if (verbose) puts("STUB: WKStringCopyJSString called");
    return NULL;
}

void* WKStringCreateWithCFString(void) {
    if (verbose) puts("STUB: WKStringCreateWithCFString called");
    return NULL;
}

void* WKStringCreateWithJSString(void) {
    if (verbose) puts("STUB: WKStringCreateWithJSString called");
    return NULL;
}

void* WKStringCreateWithUTF8CString(void) {
    if (verbose) puts("STUB: WKStringCreateWithUTF8CString called");
    return NULL;
}

void* WKStringGetCharacters(void) {
    if (verbose) puts("STUB: WKStringGetCharacters called");
    return NULL;
}

void* WKStringGetLength(void) {
    if (verbose) puts("STUB: WKStringGetLength called");
    return NULL;
}

void* WKStringGetMaximumUTF8CStringSize(void) {
    if (verbose) puts("STUB: WKStringGetMaximumUTF8CStringSize called");
    return NULL;
}

void* WKStringGetTypeID(void) {
    if (verbose) puts("STUB: WKStringGetTypeID called");
    return NULL;
}

void* WKStringGetUTF8CString(void) {
    if (verbose) puts("STUB: WKStringGetUTF8CString called");
    return NULL;
}

void* WKStringGetUTF8CStringNonStrict(void) {
    if (verbose) puts("STUB: WKStringGetUTF8CStringNonStrict called");
    return NULL;
}

void* WKStringIsEmpty(void) {
    if (verbose) puts("STUB: WKStringIsEmpty called");
    return NULL;
}

void* WKStringIsEqual(void) {
    if (verbose) puts("STUB: WKStringIsEqual called");
    return NULL;
}

void* WKStringIsEqualToUTF8CString(void) {
    if (verbose) puts("STUB: WKStringIsEqualToUTF8CString called");
    return NULL;
}

void* WKStringIsEqualToUTF8CStringIgnoringCase(void) {
    if (verbose) puts("STUB: WKStringIsEqualToUTF8CStringIgnoringCase called");
    return NULL;
}

void* WKTextCheckerSetTestingMode(void) {
    if (verbose) puts("STUB: WKTextCheckerSetTestingMode called");
    return NULL;
}

void* WKUInt64Create(void) {
    if (verbose) puts("STUB: WKUInt64Create called");
    return NULL;
}

void* WKUInt64GetTypeID(void) {
    if (verbose) puts("STUB: WKUInt64GetTypeID called");
    return NULL;
}

void* WKUInt64GetValue(void) {
    if (verbose) puts("STUB: WKUInt64GetValue called");
    return NULL;
}

void* WKURLCopyCFURL(void) {
    if (verbose) puts("STUB: WKURLCopyCFURL called");
    return NULL;
}

void* WKURLCopyHostName(void) {
    if (verbose) puts("STUB: WKURLCopyHostName called");
    return NULL;
}

void* WKURLCopyLastPathComponent(void) {
    if (verbose) puts("STUB: WKURLCopyLastPathComponent called");
    return NULL;
}

void* WKURLCopyPath(void) {
    if (verbose) puts("STUB: WKURLCopyPath called");
    return NULL;
}

void* WKURLCopyScheme(void) {
    if (verbose) puts("STUB: WKURLCopyScheme called");
    return NULL;
}

void* WKURLCopyString(void) {
    if (verbose) puts("STUB: WKURLCopyString called");
    return NULL;
}

void* WKURLCreateWithBaseURL(void) {
    if (verbose) puts("STUB: WKURLCreateWithBaseURL called");
    return NULL;
}

void* WKURLCreateWithCFURL(void) {
    if (verbose) puts("STUB: WKURLCreateWithCFURL called");
    return NULL;
}

void* WKURLCreateWithUTF8CString(void) {
    if (verbose) puts("STUB: WKURLCreateWithUTF8CString called");
    return NULL;
}

void* WKURLGetTypeID(void) {
    if (verbose) puts("STUB: WKURLGetTypeID called");
    return NULL;
}

void* WKURLIsEqual(void) {
    if (verbose) puts("STUB: WKURLIsEqual called");
    return NULL;
}

void* WKURLRequestCopyFirstPartyForCookies(void) {
    if (verbose) puts("STUB: WKURLRequestCopyFirstPartyForCookies called");
    return NULL;
}

void* WKURLRequestCopyHTTPMethod(void) {
    if (verbose) puts("STUB: WKURLRequestCopyHTTPMethod called");
    return NULL;
}

void* WKURLRequestCopyNSURLRequest(void) {
    if (verbose) puts("STUB: WKURLRequestCopyNSURLRequest called");
    return NULL;
}

void* WKURLRequestCopySettingHTTPBody(void) {
    if (verbose) puts("STUB: WKURLRequestCopySettingHTTPBody called");
    return NULL;
}

void* WKURLRequestCopyURL(void) {
    if (verbose) puts("STUB: WKURLRequestCopyURL called");
    return NULL;
}

void* WKURLRequestCreateWithNSURLRequest(void) {
    if (verbose) puts("STUB: WKURLRequestCreateWithNSURLRequest called");
    return NULL;
}

void* WKURLRequestCreateWithWKURL(void) {
    if (verbose) puts("STUB: WKURLRequestCreateWithWKURL called");
    return NULL;
}

void* WKURLRequestGetTypeID(void) {
    if (verbose) puts("STUB: WKURLRequestGetTypeID called");
    return NULL;
}

void* WKURLRequestSetDefaultTimeoutInterval(void) {
    if (verbose) puts("STUB: WKURLRequestSetDefaultTimeoutInterval called");
    return NULL;
}

void* WKURLResponseCopyMIMEType(void) {
    if (verbose) puts("STUB: WKURLResponseCopyMIMEType called");
    return NULL;
}

void* WKURLResponseCopyNSURLResponse(void) {
    if (verbose) puts("STUB: WKURLResponseCopyNSURLResponse called");
    return NULL;
}

void* WKURLResponseCopySuggestedFilename(void) {
    if (verbose) puts("STUB: WKURLResponseCopySuggestedFilename called");
    return NULL;
}

void* WKURLResponseCopyURL(void) {
    if (verbose) puts("STUB: WKURLResponseCopyURL called");
    return NULL;
}

void* WKURLResponseCreateWithNSURLResponse(void) {
    if (verbose) puts("STUB: WKURLResponseCreateWithNSURLResponse called");
    return NULL;
}

void* WKURLResponseGetExpectedContentLength(void) {
    if (verbose) puts("STUB: WKURLResponseGetExpectedContentLength called");
    return NULL;
}

void* WKURLResponseGetTypeID(void) {
    if (verbose) puts("STUB: WKURLResponseGetTypeID called");
    return NULL;
}

void* WKURLResponseHTTPStatusCode(void) {
    if (verbose) puts("STUB: WKURLResponseHTTPStatusCode called");
    return NULL;
}

void* WKURLResponseIsAttachment(void) {
    if (verbose) puts("STUB: WKURLResponseIsAttachment called");
    return NULL;
}

void* WKUserContentControllerAddUserContentFilter(void) {
    if (verbose) puts("STUB: WKUserContentControllerAddUserContentFilter called");
    return NULL;
}

void* WKUserContentControllerAddUserScript(void) {
    if (verbose) puts("STUB: WKUserContentControllerAddUserScript called");
    return NULL;
}

void* WKUserContentControllerCopyUserScripts(void) {
    if (verbose) puts("STUB: WKUserContentControllerCopyUserScripts called");
    return NULL;
}

void* WKUserContentControllerCreate(void) {
    if (verbose) puts("STUB: WKUserContentControllerCreate called");
    return NULL;
}

void* WKUserContentControllerGetTypeID(void) {
    if (verbose) puts("STUB: WKUserContentControllerGetTypeID called");
    return NULL;
}

void* WKUserContentControllerRemoveAllUserContentFilters(void) {
    if (verbose) puts("STUB: WKUserContentControllerRemoveAllUserContentFilters called");
    return NULL;
}

void* WKUserContentControllerRemoveAllUserScripts(void) {
    if (verbose) puts("STUB: WKUserContentControllerRemoveAllUserScripts called");
    return NULL;
}

void* WKUserContentExtensionStoreGetTypeID(void) {
    if (verbose) puts("STUB: WKUserContentExtensionStoreGetTypeID called");
    return NULL;
}

void* WKUserContentURLPatternCopyHost(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternCopyHost called");
    return NULL;
}

void* WKUserContentURLPatternCopyScheme(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternCopyScheme called");
    return NULL;
}

void* WKUserContentURLPatternCreate(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternCreate called");
    return NULL;
}

void* WKUserContentURLPatternGetTypeID(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternGetTypeID called");
    return NULL;
}

void* WKUserContentURLPatternIsValid(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternIsValid called");
    return NULL;
}

void* WKUserContentURLPatternMatchesSubdomains(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternMatchesSubdomains called");
    return NULL;
}

void* WKUserContentURLPatternMatchesURL(void) {
    if (verbose) puts("STUB: WKUserContentURLPatternMatchesURL called");
    return NULL;
}

void* WKUserMediaPermissionCheckGetTypeID(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionCheckGetTypeID called");
    return NULL;
}

void* WKUserMediaPermissionCheckSetUserMediaAccessInfo(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionCheckSetUserMediaAccessInfo called");
    return NULL;
}

void* WKUserMediaPermissionRequestAllow(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionRequestAllow called");
    return NULL;
}

void* WKUserMediaPermissionRequestAudioDeviceUIDs(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionRequestAudioDeviceUIDs called");
    return NULL;
}

void* WKUserMediaPermissionRequestDeny(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionRequestDeny called");
    return NULL;
}

void* WKUserMediaPermissionRequestGetTypeID(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionRequestGetTypeID called");
    return NULL;
}

void* WKUserMediaPermissionRequestVideoDeviceUIDs(void) {
    if (verbose) puts("STUB: WKUserMediaPermissionRequestVideoDeviceUIDs called");
    return NULL;
}

void* WKUserScriptCopySource(void) {
    if (verbose) puts("STUB: WKUserScriptCopySource called");
    return NULL;
}

void* WKUserScriptCreateWithSource(void) {
    if (verbose) puts("STUB: WKUserScriptCreateWithSource called");
    return NULL;
}

void* WKUserScriptGetInjectionTime(void) {
    if (verbose) puts("STUB: WKUserScriptGetInjectionTime called");
    return NULL;
}

void* WKUserScriptGetMainFrameOnly(void) {
    if (verbose) puts("STUB: WKUserScriptGetMainFrameOnly called");
    return NULL;
}

void* WKUserScriptGetTypeID(void) {
    if (verbose) puts("STUB: WKUserScriptGetTypeID called");
    return NULL;
}

void* WKWebArchiveCopyData(void) {
    if (verbose) puts("STUB: WKWebArchiveCopyData called");
    return NULL;
}

void* WKWebArchiveCopyMainResource(void) {
    if (verbose) puts("STUB: WKWebArchiveCopyMainResource called");
    return NULL;
}

void* WKWebArchiveCopySubframeArchives(void) {
    if (verbose) puts("STUB: WKWebArchiveCopySubframeArchives called");
    return NULL;
}

void* WKWebArchiveCopySubresources(void) {
    if (verbose) puts("STUB: WKWebArchiveCopySubresources called");
    return NULL;
}

void* WKWebArchiveCreate(void) {
    if (verbose) puts("STUB: WKWebArchiveCreate called");
    return NULL;
}

void* WKWebArchiveCreateFromRange(void) {
    if (verbose) puts("STUB: WKWebArchiveCreateFromRange called");
    return NULL;
}

void* WKWebArchiveCreateWithData(void) {
    if (verbose) puts("STUB: WKWebArchiveCreateWithData called");
    return NULL;
}

void* WKWebArchiveGetTypeID(void) {
    if (verbose) puts("STUB: WKWebArchiveGetTypeID called");
    return NULL;
}

void* WKWebArchiveResourceCopyData(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceCopyData called");
    return NULL;
}

void* WKWebArchiveResourceCopyMIMEType(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceCopyMIMEType called");
    return NULL;
}

void* WKWebArchiveResourceCopyTextEncoding(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceCopyTextEncoding called");
    return NULL;
}

void* WKWebArchiveResourceCopyURL(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceCopyURL called");
    return NULL;
}

void* WKWebArchiveResourceCreate(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceCreate called");
    return NULL;
}

void* WKWebArchiveResourceGetTypeID(void) {
    if (verbose) puts("STUB: WKWebArchiveResourceGetTypeID called");
    return NULL;
}

void* WKWebsiteDataStoreCreateNonPersistentDataStore(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreCreateNonPersistentDataStore called");
    return NULL;
}

void* WKWebsiteDataStoreGetDefaultDataStore(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreGetDefaultDataStore called");
    return NULL;
}

void* WKWebsiteDataStoreGetFetchCacheOrigins(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreGetFetchCacheOrigins called");
    return NULL;
}

void* WKWebsiteDataStoreGetFetchCacheSizeForOrigin(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreGetFetchCacheSizeForOrigin called");
    return NULL;
}

void* WKWebsiteDataStoreGetResourceLoadStatisticsEnabled(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreGetResourceLoadStatisticsEnabled called");
    return NULL;
}

void* WKWebsiteDataStoreGetTypeID(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreGetTypeID called");
    return NULL;
}

void* WKWebsiteDataStoreIsStatisticsGrandfathered(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreIsStatisticsGrandfathered called");
    return NULL;
}

void* WKWebsiteDataStoreIsStatisticsHasHadUserInteraction(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreIsStatisticsHasHadUserInteraction called");
    return NULL;
}

void* WKWebsiteDataStoreIsStatisticsPrevalentResource(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreIsStatisticsPrevalentResource called");
    return NULL;
}

void* WKWebsiteDataStoreIsStatisticsRegisteredAsRedirectingTo(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreIsStatisticsRegisteredAsRedirectingTo called");
    return NULL;
}

void* WKWebsiteDataStoreIsStatisticsRegisteredAsSubFrameUnder(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreIsStatisticsRegisteredAsSubFrameUnder called");
    return NULL;
}

void* WKWebsiteDataStoreRemoveAllFetchCaches(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreRemoveAllFetchCaches called");
    return NULL;
}

void* WKWebsiteDataStoreRemoveAllIndexedDatabases(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreRemoveAllIndexedDatabases called");
    return NULL;
}

void* WKWebsiteDataStoreRemoveAllServiceWorkerRegistrations(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreRemoveAllServiceWorkerRegistrations called");
    return NULL;
}

void* WKWebsiteDataStoreRemoveFetchCacheForOrigin(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreRemoveFetchCacheForOrigin called");
    return NULL;
}

void* WKWebsiteDataStoreSetResourceLoadStatisticsEnabled(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetResourceLoadStatisticsEnabled called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsGrandfathered(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsGrandfathered called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsGrandfatheringTime(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsGrandfatheringTime called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsHasHadNonRecentUserInteraction(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsHasHadNonRecentUserInteraction called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsHasHadUserInteraction(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsHasHadUserInteraction called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsLastSeen(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsLastSeen called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsMaxStatisticsEntries(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsMaxStatisticsEntries called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsMinimumTimeBetweenDataRecordsRemoval(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsMinimumTimeBetweenDataRecordsRemoval called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsNotifyPagesWhenDataRecordsWereScanned(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsNotifyPagesWhenDataRecordsWereScanned called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsNotifyPagesWhenTelemetryWasCaptured(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsNotifyPagesWhenTelemetryWasCaptured called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsPrevalentResource(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsPrevalentResource called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsPruneEntriesDownTo(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsPruneEntriesDownTo called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsShouldClassifyResourcesBeforeDataRecordsRemoval(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsShouldClassifyResourcesBeforeDataRecordsRemoval called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsShouldPartitionCookiesForHost(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsShouldPartitionCookiesForHost called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsSubframeUnderTopFrameOrigin(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsSubframeUnderTopFrameOrigin called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsSubresourceUnderTopFrameOrigin(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsSubresourceUnderTopFrameOrigin called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsSubresourceUniqueRedirectTo(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsSubresourceUniqueRedirectTo called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsTimeToLiveCookiePartitionFree(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsTimeToLiveCookiePartitionFree called");
    return NULL;
}

void* WKWebsiteDataStoreSetStatisticsTimeToLiveUserInteraction(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreSetStatisticsTimeToLiveUserInteraction called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsClearInMemoryAndPersistentStore(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsClearInMemoryAndPersistentStore called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsClearInMemoryAndPersistentStoreModifiedSinceHours(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsClearInMemoryAndPersistentStoreModifiedSinceHours called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsClearThroughWebsiteDataRemoval(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsClearThroughWebsiteDataRemoval called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsProcessStatisticsAndDataRecords(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsProcessStatisticsAndDataRecords called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsResetToConsistentState(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsResetToConsistentState called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsSubmitTelemetry(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsSubmitTelemetry called");
    return NULL;
}

void* WKWebsiteDataStoreStatisticsUpdateCookiePartitioning(void) {
    if (verbose) puts("STUB: WKWebsiteDataStoreStatisticsUpdateCookiePartitioning called");
    return NULL;
}

void* WKWebsitePoliciesCopyCustomHeaderFields(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesCopyCustomHeaderFields called");
    return NULL;
}

void* WKWebsitePoliciesCreate(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesCreate called");
    return NULL;
}

void* WKWebsitePoliciesGetAllowedAutoplayQuirks(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesGetAllowedAutoplayQuirks called");
    return NULL;
}

void* WKWebsitePoliciesGetAutoplayPolicy(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesGetAutoplayPolicy called");
    return NULL;
}

void* WKWebsitePoliciesGetContentBlockersEnabled(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesGetContentBlockersEnabled called");
    return NULL;
}

void* WKWebsitePoliciesGetDataStore(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesGetDataStore called");
    return NULL;
}

void* WKWebsitePoliciesGetTypeID(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesGetTypeID called");
    return NULL;
}

void* WKWebsitePoliciesSetAllowedAutoplayQuirks(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesSetAllowedAutoplayQuirks called");
    return NULL;
}

void* WKWebsitePoliciesSetAutoplayPolicy(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesSetAutoplayPolicy called");
    return NULL;
}

void* WKWebsitePoliciesSetContentBlockersEnabled(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesSetContentBlockersEnabled called");
    return NULL;
}

void* WKWebsitePoliciesSetCustomHeaderFields(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesSetCustomHeaderFields called");
    return NULL;
}

void* WKWebsitePoliciesSetDataStore(void) {
    if (verbose) puts("STUB: WKWebsitePoliciesSetDataStore called");
    return NULL;
}

void* WKWindowFeaturesGetTypeID(void) {
    if (verbose) puts("STUB: WKWindowFeaturesGetTypeID called");
    return NULL;
}

void* WebContentServiceInitializer(void) {
    if (verbose) puts("STUB: WebContentServiceInitializer called");
    return NULL;
}

void* _WKSetCrashReportApplicationSpecificInformation(void) {
    if (verbose) puts("STUB: _WKSetCrashReportApplicationSpecificInformation called");
    return NULL;
}

void* _ZN6WebKit20EnvironmentUtilities27stripValuesEndingWithStringEPKcS2_(void) {
    if (verbose) puts("STUB: _ZN6WebKit20EnvironmentUtilities27stripValuesEndingWithStringEPKcS2_ called");
    return NULL;
}

void* _ZN6WebKit44setCrashReportApplicationSpecificInformationEPK10__CFString(void) {
    if (verbose) puts("STUB: _ZN6WebKit44setCrashReportApplicationSpecificInformationEPK10__CFString called");
    return NULL;
}
