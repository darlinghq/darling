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


#include <AVKit/AVKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AVAddAudibleMediaSelectionOptionsToMenu(void)
{
    if (verbose) puts("STUB: AVAddAudibleMediaSelectionOptionsToMenu called");
    return NULL;
}

void* AVAddLegibleMediaSelectionOptionsToMenu(void)
{
    if (verbose) puts("STUB: AVAddLegibleMediaSelectionOptionsToMenu called");
    return NULL;
}

void* AVAssetSlowMotionTimeRangeFromNormalizedSlowMotionTimeRange(void)
{
    if (verbose) puts("STUB: AVAssetSlowMotionTimeRangeFromNormalizedSlowMotionTimeRange called");
    return NULL;
}

void* AVAsynchronousLocalizedString(void)
{
    if (verbose) puts("STUB: AVAsynchronousLocalizedString called");
    return NULL;
}

void* AVBundle(void)
{
    if (verbose) puts("STUB: AVBundle called");
    return NULL;
}

void* AVComparePointDistances(void)
{
    if (verbose) puts("STUB: AVComparePointDistances called");
    return NULL;
}

void* AVCompareSizeAspectRatios(void)
{
    if (verbose) puts("STUB: AVCompareSizeAspectRatios called");
    return NULL;
}

void* AVConstrainRectInsideRectWithoutResizing(void)
{
    if (verbose) puts("STUB: AVConstrainRectInsideRectWithoutResizing called");
    return NULL;
}

void* AVDisableUserIdleDisplaySleep(void)
{
    if (verbose) puts("STUB: AVDisableUserIdleDisplaySleep called");
    return NULL;
}

void* AVDisableUserIdleSystemSleep(void)
{
    if (verbose) puts("STUB: AVDisableUserIdleSystemSleep called");
    return NULL;
}

void* AVEnableUserIdleDisplaySleep(void)
{
    if (verbose) puts("STUB: AVEnableUserIdleDisplaySleep called");
    return NULL;
}

void* AVEnableUserIdleSystemSleep(void)
{
    if (verbose) puts("STUB: AVEnableUserIdleSystemSleep called");
    return NULL;
}

void* AVLocalizedString(void)
{
    if (verbose) puts("STUB: AVLocalizedString called");
    return NULL;
}

void* AVLocalizedStringFromTable(void)
{
    if (verbose) puts("STUB: AVLocalizedStringFromTable called");
    return NULL;
}

void* AVMarginEqual(void)
{
    if (verbose) puts("STUB: AVMarginEqual called");
    return NULL;
}

void* AVMarginFromString(void)
{
    if (verbose) puts("STUB: AVMarginFromString called");
    return NULL;
}

void* AVNormalizedSlowMotionTimeRangeFromAssetSlowMotionTimeRange(void)
{
    if (verbose) puts("STUB: AVNormalizedSlowMotionTimeRangeFromAssetSlowMotionTimeRange called");
    return NULL;
}

void* AVRectHasAspectRatio(void)
{
    if (verbose) puts("STUB: AVRectHasAspectRatio called");
    return NULL;
}

void* AVRectIsValid(void)
{
    if (verbose) puts("STUB: AVRectIsValid called");
    return NULL;
}

void* AVStringFromMargin(void)
{
    if (verbose) puts("STUB: AVStringFromMargin called");
    return NULL;
}

void* AVTrimViewDescriptionForPart(void)
{
    if (verbose) puts("STUB: AVTrimViewDescriptionForPart called");
    return NULL;
}

void* AVTrimViewFocusedPart(void)
{
    if (verbose) puts("STUB: AVTrimViewFocusedPart called");
    return NULL;
}

void* AVTrimViewNudgePartInDirection(void)
{
    if (verbose) puts("STUB: AVTrimViewNudgePartInDirection called");
    return NULL;
}

void* AVTrimViewRectForPart(void)
{
    if (verbose) puts("STUB: AVTrimViewRectForPart called");
    return NULL;
}

void* AVTrimViewSetValueForPartWithValidation(void)
{
    if (verbose) puts("STUB: AVTrimViewSetValueForPartWithValidation called");
    return NULL;
}

void* AVTrimViewValueForPart(void)
{
    if (verbose) puts("STUB: AVTrimViewValueForPart called");
    return NULL;
}

void* _AVDebugLog(void)
{
    if (verbose) puts("STUB: _AVDebugLog called");
    return NULL;
}

void* _AVLog(void)
{
    if (verbose) puts("STUB: _AVLog called");
    return NULL;
}

void* _AVMethodProem(void)
{
    if (verbose) puts("STUB: _AVMethodProem called");
    return NULL;
}

void* makeActionButton(void)
{
    if (verbose) puts("STUB: makeActionButton called");
    return NULL;
}

