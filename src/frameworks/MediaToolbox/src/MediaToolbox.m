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


#include <MediaToolbox/MediaToolbox.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AudioMentorDispose(void)
{
    if (verbose) puts("STUB: AudioMentorDispose called");
    return NULL;
}

void* AudioMentorInvalidate(void)
{
    if (verbose) puts("STUB: AudioMentorInvalidate called");
    return NULL;
}

void* AudioMentorNew(void)
{
    if (verbose) puts("STUB: AudioMentorNew called");
    return NULL;
}

void* AudioMentorSetClientPID(void)
{
    if (verbose) puts("STUB: AudioMentorSetClientPID called");
    return NULL;
}

void* AudioMentorSetModeToDoNothing(void)
{
    if (verbose) puts("STUB: AudioMentorSetModeToDoNothing called");
    return NULL;
}

void* AudioMentorSetModeToEmptyEdit(void)
{
    if (verbose) puts("STUB: AudioMentorSetModeToEmptyEdit called");
    return NULL;
}

void* AudioMentorSetModeToForwardPlayback(void)
{
    if (verbose) puts("STUB: AudioMentorSetModeToForwardPlayback called");
    return NULL;
}

void* AudioMentorSetModeToReversePlayback(void)
{
    if (verbose) puts("STUB: AudioMentorSetModeToReversePlayback called");
    return NULL;
}

void* AudioMentorSetThrottleForBackground(void)
{
    if (verbose) puts("STUB: AudioMentorSetThrottleForBackground called");
    return NULL;
}

void* AudioMentorSetTimeRemapCallback(void)
{
    if (verbose) puts("STUB: AudioMentorSetTimeRemapCallback called");
    return NULL;
}

void* DeMoofMovieFile(void)
{
    if (verbose) puts("STUB: DeMoofMovieFile called");
    return NULL;
}

void* EditMentorDispose(void)
{
    if (verbose) puts("STUB: EditMentorDispose called");
    return NULL;
}

void* EditMentorNewWithChildAudioMentor(void)
{
    if (verbose) puts("STUB: EditMentorNewWithChildAudioMentor called");
    return NULL;
}

void* EditMentorNewWithChildAudioMentorAndSingleEditSegment(void)
{
    if (verbose) puts("STUB: EditMentorNewWithChildAudioMentorAndSingleEditSegment called");
    return NULL;
}

void* EditMentorNewWithChildEditMentor(void)
{
    if (verbose) puts("STUB: EditMentorNewWithChildEditMentor called");
    return NULL;
}

void* EditMentorNewWithChildEditMentorAndSingleEditSegment(void)
{
    if (verbose) puts("STUB: EditMentorNewWithChildEditMentorAndSingleEditSegment called");
    return NULL;
}

void* EditMentorNewWithChildVideoMentor(void)
{
    if (verbose) puts("STUB: EditMentorNewWithChildVideoMentor called");
    return NULL;
}

void* EditMentorNewWithMentorCallbacks(void)
{
    if (verbose) puts("STUB: EditMentorNewWithMentorCallbacks called");
    return NULL;
}

void* EditMentorSetDebugName(void)
{
    if (verbose) puts("STUB: EditMentorSetDebugName called");
    return NULL;
}

void* EditMentorSetModeToDoNothing(void)
{
    if (verbose) puts("STUB: EditMentorSetModeToDoNothing called");
    return NULL;
}

void* EditMentorSetModeToEmptyEdit(void)
{
    if (verbose) puts("STUB: EditMentorSetModeToEmptyEdit called");
    return NULL;
}

void* EditMentorSetModeToForwardPlayback(void)
{
    if (verbose) puts("STUB: EditMentorSetModeToForwardPlayback called");
    return NULL;
}

void* EditMentorSetModeToReversePlayback(void)
{
    if (verbose) puts("STUB: EditMentorSetModeToReversePlayback called");
    return NULL;
}

void* EditMentorSetModeToScrub(void)
{
    if (verbose) puts("STUB: EditMentorSetModeToScrub called");
    return NULL;
}

void* EditMentorSetProperty(void)
{
    if (verbose) puts("STUB: EditMentorSetProperty called");
    return NULL;
}

void* EditMentorSetTimeRemapCallback(void)
{
    if (verbose) puts("STUB: EditMentorSetTimeRemapCallback called");
    return NULL;
}

void* ExtraPrivatePlayerBeginInterruption(void)
{
    if (verbose) puts("STUB: ExtraPrivatePlayerBeginInterruption called");
    return NULL;
}

void* ExtraPrivatePlayerCopyProperty(void)
{
    if (verbose) puts("STUB: ExtraPrivatePlayerCopyProperty called");
    return NULL;
}

void* ExtraPrivatePlayerEndInterruption(void)
{
    if (verbose) puts("STUB: ExtraPrivatePlayerEndInterruption called");
    return NULL;
}

void* ExtraPrivatePlayerGetNotificationCenter(void)
{
    if (verbose) puts("STUB: ExtraPrivatePlayerGetNotificationCenter called");
    return NULL;
}

void* ExtraPrivatePlayerSetProperty(void)
{
    if (verbose) puts("STUB: ExtraPrivatePlayerSetProperty called");
    return NULL;
}

void* FCSupport_CopyChapterTimeRangesForChapterGroup(void)
{
    if (verbose) puts("STUB: FCSupport_CopyChapterTimeRangesForChapterGroup called");
    return NULL;
}

void* FCSupport_CopyChapterTimesForChapterGroup(void)
{
    if (verbose) puts("STUB: FCSupport_CopyChapterTimesForChapterGroup called");
    return NULL;
}

void* FPSupport_CopyCAContext(void)
{
    if (verbose) puts("STUB: FPSupport_CopyCAContext called");
    return NULL;
}

void* FPSupport_CopyDestinationPixelBufferAttributesWithIOSurfaceSupport(void)
{
    if (verbose) puts("STUB: FPSupport_CopyDestinationPixelBufferAttributesWithIOSurfaceSupport called");
    return NULL;
}

void* FPSupport_CopyMainDisplayCAName(void)
{
    if (verbose) puts("STUB: FPSupport_CopyMainDisplayCAName called");
    return NULL;
}

void* FPSupport_CopyMainDisplayCGDisplayID(void)
{
    if (verbose) puts("STUB: FPSupport_CopyMainDisplayCGDisplayID called");
    return NULL;
}

void* FPSupport_CreateAllVideoRangesSupportedByDisplays(void)
{
    if (verbose) puts("STUB: FPSupport_CreateAllVideoRangesSupportedByDisplays called");
    return NULL;
}

void* FPSupport_CreateDestinationPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: FPSupport_CreateDestinationPixelBufferAttributes called");
    return NULL;
}

void* FPSupport_DeleteCASlot(void)
{
    if (verbose) puts("STUB: FPSupport_DeleteCASlot called");
    return NULL;
}

void* FPSupport_DeleteCASlotsInArray(void)
{
    if (verbose) puts("STUB: FPSupport_DeleteCASlotsInArray called");
    return NULL;
}

void* FPSupport_EnsureCAImageQueue(void)
{
    if (verbose) puts("STUB: FPSupport_EnsureCAImageQueue called");
    return NULL;
}

void* FPSupport_ForceVideoRangeDisplayChangeNotification(void)
{
    if (verbose) puts("STUB: FPSupport_ForceVideoRangeDisplayChangeNotification called");
    return NULL;
}

void* FPSupport_GetBestDisplayInfoForDisplay(void)
{
    if (verbose) puts("STUB: FPSupport_GetBestDisplayInfoForDisplay called");
    return NULL;
}

void* FPSupport_GetClosestCommonRefreshIntervalForRate(void)
{
    if (verbose) puts("STUB: FPSupport_GetClosestCommonRefreshIntervalForRate called");
    return NULL;
}

void* FPSupport_GetCurrentDisplayModeVideoRangeAndSizeAndFrameRate(void)
{
    if (verbose) puts("STUB: FPSupport_GetCurrentDisplayModeVideoRangeAndSizeAndFrameRate called");
    return NULL;
}

void* FPSupport_GetDefaultTrackIDForMediaType(void)
{
    if (verbose) puts("STUB: FPSupport_GetDefaultTrackIDForMediaType called");
    return NULL;
}

void* FPSupport_GetDisplayInfoForCADisplay(void)
{
    if (verbose) puts("STUB: FPSupport_GetDisplayInfoForCADisplay called");
    return NULL;
}

void* FPSupport_GetDisplayRefreshInformation(void)
{
    if (verbose) puts("STUB: FPSupport_GetDisplayRefreshInformation called");
    return NULL;
}

void* FPSupport_GetDisplayVideoRangeNotificationSingleton(void)
{
    if (verbose) puts("STUB: FPSupport_GetDisplayVideoRangeNotificationSingleton called");
    return NULL;
}

void* FPSupport_GetFirstEnabledTrackIDForMediaType(void)
{
    if (verbose) puts("STUB: FPSupport_GetFirstEnabledTrackIDForMediaType called");
    return NULL;
}

void* FPSupport_GetMainDisplayVideoRangeAndSizeAndFrameRate(void)
{
    if (verbose) puts("STUB: FPSupport_GetMainDisplayVideoRangeAndSizeAndFrameRate called");
    return NULL;
}

void* FPSupport_GetModeString(void)
{
    if (verbose) puts("STUB: FPSupport_GetModeString called");
    return NULL;
}

void* FPSupport_GetVideoRangeForCoreDisplay(void)
{
    if (verbose) puts("STUB: FPSupport_GetVideoRangeForCoreDisplay called");
    return NULL;
}

void* FPSupport_IsExternalDisplay(void)
{
    if (verbose) puts("STUB: FPSupport_IsExternalDisplay called");
    return NULL;
}

void* FVDUtilsCreateDisplayUUID(void)
{
    if (verbose) puts("STUB: FVDUtilsCreateDisplayUUID called");
    return NULL;
}

void* FVDUtilsH264DecoderSupports444(void)
{
    if (verbose) puts("STUB: FVDUtilsH264DecoderSupports444 called");
    return NULL;
}

void* FVDUtilsH264EncoderSupports1080p60(void)
{
    if (verbose) puts("STUB: FVDUtilsH264EncoderSupports1080p60 called");
    return NULL;
}

void* FVDUtilsH264EncoderSupports444(void)
{
    if (verbose) puts("STUB: FVDUtilsH264EncoderSupports444 called");
    return NULL;
}

void* FVDUtilsH264EncoderSupports4K30(void)
{
    if (verbose) puts("STUB: FVDUtilsH264EncoderSupports4K30 called");
    return NULL;
}

void* FVDUtilsH264EncoderSupportsBGRA(void)
{
    if (verbose) puts("STUB: FVDUtilsH264EncoderSupportsBGRA called");
    return NULL;
}

void* FVDUtilsH264EncoderSupportsScaling(void)
{
    if (verbose) puts("STUB: FVDUtilsH264EncoderSupportsScaling called");
    return NULL;
}

void* FVDUtilsHEVCDecodeSupported(void)
{
    if (verbose) puts("STUB: FVDUtilsHEVCDecodeSupported called");
    return NULL;
}

void* FVDUtilsHEVCEncoderPreferred(void)
{
    if (verbose) puts("STUB: FVDUtilsHEVCEncoderPreferred called");
    return NULL;
}

void* FVDUtilsHEVCEncoderSupports44410(void)
{
    if (verbose) puts("STUB: FVDUtilsHEVCEncoderSupports44410 called");
    return NULL;
}

void* FVDUtilsHEVCEncoderSupports4K60(void)
{
    if (verbose) puts("STUB: FVDUtilsHEVCEncoderSupports4K60 called");
    return NULL;
}

void* FVDUtilsMainDisplaySupportsExtendedColor(void)
{
    if (verbose) puts("STUB: FVDUtilsMainDisplaySupportsExtendedColor called");
    return NULL;
}

void* FigActivitySchedulerCreateForNewThread(void)
{
    if (verbose) puts("STUB: FigActivitySchedulerCreateForNewThread called");
    return NULL;
}

void* FigActivitySchedulerGetClassID(void)
{
    if (verbose) puts("STUB: FigActivitySchedulerGetClassID called");
    return NULL;
}

void* FigActivitySchedulerGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigActivitySchedulerGetFigBaseObject called");
    return NULL;
}

void* FigActivitySchedulerGetTypeID(void)
{
    if (verbose) puts("STUB: FigActivitySchedulerGetTypeID called");
    return NULL;
}

void* FigAirPlaySecureStopRouterFinalizeRecordAndCommitToDisk(void)
{
    if (verbose) puts("STUB: FigAirPlaySecureStopRouterFinalizeRecordAndCommitToDisk called");
    return NULL;
}

void* FigAlternateAllowedCPCFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateAllowedCPCFilterCreate called");
    return NULL;
}

void* FigAlternateContinuousFramesOnlyFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateContinuousFramesOnlyFilterCreate called");
    return NULL;
}

void* FigAlternateCopyAlternateDictionary(void)
{
    if (verbose) puts("STUB: FigAlternateCopyAlternateDictionary called");
    return NULL;
}

void* FigAlternateCreate(void)
{
    if (verbose) puts("STUB: FigAlternateCreate called");
    return NULL;
}

void* FigAlternateCreateDescriptionString(void)
{
    if (verbose) puts("STUB: FigAlternateCreateDescriptionString called");
    return NULL;
}

void* FigAlternateFilterCopyFiltered(void)
{
    if (verbose) puts("STUB: FigAlternateFilterCopyFiltered called");
    return NULL;
}

void* FigAlternateFilterGetClassID(void)
{
    if (verbose) puts("STUB: FigAlternateFilterGetClassID called");
    return NULL;
}

void* FigAlternateFilterGetTypeID(void)
{
    if (verbose) puts("STUB: FigAlternateFilterGetTypeID called");
    return NULL;
}

void* FigAlternateFilterInsert(void)
{
    if (verbose) puts("STUB: FigAlternateFilterInsert called");
    return NULL;
}

void* FigAlternateFrameRateCapFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateFrameRateCapFilterCreate called");
    return NULL;
}

void* FigAlternateGetAllowedCPC(void)
{
    if (verbose) puts("STUB: FigAlternateGetAllowedCPC called");
    return NULL;
}

void* FigAlternateGetAlternateIndex(void)
{
    if (verbose) puts("STUB: FigAlternateGetAlternateIndex called");
    return NULL;
}

void* FigAlternateGetAudioChannelCount(void)
{
    if (verbose) puts("STUB: FigAlternateGetAudioChannelCount called");
    return NULL;
}

void* FigAlternateGetAudioGroupIDString(void)
{
    if (verbose) puts("STUB: FigAlternateGetAudioGroupIDString called");
    return NULL;
}

void* FigAlternateGetAudioGroupLocatorMap(void)
{
    if (verbose) puts("STUB: FigAlternateGetAudioGroupLocatorMap called");
    return NULL;
}

void* FigAlternateGetAverageBitRate(void)
{
    if (verbose) puts("STUB: FigAlternateGetAverageBitRate called");
    return NULL;
}

void* FigAlternateGetBestAudioFormat(void)
{
    if (verbose) puts("STUB: FigAlternateGetBestAudioFormat called");
    return NULL;
}

void* FigAlternateGetBestVideoFormat(void)
{
    if (verbose) puts("STUB: FigAlternateGetBestVideoFormat called");
    return NULL;
}

void* FigAlternateGetCacheLookupIdentifier(void)
{
    if (verbose) puts("STUB: FigAlternateGetCacheLookupIdentifier called");
    return NULL;
}

void* FigAlternateGetClassID(void)
{
    if (verbose) puts("STUB: FigAlternateGetClassID called");
    return NULL;
}

void* FigAlternateGetClosedCaptionGroupIDString(void)
{
    if (verbose) puts("STUB: FigAlternateGetClosedCaptionGroupIDString called");
    return NULL;
}

void* FigAlternateGetClosedCaptionGroupLocatorMap(void)
{
    if (verbose) puts("STUB: FigAlternateGetClosedCaptionGroupLocatorMap called");
    return NULL;
}

void* FigAlternateGetCodecString(void)
{
    if (verbose) puts("STUB: FigAlternateGetCodecString called");
    return NULL;
}

void* FigAlternateGetCurrentErrorCategory(void)
{
    if (verbose) puts("STUB: FigAlternateGetCurrentErrorCategory called");
    return NULL;
}

void* FigAlternateGetDeclaredAudioChannelCount(void)
{
    if (verbose) puts("STUB: FigAlternateGetDeclaredAudioChannelCount called");
    return NULL;
}

void* FigAlternateGetFrameRate(void)
{
    if (verbose) puts("STUB: FigAlternateGetFrameRate called");
    return NULL;
}

void* FigAlternateGetHDCPLevel(void)
{
    if (verbose) puts("STUB: FigAlternateGetHDCPLevel called");
    return NULL;
}

void* FigAlternateGetHDCPString(void)
{
    if (verbose) puts("STUB: FigAlternateGetHDCPString called");
    return NULL;
}

void* FigAlternateGetLastKnownValueForIsFullyCached(void)
{
    if (verbose) puts("STUB: FigAlternateGetLastKnownValueForIsFullyCached called");
    return NULL;
}

void* FigAlternateGetNormalizedPeakBitrate(void)
{
    if (verbose) puts("STUB: FigAlternateGetNormalizedPeakBitrate called");
    return NULL;
}

void* FigAlternateGetPeakBitRate(void)
{
    if (verbose) puts("STUB: FigAlternateGetPeakBitRate called");
    return NULL;
}

void* FigAlternateGetPlaylistAlternateKeyURL(void)
{
    if (verbose) puts("STUB: FigAlternateGetPlaylistAlternateKeyURL called");
    return NULL;
}

void* FigAlternateGetRankingScore(void)
{
    if (verbose) puts("STUB: FigAlternateGetRankingScore called");
    return NULL;
}

void* FigAlternateGetRankingScoreOrAverageBWWithVideoCodecScore(void)
{
    if (verbose) puts("STUB: FigAlternateGetRankingScoreOrAverageBWWithVideoCodecScore called");
    return NULL;
}

void* FigAlternateGetRankingScoreOrPeakBWWithVideoCodecScore(void)
{
    if (verbose) puts("STUB: FigAlternateGetRankingScoreOrPeakBWWithVideoCodecScore called");
    return NULL;
}

void* FigAlternateGetRenditionInfoForMediaType(void)
{
    if (verbose) puts("STUB: FigAlternateGetRenditionInfoForMediaType called");
    return NULL;
}

void* FigAlternateGetResolution(void)
{
    if (verbose) puts("STUB: FigAlternateGetResolution called");
    return NULL;
}

void* FigAlternateGetStableStreamIdentifier(void)
{
    if (verbose) puts("STUB: FigAlternateGetStableStreamIdentifier called");
    return NULL;
}

void* FigAlternateGetStableStreamIdentifierForMediaSubstream(void)
{
    if (verbose) puts("STUB: FigAlternateGetStableStreamIdentifierForMediaSubstream called");
    return NULL;
}

void* FigAlternateGetStartupScore(void)
{
    if (verbose) puts("STUB: FigAlternateGetStartupScore called");
    return NULL;
}

void* FigAlternateGetSubtitleGroupIDString(void)
{
    if (verbose) puts("STUB: FigAlternateGetSubtitleGroupIDString called");
    return NULL;
}

void* FigAlternateGetSubtitleGroupLocatorMap(void)
{
    if (verbose) puts("STUB: FigAlternateGetSubtitleGroupLocatorMap called");
    return NULL;
}

void* FigAlternateGetTypeID(void)
{
    if (verbose) puts("STUB: FigAlternateGetTypeID called");
    return NULL;
}

void* FigAlternateGetURLForCacheLookup(void)
{
    if (verbose) puts("STUB: FigAlternateGetURLForCacheLookup called");
    return NULL;
}

void* FigAlternateGetVariantBitrateRank(void)
{
    if (verbose) puts("STUB: FigAlternateGetVariantBitrateRank called");
    return NULL;
}

void* FigAlternateGetVideoFormatLoggingStringFromVideoFormat(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoFormatLoggingStringFromVideoFormat called");
    return NULL;
}

void* FigAlternateGetVideoGroupIDString(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoGroupIDString called");
    return NULL;
}

void* FigAlternateGetVideoGroupLocatorMap(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoGroupLocatorMap called");
    return NULL;
}

void* FigAlternateGetVideoQualityIndex(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoQualityIndex called");
    return NULL;
}

void* FigAlternateGetVideoRange(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoRange called");
    return NULL;
}

void* FigAlternateGetVideoRangeString(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoRangeString called");
    return NULL;
}

void* FigAlternateGetVideoRangeStringFromVideoRange(void)
{
    if (verbose) puts("STUB: FigAlternateGetVideoRangeStringFromVideoRange called");
    return NULL;
}

void* FigAlternateHDCPFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateHDCPFilterCreate called");
    return NULL;
}

void* FigAlternateHasAudio(void)
{
    if (verbose) puts("STUB: FigAlternateHasAudio called");
    return NULL;
}

void* FigAlternateHasAudioDeclared(void)
{
    if (verbose) puts("STUB: FigAlternateHasAudioDeclared called");
    return NULL;
}

void* FigAlternateHasAudioFormat(void)
{
    if (verbose) puts("STUB: FigAlternateHasAudioFormat called");
    return NULL;
}

void* FigAlternateHasUndeclaredVideo(void)
{
    if (verbose) puts("STUB: FigAlternateHasUndeclaredVideo called");
    return NULL;
}

void* FigAlternateHasUnknownCodecs(void)
{
    if (verbose) puts("STUB: FigAlternateHasUnknownCodecs called");
    return NULL;
}

void* FigAlternateHasValidCodecs(void)
{
    if (verbose) puts("STUB: FigAlternateHasValidCodecs called");
    return NULL;
}

void* FigAlternateHasVideo(void)
{
    if (verbose) puts("STUB: FigAlternateHasVideo called");
    return NULL;
}

void* FigAlternateHasVideoDeclared(void)
{
    if (verbose) puts("STUB: FigAlternateHasVideoDeclared called");
    return NULL;
}

void* FigAlternateHasVideoFormat(void)
{
    if (verbose) puts("STUB: FigAlternateHasVideoFormat called");
    return NULL;
}

void* FigAlternateIFramesOnlyFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateIFramesOnlyFilterCreate called");
    return NULL;
}

void* FigAlternateIsAudioOnly(void)
{
    if (verbose) puts("STUB: FigAlternateIsAudioOnly called");
    return NULL;
}

void* FigAlternateIsFairPlayStreamingVideoAlternate(void)
{
    if (verbose) puts("STUB: FigAlternateIsFairPlayStreamingVideoAlternate called");
    return NULL;
}

void* FigAlternateIsIFrameOnly(void)
{
    if (verbose) puts("STUB: FigAlternateIsIFrameOnly called");
    return NULL;
}

void* FigAlternateLowPowerModeFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateLowPowerModeFilterCreate called");
    return NULL;
}

void* FigAlternateLowestVideoThenAudioFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateLowestVideoThenAudioFilterCreate called");
    return NULL;
}

void* FigAlternateMediaSelectionAudibleFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateMediaSelectionAudibleFilterCreate called");
    return NULL;
}

void* FigAlternateMediaValidationFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateMediaValidationFilterCreate called");
    return NULL;
}

void* FigAlternateMeetsVideoChoice(void)
{
    if (verbose) puts("STUB: FigAlternateMeetsVideoChoice called");
    return NULL;
}

void* FigAlternateNextHighestPeakBitRateFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateNextHighestPeakBitRateFilterCreate called");
    return NULL;
}

void* FigAlternateNormalizedPeakBitRateCapFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateNormalizedPeakBitRateCapFilterCreate called");
    return NULL;
}

void* FigAlternateOfflinePlayableForMediaSelectionFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateOfflinePlayableForMediaSelectionFilterCreate called");
    return NULL;
}

void* FigAlternatePreferBestFormatForVideoRangeFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternatePreferBestFormatForVideoRangeFilterCreate called");
    return NULL;
}

void* FigAlternatePreferredAudioFormatFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternatePreferredAudioFormatFilterCreate called");
    return NULL;
}

void* FigAlternatePreferredVideoFormatFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternatePreferredVideoFormatFilterCreate called");
    return NULL;
}

void* FigAlternateResolutionCapFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateResolutionCapFilterCreate called");
    return NULL;
}

void* FigAlternateResolutionLowerLimitFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateResolutionLowerLimitFilterCreate called");
    return NULL;
}

void* FigAlternateResolutionUpperLimitFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateResolutionUpperLimitFilterCreate called");
    return NULL;
}

void* FigAlternateSetErrorCategory(void)
{
    if (verbose) puts("STUB: FigAlternateSetErrorCategory called");
    return NULL;
}

void* FigAlternateSetIsFairPlayStreamingVideoAlternate(void)
{
    if (verbose) puts("STUB: FigAlternateSetIsFairPlayStreamingVideoAlternate called");
    return NULL;
}

void* FigAlternateSetLastKnownValueForIsFullyCached(void)
{
    if (verbose) puts("STUB: FigAlternateSetLastKnownValueForIsFullyCached called");
    return NULL;
}

void* FigAlternateSetLateDiscoveredMediaType(void)
{
    if (verbose) puts("STUB: FigAlternateSetLateDiscoveredMediaType called");
    return NULL;
}

void* FigAlternateSetStartupScore(void)
{
    if (verbose) puts("STUB: FigAlternateSetStartupScore called");
    return NULL;
}

void* FigAlternateShouldBeAvoidedForPlaybackInLPM(void)
{
    if (verbose) puts("STUB: FigAlternateShouldBeAvoidedForPlaybackInLPM called");
    return NULL;
}

void* FigAlternateSpatialAudioFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateSpatialAudioFilterCreate called");
    return NULL;
}

void* FigAlternateStreamVideoQualityIndexToCMVideoDynamicRange(void)
{
    if (verbose) puts("STUB: FigAlternateStreamVideoQualityIndexToCMVideoDynamicRange called");
    return NULL;
}

void* FigAlternateSupportedAudioFormatFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateSupportedAudioFormatFilterCreate called");
    return NULL;
}

void* FigAlternateSupportedVideoRangeFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateSupportedVideoRangeFilterCreate called");
    return NULL;
}

void* FigAlternateSuppressAudioOnlyFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateSuppressAudioOnlyFilterCreate called");
    return NULL;
}

void* FigAlternateVideoFormatFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateVideoFormatFilterCreate called");
    return NULL;
}

void* FigAlternateVideoRangeFilterCreate(void)
{
    if (verbose) puts("STUB: FigAlternateVideoRangeFilterCreate called");
    return NULL;
}

void* FigAlternateWillPlayIfFairplayProtected(void)
{
    if (verbose) puts("STUB: FigAlternateWillPlayIfFairplayProtected called");
    return NULL;
}

void* FigAssertionCopyProperty(void)
{
    if (verbose) puts("STUB: FigAssertionCopyProperty called");
    return NULL;
}

void* FigAssertionSetProperty(void)
{
    if (verbose) puts("STUB: FigAssertionSetProperty called");
    return NULL;
}

void* FigAssetCacheInspectorCreate(void)
{
    if (verbose) puts("STUB: FigAssetCacheInspectorCreate called");
    return NULL;
}

void* FigAssetCacheInspectorGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetCacheInspectorGetCMBaseObject called");
    return NULL;
}

void* FigAssetCacheInspectorGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetCacheInspectorGetClassID called");
    return NULL;
}

void* FigAssetCacheInspectorGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetCacheInspectorGetTypeID called");
    return NULL;
}

void* FigAssetCollectionFactoryCreateCollectionWithURL(void)
{
    if (verbose) puts("STUB: FigAssetCollectionFactoryCreateCollectionWithURL called");
    return NULL;
}

void* FigAssetCollectionGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetCollectionGetCMBaseObject called");
    return NULL;
}

void* FigAssetCollectionGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetCollectionGetClassID called");
    return NULL;
}

void* FigAssetCollectionGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetCollectionGetTypeID called");
    return NULL;
}

void* FigAssetCopyAssetWithDownloadToken(void)
{
    if (verbose) puts("STUB: FigAssetCopyAssetWithDownloadToken called");
    return NULL;
}

void* FigAssetCreateWithBlockBuffer(void)
{
    if (verbose) puts("STUB: FigAssetCreateWithBlockBuffer called");
    return NULL;
}

void* FigAssetCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigAssetCreateWithByteStream called");
    return NULL;
}