void* makeAudioLevelIndicatorView(void)
{
    if (verbose) puts("STUB: makeAudioLevelIndicatorView called");
    return NULL;
}

void* makeAuxiliaryControlsView(void)
{
    if (verbose) puts("STUB: makeAuxiliaryControlsView called");
    return NULL;
}

void* makeCancelButton(void)
{
    if (verbose) puts("STUB: makeCancelButton called");
    return NULL;
}

void* makeCaptureDeviceErrorImageView(void)
{
    if (verbose) puts("STUB: makeCaptureDeviceErrorImageView called");
    return NULL;
}

void* makeCaptureDeviceSelectionButton(void)
{
    if (verbose) puts("STUB: makeCaptureDeviceSelectionButton called");
    return NULL;
}

void* makeCaptureVolumeControl(void)
{
    if (verbose) puts("STUB: makeCaptureVolumeControl called");
    return NULL;
}

void* makeChapterButton(void)
{
    if (verbose) puts("STUB: makeChapterButton called");
    return NULL;
}

void* makeExternalPlaybackButton(void)
{
    if (verbose) puts("STUB: makeExternalPlaybackButton called");
    return NULL;
}

void* makeFinishingRecordingProgressIndicator(void)
{
    if (verbose) puts("STUB: makeFinishingRecordingProgressIndicator called");
    return NULL;
}

void* makeFloatingControlsPlayPauseButton(void)
{
    if (verbose) puts("STUB: makeFloatingControlsPlayPauseButton called");
    return NULL;
}

void* makeFloatingControlsScanBackwardButton(void)
{
    if (verbose) puts("STUB: makeFloatingControlsScanBackwardButton called");
    return NULL;
}

void* makeFloatingControlsScanForwardButton(void)
{
    if (verbose) puts("STUB: makeFloatingControlsScanForwardButton called");
    return NULL;
}

void* makeFloatingControlsSeekFrameBackwardButton(void)
{
    if (verbose) puts("STUB: makeFloatingControlsSeekFrameBackwardButton called");
    return NULL;
}

void* makeFloatingControlsSeekFrameForwardButton(void)
{
    if (verbose) puts("STUB: makeFloatingControlsSeekFrameForwardButton called");
    return NULL;
}

void* makeFullScreenButton(void)
{
    if (verbose) puts("STUB: makeFullScreenButton called");
    return NULL;
}

void* makeGotoEndOfSeekableRangesButton(void)
{
    if (verbose) puts("STUB: makeGotoEndOfSeekableRangesButton called");
    return NULL;
}

void* makeMediaSelectionButton(void)
{
    if (verbose) puts("STUB: makeMediaSelectionButton called");
    return NULL;
}

void* makePlayPauseButton(void)
{
    if (verbose) puts("STUB: makePlayPauseButton called");
    return NULL;
}

void* makeRecordButtonFloatingStyle(void)
{
    if (verbose) puts("STUB: makeRecordButtonFloatingStyle called");
    return NULL;
}

void* makeRecordButtonInlineStyle(void)
{
    if (verbose) puts("STUB: makeRecordButtonInlineStyle called");
    return NULL;
}

void* makeRecordingStatusView(void)
{
    if (verbose) puts("STUB: makeRecordingStatusView called");
    return NULL;
}

void* makeRedDotViewFloatingStyle(void)
{
    if (verbose) puts("STUB: makeRedDotViewFloatingStyle called");
    return NULL;
}

void* makeRedDotViewInlineStyle(void)
{
    if (verbose) puts("STUB: makeRedDotViewInlineStyle called");
    return NULL;
}

void* makeScrubber(void)
{
    if (verbose) puts("STUB: makeScrubber called");
    return NULL;
}

void* makeShareButton(void)
{
    if (verbose) puts("STUB: makeShareButton called");
    return NULL;
}

void* makeSkipBackwardThirtySecondsButton(void)
{
    if (verbose) puts("STUB: makeSkipBackwardThirtySecondsButton called");
    return NULL;
}

void* makeTimelineControl(void)
{
    if (verbose) puts("STUB: makeTimelineControl called");
    return NULL;
}

void* makeTrimButton(void)
{
    if (verbose) puts("STUB: makeTrimButton called");
    return NULL;
}

void* makeTrimView(void)
{
    if (verbose) puts("STUB: makeTrimView called");
    return NULL;
}

void* makeVolumeControl(void)
{
    if (verbose) puts("STUB: makeVolumeControl called");
    return NULL;
}

void* makeZoomHorizontallyButton(void)
{
    if (verbose) puts("STUB: makeZoomHorizontallyButton called");
    return NULL;
}

void* makeZoomVerticallyButton(void)
{
    if (verbose) puts("STUB: makeZoomVerticallyButton called");
    return NULL;
}