void* FigAssetCreateWithFormatReader(void)
{
    if (verbose) puts("STUB: FigAssetCreateWithFormatReader called");
    return NULL;
}

void* FigAssetCreateWithMovieProxyData(void)
{
    if (verbose) puts("STUB: FigAssetCreateWithMovieProxyData called");
    return NULL;
}

void* FigAssetCreateWithURL(void)
{
    if (verbose) puts("STUB: FigAssetCreateWithURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyDownloadCompletionDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyDownloadCompletionDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyDownloadStartDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyDownloadStartDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyExpirationDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyExpirationDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyLastPlayedDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyLastPlayedDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyLastPlayedOutDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyLastPlayedOutDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementCopyPriorityForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementCopyPriorityForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetContentTypeForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetContentTypeForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetDownloadCompletionDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetDownloadCompletionDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetDownloadStartDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetDownloadStartDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetExpirationDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetExpirationDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetLastPlayedDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetLastPlayedDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetLastPlayedOutDateForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetLastPlayedOutDateForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloadStorageManagementSetPriorityForAssetAtURL(void)
{
    if (verbose) puts("STUB: FigAssetDownloadStorageManagementSetPriorityForAssetAtURL called");
    return NULL;
}

void* FigAssetDownloaderGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetDownloaderGetCMBaseObject called");
    return NULL;
}

void* FigAssetDownloaderGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetDownloaderGetClassID called");
    return NULL;
}

void* FigAssetDownloaderGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetDownloaderGetTypeID called");
    return NULL;
}

void* FigAssetDownloaderRegisterDestinationLocation(void)
{
    if (verbose) puts("STUB: FigAssetDownloaderRegisterDestinationLocation called");
    return NULL;
}

void* FigAssetExportSessionCancelExport(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionCancelExport called");
    return NULL;
}

void* FigAssetExportSessionCopyProperty(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionCopyProperty called");
    return NULL;
}

void* FigAssetExportSessionCreateWithAsset(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionCreateWithAsset called");
    return NULL;
}

void* FigAssetExportSessionDetermineCompatibilityOfExportPreset(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionDetermineCompatibilityOfExportPreset called");
    return NULL;
}

void* FigAssetExportSessionDetermineCompatibleFileTypes(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionDetermineCompatibleFileTypes called");
    return NULL;
}

void* FigAssetExportSessionEstimateMaximumDurationWithCompletionHandler(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionEstimateMaximumDurationWithCompletionHandler called");
    return NULL;
}

void* FigAssetExportSessionEstimateOutputFileLengthWithCompletionHandler(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionEstimateOutputFileLengthWithCompletionHandler called");
    return NULL;
}

void* FigAssetExportSessionExportAsynchronously(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionExportAsynchronously called");
    return NULL;
}

void* FigAssetExportSessionExportGetPresetsCompatibleWithFigAsset(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionExportGetPresetsCompatibleWithFigAsset called");
    return NULL;
}

void* FigAssetExportSessionGetAllPresets(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetAllPresets called");
    return NULL;
}

void* FigAssetExportSessionGetAverageVideoBitRateForSourceAndPreset(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetAverageVideoBitRateForSourceAndPreset called");
    return NULL;
}

void* FigAssetExportSessionGetDestinationFrameDurationAndExpectedFrameRate(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetDestinationFrameDurationAndExpectedFrameRate called");
    return NULL;
}

void* FigAssetExportSessionGetEstimatedOutputFileLength(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetEstimatedOutputFileLength called");
    return NULL;
}

void* FigAssetExportSessionGetMaximumDuration(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetMaximumDuration called");
    return NULL;
}

void* FigAssetExportSessionGetSettingForFigRemaker(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetSettingForFigRemaker called");
    return NULL;
}

void* FigAssetExportSessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionGetTypeID called");
    return NULL;
}

void* FigAssetExportSessionProperty_GetAudioMix_CFDictionaryValueCallBacks(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionProperty_GetAudioMix_CFDictionaryValueCallBacks called");
    return NULL;
}

void* FigAssetExportSessionSetProperty(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionSetProperty called");
    return NULL;
}

void* FigAssetExportSessionVideoCompositionCreate(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionVideoCompositionCreate called");
    return NULL;
}

void* FigAssetExportSessionVideoCompositionGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetExportSessionVideoCompositionGetTypeID called");
    return NULL;
}

void* FigAssetExportSettingsUseHEVCHardwareEncoder(void)
{
    if (verbose) puts("STUB: FigAssetExportSettingsUseHEVCHardwareEncoder called");
    return NULL;
}

void* FigAssetGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetGetCMBaseObject called");
    return NULL;
}

void* FigAssetGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetGetClassID called");
    return NULL;
}

void* FigAssetGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetGetTypeID called");
    return NULL;
}

void* FigAssetImageGeneratorCreateFromAsset(void)
{
    if (verbose) puts("STUB: FigAssetImageGeneratorCreateFromAsset called");
    return NULL;
}

void* FigAssetImageGeneratorGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetImageGeneratorGetClassID called");
    return NULL;
}

void* FigAssetImageGeneratorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetImageGeneratorGetFigBaseObject called");
    return NULL;
}

void* FigAssetImageGeneratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetImageGeneratorGetTypeID called");
    return NULL;
}

void* FigAssetReaderCreateWithAsset(void)
{
    if (verbose) puts("STUB: FigAssetReaderCreateWithAsset called");
    return NULL;
}

void* FigAssetReaderCreateWithURLAndFormatReader(void)
{
    if (verbose) puts("STUB: FigAssetReaderCreateWithURLAndFormatReader called");
    return NULL;
}

void* FigAssetReaderGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetReaderGetClassID called");
    return NULL;
}

void* FigAssetReaderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetReaderGetFigBaseObject called");
    return NULL;
}

void* FigAssetReaderGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetReaderGetTypeID called");
    return NULL;
}

void* FigAssetTableAddAsset(void)
{
    if (verbose) puts("STUB: FigAssetTableAddAsset called");
    return NULL;
}

void* FigAssetTableCopyAsset(void)
{
    if (verbose) puts("STUB: FigAssetTableCopyAsset called");
    return NULL;
}

void* FigAssetTableRemoveAsset(void)
{
    if (verbose) puts("STUB: FigAssetTableRemoveAsset called");
    return NULL;
}

void* FigAssetTrackGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetTrackGetCMBaseObject called");
    return NULL;
}

void* FigAssetTrackGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetTrackGetClassID called");
    return NULL;
}

void* FigAssetTrackGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetTrackGetTypeID called");
    return NULL;
}

void* FigAssetWriterCreateForWritingFragmentedData(void)
{
    if (verbose) puts("STUB: FigAssetWriterCreateForWritingFragmentedData called");
    return NULL;
}

void* FigAssetWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigAssetWriterCreateWithByteStream called");
    return NULL;
}

void* FigAssetWriterCreateWithURL(void)
{
    if (verbose) puts("STUB: FigAssetWriterCreateWithURL called");
    return NULL;
}

void* FigAssetWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigAssetWriterGetClassID called");
    return NULL;
}

void* FigAssetWriterGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigAssetWriterGetFigBaseObject called");
    return NULL;
}

void* FigAssetWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigAssetWriterGetTypeID called");
    return NULL;
}

void* FigAttributePartitionGeneratorAddElement(void)
{
    if (verbose) puts("STUB: FigAttributePartitionGeneratorAddElement called");
    return NULL;
}

void* FigAttributePartitionGeneratorCopyPartitionReferences(void)
{
    if (verbose) puts("STUB: FigAttributePartitionGeneratorCopyPartitionReferences called");
    return NULL;
}

void* FigAttributePartitionGeneratorCreate(void)
{
    if (verbose) puts("STUB: FigAttributePartitionGeneratorCreate called");
    return NULL;
}

void* FigAttributePartitionGeneratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigAttributePartitionGeneratorGetTypeID called");
    return NULL;
}

void* FigAttributePartitionGeneratorPerformPartitioning(void)
{
    if (verbose) puts("STUB: FigAttributePartitionGeneratorPerformPartitioning called");
    return NULL;
}

void* FigAudioCompressionOptionsGetPreset(void)
{
    if (verbose) puts("STUB: FigAudioCompressionOptionsGetPreset called");
    return NULL;
}

void* FigAudioFileFormatWriterCanFileTypeSupportFormatDescription(void)
{
    if (verbose) puts("STUB: FigAudioFileFormatWriterCanFileTypeSupportFormatDescription called");
    return NULL;
}

void* FigAudioFileFormatWriterCanFileTypeSupportMediaType(void)
{
    if (verbose) puts("STUB: FigAudioFileFormatWriterCanFileTypeSupportMediaType called");
    return NULL;
}

void* FigAudioFileFormatWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigAudioFileFormatWriterCreateWithByteStream called");
    return NULL;
}

void* FigAudioQueueRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigAudioQueueRenderPipelineCreate called");
    return NULL;
}

void* FigAudioUnitRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigAudioUnitRenderPipelineCreate called");
    return NULL;
}

void* FigAutomaticMediaSelectionCreateSelectedMediaArrayForCriteria(void)
{
    if (verbose) puts("STUB: FigAutomaticMediaSelectionCreateSelectedMediaArrayForCriteria called");
    return NULL;
}

void* FigBufferedAirPlayAudioRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigBufferedAirPlayAudioRenderPipelineCreate called");
    return NULL;
}

void* FigBufferedAirPlayOutputCreate(void)
{
    if (verbose) puts("STUB: FigBufferedAirPlayOutputCreate called");
    return NULL;
}

void* FigBufferedAirPlayOutputGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigBufferedAirPlayOutputGetCMBaseObject called");
    return NULL;
}

void* FigBufferedAirPlayOutputGetClassID(void)
{
    if (verbose) puts("STUB: FigBufferedAirPlayOutputGetClassID called");
    return NULL;
}

void* FigBufferedAirPlayOutputGetTypeID(void)
{
    if (verbose) puts("STUB: FigBufferedAirPlayOutputGetTypeID called");
    return NULL;
}

void* FigByteFlumeCreateWithHTTP(void)
{
    if (verbose) puts("STUB: FigByteFlumeCreateWithHTTP called");
    return NULL;
}

void* FigByteFlumeCustomURLCreateWithURL(void)
{
    if (verbose) puts("STUB: FigByteFlumeCustomURLCreateWithURL called");
    return NULL;
}

void* FigByteFlumeGetClassID(void)
{
    if (verbose) puts("STUB: FigByteFlumeGetClassID called");
    return NULL;
}

void* FigByteFlumeGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigByteFlumeGetFigBaseObject called");
    return NULL;
}

void* FigByteFlumeGetTypeID(void)
{
    if (verbose) puts("STUB: FigByteFlumeGetTypeID called");
    return NULL;
}

void* FigBytePumpCreateForHTTPChunk(void)
{
    if (verbose) puts("STUB: FigBytePumpCreateForHTTPChunk called");
    return NULL;
}

void* FigBytePumpCreateForICY(void)
{
    if (verbose) puts("STUB: FigBytePumpCreateForICY called");
    return NULL;
}

void* FigBytePumpGetClassID(void)
{
    if (verbose) puts("STUB: FigBytePumpGetClassID called");
    return NULL;
}

void* FigBytePumpGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigBytePumpGetFigBaseObject called");
    return NULL;
}

void* FigBytePumpGetTypeID(void)
{
    if (verbose) puts("STUB: FigBytePumpGetTypeID called");
    return NULL;
}

void* FigBytePumpRelease(void)
{
    if (verbose) puts("STUB: FigBytePumpRelease called");
    return NULL;
}

void* FigBytePumpRetain(void)
{
    if (verbose) puts("STUB: FigBytePumpRetain called");
    return NULL;
}

void* FigByteStreamFactoryCreateStreamFromURL(void)
{
    if (verbose) puts("STUB: FigByteStreamFactoryCreateStreamFromURL called");
    return NULL;
}

void* FigByteStreamFactoryCreateStreamFromURLAsync(void)
{
    if (verbose) puts("STUB: FigByteStreamFactoryCreateStreamFromURLAsync called");
    return NULL;
}

void* FigByteStreamFactoryCreateStreamFromURLWithOptions(void)
{
    if (verbose) puts("STUB: FigByteStreamFactoryCreateStreamFromURLWithOptions called");
    return NULL;
}

void* FigByteStreamFactoryCreateStreamFromURLWithOptionsAsync(void)
{
    if (verbose) puts("STUB: FigByteStreamFactoryCreateStreamFromURLWithOptionsAsync called");
    return NULL;
}

void* FigByteStreamFactorySetCachedFileByteStreamEnable(void)
{
    if (verbose) puts("STUB: FigByteStreamFactorySetCachedFileByteStreamEnable called");
    return NULL;
}

void* FigByteStreamFactoryTranslateReferenceSecurityOptions(void)
{
    if (verbose) puts("STUB: FigByteStreamFactoryTranslateReferenceSecurityOptions called");
    return NULL;
}

void* FigCDSLayoutContextCreate(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextCreate called");
    return NULL;
}

void* FigCDSLayoutContextGetMapping(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextGetMapping called");
    return NULL;
}

void* FigCDSLayoutContextGetSubtitleSample(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextGetSubtitleSample called");
    return NULL;
}

void* FigCDSLayoutContextGetTypeID(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextGetTypeID called");
    return NULL;
}

void* FigCDSLayoutContextSetMapping(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextSetMapping called");
    return NULL;
}

void* FigCDSLayoutContextSetSubtitleSample(void)
{
    if (verbose) puts("STUB: FigCDSLayoutContextSetSubtitleSample called");
    return NULL;
}

void* FigCDSSessionClear(void)
{
    if (verbose) puts("STUB: FigCDSSessionClear called");
    return NULL;
}

void* FigCDSSessionCreate(void)
{
    if (verbose) puts("STUB: FigCDSSessionCreate called");
    return NULL;
}

void* FigCDSSessionGetLayoutContext_Overscan(void)
{
    if (verbose) puts("STUB: FigCDSSessionGetLayoutContext_Overscan called");
    return NULL;
}

void* FigCDSSessionGetRate(void)
{
    if (verbose) puts("STUB: FigCDSSessionGetRate called");
    return NULL;
}

void* FigCDSSessionGetTime(void)
{
    if (verbose) puts("STUB: FigCDSSessionGetTime called");
    return NULL;
}

void* FigCDSSessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigCDSSessionGetTypeID called");
    return NULL;
}

void* FigCDSSessionManualTrigger(void)
{
    if (verbose) puts("STUB: FigCDSSessionManualTrigger called");
    return NULL;
}

void* FigCDSSessionSetBitmapCacheRange(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetBitmapCacheRange called");
    return NULL;
}

void* FigCDSSessionSetBounds(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetBounds called");
    return NULL;
}

void* FigCDSSessionSetCACFLayerDevice(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCACFLayerDevice called");
    return NULL;
}

void* FigCDSSessionSetCALayerDevice(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCALayerDevice called");
    return NULL;
}

void* FigCDSSessionSetCGContextDevice(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCGContextDevice called");
    return NULL;
}

void* FigCDSSessionSetCacheBudget(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCacheBudget called");
    return NULL;
}

void* FigCDSSessionSetCaptionArray(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCaptionArray called");
    return NULL;
}

void* FigCDSSessionSetCaptionCacheRange(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCaptionCacheRange called");
    return NULL;
}

void* FigCDSSessionSetCaptionSample(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetCaptionSample called");
    return NULL;
}

void* FigCDSSessionSetDefaultTextStyle(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetDefaultTextStyle called");
    return NULL;
}

void* FigCDSSessionSetLayoutContext_Gravity(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetLayoutContext_Gravity called");
    return NULL;
}

void* FigCDSSessionSetLayoutContext_Overscan(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetLayoutContext_Overscan called");
    return NULL;
}

void* FigCDSSessionSetLayoutContext_VideoSize(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetLayoutContext_VideoSize called");
    return NULL;
}

void* FigCDSSessionSetLayoutContext_Viewport(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetLayoutContext_Viewport called");
    return NULL;
}

void* FigCDSSessionSetLegibleOutputDevice(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetLegibleOutputDevice called");
    return NULL;
}

void* FigCDSSessionSetMasterClock(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetMasterClock called");
    return NULL;
}

void* FigCDSSessionSetNonForcedSubtitleDisplay(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetNonForcedSubtitleDisplay called");
    return NULL;
}

void* FigCDSSessionSetPlayer(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetPlayer called");
    return NULL;
}

void* FigCDSSessionSetProfile(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetProfile called");
    return NULL;
}

void* FigCDSSessionSetRate(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetRate called");
    return NULL;
}

void* FigCDSSessionSetTime(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetTime called");
    return NULL;
}

void* FigCDSSessionSetTimeRange(void)
{
    if (verbose) puts("STUB: FigCDSSessionSetTimeRange called");
    return NULL;
}

void* FigCDSSessionStart(void)
{
    if (verbose) puts("STUB: FigCDSSessionStart called");
    return NULL;
}

void* FigCDSSessionStop(void)
{
    if (verbose) puts("STUB: FigCDSSessionStop called");
    return NULL;
}

void* FigCDSSessionSynchronizeDefaults(void)
{
    if (verbose) puts("STUB: FigCDSSessionSynchronizeDefaults called");
    return NULL;
}

void* FigCDSSessionUpdateCGContext(void)
{
    if (verbose) puts("STUB: FigCDSSessionUpdateCGContext called");
    return NULL;
}

void* FigCDSSessionUpdateLegibleOutputContext(void)
{
    if (verbose) puts("STUB: FigCDSSessionUpdateLegibleOutputContext called");
    return NULL;
}

void* FigCDSSessionUpdateOutput(void)
{
    if (verbose) puts("STUB: FigCDSSessionUpdateOutput called");
    return NULL;
}

void* FigCFHTTPCopyClientCertChain(void)
{
    if (verbose) puts("STUB: FigCFHTTPCopyClientCertChain called");
    return NULL;
}

void* FigCFHTTPCreateURLMinusQueryParam(void)
{
    if (verbose) puts("STUB: FigCFHTTPCreateURLMinusQueryParam called");
    return NULL;
}

void* FigCFHTTPCreateURLWithQueryParam(void)
{
    if (verbose) puts("STUB: FigCFHTTPCreateURLWithQueryParam called");
    return NULL;
}

void* FigCLCPCaptionConversionValidatorCreate(void)
{
    if (verbose) puts("STUB: FigCLCPCaptionConversionValidatorCreate called");
    return NULL;
}

void* FigCLCPCaptionFormatConformerCreate(void)
{
    if (verbose) puts("STUB: FigCLCPCaptionFormatConformerCreate called");
    return NULL;
}

void* FigCLCPCaptionGroupConverterFromSampleBufferCreate(void)
{
    if (verbose) puts("STUB: FigCLCPCaptionGroupConverterFromSampleBufferCreate called");
    return NULL;
}

void* FigCPECopySessionRecordStorageURLForAppIdentifier(void)
{
    if (verbose) puts("STUB: FigCPECopySessionRecordStorageURLForAppIdentifier called");
    return NULL;
}

void* FigCPECopySessionRecordsForAppIdentifier(void)
{
    if (verbose) puts("STUB: FigCPECopySessionRecordsForAppIdentifier called");
    return NULL;
}

void* FigCPECreateFormatDescriptionIdentifierForTrackIDAndIndex(void)
{
    if (verbose) puts("STUB: FigCPECreateFormatDescriptionIdentifierForTrackIDAndIndex called");
    return NULL;
}

void* FigCPEInitializeWithOptions(void)
{
    if (verbose) puts("STUB: FigCPEInitializeWithOptions called");
    return NULL;
}

void* FigCPEIsSupportedFormatDescription(void)
{
    if (verbose) puts("STUB: FigCPEIsSupportedFormatDescription called");
    return NULL;
}

void* FigCPEProtectorCreateForScheme(void)
{
    if (verbose) puts("STUB: FigCPEProtectorCreateForScheme called");
    return NULL;
}

void* FigCPEProtectorGetClassID(void)
{
    if (verbose) puts("STUB: FigCPEProtectorGetClassID called");
    return NULL;
}

void* FigCPEProtectorGetTypeID(void)
{
    if (verbose) puts("STUB: FigCPEProtectorGetTypeID called");
    return NULL;
}

void* FigCPERemoveSessionRecordsForAppIdentifier(void)
{
    if (verbose) puts("STUB: FigCPERemoveSessionRecordsForAppIdentifier called");
    return NULL;
}

void* FigCPESetSessionRecordStorageURLForAppIdentifier(void)
{
    if (verbose) puts("STUB: FigCPESetSessionRecordStorageURLForAppIdentifier called");
    return NULL;
}

void* FigCPEUninitialize(void)
{
    if (verbose) puts("STUB: FigCPEUninitialize called");
    return NULL;
}

void* FigCRABSCreate(void)
{
    if (verbose) puts("STUB: FigCRABSCreate called");
    return NULL;
}

void* FigCRABSGetByteStream(void)
{
    if (verbose) puts("STUB: FigCRABSGetByteStream called");
    return NULL;
}

void* FigCRABSGetClassID(void)
{
    if (verbose) puts("STUB: FigCRABSGetClassID called");
    return NULL;
}

void* FigCRABSScheduledIOCreate(void)
{
    if (verbose) puts("STUB: FigCRABSScheduledIOCreate called");
    return NULL;
}

void* FigCaptionCommandCreateFromPropertyList(void)
{
    if (verbose) puts("STUB: FigCaptionCommandCreateFromPropertyList called");
    return NULL;
}

void* FigCaptionCommandCreatePropertyList(void)
{
    if (verbose) puts("STUB: FigCaptionCommandCreatePropertyList called");
    return NULL;
}

void* FigCaptionConversionValidatorGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptionConversionValidatorGetCMBaseObject called");
    return NULL;
}

void* FigCaptionConversionValidatorGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptionConversionValidatorGetClassID called");
    return NULL;
}

void* FigCaptionConversionValidatorGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionConversionValidatorGetTypeID called");
    return NULL;
}

void* FigCaptionCreate(void)
{
    if (verbose) puts("STUB: FigCaptionCreate called");
    return NULL;
}

void* FigCaptionCreateCopy(void)
{
    if (verbose) puts("STUB: FigCaptionCreateCopy called");
    return NULL;
}

void* FigCaptionCreateCopyWithNewTiming(void)
{
    if (verbose) puts("STUB: FigCaptionCreateCopyWithNewTiming called");
    return NULL;
}

void* FigCaptionCreateMutable(void)
{
    if (verbose) puts("STUB: FigCaptionCreateMutable called");
    return NULL;
}

void* FigCaptionCreateMutableCopy(void)
{
    if (verbose) puts("STUB: FigCaptionCreateMutableCopy called");
    return NULL;
}

void* FigCaptionFormatConformerGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptionFormatConformerGetCMBaseObject called");
    return NULL;
}

void* FigCaptionFormatConformerGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptionFormatConformerGetClassID called");
    return NULL;
}

void* FigCaptionFormatConformerGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionFormatConformerGetTypeID called");
    return NULL;
}

void* FigCaptionGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptionGetCMBaseObject called");
    return NULL;
}

void* FigCaptionGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptionGetClassID called");
    return NULL;
}

void* FigCaptionGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionGetTypeID called");
    return NULL;
}

void* FigCaptionGroupConverterFromSampleBufferGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptionGroupConverterFromSampleBufferGetClassID called");
    return NULL;
}

void* FigCaptionGroupConverterFromSampleBufferGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionGroupConverterFromSampleBufferGetTypeID called");
    return NULL;
}

void* FigCaptionSerializerCopyProperty(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerCopyProperty called");
    return NULL;
}

void* FigCaptionSerializerCreate(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerCreate called");
    return NULL;
}

void* FigCaptionSerializerCreateCFDataFromCaptionGroup(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerCreateCFDataFromCaptionGroup called");
    return NULL;
}

void* FigCaptionSerializerCreateCaptionGroupFromCFData(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerCreateCaptionGroupFromCFData called");
    return NULL;
}

void* FigCaptionSerializerGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerGetTypeID called");
    return NULL;
}

void* FigCaptionSerializerSetProperty(void)
{
    if (verbose) puts("STUB: FigCaptionSerializerSetProperty called");
    return NULL;
}

void* FigCaptionStyleSegmentCopyAttributes(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentCopyAttributes called");
    return NULL;
}

void* FigCaptionStyleSegmentCopyCaptionData(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentCopyCaptionData called");
    return NULL;
}

void* FigCaptionStyleSegmentCopyText(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentCopyText called");
    return NULL;
}

void* FigCaptionStyleSegmentGeneratorCreate(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGeneratorCreate called");
    return NULL;
}

void* FigCaptionStyleSegmentGeneratorGetSegmentAtIndex(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGeneratorGetSegmentAtIndex called");
    return NULL;
}

void* FigCaptionStyleSegmentGeneratorGetSegmentCount(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGeneratorGetSegmentCount called");
    return NULL;
}

void* FigCaptionStyleSegmentGeneratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGeneratorGetTypeID called");
    return NULL;
}

void* FigCaptionStyleSegmentGeneratorPerform(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGeneratorPerform called");
    return NULL;
}

void* FigCaptionStyleSegmentGetTextLength(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGetTextLength called");
    return NULL;
}

void* FigCaptionStyleSegmentGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionStyleSegmentGetTypeID called");
    return NULL;
}

void* FigCaptionTimelineGeneratorAddSampleBuffer(void)
{
    if (verbose) puts("STUB: FigCaptionTimelineGeneratorAddSampleBuffer called");
    return NULL;
}

void* FigCaptionTimelineGeneratorCreate(void)
{
    if (verbose) puts("STUB: FigCaptionTimelineGeneratorCreate called");
    return NULL;
}

void* FigCaptionTimelineGeneratorFinish(void)
{
    if (verbose) puts("STUB: FigCaptionTimelineGeneratorFinish called");
    return NULL;
}

void* FigCaptionTimelineGeneratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptionTimelineGeneratorGetTypeID called");
    return NULL;
}

void* FigCaptureDeviceGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptureDeviceGetClassID called");
    return NULL;
}

void* FigCaptureDeviceGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptureDeviceGetFigBaseObject called");
    return NULL;
}

void* FigCaptureDeviceGetNotificationCenter(void)
{
    if (verbose) puts("STUB: FigCaptureDeviceGetNotificationCenter called");
    return NULL;
}

void* FigCaptureDeviceGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptureDeviceGetTypeID called");
    return NULL;
}

void* FigCaptureStreamGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptureStreamGetClassID called");
    return NULL;
}

void* FigCaptureStreamGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptureStreamGetFigBaseObject called");
    return NULL;
}

void* FigCaptureStreamGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptureStreamGetTypeID called");
    return NULL;
}

void* FigCaptureSynchronizedStreamsGroupGetClassID(void)
{
    if (verbose) puts("STUB: FigCaptureSynchronizedStreamsGroupGetClassID called");
    return NULL;
}

void* FigCaptureSynchronizedStreamsGroupGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigCaptureSynchronizedStreamsGroupGetFigBaseObject called");
    return NULL;
}

void* FigCaptureSynchronizedStreamsGroupGetTypeID(void)
{
    if (verbose) puts("STUB: FigCaptureSynchronizedStreamsGroupGetTypeID called");
    return NULL;
}

void* FigClosedCaptionsTextRasterizerCreate(void)
{
    if (verbose) puts("STUB: FigClosedCaptionsTextRasterizerCreate called");
    return NULL;
}

void* FigClosedCaptionsTextRasterizerSetCommand(void)
{
    if (verbose) puts("STUB: FigClosedCaptionsTextRasterizerSetCommand called");
    return NULL;
}

void* FigComputeMovieDimensions(void)
{
    if (verbose) puts("STUB: FigComputeMovieDimensions called");
    return NULL;
}

void* FigConsolidateMovieFragments(void)
{
    if (verbose) puts("STUB: FigConsolidateMovieFragments called");
    return NULL;
}

void* FigContentKeySessionCreateWithCallbacks(void)
{
    if (verbose) puts("STUB: FigContentKeySessionCreateWithCallbacks called");
    return NULL;
}

void* FigCopyCGColorSRGBAsArray(void)
{
    if (verbose) puts("STUB: FigCopyCGColorSRGBAsArray called");
    return NULL;
}

void* FigCopyMPEGMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopyMPEGMIMETypes called");
    return NULL;
}

void* FigCopyQTMovieMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopyQTMovieMIMETypes called");
    return NULL;
}

void* FigCopySetOfAudioSupportedMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopySetOfAudioSupportedMIMETypes called");
    return NULL;
}

void* FigCopySetOfFormatReaderSupportedFileExtensions(void)
{
    if (verbose) puts("STUB: FigCopySetOfFormatReaderSupportedFileExtensions called");
    return NULL;
}

void* FigCopySetOfFormatReaderSupportedFileTypes(void)
{
    if (verbose) puts("STUB: FigCopySetOfFormatReaderSupportedFileTypes called");
    return NULL;
}

void* FigCopySetOfFormatReaderSupportedMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopySetOfFormatReaderSupportedMIMETypes called");
    return NULL;
}

void* FigCopySetOfFormatReaderSupportedUTIs(void)
{
    if (verbose) puts("STUB: FigCopySetOfFormatReaderSupportedUTIs called");
    return NULL;
}

void* FigCopySetOfStreamingSupportedMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopySetOfStreamingSupportedMIMETypes called");
    return NULL;
}

void* FigCopySetOfStreamingSupportedUTIs(void)
{
    if (verbose) puts("STUB: FigCopySetOfStreamingSupportedUTIs called");
    return NULL;
}

void* FigCopyWebVTTMIMETypes(void)
{
    if (verbose) puts("STUB: FigCopyWebVTTMIMETypes called");
    return NULL;
}

void* FigCoreAnimationRendererCopyPixelBufferAtTime(void)
{
    if (verbose) puts("STUB: FigCoreAnimationRendererCopyPixelBufferAtTime called");
    return NULL;
}

void* FigCoreAnimationRendererCopyProperty(void)
{
    if (verbose) puts("STUB: FigCoreAnimationRendererCopyProperty called");
    return NULL;
}

void* FigCoreAnimationRendererCreate(void)
{
    if (verbose) puts("STUB: FigCoreAnimationRendererCreate called");
    return NULL;
}

void* FigCoreAnimationRendererSetProperty(void)
{
    if (verbose) puts("STUB: FigCoreAnimationRendererSetProperty called");
    return NULL;
}

void* FigCoreTextSubtitleRendererCreate(void)
{
    if (verbose) puts("STUB: FigCoreTextSubtitleRendererCreate called");
    return NULL;
}

void* FigCreate3x2MatrixArrayFromCGAffineTransform(void)
{
    if (verbose) puts("STUB: FigCreate3x2MatrixArrayFromCGAffineTransform called");
    return NULL;
}

void* FigCreate3x3MatrixArrayFromCGAffineTransform(void)
{
    if (verbose) puts("STUB: FigCreate3x3MatrixArrayFromCGAffineTransform called");
    return NULL;
}

void* FigCreateCGColorSRGB(void)
{
    if (verbose) puts("STUB: FigCreateCGColorSRGB called");
    return NULL;
}

void* FigCreateCGColorSRGBFromArray(void)
{
    if (verbose) puts("STUB: FigCreateCGColorSRGBFromArray called");
    return NULL;
}

void* FigCreateCacheMemoryAllocatorForCRABS(void)
{
    if (verbose) puts("STUB: FigCreateCacheMemoryAllocatorForCRABS called");
    return NULL;
}

void* FigCreateClosedCaptionRenderPipeline(void)
{
    if (verbose) puts("STUB: FigCreateClosedCaptionRenderPipeline called");
    return NULL;
}

void* FigCreateHLSfMP4SecurityInfoData(void)
{
    if (verbose) puts("STUB: FigCreateHLSfMP4SecurityInfoData called");
    return NULL;
}

void* FigCreateHLSfMP4SecurityInfoPlaceholderData(void)
{
    if (verbose) puts("STUB: FigCreateHLSfMP4SecurityInfoPlaceholderData called");
    return NULL;
}

void* FigCreateHapticPatternDictionaryFromAppleHapticBBuf(void)
{
    if (verbose) puts("STUB: FigCreateHapticPatternDictionaryFromAppleHapticBBuf called");
    return NULL;
}

void* FigCreateNullPixelBuffer(void)
{
    if (verbose) puts("STUB: FigCreateNullPixelBuffer called");
    return NULL;
}

void* FigCreateSubtitleRenderPipeline(void)
{
    if (verbose) puts("STUB: FigCreateSubtitleRenderPipeline called");
    return NULL;
}

void* FigDiskCacheProviderGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigDiskCacheProviderGetCFTypeID called");
    return NULL;
}

void* FigDiskCacheProviderGetClassID(void)
{
    if (verbose) puts("STUB: FigDiskCacheProviderGetClassID called");
    return NULL;
}

void* FigDiskCacheProviderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigDiskCacheProviderGetFigBaseObject called");
    return NULL;
}

void* FigDiskCacheProviderRelease(void)
{
    if (verbose) puts("STUB: FigDiskCacheProviderRelease called");
    return NULL;
}

void* FigDiskCacheProviderRetain(void)
{
    if (verbose) puts("STUB: FigDiskCacheProviderRetain called");
    return NULL;
}

void* FigDiskCacheRepositoryCheckIn(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCheckIn called");
    return NULL;
}

void* FigDiskCacheRepositoryCheckInAndOut(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCheckInAndOut called");
    return NULL;
}

void* FigDiskCacheRepositoryCheckOut(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCheckOut called");
    return NULL;
}

void* FigDiskCacheRepositoryCopyCheckedInIDs(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCopyCheckedInIDs called");
    return NULL;
}

void* FigDiskCacheRepositoryCopyDefaultParentURLForCurrentTask(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCopyDefaultParentURLForCurrentTask called");
    return NULL;
}

void* FigDiskCacheRepositoryCopyDefaultTmpDirURL(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCopyDefaultTmpDirURL called");
    return NULL;
}

void* FigDiskCacheRepositoryCopyInfo(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCopyInfo called");
    return NULL;
}

void* FigDiskCacheRepositoryCopyProperty(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCopyProperty called");
    return NULL;
}

void* FigDiskCacheRepositoryCreate(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryCreate called");
    return NULL;
}

void* FigDiskCacheRepositoryDelete(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryDelete called");
    return NULL;
}

void* FigDiskCacheRepositoryExists(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryExists called");
    return NULL;
}

void* FigDiskCacheRepositoryGetFileSize(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryGetFileSize called");
    return NULL;
}

void* FigDiskCacheRepositoryGetTypeID(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryGetTypeID called");
    return NULL;
}

void* FigDiskCacheRepositoryMakeRoom(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryMakeRoom called");
    return NULL;
}

void* FigDiskCacheRepositoryReserveBacking(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositoryReserveBacking called");
    return NULL;
}

void* FigDiskCacheRepositorySetTarget(void)
{
    if (verbose) puts("STUB: FigDiskCacheRepositorySetTarget called");
    return NULL;
}

void* FigDisplayModes_BuildColorAndTimingModes(void)
{
    if (verbose) puts("STUB: FigDisplayModes_BuildColorAndTimingModes called");
    return NULL;
}

void* FigDisplayModes_ConvertNonSquarePixelSizeToSquarePixelSize(void)
{
    if (verbose) puts("STUB: FigDisplayModes_ConvertNonSquarePixelSizeToSquarePixelSize called");
    return NULL;
}

void* FigDisplayModes_CreateColorAndTimingModesArraysWithPreferredMode(void)
{
    if (verbose) puts("STUB: FigDisplayModes_CreateColorAndTimingModesArraysWithPreferredMode called");
    return NULL;
}

void* FigDisplayModes_DetermineSenderSurfaceDimensionsFromDisplayModeAndCurrentSenderDisplaySize(void)
{
    if (verbose) puts("STUB: FigDisplayModes_DetermineSenderSurfaceDimensionsFromDisplayModeAndCurrentSenderDisplaySize called");
    return NULL;
}

void* FigDisplayModes_ExtractDimensionsFromTimingMode(void)
{
    if (verbose) puts("STUB: FigDisplayModes_ExtractDimensionsFromTimingMode called");
    return NULL;
}

void* FigDisplayModes_ExtractOverscanInfoFromTimingMode(void)
{
    if (verbose) puts("STUB: FigDisplayModes_ExtractOverscanInfoFromTimingMode called");
    return NULL;
}

void* FigDisplayModes_LookupTimingModeIDByIndex(void)
{
    if (verbose) puts("STUB: FigDisplayModes_LookupTimingModeIDByIndex called");
    return NULL;
}

void* FigEditCursorGetClassID(void)
{
    if (verbose) puts("STUB: FigEditCursorGetClassID called");
    return NULL;
}

void* FigEditCursorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigEditCursorGetFigBaseObject called");
    return NULL;
}

void* FigEditCursorGetTypeID(void)
{
    if (verbose) puts("STUB: FigEditCursorGetTypeID called");
    return NULL;
}

void* FigEditCursorServiceGetClassID(void)
{
    if (verbose) puts("STUB: FigEditCursorServiceGetClassID called");
    return NULL;
}

void* FigEditCursorServiceGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigEditCursorServiceGetFigBaseObject called");
    return NULL;
}

void* FigEditCursorServiceGetTypeID(void)
{
    if (verbose) puts("STUB: FigEditCursorServiceGetTypeID called");
    return NULL;
}

void* FigEndpointAudioSourceBufferQueueCreate(void)
{
    if (verbose) puts("STUB: FigEndpointAudioSourceBufferQueueCreate called");
    return NULL;
}

void* FigExportSettingsAverageBitRateForSourceAndExportPreset(void)
{
    if (verbose) puts("STUB: FigExportSettingsAverageBitRateForSourceAndExportPreset called");
    return NULL;
}

void* FigExportSettingsForExportPreset(void)
{
    if (verbose) puts("STUB: FigExportSettingsForExportPreset called");
    return NULL;
}

void* FigExportSettings_CopyEffectiveColorPropertyForPresetAndInputOutputFormat(void)
{
    if (verbose) puts("STUB: FigExportSettings_CopyEffectiveColorPropertyForPresetAndInputOutputFormat called");
    return NULL;
}

void* FigExportSettings_CopyEncoderIDForProfile(void)
{
    if (verbose) puts("STUB: FigExportSettings_CopyEncoderIDForProfile called");
    return NULL;
}

void* FigExportSettings_GetMaximumBitsPerComponent(void)
{
    if (verbose) puts("STUB: FigExportSettings_GetMaximumBitsPerComponent called");
    return NULL;
}

void* FigFastStartRemakerCreateWithURLs(void)
{
    if (verbose) puts("STUB: FigFastStartRemakerCreateWithURLs called");
    return NULL;
}

void* FigFileStarByteStreamCreateForFile(void)
{
    if (verbose) puts("STUB: FigFileStarByteStreamCreateForFile called");
    return NULL;
}

void* FigFileStarByteStreamCreateForFileURL(void)
{
    if (verbose) puts("STUB: FigFileStarByteStreamCreateForFileURL called");
    return NULL;
}

void* FigFileTypeDoesSupportExternalSampleReferences(void)
{
    if (verbose) puts("STUB: FigFileTypeDoesSupportExternalSampleReferences called");
    return NULL;
}

void* FigFormatReaderCreateForAVCHD(void)
{
    if (verbose) puts("STUB: FigFormatReaderCreateForAVCHD called");
    return NULL;
}

void* FigFormatReaderCreateForStream(void)
{
    if (verbose) puts("STUB: FigFormatReaderCreateForStream called");
    return NULL;
}

void* FigFormatReaderCreateForStreamReturningMatchingContainerLabel(void)
{
    if (verbose) puts("STUB: FigFormatReaderCreateForStreamReturningMatchingContainerLabel called");
    return NULL;
}

void* FigFormatReaderCreateForStreamReturningResults(void)
{
    if (verbose) puts("STUB: FigFormatReaderCreateForStreamReturningResults called");
    return NULL;
}

void* FigFormatReaderGetClassID(void)
{
    if (verbose) puts("STUB: FigFormatReaderGetClassID called");
    return NULL;
}

void* FigFormatReaderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigFormatReaderGetFigBaseObject called");
    return NULL;
}

void* FigFormatReaderGetTypeID(void)
{
    if (verbose) puts("STUB: FigFormatReaderGetTypeID called");
    return NULL;
}

void* FigFormatWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigFormatWriterGetClassID called");
    return NULL;
}

void* FigFormatWriterGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigFormatWriterGetFigBaseObject called");
    return NULL;
}

void* FigFormatWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigFormatWriterGetTypeID called");
    return NULL;
}

void* FigFullDownloadDiskCacheProviderCreate(void)
{
    if (verbose) puts("STUB: FigFullDownloadDiskCacheProviderCreate called");
    return NULL;
}

void* FigGetCGAffineTransformFrom3x2MatrixArray(void)
{
    if (verbose) puts("STUB: FigGetCGAffineTransformFrom3x2MatrixArray called");
    return NULL;
}

void* FigGetCGAffineTransformFrom3x3MatrixArray(void)
{
    if (verbose) puts("STUB: FigGetCGAffineTransformFrom3x3MatrixArray called");
    return NULL;
}

void* FigGetIOSurfaceTimingStatsFromTimingInfoDictionary(void)
{
    if (verbose) puts("STUB: FigGetIOSurfaceTimingStatsFromTimingInfoDictionary called");
    return NULL;
}

void* FigGetNotifyingObjectForSupportedFormatsChanged(void)
{
    if (verbose) puts("STUB: FigGetNotifyingObjectForSupportedFormatsChanged called");
    return NULL;
}

void* FigGetRotationAngleAndFlipsFromCGAffineTransform(void)
{
    if (verbose) puts("STUB: FigGetRotationAngleAndFlipsFromCGAffineTransform called");
    return NULL;
}

void* FigHLSPersistentStoreCreateAtURL(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStoreCreateAtURL called");
    return NULL;
}

void* FigHLSPersistentStoreGetClassID(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStoreGetClassID called");
    return NULL;
}

void* FigHLSPersistentStoreGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStoreGetFigBaseObject called");
    return NULL;
}

void* FigHLSPersistentStoreGetTypeID(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStoreGetTypeID called");
    return NULL;
}

void* FigHLSPersistentStoreRemovePersistentStore(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStoreRemovePersistentStore called");
    return NULL;
}

void* FigHLSPersistentStreamInfoGetClassID(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStreamInfoGetClassID called");
    return NULL;
}

void* FigHLSPersistentStreamInfoGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStreamInfoGetFigBaseObject called");
    return NULL;
}

void* FigHLSPersistentStreamInfoGetTypeID(void)
{
    if (verbose) puts("STUB: FigHLSPersistentStreamInfoGetTypeID called");
    return NULL;
}

void* FigHTTPRequestCreate(void)
{
    if (verbose) puts("STUB: FigHTTPRequestCreate called");
    return NULL;
}

void* FigID3MetadataCreateChapterArrayFromTopLevelTableOfContents(void)
{
    if (verbose) puts("STUB: FigID3MetadataCreateChapterArrayFromTopLevelTableOfContents called");
    return NULL;
}

void* FigISOUserDataCreateItemDataFromText(void)
{
    if (verbose) puts("STUB: FigISOUserDataCreateItemDataFromText called");
    return NULL;
}

void* FigISOUserDataGetOSTypeKeyToStringKeyMapping(void)
{
    if (verbose) puts("STUB: FigISOUserDataGetOSTypeKeyToStringKeyMapping called");
    return NULL;
}

void* FigImageQueueCreateForCacheAndFanOut(void)
{
    if (verbose) puts("STUB: FigImageQueueCreateForCacheAndFanOut called");
    return NULL;
}

void* FigImageQueueCreateForCoreAnimation(void)
{
    if (verbose) puts("STUB: FigImageQueueCreateForCoreAnimation called");
    return NULL;
}

void* FigImageQueueCreateForCoreVideo(void)
{
    if (verbose) puts("STUB: FigImageQueueCreateForCoreVideo called");
    return NULL;
}

void* FigImageQueueCreateForDiscarding(void)
{
    if (verbose) puts("STUB: FigImageQueueCreateForDiscarding called");
    return NULL;
}

void* FigImageQueueGetClassID(void)
{
    if (verbose) puts("STUB: FigImageQueueGetClassID called");
    return NULL;
}

void* FigImageQueueGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigImageQueueGetFigBaseObject called");
    return NULL;
}

void* FigImageQueueGetTypeID(void)
{
    if (verbose) puts("STUB: FigImageQueueGetTypeID called");
    return NULL;
}

void* FigIsNullPixelBuffer(void)
{
    if (verbose) puts("STUB: FigIsNullPixelBuffer called");
    return NULL;
}

void* FigLayerSynchronizerCreate(void)
{
    if (verbose) puts("STUB: FigLayerSynchronizerCreate called");
    return NULL;
}

void* FigLayerSynchronizerDestroy(void)
{
    if (verbose) puts("STUB: FigLayerSynchronizerDestroy called");
    return NULL;
}

void* FigLayerSynchronizerSetLayerArray(void)
{
    if (verbose) puts("STUB: FigLayerSynchronizerSetLayerArray called");
    return NULL;
}

void* FigLayerSynchronizerSynchronizeToMoment(void)
{
    if (verbose) puts("STUB: FigLayerSynchronizerSynchronizeToMoment called");
    return NULL;
}

void* FigLayerViewGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigLayerViewGetCMBaseObject called");
    return NULL;
}

void* FigLayerViewGetClassID(void)
{
    if (verbose) puts("STUB: FigLayerViewGetClassID called");
    return NULL;
}

void* FigLayerViewGetTypeID(void)
{
    if (verbose) puts("STUB: FigLayerViewGetTypeID called");
    return NULL;
}

void* FigLimitedDiskCacheProviderCreate(void)
{
    if (verbose) puts("STUB: FigLimitedDiskCacheProviderCreate called");
    return NULL;
}

void* FigMPEG2ParserAddTransportStreamTrack(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserAddTransportStreamTrack called");
    return NULL;
}

void* FigMPEG2ParserCopyProgramList(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserCopyProgramList called");
    return NULL;
}

void* FigMPEG2ParserCopyTrackList(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserCopyTrackList called");
    return NULL;
}

void* FigMPEG2ParserCreate(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserCreate called");
    return NULL;
}

void* FigMPEG2ParserGetClassID(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserGetClassID called");
    return NULL;
}

void* FigMPEG2ParserGetFrameRateForVideoTrack(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserGetFrameRateForVideoTrack called");
    return NULL;
}

void* FigMPEG2ParserGetStreamType(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserGetStreamType called");
    return NULL;
}

void* FigMPEG2ParserGetTimeCodeValueForVideoTrack(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserGetTimeCodeValueForVideoTrack called");
    return NULL;
}

void* FigMPEG2ParserGetTypeID(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserGetTypeID called");
    return NULL;
}

void* FigMPEG2ParserInjectData(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserInjectData called");
    return NULL;
}

void* FigMPEG2ParserInstallCallbacksForTrack(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserInstallCallbacksForTrack called");
    return NULL;
}

void* FigMPEG2ParserIssueCommands(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserIssueCommands called");
    return NULL;
}

void* FigMPEG2ParserSetBaseTimestamp(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserSetBaseTimestamp called");
    return NULL;
}

void* FigMPEG2ParserSetDecryptor(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserSetDecryptor called");
    return NULL;
}

void* FigMPEG2ParserSniff(void)
{
    if (verbose) puts("STUB: FigMPEG2ParserSniff called");
    return NULL;
}

void* FigMPEG2SampleGeneratorCreate(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorCreate called");
    return NULL;
}

void* FigMPEG2SampleGeneratorDestroy(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorDestroy called");
    return NULL;
}

void* FigMPEG2SampleGeneratorFlush(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorFlush called");
    return NULL;
}

void* FigMPEG2SampleGeneratorGetTrackBitRate(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorGetTrackBitRate called");
    return NULL;
}

void* FigMPEG2SampleGeneratorPushFrame(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorPushFrame called");
    return NULL;
}

void* FigMPEG2SampleGeneratorResync(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorResync called");
    return NULL;
}

void* FigMPEG2SampleGeneratorSetMaxFramesPerSampleBuffer(void)
{
    if (verbose) puts("STUB: FigMPEG2SampleGeneratorSetMaxFramesPerSampleBuffer called");
    return NULL;
}

void* FigManifoldAdjustTimestampAndUpdateRefTime(void)
{
    if (verbose) puts("STUB: FigManifoldAdjustTimestampAndUpdateRefTime called");
    return NULL;
}

void* FigManifoldCreateForGap(void)
{
    if (verbose) puts("STUB: FigManifoldCreateForGap called");
    return NULL;
}

void* FigManifoldCreateForICY(void)
{
    if (verbose) puts("STUB: FigManifoldCreateForICY called");
    return NULL;
}

void* FigManifoldCreateForMPEG2(void)
{
    if (verbose) puts("STUB: FigManifoldCreateForMPEG2 called");
    return NULL;
}

void* FigManifoldCreateForMovieFragmentStream(void)
{
    if (verbose) puts("STUB: FigManifoldCreateForMovieFragmentStream called");
    return NULL;
}

void* FigManifoldCreateForWebVTT(void)
{
    if (verbose) puts("STUB: FigManifoldCreateForWebVTT called");
    return NULL;
}

void* FigManifoldGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigManifoldGetCFTypeID called");
    return NULL;
}

void* FigManifoldGetClassID(void)
{
    if (verbose) puts("STUB: FigManifoldGetClassID called");
    return NULL;
}

void* FigManifoldGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigManifoldGetFigBaseObject called");
    return NULL;
}

void* FigManifoldRelease(void)
{
    if (verbose) puts("STUB: FigManifoldRelease called");
    return NULL;
}

void* FigManifoldRetain(void)
{
    if (verbose) puts("STUB: FigManifoldRetain called");
    return NULL;
}

void* FigMediaFileSegmenterCreateWithURL(void)
{
    if (verbose) puts("STUB: FigMediaFileSegmenterCreateWithURL called");
    return NULL;
}

void* FigMediaFileSegmenterGetClassID(void)
{
    if (verbose) puts("STUB: FigMediaFileSegmenterGetClassID called");
    return NULL;
}

void* FigMediaFileSegmenterGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMediaFileSegmenterGetFigBaseObject called");
    return NULL;
}

void* FigMediaFileSegmenterGetTypeID(void)
{
    if (verbose) puts("STUB: FigMediaFileSegmenterGetTypeID called");
    return NULL;
}

void* FigMediaProcessorCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCopyPerformanceDictionary called");
    return NULL;
}

void* FigMediaProcessorCreate(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreate called");
    return NULL;
}

void* FigMediaProcessorCreateForAudioCompression(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForAudioCompression called");
    return NULL;
}

void* FigMediaProcessorCreateForAudioCompressionWithFormatWriter(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForAudioCompressionWithFormatWriter called");
    return NULL;
}

void* FigMediaProcessorCreateForAudioDecompressionWithBufferQueue(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForAudioDecompressionWithBufferQueue called");
    return NULL;
}

void* FigMediaProcessorCreateForVideoCompression(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForVideoCompression called");
    return NULL;
}

void* FigMediaProcessorCreateForVideoCompressionWithFormatWriter(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForVideoCompressionWithFormatWriter called");
    return NULL;
}

void* FigMediaProcessorCreateForVideoCompressionWithFrameSilo(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForVideoCompressionWithFrameSilo called");
    return NULL;
}

void* FigMediaProcessorCreateForVideoRotation(void)
{
    if (verbose) puts("STUB: FigMediaProcessorCreateForVideoRotation called");
    return NULL;
}

void* FigMediaProcessorFlush(void)
{
    if (verbose) puts("STUB: FigMediaProcessorFlush called");
    return NULL;
}

void* FigMediaProcessorGetPixelBufferPool(void)
{
    if (verbose) puts("STUB: FigMediaProcessorGetPixelBufferPool called");
    return NULL;
}

void* FigMediaProcessorGetSampleBufferProcessor(void)
{
    if (verbose) puts("STUB: FigMediaProcessorGetSampleBufferProcessor called");
    return NULL;
}

void* FigMediaProcessorGetTypeID(void)
{
    if (verbose) puts("STUB: FigMediaProcessorGetTypeID called");
    return NULL;
}

void* FigMediaProcessorGetVTCompressionSession(void)
{
    if (verbose) puts("STUB: FigMediaProcessorGetVTCompressionSession called");
    return NULL;
}

void* FigMediaProcessorGo(void)
{
    if (verbose) puts("STUB: FigMediaProcessorGo called");
    return NULL;
}

void* FigMediaProcessorInvalidate(void)
{
    if (verbose) puts("STUB: FigMediaProcessorInvalidate called");
    return NULL;
}

void* FigMediaProcessorIsAtEndOfOutputData(void)
{
    if (verbose) puts("STUB: FigMediaProcessorIsAtEndOfOutputData called");
    return NULL;
}

void* FigMediaProcessorReset(void)
{
    if (verbose) puts("STUB: FigMediaProcessorReset called");
    return NULL;
}

void* FigMediaProcessorSetProperty(void)
{
    if (verbose) puts("STUB: FigMediaProcessorSetProperty called");
    return NULL;
}

void* FigMediaProcessorStop(void)
{
    if (verbose) puts("STUB: FigMediaProcessorStop called");
    return NULL;
}

void* FigMediaProcessorWaitUntilCompletelyStopped(void)
{
    if (verbose) puts("STUB: FigMediaProcessorWaitUntilCompletelyStopped called");
    return NULL;
}

void* FigMediaValidatorCopyByteStream(void)
{
    if (verbose) puts("STUB: FigMediaValidatorCopyByteStream called");
    return NULL;
}

void* FigMediaValidatorCopyResolvedURL(void)
{
    if (verbose) puts("STUB: FigMediaValidatorCopyResolvedURL called");
    return NULL;
}

void* FigMediaValidatorCreate(void)
{
    if (verbose) puts("STUB: FigMediaValidatorCreate called");
    return NULL;
}

void* FigMediaValidatorCreateSync(void)
{
    if (verbose) puts("STUB: FigMediaValidatorCreateSync called");
    return NULL;
}

void* FigMediaValidatorCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigMediaValidatorCreateWithByteStream called");
    return NULL;
}

void* FigMediaValidatorGetMaxFrameRate(void)
{
    if (verbose) puts("STUB: FigMediaValidatorGetMaxFrameRate called");
    return NULL;
}

void* FigMediaValidatorGetStatus(void)
{
    if (verbose) puts("STUB: FigMediaValidatorGetStatus called");
    return NULL;
}

void* FigMediaValidatorIsNonByteStreamURLStreaming(void)
{
    if (verbose) puts("STUB: FigMediaValidatorIsNonByteStreamURLStreaming called");
    return NULL;
}

void* FigMediaValidatorIsNonRefMovieURLStreaming(void)
{
    if (verbose) puts("STUB: FigMediaValidatorIsNonRefMovieURLStreaming called");
    return NULL;
}

void* FigMediaValidatorMIMETypeIsShoutcastAudio(void)
{
    if (verbose) puts("STUB: FigMediaValidatorMIMETypeIsShoutcastAudio called");
    return NULL;
}

void* FigMediaValidatorRelease(void)
{
    if (verbose) puts("STUB: FigMediaValidatorRelease called");
    return NULL;
}

void* FigMediaValidatorValidateByteStream(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateByteStream called");
    return NULL;
}

void* FigMediaValidatorValidateFormatDescription(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateFormatDescription called");
    return NULL;
}

void* FigMediaValidatorValidateIndividualTrack(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateIndividualTrack called");
    return NULL;
}

void* FigMediaValidatorValidateIndividualTrackForDecoding(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateIndividualTrackForDecoding called");
    return NULL;
}

void* FigMediaValidatorValidateRFC4281CodecsForStreaming(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateRFC4281CodecsForStreaming called");
    return NULL;
}

void* FigMediaValidatorValidateRFC4281ExtendedMIMEType(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateRFC4281ExtendedMIMEType called");
    return NULL;
}

void* FigMediaValidatorValidateRFC4281ExtendedMIMETypeForStreaming(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateRFC4281ExtendedMIMETypeForStreaming called");
    return NULL;
}

void* FigMediaValidatorValidateURL(void)
{
    if (verbose) puts("STUB: FigMediaValidatorValidateURL called");
    return NULL;
}

void* FigMentorNotificationPayloadCreate(void)
{
    if (verbose) puts("STUB: FigMentorNotificationPayloadCreate called");
    return NULL;
}

void* FigMetadataArrayCopyItemWithKeyAndIndex(void)
{
    if (verbose) puts("STUB: FigMetadataArrayCopyItemWithKeyAndIndex called");
    return NULL;
}

void* FigMetadataArrayCopyKeyAtIndex(void)
{
    if (verbose) puts("STUB: FigMetadataArrayCopyKeyAtIndex called");
    return NULL;
}

void* FigMetadataArrayGetItemCount(void)
{
    if (verbose) puts("STUB: FigMetadataArrayGetItemCount called");
    return NULL;
}

void* FigMetadataArrayGetKeyCount(void)
{
    if (verbose) puts("STUB: FigMetadataArrayGetKeyCount called");
    return NULL;
}

void* FigMetadataArrayHasKey(void)
{
    if (verbose) puts("STUB: FigMetadataArrayHasKey called");
    return NULL;
}

void* FigMetadataConverterCreateForQuickTime(void)
{
    if (verbose) puts("STUB: FigMetadataConverterCreateForQuickTime called");
    return NULL;
}

void* FigMetadataConverterCreateForQuickTimeToFromiTunes(void)
{
    if (verbose) puts("STUB: FigMetadataConverterCreateForQuickTimeToFromiTunes called");
    return NULL;
}

void* FigMetadataConverterGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigMetadataConverterGetCFTypeID called");
    return NULL;
}

void* FigMetadataConverterGetClassID(void)
{
    if (verbose) puts("STUB: FigMetadataConverterGetClassID called");
    return NULL;
}

void* FigMetadataConverterGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMetadataConverterGetFigBaseObject called");
    return NULL;
}

void* FigMetadataCopyAllMetadataItems(void)
{
    if (verbose) puts("STUB: FigMetadataCopyAllMetadataItems called");
    return NULL;
}

void* FigMetadataCopyCommonMetadataForAVCHD(void)
{
    if (verbose) puts("STUB: FigMetadataCopyCommonMetadataForAVCHD called");
    return NULL;
}

void* FigMetadataCopyMovieCommonMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataCopyMovieCommonMetadata called");
    return NULL;
}

void* FigMetadataCopyMovieMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataCopyMovieMetadata called");
    return NULL;
}

void* FigMetadataCopyTrackCommonMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataCopyTrackCommonMetadata called");
    return NULL;
}

void* FigMetadataCopyTrackMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataCopyTrackMetadata called");
    return NULL;
}

void* FigMetadataCreateConverter(void)
{
    if (verbose) puts("STUB: FigMetadataCreateConverter called");
    return NULL;
}

void* FigMetadataCreateMetadataItemsFromSampleBuffer(void)
{
    if (verbose) puts("STUB: FigMetadataCreateMetadataItemsFromSampleBuffer called");
    return NULL;
}

void* FigMetadataCreateSampleBuffer(void)
{
    if (verbose) puts("STUB: FigMetadataCreateSampleBuffer called");
    return NULL;
}

void* FigMetadataGetAllCommonKeys(void)
{
    if (verbose) puts("STUB: FigMetadataGetAllCommonKeys called");
    return NULL;
}

void* FigMetadataGetCommonKey(void)
{
    if (verbose) puts("STUB: FigMetadataGetCommonKey called");
    return NULL;
}

void* FigMetadataGetCoreMediaDataType(void)
{
    if (verbose) puts("STUB: FigMetadataGetCoreMediaDataType called");
    return NULL;
}

void* FigMetadataGetDataTypeWithNamespaceForCoreMediaDataType(void)
{
    if (verbose) puts("STUB: FigMetadataGetDataTypeWithNamespaceForCoreMediaDataType called");
    return NULL;
}

void* FigMetadataGetISO639_1FromISO639_2T(void)
{
    if (verbose) puts("STUB: FigMetadataGetISO639_1FromISO639_2T called");
    return NULL;
}

void* FigMetadataGetPackedISO639_2TFromLocaleIdentifier(void)
{
    if (verbose) puts("STUB: FigMetadataGetPackedISO639_2TFromLocaleIdentifier called");
    return NULL;
}

void* FigMetadataReaderCreateForAVCHDMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForAVCHDMetadata called");
    return NULL;
}

void* FigMetadataReaderCreateForBoxedMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForBoxedMetadata called");
    return NULL;
}

void* FigMetadataReaderCreateForID3(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForID3 called");
    return NULL;
}

void* FigMetadataReaderCreateForID3WithBackwardCompatibility(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForID3WithBackwardCompatibility called");
    return NULL;
}

void* FigMetadataReaderCreateForISOUserData(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForISOUserData called");
    return NULL;
}

void* FigMetadataReaderCreateForISOUserDataArray(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForISOUserDataArray called");
    return NULL;
}

void* FigMetadataReaderCreateForISOUserDataBlockBuffer(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForISOUserDataBlockBuffer called");
    return NULL;
}

void* FigMetadataReaderCreateForQuickTimeMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForQuickTimeMetadata called");
    return NULL;
}

void* FigMetadataReaderCreateForQuickTimeMetadataArray(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForQuickTimeMetadataArray called");
    return NULL;
}

void* FigMetadataReaderCreateForQuickTimeUserData(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForQuickTimeUserData called");
    return NULL;
}

void* FigMetadataReaderCreateForQuickTimeUserDataArray(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForQuickTimeUserDataArray called");
    return NULL;
}

void* FigMetadataReaderCreateForQuickTimeUserDataBlockBuffer(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForQuickTimeUserDataBlockBuffer called");
    return NULL;
}

void* FigMetadataReaderCreateForTimedID3(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForTimedID3 called");
    return NULL;
}

void* FigMetadataReaderCreateForiTunes(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForiTunes called");
    return NULL;
}

void* FigMetadataReaderCreateForiTunesMetadataArray(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateForiTunesMetadataArray called");
    return NULL;
}

void* FigMetadataReaderCreateWithSampleBufferForIcy(void)
{
    if (verbose) puts("STUB: FigMetadataReaderCreateWithSampleBufferForIcy called");
    return NULL;
}

void* FigMetadataReaderGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigMetadataReaderGetCFTypeID called");
    return NULL;
}

void* FigMetadataReaderGetClassID(void)
{
    if (verbose) puts("STUB: FigMetadataReaderGetClassID called");
    return NULL;
}

void* FigMetadataReaderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMetadataReaderGetFigBaseObject called");
    return NULL;
}

void* FigMetadataReaderRelease(void)
{
    if (verbose) puts("STUB: FigMetadataReaderRelease called");
    return NULL;
}

void* FigMetadataReaderRetain(void)
{
    if (verbose) puts("STUB: FigMetadataReaderRetain called");
    return NULL;
}

void* FigMetadataRewriterDictionaryCreateFromFigMetadataPropertyArray(void)
{
    if (verbose) puts("STUB: FigMetadataRewriterDictionaryCreateFromFigMetadataPropertyArray called");
    return NULL;
}

void* FigMetadataRewriterSetValues(void)
{
    if (verbose) puts("STUB: FigMetadataRewriterSetValues called");
    return NULL;
}

void* FigMetadataSerializerCreateForISOUserData(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerCreateForISOUserData called");
    return NULL;
}

void* FigMetadataSerializerCreateForQuickTimeMetadata(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerCreateForQuickTimeMetadata called");
    return NULL;
}

void* FigMetadataSerializerCreateForQuickTimeUserData(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerCreateForQuickTimeUserData called");
    return NULL;
}

void* FigMetadataSerializerCreateForiTunes(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerCreateForiTunes called");
    return NULL;
}

void* FigMetadataSerializerGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerGetCFTypeID called");
    return NULL;
}

void* FigMetadataSerializerGetClassID(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerGetClassID called");
    return NULL;
}

void* FigMetadataSerializerGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerGetFigBaseObject called");
    return NULL;
}

void* FigMetadataSerializerGetFigMetadataWriter(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerGetFigMetadataWriter called");
    return NULL;
}

void* FigMetadataSerializerRelease(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerRelease called");
    return NULL;
}

void* FigMetadataSerializerRetain(void)
{
    if (verbose) puts("STUB: FigMetadataSerializerRetain called");
    return NULL;
}

void* FigMetadataStringKeyToOSTypeKeyWithKeySpace(void)
{
    if (verbose) puts("STUB: FigMetadataStringKeyToOSTypeKeyWithKeySpace called");
    return NULL;
}

void* FigMetadataWriterGetCFTypeID(void)
{
    if (verbose) puts("STUB: FigMetadataWriterGetCFTypeID called");
    return NULL;
}

void* FigMetadataWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigMetadataWriterGetClassID called");
    return NULL;
}

void* FigMetadataWriterGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMetadataWriterGetFigBaseObject called");
    return NULL;
}

void* FigMetadataWriterRelease(void)
{
    if (verbose) puts("STUB: FigMetadataWriterRelease called");
    return NULL;
}

void* FigMetadataWriterRetain(void)
{
    if (verbose) puts("STUB: FigMetadataWriterRetain called");
    return NULL;
}

void* FigMovieFormatWriterCanFileTypeSupportFormatDescription(void)
{
    if (verbose) puts("STUB: FigMovieFormatWriterCanFileTypeSupportFormatDescription called");
    return NULL;
}

void* FigMovieFormatWriterCanFileTypeSupportMediaType(void)
{
    if (verbose) puts("STUB: FigMovieFormatWriterCanFileTypeSupportMediaType called");
    return NULL;
}

void* FigMovieFormatWriterCreateForWritingMovieFragments(void)
{
    if (verbose) puts("STUB: FigMovieFormatWriterCreateForWritingMovieFragments called");
    return NULL;
}

void* FigMovieFormatWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigMovieFormatWriterCreateWithByteStream called");
    return NULL;
}

void* FigMovieUsesFragments(void)
{
    if (verbose) puts("STUB: FigMovieUsesFragments called");
    return NULL;
}

void* FigMutableCompositionCreate(void)
{
    if (verbose) puts("STUB: FigMutableCompositionCreate called");
    return NULL;
}

void* FigMutableCompositionGetClassID(void)
{
    if (verbose) puts("STUB: FigMutableCompositionGetClassID called");
    return NULL;
}

void* FigMutableCompositionGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMutableCompositionGetFigBaseObject called");
    return NULL;
}

void* FigMutableCompositionGetTypeID(void)
{
    if (verbose) puts("STUB: FigMutableCompositionGetTypeID called");
    return NULL;
}

void* FigMutableMovieCreateEmpty(void)
{
    if (verbose) puts("STUB: FigMutableMovieCreateEmpty called");
    return NULL;
}

void* FigMutableMovieCreateFromData(void)
{
    if (verbose) puts("STUB: FigMutableMovieCreateFromData called");
    return NULL;
}

void* FigMutableMovieCreateFromFormatReader(void)
{
    if (verbose) puts("STUB: FigMutableMovieCreateFromFormatReader called");
    return NULL;
}

void* FigMutableMovieCreateWithURL(void)
{
    if (verbose) puts("STUB: FigMutableMovieCreateWithURL called");
    return NULL;
}

void* FigMutableMovieGetClassID(void)
{
    if (verbose) puts("STUB: FigMutableMovieGetClassID called");
    return NULL;
}

void* FigMutableMovieGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigMutableMovieGetFigBaseObject called");
    return NULL;
}

void* FigMutableMovieGetTypeID(void)
{
    if (verbose) puts("STUB: FigMutableMovieGetTypeID called");
    return NULL;
}

void* FigNSURLSessionAssertionCreate(void)
{
    if (verbose) puts("STUB: FigNSURLSessionAssertionCreate called");
    return NULL;
}

void* FigNSURLSessionAssertionGetTypeID(void)
{
    if (verbose) puts("STUB: FigNSURLSessionAssertionGetTypeID called");
    return NULL;
}

void* FigNeroCreate(void)
{
    if (verbose) puts("STUB: FigNeroCreate called");
    return NULL;
}

void* FigNeroGetTypeID(void)
{
    if (verbose) puts("STUB: FigNeroGetTypeID called");
    return NULL;
}

void* FigNeroSendAudioInputBuffer(void)
{
    if (verbose) puts("STUB: FigNeroSendAudioInputBuffer called");
    return NULL;
}

void* FigNeroSendMessage(void)
{
    if (verbose) puts("STUB: FigNeroSendMessage called");
    return NULL;
}

void* FigNeroSetup(void)
{
    if (verbose) puts("STUB: FigNeroSetup called");
    return NULL;
}

void* FigNeroStartStream(void)
{
    if (verbose) puts("STUB: FigNeroStartStream called");
    return NULL;
}

void* FigNeroStopStream(void)
{
    if (verbose) puts("STUB: FigNeroStopStream called");
    return NULL;
}

void* FigNeroTeardown(void)
{
    if (verbose) puts("STUB: FigNeroTeardown called");
    return NULL;
}

void* FigOctaviaVideoRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigOctaviaVideoRenderPipelineCreate called");
    return NULL;
}

void* FigOctaviaVideoRenderPipelineCreateWithFigImageQueueArray(void)
{
    if (verbose) puts("STUB: FigOctaviaVideoRenderPipelineCreateWithFigImageQueueArray called");
    return NULL;
}

void* FigOfflineAudioRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigOfflineAudioRenderPipelineCreate called");
    return NULL;
}

void* FigOutputMonitorIsScreenProbablyBeingRecorded(void)
{
    if (verbose) puts("STUB: FigOutputMonitorIsScreenProbablyBeingRecorded called");
    return NULL;
}

void* FigParseHLSfMP4SecurityInfoAndCopyIV(void)
{
    if (verbose) puts("STUB: FigParseHLSfMP4SecurityInfoAndCopyIV called");
    return NULL;
}

void* FigParseHLSfMP4SecurityInfoFromFormatDescriptionAndCopyIV(void)
{
    if (verbose) puts("STUB: FigParseHLSfMP4SecurityInfoFromFormatDescriptionAndCopyIV called");
    return NULL;
}

void* FigPartialSampleTableAccessorCopy(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorCopy called");
    return NULL;
}

void* FigPartialSampleTableAccessorDestroy(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorDestroy called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetChunkDetails(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetChunkDetails called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetDependencyInfo(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetDependencyInfo called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetExtendedDependencyAttributeInfo(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetExtendedDependencyAttributeInfo called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetFormatDescriptionIdentifier(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetFormatDescriptionIdentifier called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetSampleTiming(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetSampleTiming called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetSyncInfo(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetSyncInfo called");
    return NULL;
}

void* FigPartialSampleTableAccessorGetUnrefinedSampleLocation(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorGetUnrefinedSampleLocation called");
    return NULL;
}

void* FigPartialSampleTableAccessorStep(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableAccessorStep called");
    return NULL;
}

void* FigPartialSampleTableCreateAccessorAtIndex(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableCreateAccessorAtIndex called");
    return NULL;
}

void* FigPartialSampleTableCreateForWriting(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableCreateForWriting called");
    return NULL;
}

void* FigPartialSampleTableCreateFromBlockBuffer(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableCreateFromBlockBuffer called");
    return NULL;
}

void* FigPartialSampleTableFinishWriting(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableFinishWriting called");
    return NULL;
}

void* FigPartialSampleTableGetTypeID(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableGetTypeID called");
    return NULL;
}

void* FigPartialSampleTableHelperCreateOutOfBandObjectsFromCompactData(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableHelperCreateOutOfBandObjectsFromCompactData called");
    return NULL;
}

void* FigPartialSampleTableHelperFillCompactDataForOutOfBandObjects(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableHelperFillCompactDataForOutOfBandObjects called");
    return NULL;
}

void* FigPartialSampleTableTryAppendSampleInformation(void)
{
    if (verbose) puts("STUB: FigPartialSampleTableTryAppendSampleInformation called");
    return NULL;
}

void* FigPhotoAuxiliaryImageMetadataCreateFromXMPData(void)
{
    if (verbose) puts("STUB: FigPhotoAuxiliaryImageMetadataCreateFromXMPData called");
    return NULL;
}

void* FigPhotoAuxiliaryImageMetadataCreateXMPData(void)
{
    if (verbose) puts("STUB: FigPhotoAuxiliaryImageMetadataCreateXMPData called");
    return NULL;
}

void* FigPhotoCodecSessionPoolCreate(void)
{
    if (verbose) puts("STUB: FigPhotoCodecSessionPoolCreate called");
    return NULL;
}

void* FigPhotoCodecSessionPoolCreateCompressionSession(void)
{
    if (verbose) puts("STUB: FigPhotoCodecSessionPoolCreateCompressionSession called");
    return NULL;
}

void* FigPhotoCodecSessionPoolCreateDecompressionSession(void)
{
    if (verbose) puts("STUB: FigPhotoCodecSessionPoolCreateDecompressionSession called");
    return NULL;
}

void* FigPhotoCodecSessionPoolFlush(void)
{
    if (verbose) puts("STUB: FigPhotoCodecSessionPoolFlush called");
    return NULL;
}

void* FigPhotoCodecSessionPoolRecycleSession(void)
{
    if (verbose) puts("STUB: FigPhotoCodecSessionPoolRecycleSession called");
    return NULL;
}

void* FigPhotoCompressionCreateContainerFromImageExt(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionCreateContainerFromImageExt called");
    return NULL;
}

void* FigPhotoCompressionCreateDataContainerFromImage(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionCreateDataContainerFromImage called");
    return NULL;
}

void* FigPhotoCompressionCreateNativePixelFormatArray(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionCreateNativePixelFormatArray called");
    return NULL;
}

void* FigPhotoCompressionDiscardCaches(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionDiscardCaches called");
    return NULL;
}

void* FigPhotoCompressionSessionAddAuxiliaryImage(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddAuxiliaryImage called");
    return NULL;
}

void* FigPhotoCompressionSessionAddAuxiliaryImageFromDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddAuxiliaryImageFromDictionaryRepresentation called");
    return NULL;
}

void* FigPhotoCompressionSessionAddCompressedImage(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddCompressedImage called");
    return NULL;
}

void* FigPhotoCompressionSessionAddCompressedMovie(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddCompressedMovie called");
    return NULL;
}

void* FigPhotoCompressionSessionAddDebugMetadata(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddDebugMetadata called");
    return NULL;
}

void* FigPhotoCompressionSessionAddExif(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddExif called");
    return NULL;
}

void* FigPhotoCompressionSessionAddImage(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddImage called");
    return NULL;
}

void* FigPhotoCompressionSessionAddImageToSequence(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddImageToSequence called");
    return NULL;
}

void* FigPhotoCompressionSessionAddMetadataFromImageProperties(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddMetadataFromImageProperties called");
    return NULL;
}

void* FigPhotoCompressionSessionAddThumbnail(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddThumbnail called");
    return NULL;
}

void* FigPhotoCompressionSessionAddXMP(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionAddXMP called");
    return NULL;
}

void* FigPhotoCompressionSessionCloseContainer(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionCloseContainer called");
    return NULL;
}

void* FigPhotoCompressionSessionCloseContainerAndCopyBacking(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionCloseContainerAndCopyBacking called");
    return NULL;
}

void* FigPhotoCompressionSessionCreate(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionCreate called");
    return NULL;
}

void* FigPhotoCompressionSessionDiscardCaches(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionDiscardCaches called");
    return NULL;
}

void* FigPhotoCompressionSessionEndImageSequence(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionEndImageSequence called");
    return NULL;
}

void* FigPhotoCompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionGetTypeID called");
    return NULL;
}

void* FigPhotoCompressionSessionInvalidate(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionInvalidate called");
    return NULL;
}

void* FigPhotoCompressionSessionIsContainerOpen(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionIsContainerOpen called");
    return NULL;
}

void* FigPhotoCompressionSessionOpenEmptyContainer(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionOpenEmptyContainer called");
    return NULL;
}

void* FigPhotoCompressionSessionOpenExistingContainer(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionOpenExistingContainer called");
    return NULL;
}

void* FigPhotoCompressionSessionOpenExistingContainerForModification(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionOpenExistingContainerForModification called");
    return NULL;
}

void* FigPhotoCompressionSessionOpenExistingContainerForModificationF(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionOpenExistingContainerForModificationF called");
    return NULL;
}

void* FigPhotoCompressionSessionReleaseHardwareResources(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionReleaseHardwareResources called");
    return NULL;
}

void* FigPhotoCompressionSessionStartImageSequence(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSessionStartImageSequence called");
    return NULL;
}

void* FigPhotoCompressionSuggestedPaddingForOptions(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSuggestedPaddingForOptions called");
    return NULL;
}

void* FigPhotoCompressionSupportedForContainerAndCodec(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionSupportedForContainerAndCodec called");
    return NULL;
}

void* FigPhotoCompressionWriteFileContainerFromImage(void)
{
    if (verbose) puts("STUB: FigPhotoCompressionWriteFileContainerFromImage called");
    return NULL;
}

void* FigPhotoCreateCompressionCompatiblePixelBuffer(void)
{
    if (verbose) puts("STUB: FigPhotoCreateCompressionCompatiblePixelBuffer called");
    return NULL;
}

void* FigPhotoCreateFractionalPixelBuffer(void)
{
    if (verbose) puts("STUB: FigPhotoCreateFractionalPixelBuffer called");
    return NULL;
}

void* FigPhotoDecompressionContainerCancelAsyncRequest(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCancelAsyncRequest called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageBitDepthForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageBitDepthForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageCGColorSpaceForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageCGColorSpaceForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageFormatDescriptionForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageFormatDescriptionForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageMetadataForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageMetadataForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageSubTypeDataForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageSubTypeDataForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyAuxiliaryImageTypeURNForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyAuxiliaryImageTypeURNForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyCGColorSpaceForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyCGColorSpaceForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyDebugMetadataForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyDebugMetadataForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyExifForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyExifForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyFormatDescriptionForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyFormatDescriptionForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyImageBitDepthForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyImageBitDepthForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyImagePropertiesForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyImagePropertiesForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyImageSubsamplingForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyImageSubsamplingForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyThumbnailBitDepthForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyThumbnailBitDepthForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyThumbnailFormatDescriptionForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyThumbnailFormatDescriptionForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCopyXMPForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCopyXMPForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateAuxiliaryImageDictionaryRepresentation(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateAuxiliaryImageDictionaryRepresentation called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateAuxiliaryImageForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateAuxiliaryImageForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateDictionaryDescription(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateDictionaryDescription called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateImageForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateImageForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateOutputBufferAttributesForAuxiliaryImageIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateOutputBufferAttributesForAuxiliaryImageIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateOutputBufferAttributesForImageIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateOutputBufferAttributesForImageIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateOutputBufferAttributesForThumbnailIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateOutputBufferAttributesForThumbnailIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateSequenceContainer(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateSequenceContainer called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateThumbnailImageForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateThumbnailImageForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateTileIteratorForAuxiliaryImageIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateTileIteratorForAuxiliaryImageIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateTileIteratorForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateTileIteratorForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerCreateTileIteratorForThumbnailImageIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerCreateTileIteratorForThumbnailImageIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerDecodeImageForIndexAsync(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerDecodeImageForIndexAsync called");
    return NULL;
}

void* FigPhotoDecompressionContainerDecodeImageForIndexAsyncF(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerDecodeImageForIndexAsyncF called");
    return NULL;
}

void* FigPhotoDecompressionContainerDecodeThumbnailForIndexAsync(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerDecodeThumbnailForIndexAsync called");
    return NULL;
}

void* FigPhotoDecompressionContainerDecodeThumbnailForIndexAsyncF(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerDecodeThumbnailForIndexAsyncF called");
    return NULL;
}

void* FigPhotoDecompressionContainerDetectSourceIsMIAF(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerDetectSourceIsMIAF called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetAuxiliaryImageCountForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetAuxiliaryImageCountForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetAuxiliaryImageDimensionsForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetAuxiliaryImageDimensionsForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetAuxiliaryImageGeometryForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetAuxiliaryImageGeometryForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetAuxiliaryImageTypeForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetAuxiliaryImageTypeForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetContainerFormat(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetContainerFormat called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetDebugMetadataCountForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetDebugMetadataCountForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetExifCountForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetExifCountForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetImageCount(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetImageCount called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetImageGeometryForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetImageGeometryForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetPrimaryItemIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetPrimaryItemIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetSequenceCount(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetSequenceCount called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetThumbnailCountForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetThumbnailCountForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetThumbnailDimensionsForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetThumbnailDimensionsForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetThumbnailGeometryForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetThumbnailGeometryForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetTypeID called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetVideoAndAudioCount(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetVideoAndAudioCount called");
    return NULL;
}

void* FigPhotoDecompressionContainerGetXMPCountForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerGetXMPCountForIndex called");
    return NULL;
}

void* FigPhotoDecompressionContainerJFIFTranscode(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerJFIFTranscode called");
    return NULL;
}

void* FigPhotoDecompressionContainerPredictTranscodedSize(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionContainerPredictTranscodedSize called");
    return NULL;
}

void* FigPhotoDecompressionCreateCGImageForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionCreateCGImageForIndex called");
    return NULL;
}

void* FigPhotoDecompressionCreateCVPixelBufferForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionCreateCVPixelBufferForIndex called");
    return NULL;
}

void* FigPhotoDecompressionCreateNativePixelFormatArray(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionCreateNativePixelFormatArray called");
    return NULL;
}

void* FigPhotoDecompressionDecodeIntoRGBSurfaceForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionDecodeIntoRGBSurfaceForIndex called");
    return NULL;
}

void* FigPhotoDecompressionDetectContainerFormat(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionDetectContainerFormat called");
    return NULL;
}

void* FigPhotoDecompressionDetectSourceIsMIAF(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionDetectSourceIsMIAF called");
    return NULL;
}

void* FigPhotoDecompressionDiscardCaches(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionDiscardCaches called");
    return NULL;
}

void* FigPhotoDecompressionGetImageCount(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionGetImageCount called");
    return NULL;
}

void* FigPhotoDecompressionGetImageGeometryForIndex(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionGetImageGeometryForIndex called");
    return NULL;
}

void* FigPhotoDecompressionSessionCreate(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionCreate called");
    return NULL;
}

void* FigPhotoDecompressionSessionCreateContainer(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionCreateContainer called");
    return NULL;
}

void* FigPhotoDecompressionSessionDetachSurface(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionDetachSurface called");
    return NULL;
}

void* FigPhotoDecompressionSessionDiscardCachedBuffers(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionDiscardCachedBuffers called");
    return NULL;
}

void* FigPhotoDecompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionGetTypeID called");
    return NULL;
}

void* FigPhotoDecompressionSessionReleaseHardwareResources(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionReleaseHardwareResources called");
    return NULL;
}

void* FigPhotoDecompressionSessionReserveRequestID(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionReserveRequestID called");
    return NULL;
}

void* FigPhotoDecompressionSessionSupportsCodec(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSessionSupportsCodec called");
    return NULL;
}

void* FigPhotoDecompressionSupportedForContainerAndCodec(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionSupportedForContainerAndCodec called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorCancelAsyncRequest(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorCancelAsyncRequest called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorCopyTileSampleBuffer(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorCopyTileSampleBuffer called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorDecodeTileAsync(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorDecodeTileAsync called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorDecodeTileAsyncF(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorDecodeTileAsyncF called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorGetTileItemType(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorGetTileItemType called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorGetTileSourceRect(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorGetTileSourceRect called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorGetTypeID called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorMoveToPosition(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorMoveToPosition called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorReset(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorReset called");
    return NULL;
}

void* FigPhotoDecompressionTileIteratorStepAndReportPosition(void)
{
    if (verbose) puts("STUB: FigPhotoDecompressionTileIteratorStepAndReportPosition called");
    return NULL;
}

void* FigPhotoDeviceSupportsTileDecodingToCanvas(void)
{
    if (verbose) puts("STUB: FigPhotoDeviceSupportsTileDecodingToCanvas called");
    return NULL;
}

void* FigPhotoEncoderSupportsPixelFormat(void)
{
    if (verbose) puts("STUB: FigPhotoEncoderSupportsPixelFormat called");
    return NULL;
}

void* FigPhotoGetAPIVersion(void)
{
    if (verbose) puts("STUB: FigPhotoGetAPIVersion called");
    return NULL;
}

void* FigPhotoGetCPUCount(void)
{
    if (verbose) puts("STUB: FigPhotoGetCPUCount called");
    return NULL;
}

void* FigPhotoGetImageType(void)
{
    if (verbose) puts("STUB: FigPhotoGetImageType called");
    return NULL;
}

void* FigPhotoIsTileDecoderAvailable(void)
{
    if (verbose) puts("STUB: FigPhotoIsTileDecoderAvailable called");
    return NULL;
}

void* FigPhotoIsTileEncoderAvailable(void)
{
    if (verbose) puts("STUB: FigPhotoIsTileEncoderAvailable called");
    return NULL;
}

void* FigPhotoReleaseHardwareResources(void)
{
    if (verbose) puts("STUB: FigPhotoReleaseHardwareResources called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionCreate(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionCreate called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionDiscardCaches(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionDiscardCaches called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionGetTypeID called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionTransformForMaxSideLength(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionTransformForMaxSideLength called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionTransformForSize(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionTransformForSize called");
    return NULL;
}

void* FigPhotoScaleAndRotateSessionTransformIntoCanvas(void)
{
    if (verbose) puts("STUB: FigPhotoScaleAndRotateSessionTransformIntoCanvas called");
    return NULL;
}

void* FigPhotoSupportsAVC400Encode(void)
{
    if (verbose) puts("STUB: FigPhotoSupportsAVC400Encode called");
    return NULL;
}

void* FigPhotoSupportsAVCEncode(void)
{
    if (verbose) puts("STUB: FigPhotoSupportsAVCEncode called");
    return NULL;
}

void* FigPhotoSupportsHEVCEncode(void)
{
    if (verbose) puts("STUB: FigPhotoSupportsHEVCEncode called");
    return NULL;
}

void* FigPhotoSurfacePoolCreate(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolCreate called");
    return NULL;
}

void* FigPhotoSurfacePoolCreateImageSurface(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolCreateImageSurface called");
    return NULL;
}

void* FigPhotoSurfacePoolCreateMemorySurface(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolCreateMemorySurface called");
    return NULL;
}

void* FigPhotoSurfacePoolCreatePixelBuffer(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolCreatePixelBuffer called");
    return NULL;
}

void* FigPhotoSurfacePoolDetachImageSurface(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolDetachImageSurface called");
    return NULL;
}

void* FigPhotoSurfacePoolDetachMemorySurface(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolDetachMemorySurface called");
    return NULL;
}

void* FigPhotoSurfacePoolDiscardCaches(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolDiscardCaches called");
    return NULL;
}

void* FigPhotoSurfacePoolGetStats(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolGetStats called");
    return NULL;
}

void* FigPhotoSurfacePoolGetTypeID(void)
{
    if (verbose) puts("STUB: FigPhotoSurfacePoolGetTypeID called");
    return NULL;
}

void* FigPictureCollectionCreateFromByteStream(void)
{
    if (verbose) puts("STUB: FigPictureCollectionCreateFromByteStream called");
    return NULL;
}

void* FigPictureCollectionGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigPictureCollectionGetCMBaseObject called");
    return NULL;
}

void* FigPictureCollectionGetClassID(void)
{
    if (verbose) puts("STUB: FigPictureCollectionGetClassID called");
    return NULL;
}

void* FigPictureCollectionGetTypeID(void)
{
    if (verbose) puts("STUB: FigPictureCollectionGetTypeID called");
    return NULL;
}

void* FigPictureReaderGetClassID(void)
{
    if (verbose) puts("STUB: FigPictureReaderGetClassID called");
    return NULL;
}

void* FigPictureReaderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigPictureReaderGetFigBaseObject called");
    return NULL;
}

void* FigPictureReaderGetTypeID(void)
{
    if (verbose) puts("STUB: FigPictureReaderGetTypeID called");
    return NULL;
}

void* FigPictureTileCursorGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorGetCMBaseObject called");
    return NULL;
}

void* FigPictureTileCursorGetClassID(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorGetClassID called");
    return NULL;
}

void* FigPictureTileCursorGetTypeID(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorGetTypeID called");
    return NULL;
}

void* FigPictureTileCursorServiceGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorServiceGetCMBaseObject called");
    return NULL;
}

void* FigPictureTileCursorServiceGetClassID(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorServiceGetClassID called");
    return NULL;
}

void* FigPictureTileCursorServiceGetTypeID(void)
{
    if (verbose) puts("STUB: FigPictureTileCursorServiceGetTypeID called");
    return NULL;
}

void* FigPlaybackAssertionCreate(void)
{
    if (verbose) puts("STUB: FigPlaybackAssertionCreate called");
    return NULL;
}

void* FigPlaybackAssertionGetTypeID(void)
{
    if (verbose) puts("STUB: FigPlaybackAssertionGetTypeID called");
    return NULL;
}

void* FigPlaybackBossCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigPlaybackBossCopyPerformanceDictionary called");
    return NULL;
}

void* FigPlaybackBossCopyTrackPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigPlaybackBossCopyTrackPerformanceDictionary called");
    return NULL;
}

void* FigPlaybackBossCreate(void)
{
    if (verbose) puts("STUB: FigPlaybackBossCreate called");
    return NULL;
}

void* FigPlaybackBossDidReachEnd(void)
{
    if (verbose) puts("STUB: FigPlaybackBossDidReachEnd called");
    return NULL;
}

void* FigPlaybackBossGetAdvanceTimeForOverlappedPlayback(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetAdvanceTimeForOverlappedPlayback called");
    return NULL;
}

void* FigPlaybackBossGetAudibleRateRange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetAudibleRateRange called");
    return NULL;
}

void* FigPlaybackBossGetContinuePlayingDuringPrerollForRateChange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetContinuePlayingDuringPrerollForRateChange called");
    return NULL;
}

void* FigPlaybackBossGetContinuePlayingDuringPrerollForSeek(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetContinuePlayingDuringPrerollForSeek called");
    return NULL;
}

void* FigPlaybackBossGetDuration(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetDuration called");
    return NULL;
}

void* FigPlaybackBossGetEndTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetEndTime called");
    return NULL;
}

void* FigPlaybackBossGetIFrameOnlySpeedThreshold(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetIFrameOnlySpeedThreshold called");
    return NULL;
}

void* FigPlaybackBossGetMuted(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetMuted called");
    return NULL;
}

void* FigPlaybackBossGetRate(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetRate called");
    return NULL;
}

void* FigPlaybackBossGetRateSnapping(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetRateSnapping called");
    return NULL;
}

void* FigPlaybackBossGetReversalLimits(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetReversalLimits called");
    return NULL;
}

void* FigPlaybackBossGetReverseEndTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetReverseEndTime called");
    return NULL;
}

void* FigPlaybackBossGetScheduledEndTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetScheduledEndTime called");
    return NULL;
}

void* FigPlaybackBossGetScrubPolicy(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetScrubPolicy called");
    return NULL;
}

void* FigPlaybackBossGetStopAtEnd(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetStopAtEnd called");
    return NULL;
}

void* FigPlaybackBossGetTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetTime called");
    return NULL;
}

void* FigPlaybackBossGetTimebase(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetTimebase called");
    return NULL;
}

void* FigPlaybackBossGetTypeID(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetTypeID called");
    return NULL;
}

void* FigPlaybackBossGetUseIFrameOnlyPlaybackForHighRateScaledEdits(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetUseIFrameOnlyPlaybackForHighRateScaledEdits called");
    return NULL;
}

void* FigPlaybackBossGetVolume(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGetVolume called");
    return NULL;
}

void* FigPlaybackBossGoQuiescent(void)
{
    if (verbose) puts("STUB: FigPlaybackBossGoQuiescent called");
    return NULL;
}

void* FigPlaybackBossInterruptQueueing(void)
{
    if (verbose) puts("STUB: FigPlaybackBossInterruptQueueing called");
    return NULL;
}

void* FigPlaybackBossInvalidate(void)
{
    if (verbose) puts("STUB: FigPlaybackBossInvalidate called");
    return NULL;
}

void* FigPlaybackBossPrerollAndScheduleGaplessStart(void)
{
    if (verbose) puts("STUB: FigPlaybackBossPrerollAndScheduleGaplessStart called");
    return NULL;
}

void* FigPlaybackBossRefreshVideoComposition(void)
{
    if (verbose) puts("STUB: FigPlaybackBossRefreshVideoComposition called");
    return NULL;
}

void* FigPlaybackBossRemoveAndAddTracks(void)
{
    if (verbose) puts("STUB: FigPlaybackBossRemoveAndAddTracks called");
    return NULL;
}

void* FigPlaybackBossSendSampleForTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSendSampleForTime called");
    return NULL;
}

void* FigPlaybackBossSendSamplesForTimeRange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSendSamplesForTimeRange called");
    return NULL;
}

void* FigPlaybackBossSetAdvanceTimeForOverlappedPlayback(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetAdvanceTimeForOverlappedPlayback called");
    return NULL;
}

void* FigPlaybackBossSetAudibleRateRange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetAudibleRateRange called");
    return NULL;
}

void* FigPlaybackBossSetClientPID(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetClientPID called");
    return NULL;
}

void* FigPlaybackBossSetContinuePlayingDuringPrerollForRateChange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetContinuePlayingDuringPrerollForRateChange called");
    return NULL;
}

void* FigPlaybackBossSetContinuePlayingDuringPrerollForSeek(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetContinuePlayingDuringPrerollForSeek called");
    return NULL;
}

void* FigPlaybackBossSetEndTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetEndTime called");
    return NULL;
}

void* FigPlaybackBossSetIFrameOnlySpeedThreshold(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetIFrameOnlySpeedThreshold called");
    return NULL;
}

void* FigPlaybackBossSetMinimumIntervalForIFrameOnlyPlayback(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetMinimumIntervalForIFrameOnlyPlayback called");
    return NULL;
}

void* FigPlaybackBossSetMuted(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetMuted called");
    return NULL;
}

void* FigPlaybackBossSetRate(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetRate called");
    return NULL;
}

void* FigPlaybackBossSetRateAndAnchorTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetRateAndAnchorTime called");
    return NULL;
}

void* FigPlaybackBossSetRateSnapping(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetRateSnapping called");
    return NULL;
}

void* FigPlaybackBossSetReversalLimits(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetReversalLimits called");
    return NULL;
}

void* FigPlaybackBossSetReverseEndTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetReverseEndTime called");
    return NULL;
}

void* FigPlaybackBossSetScrubPolicy(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetScrubPolicy called");
    return NULL;
}

void* FigPlaybackBossSetSeekingWaitsForVideoCompositionRendering(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetSeekingWaitsForVideoCompositionRendering called");
    return NULL;
}

void* FigPlaybackBossSetStopAtEnd(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetStopAtEnd called");
    return NULL;
}

void* FigPlaybackBossSetThrottleForBackground(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetThrottleForBackground called");
    return NULL;
}

void* FigPlaybackBossSetTime(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetTime called");
    return NULL;
}

void* FigPlaybackBossSetTimeWithRange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetTimeWithRange called");
    return NULL;
}

void* FigPlaybackBossSetTrackList(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetTrackList called");
    return NULL;
}

void* FigPlaybackBossSetUseIFrameOnlyPlaybackForHighRateScaledEdits(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetUseIFrameOnlyPlaybackForHighRateScaledEdits called");
    return NULL;
}

void* FigPlaybackBossSetVideoCompositionProcessor(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetVideoCompositionProcessor called");
    return NULL;
}

void* FigPlaybackBossSetVolume(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSetVolume called");
    return NULL;
}

void* FigPlaybackBossSnapTimeToIFrame(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSnapTimeToIFrame called");
    return NULL;
}

void* FigPlaybackBossSnapTimeToIFrameWithRange(void)
{
    if (verbose) puts("STUB: FigPlaybackBossSnapTimeToIFrameWithRange called");
    return NULL;
}

void* FigPlaybackBossStartPreroll(void)
{
    if (verbose) puts("STUB: FigPlaybackBossStartPreroll called");
    return NULL;
}

void* FigPlaybackBossStepByCount(void)
{
    if (verbose) puts("STUB: FigPlaybackBossStepByCount called");
    return NULL;
}

void* FigPlaybackItemGetClassID(void)
{
    if (verbose) puts("STUB: FigPlaybackItemGetClassID called");
    return NULL;
}

void* FigPlaybackItemGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigPlaybackItemGetFigBaseObject called");
    return NULL;
}

void* FigPlaybackItemGetTypeID(void)
{
    if (verbose) puts("STUB: FigPlaybackItemGetTypeID called");
    return NULL;
}

void* FigPlaybackItemLogCreateW3CLogData(void)
{
    if (verbose) puts("STUB: FigPlaybackItemLogCreateW3CLogData called");
    return NULL;
}

void* FigPlaybackRateChangeReasonGetDescription(void)
{
    if (verbose) puts("STUB: FigPlaybackRateChangeReasonGetDescription called");
    return NULL;
}

void* FigPlayerAirPlayCreate(void)
{
    if (verbose) puts("STUB: FigPlayerAirPlayCreate called");
    return NULL;
}

void* FigPlayerAirPlayCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerAirPlayCreateWithOptions called");
    return NULL;
}

void* FigPlayerAsyncCreate(void)
{
    if (verbose) puts("STUB: FigPlayerAsyncCreate called");
    return NULL;
}

void* FigPlayerAsyncCreateWithEngineTopology(void)
{
    if (verbose) puts("STUB: FigPlayerAsyncCreateWithEngineTopology called");
    return NULL;
}

void* FigPlayerAsyncCreateWithEngineTopologyAndOptions(void)
{
    if (verbose) puts("STUB: FigPlayerAsyncCreateWithEngineTopologyAndOptions called");
    return NULL;
}

void* FigPlayerAsyncCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerAsyncCreateWithOptions called");
    return NULL;
}

void* FigPlayerCentralCreate(void)
{
    if (verbose) puts("STUB: FigPlayerCentralCreate called");
    return NULL;
}

void* FigPlayerCentralCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerCentralCreateWithOptions called");
    return NULL;
}

void* FigPlayerFileCreate(void)
{
    if (verbose) puts("STUB: FigPlayerFileCreate called");
    return NULL;
}

void* FigPlayerFileCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerFileCreateWithOptions called");
    return NULL;
}

void* FigPlayerGetClassID(void)
{
    if (verbose) puts("STUB: FigPlayerGetClassID called");
    return NULL;
}

void* FigPlayerGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigPlayerGetFigBaseObject called");
    return NULL;
}

void* FigPlayerGetTypeID(void)
{
    if (verbose) puts("STUB: FigPlayerGetTypeID called");
    return NULL;
}

void* FigPlayerStreamCreate(void)
{
    if (verbose) puts("STUB: FigPlayerStreamCreate called");
    return NULL;
}

void* FigPlayerStreamCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerStreamCreateWithOptions called");
    return NULL;
}

void* FigPlayerSurrogateCreate(void)
{
    if (verbose) puts("STUB: FigPlayerSurrogateCreate called");
    return NULL;
}

void* FigPlayerSurrogateCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigPlayerSurrogateCreateWithOptions called");
    return NULL;
}

void* FigPluginByteSourceCreateWithCMByteStream(void)
{
    if (verbose) puts("STUB: FigPluginByteSourceCreateWithCMByteStream called");
    return NULL;
}

void* FigPluginByteSourceGetClassID(void)
{
    if (verbose) puts("STUB: FigPluginByteSourceGetClassID called");
    return NULL;
}

void* FigPluginByteSourceGetTypeID(void)
{
    if (verbose) puts("STUB: FigPluginByteSourceGetTypeID called");
    return NULL;
}

void* FigReadAheadAssertionCreate(void)
{
    if (verbose) puts("STUB: FigReadAheadAssertionCreate called");
    return NULL;
}

void* FigReadAheadAssertionGetTypeID(void)
{
    if (verbose) puts("STUB: FigReadAheadAssertionGetTypeID called");
    return NULL;
}

void* FigRemakerCanFileTypeSupportFormatDescription(void)
{
    if (verbose) puts("STUB: FigRemakerCanFileTypeSupportFormatDescription called");
    return NULL;
}

void* FigRemakerCanFileTypeSupportMediaType(void)
{
    if (verbose) puts("STUB: FigRemakerCanFileTypeSupportMediaType called");
    return NULL;
}

void* FigRemakerCreateBaseWithURLs(void)
{
    if (verbose) puts("STUB: FigRemakerCreateBaseWithURLs called");
    return NULL;
}

void* FigRemakerCreateWithAsset(void)
{
    if (verbose) puts("STUB: FigRemakerCreateWithAsset called");
    return NULL;
}

void* FigRemakerCreateWithURLs(void)
{
    if (verbose) puts("STUB: FigRemakerCreateWithURLs called");
    return NULL;
}

void* FigRemakerGetClassID(void)
{
    if (verbose) puts("STUB: FigRemakerGetClassID called");
    return NULL;
}

void* FigRemakerGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigRemakerGetFigBaseObject called");
    return NULL;
}

void* FigRemakerGetTypeID(void)
{
    if (verbose) puts("STUB: FigRemakerGetTypeID called");
    return NULL;
}

void* FigRemakerIsFormatDescriptionProtected(void)
{
    if (verbose) puts("STUB: FigRemakerIsFormatDescriptionProtected called");
    return NULL;
}

void* FigRenderPipelineCreateForWritingTrack(void)
{
    if (verbose) puts("STUB: FigRenderPipelineCreateForWritingTrack called");
    return NULL;
}

void* FigRenderPipelineGetClassID(void)
{
    if (verbose) puts("STUB: FigRenderPipelineGetClassID called");
    return NULL;
}

void* FigRenderPipelineGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigRenderPipelineGetFigBaseObject called");
    return NULL;
}

void* FigRenderPipelineGetTypeID(void)
{
    if (verbose) puts("STUB: FigRenderPipelineGetTypeID called");
    return NULL;
}

void* FigRewriteMetadata(void)
{
    if (verbose) puts("STUB: FigRewriteMetadata called");
    return NULL;
}

void* FigSCCFormatWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigSCCFormatWriterCreateWithByteStream called");
    return NULL;
}

void* FigSTSCreate(void)
{
    if (verbose) puts("STUB: FigSTSCreate called");
    return NULL;
}

void* FigSTSGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigSTSGetCMBaseObject called");
    return NULL;
}

void* FigSTSGetClassID(void)
{
    if (verbose) puts("STUB: FigSTSGetClassID called");
    return NULL;
}

void* FigSTSGetTypeID(void)
{
    if (verbose) puts("STUB: FigSTSGetTypeID called");
    return NULL;
}

void* FigSampleBufferAudioRendererAirPlayCreate(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererAirPlayCreate called");
    return NULL;
}

void* FigSampleBufferAudioRendererCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererCreateWithOptions called");
    return NULL;
}

void* FigSampleBufferAudioRendererGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererGetClassID called");
    return NULL;
}

void* FigSampleBufferAudioRendererGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererGetFigBaseObject called");
    return NULL;
}

void* FigSampleBufferAudioRendererGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererGetTypeID called");
    return NULL;
}

void* FigSampleBufferAudioRendererRoutingSessionCreate(void)
{
    if (verbose) puts("STUB: FigSampleBufferAudioRendererRoutingSessionCreate called");
    return NULL;
}

void* FigSampleBufferConsumerCreateForBufferQueue(void)
{
    if (verbose) puts("STUB: FigSampleBufferConsumerCreateForBufferQueue called");
    return NULL;
}

void* FigSampleBufferConsumerGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleBufferConsumerGetClassID called");
    return NULL;
}

void* FigSampleBufferConsumerGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleBufferConsumerGetFigBaseObject called");
    return NULL;
}

void* FigSampleBufferConsumerGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleBufferConsumerGetTypeID called");
    return NULL;
}

void* FigSampleBufferCreateFromFigCaptions(void)
{
    if (verbose) puts("STUB: FigSampleBufferCreateFromFigCaptions called");
    return NULL;
}

void* FigSampleBufferProcessorAcquireHardware(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorAcquireHardware called");
    return NULL;
}

void* FigSampleBufferProcessorCreateCustomProcessors(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateCustomProcessors called");
    return NULL;
}

void* FigSampleBufferProcessorCreateForCustom(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateForCustom called");
    return NULL;
}

void* FigSampleBufferProcessorCreateForMonochrome(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateForMonochrome called");
    return NULL;
}

void* FigSampleBufferProcessorCreateForVideoRotation(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateForVideoRotation called");
    return NULL;
}

void* FigSampleBufferProcessorCreateWithAudioCompression(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateWithAudioCompression called");
    return NULL;
}

void* FigSampleBufferProcessorCreateWithAudioDecompression(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateWithAudioDecompression called");
    return NULL;
}

void* FigSampleBufferProcessorCreateWithVTCompressionSession(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorCreateWithVTCompressionSession called");
    return NULL;
}

void* FigSampleBufferProcessorGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorGetClassID called");
    return NULL;
}

void* FigSampleBufferProcessorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorGetFigBaseObject called");
    return NULL;
}

void* FigSampleBufferProcessorGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorGetTypeID called");
    return NULL;
}

void* FigSampleBufferProcessorGetVTCompressionSession(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorGetVTCompressionSession called");
    return NULL;
}

void* FigSampleBufferProcessorRelinquishHardware(void)
{
    if (verbose) puts("STUB: FigSampleBufferProcessorRelinquishHardware called");
    return NULL;
}

void* FigSampleBufferProviderCreateForBufferQueue(void)
{
    if (verbose) puts("STUB: FigSampleBufferProviderCreateForBufferQueue called");
    return NULL;
}

void* FigSampleBufferProviderCreateForVisualContext(void)
{
    if (verbose) puts("STUB: FigSampleBufferProviderCreateForVisualContext called");
    return NULL;
}

void* FigSampleBufferProviderGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleBufferProviderGetClassID called");
    return NULL;
}

void* FigSampleBufferProviderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleBufferProviderGetFigBaseObject called");
    return NULL;
}

void* FigSampleBufferProviderGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleBufferProviderGetTypeID called");
    return NULL;
}

void* FigSampleBufferRenderSynchronizerCreateWithOptions(void)
{
    if (verbose) puts("STUB: FigSampleBufferRenderSynchronizerCreateWithOptions called");
    return NULL;
}

void* FigSampleBufferRenderSynchronizerGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleBufferRenderSynchronizerGetClassID called");
    return NULL;
}

void* FigSampleBufferRenderSynchronizerGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleBufferRenderSynchronizerGetTypeID called");
    return NULL;
}

void* FigSampleCursorGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleCursorGetClassID called");
    return NULL;
}

void* FigSampleCursorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleCursorGetFigBaseObject called");
    return NULL;
}

void* FigSampleCursorGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleCursorGetTypeID called");
    return NULL;
}

void* FigSampleCursorServiceGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleCursorServiceGetClassID called");
    return NULL;
}

void* FigSampleCursorServiceGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleCursorServiceGetFigBaseObject called");
    return NULL;
}

void* FigSampleCursorServiceGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleCursorServiceGetTypeID called");
    return NULL;
}

void* FigSampleGeneratorCreateForFormatReaderProducingByteReferenceSampleBuffers(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorCreateForFormatReaderProducingByteReferenceSampleBuffers called");
    return NULL;
}

void* FigSampleGeneratorCreateForFormatReaderUsingByteStreams(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorCreateForFormatReaderUsingByteStreams called");
    return NULL;
}

void* FigSampleGeneratorCreateForFormatReaderUsingScheduledIO(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorCreateForFormatReaderUsingScheduledIO called");
    return NULL;
}

void* FigSampleGeneratorCreateForTrackReaderUsingScheduledIO(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorCreateForTrackReaderUsingScheduledIO called");
    return NULL;
}

void* FigSampleGeneratorDataSourceCacheCreate(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorDataSourceCacheCreate called");
    return NULL;
}

void* FigSampleGeneratorGetClassID(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorGetClassID called");
    return NULL;
}

void* FigSampleGeneratorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorGetFigBaseObject called");
    return NULL;
}

void* FigSampleGeneratorGetTypeID(void)
{
    if (verbose) puts("STUB: FigSampleGeneratorGetTypeID called");
    return NULL;
}

void* FigSecureStopManagerCopyDefaultRuntimeInstance(void)
{
    if (verbose) puts("STUB: FigSecureStopManagerCopyDefaultRuntimeInstance called");
    return NULL;
}

void* FigSecureStopManagerCreate(void)
{
    if (verbose) puts("STUB: FigSecureStopManagerCreate called");
    return NULL;
}

void* FigShared_CopyCacheLastModifiedDate(void)
{
    if (verbose) puts("STUB: FigShared_CopyCacheLastModifiedDate called");
    return NULL;
}

void* FigShared_CopyDiskCacheCheckedInIDs(void)
{
    if (verbose) puts("STUB: FigShared_CopyDiskCacheCheckedInIDs called");
    return NULL;
}

void* FigShared_CopyDiskCacheParams(void)
{
    if (verbose) puts("STUB: FigShared_CopyDiskCacheParams called");
    return NULL;
}

void* FigShared_DeleteFromDiskCache(void)
{
    if (verbose) puts("STUB: FigShared_DeleteFromDiskCache called");
    return NULL;
}

void* FigShared_GetCacheFileSize(void)
{
    if (verbose) puts("STUB: FigShared_GetCacheFileSize called");
    return NULL;
}

void* FigShared_MakeRoomInDiskCache(void)
{
    if (verbose) puts("STUB: FigShared_MakeRoomInDiskCache called");
    return NULL;
}

void* FigShared_OneTimeInitialization(void)
{
    if (verbose) puts("STUB: FigShared_OneTimeInitialization called");
    return NULL;
}

void* FigShared_SetDiskCacheParams(void)
{
    if (verbose) puts("STUB: FigShared_SetDiskCacheParams called");
    return NULL;
}

void* FigSimpleAlternateFilterCreate(void)
{
    if (verbose) puts("STUB: FigSimpleAlternateFilterCreate called");
    return NULL;
}

void* FigSimpleRenderPipelineCreateWithCallback(void)
{
    if (verbose) puts("STUB: FigSimpleRenderPipelineCreateWithCallback called");
    return NULL;
}

void* FigStreamAssetImageGeneratorCreateFromAsset(void)
{
    if (verbose) puts("STUB: FigStreamAssetImageGeneratorCreateFromAsset called");
    return NULL;
}

void* FigStreamAssetImageGeneratorCreateFromAssetWithOptions(void)
{
    if (verbose) puts("STUB: FigStreamAssetImageGeneratorCreateFromAssetWithOptions called");
    return NULL;
}

void* FigStreamPlaylistDestroy(void)
{
    if (verbose) puts("STUB: FigStreamPlaylistDestroy called");
    return NULL;
}

void* FigStreamPlaylistParse(void)
{
    if (verbose) puts("STUB: FigStreamPlaylistParse called");
    return NULL;
}

void* FigStreamPlaylistPredictMediaSequenceAndPartForAdvance(void)
{
    if (verbose) puts("STUB: FigStreamPlaylistPredictMediaSequenceAndPartForAdvance called");
    return NULL;
}

void* FigStreamingAssetDownloaderCreateWithAsset(void)
{
    if (verbose) puts("STUB: FigStreamingAssetDownloaderCreateWithAsset called");
    return NULL;
}

void* FigStreamingAssetDownloaderCreateWithURL(void)
{
    if (verbose) puts("STUB: FigStreamingAssetDownloaderCreateWithURL called");
    return NULL;
}

void* FigStreamingCacheAddMediaMap(void)
{
    if (verbose) puts("STUB: FigStreamingCacheAddMediaMap called");
    return NULL;
}

void* FigStreamingCacheCleanupStreamsForPersisting(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCleanupStreamsForPersisting called");
    return NULL;
}

void* FigStreamingCacheCopyChapterArtwork(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyChapterArtwork called");
    return NULL;
}

void* FigStreamingCacheCopyCompletedMediaStreamURLs(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyCompletedMediaStreamURLs called");
    return NULL;
}

void* FigStreamingCacheCopyCryptKey(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyCryptKey called");
    return NULL;
}

void* FigStreamingCacheCopyMasterPlaylist(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyMasterPlaylist called");
    return NULL;
}

void* FigStreamingCacheCopyMediaMap(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyMediaMap called");
    return NULL;
}

void* FigStreamingCacheCopyProperty(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopyProperty called");
    return NULL;
}

void* FigStreamingCacheCopySessionData(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCopySessionData called");
    return NULL;
}

void* FigStreamingCacheCreate(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCreate called");
    return NULL;
}

void* FigStreamingCacheCreateMediaStream(void)
{
    if (verbose) puts("STUB: FigStreamingCacheCreateMediaStream called");
    return NULL;
}

void* FigStreamingCacheFlushIO(void)
{
    if (verbose) puts("STUB: FigStreamingCacheFlushIO called");
    return NULL;
}

void* FigStreamingCacheGetLongestContiguousMediaStreamInCacheAtTime(void)
{
    if (verbose) puts("STUB: FigStreamingCacheGetLongestContiguousMediaStreamInCacheAtTime called");
    return NULL;
}

void* FigStreamingCacheGetTypeID(void)
{
    if (verbose) puts("STUB: FigStreamingCacheGetTypeID called");
    return NULL;
}

void* FigStreamingCacheHintPlaybackTime(void)
{
    if (verbose) puts("STUB: FigStreamingCacheHintPlaybackTime called");
    return NULL;
}

void* FigStreamingCacheInvalidate(void)
{
    if (verbose) puts("STUB: FigStreamingCacheInvalidate called");
    return NULL;
}

void* FigStreamingCacheIsMediaStreamInCacheAtTime(void)
{
    if (verbose) puts("STUB: FigStreamingCacheIsMediaStreamInCacheAtTime called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentAppendBytes(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentAppendBytes called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentCopyBytes(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentCopyBytes called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentGetLength(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentGetLength called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentIsComplete(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentIsComplete called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentLock(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentLock called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentMarkComplete(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentMarkComplete called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentSetDuplicateSegments(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentSetDuplicateSegments called");
    return NULL;
}

void* FigStreamingCacheMediaSegmentUnlock(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaSegmentUnlock called");
    return NULL;
}

void* FigStreamingCacheMediaStreamCopyCachedTimeRanges(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamCopyCachedTimeRanges called");
    return NULL;
}

void* FigStreamingCacheMediaStreamCopyPersistent(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamCopyPersistent called");
    return NULL;
}

void* FigStreamingCacheMediaStreamCopyPlaylist(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamCopyPlaylist called");
    return NULL;
}

void* FigStreamingCacheMediaStreamCreateMediaSegment(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamCreateMediaSegment called");
    return NULL;
}

void* FigStreamingCacheMediaStreamGetMediaSegmentAtOffset(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamGetMediaSegmentAtOffset called");
    return NULL;
}

void* FigStreamingCacheMediaStreamGetMediaSegmentAtTime(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamGetMediaSegmentAtTime called");
    return NULL;
}

void* FigStreamingCacheMediaStreamIsComplete(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamIsComplete called");
    return NULL;
}

void* FigStreamingCacheMediaStreamMarkComplete(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamMarkComplete called");
    return NULL;
}

void* FigStreamingCacheMediaStreamReserveSpaceForByteSize(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamReserveSpaceForByteSize called");
    return NULL;
}

void* FigStreamingCacheMediaStreamSetPersistent(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamSetPersistent called");
    return NULL;
}

void* FigStreamingCacheMediaStreamSetPlaylist(void)
{
    if (verbose) puts("STUB: FigStreamingCacheMediaStreamSetPlaylist called");
    return NULL;
}

void* FigStreamingCacheRemoveMediaMap(void)
{
    if (verbose) puts("STUB: FigStreamingCacheRemoveMediaMap called");
    return NULL;
}

void* FigStreamingCacheSetCacheTimeRange(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetCacheTimeRange called");
    return NULL;
}

void* FigStreamingCacheSetChapterArtwork(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetChapterArtwork called");
    return NULL;
}

void* FigStreamingCacheSetCryptKey(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetCryptKey called");
    return NULL;
}

void* FigStreamingCacheSetMasterPlaylist(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetMasterPlaylist called");
    return NULL;
}

void* FigStreamingCacheSetProperty(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetProperty called");
    return NULL;
}

void* FigStreamingCacheSetSessionData(void)
{
    if (verbose) puts("STUB: FigStreamingCacheSetSessionData called");
    return NULL;
}

void* FigSubtitleRendererDrawSubtitleAttributedStringDirect(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererDrawSubtitleAttributedStringDirect called");
    return NULL;
}

void* FigSubtitleRendererDrawSubtitleText(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererDrawSubtitleText called");
    return NULL;
}

void* FigSubtitleRendererDrawSubtitleTextDirect(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererDrawSubtitleTextDirect called");
    return NULL;
}

void* FigSubtitleRendererLayout(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererLayout called");
    return NULL;
}

void* FigSubtitleRendererNotificationBarrier(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererNotificationBarrier called");
    return NULL;
}

void* FigSubtitleRendererUseCoreTextRendererByDefault(void)
{
    if (verbose) puts("STUB: FigSubtitleRendererUseCoreTextRendererByDefault called");
    return NULL;
}

void* FigSubtitleSampleCreateFromPropertyList(void)
{
    if (verbose) puts("STUB: FigSubtitleSampleCreateFromPropertyList called");
    return NULL;
}

void* FigSubtitleSampleCreateFromPropertyListAndInsertInfoForRenderer(void)
{
    if (verbose) puts("STUB: FigSubtitleSampleCreateFromPropertyListAndInsertInfoForRenderer called");
    return NULL;
}

void* FigSubtitleSampleCreatePropertyList(void)
{
    if (verbose) puts("STUB: FigSubtitleSampleCreatePropertyList called");
    return NULL;
}

void* FigSubtitleSampleInsertInfoForRenderer(void)
{
    if (verbose) puts("STUB: FigSubtitleSampleInsertInfoForRenderer called");
    return NULL;
}

void* FigSubtitleTextRasterizerCreate(void)
{
    if (verbose) puts("STUB: FigSubtitleTextRasterizerCreate called");
    return NULL;
}

void* FigSubtitleTextRasterizerSetSubtitleSample(void)
{
    if (verbose) puts("STUB: FigSubtitleTextRasterizerSetSubtitleSample called");
    return NULL;
}

void* FigSupportsIOSurfaceTimingInfo(void)
{
    if (verbose) puts("STUB: FigSupportsIOSurfaceTimingInfo called");
    return NULL;
}

void* FigSyncMomentSourceChangeFlagsMask(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceChangeFlagsMask called");
    return NULL;
}

void* FigSyncMomentSourceCreateWithDispatchQueue(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceCreateWithDispatchQueue called");
    return NULL;
}

void* FigSyncMomentSourceCreateWithDispatchQueueAndDestructor(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceCreateWithDispatchQueueAndDestructor called");
    return NULL;
}

void* FigSyncMomentSourceDestroy(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceDestroy called");
    return NULL;
}

void* FigSyncMomentSourceGetTypeID(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceGetTypeID called");
    return NULL;
}

void* FigSyncMomentSourceSetTimebase(void)
{
    if (verbose) puts("STUB: FigSyncMomentSourceSetTimebase called");
    return NULL;
}

void* FigTTMLAddAttributeToCFDictionary(void)
{
    if (verbose) puts("STUB: FigTTMLAddAttributeToCFDictionary called");
    return NULL;
}

void* FigTTMLAddCurrentElementAttributesToDictionary(void)
{
    if (verbose) puts("STUB: FigTTMLAddCurrentElementAttributesToDictionary called");
    return NULL;
}

void* FigTTMLAppendDebugDescToCFString(void)
{
    if (verbose) puts("STUB: FigTTMLAppendDebugDescToCFString called");
    return NULL;
}

void* FigTTMLCFStringArrayCreateWithNMTOKENS(void)
{
    if (verbose) puts("STUB: FigTTMLCFStringArrayCreateWithNMTOKENS called");
    return NULL;
}

void* FigTTMLCGColorCopyAsTTMLColorSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLCGColorCopyAsTTMLColorSyntax called");
    return NULL;
}

void* FigTTMLCGColorCreateWithColorSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLCGColorCreateWithColorSyntax called");
    return NULL;
}

void* FigTTMLCMTimeCopyAsClockTimeSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLCMTimeCopyAsClockTimeSyntax called");
    return NULL;
}

void* FigTTMLCMTimeCopyAsOffsetTimeSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLCMTimeCopyAsOffsetTimeSyntax called");
    return NULL;
}

void* FigTTMLCaptionGroupConverterFromSampleBufferCreate(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionGroupConverterFromSampleBufferCreate called");
    return NULL;
}

void* FigTTMLCaptionWriterAddCaption(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterAddCaption called");
    return NULL;
}

void* FigTTMLCaptionWriterCreateForDocument(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterCreateForDocument called");
    return NULL;
}

void* FigTTMLCaptionWriterFinish(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterFinish called");
    return NULL;
}

void* FigTTMLCaptionWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterGetClassID called");
    return NULL;
}

void* FigTTMLCaptionWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterGetTypeID called");
    return NULL;
}

void* FigTTMLCaptionWriterInvalidate(void)
{
    if (verbose) puts("STUB: FigTTMLCaptionWriterInvalidate called");
    return NULL;
}

void* FigTTMLCopyNamespaceAndLocalNameOfCurrentNode(void)
{
    if (verbose) puts("STUB: FigTTMLCopyNamespaceAndLocalNameOfCurrentNode called");
    return NULL;
}

void* FigTTMLCopyRGBAColorAsColorSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLCopyRGBAColorAsColorSyntax called");
    return NULL;
}

void* FigTTMLCopyStringWithDefaultWhitespaceHandling(void)
{
    if (verbose) puts("STUB: FigTTMLCopyStringWithDefaultWhitespaceHandling called");
    return NULL;
}

void* FigTTMLCreateReversedStringArrayFromNMTOKENS(void)
{
    if (verbose) puts("STUB: FigTTMLCreateReversedStringArrayFromNMTOKENS called");
    return NULL;
}

void* FigTTMLCreateStringArrayFromNMTOKENS(void)
{
    if (verbose) puts("STUB: FigTTMLCreateStringArrayFromNMTOKENS called");
    return NULL;
}

void* FigTTMLDivCreate(void)
{
    if (verbose) puts("STUB: FigTTMLDivCreate called");
    return NULL;
}

void* FigTTMLDocumentWriterAddCaptionData(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterAddCaptionData called");
    return NULL;
}

void* FigTTMLDocumentWriterAddText(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterAddText called");
    return NULL;
}

void* FigTTMLDocumentWriterCopyProperty(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCopyProperty called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateForSerializer(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateForSerializer called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateForiTT(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateForiTT called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateLanguageIdentifierInserter(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateLanguageIdentifierInserter called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateLineBreaker(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateLineBreaker called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateParagraphStyleAttributeInserter(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateParagraphStyleAttributeInserter called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateProfileInserter(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateProfileInserter called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateRegionBuilder(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateRegionBuilder called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateRegionStyleOptimizer(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateRegionStyleOptimizer called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateRubySpanBuilder(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateRubySpanBuilder called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateStyleOptimizer(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateStyleOptimizer called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateStyleSpanBuilder(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateStyleSpanBuilder called");
    return NULL;
}

void* FigTTMLDocumentWriterCreateWritingDirectionSpanBuilder(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterCreateWritingDirectionSpanBuilder called");
    return NULL;
}

void* FigTTMLDocumentWriterEndElement(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterEndElement called");
    return NULL;
}

void* FigTTMLDocumentWriterFlush(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterFlush called");
    return NULL;
}

void* FigTTMLDocumentWriterGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterGetCMBaseObject called");
    return NULL;
}

void* FigTTMLDocumentWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterGetClassID called");
    return NULL;
}

void* FigTTMLDocumentWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterGetTypeID called");
    return NULL;
}

void* FigTTMLDocumentWriterInvalidate(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterInvalidate called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_ActiveArea(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_ActiveArea called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_BackgroundColor(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_BackgroundColor called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_CellResolution(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_CellResolution called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_ClipOverflow(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_ClipOverflow called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Decoration(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Decoration called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_DisplayAlign(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_DisplayAlign called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_DisplayAspectRatio(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_DisplayAspectRatio called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_FillLineGap(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_FillLineGap called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_FontFamily(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_FontFamily called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_FontSize(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_FontSize called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_FontStyle(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_FontStyle called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_FontWeight(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_FontWeight called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_ForcedDisplay(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_ForcedDisplay called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Hidden(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Hidden called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Identifier(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Identifier called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Invisible(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Invisible called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_LineHeight(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_LineHeight called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_LinePadding(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_LinePadding called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_MultiRowAlign(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_MultiRowAlign called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Opacity(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Opacity called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Padding(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Padding called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_Position(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_Position called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_PreventLineWrapping(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_PreventLineWrapping called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_ShowBackground(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_ShowBackground called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_TextAlign(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_TextAlign called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_TextColor(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_TextColor called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_TextCombine(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_TextCombine called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_TextOutline(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_TextOutline called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_WritingDirection(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_WritingDirection called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_WritingMode(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_WritingMode called");
    return NULL;
}

void* FigTTMLDocumentWriterMapPropertyToAttribute_ZIndex(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapPropertyToAttribute_ZIndex called");
    return NULL;
}

void* FigTTMLDocumentWriterMapWidthAndHeightPropertiesToExtentAttributeAndSet(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterMapWidthAndHeightPropertiesToExtentAttributeAndSet called");
    return NULL;
}

void* FigTTMLDocumentWriterSetAttribute(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterSetAttribute called");
    return NULL;
}

void* FigTTMLDocumentWriterSetProperty(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterSetProperty called");
    return NULL;
}

void* FigTTMLDocumentWriterStartElement(void)
{
    if (verbose) puts("STUB: FigTTMLDocumentWriterStartElement called");
    return NULL;
}

void* FigTTMLFormatWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigTTMLFormatWriterCreateWithByteStream called");
    return NULL;
}

void* FigTTMLGetInheritableStyleAttributeNames(void)
{
    if (verbose) puts("STUB: FigTTMLGetInheritableStyleAttributeNames called");
    return NULL;
}

void* FigTTMLGetLibXMLAccess(void)
{
    if (verbose) puts("STUB: FigTTMLGetLibXMLAccess called");
    return NULL;
}

void* FigTTMLInitializeNodeBaseStorage(void)
{
    if (verbose) puts("STUB: FigTTMLInitializeNodeBaseStorage called");
    return NULL;
}

void* FigTTMLIntervalTreeAppendActiveNodesToArray(void)
{
    if (verbose) puts("STUB: FigTTMLIntervalTreeAppendActiveNodesToArray called");
    return NULL;
}

void* FigTTMLIntervalTreeCreate(void)
{
    if (verbose) puts("STUB: FigTTMLIntervalTreeCreate called");
    return NULL;
}

void* FigTTMLIntervalTreeGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLIntervalTreeGetTypeID called");
    return NULL;
}

void* FigTTMLLayoutCreate(void)
{
    if (verbose) puts("STUB: FigTTMLLayoutCreate called");
    return NULL;
}

void* FigTTMLLayoutCreateDefault(void)
{
    if (verbose) puts("STUB: FigTTMLLayoutCreateDefault called");
    return NULL;
}

void* FigTTMLMoveCurrentNodeTo(void)
{
    if (verbose) puts("STUB: FigTTMLMoveCurrentNodeTo called");
    return NULL;
}

void* FigTTMLNodeCopyAttributes(void)
{
    if (verbose) puts("STUB: FigTTMLNodeCopyAttributes called");
    return NULL;
}

void* FigTTMLNodeCopyChildNodeArray(void)
{
    if (verbose) puts("STUB: FigTTMLNodeCopyChildNodeArray called");
    return NULL;
}

void* FigTTMLNodeCopyNodeValue(void)
{
    if (verbose) puts("STUB: FigTTMLNodeCopyNodeValue called");
    return NULL;
}

void* FigTTMLNodeCopySkippedElementLocalNames(void)
{
    if (verbose) puts("STUB: FigTTMLNodeCopySkippedElementLocalNames called");
    return NULL;
}

void* FigTTMLNodeGetActiveTimeRange(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetActiveTimeRange called");
    return NULL;
}

void* FigTTMLNodeGetClassID(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetClassID called");
    return NULL;
}

void* FigTTMLNodeGetDocumentOrderIndex(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetDocumentOrderIndex called");
    return NULL;
}

void* FigTTMLNodeGetNodeType(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetNodeType called");
    return NULL;
}

void* FigTTMLNodeGetParameters(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetParameters called");
    return NULL;
}

void* FigTTMLNodeGetParentNode(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetParentNode called");
    return NULL;
}

void* FigTTMLNodeGetPrecedingNode(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetPrecedingNode called");
    return NULL;
}

void* FigTTMLNodeGetRegionAttributeValue(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetRegionAttributeValue called");
    return NULL;
}

void* FigTTMLNodeGetTimeContainerSemantics(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetTimeContainerSemantics called");
    return NULL;
}

void* FigTTMLNodeGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetTypeID called");
    return NULL;
}

void* FigTTMLNodeGetXMLID(void)
{
    if (verbose) puts("STUB: FigTTMLNodeGetXMLID called");
    return NULL;
}

void* FigTTMLNodeIsAssociatedWithRegionID(void)
{
    if (verbose) puts("STUB: FigTTMLNodeIsAssociatedWithRegionID called");
    return NULL;
}

void* FigTTMLNodeUseDefaultWhitespaceHandling(void)
{
    if (verbose) puts("STUB: FigTTMLNodeUseDefaultWhitespaceHandling called");
    return NULL;
}

void* FigTTMLParseActiveArea(void)
{
    if (verbose) puts("STUB: FigTTMLParseActiveArea called");
    return NULL;
}

void* FigTTMLParseAndCreateAnimationClassNode(void)
{
    if (verbose) puts("STUB: FigTTMLParseAndCreateAnimationClassNode called");
    return NULL;
}

void* FigTTMLParseAndCreateBlockClassNode(void)
{
    if (verbose) puts("STUB: FigTTMLParseAndCreateBlockClassNode called");
    return NULL;
}

void* FigTTMLParseAndCreateInlineClassNode(void)
{
    if (verbose) puts("STUB: FigTTMLParseAndCreateInlineClassNode called");
    return NULL;
}

void* FigTTMLParseCellResolution(void)
{
    if (verbose) puts("STUB: FigTTMLParseCellResolution called");
    return NULL;
}

void* FigTTMLParseColor(void)
{
    if (verbose) puts("STUB: FigTTMLParseColor called");
    return NULL;
}

void* FigTTMLParseFontFamilyAndCreateFontFamilyNameList(void)
{
    if (verbose) puts("STUB: FigTTMLParseFontFamilyAndCreateFontFamilyNameList called");
    return NULL;
}

void* FigTTMLParseFontFamilySyntaxAndCreateFontFamilyNameList(void)
{
    if (verbose) puts("STUB: FigTTMLParseFontFamilySyntaxAndCreateFontFamilyNameList called");
    return NULL;
}

void* FigTTMLParseFrameRate(void)
{
    if (verbose) puts("STUB: FigTTMLParseFrameRate called");
    return NULL;
}

void* FigTTMLParseFrameRateMultiplier(void)
{
    if (verbose) puts("STUB: FigTTMLParseFrameRateMultiplier called");
    return NULL;
}

void* FigTTMLParseLength(void)
{
    if (verbose) puts("STUB: FigTTMLParseLength called");
    return NULL;
}

void* FigTTMLParseLengthSyntax(void)
{
    if (verbose) puts("STUB: FigTTMLParseLengthSyntax called");
    return NULL;
}

void* FigTTMLParseNode(void)
{
    if (verbose) puts("STUB: FigTTMLParseNode called");
    return NULL;
}

void* FigTTMLParsePixelAspectRatio(void)
{
    if (verbose) puts("STUB: FigTTMLParsePixelAspectRatio called");
    return NULL;
}

void* FigTTMLParsePixelResolution(void)
{
    if (verbose) puts("STUB: FigTTMLParsePixelResolution called");
    return NULL;
}

void* FigTTMLParseTextCombineDigits(void)
{
    if (verbose) puts("STUB: FigTTMLParseTextCombineDigits called");
    return NULL;
}

void* FigTTMLParseTickRate(void)
{
    if (verbose) puts("STUB: FigTTMLParseTickRate called");
    return NULL;
}

void* FigTTMLParseTimeExpression(void)
{
    if (verbose) puts("STUB: FigTTMLParseTimeExpression called");
    return NULL;
}

void* FigTTMLReleaseNodeBaseStorage(void)
{
    if (verbose) puts("STUB: FigTTMLReleaseNodeBaseStorage called");
    return NULL;
}

void* FigTTMLSerializerAddText(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerAddText called");
    return NULL;
}

void* FigTTMLSerializerCopyProperty(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerCopyProperty called");
    return NULL;
}

void* FigTTMLSerializerCreateForByteStream(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerCreateForByteStream called");
    return NULL;
}

void* FigTTMLSerializerCreateIndenter(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerCreateIndenter called");
    return NULL;
}

void* FigTTMLSerializerEndElement(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerEndElement called");
    return NULL;
}

void* FigTTMLSerializerFlush(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerFlush called");
    return NULL;
}

void* FigTTMLSerializerGetClassID(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerGetClassID called");
    return NULL;
}

void* FigTTMLSerializerGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerGetTypeID called");
    return NULL;
}

void* FigTTMLSerializerInvalidate(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerInvalidate called");
    return NULL;
}

void* FigTTMLSerializerSetAttribute(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerSetAttribute called");
    return NULL;
}

void* FigTTMLSerializerSetProperty(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerSetProperty called");
    return NULL;
}

void* FigTTMLSerializerStartElement(void)
{
    if (verbose) puts("STUB: FigTTMLSerializerStartElement called");
    return NULL;
}

void* FigTTMLSkipNode(void)
{
    if (verbose) puts("STUB: FigTTMLSkipNode called");
    return NULL;
}

void* FigTTMLSynchronicTreeCopyCSS(void)
{
    if (verbose) puts("STUB: FigTTMLSynchronicTreeCopyCSS called");
    return NULL;
}

void* FigTTMLSynchronicTreeCopyCaptionArray(void)
{
    if (verbose) puts("STUB: FigTTMLSynchronicTreeCopyCaptionArray called");
    return NULL;
}

void* FigTTMLSynchronicTreeCopySSS(void)
{
    if (verbose) puts("STUB: FigTTMLSynchronicTreeCopySSS called");
    return NULL;
}

void* FigTTMLSynchronicTreeCopyTextNodeArray(void)
{
    if (verbose) puts("STUB: FigTTMLSynchronicTreeCopyTextNodeArray called");
    return NULL;
}

void* FigTTMLSynchronicTreeGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLSynchronicTreeGetTypeID called");
    return NULL;
}

void* FigTTMLTextCopyContent(void)
{
    if (verbose) puts("STUB: FigTTMLTextCopyContent called");
    return NULL;
}

void* FigTTMLTextCreate(void)
{
    if (verbose) puts("STUB: FigTTMLTextCreate called");
    return NULL;
}

void* FigTTMLTreeAppendActiveNodesToArray(void)
{
    if (verbose) puts("STUB: FigTTMLTreeAppendActiveNodesToArray called");
    return NULL;
}

void* FigTTMLTreeCopyActiveRegionArray(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCopyActiveRegionArray called");
    return NULL;
}

void* FigTTMLTreeCopyFeatureAndExtensionSet(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCopyFeatureAndExtensionSet called");
    return NULL;
}

void* FigTTMLTreeCopySliceTimeRangeArray(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCopySliceTimeRangeArray called");
    return NULL;
}

void* FigTTMLTreeCreate(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCreate called");
    return NULL;
}

void* FigTTMLTreeCreateSynchronicTree(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCreateSynchronicTree called");
    return NULL;
}

void* FigTTMLTreeCreateWithCString(void)
{
    if (verbose) puts("STUB: FigTTMLTreeCreateWithCString called");
    return NULL;
}

void* FigTTMLTreeGetBodyNode(void)
{
    if (verbose) puts("STUB: FigTTMLTreeGetBodyNode called");
    return NULL;
}

void* FigTTMLTreeGetNodeByID(void)
{
    if (verbose) puts("STUB: FigTTMLTreeGetNodeByID called");
    return NULL;
}

void* FigTTMLTreeGetParameters(void)
{
    if (verbose) puts("STUB: FigTTMLTreeGetParameters called");
    return NULL;
}

void* FigTTMLTreeGetRootNode(void)
{
    if (verbose) puts("STUB: FigTTMLTreeGetRootNode called");
    return NULL;
}

void* FigTTMLTreeGetTypeID(void)
{
    if (verbose) puts("STUB: FigTTMLTreeGetTypeID called");
    return NULL;
}

void* FigTTMLTreeWalkFromNode(void)
{
    if (verbose) puts("STUB: FigTTMLTreeWalkFromNode called");
    return NULL;
}

void* FigTextMarkupAttributedStringCreateFromPropertyList(void)
{
    if (verbose) puts("STUB: FigTextMarkupAttributedStringCreateFromPropertyList called");
    return NULL;
}

void* FigTextRasterizerGetTypeID(void)
{
    if (verbose) puts("STUB: FigTextRasterizerGetTypeID called");
    return NULL;
}

void* FigTextSampleBufferCopyText(void)
{
    if (verbose) puts("STUB: FigTextSampleBufferCopyText called");
    return NULL;
}

void* FigThingFind(void)
{
    if (verbose) puts("STUB: FigThingFind called");
    return NULL;
}

void* FigTimebaseGetSyncMoment(void)
{
    if (verbose) puts("STUB: FigTimebaseGetSyncMoment called");
    return NULL;
}

void* FigTimebaseSynchronizeToMoment(void)
{
    if (verbose) puts("STUB: FigTimebaseSynchronizeToMoment called");
    return NULL;
}

void* FigTimedMetadataRenderPipelineCreate(void)
{
    if (verbose) puts("STUB: FigTimedMetadataRenderPipelineCreate called");
    return NULL;
}

void* FigTrackReaderGetClassID(void)
{
    if (verbose) puts("STUB: FigTrackReaderGetClassID called");
    return NULL;
}

void* FigTrackReaderGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigTrackReaderGetFigBaseObject called");
    return NULL;
}

void* FigTrackReaderGetTypeID(void)
{
    if (verbose) puts("STUB: FigTrackReaderGetTypeID called");
    return NULL;
}

void* FigUserDataGetOSTypeKeyToStringKeyMapping(void)
{
    if (verbose) puts("STUB: FigUserDataGetOSTypeKeyToStringKeyMapping called");
    return NULL;
}

void* FigVCPCompositorHostAnticipateRendering(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostAnticipateRendering called");
    return NULL;
}

void* FigVCPCompositorHostCancelPendingFrames(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostCancelPendingFrames called");
    return NULL;
}

void* FigVCPCompositorHostComposeFrameAsync(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostComposeFrameAsync called");
    return NULL;
}

void* FigVCPCompositorHostCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPCompositorHostCopyProperty(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostCopyProperty called");
    return NULL;
}

void* FigVCPCompositorHostCreate(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostCreate called");
    return NULL;
}

void* FigVCPCompositorHostGetCompositor(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetCompositor called");
    return NULL;
}

void* FigVCPCompositorHostGetCompositorInstanceID(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetCompositorInstanceID called");
    return NULL;
}

void* FigVCPCompositorHostGetPendingFramesCount(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetPendingFramesCount called");
    return NULL;
}

void* FigVCPCompositorHostGetRenderSize(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetRenderSize called");
    return NULL;
}

void* FigVCPCompositorHostGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetTypeID called");
    return NULL;
}

void* FigVCPCompositorHostGetUpcomingImageInfo(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostGetUpcomingImageInfo called");
    return NULL;
}

void* FigVCPCompositorHostInvalidate(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostInvalidate called");
    return NULL;
}

void* FigVCPCompositorHostIsAcceptingRequest(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostIsAcceptingRequest called");
    return NULL;
}

void* FigVCPCompositorHostPrepareToCompose(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostPrepareToCompose called");
    return NULL;
}

void* FigVCPCompositorHostPrerollForRendering(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostPrerollForRendering called");
    return NULL;
}

void* FigVCPCompositorHostSetCompletePrerollCallback(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostSetCompletePrerollCallback called");
    return NULL;
}

void* FigVCPCompositorHostSetDidReachEndOfOutputCallback(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostSetDidReachEndOfOutputCallback called");
    return NULL;
}

void* FigVCPCompositorHostSetOutputCallback(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostSetOutputCallback called");
    return NULL;
}

void* FigVCPCompositorHostSetProperty(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostSetProperty called");
    return NULL;
}

void* FigVCPCompositorHostStartAcceptingRequest(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostStartAcceptingRequest called");
    return NULL;
}

void* FigVCPCompositorHostStopAcceptingRequest(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostStopAcceptingRequest called");
    return NULL;
}

void* FigVCPCompositorHostSupportsRenderScale(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostSupportsRenderScale called");
    return NULL;
}

void* FigVCPCompositorHostWaitForAllPendingFramesToComplete(void)
{
    if (verbose) puts("STUB: FigVCPCompositorHostWaitForAllPendingFramesToComplete called");
    return NULL;
}

void* FigVCPCoreAnimationPostProcessorCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationPostProcessorCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPCoreAnimationPostProcessorCopyPixelBufferAtTime(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationPostProcessorCopyPixelBufferAtTime called");
    return NULL;
}

void* FigVCPCoreAnimationPostProcessorCreate(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationPostProcessorCreate called");
    return NULL;
}

void* FigVCPCoreAnimationPostProcessorGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationPostProcessorGetTypeID called");
    return NULL;
}

void* FigVCPCoreAnimationPostProcessorThrottleForBackground(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationPostProcessorThrottleForBackground called");
    return NULL;
}

void* FigVCPCoreAnimationSourceCreate(void)
{
    if (verbose) puts("STUB: FigVCPCoreAnimationSourceCreate called");
    return NULL;
}

void* FigVCPDirectionIsBackwardOrPaused(void)
{
    if (verbose) puts("STUB: FigVCPDirectionIsBackwardOrPaused called");
    return NULL;
}

void* FigVCPDirectionIsForwardOrPaused(void)
{
    if (verbose) puts("STUB: FigVCPDirectionIsForwardOrPaused called");
    return NULL;
}

void* FigVCPGetFoldedTime(void)
{
    if (verbose) puts("STUB: FigVCPGetFoldedTime called");
    return NULL;
}

void* FigVCPGetFoldedTimeRanges(void)
{
    if (verbose) puts("STUB: FigVCPGetFoldedTimeRanges called");
    return NULL;
}

void* FigVCPInstructionSourceCopyInstructionAtTime(void)
{
    if (verbose) puts("STUB: FigVCPInstructionSourceCopyInstructionAtTime called");
    return NULL;
}

void* FigVCPInstructionSourceCreate(void)
{
    if (verbose) puts("STUB: FigVCPInstructionSourceCreate called");
    return NULL;
}

void* FigVCPInstructionSourceGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPInstructionSourceGetTypeID called");
    return NULL;
}

void* FigVCPInstructionSourceInvalidateCache(void)
{
    if (verbose) puts("STUB: FigVCPInstructionSourceInvalidateCache called");
    return NULL;
}

void* FigVCPInvalidateEpilogue(void)
{
    if (verbose) puts("STUB: FigVCPInvalidateEpilogue called");
    return NULL;
}

void* FigVCPInvalidatePrologue(void)
{
    if (verbose) puts("STUB: FigVCPInvalidatePrologue called");
    return NULL;
}

void* FigVCPIsValid(void)
{
    if (verbose) puts("STUB: FigVCPIsValid called");
    return NULL;
}

void* FigVCPOutputAllOutputsCanAccomodateFrame(void)
{
    if (verbose) puts("STUB: FigVCPOutputAllOutputsCanAccomodateFrame called");
    return NULL;
}

void* FigVCPOutputConformerCopyConformedPixelBuffer(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerCopyConformedPixelBuffer called");
    return NULL;
}

void* FigVCPOutputConformerCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPOutputConformerCopyProperty(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerCopyProperty called");
    return NULL;
}

void* FigVCPOutputConformerCreate(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerCreate called");
    return NULL;
}

void* FigVCPOutputConformerGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerGetTypeID called");
    return NULL;
}

void* FigVCPOutputConformerSetProperty(void)
{
    if (verbose) puts("STUB: FigVCPOutputConformerSetProperty called");
    return NULL;
}

void* FigVCPOutputCreate(void)
{
    if (verbose) puts("STUB: FigVCPOutputCreate called");
    return NULL;
}

void* FigVCPOutputDurationWaterLevelCreate(void)
{
    if (verbose) puts("STUB: FigVCPOutputDurationWaterLevelCreate called");
    return NULL;
}

void* FigVCPOutputDurationWaterLevelGetLowerBound(void)
{
    if (verbose) puts("STUB: FigVCPOutputDurationWaterLevelGetLowerBound called");
    return NULL;
}

void* FigVCPOutputDurationWaterLevelGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPOutputDurationWaterLevelGetTypeID called");
    return NULL;
}

void* FigVCPOutputDurationWaterLevelGetUpperBound(void)
{
    if (verbose) puts("STUB: FigVCPOutputDurationWaterLevelGetUpperBound called");
    return NULL;
}

void* FigVCPOutputEnqueueImage(void)
{
    if (verbose) puts("STUB: FigVCPOutputEnqueueImage called");
    return NULL;
}

void* FigVCPOutputFlush(void)
{
    if (verbose) puts("STUB: FigVCPOutputFlush called");
    return NULL;
}

void* FigVCPOutputGetOutputDurationWaterLevels(void)
{
    if (verbose) puts("STUB: FigVCPOutputGetOutputDurationWaterLevels called");
    return NULL;
}

void* FigVCPOutputGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPOutputGetTypeID called");
    return NULL;
}

void* FigVCPOutputInvalidate(void)
{
    if (verbose) puts("STUB: FigVCPOutputInvalidate called");
    return NULL;
}

void* FigVCPOutputSetDirection(void)
{
    if (verbose) puts("STUB: FigVCPOutputSetDirection called");
    return NULL;
}

void* FigVCPOutputSetOutputDurationHighAndLowWaterLevels(void)
{
    if (verbose) puts("STUB: FigVCPOutputSetOutputDurationHighAndLowWaterLevels called");
    return NULL;
}

void* FigVCPOutputSetUpcomingImageInfo(void)
{
    if (verbose) puts("STUB: FigVCPOutputSetUpcomingImageInfo called");
    return NULL;
}

void* FigVCPOutputWasJustFlushed(void)
{
    if (verbose) puts("STUB: FigVCPOutputWasJustFlushed called");
    return NULL;
}

void* FigVCPPendingFrameCreate(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameCreate called");
    return NULL;
}

void* FigVCPPendingFrameGetCompletionStatus(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetCompletionStatus called");
    return NULL;
}

void* FigVCPPendingFrameGetComposedFrame(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetComposedFrame called");
    return NULL;
}

void* FigVCPPendingFrameGetCompositionStartTime(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetCompositionStartTime called");
    return NULL;
}

void* FigVCPPendingFrameGetFoldedFrameTime(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetFoldedFrameTime called");
    return NULL;
}

void* FigVCPPendingFrameGetFrameTime(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetFrameTime called");
    return NULL;
}

void* FigVCPPendingFrameGetOrder(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetOrder called");
    return NULL;
}

void* FigVCPPendingFrameGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameGetTypeID called");
    return NULL;
}

void* FigVCPPendingFrameIsImmediateFrame(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameIsImmediateFrame called");
    return NULL;
}

void* FigVCPPendingFrameReleaseAndClearSourceFrames(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameReleaseAndClearSourceFrames called");
    return NULL;
}

void* FigVCPPendingFrameSkipCoreAnimationPostProcessing(void)
{
    if (verbose) puts("STUB: FigVCPPendingFrameSkipCoreAnimationPostProcessing called");
    return NULL;
}

void* FigVCPPreprocessParametersCreate(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersCreate called");
    return NULL;
}

void* FigVCPPreprocessParametersCreateCopy(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersCreateCopy called");
    return NULL;
}

void* FigVCPPreprocessParametersCreateMutable(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersCreateMutable called");
    return NULL;
}

void* FigVCPPreprocessParametersGetColorPrimaries(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetColorPrimaries called");
    return NULL;
}

void* FigVCPPreprocessParametersGetPixelBufferAttributes(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetPixelBufferAttributes called");
    return NULL;
}

void* FigVCPPreprocessParametersGetRenderScale(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetRenderScale called");
    return NULL;
}

void* FigVCPPreprocessParametersGetTransferFunction(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetTransferFunction called");
    return NULL;
}

void* FigVCPPreprocessParametersGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetTypeID called");
    return NULL;
}

void* FigVCPPreprocessParametersGetYCbCrMatrix(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersGetYCbCrMatrix called");
    return NULL;
}

void* FigVCPPreprocessParametersSetProperty(void)
{
    if (verbose) puts("STUB: FigVCPPreprocessParametersSetProperty called");
    return NULL;
}

void* FigVCPProcessingStateAdvanceCurrentTime(void)
{
    if (verbose) puts("STUB: FigVCPProcessingStateAdvanceCurrentTime called");
    return NULL;
}

void* FigVCPProcessingStateHasReachedTheEnd(void)
{
    if (verbose) puts("STUB: FigVCPProcessingStateHasReachedTheEnd called");
    return NULL;
}

void* FigVCPProcessingStateSetCurrentTimeWithSourceFrameTiming(void)
{
    if (verbose) puts("STUB: FigVCPProcessingStateSetCurrentTimeWithSourceFrameTiming called");
    return NULL;
}

void* FigVCPSourceArrayApplyFunction(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayApplyFunction called");
    return NULL;
}

void* FigVCPSourceArrayContainsCoreAnimationSource(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayContainsCoreAnimationSource called");
    return NULL;
}

void* FigVCPSourceArrayCopyAndAddNewCoreAnimationSource(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayCopyAndAddNewCoreAnimationSource called");
    return NULL;
}

void* FigVCPSourceArrayCopyAndAddNewVisualContext(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayCopyAndAddNewVisualContext called");
    return NULL;
}

void* FigVCPSourceArrayCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPSourceArrayCreateEmpty(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayCreateEmpty called");
    return NULL;
}

void* FigVCPSourceArrayGetCount(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayGetCount called");
    return NULL;
}

void* FigVCPSourceArrayGetSourceAtIndex(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayGetSourceAtIndex called");
    return NULL;
}

void* FigVCPSourceArrayGetSourceWithTrackID(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayGetSourceWithTrackID called");
    return NULL;
}

void* FigVCPSourceArrayGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayGetTypeID called");
    return NULL;
}

void* FigVCPSourceArrayNoMoreImagesOnAnySource(void)
{
    if (verbose) puts("STUB: FigVCPSourceArrayNoMoreImagesOnAnySource called");
    return NULL;
}

void* FigVCPSourceCheckImageAvailability(void)
{
    if (verbose) puts("STUB: FigVCPSourceCheckImageAvailability called");
    return NULL;
}

void* FigVCPSourceCopyConformedImage(void)
{
    if (verbose) puts("STUB: FigVCPSourceCopyConformedImage called");
    return NULL;
}

void* FigVCPSourceCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPSourceCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPSourceGetEarliestSequentialImageTimeAfterTime(void)
{
    if (verbose) puts("STUB: FigVCPSourceGetEarliestSequentialImageTimeAfterTime called");
    return NULL;
}

void* FigVCPSourceGetSourceImageAtCurrentTime(void)
{
    if (verbose) puts("STUB: FigVCPSourceGetSourceImageAtCurrentTime called");
    return NULL;
}

void* FigVCPSourceGetTrackID(void)
{
    if (verbose) puts("STUB: FigVCPSourceGetTrackID called");
    return NULL;
}

void* FigVCPSourceGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPSourceGetTypeID called");
    return NULL;
}

void* FigVCPSourceHasNoMoreImages(void)
{
    if (verbose) puts("STUB: FigVCPSourceHasNoMoreImages called");
    return NULL;
}

void* FigVCPSourceHoldTriggerUntilAvailabilityChanges(void)
{
    if (verbose) puts("STUB: FigVCPSourceHoldTriggerUntilAvailabilityChanges called");
    return NULL;
}

void* FigVCPSourceInvalidate(void)
{
    if (verbose) puts("STUB: FigVCPSourceInvalidate called");
    return NULL;
}

void* FigVCPSourceSetCurrentTime(void)
{
    if (verbose) puts("STUB: FigVCPSourceSetCurrentTime called");
    return NULL;
}

void* FigVCPSourceSetPreprocessParameters(void)
{
    if (verbose) puts("STUB: FigVCPSourceSetPreprocessParameters called");
    return NULL;
}

void* FigVCPSourceShouldTriggerWhenAvailabilityChanges(void)
{
    if (verbose) puts("STUB: FigVCPSourceShouldTriggerWhenAvailabilityChanges called");
    return NULL;
}

void* FigVCPSourceStartOver(void)
{
    if (verbose) puts("STUB: FigVCPSourceStartOver called");
    return NULL;
}

void* FigVCPSourceThrottleForBackground(void)
{
    if (verbose) puts("STUB: FigVCPSourceThrottleForBackground called");
    return NULL;
}

void* FigVCPTallyCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: FigVCPTallyCopyPerformanceDictionary called");
    return NULL;
}

void* FigVCPTallyUpdateSkips(void)
{
    if (verbose) puts("STUB: FigVCPTallyUpdateSkips called");
    return NULL;
}

void* FigVCPTallyUpdateTimingInfo(void)
{
    if (verbose) puts("STUB: FigVCPTallyUpdateTimingInfo called");
    return NULL;
}

void* FigVCPTimingCopyProcessingState(void)
{
    if (verbose) puts("STUB: FigVCPTimingCopyProcessingState called");
    return NULL;
}

void* FigVCPTimingCopyTimebase(void)
{
    if (verbose) puts("STUB: FigVCPTimingCopyTimebase called");
    return NULL;
}

void* FigVCPTimingCreate(void)
{
    if (verbose) puts("STUB: FigVCPTimingCreate called");
    return NULL;
}

void* FigVCPTimingGetLoopTimeRange(void)
{
    if (verbose) puts("STUB: FigVCPTimingGetLoopTimeRange called");
    return NULL;
}

void* FigVCPTimingGetOutputFrameDuration(void)
{
    if (verbose) puts("STUB: FigVCPTimingGetOutputFrameDuration called");
    return NULL;
}

void* FigVCPTimingGetSourceTrackIDForFrameTiming(void)
{
    if (verbose) puts("STUB: FigVCPTimingGetSourceTrackIDForFrameTiming called");
    return NULL;
}

void* FigVCPTimingGetTypeID(void)
{
    if (verbose) puts("STUB: FigVCPTimingGetTypeID called");
    return NULL;
}

void* FigVCPTimingInvalidate(void)
{
    if (verbose) puts("STUB: FigVCPTimingInvalidate called");
    return NULL;
}

void* FigVCPTimingIsProcessingStateFresh(void)
{
    if (verbose) puts("STUB: FigVCPTimingIsProcessingStateFresh called");
    return NULL;
}

void* FigVCPTimingRepeatIdenticalFrames(void)
{
    if (verbose) puts("STUB: FigVCPTimingRepeatIdenticalFrames called");
    return NULL;
}

void* FigVCPTimingResetNextFrameTimeToCurrentTimebaseTime(void)
{
    if (verbose) puts("STUB: FigVCPTimingResetNextFrameTimeToCurrentTimebaseTime called");
    return NULL;
}

void* FigVCPTimingSaveProcessingState(void)
{
    if (verbose) puts("STUB: FigVCPTimingSaveProcessingState called");
    return NULL;
}

void* FigVCPTimingSetLoopTimeRange(void)
{
    if (verbose) puts("STUB: FigVCPTimingSetLoopTimeRange called");
    return NULL;
}

void* FigVCPTimingSetTimebase(void)
{
    if (verbose) puts("STUB: FigVCPTimingSetTimebase called");
    return NULL;
}

void* FigVCPTimingStartOver(void)
{
    if (verbose) puts("STUB: FigVCPTimingStartOver called");
    return NULL;
}

void* FigVCPTimingUsesTimebase(void)
{
    if (verbose) puts("STUB: FigVCPTimingUsesTimebase called");
    return NULL;
}

void* FigVCPVisualContextSourceCreate(void)
{
    if (verbose) puts("STUB: FigVCPVisualContextSourceCreate called");
    return NULL;
}

void* FigVTTCGColorCopyAsVTTColorSyntax(void)
{
    if (verbose) puts("STUB: FigVTTCGColorCopyAsVTTColorSyntax called");
    return NULL;
}

void* FigVTTCGColorCopyVTTColorAttributeValueAsCSSColorSyntax(void)
{
    if (verbose) puts("STUB: FigVTTCGColorCopyVTTColorAttributeValueAsCSSColorSyntax called");
    return NULL;
}

void* FigVTTCaptionWriterAddCaption(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterAddCaption called");
    return NULL;
}

void* FigVTTCaptionWriterCreateForDocument(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterCreateForDocument called");
    return NULL;
}

void* FigVTTCaptionWriterFinish(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterFinish called");
    return NULL;
}

void* FigVTTCaptionWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterGetClassID called");
    return NULL;
}

void* FigVTTCaptionWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterGetTypeID called");
    return NULL;
}

void* FigVTTCaptionWriterInvalidate(void)
{
    if (verbose) puts("STUB: FigVTTCaptionWriterInvalidate called");
    return NULL;
}

void* FigVTTCopyCMTimeAsTimeStamp(void)
{
    if (verbose) puts("STUB: FigVTTCopyCMTimeAsTimeStamp called");
    return NULL;
}

void* FigVTTCopyNodeTypeName(void)
{
    if (verbose) puts("STUB: FigVTTCopyNodeTypeName called");
    return NULL;
}

void* FigVTTCopyRGBAColorAsColorSyntax(void)
{
    if (verbose) puts("STUB: FigVTTCopyRGBAColorAsColorSyntax called");
    return NULL;
}

void* FigVTTCopySectionTypeName(void)
{
    if (verbose) puts("STUB: FigVTTCopySectionTypeName called");
    return NULL;
}

void* FigVTTCueCreate(void)
{
    if (verbose) puts("STUB: FigVTTCueCreate called");
    return NULL;
}

void* FigVTTDocumentWriterCopyProperty(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterCopyProperty called");
    return NULL;
}

void* FigVTTDocumentWriterCreateAndAddNode(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterCreateAndAddNode called");
    return NULL;
}

void* FigVTTDocumentWriterCreateForVTT(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterCreateForVTT called");
    return NULL;
}

void* FigVTTDocumentWriterFlush(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterFlush called");
    return NULL;
}

void* FigVTTDocumentWriterGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterGetCMBaseObject called");
    return NULL;
}

void* FigVTTDocumentWriterGetClassID(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterGetClassID called");
    return NULL;
}

void* FigVTTDocumentWriterGetTypeID(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterGetTypeID called");
    return NULL;
}

void* FigVTTDocumentWriterInvalidate(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterInvalidate called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_BackgroundColor(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_BackgroundColor called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_Decoration(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_Decoration called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_FontFamily(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_FontFamily called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_FontSize(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_FontSize called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_FontStyle(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_FontStyle called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_FontWeight(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_FontWeight called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_TextAlign(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_TextAlign called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_TextColor(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_TextColor called");
    return NULL;
}

void* FigVTTDocumentWriterMapAttributeToCSS_WritingMode(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapAttributeToCSS_WritingMode called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_BackgroundColor(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_BackgroundColor called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_Decoration(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_Decoration called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_FontFamily(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_FontFamily called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_FontSize(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_FontSize called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_FontStyle(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_FontStyle called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_FontWeight(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_FontWeight called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_TextAlign(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_TextAlign called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_TextColor(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_TextColor called");
    return NULL;
}

void* FigVTTDocumentWriterMapPropertyToAttribute_WritingMode(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterMapPropertyToAttribute_WritingMode called");
    return NULL;
}

void* FigVTTDocumentWriterSetProperty(void)
{
    if (verbose) puts("STUB: FigVTTDocumentWriterSetProperty called");
    return NULL;
}

void* FigVTTInitializeNodeBaseStorage(void)
{
    if (verbose) puts("STUB: FigVTTInitializeNodeBaseStorage called");
    return NULL;
}

void* FigVTTMetadataCreate(void)
{
    if (verbose) puts("STUB: FigVTTMetadataCreate called");
    return NULL;
}

void* FigVTTNodeCopyAttributes(void)
{
    if (verbose) puts("STUB: FigVTTNodeCopyAttributes called");
    return NULL;
}

void* FigVTTNodeCopyChildNodeArray(void)
{
    if (verbose) puts("STUB: FigVTTNodeCopyChildNodeArray called");
    return NULL;
}

void* FigVTTNodeCopyDocumentSerialization(void)
{
    if (verbose) puts("STUB: FigVTTNodeCopyDocumentSerialization called");
    return NULL;
}

void* FigVTTNodeCopyProperty(void)
{
    if (verbose) puts("STUB: FigVTTNodeCopyProperty called");
    return NULL;
}

void* FigVTTNodeCopyValue(void)
{
    if (verbose) puts("STUB: FigVTTNodeCopyValue called");
    return NULL;
}

void* FigVTTNodeGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVTTNodeGetCMBaseObject called");
    return NULL;
}

void* FigVTTNodeGetClassID(void)
{
    if (verbose) puts("STUB: FigVTTNodeGetClassID called");
    return NULL;
}

void* FigVTTNodeGetNodeType(void)
{
    if (verbose) puts("STUB: FigVTTNodeGetNodeType called");
    return NULL;
}

void* FigVTTNodeGetTypeID(void)
{
    if (verbose) puts("STUB: FigVTTNodeGetTypeID called");
    return NULL;
}

void* FigVTTNodeSetAttribute(void)
{
    if (verbose) puts("STUB: FigVTTNodeSetAttribute called");
    return NULL;
}

void* FigVTTNodeSetChildNodeArray(void)
{
    if (verbose) puts("STUB: FigVTTNodeSetChildNodeArray called");
    return NULL;
}

void* FigVTTNodeSetProperty(void)
{
    if (verbose) puts("STUB: FigVTTNodeSetProperty called");
    return NULL;
}

void* FigVTTNodeSetValue(void)
{
    if (verbose) puts("STUB: FigVTTNodeSetValue called");
    return NULL;
}

void* FigVTTNoteBlockCreate(void)
{
    if (verbose) puts("STUB: FigVTTNoteBlockCreate called");
    return NULL;
}

void* FigVTTRegionBlockCreate(void)
{
    if (verbose) puts("STUB: FigVTTRegionBlockCreate called");
    return NULL;
}

void* FigVTTReleaseNodeBaseStorage(void)
{
    if (verbose) puts("STUB: FigVTTReleaseNodeBaseStorage called");
    return NULL;
}

void* FigVTTSerializerAddText(void)
{
    if (verbose) puts("STUB: FigVTTSerializerAddText called");
    return NULL;
}

void* FigVTTSerializerCopyProperty(void)
{
    if (verbose) puts("STUB: FigVTTSerializerCopyProperty called");
    return NULL;
}

void* FigVTTSerializerCreateForByteStream(void)
{
    if (verbose) puts("STUB: FigVTTSerializerCreateForByteStream called");
    return NULL;
}

void* FigVTTSerializerFlush(void)
{
    if (verbose) puts("STUB: FigVTTSerializerFlush called");
    return NULL;
}

void* FigVTTSerializerGetClassID(void)
{
    if (verbose) puts("STUB: FigVTTSerializerGetClassID called");
    return NULL;
}

void* FigVTTSerializerGetTypeID(void)
{
    if (verbose) puts("STUB: FigVTTSerializerGetTypeID called");
    return NULL;
}

void* FigVTTSerializerInvalidate(void)
{
    if (verbose) puts("STUB: FigVTTSerializerInvalidate called");
    return NULL;
}

void* FigVTTSerializerSetProperty(void)
{
    if (verbose) puts("STUB: FigVTTSerializerSetProperty called");
    return NULL;
}

void* FigVTTSpanCreate(void)
{
    if (verbose) puts("STUB: FigVTTSpanCreate called");
    return NULL;
}

void* FigVTTSpanCreateCopyWithValueRange(void)
{
    if (verbose) puts("STUB: FigVTTSpanCreateCopyWithValueRange called");
    return NULL;
}

void* FigVTTStringifyFigGeometryDimensionAndAppend(void)
{
    if (verbose) puts("STUB: FigVTTStringifyFigGeometryDimensionAndAppend called");
    return NULL;
}

void* FigVTTStringifyFigGeometryPointAndAppend(void)
{
    if (verbose) puts("STUB: FigVTTStringifyFigGeometryPointAndAppend called");
    return NULL;
}

void* FigVTTStyleBlockCreate(void)
{
    if (verbose) puts("STUB: FigVTTStyleBlockCreate called");
    return NULL;
}

void* FigVTTStyleCreate(void)
{
    if (verbose) puts("STUB: FigVTTStyleCreate called");
    return NULL;
}

void* FigVTTStyleGetStyleName(void)
{
    if (verbose) puts("STUB: FigVTTStyleGetStyleName called");
    return NULL;
}

void* FigValeriaGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigValeriaGetCMBaseObject called");
    return NULL;
}

void* FigValeriaGetClassID(void)
{
    if (verbose) puts("STUB: FigValeriaGetClassID called");
    return NULL;
}

void* FigValeriaGetTypeID(void)
{
    if (verbose) puts("STUB: FigValeriaGetTypeID called");
    return NULL;
}

void* FigVideoCompositionInstructionArrayIsValid(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionArrayIsValid called");
    return NULL;
}

void* FigVideoCompositionInstructionCopyRequiredTrackIDArrayForTime(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionCopyRequiredTrackIDArrayForTime called");
    return NULL;
}

void* FigVideoCompositionInstructionEvaluateTweenedAffineMatrix(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionEvaluateTweenedAffineMatrix called");
    return NULL;
}

void* FigVideoCompositionInstructionEvaluateTweenedCropRectangle(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionEvaluateTweenedCropRectangle called");
    return NULL;
}

void* FigVideoCompositionInstructionEvaluateTweenedOpacity(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionEvaluateTweenedOpacity called");
    return NULL;
}

void* FigVideoCompositionInstructionGetNormalizedTime(void)
{
    if (verbose) puts("STUB: FigVideoCompositionInstructionGetNormalizedTime called");
    return NULL;
}

void* FigVideoCompositionProcessorCopyProperty(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorCopyProperty called");
    return NULL;
}

void* FigVideoCompositionProcessorCreateWithImageQueue(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorCreateWithImageQueue called");
    return NULL;
}

void* FigVideoCompositionProcessorCreateWithImageQueueArray(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorCreateWithImageQueueArray called");
    return NULL;
}

void* FigVideoCompositionProcessorGetTypeID(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorGetTypeID called");
    return NULL;
}

void* FigVideoCompositionProcessorGetVideoCompositor(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorGetVideoCompositor called");
    return NULL;
}

void* FigVideoCompositionProcessorGo(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorGo called");
    return NULL;
}

void* FigVideoCompositionProcessorInvalidate(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorInvalidate called");
    return NULL;
}

void* FigVideoCompositionProcessorIsAtEndOfOutputData(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorIsAtEndOfOutputData called");
    return NULL;
}

void* FigVideoCompositionProcessorRequestPreroll(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorRequestPreroll called");
    return NULL;
}

void* FigVideoCompositionProcessorResetCompositionInstruction(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorResetCompositionInstruction called");
    return NULL;
}

void* FigVideoCompositionProcessorSetAnimationLayer(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetAnimationLayer called");
    return NULL;
}

void* FigVideoCompositionProcessorSetCompositionInstructionCallback(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetCompositionInstructionCallback called");
    return NULL;
}

void* FigVideoCompositionProcessorSetOutputDurationHighAndLowWaterLevels(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetOutputDurationHighAndLowWaterLevels called");
    return NULL;
}

void* FigVideoCompositionProcessorSetPlaybackTimeRange(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetPlaybackTimeRange called");
    return NULL;
}

void* FigVideoCompositionProcessorSetProperty(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetProperty called");
    return NULL;
}

void* FigVideoCompositionProcessorSetSourceVisualContext(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorSetSourceVisualContext called");
    return NULL;
}

void* FigVideoCompositionProcessorStartRenderingFramesForTimeRange(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorStartRenderingFramesForTimeRange called");
    return NULL;
}

void* FigVideoCompositionProcessorStop(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorStop called");
    return NULL;
}

void* FigVideoCompositionProcessorWaitUntilCompletelyStopped(void)
{
    if (verbose) puts("STUB: FigVideoCompositionProcessorWaitUntilCompletelyStopped called");
    return NULL;
}

void* FigVideoCompositorCreateBasic(void)
{
    if (verbose) puts("STUB: FigVideoCompositorCreateBasic called");
    return NULL;
}

void* FigVideoCompositorCreateCustom(void)
{
    if (verbose) puts("STUB: FigVideoCompositorCreateCustom called");
    return NULL;
}

void* FigVideoCompositorCreateOpenGL(void)
{
    if (verbose) puts("STUB: FigVideoCompositorCreateOpenGL called");
    return NULL;
}

void* FigVideoCompositorGetClassID(void)
{
    if (verbose) puts("STUB: FigVideoCompositorGetClassID called");
    return NULL;
}

void* FigVideoCompositorGetFigBaseObject(void)
{
    if (verbose) puts("STUB: FigVideoCompositorGetFigBaseObject called");
    return NULL;
}

void* FigVideoCompositorGetTypeID(void)
{
    if (verbose) puts("STUB: FigVideoCompositorGetTypeID called");
    return NULL;
}

void* FigVideoCompositorUtilityAreCleanAperturesEqual(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityAreCleanAperturesEqual called");
    return NULL;
}

void* FigVideoCompositorUtilityArePixelAspectRatiosEqual(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityArePixelAspectRatiosEqual called");
    return NULL;
}

void* FigVideoCompositorUtilityCopyRenderDimensionsAndEdgeProcessingPixels(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityCopyRenderDimensionsAndEdgeProcessingPixels called");
    return NULL;
}

void* FigVideoCompositorUtilityCopyRenderPixelBufferDimensionsAndCleanAperture(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityCopyRenderPixelBufferDimensionsAndCleanAperture called");
    return NULL;
}

void* FigVideoCompositorUtilityGetCleanAperture(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityGetCleanAperture called");
    return NULL;
}

void* FigVideoCompositorUtilityGetEdgeProcessingPixels(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityGetEdgeProcessingPixels called");
    return NULL;
}

void* FigVideoCompositorUtilityGetPixelAspectRatio(void)
{
    if (verbose) puts("STUB: FigVideoCompositorUtilityGetPixelAspectRatio called");
    return NULL;
}

void* FigVideoQueueCreateLocal(void)
{
    if (verbose) puts("STUB: FigVideoQueueCreateLocal called");
    return NULL;
}

void* FigVideoQueueGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVideoQueueGetCMBaseObject called");
    return NULL;
}

void* FigVideoQueueGetClassID(void)
{
    if (verbose) puts("STUB: FigVideoQueueGetClassID called");
    return NULL;
}

void* FigVideoQueueGetTypeID(void)
{
    if (verbose) puts("STUB: FigVideoQueueGetTypeID called");
    return NULL;
}

void* FigVideoRenderPipelineCreateWithFigImageQueueArray(void)
{
    if (verbose) puts("STUB: FigVideoRenderPipelineCreateWithFigImageQueueArray called");
    return NULL;
}

void* FigVideoRenderPipelineCreateWithLKImageQueue(void)
{
    if (verbose) puts("STUB: FigVideoRenderPipelineCreateWithLKImageQueue called");
    return NULL;
}

void* FigVideoRenderPipelineCreateWithVisualContext(void)
{
    if (verbose) puts("STUB: FigVideoRenderPipelineCreateWithVisualContext called");
    return NULL;
}

void* FigVirtualDisplayCopyDefaultTimestampInfo(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayCopyDefaultTimestampInfo called");
    return NULL;
}

void* FigVirtualDisplayProcessorAccessoryCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayProcessorAccessoryCreate called");
    return NULL;
}

void* FigVirtualDisplayProcessorCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayProcessorCreate called");
    return NULL;
}

void* FigVirtualDisplayProcessorGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayProcessorGetCMBaseObject called");
    return NULL;
}

void* FigVirtualDisplayProcessorGetClassID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayProcessorGetClassID called");
    return NULL;
}

void* FigVirtualDisplayProcessorGetTypeID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplayProcessorGetTypeID called");
    return NULL;
}

void* FigVirtualDisplaySessionActivate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionActivate called");
    return NULL;
}

void* FigVirtualDisplaySessionActivateWithSourceAndSink(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionActivateWithSourceAndSink called");
    return NULL;
}

void* FigVirtualDisplaySessionCreateWithComponents(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionCreateWithComponents called");
    return NULL;
}

void* FigVirtualDisplaySessionCreateWithConfiguration(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionCreateWithConfiguration called");
    return NULL;
}

void* FigVirtualDisplaySessionDeactivate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionDeactivate called");
    return NULL;
}

void* FigVirtualDisplaySessionGetClassID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionGetClassID called");
    return NULL;
}

void* FigVirtualDisplaySessionGetTypeID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionGetTypeID called");
    return NULL;
}

void* FigVirtualDisplaySessionIsIdle(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySessionIsIdle called");
    return NULL;
}

void* FigVirtualDisplaySinkConduitCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySinkConduitCreate called");
    return NULL;
}

void* FigVirtualDisplaySinkFileWriterCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySinkFileWriterCreate called");
    return NULL;
}

void* FigVirtualDisplaySinkGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySinkGetCMBaseObject called");
    return NULL;
}

void* FigVirtualDisplaySinkGetClassID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySinkGetClassID called");
    return NULL;
}

void* FigVirtualDisplaySinkGetTypeID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySinkGetTypeID called");
    return NULL;
}

void* FigVirtualDisplaySourceGetCMBaseObject(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceGetCMBaseObject called");
    return NULL;
}

void* FigVirtualDisplaySourceGetClassID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceGetClassID called");
    return NULL;
}

void* FigVirtualDisplaySourceGetTypeID(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceGetTypeID called");
    return NULL;
}

void* FigVirtualDisplaySourceMovieCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceMovieCreate called");
    return NULL;
}

void* FigVirtualDisplaySourceNullCreate(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceNullCreate called");
    return NULL;
}

void* FigVirtualDisplaySourceScreenGetCGPerformanceData(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceScreenGetCGPerformanceData called");
    return NULL;
}

void* FigVirtualDisplaySourceScreenGetCurrent(void)
{
    if (verbose) puts("STUB: FigVirtualDisplaySourceScreenGetCurrent called");
    return NULL;
}

void* FigVisualContextBecameEmpty(void)
{
    if (verbose) puts("STUB: FigVisualContextBecameEmpty called");
    return NULL;
}

void* FigVisualContextConvertHostTimeToImageTime(void)
{
    if (verbose) puts("STUB: FigVisualContextConvertHostTimeToImageTime called");
    return NULL;
}

void* FigVisualContextCopyImageForTime(void)
{
    if (verbose) puts("STUB: FigVisualContextCopyImageForTime called");
    return NULL;
}

void* FigVisualContextCopyImageForTime2(void)
{
    if (verbose) puts("STUB: FigVisualContextCopyImageForTime2 called");
    return NULL;
}

void* FigVisualContextCreateBasic(void)
{
    if (verbose) puts("STUB: FigVisualContextCreateBasic called");
    return NULL;
}

void* FigVisualContextCreateDerived(void)
{
    if (verbose) puts("STUB: FigVisualContextCreateDerived called");
    return NULL;
}

void* FigVisualContextGetEarliestSequentialImageTime(void)
{
    if (verbose) puts("STUB: FigVisualContextGetEarliestSequentialImageTime called");
    return NULL;
}

void* FigVisualContextGetEarliestSequentialImageTimeAfterTime(void)
{
    if (verbose) puts("STUB: FigVisualContextGetEarliestSequentialImageTimeAfterTime called");
    return NULL;
}

void* FigVisualContextGetTypeID(void)
{
    if (verbose) puts("STUB: FigVisualContextGetTypeID called");
    return NULL;
}

void* FigVisualContextImageAvailableImmediate(void)
{
    if (verbose) puts("STUB: FigVisualContextImageAvailableImmediate called");
    return NULL;
}

void* FigVisualContextImageAvailableSequential(void)
{
    if (verbose) puts("STUB: FigVisualContextImageAvailableSequential called");
    return NULL;
}

void* FigVisualContextInvalidate(void)
{
    if (verbose) puts("STUB: FigVisualContextInvalidate called");
    return NULL;
}

void* FigVisualContextIsNewImageAvailable(void)
{
    if (verbose) puts("STUB: FigVisualContextIsNewImageAvailable called");
    return NULL;
}

void* FigVisualContextNoMoreImages(void)
{
    if (verbose) puts("STUB: FigVisualContextNoMoreImages called");
    return NULL;
}

void* FigVisualContextRelease(void)
{
    if (verbose) puts("STUB: FigVisualContextRelease called");
    return NULL;
}

void* FigVisualContextRetain(void)
{
    if (verbose) puts("STUB: FigVisualContextRetain called");
    return NULL;
}

void* FigVisualContextSetBecameEmptyCallback(void)
{
    if (verbose) puts("STUB: FigVisualContextSetBecameEmptyCallback called");
    return NULL;
}

void* FigVisualContextSetImageAvailableImmediateCallback(void)
{
    if (verbose) puts("STUB: FigVisualContextSetImageAvailableImmediateCallback called");
    return NULL;
}

void* FigVisualContextSetImageAvailableSequentialCallback(void)
{
    if (verbose) puts("STUB: FigVisualContextSetImageAvailableSequentialCallback called");
    return NULL;
}

void* FigVisualContextSetNoMoreImagesCallback(void)
{
    if (verbose) puts("STUB: FigVisualContextSetNoMoreImagesCallback called");
    return NULL;
}

void* FigVisualContextSetProducerCallbacks(void)
{
    if (verbose) puts("STUB: FigVisualContextSetProducerCallbacks called");
    return NULL;
}

void* FigVisualContextTask(void)
{
    if (verbose) puts("STUB: FigVisualContextTask called");
    return NULL;
}

void* FigWebVTTFormatWriterCreateWithByteStream(void)
{
    if (verbose) puts("STUB: FigWebVTTFormatWriterCreateWithByteStream called");
    return NULL;
}

void* FigWriteHLSfMP4SecurityInfoIncludingAtomHeader(void)
{
    if (verbose) puts("STUB: FigWriteHLSfMP4SecurityInfoIncludingAtomHeader called");
    return NULL;
}

void* FigWriteHLSfMP4SecurityInfoPlaceholderIncludingAtomHeader(void)
{
    if (verbose) puts("STUB: FigWriteHLSfMP4SecurityInfoPlaceholderIncludingAtomHeader called");
    return NULL;
}

void* FigiTunesMetadataGetOSTypeKeyToShortStringKeyMapping(void)
{
    if (verbose) puts("STUB: FigiTunesMetadataGetOSTypeKeyToShortStringKeyMapping called");
    return NULL;
}

void* IFFParseFileTypeAtom(void)
{
    if (verbose) puts("STUB: IFFParseFileTypeAtom called");
    return NULL;
}

void* IsReferenceMovie(void)
{
    if (verbose) puts("STUB: IsReferenceMovie called");
    return NULL;
}

void* MTAudioProcessingTapCreate(void)
{
    if (verbose) puts("STUB: MTAudioProcessingTapCreate called");
    return NULL;
}

void* MTAudioProcessingTapGetSourceAudio(void)
{
    if (verbose) puts("STUB: MTAudioProcessingTapGetSourceAudio called");
    return NULL;
}

void* MTAudioProcessingTapGetStorage(void)
{
    if (verbose) puts("STUB: MTAudioProcessingTapGetStorage called");
    return NULL;
}

void* MTAudioProcessingTapGetTypeID(void)
{
    if (verbose) puts("STUB: MTAudioProcessingTapGetTypeID called");
    return NULL;
}

void* MTCapabilityIsAvailable(void)
{
    if (verbose) puts("STUB: MTCapabilityIsAvailable called");
    return NULL;
}

void* MTConvertCaptionStyleDataFromCSSToCMTextMarkup(void)
{
    if (verbose) puts("STUB: MTConvertCaptionStyleDataFromCSSToCMTextMarkup called");
    return NULL;
}

void* MTCopyImageDataForAssetAtURL(void)
{
    if (verbose) puts("STUB: MTCopyImageDataForAssetAtURL called");
    return NULL;
}

void* MTCopyLocalizedNameForMediaSubType(void)
{
    if (verbose) puts("STUB: MTCopyLocalizedNameForMediaSubType called");
    return NULL;
}

void* MTCopyLocalizedNameForMediaType(void)
{
    if (verbose) puts("STUB: MTCopyLocalizedNameForMediaType called");
    return NULL;
}

void* MTCopyLocalizedStringForVideoDynamicRange(void)
{
    if (verbose) puts("STUB: MTCopyLocalizedStringForVideoDynamicRange called");
    return NULL;
}

void* MTCopyNameForAssetAtURL(void)
{
    if (verbose) puts("STUB: MTCopyNameForAssetAtURL called");
    return NULL;
}

void* MTCopyStringForColorInfo(void)
{
    if (verbose) puts("STUB: MTCopyStringForColorInfo called");
    return NULL;
}

void* MTCopyStringsForMediaTypeAndSubType(void)
{
    if (verbose) puts("STUB: MTCopyStringsForMediaTypeAndSubType called");
    return NULL;
}

void* MTCreate3x3MatrixArrayFromCGAffineTransform(void)
{
    if (verbose) puts("STUB: MTCreate3x3MatrixArrayFromCGAffineTransform called");
    return NULL;
}

void* MTEnableCaption2015Behavior(void)
{
    if (verbose) puts("STUB: MTEnableCaption2015Behavior called");
    return NULL;
}

void* MTGetCGAffineTransformFrom3x3MatrixArray(void)
{
    if (verbose) puts("STUB: MTGetCGAffineTransformFrom3x3MatrixArray called");
    return NULL;
}

void* MTGetCaption2015Behavior(void)
{
    if (verbose) puts("STUB: MTGetCaption2015Behavior called");
    return NULL;
}

void* MTGetDefaultStreamingHDRLowPowerPreference(void)
{
    if (verbose) puts("STUB: MTGetDefaultStreamingHDRLowPowerPreference called");
    return NULL;
}

void* MTGetMaxRecommendedResolutionForTVApp(void)
{
    if (verbose) puts("STUB: MTGetMaxRecommendedResolutionForTVApp called");
    return NULL;
}

void* MTGetStreamingHDRLowPowerPreference(void)
{
    if (verbose) puts("STUB: MTGetStreamingHDRLowPowerPreference called");
    return NULL;
}

void* MTOverrideShouldPlayHDRVideo(void)
{
    if (verbose) puts("STUB: MTOverrideShouldPlayHDRVideo called");
    return NULL;
}

void* MTPluginByteSourceCopyFileName(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceCopyFileName called");
    return NULL;
}

void* MTPluginByteSourceCopyFileNamesInRelativeDirectory(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceCopyFileNamesInRelativeDirectory called");
    return NULL;
}

void* MTPluginByteSourceCopyTypeIdentifier(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceCopyTypeIdentifier called");
    return NULL;
}

void* MTPluginByteSourceCreateAuxiliaryByteSource(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceCreateAuxiliaryByteSource called");
    return NULL;
}

void* MTPluginByteSourceGetLength(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceGetLength called");
    return NULL;
}

void* MTPluginByteSourceGetTypeID(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceGetTypeID called");
    return NULL;
}

void* MTPluginByteSourceRead(void)
{
    if (verbose) puts("STUB: MTPluginByteSourceRead called");
    return NULL;
}

void* MTPluginFormatReaderCreateWithFigPluginByteSourceAndInstantiationFunction(void)
{
    if (verbose) puts("STUB: MTPluginFormatReaderCreateWithFigPluginByteSourceAndInstantiationFunction called");
    return NULL;
}

void* MTPluginFormatReaderDisableSandboxing(void)
{
    if (verbose) puts("STUB: MTPluginFormatReaderDisableSandboxing called");
    return NULL;
}

void* MTPluginFormatReaderGetCMBaseObject(void)
{
    if (verbose) puts("STUB: MTPluginFormatReaderGetCMBaseObject called");
    return NULL;
}

void* MTPluginFormatReaderGetClassID(void)
{
    if (verbose) puts("STUB: MTPluginFormatReaderGetClassID called");
    return NULL;
}

void* MTPluginFormatReaderGetTypeID(void)
{
    if (verbose) puts("STUB: MTPluginFormatReaderGetTypeID called");
    return NULL;
}

void* MTPluginFormatReader_XPCServiceConnectionHandler(void)
{
    if (verbose) puts("STUB: MTPluginFormatReader_XPCServiceConnectionHandler called");
    return NULL;
}

void* MTPluginMetadataItemCopyProperties(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemCopyProperties called");
    return NULL;
}

void* MTPluginMetadataItemCreate(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemCreate called");
    return NULL;
}

void* MTPluginMetadataItemCreateWithProperties(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemCreateWithProperties called");
    return NULL;
}

void* MTPluginMetadataItemGetDataType(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetDataType called");
    return NULL;
}

void* MTPluginMetadataItemGetExtendedLanguageTag(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetExtendedLanguageTag called");
    return NULL;
}

void* MTPluginMetadataItemGetInfo(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetInfo called");
    return NULL;
}

void* MTPluginMetadataItemGetKey(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetKey called");
    return NULL;
}

void* MTPluginMetadataItemGetTypeID(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetTypeID called");
    return NULL;
}

void* MTPluginMetadataItemGetValue(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemGetValue called");
    return NULL;
}

void* MTPluginMetadataItemSetDataType(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemSetDataType called");
    return NULL;
}

void* MTPluginMetadataItemSetExtendedLanguageTag(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemSetExtendedLanguageTag called");
    return NULL;
}

void* MTPluginMetadataItemSetInfo(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemSetInfo called");
    return NULL;
}

void* MTPluginMetadataItemSetKey(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemSetKey called");
    return NULL;
}

void* MTPluginMetadataItemSetValue(void)
{
    if (verbose) puts("STUB: MTPluginMetadataItemSetValue called");
    return NULL;
}

void* MTPluginSampleCursorGetCMBaseObject(void)
{
    if (verbose) puts("STUB: MTPluginSampleCursorGetCMBaseObject called");
    return NULL;
}

void* MTPluginSampleCursorGetClassID(void)
{
    if (verbose) puts("STUB: MTPluginSampleCursorGetClassID called");
    return NULL;
}

void* MTPluginSampleCursorGetTypeID(void)
{
    if (verbose) puts("STUB: MTPluginSampleCursorGetTypeID called");
    return NULL;
}

void* MTPluginTrackReaderGetCMBaseObject(void)
{
    if (verbose) puts("STUB: MTPluginTrackReaderGetCMBaseObject called");
    return NULL;
}

void* MTPluginTrackReaderGetClassID(void)
{
    if (verbose) puts("STUB: MTPluginTrackReaderGetClassID called");
    return NULL;
}

void* MTPluginTrackReaderGetTypeID(void)
{
    if (verbose) puts("STUB: MTPluginTrackReaderGetTypeID called");
    return NULL;
}

void* MTRegisterPluginFormatReaderBundleDirectory(void)
{
    if (verbose) puts("STUB: MTRegisterPluginFormatReaderBundleDirectory called");
    return NULL;
}

void* MTRegisterProfessionalVideoWorkflowFormatReaders(void)
{
    if (verbose) puts("STUB: MTRegisterProfessionalVideoWorkflowFormatReaders called");
    return NULL;
}

void* MTSetStreamingHDRLowPowerPreference(void)
{
    if (verbose) puts("STUB: MTSetStreamingHDRLowPowerPreference called");
    return NULL;
}

void* MTShouldPlayHDRVideo(void)
{
    if (verbose) puts("STUB: MTShouldPlayHDRVideo called");
    return NULL;
}

void* MT_GetShouldPlayHDRVideoNotificationSingleton(void)
{
    if (verbose) puts("STUB: MT_GetShouldPlayHDRVideoNotificationSingleton called");
    return NULL;
}

void* MovieProxyDataCreateForByteStream(void)
{
    if (verbose) puts("STUB: MovieProxyDataCreateForByteStream called");
    return NULL;
}

void* MovieUsesFragments(void)
{
    if (verbose) puts("STUB: MovieUsesFragments called");
    return NULL;
}

void* ObtainCFDataSinfsFromMedia(void)
{
    if (verbose) puts("STUB: ObtainCFDataSinfsFromMedia called");
    return NULL;
}

void* ObtainSinfInfoFromMedia(void)
{
    if (verbose) puts("STUB: ObtainSinfInfoFromMedia called");
    return NULL;
}

void* PKDCopyCryptKeyIdentifiersFromPSSH(void)
{
    if (verbose) puts("STUB: PKDCopyCryptKeyIdentifiersFromPSSH called");
    return NULL;
}

void* PKDCopyDecryptFormatTypeFromCodecType(void)
{
    if (verbose) puts("STUB: PKDCopyDecryptFormatTypeFromCodecType called");
    return NULL;
}

void* PKDCopyDefaultKeyIDFromSinfArray(void)
{
    if (verbose) puts("STUB: PKDCopyDefaultKeyIDFromSinfArray called");
    return NULL;
}

void* PKDCopyKeyIDAndAssetIDFromPSSH(void)
{
    if (verbose) puts("STUB: PKDCopyKeyIDAndAssetIDFromPSSH called");
    return NULL;
}

void* PKDPersistentKeyIsForCTRModeDecryption(void)
{
    if (verbose) puts("STUB: PKDPersistentKeyIsForCTRModeDecryption called");
    return NULL;
}

void* PostProcessPurchasedItem(void)
{
    if (verbose) puts("STUB: PostProcessPurchasedItem called");
    return NULL;
}

void* QTMovieFormatReaderCreateFromStream(void)
{
    if (verbose) puts("STUB: QTMovieFormatReaderCreateFromStream called");
    return NULL;
}

void* RTMCopyGeneralMetrics(void)
{
    if (verbose) puts("STUB: RTMCopyGeneralMetrics called");
    return NULL;
}

void* RTMFreeRecording(void)
{
    if (verbose) puts("STUB: RTMFreeRecording called");
    return NULL;
}

void* RTMReadResultsFromFile(void)
{
    if (verbose) puts("STUB: RTMReadResultsFromFile called");
    return NULL;
}

void* RTMStartGeneralRecording(void)
{
    if (verbose) puts("STUB: RTMStartGeneralRecording called");
    return NULL;
}

void* RTMWriteResultsToFile(void)
{
    if (verbose) puts("STUB: RTMWriteResultsToFile called");
    return NULL;
}

void* ResolveReferenceMovie(void)
{
    if (verbose) puts("STUB: ResolveReferenceMovie called");
    return NULL;
}

void* ResolveReferenceMovieWithOptions(void)
{
    if (verbose) puts("STUB: ResolveReferenceMovieWithOptions called");
    return NULL;
}

void* SlimVideoDecoder_CreateInstance(void)
{
    if (verbose) puts("STUB: SlimVideoDecoder_CreateInstance called");
    return NULL;
}

void* SlimVideoEncoder_CreateInstance(void)
{
    if (verbose) puts("STUB: SlimVideoEncoder_CreateInstance called");
    return NULL;
}

void* TTMLParserCreate(void)
{
    if (verbose) puts("STUB: TTMLParserCreate called");
    return NULL;
}

void* TTMLParserGetTypeID(void)
{
    if (verbose) puts("STUB: TTMLParserGetTypeID called");
    return NULL;
}

void* TTMLParserParseStream(void)
{
    if (verbose) puts("STUB: TTMLParserParseStream called");
    return NULL;
}

void* TTMLParserRegisterReportingCallbacks(void)
{
    if (verbose) puts("STUB: TTMLParserRegisterReportingCallbacks called");
    return NULL;
}

void* VMC2CopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: VMC2CopyPerformanceDictionary called");
    return NULL;
}

void* VMC2CopyProperty(void)
{
    if (verbose) puts("STUB: VMC2CopyProperty called");
    return NULL;
}

void* VMC2CreateWithCallbacks(void)
{
    if (verbose) puts("STUB: VMC2CreateWithCallbacks called");
    return NULL;
}

void* VMC2CreateWithQueues(void)
{
    if (verbose) puts("STUB: VMC2CreateWithQueues called");
    return NULL;
}

void* VMC2CreateWithQueues2(void)
{
    if (verbose) puts("STUB: VMC2CreateWithQueues2 called");
    return NULL;
}

void* VMC2GetTypeID(void)
{
    if (verbose) puts("STUB: VMC2GetTypeID called");
    return NULL;
}

void* VMC2Go(void)
{
    if (verbose) puts("STUB: VMC2Go called");
    return NULL;
}

void* VMC2Invalidate(void)
{
    if (verbose) puts("STUB: VMC2Invalidate called");
    return NULL;
}

void* VMC2IsAtEndOfOutputData(void)
{
    if (verbose) puts("STUB: VMC2IsAtEndOfOutputData called");
    return NULL;
}

void* VMC2MakeOutputCallbacksForImageQueueArray(void)
{
    if (verbose) puts("STUB: VMC2MakeOutputCallbacksForImageQueueArray called");
    return NULL;
}

void* VMC2PretendOutputIsLow(void)
{
    if (verbose) puts("STUB: VMC2PretendOutputIsLow called");
    return NULL;
}

void* VMC2RequestDecodeForPreroll(void)
{
    if (verbose) puts("STUB: VMC2RequestDecodeForPreroll called");
    return NULL;
}

void* VMC2Reset(void)
{
    if (verbose) puts("STUB: VMC2Reset called");
    return NULL;
}

void* VMC2SetMinUpcomingPresentationTimeStamp(void)
{
    if (verbose) puts("STUB: VMC2SetMinUpcomingPresentationTimeStamp called");
    return NULL;
}

void* VMC2SetProperty(void)
{
    if (verbose) puts("STUB: VMC2SetProperty called");
    return NULL;
}

void* VMC2SetTimebase(void)
{
    if (verbose) puts("STUB: VMC2SetTimebase called");
    return NULL;
}

void* VMC2Stop(void)
{
    if (verbose) puts("STUB: VMC2Stop called");
    return NULL;
}

void* VMC2WaitUntilCompletelyStopped(void)
{
    if (verbose) puts("STUB: VMC2WaitUntilCompletelyStopped called");
    return NULL;
}

void* VMC2WaitUntilPrerolled(void)
{
    if (verbose) puts("STUB: VMC2WaitUntilPrerolled called");
    return NULL;
}

void* VideoMentorCopyPerformanceDictionary(void)
{
    if (verbose) puts("STUB: VideoMentorCopyPerformanceDictionary called");
    return NULL;
}

void* VideoMentorDispose(void)
{
    if (verbose) puts("STUB: VideoMentorDispose called");
    return NULL;
}

void* VideoMentorInvalidate(void)
{
    if (verbose) puts("STUB: VideoMentorInvalidate called");
    return NULL;
}

void* VideoMentorNew(void)
{
    if (verbose) puts("STUB: VideoMentorNew called");
    return NULL;
}

void* VideoMentorSetClientPID(void)
{
    if (verbose) puts("STUB: VideoMentorSetClientPID called");
    return NULL;
}

void* VideoMentorSetModeToDoNothing(void)
{
    if (verbose) puts("STUB: VideoMentorSetModeToDoNothing called");
    return NULL;
}

void* VideoMentorSetModeToEmptyEdit(void)
{
    if (verbose) puts("STUB: VideoMentorSetModeToEmptyEdit called");
    return NULL;
}

void* VideoMentorSetModeToForwardPlayback(void)
{
    if (verbose) puts("STUB: VideoMentorSetModeToForwardPlayback called");
    return NULL;
}

void* VideoMentorSetModeToReversePlayback(void)
{
    if (verbose) puts("STUB: VideoMentorSetModeToReversePlayback called");
    return NULL;
}

void* VideoMentorSetModeToScrub(void)
{
    if (verbose) puts("STUB: VideoMentorSetModeToScrub called");
    return NULL;
}

void* VideoMentorSetProperty(void)
{
    if (verbose) puts("STUB: VideoMentorSetProperty called");
    return NULL;
}

void* VideoMentorSetThrottleForBackground(void)
{
    if (verbose) puts("STUB: VideoMentorSetThrottleForBackground called");
    return NULL;
}

void* VideoMentorSetTimeRemapCallback(void)
{
    if (verbose) puts("STUB: VideoMentorSetTimeRemapCallback called");
    return NULL;
}

void* fpSupport_createDefaultGaplessDictionary(void)
{
    if (verbose) puts("STUB: fpSupport_createDefaultGaplessDictionary called");
    return NULL;
}
