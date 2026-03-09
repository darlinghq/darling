/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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


#include <QuickTime/QuickTime.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AbortPrePrerollMovie(void)
{
    if (verbose) puts("STUB: AbortPrePrerollMovie called");
    return NULL;
}

void* AddCallBackToTimeBase(void)
{
    if (verbose) puts("STUB: AddCallBackToTimeBase called");
    return NULL;
}

void* AddClonedTrackToMovie(void)
{
    if (verbose) puts("STUB: AddClonedTrackToMovie called");
    return NULL;
}

void* AddEmptyTrackToMovie(void)
{
    if (verbose) puts("STUB: AddEmptyTrackToMovie called");
    return NULL;
}

void* AddFilePreview(void)
{
    if (verbose) puts("STUB: AddFilePreview called");
    return NULL;
}

void* AddImageDescriptionExtension(void)
{
    if (verbose) puts("STUB: AddImageDescriptionExtension called");
    return NULL;
}

void* AddMediaDataRef(void)
{
    if (verbose) puts("STUB: AddMediaDataRef called");
    return NULL;
}

void* AddMediaSample(void)
{
    if (verbose) puts("STUB: AddMediaSample called");
    return NULL;
}

void* AddMediaSample2(void)
{
    if (verbose) puts("STUB: AddMediaSample2 called");
    return NULL;
}

void* AddMediaSampleFromEncodedFrame(void)
{
    if (verbose) puts("STUB: AddMediaSampleFromEncodedFrame called");
    return NULL;
}

void* AddMediaSampleReference(void)
{
    if (verbose) puts("STUB: AddMediaSampleReference called");
    return NULL;
}

void* AddMediaSampleReferences(void)
{
    if (verbose) puts("STUB: AddMediaSampleReferences called");
    return NULL;
}

void* AddMediaSampleReferences64(void)
{
    if (verbose) puts("STUB: AddMediaSampleReferences64 called");
    return NULL;
}

void* AddMovieExecuteWiredActionsProc(void)
{
    if (verbose) puts("STUB: AddMovieExecuteWiredActionsProc called");
    return NULL;
}

void* AddMovieResource(void)
{
    if (verbose) puts("STUB: AddMovieResource called");
    return NULL;
}

void* AddMovieSelection(void)
{
    if (verbose) puts("STUB: AddMovieSelection called");
    return NULL;
}

void* AddMovieToStorage(void)
{
    if (verbose) puts("STUB: AddMovieToStorage called");
    return NULL;
}

void* AddSampleTableEntriesToMedia(void)
{
    if (verbose) puts("STUB: AddSampleTableEntriesToMedia called");
    return NULL;
}

void* AddSampleTableToMedia(void)
{
    if (verbose) puts("STUB: AddSampleTableToMedia called");
    return NULL;
}

void* AddSoundDescriptionExtension(void)
{
    if (verbose) puts("STUB: AddSoundDescriptionExtension called");
    return NULL;
}

void* AddTime(void)
{
    if (verbose) puts("STUB: AddTime called");
    return NULL;
}

void* AddTrackReference(void)
{
    if (verbose) puts("STUB: AddTrackReference called");
    return NULL;
}

void* AddUserData(void)
{
    if (verbose) puts("STUB: AddUserData called");
    return NULL;
}

void* AddUserDataText(void)
{
    if (verbose) puts("STUB: AddUserDataText called");
    return NULL;
}

void* AdjustMediaDisplayEndTimeToDecodeDuration(void)
{
    if (verbose) puts("STUB: AdjustMediaDisplayEndTimeToDecodeDuration called");
    return NULL;
}

void* AlignMatrix(void)
{
    if (verbose) puts("STUB: AlignMatrix called");
    return NULL;
}

void* AlignScreenRect(void)
{
    if (verbose) puts("STUB: AlignScreenRect called");
    return NULL;
}

void* AlignWindow(void)
{
    if (verbose) puts("STUB: AlignWindow called");
    return NULL;
}

void* AttachMovieToCurrentThread(void)
{
    if (verbose) puts("STUB: AttachMovieToCurrentThread called");
    return NULL;
}

void* AttachTimeBaseToCurrentThread(void)
{
    if (verbose) puts("STUB: AttachTimeBaseToCurrentThread called");
    return NULL;
}

void* AudioContextAddProcessor(void)
{
    if (verbose) puts("STUB: AudioContextAddProcessor called");
    return NULL;
}

void* AudioContextCreateChildContext(void)
{
    if (verbose) puts("STUB: AudioContextCreateChildContext called");
    return NULL;
}

void* AudioContextDeferredStart(void)
{
    if (verbose) puts("STUB: AudioContextDeferredStart called");
    return NULL;
}

void* AudioContextDispose(void)
{
    if (verbose) puts("STUB: AudioContextDispose called");
    return NULL;
}

void* AudioContextFinishStopping(void)
{
    if (verbose) puts("STUB: AudioContextFinishStopping called");
    return NULL;
}

void* AudioContextGetFormat(void)
{
    if (verbose) puts("STUB: AudioContextGetFormat called");
    return NULL;
}

void* AudioContextGetParentContextCount(void)
{
    if (verbose) puts("STUB: AudioContextGetParentContextCount called");
    return NULL;
}

void* AudioContextGetParentContextWithIndex(void)
{
    if (verbose) puts("STUB: AudioContextGetParentContextWithIndex called");
    return NULL;
}

void* AudioContextGetProcessorCount(void)
{
    if (verbose) puts("STUB: AudioContextGetProcessorCount called");
    return NULL;
}

void* AudioContextGetProcessorWithIndex(void)
{
    if (verbose) puts("STUB: AudioContextGetProcessorWithIndex called");
    return NULL;
}

void* AudioContextGetRootContextCount(void)
{
    if (verbose) puts("STUB: AudioContextGetRootContextCount called");
    return NULL;
}

void* AudioContextGetRootContextWithIndex(void)
{
    if (verbose) puts("STUB: AudioContextGetRootContextWithIndex called");
    return NULL;
}

void* AudioContextGetSupplier(void)
{
    if (verbose) puts("STUB: AudioContextGetSupplier called");
    return NULL;
}

void* AudioContextInitialize(void)
{
    if (verbose) puts("STUB: AudioContextInitialize called");
    return NULL;
}

void* AudioContextPrivateChannelProperty(void)
{
    if (verbose) puts("STUB: AudioContextPrivateChannelProperty called");
    return NULL;
}

void* AudioContextPrivateComponentMessage(void)
{
    if (verbose) puts("STUB: AudioContextPrivateComponentMessage called");
    return NULL;
}

void* AudioContextPrivateGetChannel(void)
{
    if (verbose) puts("STUB: AudioContextPrivateGetChannel called");
    return NULL;
}

void* AudioContextPrivateGetProcessor(void)
{
    if (verbose) puts("STUB: AudioContextPrivateGetProcessor called");
    return NULL;
}

void* AudioContextPrivateInitializeContext(void)
{
    if (verbose) puts("STUB: AudioContextPrivateInitializeContext called");
    return NULL;
}

void* AudioContextPrivateRelease(void)
{
    if (verbose) puts("STUB: AudioContextPrivateRelease called");
    return NULL;
}

void* AudioContextPrivateRetain(void)
{
    if (verbose) puts("STUB: AudioContextPrivateRetain called");
    return NULL;
}

void* AudioContextPrivateUninitializeContext(void)
{
    if (verbose) puts("STUB: AudioContextPrivateUninitializeContext called");
    return NULL;
}

void* AudioContextProtectedReset(void)
{
    if (verbose) puts("STUB: AudioContextProtectedReset called");
    return NULL;
}

void* AudioContextRelease(void)
{
    if (verbose) puts("STUB: AudioContextRelease called");
    return NULL;
}

void* AudioContextRemoveProcessor(void)
{
    if (verbose) puts("STUB: AudioContextRemoveProcessor called");
    return NULL;
}

void* AudioContextReset(void)
{
    if (verbose) puts("STUB: AudioContextReset called");
    return NULL;
}

void* AudioContextRetain(void)
{
    if (verbose) puts("STUB: AudioContextRetain called");
    return NULL;
}

void* AudioContextSendMessage(void)
{
    if (verbose) puts("STUB: AudioContextSendMessage called");
    return NULL;
}

void* AudioContextSetFormat(void)
{
    if (verbose) puts("STUB: AudioContextSetFormat called");
    return NULL;
}

void* AudioContextSetSupplier(void)
{
    if (verbose) puts("STUB: AudioContextSetSupplier called");
    return NULL;
}

void* AudioContextStart(void)
{
    if (verbose) puts("STUB: AudioContextStart called");
    return NULL;
}

void* AudioContextStop(void)
{
    if (verbose) puts("STUB: AudioContextStop called");
    return NULL;
}

void* AudioContextTask(void)
{
    if (verbose) puts("STUB: AudioContextTask called");
    return NULL;
}

void* AudioContextUninitialize(void)
{
    if (verbose) puts("STUB: AudioContextUninitialize called");
    return NULL;
}

void* AudioEngineCreate(void)
{
    if (verbose) puts("STUB: AudioEngineCreate called");
    return NULL;
}

void* AudioEngineCreateContext(void)
{
    if (verbose) puts("STUB: AudioEngineCreateContext called");
    return NULL;
}

void* AudioEngineCreateContextWithDeviceDescription(void)
{
    if (verbose) puts("STUB: AudioEngineCreateContextWithDeviceDescription called");
    return NULL;
}

void* AudioEngineCreateContextWithPlist(void)
{
    if (verbose) puts("STUB: AudioEngineCreateContextWithPlist called");
    return NULL;
}

void* AudioEngineDispose(void)
{
    if (verbose) puts("STUB: AudioEngineDispose called");
    return NULL;
}

void* AudioEngineInitialize(void)
{
    if (verbose) puts("STUB: AudioEngineInitialize called");
    return NULL;
}

void* AudioEngineRelease(void)
{
    if (verbose) puts("STUB: AudioEngineRelease called");
    return NULL;
}

void* AudioEngineRetain(void)
{
    if (verbose) puts("STUB: AudioEngineRetain called");
    return NULL;
}

void* AudioEngineUninitialize(void)
{
    if (verbose) puts("STUB: AudioEngineUninitialize called");
    return NULL;
}

void* AuthHGetAuthenticationInfo(void)
{
    if (verbose) puts("STUB: AuthHGetAuthenticationInfo called");
    return NULL;
}

void* AuthHSetAuthenticationInfo(void)
{
    if (verbose) puts("STUB: AuthHSetAuthenticationInfo called");
    return NULL;
}

void* BMPCompressorSetEmitHeader(void)
{
    if (verbose) puts("STUB: BMPCompressorSetEmitHeader called");
    return NULL;
}

void* BeginFullScreen(void)
{
    if (verbose) puts("STUB: BeginFullScreen called");
    return NULL;
}

void* BeginMediaEdits(void)
{
    if (verbose) puts("STUB: BeginMediaEdits called");
    return NULL;
}

void* BlendBlitterSetBlendValue(void)
{
    if (verbose) puts("STUB: BlendBlitterSetBlendValue called");
    return NULL;
}

void* BlitterPreflight(void)
{
    if (verbose) puts("STUB: BlitterPreflight called");
    return NULL;
}

void* BlitterRenderFrame(void)
{
    if (verbose) puts("STUB: BlitterRenderFrame called");
    return NULL;
}

void* BlitterSetDest(void)
{
    if (verbose) puts("STUB: BlitterSetDest called");
    return NULL;
}

void* BlitterSetSource(void)
{
    if (verbose) puts("STUB: BlitterSetSource called");
    return NULL;
}

void* BumpMediaSeed(void)
{
    if (verbose) puts("STUB: BumpMediaSeed called");
    return NULL;
}

void* BumpTrackSeed(void)
{
    if (verbose) puts("STUB: BumpTrackSeed called");
    return NULL;
}

void* CDSequenceBusy(void)
{
    if (verbose) puts("STUB: CDSequenceBusy called");
    return NULL;
}

void* CDSequenceChangedSourceData(void)
{
    if (verbose) puts("STUB: CDSequenceChangedSourceData called");
    return NULL;
}

void* CDSequenceDisposeDataSource(void)
{
    if (verbose) puts("STUB: CDSequenceDisposeDataSource called");
    return NULL;
}

void* CDSequenceDisposeMemory(void)
{
    if (verbose) puts("STUB: CDSequenceDisposeMemory called");
    return NULL;
}

void* CDSequenceEnd(void)
{
    if (verbose) puts("STUB: CDSequenceEnd called");
    return NULL;
}

void* CDSequenceEquivalentImageDescription(void)
{
    if (verbose) puts("STUB: CDSequenceEquivalentImageDescription called");
    return NULL;
}

void* CDSequenceEquivalentImageDescriptionS(void)
{
    if (verbose) puts("STUB: CDSequenceEquivalentImageDescriptionS called");
    return NULL;
}

void* CDSequenceFlush(void)
{
    if (verbose) puts("STUB: CDSequenceFlush called");
    return NULL;
}

void* CDSequenceGetDataSource(void)
{
    if (verbose) puts("STUB: CDSequenceGetDataSource called");
    return NULL;
}

void* CDSequenceInvalidate(void)
{
    if (verbose) puts("STUB: CDSequenceInvalidate called");
    return NULL;
}

void* CDSequenceNewDataSource(void)
{
    if (verbose) puts("STUB: CDSequenceNewDataSource called");
    return NULL;
}

void* CDSequenceNewMemory(void)
{
    if (verbose) puts("STUB: CDSequenceNewMemory called");
    return NULL;
}

void* CDSequenceSetSourceData(void)
{
    if (verbose) puts("STUB: CDSequenceSetSourceData called");
    return NULL;
}

void* CDSequenceSetSourceDataQueue(void)
{
    if (verbose) puts("STUB: CDSequenceSetSourceDataQueue called");
    return NULL;
}

void* CDSequenceSetTimeBase(void)
{
    if (verbose) puts("STUB: CDSequenceSetTimeBase called");
    return NULL;
}

void* CGDataImportGetInfo(void)
{
    if (verbose) puts("STUB: CGDataImportGetInfo called");
    return NULL;
}

void* CGDataImportGetMimeTypes(void)
{
    if (verbose) puts("STUB: CGDataImportGetMimeTypes called");
    return NULL;
}

void* CGDataImportNewAtomContainerFromData(void)
{
    if (verbose) puts("STUB: CGDataImportNewAtomContainerFromData called");
    return NULL;
}

void* CGDataImportSetInfo(void)
{
    if (verbose) puts("STUB: CGDataImportSetInfo called");
    return NULL;
}

void* CGFileCacheClear(void)
{
    if (verbose) puts("STUB: CGFileCacheClear called");
    return NULL;
}

void* CGFileCacheCloseFile(void)
{
    if (verbose) puts("STUB: CGFileCacheCloseFile called");
    return NULL;
}

void* CGFileCacheFlush(void)
{
    if (verbose) puts("STUB: CGFileCacheFlush called");
    return NULL;
}

void* CGFileCacheGetData(void)
{
    if (verbose) puts("STUB: CGFileCacheGetData called");
    return NULL;
}

void* CGFileCacheGetDataRef(void)
{
    if (verbose) puts("STUB: CGFileCacheGetDataRef called");
    return NULL;
}

void* CGFileCacheGetFileSize(void)
{
    if (verbose) puts("STUB: CGFileCacheGetFileSize called");
    return NULL;
}

void* CGFileCacheGetFileSpec(void)
{
    if (verbose) puts("STUB: CGFileCacheGetFileSpec called");
    return NULL;
}

void* CGFileCacheGetLastModified(void)
{
    if (verbose) puts("STUB: CGFileCacheGetLastModified called");
    return NULL;
}

void* CGFileCacheGetMIMEType(void)
{
    if (verbose) puts("STUB: CGFileCacheGetMIMEType called");
    return NULL;
}

void* CGFileCacheGetMacOSFileType(void)
{
    if (verbose) puts("STUB: CGFileCacheGetMacOSFileType called");
    return NULL;
}

void* CGFileCacheInitialize(void)
{
    if (verbose) puts("STUB: CGFileCacheInitialize called");
    return NULL;
}

void* CGFileCacheIsCached(void)
{
    if (verbose) puts("STUB: CGFileCacheIsCached called");
    return NULL;
}

void* CGFileCacheOpenForRead(void)
{
    if (verbose) puts("STUB: CGFileCacheOpenForRead called");
    return NULL;
}

void* CGFileCacheOpenForWrite(void)
{
    if (verbose) puts("STUB: CGFileCacheOpenForWrite called");
    return NULL;
}

void* CGFileCachePutData(void)
{
    if (verbose) puts("STUB: CGFileCachePutData called");
    return NULL;
}

void* CGFileCacheRemoveFile(void)
{
    if (verbose) puts("STUB: CGFileCacheRemoveFile called");
    return NULL;
}

void* CGFileCacheSetDataRef(void)
{
    if (verbose) puts("STUB: CGFileCacheSetDataRef called");
    return NULL;
}

void* CGFileCacheSetLastModified(void)
{
    if (verbose) puts("STUB: CGFileCacheSetLastModified called");
    return NULL;
}

void* CGFileCacheSetMIMEType(void)
{
    if (verbose) puts("STUB: CGFileCacheSetMIMEType called");
    return NULL;
}

void* CGFileCacheSetMacOSFileType(void)
{
    if (verbose) puts("STUB: CGFileCacheSetMacOSFileType called");
    return NULL;
}

void* CGSubscriptionGetInfo(void)
{
    if (verbose) puts("STUB: CGSubscriptionGetInfo called");
    return NULL;
}

void* CGSubscriptionGetNotificationProc(void)
{
    if (verbose) puts("STUB: CGSubscriptionGetNotificationProc called");
    return NULL;
}

void* CGSubscriptionGetSettings(void)
{
    if (verbose) puts("STUB: CGSubscriptionGetSettings called");
    return NULL;
}

void* CGSubscriptionIdle(void)
{
    if (verbose) puts("STUB: CGSubscriptionIdle called");
    return NULL;
}

void* CGSubscriptionInitialize(void)
{
    if (verbose) puts("STUB: CGSubscriptionInitialize called");
    return NULL;
}

void* CGSubscriptionSetInfo(void)
{
    if (verbose) puts("STUB: CGSubscriptionSetInfo called");
    return NULL;
}

void* CGSubscriptionSetNotificationProc(void)
{
    if (verbose) puts("STUB: CGSubscriptionSetNotificationProc called");
    return NULL;
}

void* CGSubscriptionSetSettings(void)
{
    if (verbose) puts("STUB: CGSubscriptionSetSettings called");
    return NULL;
}

void* CGSubscriptionUpdate(void)
{
    if (verbose) puts("STUB: CGSubscriptionUpdate called");
    return NULL;
}

void* CallComponentExecuteWiredAction(void)
{
    if (verbose) puts("STUB: CallComponentExecuteWiredAction called");
    return NULL;
}

void* CallMeWhen(void)
{
    if (verbose) puts("STUB: CallMeWhen called");
    return NULL;
}

void* CallQTProgressProc(void)
{
    if (verbose) puts("STUB: CallQTProgressProc called");
    return NULL;
}

void* CanQuickTimeOpenDataRef(void)
{
    if (verbose) puts("STUB: CanQuickTimeOpenDataRef called");
    return NULL;
}

void* CanQuickTimeOpenFile(void)
{
    if (verbose) puts("STUB: CanQuickTimeOpenFile called");
    return NULL;
}

void* CancelCallBack(void)
{
    if (verbose) puts("STUB: CancelCallBack called");
    return NULL;
}

void* CapsLockDown(void)
{
    if (verbose) puts("STUB: CapsLockDown called");
    return NULL;
}

void* ChannelGuideAddSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideAddSubscription called");
    return NULL;
}

void* ChannelGuideBeginSubscriptionEdits(void)
{
    if (verbose) puts("STUB: ChannelGuideBeginSubscriptionEdits called");
    return NULL;
}

void* ChannelGuideBeginSubscriptionIterator(void)
{
    if (verbose) puts("STUB: ChannelGuideBeginSubscriptionIterator called");
    return NULL;
}

void* ChannelGuideChangedSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideChangedSubscription called");
    return NULL;
}

void* ChannelGuideEndSubscriptionEdits(void)
{
    if (verbose) puts("STUB: ChannelGuideEndSubscriptionEdits called");
    return NULL;
}

void* ChannelGuideEndSubscriptionIterator(void)
{
    if (verbose) puts("STUB: ChannelGuideEndSubscriptionIterator called");
    return NULL;
}

void* ChannelGuideGetNextSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideGetNextSubscription called");
    return NULL;
}

void* ChannelGuideGetSubscriptionAtom(void)
{
    if (verbose) puts("STUB: ChannelGuideGetSubscriptionAtom called");
    return NULL;
}

void* ChannelGuideGetSubscriptionCount(void)
{
    if (verbose) puts("STUB: ChannelGuideGetSubscriptionCount called");
    return NULL;
}

void* ChannelGuideGetSubscriptionFlags(void)
{
    if (verbose) puts("STUB: ChannelGuideGetSubscriptionFlags called");
    return NULL;
}

void* ChannelGuideHasSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideHasSubscription called");
    return NULL;
}

void* ChannelGuideIdle(void)
{
    if (verbose) puts("STUB: ChannelGuideIdle called");
    return NULL;
}

void* ChannelGuideInitialize(void)
{
    if (verbose) puts("STUB: ChannelGuideInitialize called");
    return NULL;
}

void* ChannelGuideReleaseSubscriptionAtom(void)
{
    if (verbose) puts("STUB: ChannelGuideReleaseSubscriptionAtom called");
    return NULL;
}

void* ChannelGuideRemoveSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideRemoveSubscription called");
    return NULL;
}

void* ChannelGuideSetSubscriptionDataRef(void)
{
    if (verbose) puts("STUB: ChannelGuideSetSubscriptionDataRef called");
    return NULL;
}

void* ChannelGuideSetSubscriptionFlags(void)
{
    if (verbose) puts("STUB: ChannelGuideSetSubscriptionFlags called");
    return NULL;
}

void* ChannelGuideSetSubscriptionIndex(void)
{
    if (verbose) puts("STUB: ChannelGuideSetSubscriptionIndex called");
    return NULL;
}

void* ChannelGuideSetSubscriptionPosition(void)
{
    if (verbose) puts("STUB: ChannelGuideSetSubscriptionPosition called");
    return NULL;
}

void* ChannelGuideSetSubscriptionTitle(void)
{
    if (verbose) puts("STUB: ChannelGuideSetSubscriptionTitle called");
    return NULL;
}

void* ChannelGuideUpdateDefaults(void)
{
    if (verbose) puts("STUB: ChannelGuideUpdateDefaults called");
    return NULL;
}

void* ChannelGuideUpdateSubscription(void)
{
    if (verbose) puts("STUB: ChannelGuideUpdateSubscription called");
    return NULL;
}

void* CheckQuickTimeRegistration(void)
{
    if (verbose) puts("STUB: CheckQuickTimeRegistration called");
    return NULL;
}

void* CheckQuickTimeVersionRegistration(void)
{
    if (verbose) puts("STUB: CheckQuickTimeVersionRegistration called");
    return NULL;
}

void* ChooseMovieClock(void)
{
    if (verbose) puts("STUB: ChooseMovieClock called");
    return NULL;
}

void* ChromaBlitterSetKeyColor(void)
{
    if (verbose) puts("STUB: ChromaBlitterSetKeyColor called");
    return NULL;
}

void* ClearMovieChanged(void)
{
    if (verbose) puts("STUB: ClearMovieChanged called");
    return NULL;
}

void* ClearMovieSelection(void)
{
    if (verbose) puts("STUB: ClearMovieSelection called");
    return NULL;
}

void* ClearMoviesStickyError(void)
{
    if (verbose) puts("STUB: ClearMoviesStickyError called");
    return NULL;
}

void* ClockCallMeWhen(void)
{
    if (verbose) puts("STUB: ClockCallMeWhen called");
    return NULL;
}

void* ClockCancelCallBack(void)
{
    if (verbose) puts("STUB: ClockCancelCallBack called");
    return NULL;
}

void* ClockDisposeCallBack(void)
{
    if (verbose) puts("STUB: ClockDisposeCallBack called");
    return NULL;
}

void* ClockGetFastDispatchInfo(void)
{
    if (verbose) puts("STUB: ClockGetFastDispatchInfo called");
    return NULL;
}

void* ClockGetRate(void)
{
    if (verbose) puts("STUB: ClockGetRate called");
    return NULL;
}

void* ClockGetRateChangeConstraints(void)
{
    if (verbose) puts("STUB: ClockGetRateChangeConstraints called");
    return NULL;
}

void* ClockGetSampleForHostTime(void)
{
    if (verbose) puts("STUB: ClockGetSampleForHostTime called");
    return NULL;
}

void* ClockGetTime(void)
{
    if (verbose) puts("STUB: ClockGetTime called");
    return NULL;
}

void* ClockGetTimesForRateChange(void)
{
    if (verbose) puts("STUB: ClockGetTimesForRateChange called");
    return NULL;
}

void* ClockNewCallBack(void)
{
    if (verbose) puts("STUB: ClockNewCallBack called");
    return NULL;
}

void* ClockRateChanged(void)
{
    if (verbose) puts("STUB: ClockRateChanged called");
    return NULL;
}

void* ClockSetAudioDeviceUID(void)
{
    if (verbose) puts("STUB: ClockSetAudioDeviceUID called");
    return NULL;
}

void* ClockSetSampleOffset(void)
{
    if (verbose) puts("STUB: ClockSetSampleOffset called");
    return NULL;
}

void* ClockSetSampleOffsetAndLatency(void)
{
    if (verbose) puts("STUB: ClockSetSampleOffsetAndLatency called");
    return NULL;
}

void* ClockSetTimeBase(void)
{
    if (verbose) puts("STUB: ClockSetTimeBase called");
    return NULL;
}

void* ClockStartStopChanged(void)
{
    if (verbose) puts("STUB: ClockStartStopChanged called");
    return NULL;
}

void* ClockTimeChanged(void)
{
    if (verbose) puts("STUB: ClockTimeChanged called");
    return NULL;
}

void* ClockUpdateSoundSamples64(void)
{
    if (verbose) puts("STUB: ClockUpdateSoundSamples64 called");
    return NULL;
}

void* ClockUpdateSoundSamplesAndTimeStamp(void)
{
    if (verbose) puts("STUB: ClockUpdateSoundSamplesAndTimeStamp called");
    return NULL;
}

void* CloneRgn(void)
{
    if (verbose) puts("STUB: CloneRgn called");
    return NULL;
}

void* CloseMovieFile(void)
{
    if (verbose) puts("STUB: CloseMovieFile called");
    return NULL;
}

void* CloseMovieStorage(void)
{
    if (verbose) puts("STUB: CloseMovieStorage called");
    return NULL;
}

void* ClosedCaptionGetCCData(void)
{
    if (verbose) puts("STUB: ClosedCaptionGetCCData called");
    return NULL;
}

void* ClosedCaptionGetRawText(void)
{
    if (verbose) puts("STUB: ClosedCaptionGetRawText called");
    return NULL;
}

void* ClosedCaptionGetText(void)
{
    if (verbose) puts("STUB: ClosedCaptionGetText called");
    return NULL;
}

void* ClosedCaptionGetTimeBase(void)
{
    if (verbose) puts("STUB: ClosedCaptionGetTimeBase called");
    return NULL;
}

void* ClosedCaptionIdle(void)
{
    if (verbose) puts("STUB: ClosedCaptionIdle called");
    return NULL;
}

void* ClosedCaptionPause(void)
{
    if (verbose) puts("STUB: ClosedCaptionPause called");
    return NULL;
}

void* ClosedCaptionPrepare(void)
{
    if (verbose) puts("STUB: ClosedCaptionPrepare called");
    return NULL;
}

void* ClosedCaptionRelease(void)
{
    if (verbose) puts("STUB: ClosedCaptionRelease called");
    return NULL;
}

void* ClosedCaptionResetCaption(void)
{
    if (verbose) puts("STUB: ClosedCaptionResetCaption called");
    return NULL;
}

void* ClosedCaptionSetTimeBase(void)
{
    if (verbose) puts("STUB: ClosedCaptionSetTimeBase called");
    return NULL;
}

void* ClosedCaptionStop(void)
{
    if (verbose) puts("STUB: ClosedCaptionStop called");
    return NULL;
}

void* CodecCopyBits(void)
{
    if (verbose) puts("STUB: CodecCopyBits called");
    return NULL;
}

void* CodecManagerVersion(void)
{
    if (verbose) puts("STUB: CodecManagerVersion called");
    return NULL;
}

void* CommandDown(void)
{
    if (verbose) puts("STUB: CommandDown called");
    return NULL;
}

void* CompAdd(void)
{
    if (verbose) puts("STUB: CompAdd called");
    return NULL;
}

void* CompCompare(void)
{
    if (verbose) puts("STUB: CompCompare called");
    return NULL;
}

void* CompDiv(void)
{
    if (verbose) puts("STUB: CompDiv called");
    return NULL;
}

void* CompFixMul(void)
{
    if (verbose) puts("STUB: CompFixMul called");
    return NULL;
}

void* CompMul(void)
{
    if (verbose) puts("STUB: CompMul called");
    return NULL;
}

void* CompMulDiv(void)
{
    if (verbose) puts("STUB: CompMulDiv called");
    return NULL;
}

void* CompMulDivTrunc(void)
{
    if (verbose) puts("STUB: CompMulDivTrunc called");
    return NULL;
}

void* CompNeg(void)
{
    if (verbose) puts("STUB: CompNeg called");
    return NULL;
}

void* CompShift(void)
{
    if (verbose) puts("STUB: CompShift called");
    return NULL;
}

void* CompSquareRoot(void)
{
    if (verbose) puts("STUB: CompSquareRoot called");
    return NULL;
}

void* CompSub(void)
{
    if (verbose) puts("STUB: CompSub called");
    return NULL;
}

void* ComplexTransformRgn(void)
{
    if (verbose) puts("STUB: ComplexTransformRgn called");
    return NULL;
}

void* ComponentMessageCoerceValueToType(void)
{
    if (verbose) puts("STUB: ComponentMessageCoerceValueToType called");
    return NULL;
}

void* ComponentMessageGetSpatialProperty(void)
{
    if (verbose) puts("STUB: ComponentMessageGetSpatialProperty called");
    return NULL;
}

void* ComponentMessageSetSpatialProperty(void)
{
    if (verbose) puts("STUB: ComponentMessageSetSpatialProperty called");
    return NULL;
}

void* CompressImage(void)
{
    if (verbose) puts("STUB: CompressImage called");
    return NULL;
}

void* CompressPicture(void)
{
    if (verbose) puts("STUB: CompressPicture called");
    return NULL;
}

void* CompressPictureFile(void)
{
    if (verbose) puts("STUB: CompressPictureFile called");
    return NULL;
}

void* CompressSequenceBegin(void)
{
    if (verbose) puts("STUB: CompressSequenceBegin called");
    return NULL;
}

void* CompressSequenceFrame(void)
{
    if (verbose) puts("STUB: CompressSequenceFrame called");
    return NULL;
}

void* ConcatMatrix(void)
{
    if (verbose) puts("STUB: ConcatMatrix called");
    return NULL;
}

void* ControlDown(void)
{
    if (verbose) puts("STUB: ControlDown called");
    return NULL;
}

void* ConvertDataRefToMovieDataRef(void)
{
    if (verbose) puts("STUB: ConvertDataRefToMovieDataRef called");
    return NULL;
}

void* ConvertFileToMovieFile(void)
{
    if (verbose) puts("STUB: ConvertFileToMovieFile called");
    return NULL;
}

void* ConvertImage(void)
{
    if (verbose) puts("STUB: ConvertImage called");
    return NULL;
}

void* ConvertMovieToDataRef(void)
{
    if (verbose) puts("STUB: ConvertMovieToDataRef called");
    return NULL;
}

void* ConvertMovieToFile(void)
{
    if (verbose) puts("STUB: ConvertMovieToFile called");
    return NULL;
}

void* ConvertTime(void)
{
    if (verbose) puts("STUB: ConvertTime called");
    return NULL;
}

void* ConvertTimeAtRate1(void)
{
    if (verbose) puts("STUB: ConvertTimeAtRate1 called");
    return NULL;
}

void* ConvertTimeScale(void)
{
    if (verbose) puts("STUB: ConvertTimeScale called");
    return NULL;
}

void* ConvertTimeToClockTime(void)
{
    if (verbose) puts("STUB: ConvertTimeToClockTime called");
    return NULL;
}

void* CookieHGetCookie(void)
{
    if (verbose) puts("STUB: CookieHGetCookie called");
    return NULL;
}

void* CookieHSetCookie(void)
{
    if (verbose) puts("STUB: CookieHSetCookie called");
    return NULL;
}

void* CoolCopyRgn(void)
{
    if (verbose) puts("STUB: CoolCopyRgn called");
    return NULL;
}

void* CopyComponentInterfaceDictionary(void)
{
    if (verbose) puts("STUB: CopyComponentInterfaceDictionary called");
    return NULL;
}

void* CopyHandleData(void)
{
    if (verbose) puts("STUB: CopyHandleData called");
    return NULL;
}

void* CopyMatrix(void)
{
    if (verbose) puts("STUB: CopyMatrix called");
    return NULL;
}

void* CopyMediaMutableSampleTable(void)
{
    if (verbose) puts("STUB: CopyMediaMutableSampleTable called");
    return NULL;
}

void* CopyMediaUserData(void)
{
    if (verbose) puts("STUB: CopyMediaUserData called");
    return NULL;
}

void* CopyMovieSelection(void)
{
    if (verbose) puts("STUB: CopyMovieSelection called");
    return NULL;
}

void* CopyMovieSettings(void)
{
    if (verbose) puts("STUB: CopyMovieSettings called");
    return NULL;
}

void* CopyMovieUserData(void)
{
    if (verbose) puts("STUB: CopyMovieUserData called");
    return NULL;
}

void* CopyMutableSampleTableFromMediaEntries(void)
{
    if (verbose) puts("STUB: CopyMutableSampleTableFromMediaEntries called");
    return NULL;
}

void* CopyTrackSettings(void)
{
    if (verbose) puts("STUB: CopyTrackSettings called");
    return NULL;
}

void* CopyTrackUserData(void)
{
    if (verbose) puts("STUB: CopyTrackUserData called");
    return NULL;
}

void* CopyUserData(void)
{
    if (verbose) puts("STUB: CopyUserData called");
    return NULL;
}

void* CountImageDescriptionExtensionType(void)
{
    if (verbose) puts("STUB: CountImageDescriptionExtensionType called");
    return NULL;
}

void* CountUserDataType(void)
{
    if (verbose) puts("STUB: CountUserDataType called");
    return NULL;
}

void* CreateComponentInterfaceDictionaryFromXMLData(void)
{
    if (verbose) puts("STUB: CreateComponentInterfaceDictionaryFromXMLData called");
    return NULL;
}

void* CreateComponentMessage(void)
{
    if (verbose) puts("STUB: CreateComponentMessage called");
    return NULL;
}

void* CreateMovieControl(void)
{
    if (verbose) puts("STUB: CreateMovieControl called");
    return NULL;
}

void* CreateMovieFile(void)
{
    if (verbose) puts("STUB: CreateMovieFile called");
    return NULL;
}

void* CreateMovieFileUnicode(void)
{
    if (verbose) puts("STUB: CreateMovieFileUnicode called");
    return NULL;
}

void* CreateMovieFile_priv(void)
{
    if (verbose) puts("STUB: CreateMovieFile_priv called");
    return NULL;
}

void* CreateMovieStorage(void)
{
    if (verbose) puts("STUB: CreateMovieStorage called");
    return NULL;
}

void* CreateQuickTimeErrorDialog(void)
{
    if (verbose) puts("STUB: CreateQuickTimeErrorDialog called");
    return NULL;
}

void* CreateShortcutMovieFile(void)
{
    if (verbose) puts("STUB: CreateShortcutMovieFile called");
    return NULL;
}

void* CurveAddAtomToVectorStream(void)
{
    if (verbose) puts("STUB: CurveAddAtomToVectorStream called");
    return NULL;
}

void* CurveAddPathAtomToVectorStream(void)
{
    if (verbose) puts("STUB: CurveAddPathAtomToVectorStream called");
    return NULL;
}

void* CurveAddZeroAtomToVectorStream(void)
{
    if (verbose) puts("STUB: CurveAddZeroAtomToVectorStream called");
    return NULL;
}

void* CurveCountPointsInPath(void)
{
    if (verbose) puts("STUB: CurveCountPointsInPath called");
    return NULL;
}

void* CurveCreateVectorStream(void)
{
    if (verbose) puts("STUB: CurveCreateVectorStream called");
    return NULL;
}

void* CurveDrawPath(void)
{
    if (verbose) puts("STUB: CurveDrawPath called");
    return NULL;
}

void* CurveDrawPathSetup(void)
{
    if (verbose) puts("STUB: CurveDrawPathSetup called");
    return NULL;
}

void* CurveDrawPathTeardown(void)
{
    if (verbose) puts("STUB: CurveDrawPathTeardown called");
    return NULL;
}

void* CurveFindFirstXY(void)
{
    if (verbose) puts("STUB: CurveFindFirstXY called");
    return NULL;
}

void* CurveGetAtomDataFromVectorStream(void)
{
    if (verbose) puts("STUB: CurveGetAtomDataFromVectorStream called");
    return NULL;
}

void* CurveGetLength(void)
{
    if (verbose) puts("STUB: CurveGetLength called");
    return NULL;
}

void* CurveGetNearestPathPoint(void)
{
    if (verbose) puts("STUB: CurveGetNearestPathPoint called");
    return NULL;
}

void* CurveGetPathPoint(void)
{
    if (verbose) puts("STUB: CurveGetPathPoint called");
    return NULL;
}

void* CurveInsertPointIntoPath(void)
{
    if (verbose) puts("STUB: CurveInsertPointIntoPath called");
    return NULL;
}

void* CurveLengthToPoint(void)
{
    if (verbose) puts("STUB: CurveLengthToPoint called");
    return NULL;
}

void* CurveNewPath(void)
{
    if (verbose) puts("STUB: CurveNewPath called");
    return NULL;
}

void* CurvePathPointToLength(void)
{
    if (verbose) puts("STUB: CurvePathPointToLength called");
    return NULL;
}

void* CurveSetPathPoint(void)
{
    if (verbose) puts("STUB: CurveSetPathPoint called");
    return NULL;
}

void* CutMovieSelection(void)
{
    if (verbose) puts("STUB: CutMovieSelection called");
    return NULL;
}

void* DataCodecBeginInterruptSafe(void)
{
    if (verbose) puts("STUB: DataCodecBeginInterruptSafe called");
    return NULL;
}

void* DataCodecCompress(void)
{
    if (verbose) puts("STUB: DataCodecCompress called");
    return NULL;
}

void* DataCodecCompressPartial(void)
{
    if (verbose) puts("STUB: DataCodecCompressPartial called");
    return NULL;
}

void* DataCodecDecompress(void)
{
    if (verbose) puts("STUB: DataCodecDecompress called");
    return NULL;
}

void* DataCodecDecompressPartial(void)
{
    if (verbose) puts("STUB: DataCodecDecompressPartial called");
    return NULL;
}

void* DataCodecEndInterruptSafe(void)
{
    if (verbose) puts("STUB: DataCodecEndInterruptSafe called");
    return NULL;
}

void* DataCodecGetCompressBufferSize(void)
{
    if (verbose) puts("STUB: DataCodecGetCompressBufferSize called");
    return NULL;
}

void* DataHAddMovie(void)
{
    if (verbose) puts("STUB: DataHAddMovie called");
    return NULL;
}

void* DataHAppend64(void)
{
    if (verbose) puts("STUB: DataHAppend64 called");
    return NULL;
}

void* DataHCanUseDataRef(void)
{
    if (verbose) puts("STUB: DataHCanUseDataRef called");
    return NULL;
}

void* DataHCloseForRead(void)
{
    if (verbose) puts("STUB: DataHCloseForRead called");
    return NULL;
}

void* DataHCloseForWrite(void)
{
    if (verbose) puts("STUB: DataHCloseForWrite called");
    return NULL;
}

void* DataHCompareDataRef(void)
{
    if (verbose) puts("STUB: DataHCompareDataRef called");
    return NULL;
}

void* DataHCreateFile(void)
{
    if (verbose) puts("STUB: DataHCreateFile called");
    return NULL;
}

void* DataHCreateFileWithFlags(void)
{
    if (verbose) puts("STUB: DataHCreateFileWithFlags called");
    return NULL;
}

void* DataHDeleteFile(void)
{
    if (verbose) puts("STUB: DataHDeleteFile called");
    return NULL;
}

void* DataHDoesBuffer(void)
{
    if (verbose) puts("STUB: DataHDoesBuffer called");
    return NULL;
}

void* DataHExchangeFiles(void)
{
    if (verbose) puts("STUB: DataHExchangeFiles called");
    return NULL;
}

void* DataHFinishData(void)
{
    if (verbose) puts("STUB: DataHFinishData called");
    return NULL;
}

void* DataHFlushCache(void)
{
    if (verbose) puts("STUB: DataHFlushCache called");
    return NULL;
}

void* DataHFlushData(void)
{
    if (verbose) puts("STUB: DataHFlushData called");
    return NULL;
}

void* DataHFrameAdvanceMovie(void)
{
    if (verbose) puts("STUB: DataHFrameAdvanceMovie called");
    return NULL;
}

void* DataHFrameRewindMovie(void)
{
    if (verbose) puts("STUB: DataHFrameRewindMovie called");
    return NULL;
}

void* DataHGetAvailableFileSize(void)
{
    if (verbose) puts("STUB: DataHGetAvailableFileSize called");
    return NULL;
}

void* DataHGetAvailableFileSize64(void)
{
    if (verbose) puts("STUB: DataHGetAvailableFileSize64 called");
    return NULL;
}

void* DataHGetBufferSpecs(void)
{
    if (verbose) puts("STUB: DataHGetBufferSpecs called");
    return NULL;
}

void* DataHGetCacheSizeLimit(void)
{
    if (verbose) puts("STUB: DataHGetCacheSizeLimit called");
    return NULL;
}

void* DataHGetChapterList(void)
{
    if (verbose) puts("STUB: DataHGetChapterList called");
    return NULL;
}

void* DataHGetData(void)
{
    if (verbose) puts("STUB: DataHGetData called");
    return NULL;
}

void* DataHGetDataAvailability(void)
{
    if (verbose) puts("STUB: DataHGetDataAvailability called");
    return NULL;
}

void* DataHGetDataAvailability64(void)
{
    if (verbose) puts("STUB: DataHGetDataAvailability64 called");
    return NULL;
}

void* DataHGetDataInBuffer(void)
{
    if (verbose) puts("STUB: DataHGetDataInBuffer called");
    return NULL;
}

void* DataHGetDataRate(void)
{
    if (verbose) puts("STUB: DataHGetDataRate called");
    return NULL;
}

void* DataHGetDataRef(void)
{
    if (verbose) puts("STUB: DataHGetDataRef called");
    return NULL;
}

void* DataHGetDataRefAsType(void)
{
    if (verbose) puts("STUB: DataHGetDataRefAsType called");
    return NULL;
}

void* DataHGetDataRefExtension(void)
{
    if (verbose) puts("STUB: DataHGetDataRefExtension called");
    return NULL;
}

void* DataHGetDataRefWithAnchor(void)
{
    if (verbose) puts("STUB: DataHGetDataRefWithAnchor called");
    return NULL;
}

void* DataHGetDeviceIndex(void)
{
    if (verbose) puts("STUB: DataHGetDeviceIndex called");
    return NULL;
}

void* DataHGetFileName(void)
{
    if (verbose) puts("STUB: DataHGetFileName called");
    return NULL;
}

void* DataHGetFileNameCFString(void)
{
    if (verbose) puts("STUB: DataHGetFileNameCFString called");
    return NULL;
}

void* DataHGetFileSize(void)
{
    if (verbose) puts("STUB: DataHGetFileSize called");
    return NULL;
}

void* DataHGetFileSize64(void)
{
    if (verbose) puts("STUB: DataHGetFileSize64 called");
    return NULL;
}

void* DataHGetFileSizeAsync(void)
{
    if (verbose) puts("STUB: DataHGetFileSizeAsync called");
    return NULL;
}

void* DataHGetFileTypeOrdering(void)
{
    if (verbose) puts("STUB: DataHGetFileTypeOrdering called");
    return NULL;
}

void* DataHGetFreeSpace(void)
{
    if (verbose) puts("STUB: DataHGetFreeSpace called");
    return NULL;
}

void* DataHGetFreeSpace64(void)
{
    if (verbose) puts("STUB: DataHGetFreeSpace64 called");
    return NULL;
}

void* DataHGetInfo(void)
{
    if (verbose) puts("STUB: DataHGetInfo called");
    return NULL;
}

void* DataHGetInfoFlags(void)
{
    if (verbose) puts("STUB: DataHGetInfoFlags called");
    return NULL;
}

void* DataHGetLastModified(void)
{
    if (verbose) puts("STUB: DataHGetLastModified called");
    return NULL;
}

void* DataHGetMIMEType(void)
{
    if (verbose) puts("STUB: DataHGetMIMEType called");
    return NULL;
}

void* DataHGetMIMETypeAsync(void)
{
    if (verbose) puts("STUB: DataHGetMIMETypeAsync called");
    return NULL;
}

void* DataHGetMacOSFileType(void)
{
    if (verbose) puts("STUB: DataHGetMacOSFileType called");
    return NULL;
}

void* DataHGetMovie(void)
{
    if (verbose) puts("STUB: DataHGetMovie called");
    return NULL;
}

void* DataHGetMovieWithFlags(void)
{
    if (verbose) puts("STUB: DataHGetMovieWithFlags called");
    return NULL;
}

void* DataHGetPreferredBlockSize(void)
{
    if (verbose) puts("STUB: DataHGetPreferredBlockSize called");
    return NULL;
}

void* DataHGetScheduleAheadTime(void)
{
    if (verbose) puts("STUB: DataHGetScheduleAheadTime called");
    return NULL;
}

void* DataHGetTemporaryDataRefCapabilities(void)
{
    if (verbose) puts("STUB: DataHGetTemporaryDataRefCapabilities called");
    return NULL;
}

void* DataHGetVolumeList(void)
{
    if (verbose) puts("STUB: DataHGetVolumeList called");
    return NULL;
}

void* DataHInit(void)
{
    if (verbose) puts("STUB: DataHInit called");
    return NULL;
}

void* DataHIsStreamingDataHandler(void)
{
    if (verbose) puts("STUB: DataHIsStreamingDataHandler called");
    return NULL;
}

void* DataHLoadIntoRam(void)
{
    if (verbose) puts("STUB: DataHLoadIntoRam called");
    return NULL;
}

void* DataHMoveFile(void)
{
    if (verbose) puts("STUB: DataHMoveFile called");
    return NULL;
}

void* DataHOpenForRead(void)
{
    if (verbose) puts("STUB: DataHOpenForRead called");
    return NULL;
}

void* DataHOpenForWrite(void)
{
    if (verbose) puts("STUB: DataHOpenForWrite called");
    return NULL;
}

void* DataHPlaybackHints(void)
{
    if (verbose) puts("STUB: DataHPlaybackHints called");
    return NULL;
}

void* DataHPlaybackHints64(void)
{
    if (verbose) puts("STUB: DataHPlaybackHints64 called");
    return NULL;
}

void* DataHPlaybackTimeHints(void)
{
    if (verbose) puts("STUB: DataHPlaybackTimeHints called");
    return NULL;
}

void* DataHPollRead(void)
{
    if (verbose) puts("STUB: DataHPollRead called");
    return NULL;
}

void* DataHPreallocate(void)
{
    if (verbose) puts("STUB: DataHPreallocate called");
    return NULL;
}

void* DataHPreallocate64(void)
{
    if (verbose) puts("STUB: DataHPreallocate64 called");
    return NULL;
}

void* DataHPreextend(void)
{
    if (verbose) puts("STUB: DataHPreextend called");
    return NULL;
}

void* DataHPreextend64(void)
{
    if (verbose) puts("STUB: DataHPreextend64 called");
    return NULL;
}

void* DataHPutData(void)
{
    if (verbose) puts("STUB: DataHPutData called");
    return NULL;
}

void* DataHReadAsync(void)
{
    if (verbose) puts("STUB: DataHReadAsync called");
    return NULL;
}

void* DataHRenameFile(void)
{
    if (verbose) puts("STUB: DataHRenameFile called");
    return NULL;
}

void* DataHResolveDataRef(void)
{
    if (verbose) puts("STUB: DataHResolveDataRef called");
    return NULL;
}

void* DataHScheduleData(void)
{
    if (verbose) puts("STUB: DataHScheduleData called");
    return NULL;
}

void* DataHScheduleData64(void)
{
    if (verbose) puts("STUB: DataHScheduleData64 called");
    return NULL;
}

void* DataHSeek(void)
{
    if (verbose) puts("STUB: DataHSeek called");
    return NULL;
}

void* DataHSetCacheSizeLimit(void)
{
    if (verbose) puts("STUB: DataHSetCacheSizeLimit called");
    return NULL;
}

void* DataHSetDataRef(void)
{
    if (verbose) puts("STUB: DataHSetDataRef called");
    return NULL;
}

void* DataHSetDataRefExtension(void)
{
    if (verbose) puts("STUB: DataHSetDataRefExtension called");
    return NULL;
}

void* DataHSetDataRefWithAnchor(void)
{
    if (verbose) puts("STUB: DataHSetDataRefWithAnchor called");
    return NULL;
}

void* DataHSetFileSize(void)
{
    if (verbose) puts("STUB: DataHSetFileSize called");
    return NULL;
}

void* DataHSetFileSize64(void)
{
    if (verbose) puts("STUB: DataHSetFileSize64 called");
    return NULL;
}

void* DataHSetIdleManager(void)
{
    if (verbose) puts("STUB: DataHSetIdleManager called");
    return NULL;
}

void* DataHSetInfo(void)
{
    if (verbose) puts("STUB: DataHSetInfo called");
    return NULL;
}

void* DataHSetLastModified(void)
{
    if (verbose) puts("STUB: DataHSetLastModified called");
    return NULL;
}

void* DataHSetMacOSFileType(void)
{
    if (verbose) puts("STUB: DataHSetMacOSFileType called");
    return NULL;
}

void* DataHSetMovieUsageFlags(void)
{
    if (verbose) puts("STUB: DataHSetMovieUsageFlags called");
    return NULL;
}

void* DataHSetRate(void)
{
    if (verbose) puts("STUB: DataHSetRate called");
    return NULL;
}

void* DataHSetTimeBase(void)
{
    if (verbose) puts("STUB: DataHSetTimeBase called");
    return NULL;
}

void* DataHSetTimeHints(void)
{
    if (verbose) puts("STUB: DataHSetTimeHints called");
    return NULL;
}

void* DataHTask(void)
{
    if (verbose) puts("STUB: DataHTask called");
    return NULL;
}

void* DataHUpdateMovie(void)
{
    if (verbose) puts("STUB: DataHUpdateMovie called");
    return NULL;
}

void* DataHUseTemporaryDataRef(void)
{
    if (verbose) puts("STUB: DataHUseTemporaryDataRef called");
    return NULL;
}

void* DataHWrite(void)
{
    if (verbose) puts("STUB: DataHWrite called");
    return NULL;
}

void* DataHWrite64(void)
{
    if (verbose) puts("STUB: DataHWrite64 called");
    return NULL;
}

void* DbgAssert(void)
{
    if (verbose) puts("STUB: DbgAssert called");
    return NULL;
}

void* DbgBreakPoint(void)
{
    if (verbose) puts("STUB: DbgBreakPoint called");
    return NULL;
}

void* DbgPrint(void)
{
    if (verbose) puts("STUB: DbgPrint called");
    return NULL;
}

void* DbgPrintMessage(void)
{
    if (verbose) puts("STUB: DbgPrintMessage called");
    return NULL;
}

void* DbgSetTraceUsingMediaKeys(void)
{
    if (verbose) puts("STUB: DbgSetTraceUsingMediaKeys called");
    return NULL;
}

void* DbgTrace(void)
{
    if (verbose) puts("STUB: DbgTrace called");
    return NULL;
}

void* DecompressImage(void)
{
    if (verbose) puts("STUB: DecompressImage called");
    return NULL;
}

void* DecompressSequenceBegin(void)
{
    if (verbose) puts("STUB: DecompressSequenceBegin called");
    return NULL;
}

void* DecompressSequenceBeginS(void)
{
    if (verbose) puts("STUB: DecompressSequenceBeginS called");
    return NULL;
}

void* DecompressSequenceFrame(void)
{
    if (verbose) puts("STUB: DecompressSequenceFrame called");
    return NULL;
}

void* DecompressSequenceFrameS(void)
{
    if (verbose) puts("STUB: DecompressSequenceFrameS called");
    return NULL;
}

void* DecompressSequenceFrameWhen(void)
{
    if (verbose) puts("STUB: DecompressSequenceFrameWhen called");
    return NULL;
}

void* DeleteIndexedComponentMessageParameter(void)
{
    if (verbose) puts("STUB: DeleteIndexedComponentMessageParameter called");
    return NULL;
}

void* DeleteMovieFile(void)
{
    if (verbose) puts("STUB: DeleteMovieFile called");
    return NULL;
}

void* DeleteMovieFileUnicode(void)
{
    if (verbose) puts("STUB: DeleteMovieFileUnicode called");
    return NULL;
}

void* DeleteMovieSegment(void)
{
    if (verbose) puts("STUB: DeleteMovieSegment called");
    return NULL;
}

void* DeleteMovieStorage(void)
{
    if (verbose) puts("STUB: DeleteMovieStorage called");
    return NULL;
}

void* DeleteNamedComponentMessageParameter(void)
{
    if (verbose) puts("STUB: DeleteNamedComponentMessageParameter called");
    return NULL;
}

void* DeleteTrackReference(void)
{
    if (verbose) puts("STUB: DeleteTrackReference called");
    return NULL;
}

void* DeleteTrackSegment(void)
{
    if (verbose) puts("STUB: DeleteTrackSegment called");
    return NULL;
}

void* DequeueSyncTask(void)
{
    if (verbose) puts("STUB: DequeueSyncTask called");
    return NULL;
}

void* DequeueSyncTaskFromTimeBaseThread(void)
{
    if (verbose) puts("STUB: DequeueSyncTaskFromTimeBaseThread called");
    return NULL;
}

void* DetachMovieFromCurrentThread(void)
{
    if (verbose) puts("STUB: DetachMovieFromCurrentThread called");
    return NULL;
}

void* DetachTimeBaseFromCurrentThread(void)
{
    if (verbose) puts("STUB: DetachTimeBaseFromCurrentThread called");
    return NULL;
}

void* DisposeActionsUPP(void)
{
    if (verbose) puts("STUB: DisposeActionsUPP called");
    return NULL;
}

void* DisposeAllSprites(void)
{
    if (verbose) puts("STUB: DisposeAllSprites called");
    return NULL;
}

void* DisposeBlitterUPP(void)
{
    if (verbose) puts("STUB: DisposeBlitterUPP called");
    return NULL;
}

void* DisposeCDataHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeCDataHandlerUPP called");
    return NULL;
}

void* DisposeCGNotificationUPP(void)
{
    if (verbose) puts("STUB: DisposeCGNotificationUPP called");
    return NULL;
}

void* DisposeCallBack(void)
{
    if (verbose) puts("STUB: DisposeCallBack called");
    return NULL;
}

void* DisposeCancelledDataUPP(void)
{
    if (verbose) puts("STUB: DisposeCancelledDataUPP called");
    return NULL;
}

void* DisposeCharDataHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeCharDataHandlerUPP called");
    return NULL;
}

void* DisposeCodecNameList(void)
{
    if (verbose) puts("STUB: DisposeCodecNameList called");
    return NULL;
}

void* DisposeCommentHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeCommentHandlerUPP called");
    return NULL;
}

void* DisposeCursorNotify(void)
{
    if (verbose) puts("STUB: DisposeCursorNotify called");
    return NULL;
}

void* DisposeDataHCompletionUPP(void)
{
    if (verbose) puts("STUB: DisposeDataHCompletionUPP called");
    return NULL;
}

void* DisposeDoMCActionUPP(void)
{
    if (verbose) puts("STUB: DisposeDoMCActionUPP called");
    return NULL;
}

void* DisposeEndDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeEndDocumentHandlerUPP called");
    return NULL;
}

void* DisposeEndElementHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeEndElementHandlerUPP called");
    return NULL;
}

void* DisposeEraseUPP(void)
{
    if (verbose) puts("STUB: DisposeEraseUPP called");
    return NULL;
}

void* DisposeExitToShellUPP(void)
{
    if (verbose) puts("STUB: DisposeExitToShellUPP called");
    return NULL;
}

void* DisposeForEachMediaUPP(void)
{
    if (verbose) puts("STUB: DisposeForEachMediaUPP called");
    return NULL;
}

void* DisposeForEachTrackUPP(void)
{
    if (verbose) puts("STUB: DisposeForEachTrackUPP called");
    return NULL;
}

void* DisposeGetAtomLeafUPP(void)
{
    if (verbose) puts("STUB: DisposeGetAtomLeafUPP called");
    return NULL;
}

void* DisposeGetMoreMovieDataUPP(void)
{
    if (verbose) puts("STUB: DisposeGetMoreMovieDataUPP called");
    return NULL;
}

void* DisposeGetMovieUPP(void)
{
    if (verbose) puts("STUB: DisposeGetMovieUPP called");
    return NULL;
}

void* DisposeICMAlignmentUPP(void)
{
    if (verbose) puts("STUB: DisposeICMAlignmentUPP called");
    return NULL;
}

void* DisposeICMCompletionUPP(void)
{
    if (verbose) puts("STUB: DisposeICMCompletionUPP called");
    return NULL;
}

void* DisposeICMConvertDataFormatUPP(void)
{
    if (verbose) puts("STUB: DisposeICMConvertDataFormatUPP called");
    return NULL;
}

void* DisposeICMCursorShieldedUPP(void)
{
    if (verbose) puts("STUB: DisposeICMCursorShieldedUPP called");
    return NULL;
}

void* DisposeICMDataUPP(void)
{
    if (verbose) puts("STUB: DisposeICMDataUPP called");
    return NULL;
}

void* DisposeICMFlushUPP(void)
{
    if (verbose) puts("STUB: DisposeICMFlushUPP called");
    return NULL;
}

void* DisposeICMMemoryDisposedUPP(void)
{
    if (verbose) puts("STUB: DisposeICMMemoryDisposedUPP called");
    return NULL;
}

void* DisposeICMProgressUPP(void)
{
    if (verbose) puts("STUB: DisposeICMProgressUPP called");
    return NULL;
}

void* DisposeImageCodecDrawBandCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeImageCodecDrawBandCompleteUPP called");
    return NULL;
}

void* DisposeImageCodecMPDrawBandUPP(void)
{
    if (verbose) puts("STUB: DisposeImageCodecMPDrawBandUPP called");
    return NULL;
}

void* DisposeImageCodecTimeTriggerUPP(void)
{
    if (verbose) puts("STUB: DisposeImageCodecTimeTriggerUPP called");
    return NULL;
}

void* DisposeMCActionFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeMCActionFilterUPP called");
    return NULL;
}

void* DisposeMCActionFilterWithRefConUPP(void)
{
    if (verbose) puts("STUB: DisposeMCActionFilterWithRefConUPP called");
    return NULL;
}

void* DisposeMCActionNotificationUPP(void)
{
    if (verbose) puts("STUB: DisposeMCActionNotificationUPP called");
    return NULL;
}

void* DisposeMatte(void)
{
    if (verbose) puts("STUB: DisposeMatte called");
    return NULL;
}

void* DisposeMovie(void)
{
    if (verbose) puts("STUB: DisposeMovie called");
    return NULL;
}

void* DisposeMovieController(void)
{
    if (verbose) puts("STUB: DisposeMovieController called");
    return NULL;
}

void* DisposeMovieDrawingCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieDrawingCompleteUPP called");
    return NULL;
}

void* DisposeMovieEditState(void)
{
    if (verbose) puts("STUB: DisposeMovieEditState called");
    return NULL;
}

void* DisposeMovieExecuteWiredActionsUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieExecuteWiredActionsUPP called");
    return NULL;
}

void* DisposeMovieExportGetDataUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieExportGetDataUPP called");
    return NULL;
}

void* DisposeMovieExportGetPropertyUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieExportGetPropertyUPP called");
    return NULL;
}

void* DisposeMovieExportStageReachedCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieExportStageReachedCallbackUPP called");
    return NULL;
}

void* DisposeMoviePrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeMoviePrePrerollCompleteUPP called");
    return NULL;
}

void* DisposeMoviePreviewCallOutUPP(void)
{
    if (verbose) puts("STUB: DisposeMoviePreviewCallOutUPP called");
    return NULL;
}

void* DisposeMovieProgressUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieProgressUPP called");
    return NULL;
}

void* DisposeMovieRgnCoverUPP(void)
{
    if (verbose) puts("STUB: DisposeMovieRgnCoverUPP called");
    return NULL;
}

void* DisposeMovieTrack(void)
{
    if (verbose) puts("STUB: DisposeMovieTrack called");
    return NULL;
}

void* DisposeMoviesErrorUPP(void)
{
    if (verbose) puts("STUB: DisposeMoviesErrorUPP called");
    return NULL;
}

void* DisposeMusicMIDIReadHookUPP(void)
{
    if (verbose) puts("STUB: DisposeMusicMIDIReadHookUPP called");
    return NULL;
}

void* DisposeMusicMIDISendUPP(void)
{
    if (verbose) puts("STUB: DisposeMusicMIDISendUPP called");
    return NULL;
}

void* DisposeMusicOfflineDataUPP(void)
{
    if (verbose) puts("STUB: DisposeMusicOfflineDataUPP called");
    return NULL;
}

void* DisposePBOpenUPP(void)
{
    if (verbose) puts("STUB: DisposePBOpenUPP called");
    return NULL;
}

void* DisposePrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposePrePrerollCompleteUPP called");
    return NULL;
}

void* DisposePreprocessInstructionHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposePreprocessInstructionHandlerUPP called");
    return NULL;
}

void* DisposePutNeedRoomUPP(void)
{
    if (verbose) puts("STUB: DisposePutNeedRoomUPP called");
    return NULL;
}

void* DisposePutPatchLongUPP(void)
{
    if (verbose) puts("STUB: DisposePutPatchLongUPP called");
    return NULL;
}

void* DisposeQDPixUPP(void)
{
    if (verbose) puts("STUB: DisposeQDPixUPP called");
    return NULL;
}

void* DisposeQTAllDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeQTAllDataDoneCompleteUPP called");
    return NULL;
}

void* DisposeQTAllocatorAllocateUPP(void)
{
    if (verbose) puts("STUB: DisposeQTAllocatorAllocateUPP called");
    return NULL;
}

void* DisposeQTAllocatorDisposeAllocatorUPP(void)
{
    if (verbose) puts("STUB: DisposeQTAllocatorDisposeAllocatorUPP called");
    return NULL;
}

void* DisposeQTAllocatorFreeUPP(void)
{
    if (verbose) puts("STUB: DisposeQTAllocatorFreeUPP called");
    return NULL;
}

void* DisposeQTAllocatorReallocateUPP(void)
{
    if (verbose) puts("STUB: DisposeQTAllocatorReallocateUPP called");
    return NULL;
}

void* DisposeQTBandwidthNotificationUPP(void)
{
    if (verbose) puts("STUB: DisposeQTBandwidthNotificationUPP called");
    return NULL;
}

void* DisposeQTCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTCallBackUPP called");
    return NULL;
}

void* DisposeQTComponentPropertyListenerFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeQTComponentPropertyListenerFilterUPP called");
    return NULL;
}

void* DisposeQTComponentPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: DisposeQTComponentPropertyListenerUPP called");
    return NULL;
}

void* DisposeQTDRMPrivAPICallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTDRMPrivAPICallBackUPP called");
    return NULL;
}

void* DisposeQTDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeQTDataDoneCompleteUPP called");
    return NULL;
}

void* DisposeQTDataStashValueDisposeUPP(void)
{
    if (verbose) puts("STUB: DisposeQTDataStashValueDisposeUPP called");
    return NULL;
}

void* DisposeQTEffectListFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeQTEffectListFilterUPP called");
    return NULL;
}

void* DisposeQTFlattenCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTFlattenCallBackUPP called");
    return NULL;
}

void* DisposeQTGenerateHashValueUPP(void)
{
    if (verbose) puts("STUB: DisposeQTGenerateHashValueUPP called");
    return NULL;
}

void* DisposeQTHashKeysAreEqualUPP(void)
{
    if (verbose) puts("STUB: DisposeQTHashKeysAreEqualUPP called");
    return NULL;
}

void* DisposeQTMemPoolCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTMemPoolCallBackUPP called");
    return NULL;
}

void* DisposeQTMessageDispatchUPP(void)
{
    if (verbose) puts("STUB: DisposeQTMessageDispatchUPP called");
    return NULL;
}

void* DisposeQTMissingSoftwareCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTMissingSoftwareCallbackUPP called");
    return NULL;
}

void* DisposeQTMoviePropertyListenerUPP(void)
{
    if (verbose) puts("STUB: DisposeQTMoviePropertyListenerUPP called");
    return NULL;
}

void* DisposeQTMovieStatusPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: DisposeQTMovieStatusPropertyListenerUPP called");
    return NULL;
}

void* DisposeQTNextTaskNeededSoonerCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTNextTaskNeededSoonerCallbackUPP called");
    return NULL;
}

void* DisposeQTProcessPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: DisposeQTProcessPropertyListenerUPP called");
    return NULL;
}

void* DisposeQTReleaseKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: DisposeQTReleaseKeyAndValueUPP called");
    return NULL;
}

void* DisposeQTRestrictions(void)
{
    if (verbose) puts("STUB: DisposeQTRestrictions called");
    return NULL;
}

void* DisposeQTRetainKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: DisposeQTRetainKeyAndValueUPP called");
    return NULL;
}

void* DisposeQTSDataDispatchFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSDataDispatchFilterUPP called");
    return NULL;
}

void* DisposeQTSDialogSettingsUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSDialogSettingsUPP called");
    return NULL;
}

void* DisposeQTSModalFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSModalFilterUPP called");
    return NULL;
}

void* DisposeQTSNotificationUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSNotificationUPP called");
    return NULL;
}

void* DisposeQTSPanelFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSPanelFilterUPP called");
    return NULL;
}

void* DisposeQTSSourcerDataReleaseUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSSourcerDataReleaseUPP called");
    return NULL;
}

void* DisposeQTSStreamModuleUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSStreamModuleUPP called");
    return NULL;
}

void* DisposeQTSTrackHinterProgressUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSTrackHinterProgressUPP called");
    return NULL;
}

void* DisposeQTSyncTaskUPP(void)
{
    if (verbose) puts("STUB: DisposeQTSyncTaskUPP called");
    return NULL;
}

void* DisposeQTTrackPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: DisposeQTTrackPropertyListenerUPP called");
    return NULL;
}

void* DisposeQTUpdateMovieProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTUpdateMovieProfileCallbackUPP called");
    return NULL;
}

void* DisposeQTUpdateTrackProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeQTUpdateTrackProfileCallbackUPP called");
    return NULL;
}

void* DisposeQTVRBackBufferImagingUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVRBackBufferImagingUPP called");
    return NULL;
}

void* DisposeQTVREnteringNodeUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVREnteringNodeUPP called");
    return NULL;
}

void* DisposeQTVRImagingCompleteUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVRImagingCompleteUPP called");
    return NULL;
}

void* DisposeQTVRInterceptUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVRInterceptUPP called");
    return NULL;
}

void* DisposeQTVRLeavingNodeUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVRLeavingNodeUPP called");
    return NULL;
}

void* DisposeQTVRMouseOverHotSpotUPP(void)
{
    if (verbose) puts("STUB: DisposeQTVRMouseOverHotSpotUPP called");
    return NULL;
}

void* DisposeQTWorkContextDataReleaseUPP(void)
{
    if (verbose) puts("STUB: DisposeQTWorkContextDataReleaseUPP called");
    return NULL;
}

void* DisposeQuickTimeErrorDialog(void)
{
    if (verbose) puts("STUB: DisposeQuickTimeErrorDialog called");
    return NULL;
}

void* DisposeRTPMPDataReleaseUPP(void)
{
    if (verbose) puts("STUB: DisposeRTPMPDataReleaseUPP called");
    return NULL;
}

void* DisposeRTPPBCallbackUPP(void)
{
    if (verbose) puts("STUB: DisposeRTPPBCallbackUPP called");
    return NULL;
}

void* DisposeRemapTable(void)
{
    if (verbose) puts("STUB: DisposeRemapTable called");
    return NULL;
}

void* DisposeSCModalFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeSCModalFilterUPP called");
    return NULL;
}

void* DisposeSCModalHookUPP(void)
{
    if (verbose) puts("STUB: DisposeSCModalHookUPP called");
    return NULL;
}

void* DisposeSFModalFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeSFModalFilterUPP called");
    return NULL;
}

void* DisposeSGAddFrameBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGAddFrameBottleUPP called");
    return NULL;
}

void* DisposeSGCompressBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGCompressBottleUPP called");
    return NULL;
}

void* DisposeSGCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGCompressCompleteBottleUPP called");
    return NULL;
}

void* DisposeSGDataUPP(void)
{
    if (verbose) puts("STUB: DisposeSGDataUPP called");
    return NULL;
}

void* DisposeSGDisplayBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGDisplayBottleUPP called");
    return NULL;
}

void* DisposeSGDisplayCompressBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGDisplayCompressBottleUPP called");
    return NULL;
}

void* DisposeSGGrabBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGGrabBottleUPP called");
    return NULL;
}

void* DisposeSGGrabCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGGrabCompleteBottleUPP called");
    return NULL;
}

void* DisposeSGGrabCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGGrabCompressCompleteBottleUPP called");
    return NULL;
}

void* DisposeSGModalFilterUPP(void)
{
    if (verbose) puts("STUB: DisposeSGModalFilterUPP called");
    return NULL;
}

void* DisposeSGTransferFrameBottleUPP(void)
{
    if (verbose) puts("STUB: DisposeSGTransferFrameBottleUPP called");
    return NULL;
}

void* DisposeSHChunkDataReleaseUPP(void)
{
    if (verbose) puts("STUB: DisposeSHChunkDataReleaseUPP called");
    return NULL;
}

void* DisposeSHNotificationUPP(void)
{
    if (verbose) puts("STUB: DisposeSHNotificationUPP called");
    return NULL;
}

void* DisposeSHSendDataUPP(void)
{
    if (verbose) puts("STUB: DisposeSHSendDataUPP called");
    return NULL;
}

void* DisposeSequenceFilterDataUPP(void)
{
    if (verbose) puts("STUB: DisposeSequenceFilterDataUPP called");
    return NULL;
}

void* DisposeSprite(void)
{
    if (verbose) puts("STUB: DisposeSprite called");
    return NULL;
}

void* DisposeSpriteWorld(void)
{
    if (verbose) puts("STUB: DisposeSpriteWorld called");
    return NULL;
}

void* DisposeStartDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeStartDocumentHandlerUPP called");
    return NULL;
}

void* DisposeStartElementHandlerUPP(void)
{
    if (verbose) puts("STUB: DisposeStartElementHandlerUPP called");
    return NULL;
}

void* DisposeStdPixUPP(void)
{
    if (verbose) puts("STUB: DisposeStdPixUPP called");
    return NULL;
}

void* DisposeTextMediaUPP(void)
{
    if (verbose) puts("STUB: DisposeTextMediaUPP called");
    return NULL;
}

void* DisposeTimeBase(void)
{
    if (verbose) puts("STUB: DisposeTimeBase called");
    return NULL;
}

void* DisposeTrackEditState(void)
{
    if (verbose) puts("STUB: DisposeTrackEditState called");
    return NULL;
}

void* DisposeTrackMedia(void)
{
    if (verbose) puts("STUB: DisposeTrackMedia called");
    return NULL;
}

void* DisposeTrackTransferUPP(void)
{
    if (verbose) puts("STUB: DisposeTrackTransferUPP called");
    return NULL;
}

void* DisposeTuneCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeTuneCallBackUPP called");
    return NULL;
}

void* DisposeTunePlayCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeTunePlayCallBackUPP called");
    return NULL;
}

void* DisposeTweenerDataUPP(void)
{
    if (verbose) puts("STUB: DisposeTweenerDataUPP called");
    return NULL;
}

void* DisposeUpdaterAppHandleMenuEventUPP(void)
{
    if (verbose) puts("STUB: DisposeUpdaterAppHandleMenuEventUPP called");
    return NULL;
}

void* DisposeUseThisTrackForTimeTableUPP(void)
{
    if (verbose) puts("STUB: DisposeUseThisTrackForTimeTableUPP called");
    return NULL;
}

void* DisposeUserData(void)
{
    if (verbose) puts("STUB: DisposeUserData called");
    return NULL;
}

void* DisposeVdigIntUPP(void)
{
    if (verbose) puts("STUB: DisposeVdigIntUPP called");
    return NULL;
}

void* DisposeYUVPtrsUPP(void)
{
    if (verbose) puts("STUB: DisposeYUVPtrsUPP called");
    return NULL;
}

void* DoMovieUpdateCheck(void)
{
    if (verbose) puts("STUB: DoMovieUpdateCheck called");
    return NULL;
}

void* DragAlignedGrayRgn(void)
{
    if (verbose) puts("STUB: DragAlignedGrayRgn called");
    return NULL;
}

void* DragAlignedWindow(void)
{
    if (verbose) puts("STUB: DragAlignedWindow called");
    return NULL;
}

void* DrawPictureFile(void)
{
    if (verbose) puts("STUB: DrawPictureFile called");
    return NULL;
}

void* DrawTrimmedPicture(void)
{
    if (verbose) puts("STUB: DrawTrimmedPicture called");
    return NULL;
}

void* DrawTrimmedPictureFile(void)
{
    if (verbose) puts("STUB: DrawTrimmedPictureFile called");
    return NULL;
}

void* EndFullScreen(void)
{
    if (verbose) puts("STUB: EndFullScreen called");
    return NULL;
}

void* EndMediaEdits(void)
{
    if (verbose) puts("STUB: EndMediaEdits called");
    return NULL;
}

void* EndianCommonChunk_NtoB(void)
{
    if (verbose) puts("STUB: EndianCommonChunk_NtoB called");
    return NULL;
}

void* EndianContainerChunk_NtoB(void)
{
    if (verbose) puts("STUB: EndianContainerChunk_NtoB called");
    return NULL;
}

void* EndianExtCommonChunk_NtoB(void)
{
    if (verbose) puts("STUB: EndianExtCommonChunk_NtoB called");
    return NULL;
}

void* EndianImageDescription_BtoN(void)
{
    if (verbose) puts("STUB: EndianImageDescription_BtoN called");
    return NULL;
}

void* EndianImageDescription_NtoB(void)
{
    if (verbose) puts("STUB: EndianImageDescription_NtoB called");
    return NULL;
}

void* EndianMatrixRecord_BtoN(void)
{
    if (verbose) puts("STUB: EndianMatrixRecord_BtoN called");
    return NULL;
}

void* EndianModifierTrackGraphicsModeRecord_BtoN(void)
{
    if (verbose) puts("STUB: EndianModifierTrackGraphicsModeRecord_BtoN called");
    return NULL;
}

void* EndianSoundDataChunk_NtoB(void)
{
    if (verbose) puts("STUB: EndianSoundDataChunk_NtoB called");
    return NULL;
}

void* EndianVersionChunk_NtoB(void)
{
    if (verbose) puts("STUB: EndianVersionChunk_NtoB called");
    return NULL;
}

void* EnterMovies(void)
{
    if (verbose) puts("STUB: EnterMovies called");
    return NULL;
}

void* EnterMoviesOnThread(void)
{
    if (verbose) puts("STUB: EnterMoviesOnThread called");
    return NULL;
}

void* EqualHandles(void)
{
    if (verbose) puts("STUB: EqualHandles called");
    return NULL;
}

void* EqualMatrix(void)
{
    if (verbose) puts("STUB: EqualMatrix called");
    return NULL;
}

void* ExecuteCallBack(void)
{
    if (verbose) puts("STUB: ExecuteCallBack called");
    return NULL;
}

void* ExitMovies(void)
{
    if (verbose) puts("STUB: ExitMovies called");
    return NULL;
}

void* ExitMoviesOnThread(void)
{
    if (verbose) puts("STUB: ExitMoviesOnThread called");
    return NULL;
}

void* ExtendMediaDecodeDurationToDisplayEndTime(void)
{
    if (verbose) puts("STUB: ExtendMediaDecodeDurationToDisplayEndTime called");
    return NULL;
}

void* FCompressImage(void)
{
    if (verbose) puts("STUB: FCompressImage called");
    return NULL;
}

void* FCompressPicture(void)
{
    if (verbose) puts("STUB: FCompressPicture called");
    return NULL;
}

void* FCompressPictureFile(void)
{
    if (verbose) puts("STUB: FCompressPictureFile called");
    return NULL;
}

void* FDecompressImage(void)
{
    if (verbose) puts("STUB: FDecompressImage called");
    return NULL;
}

void* FSSpecEqual(void)
{
    if (verbose) puts("STUB: FSSpecEqual called");
    return NULL;
}

void* FindCodec(void)
{
    if (verbose) puts("STUB: FindCodec called");
    return NULL;
}

void* FixExp2(void)
{
    if (verbose) puts("STUB: FixExp2 called");
    return NULL;
}

void* FixLog2(void)
{
    if (verbose) puts("STUB: FixLog2 called");
    return NULL;
}

void* FixMulDiv(void)
{
    if (verbose) puts("STUB: FixMulDiv called");
    return NULL;
}

void* FixPow(void)
{
    if (verbose) puts("STUB: FixPow called");
    return NULL;
}

void* FlashMediaDoButtonActions(void)
{
    if (verbose) puts("STUB: FlashMediaDoButtonActions called");
    return NULL;
}

void* FlashMediaFrameLabelToMovieTime(void)
{
    if (verbose) puts("STUB: FlashMediaFrameLabelToMovieTime called");
    return NULL;
}

void* FlashMediaFrameNumberToMovieTime(void)
{
    if (verbose) puts("STUB: FlashMediaFrameNumberToMovieTime called");
    return NULL;
}

void* FlashMediaGetDisplayedFrameNumber(void)
{
    if (verbose) puts("STUB: FlashMediaGetDisplayedFrameNumber called");
    return NULL;
}

void* FlashMediaGetFlashVariable(void)
{
    if (verbose) puts("STUB: FlashMediaGetFlashVariable called");
    return NULL;
}

void* FlashMediaGetRefConBounds(void)
{
    if (verbose) puts("STUB: FlashMediaGetRefConBounds called");
    return NULL;
}

void* FlashMediaGetRefConID(void)
{
    if (verbose) puts("STUB: FlashMediaGetRefConID called");
    return NULL;
}

void* FlashMediaGetSupportedSwfVersion(void)
{
    if (verbose) puts("STUB: FlashMediaGetSupportedSwfVersion called");
    return NULL;
}

void* FlashMediaIDToRefCon(void)
{
    if (verbose) puts("STUB: FlashMediaIDToRefCon called");
    return NULL;
}

void* FlashMediaSetFlashVariable(void)
{
    if (verbose) puts("STUB: FlashMediaSetFlashVariable called");
    return NULL;
}

void* FlashMediaSetPan(void)
{
    if (verbose) puts("STUB: FlashMediaSetPan called");
    return NULL;
}

void* FlashMediaSetZoom(void)
{
    if (verbose) puts("STUB: FlashMediaSetZoom called");
    return NULL;
}

void* FlashMediaSetZoomRect(void)
{
    if (verbose) puts("STUB: FlashMediaSetZoomRect called");
    return NULL;
}

void* FlattenMovie(void)
{
    if (verbose) puts("STUB: FlattenMovie called");
    return NULL;
}

void* FlattenMovieData(void)
{
    if (verbose) puts("STUB: FlattenMovieData called");
    return NULL;
}

void* FlattenMovieDataToDataRef(void)
{
    if (verbose) puts("STUB: FlattenMovieDataToDataRef called");
    return NULL;
}

void* FlattenMovieDataWithProc(void)
{
    if (verbose) puts("STUB: FlattenMovieDataWithProc called");
    return NULL;
}

void* FracSinCos(void)
{
    if (verbose) puts("STUB: FracSinCos called");
    return NULL;
}

void* FullScreenBeginPresentation(void)
{
    if (verbose) puts("STUB: FullScreenBeginPresentation called");
    return NULL;
}

void* FullScreenEndPresentation(void)
{
    if (verbose) puts("STUB: FullScreenEndPresentation called");
    return NULL;
}

void* GDBDumpVisualStats(void)
{
    if (verbose) puts("STUB: GDBDumpVisualStats called");
    return NULL;
}

void* GDBResetVisualStats(void)
{
    if (verbose) puts("STUB: GDBResetVisualStats called");
    return NULL;
}

void* GDGetScale(void)
{
    if (verbose) puts("STUB: GDGetScale called");
    return NULL;
}

void* GDHasScale(void)
{
    if (verbose) puts("STUB: GDHasScale called");
    return NULL;
}

void* GDSetScale(void)
{
    if (verbose) puts("STUB: GDSetScale called");
    return NULL;
}

void* GenerateMovieApertureModeDimensions(void)
{
    if (verbose) puts("STUB: GenerateMovieApertureModeDimensions called");
    return NULL;
}

void* GenerateTrackApertureModeDimensions(void)
{
    if (verbose) puts("STUB: GenerateTrackApertureModeDimensions called");
    return NULL;
}

void* GetAtomID(void)
{
    if (verbose) puts("STUB: GetAtomID called");
    return NULL;
}

void* GetAtomList(void)
{
    if (verbose) puts("STUB: GetAtomList called");
    return NULL;
}

void* GetBestDeviceRect(void)
{
    if (verbose) puts("STUB: GetBestDeviceRect called");
    return NULL;
}

void* GetCSequenceDataRateParams(void)
{
    if (verbose) puts("STUB: GetCSequenceDataRateParams called");
    return NULL;
}

void* GetCSequenceFrameNumber(void)
{
    if (verbose) puts("STUB: GetCSequenceFrameNumber called");
    return NULL;
}

void* GetCSequenceKeyFrameRate(void)
{
    if (verbose) puts("STUB: GetCSequenceKeyFrameRate called");
    return NULL;
}

void* GetCSequenceMaxCompressionSize(void)
{
    if (verbose) puts("STUB: GetCSequenceMaxCompressionSize called");
    return NULL;
}

void* GetCSequencePrevBuffer(void)
{
    if (verbose) puts("STUB: GetCSequencePrevBuffer called");
    return NULL;
}

void* GetCallBackTimeBase(void)
{
    if (verbose) puts("STUB: GetCallBackTimeBase called");
    return NULL;
}

void* GetCallBackType(void)
{
    if (verbose) puts("STUB: GetCallBackType called");
    return NULL;
}

void* GetCodecInfo(void)
{
    if (verbose) puts("STUB: GetCodecInfo called");
    return NULL;
}

void* GetCodecNameList(void)
{
    if (verbose) puts("STUB: GetCodecNameList called");
    return NULL;
}

void* GetComponentMessageAttribute(void)
{
    if (verbose) puts("STUB: GetComponentMessageAttribute called");
    return NULL;
}

void* GetComponentMessageAttributesBulk(void)
{
    if (verbose) puts("STUB: GetComponentMessageAttributesBulk called");
    return NULL;
}

void* GetComponentMessageClassAndID(void)
{
    if (verbose) puts("STUB: GetComponentMessageClassAndID called");
    return NULL;
}

void* GetComponentMessageParamCount(void)
{
    if (verbose) puts("STUB: GetComponentMessageParamCount called");
    return NULL;
}

void* GetComponentMessageReplyHasBeenSent(void)
{
    if (verbose) puts("STUB: GetComponentMessageReplyHasBeenSent called");
    return NULL;
}

void* GetComponentMessageStatus(void)
{
    if (verbose) puts("STUB: GetComponentMessageStatus called");
    return NULL;
}

void* GetComponentMessageTime(void)
{
    if (verbose) puts("STUB: GetComponentMessageTime called");
    return NULL;
}

void* GetComponentPropertiesBulk(void)
{
    if (verbose) puts("STUB: GetComponentPropertiesBulk called");
    return NULL;
}

void* GetComponentPropertyAsType(void)
{
    if (verbose) puts("STUB: GetComponentPropertyAsType called");
    return NULL;
}

void* GetCompressedImageSize(void)
{
    if (verbose) puts("STUB: GetCompressedImageSize called");
    return NULL;
}

void* GetCompressedPixMapInfo(void)
{
    if (verbose) puts("STUB: GetCompressedPixMapInfo called");
    return NULL;
}

void* GetCompressionTime(void)
{
    if (verbose) puts("STUB: GetCompressionTime called");
    return NULL;
}

void* GetDSequenceImageBuffer(void)
{
    if (verbose) puts("STUB: GetDSequenceImageBuffer called");
    return NULL;
}

void* GetDSequenceMatrix(void)
{
    if (verbose) puts("STUB: GetDSequenceMatrix called");
    return NULL;
}

void* GetDSequenceNonScheduledDisplayDirection(void)
{
    if (verbose) puts("STUB: GetDSequenceNonScheduledDisplayDirection called");
    return NULL;
}

void* GetDSequenceNonScheduledDisplayTime(void)
{
    if (verbose) puts("STUB: GetDSequenceNonScheduledDisplayTime called");
    return NULL;
}

void* GetDSequenceScreenBuffer(void)
{
    if (verbose) puts("STUB: GetDSequenceScreenBuffer called");
    return NULL;
}

void* GetDataHandler(void)
{
    if (verbose) puts("STUB: GetDataHandler called");
    return NULL;
}

void* GetFirstCallBack(void)
{
    if (verbose) puts("STUB: GetFirstCallBack called");
    return NULL;
}

void* GetFixedSizeAtom(void)
{
    if (verbose) puts("STUB: GetFixedSizeAtom called");
    return NULL;
}

void* GetGraphicsImporterForDataRef(void)
{
    if (verbose) puts("STUB: GetGraphicsImporterForDataRef called");
    return NULL;
}

void* GetGraphicsImporterForDataRefWithFlags(void)
{
    if (verbose) puts("STUB: GetGraphicsImporterForDataRefWithFlags called");
    return NULL;
}

void* GetGraphicsImporterForFile(void)
{
    if (verbose) puts("STUB: GetGraphicsImporterForFile called");
    return NULL;
}

void* GetGraphicsImporterForFileWithFlags(void)
{
    if (verbose) puts("STUB: GetGraphicsImporterForFileWithFlags called");
    return NULL;
}

void* GetHandleAtom(void)
{
    if (verbose) puts("STUB: GetHandleAtom called");
    return NULL;
}

void* GetHandlerAtom(void)
{
    if (verbose) puts("STUB: GetHandlerAtom called");
    return NULL;
}

void* GetImageDescriptionCTable(void)
{
    if (verbose) puts("STUB: GetImageDescriptionCTable called");
    return NULL;
}

void* GetImageDescriptionExtension(void)
{
    if (verbose) puts("STUB: GetImageDescriptionExtension called");
    return NULL;
}

void* GetIndexedComponentMessageParameter(void)
{
    if (verbose) puts("STUB: GetIndexedComponentMessageParameter called");
    return NULL;
}

void* GetIndexedComponentMessageParameterInfo(void)
{
    if (verbose) puts("STUB: GetIndexedComponentMessageParameterInfo called");
    return NULL;
}

void* GetLockedFixedSizeAtom(void)
{
    if (verbose) puts("STUB: GetLockedFixedSizeAtom called");
    return NULL;
}

void* GetMP4ASBDfromESDS(void)
{
    if (verbose) puts("STUB: GetMP4ASBDfromESDS called");
    return NULL;
}

void* GetMatrixType(void)
{
    if (verbose) puts("STUB: GetMatrixType called");
    return NULL;
}

void* GetMaxCompressionSize(void)
{
    if (verbose) puts("STUB: GetMaxCompressionSize called");
    return NULL;
}

void* GetMaxLoadedTimeInMovie(void)
{
    if (verbose) puts("STUB: GetMaxLoadedTimeInMovie called");
    return NULL;
}

void* GetMediaAdvanceDecodeTime(void)
{
    if (verbose) puts("STUB: GetMediaAdvanceDecodeTime called");
    return NULL;
}

void* GetMediaBoundaryTimes(void)
{
    if (verbose) puts("STUB: GetMediaBoundaryTimes called");
    return NULL;
}

void* GetMediaCreationTime(void)
{
    if (verbose) puts("STUB: GetMediaCreationTime called");
    return NULL;
}

void* GetMediaDataHandler(void)
{
    if (verbose) puts("STUB: GetMediaDataHandler called");
    return NULL;
}

void* GetMediaDataHandlerDescription(void)
{
    if (verbose) puts("STUB: GetMediaDataHandlerDescription called");
    return NULL;
}

void* GetMediaDataRef(void)
{
    if (verbose) puts("STUB: GetMediaDataRef called");
    return NULL;
}

void* GetMediaDataRefCount(void)
{
    if (verbose) puts("STUB: GetMediaDataRefCount called");
    return NULL;
}

void* GetMediaDataSize(void)
{
    if (verbose) puts("STUB: GetMediaDataSize called");
    return NULL;
}

void* GetMediaDataSize64(void)
{
    if (verbose) puts("STUB: GetMediaDataSize64 called");
    return NULL;
}

void* GetMediaDataSizeTime64(void)
{
    if (verbose) puts("STUB: GetMediaDataSizeTime64 called");
    return NULL;
}

void* GetMediaDecodeDuration(void)
{
    if (verbose) puts("STUB: GetMediaDecodeDuration called");
    return NULL;
}

void* GetMediaDisplayDuration(void)
{
    if (verbose) puts("STUB: GetMediaDisplayDuration called");
    return NULL;
}

void* GetMediaDisplayEndTime(void)
{
    if (verbose) puts("STUB: GetMediaDisplayEndTime called");
    return NULL;
}

void* GetMediaDisplayStartTime(void)
{
    if (verbose) puts("STUB: GetMediaDisplayStartTime called");
    return NULL;
}

void* GetMediaDuration(void)
{
    if (verbose) puts("STUB: GetMediaDuration called");
    return NULL;
}

void* GetMediaHandler(void)
{
    if (verbose) puts("STUB: GetMediaHandler called");
    return NULL;
}

void* GetMediaHandlerDescription(void)
{
    if (verbose) puts("STUB: GetMediaHandlerDescription called");
    return NULL;
}

void* GetMediaInputMap(void)
{
    if (verbose) puts("STUB: GetMediaInputMap called");
    return NULL;
}

void* GetMediaLanguage(void)
{
    if (verbose) puts("STUB: GetMediaLanguage called");
    return NULL;
}

void* GetMediaModificationTime(void)
{
    if (verbose) puts("STUB: GetMediaModificationTime called");
    return NULL;
}

void* GetMediaNextInterestingDecodeTime(void)
{
    if (verbose) puts("STUB: GetMediaNextInterestingDecodeTime called");
    return NULL;
}

void* GetMediaNextInterestingDisplayTime(void)
{
    if (verbose) puts("STUB: GetMediaNextInterestingDisplayTime called");
    return NULL;
}

void* GetMediaNextInterestingTime(void)
{
    if (verbose) puts("STUB: GetMediaNextInterestingTime called");
    return NULL;
}

void* GetMediaPlayHints(void)
{
    if (verbose) puts("STUB: GetMediaPlayHints called");
    return NULL;
}

void* GetMediaPreferredChunkSize(void)
{
    if (verbose) puts("STUB: GetMediaPreferredChunkSize called");
    return NULL;
}

void* GetMediaPropertyAtom(void)
{
    if (verbose) puts("STUB: GetMediaPropertyAtom called");
    return NULL;
}

void* GetMediaQuality(void)
{
    if (verbose) puts("STUB: GetMediaQuality called");
    return NULL;
}

void* GetMediaSample(void)
{
    if (verbose) puts("STUB: GetMediaSample called");
    return NULL;
}

void* GetMediaSample2(void)
{
    if (verbose) puts("STUB: GetMediaSample2 called");
    return NULL;
}

void* GetMediaSampleCount(void)
{
    if (verbose) puts("STUB: GetMediaSampleCount called");
    return NULL;
}

void* GetMediaSampleDescription(void)
{
    if (verbose) puts("STUB: GetMediaSampleDescription called");
    return NULL;
}

void* GetMediaSampleDescriptionCount(void)
{
    if (verbose) puts("STUB: GetMediaSampleDescriptionCount called");
    return NULL;
}

void* GetMediaSampleReference(void)
{
    if (verbose) puts("STUB: GetMediaSampleReference called");
    return NULL;
}

void* GetMediaSampleReferences(void)
{
    if (verbose) puts("STUB: GetMediaSampleReferences called");
    return NULL;
}

void* GetMediaSampleReferences64(void)
{
    if (verbose) puts("STUB: GetMediaSampleReferences64 called");
    return NULL;
}

void* GetMediaSeed(void)
{
    if (verbose) puts("STUB: GetMediaSeed called");
    return NULL;
}

void* GetMediaShadowSync(void)
{
    if (verbose) puts("STUB: GetMediaShadowSync called");
    return NULL;
}

void* GetMediaSyncSampleCount(void)
{
    if (verbose) puts("STUB: GetMediaSyncSampleCount called");
    return NULL;
}

void* GetMediaTimeScale(void)
{
    if (verbose) puts("STUB: GetMediaTimeScale called");
    return NULL;
}

void* GetMediaTrack(void)
{
    if (verbose) puts("STUB: GetMediaTrack called");
    return NULL;
}

void* GetMediaUserData(void)
{
    if (verbose) puts("STUB: GetMediaUserData called");
    return NULL;
}

void* GetMovieActive(void)
{
    if (verbose) puts("STUB: GetMovieActive called");
    return NULL;
}

void* GetMovieActiveSegment(void)
{
    if (verbose) puts("STUB: GetMovieActiveSegment called");
    return NULL;
}

void* GetMovieAnchorDataRef(void)
{
    if (verbose) puts("STUB: GetMovieAnchorDataRef called");
    return NULL;
}

void* GetMovieAudioBalance(void)
{
    if (verbose) puts("STUB: GetMovieAudioBalance called");
    return NULL;
}

void* GetMovieAudioContext(void)
{
    if (verbose) puts("STUB: GetMovieAudioContext called");
    return NULL;
}

void* GetMovieAudioFade(void)
{
    if (verbose) puts("STUB: GetMovieAudioFade called");
    return NULL;
}

void* GetMovieAudioFrequencyLevels(void)
{
    if (verbose) puts("STUB: GetMovieAudioFrequencyLevels called");
    return NULL;
}

void* GetMovieAudioFrequencyMeteringBandFrequencies(void)
{
    if (verbose) puts("STUB: GetMovieAudioFrequencyMeteringBandFrequencies called");
    return NULL;
}

void* GetMovieAudioFrequencyMeteringNumBands(void)
{
    if (verbose) puts("STUB: GetMovieAudioFrequencyMeteringNumBands called");
    return NULL;
}

void* GetMovieAudioGain(void)
{
    if (verbose) puts("STUB: GetMovieAudioGain called");
    return NULL;
}

void* GetMovieAudioMute(void)
{
    if (verbose) puts("STUB: GetMovieAudioMute called");
    return NULL;
}

void* GetMovieAudioVolumeLevels(void)
{
    if (verbose) puts("STUB: GetMovieAudioVolumeLevels called");
    return NULL;
}

void* GetMovieAudioVolumeMeteringEnabled(void)
{
    if (verbose) puts("STUB: GetMovieAudioVolumeMeteringEnabled called");
    return NULL;
}

void* GetMovieBoundsRgn(void)
{
    if (verbose) puts("STUB: GetMovieBoundsRgn called");
    return NULL;
}

void* GetMovieBox(void)
{
    if (verbose) puts("STUB: GetMovieBox called");
    return NULL;
}

void* GetMovieClipRgn(void)
{
    if (verbose) puts("STUB: GetMovieClipRgn called");
    return NULL;
}

void* GetMovieColorTable(void)
{
    if (verbose) puts("STUB: GetMovieColorTable called");
    return NULL;
}

void* GetMovieCompositeBufferFlags(void)
{
    if (verbose) puts("STUB: GetMovieCompositeBufferFlags called");
    return NULL;
}

void* GetMovieCoverProcs(void)
{
    if (verbose) puts("STUB: GetMovieCoverProcs called");
    return NULL;
}

void* GetMovieCreationTime(void)
{
    if (verbose) puts("STUB: GetMovieCreationTime called");
    return NULL;
}

void* GetMovieDataSize(void)
{
    if (verbose) puts("STUB: GetMovieDataSize called");
    return NULL;
}

void* GetMovieDataSize64(void)
{
    if (verbose) puts("STUB: GetMovieDataSize64 called");
    return NULL;
}

void* GetMovieDefaultDataRef(void)
{
    if (verbose) puts("STUB: GetMovieDefaultDataRef called");
    return NULL;
}

void* GetMovieDisplayBoundsRgn(void)
{
    if (verbose) puts("STUB: GetMovieDisplayBoundsRgn called");
    return NULL;
}

void* GetMovieDisplayClipRgn(void)
{
    if (verbose) puts("STUB: GetMovieDisplayClipRgn called");
    return NULL;
}

void* GetMovieDuration(void)
{
    if (verbose) puts("STUB: GetMovieDuration called");
    return NULL;
}

void* GetMovieEffectiveRate(void)
{
    if (verbose) puts("STUB: GetMovieEffectiveRate called");
    return NULL;
}

void* GetMovieGWorld(void)
{
    if (verbose) puts("STUB: GetMovieGWorld called");
    return NULL;
}

void* GetMovieIdleManager(void)
{
    if (verbose) puts("STUB: GetMovieIdleManager called");
    return NULL;
}

void* GetMovieImporterForDataRef(void)
{
    if (verbose) puts("STUB: GetMovieImporterForDataRef called");
    return NULL;
}

void* GetMovieIndTrack(void)
{
    if (verbose) puts("STUB: GetMovieIndTrack called");
    return NULL;
}

void* GetMovieIndTrackType(void)
{
    if (verbose) puts("STUB: GetMovieIndTrackType called");
    return NULL;
}

void* GetMovieLoadState(void)
{
    if (verbose) puts("STUB: GetMovieLoadState called");
    return NULL;
}

void* GetMovieMatrix(void)
{
    if (verbose) puts("STUB: GetMovieMatrix called");
    return NULL;
}

void* GetMovieMaxBitDepth(void)
{
    if (verbose) puts("STUB: GetMovieMaxBitDepth called");
    return NULL;
}

void* GetMovieModificationTime(void)
{
    if (verbose) puts("STUB: GetMovieModificationTime called");
    return NULL;
}

void* GetMovieNaturalBoundsRect(void)
{
    if (verbose) puts("STUB: GetMovieNaturalBoundsRect called");
    return NULL;
}

void* GetMovieNextInterestingTime(void)
{
    if (verbose) puts("STUB: GetMovieNextInterestingTime called");
    return NULL;
}

void* GetMovieParentTrack(void)
{
    if (verbose) puts("STUB: GetMovieParentTrack called");
    return NULL;
}

void* GetMoviePict(void)
{
    if (verbose) puts("STUB: GetMoviePict called");
    return NULL;
}

void* GetMoviePosterPict(void)
{
    if (verbose) puts("STUB: GetMoviePosterPict called");
    return NULL;
}

void* GetMoviePosterTime(void)
{
    if (verbose) puts("STUB: GetMoviePosterTime called");
    return NULL;
}

void* GetMoviePreferredRate(void)
{
    if (verbose) puts("STUB: GetMoviePreferredRate called");
    return NULL;
}

void* GetMoviePreferredVolume(void)
{
    if (verbose) puts("STUB: GetMoviePreferredVolume called");
    return NULL;
}

void* GetMoviePreviewMode(void)
{
    if (verbose) puts("STUB: GetMoviePreviewMode called");
    return NULL;
}

void* GetMoviePreviewTime(void)
{
    if (verbose) puts("STUB: GetMoviePreviewTime called");
    return NULL;
}

void* GetMovieProgressProc(void)
{
    if (verbose) puts("STUB: GetMovieProgressProc called");
    return NULL;
}

void* GetMoviePropertyAtom(void)
{
    if (verbose) puts("STUB: GetMoviePropertyAtom called");
    return NULL;
}

void* GetMovieQTList(void)
{
    if (verbose) puts("STUB: GetMovieQTList called");
    return NULL;
}

void* GetMovieRate(void)
{
    if (verbose) puts("STUB: GetMovieRate called");
    return NULL;
}

void* GetMovieRateChangeConstraints(void)
{
    if (verbose) puts("STUB: GetMovieRateChangeConstraints called");
    return NULL;
}

void* GetMovieSeed(void)
{
    if (verbose) puts("STUB: GetMovieSeed called");
    return NULL;
}

void* GetMovieSegmentDisplayBoundsRgn(void)
{
    if (verbose) puts("STUB: GetMovieSegmentDisplayBoundsRgn called");
    return NULL;
}

void* GetMovieSelection(void)
{
    if (verbose) puts("STUB: GetMovieSelection called");
    return NULL;
}

void* GetMovieStatus(void)
{
    if (verbose) puts("STUB: GetMovieStatus called");
    return NULL;
}

void* GetMovieThreadAttachState(void)
{
    if (verbose) puts("STUB: GetMovieThreadAttachState called");
    return NULL;
}

void* GetMovieTime(void)
{
    if (verbose) puts("STUB: GetMovieTime called");
    return NULL;
}

void* GetMovieTimeBase(void)
{
    if (verbose) puts("STUB: GetMovieTimeBase called");
    return NULL;
}

void* GetMovieTimeScale(void)
{
    if (verbose) puts("STUB: GetMovieTimeScale called");
    return NULL;
}

void* GetMovieTrack(void)
{
    if (verbose) puts("STUB: GetMovieTrack called");
    return NULL;
}

void* GetMovieTrackCount(void)
{
    if (verbose) puts("STUB: GetMovieTrackCount called");
    return NULL;
}

void* GetMovieUserData(void)
{
    if (verbose) puts("STUB: GetMovieUserData called");
    return NULL;
}

void* GetMovieVisualBrightness(void)
{
    if (verbose) puts("STUB: GetMovieVisualBrightness called");
    return NULL;
}

void* GetMovieVisualContext(void)
{
    if (verbose) puts("STUB: GetMovieVisualContext called");
    return NULL;
}

void* GetMovieVisualContrast(void)
{
    if (verbose) puts("STUB: GetMovieVisualContrast called");
    return NULL;
}

void* GetMovieVisualHue(void)
{
    if (verbose) puts("STUB: GetMovieVisualHue called");
    return NULL;
}

void* GetMovieVisualSaturation(void)
{
    if (verbose) puts("STUB: GetMovieVisualSaturation called");
    return NULL;
}

void* GetMovieVolume(void)
{
    if (verbose) puts("STUB: GetMovieVolume called");
    return NULL;
}

void* GetMoviesError(void)
{
    if (verbose) puts("STUB: GetMoviesError called");
    return NULL;
}

void* GetMoviesStickyError(void)
{
    if (verbose) puts("STUB: GetMoviesStickyError called");
    return NULL;
}

void* GetNamedComponentMessageParameter(void)
{
    if (verbose) puts("STUB: GetNamedComponentMessageParameter called");
    return NULL;
}

void* GetNamedComponentMessageParameterInfo(void)
{
    if (verbose) puts("STUB: GetNamedComponentMessageParameterInfo called");
    return NULL;
}

void* GetNextCallBack(void)
{
    if (verbose) puts("STUB: GetNextCallBack called");
    return NULL;
}

void* GetNextImageDescriptionExtensionType(void)
{
    if (verbose) puts("STUB: GetNextImageDescriptionExtensionType called");
    return NULL;
}

void* GetNextTrackForCompositing(void)
{
    if (verbose) puts("STUB: GetNextTrackForCompositing called");
    return NULL;
}

void* GetNextTrackReferenceType(void)
{
    if (verbose) puts("STUB: GetNextTrackReferenceType called");
    return NULL;
}

void* GetNextUserDataType(void)
{
    if (verbose) puts("STUB: GetNextUserDataType called");
    return NULL;
}

void* GetOriginalTrackEnabled(void)
{
    if (verbose) puts("STUB: GetOriginalTrackEnabled called");
    return NULL;
}

void* GetPictureFileHeader(void)
{
    if (verbose) puts("STUB: GetPictureFileHeader called");
    return NULL;
}

void* GetPosterBox(void)
{
    if (verbose) puts("STUB: GetPosterBox called");
    return NULL;
}

void* GetPrevTrackForCompositing(void)
{
    if (verbose) puts("STUB: GetPrevTrackForCompositing called");
    return NULL;
}

void* GetPublicOfSize(void)
{
    if (verbose) puts("STUB: GetPublicOfSize called");
    return NULL;
}

void* GetQTContextProperty(void)
{
    if (verbose) puts("STUB: GetQTContextProperty called");
    return NULL;
}

void* GetQTContextPropertyInfo(void)
{
    if (verbose) puts("STUB: GetQTContextPropertyInfo called");
    return NULL;
}

void* GetQuickTimePreference(void)
{
    if (verbose) puts("STUB: GetQuickTimePreference called");
    return NULL;
}

void* GetQuickTimePreferences(void)
{
    if (verbose) puts("STUB: GetQuickTimePreferences called");
    return NULL;
}

void* GetQuickTimePreferencesForReading(void)
{
    if (verbose) puts("STUB: GetQuickTimePreferencesForReading called");
    return NULL;
}

void* GetQuickTimePreferencesForWriting(void)
{
    if (verbose) puts("STUB: GetQuickTimePreferencesForWriting called");
    return NULL;
}

void* GetSimilarity(void)
{
    if (verbose) puts("STUB: GetSimilarity called");
    return NULL;
}

void* GetSizedPublic(void)
{
    if (verbose) puts("STUB: GetSizedPublic called");
    return NULL;
}

void* GetSoundDescriptionExtension(void)
{
    if (verbose) puts("STUB: GetSoundDescriptionExtension called");
    return NULL;
}

void* GetSpriteProperty(void)
{
    if (verbose) puts("STUB: GetSpriteProperty called");
    return NULL;
}

void* GetTimeBaseEffectiveRate(void)
{
    if (verbose) puts("STUB: GetTimeBaseEffectiveRate called");
    return NULL;
}

void* GetTimeBaseFlags(void)
{
    if (verbose) puts("STUB: GetTimeBaseFlags called");
    return NULL;
}

void* GetTimeBaseMasterClock(void)
{
    if (verbose) puts("STUB: GetTimeBaseMasterClock called");
    return NULL;
}

void* GetTimeBaseMasterOffsetTimeBase(void)
{
    if (verbose) puts("STUB: GetTimeBaseMasterOffsetTimeBase called");
    return NULL;
}

void* GetTimeBaseMasterTimeBase(void)
{
    if (verbose) puts("STUB: GetTimeBaseMasterTimeBase called");
    return NULL;
}

void* GetTimeBasePrerollRate(void)
{
    if (verbose) puts("STUB: GetTimeBasePrerollRate called");
    return NULL;
}

void* GetTimeBaseRate(void)
{
    if (verbose) puts("STUB: GetTimeBaseRate called");
    return NULL;
}

void* GetTimeBaseRateChangeStatus(void)
{
    if (verbose) puts("STUB: GetTimeBaseRateChangeStatus called");
    return NULL;
}

void* GetTimeBaseStartTime(void)
{
    if (verbose) puts("STUB: GetTimeBaseStartTime called");
    return NULL;
}

void* GetTimeBaseStatus(void)
{
    if (verbose) puts("STUB: GetTimeBaseStatus called");
    return NULL;
}

void* GetTimeBaseStopTime(void)
{
    if (verbose) puts("STUB: GetTimeBaseStopTime called");
    return NULL;
}

void* GetTimeBaseThreadAttachState(void)
{
    if (verbose) puts("STUB: GetTimeBaseThreadAttachState called");
    return NULL;
}

void* GetTimeBaseTime(void)
{
    if (verbose) puts("STUB: GetTimeBaseTime called");
    return NULL;
}

void* GetTimeBaseTimesForLastChange(void)
{
    if (verbose) puts("STUB: GetTimeBaseTimesForLastChange called");
    return NULL;
}

void* GetTrackActiveSegment(void)
{
    if (verbose) puts("STUB: GetTrackActiveSegment called");
    return NULL;
}

void* GetTrackAlternate(void)
{
    if (verbose) puts("STUB: GetTrackAlternate called");
    return NULL;
}

void* GetTrackAudioGain(void)
{
    if (verbose) puts("STUB: GetTrackAudioGain called");
    return NULL;
}

void* GetTrackAudioMute(void)
{
    if (verbose) puts("STUB: GetTrackAudioMute called");
    return NULL;
}

void* GetTrackAudioVolumeLevels(void)
{
    if (verbose) puts("STUB: GetTrackAudioVolumeLevels called");
    return NULL;
}

void* GetTrackAudioVolumeMeteringEnabled(void)
{
    if (verbose) puts("STUB: GetTrackAudioVolumeMeteringEnabled called");
    return NULL;
}

void* GetTrackBoundsRgn(void)
{
    if (verbose) puts("STUB: GetTrackBoundsRgn called");
    return NULL;
}

void* GetTrackClipRgn(void)
{
    if (verbose) puts("STUB: GetTrackClipRgn called");
    return NULL;
}

void* GetTrackCreationTime(void)
{
    if (verbose) puts("STUB: GetTrackCreationTime called");
    return NULL;
}

void* GetTrackDataSize(void)
{
    if (verbose) puts("STUB: GetTrackDataSize called");
    return NULL;
}

void* GetTrackDataSize64(void)
{
    if (verbose) puts("STUB: GetTrackDataSize64 called");
    return NULL;
}

void* GetTrackDimensions(void)
{
    if (verbose) puts("STUB: GetTrackDimensions called");
    return NULL;
}

void* GetTrackDisplayBoundsRgn(void)
{
    if (verbose) puts("STUB: GetTrackDisplayBoundsRgn called");
    return NULL;
}

void* GetTrackDisplayMatrix(void)
{
    if (verbose) puts("STUB: GetTrackDisplayMatrix called");
    return NULL;
}

void* GetTrackDisplayRgn(void)
{
    if (verbose) puts("STUB: GetTrackDisplayRgn called");
    return NULL;
}

void* GetTrackDuration(void)
{
    if (verbose) puts("STUB: GetTrackDuration called");
    return NULL;
}

void* GetTrackEditCount(void)
{
    if (verbose) puts("STUB: GetTrackEditCount called");
    return NULL;
}

void* GetTrackEditIndex(void)
{
    if (verbose) puts("STUB: GetTrackEditIndex called");
    return NULL;
}

void* GetTrackEditInfo(void)
{
    if (verbose) puts("STUB: GetTrackEditInfo called");
    return NULL;
}

void* GetTrackEditRate(void)
{
    if (verbose) puts("STUB: GetTrackEditRate called");
    return NULL;
}

void* GetTrackEditRate64(void)
{
    if (verbose) puts("STUB: GetTrackEditRate64 called");
    return NULL;
}

void* GetTrackEnabled(void)
{
    if (verbose) puts("STUB: GetTrackEnabled called");
    return NULL;
}

void* GetTrackID(void)
{
    if (verbose) puts("STUB: GetTrackID called");
    return NULL;
}

void* GetTrackIndReferenced(void)
{
    if (verbose) puts("STUB: GetTrackIndReferenced called");
    return NULL;
}

void* GetTrackLayer(void)
{
    if (verbose) puts("STUB: GetTrackLayer called");
    return NULL;
}

void* GetTrackLoadSettings(void)
{
    if (verbose) puts("STUB: GetTrackLoadSettings called");
    return NULL;
}

void* GetTrackMatrix(void)
{
    if (verbose) puts("STUB: GetTrackMatrix called");
    return NULL;
}

void* GetTrackMatte(void)
{
    if (verbose) puts("STUB: GetTrackMatte called");
    return NULL;
}

void* GetTrackMedia(void)
{
    if (verbose) puts("STUB: GetTrackMedia called");
    return NULL;
}

void* GetTrackModificationTime(void)
{
    if (verbose) puts("STUB: GetTrackModificationTime called");
    return NULL;
}

void* GetTrackMovie(void)
{
    if (verbose) puts("STUB: GetTrackMovie called");
    return NULL;
}

void* GetTrackMovieBoundsRgn(void)
{
    if (verbose) puts("STUB: GetTrackMovieBoundsRgn called");
    return NULL;
}

void* GetTrackNextInterestingTime(void)
{
    if (verbose) puts("STUB: GetTrackNextInterestingTime called");
    return NULL;
}

void* GetTrackOffset(void)
{
    if (verbose) puts("STUB: GetTrackOffset called");
    return NULL;
}

void* GetTrackPict(void)
{
    if (verbose) puts("STUB: GetTrackPict called");
    return NULL;
}

void* GetTrackReference(void)
{
    if (verbose) puts("STUB: GetTrackReference called");
    return NULL;
}

void* GetTrackReferenceCount(void)
{
    if (verbose) puts("STUB: GetTrackReferenceCount called");
    return NULL;
}

void* GetTrackSeed(void)
{
    if (verbose) puts("STUB: GetTrackSeed called");
    return NULL;
}

void* GetTrackSegmentDisplayBoundsRgn(void)
{
    if (verbose) puts("STUB: GetTrackSegmentDisplayBoundsRgn called");
    return NULL;
}

void* GetTrackSoundLocalizationSettings(void)
{
    if (verbose) puts("STUB: GetTrackSoundLocalizationSettings called");
    return NULL;
}

void* GetTrackStatus(void)
{
    if (verbose) puts("STUB: GetTrackStatus called");
    return NULL;
}

void* GetTrackUsage(void)
{
    if (verbose) puts("STUB: GetTrackUsage called");
    return NULL;
}

void* GetTrackUserData(void)
{
    if (verbose) puts("STUB: GetTrackUserData called");
    return NULL;
}

void* GetTrackVolume(void)
{
    if (verbose) puts("STUB: GetTrackVolume called");
    return NULL;
}

void* GetUUIDImageDescriptionExtension(void)
{
    if (verbose) puts("STUB: GetUUIDImageDescriptionExtension called");
    return NULL;
}

void* GetUUIDSoundDescriptionExtension(void)
{
    if (verbose) puts("STUB: GetUUIDSoundDescriptionExtension called");
    return NULL;
}

void* GetUserData(void)
{
    if (verbose) puts("STUB: GetUserData called");
    return NULL;
}

void* GetUserDataAtom(void)
{
    if (verbose) puts("STUB: GetUserDataAtom called");
    return NULL;
}

void* GetUserDataItem(void)
{
    if (verbose) puts("STUB: GetUserDataItem called");
    return NULL;
}

void* GetUserDataText(void)
{
    if (verbose) puts("STUB: GetUserDataText called");
    return NULL;
}

void* GetWarholVersion(void)
{
    if (verbose) puts("STUB: GetWarholVersion called");
    return NULL;
}

void* GoToBeginningOfMovie(void)
{
    if (verbose) puts("STUB: GoToBeginningOfMovie called");
    return NULL;
}

void* GoToEndOfMovie(void)
{
    if (verbose) puts("STUB: GoToEndOfMovie called");
    return NULL;
}

void* GraphicsExportCanTranscode(void)
{
    if (verbose) puts("STUB: GraphicsExportCanTranscode called");
    return NULL;
}

void* GraphicsExportCanUseCompressor(void)
{
    if (verbose) puts("STUB: GraphicsExportCanUseCompressor called");
    return NULL;
}

void* GraphicsExportDoExport(void)
{
    if (verbose) puts("STUB: GraphicsExportDoExport called");
    return NULL;
}

void* GraphicsExportDoStandaloneExport(void)
{
    if (verbose) puts("STUB: GraphicsExportDoStandaloneExport called");
    return NULL;
}

void* GraphicsExportDoTranscode(void)
{
    if (verbose) puts("STUB: GraphicsExportDoTranscode called");
    return NULL;
}

void* GraphicsExportDoUseCompressor(void)
{
    if (verbose) puts("STUB: GraphicsExportDoUseCompressor called");
    return NULL;
}

void* GraphicsExportDrawInputImage(void)
{
    if (verbose) puts("STUB: GraphicsExportDrawInputImage called");
    return NULL;
}

void* GraphicsExportGetColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsExportGetColorSyncProfile called");
    return NULL;
}

void* GraphicsExportGetColorTable(void)
{
    if (verbose) puts("STUB: GraphicsExportGetColorTable called");
    return NULL;
}

void* GraphicsExportGetCompressionMethod(void)
{
    if (verbose) puts("STUB: GraphicsExportGetCompressionMethod called");
    return NULL;
}

void* GraphicsExportGetCompressionQuality(void)
{
    if (verbose) puts("STUB: GraphicsExportGetCompressionQuality called");
    return NULL;
}

void* GraphicsExportGetDefaultFileNameExtension(void)
{
    if (verbose) puts("STUB: GraphicsExportGetDefaultFileNameExtension called");
    return NULL;
}

void* GraphicsExportGetDefaultFileTypeAndCreator(void)
{
    if (verbose) puts("STUB: GraphicsExportGetDefaultFileTypeAndCreator called");
    return NULL;
}

void* GraphicsExportGetDepth(void)
{
    if (verbose) puts("STUB: GraphicsExportGetDepth called");
    return NULL;
}

void* GraphicsExportGetDontRecompress(void)
{
    if (verbose) puts("STUB: GraphicsExportGetDontRecompress called");
    return NULL;
}

void* GraphicsExportGetExifEnabled(void)
{
    if (verbose) puts("STUB: GraphicsExportGetExifEnabled called");
    return NULL;
}

void* GraphicsExportGetFlags(void)
{
    if (verbose) puts("STUB: GraphicsExportGetFlags called");
    return NULL;
}

void* GraphicsExportGetInputCGBitmapContext(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputCGBitmapContext called");
    return NULL;
}

void* GraphicsExportGetInputCGImage(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputCGImage called");
    return NULL;
}

void* GraphicsExportGetInputDataReference(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputDataReference called");
    return NULL;
}

void* GraphicsExportGetInputDataSize(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputDataSize called");
    return NULL;
}

void* GraphicsExportGetInputFile(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputFile called");
    return NULL;
}

void* GraphicsExportGetInputGWorld(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputGWorld called");
    return NULL;
}

void* GraphicsExportGetInputGraphicsImporter(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputGraphicsImporter called");
    return NULL;
}

void* GraphicsExportGetInputHandle(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputHandle called");
    return NULL;
}

void* GraphicsExportGetInputImageDepth(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputImageDepth called");
    return NULL;
}

void* GraphicsExportGetInputImageDescription(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputImageDescription called");
    return NULL;
}

void* GraphicsExportGetInputImageDimensions(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputImageDimensions called");
    return NULL;
}

void* GraphicsExportGetInputOffsetAndLimit(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputOffsetAndLimit called");
    return NULL;
}

void* GraphicsExportGetInputPicture(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputPicture called");
    return NULL;
}

void* GraphicsExportGetInputPixmap(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputPixmap called");
    return NULL;
}

void* GraphicsExportGetInputPtr(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInputPtr called");
    return NULL;
}

void* GraphicsExportGetInterlaceStyle(void)
{
    if (verbose) puts("STUB: GraphicsExportGetInterlaceStyle called");
    return NULL;
}

void* GraphicsExportGetMIMETypeList(void)
{
    if (verbose) puts("STUB: GraphicsExportGetMIMETypeList called");
    return NULL;
}

void* GraphicsExportGetMetaData(void)
{
    if (verbose) puts("STUB: GraphicsExportGetMetaData called");
    return NULL;
}

void* GraphicsExportGetOutputDataReference(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputDataReference called");
    return NULL;
}

void* GraphicsExportGetOutputFile(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputFile called");
    return NULL;
}

void* GraphicsExportGetOutputFileTypeAndCreator(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputFileTypeAndCreator called");
    return NULL;
}

void* GraphicsExportGetOutputHandle(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputHandle called");
    return NULL;
}

void* GraphicsExportGetOutputMark(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputMark called");
    return NULL;
}

void* GraphicsExportGetOutputOffsetAndMaxSize(void)
{
    if (verbose) puts("STUB: GraphicsExportGetOutputOffsetAndMaxSize called");
    return NULL;
}

void* GraphicsExportGetProgressProc(void)
{
    if (verbose) puts("STUB: GraphicsExportGetProgressProc called");
    return NULL;
}

void* GraphicsExportGetResolution(void)
{
    if (verbose) puts("STUB: GraphicsExportGetResolution called");
    return NULL;
}

void* GraphicsExportGetSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: GraphicsExportGetSettingsAsAtomContainer called");
    return NULL;
}

void* GraphicsExportGetSettingsAsText(void)
{
    if (verbose) puts("STUB: GraphicsExportGetSettingsAsText called");
    return NULL;
}

void* GraphicsExportGetTargetDataSize(void)
{
    if (verbose) puts("STUB: GraphicsExportGetTargetDataSize called");
    return NULL;
}

void* GraphicsExportGetThumbnailEnabled(void)
{
    if (verbose) puts("STUB: GraphicsExportGetThumbnailEnabled called");
    return NULL;
}

void* GraphicsExportMayExporterReadInputData(void)
{
    if (verbose) puts("STUB: GraphicsExportMayExporterReadInputData called");
    return NULL;
}

void* GraphicsExportReadInputData(void)
{
    if (verbose) puts("STUB: GraphicsExportReadInputData called");
    return NULL;
}

void* GraphicsExportReadOutputData(void)
{
    if (verbose) puts("STUB: GraphicsExportReadOutputData called");
    return NULL;
}

void* GraphicsExportRequestSettings(void)
{
    if (verbose) puts("STUB: GraphicsExportRequestSettings called");
    return NULL;
}

void* GraphicsExportSetColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsExportSetColorSyncProfile called");
    return NULL;
}

void* GraphicsExportSetColorTable(void)
{
    if (verbose) puts("STUB: GraphicsExportSetColorTable called");
    return NULL;
}

void* GraphicsExportSetCompressionMethod(void)
{
    if (verbose) puts("STUB: GraphicsExportSetCompressionMethod called");
    return NULL;
}

void* GraphicsExportSetCompressionQuality(void)
{
    if (verbose) puts("STUB: GraphicsExportSetCompressionQuality called");
    return NULL;
}

void* GraphicsExportSetDepth(void)
{
    if (verbose) puts("STUB: GraphicsExportSetDepth called");
    return NULL;
}

void* GraphicsExportSetDontRecompress(void)
{
    if (verbose) puts("STUB: GraphicsExportSetDontRecompress called");
    return NULL;
}

void* GraphicsExportSetExifEnabled(void)
{
    if (verbose) puts("STUB: GraphicsExportSetExifEnabled called");
    return NULL;
}

void* GraphicsExportSetFlags(void)
{
    if (verbose) puts("STUB: GraphicsExportSetFlags called");
    return NULL;
}

void* GraphicsExportSetInputCGBitmapContext(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputCGBitmapContext called");
    return NULL;
}

void* GraphicsExportSetInputCGImage(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputCGImage called");
    return NULL;
}

void* GraphicsExportSetInputDataReference(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputDataReference called");
    return NULL;
}

void* GraphicsExportSetInputFile(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputFile called");
    return NULL;
}

void* GraphicsExportSetInputGWorld(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputGWorld called");
    return NULL;
}

void* GraphicsExportSetInputGraphicsImporter(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputGraphicsImporter called");
    return NULL;
}

void* GraphicsExportSetInputHandle(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputHandle called");
    return NULL;
}

void* GraphicsExportSetInputOffsetAndLimit(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputOffsetAndLimit called");
    return NULL;
}

void* GraphicsExportSetInputPicture(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputPicture called");
    return NULL;
}

void* GraphicsExportSetInputPixmap(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputPixmap called");
    return NULL;
}

void* GraphicsExportSetInputPtr(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInputPtr called");
    return NULL;
}

void* GraphicsExportSetInterlaceStyle(void)
{
    if (verbose) puts("STUB: GraphicsExportSetInterlaceStyle called");
    return NULL;
}

void* GraphicsExportSetMetaData(void)
{
    if (verbose) puts("STUB: GraphicsExportSetMetaData called");
    return NULL;
}

void* GraphicsExportSetOutputDataReference(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputDataReference called");
    return NULL;
}

void* GraphicsExportSetOutputFile(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputFile called");
    return NULL;
}

void* GraphicsExportSetOutputFileTypeAndCreator(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputFileTypeAndCreator called");
    return NULL;
}

void* GraphicsExportSetOutputHandle(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputHandle called");
    return NULL;
}

void* GraphicsExportSetOutputMark(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputMark called");
    return NULL;
}

void* GraphicsExportSetOutputOffsetAndMaxSize(void)
{
    if (verbose) puts("STUB: GraphicsExportSetOutputOffsetAndMaxSize called");
    return NULL;
}

void* GraphicsExportSetProgressProc(void)
{
    if (verbose) puts("STUB: GraphicsExportSetProgressProc called");
    return NULL;
}

void* GraphicsExportSetResolution(void)
{
    if (verbose) puts("STUB: GraphicsExportSetResolution called");
    return NULL;
}

void* GraphicsExportSetSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: GraphicsExportSetSettingsFromAtomContainer called");
    return NULL;
}

void* GraphicsExportSetTargetDataSize(void)
{
    if (verbose) puts("STUB: GraphicsExportSetTargetDataSize called");
    return NULL;
}

void* GraphicsExportSetThumbnailEnabled(void)
{
    if (verbose) puts("STUB: GraphicsExportSetThumbnailEnabled called");
    return NULL;
}

void* GraphicsExportWriteOutputData(void)
{
    if (verbose) puts("STUB: GraphicsExportWriteOutputData called");
    return NULL;
}

void* GraphicsImageImportGetSequenceEnabled(void)
{
    if (verbose) puts("STUB: GraphicsImageImportGetSequenceEnabled called");
    return NULL;
}

void* GraphicsImageImportSetSequenceEnabled(void)
{
    if (verbose) puts("STUB: GraphicsImageImportSetSequenceEnabled called");
    return NULL;
}

void* GraphicsImportCreateCGImage(void)
{
    if (verbose) puts("STUB: GraphicsImportCreateCGImage called");
    return NULL;
}

void* GraphicsImportDoExportImageFileDialog(void)
{
    if (verbose) puts("STUB: GraphicsImportDoExportImageFileDialog called");
    return NULL;
}

void* GraphicsImportDoExportImageFileToDataRefDialog(void)
{
    if (verbose) puts("STUB: GraphicsImportDoExportImageFileToDataRefDialog called");
    return NULL;
}

void* GraphicsImportDoesDrawAllPixels(void)
{
    if (verbose) puts("STUB: GraphicsImportDoesDrawAllPixels called");
    return NULL;
}

void* GraphicsImportDraw(void)
{
    if (verbose) puts("STUB: GraphicsImportDraw called");
    return NULL;
}

void* GraphicsImportExportImageFile(void)
{
    if (verbose) puts("STUB: GraphicsImportExportImageFile called");
    return NULL;
}

void* GraphicsImportExportImageFileToDataRef(void)
{
    if (verbose) puts("STUB: GraphicsImportExportImageFileToDataRef called");
    return NULL;
}

void* GraphicsImportGetAliasedDataReference(void)
{
    if (verbose) puts("STUB: GraphicsImportGetAliasedDataReference called");
    return NULL;
}

void* GraphicsImportGetAsPicture(void)
{
    if (verbose) puts("STUB: GraphicsImportGetAsPicture called");
    return NULL;
}

void* GraphicsImportGetBaseDataOffsetAndSize64(void)
{
    if (verbose) puts("STUB: GraphicsImportGetBaseDataOffsetAndSize64 called");
    return NULL;
}

void* GraphicsImportGetBoundsRect(void)
{
    if (verbose) puts("STUB: GraphicsImportGetBoundsRect called");
    return NULL;
}

void* GraphicsImportGetClip(void)
{
    if (verbose) puts("STUB: GraphicsImportGetClip called");
    return NULL;
}

void* GraphicsImportGetColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsImportGetColorSyncProfile called");
    return NULL;
}

void* GraphicsImportGetDataFile(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataFile called");
    return NULL;
}

void* GraphicsImportGetDataHandle(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataHandle called");
    return NULL;
}

void* GraphicsImportGetDataOffsetAndSize(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataOffsetAndSize called");
    return NULL;
}

void* GraphicsImportGetDataOffsetAndSize64(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataOffsetAndSize64 called");
    return NULL;
}

void* GraphicsImportGetDataReference(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataReference called");
    return NULL;
}

void* GraphicsImportGetDataReferenceOffsetAndLimit(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataReferenceOffsetAndLimit called");
    return NULL;
}

void* GraphicsImportGetDataReferenceOffsetAndLimit64(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDataReferenceOffsetAndLimit64 called");
    return NULL;
}

void* GraphicsImportGetDefaultClip(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDefaultClip called");
    return NULL;
}

void* GraphicsImportGetDefaultGraphicsMode(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDefaultGraphicsMode called");
    return NULL;
}

void* GraphicsImportGetDefaultMatrix(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDefaultMatrix called");
    return NULL;
}

void* GraphicsImportGetDefaultSourceRect(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDefaultSourceRect called");
    return NULL;
}

void* GraphicsImportGetDestRect(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDestRect called");
    return NULL;
}

void* GraphicsImportGetDestinationColorSyncProfileRef(void)
{
    if (verbose) puts("STUB: GraphicsImportGetDestinationColorSyncProfileRef called");
    return NULL;
}

void* GraphicsImportGetExportImageTypeList(void)
{
    if (verbose) puts("STUB: GraphicsImportGetExportImageTypeList called");
    return NULL;
}

void* GraphicsImportGetExportSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: GraphicsImportGetExportSettingsAsAtomContainer called");
    return NULL;
}

void* GraphicsImportGetFlags(void)
{
    if (verbose) puts("STUB: GraphicsImportGetFlags called");
    return NULL;
}

void* GraphicsImportGetGWorld(void)
{
    if (verbose) puts("STUB: GraphicsImportGetGWorld called");
    return NULL;
}

void* GraphicsImportGetGenericColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsImportGetGenericColorSyncProfile called");
    return NULL;
}

void* GraphicsImportGetGraphicsMode(void)
{
    if (verbose) puts("STUB: GraphicsImportGetGraphicsMode called");
    return NULL;
}

void* GraphicsImportGetImageCount(void)
{
    if (verbose) puts("STUB: GraphicsImportGetImageCount called");
    return NULL;
}

void* GraphicsImportGetImageDescription(void)
{
    if (verbose) puts("STUB: GraphicsImportGetImageDescription called");
    return NULL;
}

void* GraphicsImportGetImageIndex(void)
{
    if (verbose) puts("STUB: GraphicsImportGetImageIndex called");
    return NULL;
}

void* GraphicsImportGetMIMETypeList(void)
{
    if (verbose) puts("STUB: GraphicsImportGetMIMETypeList called");
    return NULL;
}

void* GraphicsImportGetMatrix(void)
{
    if (verbose) puts("STUB: GraphicsImportGetMatrix called");
    return NULL;
}

void* GraphicsImportGetMetaData(void)
{
    if (verbose) puts("STUB: GraphicsImportGetMetaData called");
    return NULL;
}

void* GraphicsImportGetNaturalBounds(void)
{
    if (verbose) puts("STUB: GraphicsImportGetNaturalBounds called");
    return NULL;
}

void* GraphicsImportGetOverrideSourceColorSyncProfileRef(void)
{
    if (verbose) puts("STUB: GraphicsImportGetOverrideSourceColorSyncProfileRef called");
    return NULL;
}

void* GraphicsImportGetPlaybackTimeHints(void)
{
    if (verbose) puts("STUB: GraphicsImportGetPlaybackTimeHints called");
    return NULL;
}

void* GraphicsImportGetProgressProc(void)
{
    if (verbose) puts("STUB: GraphicsImportGetProgressProc called");
    return NULL;
}

void* GraphicsImportGetQuality(void)
{
    if (verbose) puts("STUB: GraphicsImportGetQuality called");
    return NULL;
}

void* GraphicsImportGetReturnGenericColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsImportGetReturnGenericColorSyncProfile called");
    return NULL;
}

void* GraphicsImportGetSourceRect(void)
{
    if (verbose) puts("STUB: GraphicsImportGetSourceRect called");
    return NULL;
}

void* GraphicsImportReadData(void)
{
    if (verbose) puts("STUB: GraphicsImportReadData called");
    return NULL;
}

void* GraphicsImportReadData64(void)
{
    if (verbose) puts("STUB: GraphicsImportReadData64 called");
    return NULL;
}

void* GraphicsImportSaveAsPicture(void)
{
    if (verbose) puts("STUB: GraphicsImportSaveAsPicture called");
    return NULL;
}

void* GraphicsImportSaveAsPictureToDataRef(void)
{
    if (verbose) puts("STUB: GraphicsImportSaveAsPictureToDataRef called");
    return NULL;
}

void* GraphicsImportSaveAsQuickTimeImageFile(void)
{
    if (verbose) puts("STUB: GraphicsImportSaveAsQuickTimeImageFile called");
    return NULL;
}

void* GraphicsImportSaveAsQuickTimeImageFileToDataRef(void)
{
    if (verbose) puts("STUB: GraphicsImportSaveAsQuickTimeImageFileToDataRef called");
    return NULL;
}

void* GraphicsImportSetBoundsRect(void)
{
    if (verbose) puts("STUB: GraphicsImportSetBoundsRect called");
    return NULL;
}

void* GraphicsImportSetClip(void)
{
    if (verbose) puts("STUB: GraphicsImportSetClip called");
    return NULL;
}

void* GraphicsImportSetDataFile(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDataFile called");
    return NULL;
}

void* GraphicsImportSetDataHandle(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDataHandle called");
    return NULL;
}

void* GraphicsImportSetDataReference(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDataReference called");
    return NULL;
}

void* GraphicsImportSetDataReferenceOffsetAndLimit(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDataReferenceOffsetAndLimit called");
    return NULL;
}

void* GraphicsImportSetDataReferenceOffsetAndLimit64(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDataReferenceOffsetAndLimit64 called");
    return NULL;
}

void* GraphicsImportSetDestRect(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDestRect called");
    return NULL;
}

void* GraphicsImportSetDestinationColorSyncProfileRef(void)
{
    if (verbose) puts("STUB: GraphicsImportSetDestinationColorSyncProfileRef called");
    return NULL;
}

void* GraphicsImportSetExportSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: GraphicsImportSetExportSettingsFromAtomContainer called");
    return NULL;
}

void* GraphicsImportSetFlags(void)
{
    if (verbose) puts("STUB: GraphicsImportSetFlags called");
    return NULL;
}

void* GraphicsImportSetGWorld(void)
{
    if (verbose) puts("STUB: GraphicsImportSetGWorld called");
    return NULL;
}

void* GraphicsImportSetGraphicsMode(void)
{
    if (verbose) puts("STUB: GraphicsImportSetGraphicsMode called");
    return NULL;
}

void* GraphicsImportSetImageIndex(void)
{
    if (verbose) puts("STUB: GraphicsImportSetImageIndex called");
    return NULL;
}

void* GraphicsImportSetImageIndexToThumbnail(void)
{
    if (verbose) puts("STUB: GraphicsImportSetImageIndexToThumbnail called");
    return NULL;
}

void* GraphicsImportSetMatrix(void)
{
    if (verbose) puts("STUB: GraphicsImportSetMatrix called");
    return NULL;
}

void* GraphicsImportSetOverrideSourceColorSyncProfileRef(void)
{
    if (verbose) puts("STUB: GraphicsImportSetOverrideSourceColorSyncProfileRef called");
    return NULL;
}

void* GraphicsImportSetPlaybackTimeHints(void)
{
    if (verbose) puts("STUB: GraphicsImportSetPlaybackTimeHints called");
    return NULL;
}

void* GraphicsImportSetProgressProc(void)
{
    if (verbose) puts("STUB: GraphicsImportSetProgressProc called");
    return NULL;
}

void* GraphicsImportSetQuality(void)
{
    if (verbose) puts("STUB: GraphicsImportSetQuality called");
    return NULL;
}

void* GraphicsImportSetReturnGenericColorSyncProfile(void)
{
    if (verbose) puts("STUB: GraphicsImportSetReturnGenericColorSyncProfile called");
    return NULL;
}

void* GraphicsImportSetSourceRect(void)
{
    if (verbose) puts("STUB: GraphicsImportSetSourceRect called");
    return NULL;
}

void* GraphicsImportValidate(void)
{
    if (verbose) puts("STUB: GraphicsImportValidate called");
    return NULL;
}

void* GraphicsImportWillUseColorMatching(void)
{
    if (verbose) puts("STUB: GraphicsImportWillUseColorMatching called");
    return NULL;
}

void* HIMovieViewChangeAttributes(void)
{
    if (verbose) puts("STUB: HIMovieViewChangeAttributes called");
    return NULL;
}

void* HIMovieViewCreate(void)
{
    if (verbose) puts("STUB: HIMovieViewCreate called");
    return NULL;
}

void* HIMovieViewGetAttributes(void)
{
    if (verbose) puts("STUB: HIMovieViewGetAttributes called");
    return NULL;
}

void* HIMovieViewGetControllerBarSize(void)
{
    if (verbose) puts("STUB: HIMovieViewGetControllerBarSize called");
    return NULL;
}

void* HIMovieViewGetMovie(void)
{
    if (verbose) puts("STUB: HIMovieViewGetMovie called");
    return NULL;
}

void* HIMovieViewGetMovieController(void)
{
    if (verbose) puts("STUB: HIMovieViewGetMovieController called");
    return NULL;
}

void* HIMovieViewPause(void)
{
    if (verbose) puts("STUB: HIMovieViewPause called");
    return NULL;
}

void* HIMovieViewPlay(void)
{
    if (verbose) puts("STUB: HIMovieViewPlay called");
    return NULL;
}

void* HIMovieViewRegister(void)
{
    if (verbose) puts("STUB: HIMovieViewRegister called");
    return NULL;
}

void* HIMovieViewSetMovie(void)
{
    if (verbose) puts("STUB: HIMovieViewSetMovie called");
    return NULL;
}

void* HIMovieViewUnregister(void)
{
    if (verbose) puts("STUB: HIMovieViewUnregister called");
    return NULL;
}

void* HasMovieChanged(void)
{
    if (verbose) puts("STUB: HasMovieChanged called");
    return NULL;
}

void* HitTestDSequenceData(void)
{
    if (verbose) puts("STUB: HitTestDSequenceData called");
    return NULL;
}

void* ICMCheckColorSyncProfileColorSpace(void)
{
    if (verbose) puts("STUB: ICMCheckColorSyncProfileColorSpace called");
    return NULL;
}

void* ICMCompressionFrameOptionsCreate(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsCreate called");
    return NULL;
}

void* ICMCompressionFrameOptionsCreateCopy(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsCreateCopy called");
    return NULL;
}

void* ICMCompressionFrameOptionsGetForceKeyFrame(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsGetForceKeyFrame called");
    return NULL;
}

void* ICMCompressionFrameOptionsGetFrameType(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsGetFrameType called");
    return NULL;
}

void* ICMCompressionFrameOptionsGetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsGetProperty called");
    return NULL;
}

void* ICMCompressionFrameOptionsGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsGetPropertyInfo called");
    return NULL;
}

void* ICMCompressionFrameOptionsGetTypeID(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsGetTypeID called");
    return NULL;
}

void* ICMCompressionFrameOptionsRelease(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsRelease called");
    return NULL;
}

void* ICMCompressionFrameOptionsRetain(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsRetain called");
    return NULL;
}

void* ICMCompressionFrameOptionsSetForceKeyFrame(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsSetForceKeyFrame called");
    return NULL;
}

void* ICMCompressionFrameOptionsSetFrameType(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsSetFrameType called");
    return NULL;
}

void* ICMCompressionFrameOptionsSetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionFrameOptionsSetProperty called");
    return NULL;
}

void* ICMCompressionSessionBeginPass(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionBeginPass called");
    return NULL;
}

void* ICMCompressionSessionCompleteFrames(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionCompleteFrames called");
    return NULL;
}

void* ICMCompressionSessionCreate(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionCreate called");
    return NULL;
}

void* ICMCompressionSessionEncodeFrame(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionEncodeFrame called");
    return NULL;
}

void* ICMCompressionSessionEndPass(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionEndPass called");
    return NULL;
}

void* ICMCompressionSessionGetImageDescription(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetImageDescription called");
    return NULL;
}

void* ICMCompressionSessionGetPixelBufferPool(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetPixelBufferPool called");
    return NULL;
}

void* ICMCompressionSessionGetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetProperty called");
    return NULL;
}

void* ICMCompressionSessionGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetPropertyInfo called");
    return NULL;
}

void* ICMCompressionSessionGetTimeScale(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetTimeScale called");
    return NULL;
}

void* ICMCompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionGetTypeID called");
    return NULL;
}

void* ICMCompressionSessionOptionsCreate(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsCreate called");
    return NULL;
}

void* ICMCompressionSessionOptionsCreateCopy(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsCreateCopy called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetAllowFrameReordering(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetAllowFrameReordering called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetAllowFrameTimeChanges(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetAllowFrameTimeChanges called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetAllowTemporalCompression(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetAllowTemporalCompression called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetDurationsNeeded(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetDurationsNeeded called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetMaxKeyFrameInterval(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetMaxKeyFrameInterval called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetProperty called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetPropertyInfo called");
    return NULL;
}

void* ICMCompressionSessionOptionsGetTypeID(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsGetTypeID called");
    return NULL;
}

void* ICMCompressionSessionOptionsRelease(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsRelease called");
    return NULL;
}

void* ICMCompressionSessionOptionsRetain(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsRetain called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetAllowFrameReordering(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetAllowFrameReordering called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetAllowFrameTimeChanges(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetAllowFrameTimeChanges called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetAllowTemporalCompression(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetAllowTemporalCompression called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetDurationsNeeded(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetDurationsNeeded called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetMaxKeyFrameInterval(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetMaxKeyFrameInterval called");
    return NULL;
}

void* ICMCompressionSessionOptionsSetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionOptionsSetProperty called");
    return NULL;
}

void* ICMCompressionSessionProcessBetweenPasses(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionProcessBetweenPasses called");
    return NULL;
}

void* ICMCompressionSessionRelease(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionRelease called");
    return NULL;
}

void* ICMCompressionSessionRetain(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionRetain called");
    return NULL;
}

void* ICMCompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionSetProperty called");
    return NULL;
}

void* ICMCompressionSessionSmash(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionSmash called");
    return NULL;
}

void* ICMCompressionSessionSupportsMultiPassEncoding(void)
{
    if (verbose) puts("STUB: ICMCompressionSessionSupportsMultiPassEncoding called");
    return NULL;
}

void* ICMCompressorSessionDropFrame(void)
{
    if (verbose) puts("STUB: ICMCompressorSessionDropFrame called");
    return NULL;
}

void* ICMCompressorSessionEmitEncodedFrame(void)
{
    if (verbose) puts("STUB: ICMCompressorSessionEmitEncodedFrame called");
    return NULL;
}

void* ICMCompressorSourceFrameDetachPixelBuffer(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameDetachPixelBuffer called");
    return NULL;
}

void* ICMCompressorSourceFrameGetDisplayNumber(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameGetDisplayNumber called");
    return NULL;
}

void* ICMCompressorSourceFrameGetDisplayTimeStampAndDuration(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameGetDisplayTimeStampAndDuration called");
    return NULL;
}

void* ICMCompressorSourceFrameGetFrameOptions(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameGetFrameOptions called");
    return NULL;
}

void* ICMCompressorSourceFrameGetPixelBuffer(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameGetPixelBuffer called");
    return NULL;
}

void* ICMCompressorSourceFrameGetTypeID(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameGetTypeID called");
    return NULL;
}

void* ICMCompressorSourceFrameRelease(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameRelease called");
    return NULL;
}

void* ICMCompressorSourceFrameRetain(void)
{
    if (verbose) puts("STUB: ICMCompressorSourceFrameRetain called");
    return NULL;
}

void* ICMCopyFromPixelBufferToPixelBufferWithHelper(void)
{
    if (verbose) puts("STUB: ICMCopyFromPixelBufferToPixelBufferWithHelper called");
    return NULL;
}

void* ICMCreateCVPixelBufferForImageData(void)
{
    if (verbose) puts("STUB: ICMCreateCVPixelBufferForImageData called");
    return NULL;
}

void* ICMCreatePixelConversionHelper(void)
{
    if (verbose) puts("STUB: ICMCreatePixelConversionHelper called");
    return NULL;
}

void* ICMCreateUsablePixelBufferAttributes(void)
{
    if (verbose) puts("STUB: ICMCreateUsablePixelBufferAttributes called");
    return NULL;
}

void* ICMDecompressComplete(void)
{
    if (verbose) puts("STUB: ICMDecompressComplete called");
    return NULL;
}

void* ICMDecompressCompleteS(void)
{
    if (verbose) puts("STUB: ICMDecompressCompleteS called");
    return NULL;
}

void* ICMDecompressCompleteWithBuffer(void)
{
    if (verbose) puts("STUB: ICMDecompressCompleteWithBuffer called");
    return NULL;
}

void* ICMDecompressionFrameOptionsCreate(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsCreate called");
    return NULL;
}

void* ICMDecompressionFrameOptionsCreateCopy(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsCreateCopy called");
    return NULL;
}

void* ICMDecompressionFrameOptionsCreateForSession(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsCreateForSession called");
    return NULL;
}

void* ICMDecompressionFrameOptionsGetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsGetProperty called");
    return NULL;
}

void* ICMDecompressionFrameOptionsGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsGetPropertyInfo called");
    return NULL;
}

void* ICMDecompressionFrameOptionsGetTypeID(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsGetTypeID called");
    return NULL;
}

void* ICMDecompressionFrameOptionsRelease(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsRelease called");
    return NULL;
}

void* ICMDecompressionFrameOptionsRetain(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsRetain called");
    return NULL;
}

void* ICMDecompressionFrameOptionsSetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionFrameOptionsSetProperty called");
    return NULL;
}

void* ICMDecompressionSessionCreate(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionCreate called");
    return NULL;
}

void* ICMDecompressionSessionCreateForVisualContext(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionCreateForVisualContext called");
    return NULL;
}

void* ICMDecompressionSessionCreateForVisualContextWithoutConnecting(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionCreateForVisualContextWithoutConnecting called");
    return NULL;
}

void* ICMDecompressionSessionDecodeFrame(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionDecodeFrame called");
    return NULL;
}

void* ICMDecompressionSessionFlush(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionFlush called");
    return NULL;
}

void* ICMDecompressionSessionGetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionGetProperty called");
    return NULL;
}

void* ICMDecompressionSessionGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionGetPropertyInfo called");
    return NULL;
}

void* ICMDecompressionSessionGetTypeID(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionGetTypeID called");
    return NULL;
}

void* ICMDecompressionSessionOptionsCreate(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsCreate called");
    return NULL;
}

void* ICMDecompressionSessionOptionsCreateCopy(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsCreateCopy called");
    return NULL;
}

void* ICMDecompressionSessionOptionsGetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsGetProperty called");
    return NULL;
}

void* ICMDecompressionSessionOptionsGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsGetPropertyInfo called");
    return NULL;
}

void* ICMDecompressionSessionOptionsGetTypeID(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsGetTypeID called");
    return NULL;
}

void* ICMDecompressionSessionOptionsRelease(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsRelease called");
    return NULL;
}

void* ICMDecompressionSessionOptionsRetain(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsRetain called");
    return NULL;
}

void* ICMDecompressionSessionOptionsSetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionOptionsSetProperty called");
    return NULL;
}

void* ICMDecompressionSessionQueueEmptyFrame(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionQueueEmptyFrame called");
    return NULL;
}

void* ICMDecompressionSessionRelease(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionRelease called");
    return NULL;
}

void* ICMDecompressionSessionRetain(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionRetain called");
    return NULL;
}

void* ICMDecompressionSessionSendPixelBuffer(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSendPixelBuffer called");
    return NULL;
}

void* ICMDecompressionSessionSetNonScheduledDisplayDirection(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSetNonScheduledDisplayDirection called");
    return NULL;
}

void* ICMDecompressionSessionSetNonScheduledDisplayTime(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSetNonScheduledDisplayTime called");
    return NULL;
}

void* ICMDecompressionSessionSetProperty(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSetProperty called");
    return NULL;
}

void* ICMDecompressionSessionSwitchOffVisualContext(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSwitchOffVisualContext called");
    return NULL;
}

void* ICMDecompressionSessionSwitchOnVisualContext(void)
{
    if (verbose) puts("STUB: ICMDecompressionSessionSwitchOnVisualContext called");
    return NULL;
}

void* ICMEncodedFrameCreateMutable(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameCreateMutable called");
    return NULL;
}

void* ICMEncodedFrameGetBufferSize(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetBufferSize called");
    return NULL;
}

void* ICMEncodedFrameGetDataPtr(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDataPtr called");
    return NULL;
}

void* ICMEncodedFrameGetDataSize(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDataSize called");
    return NULL;
}

void* ICMEncodedFrameGetDecodeDuration(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDecodeDuration called");
    return NULL;
}

void* ICMEncodedFrameGetDecodeNumber(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDecodeNumber called");
    return NULL;
}

void* ICMEncodedFrameGetDecodeTimeStamp(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDecodeTimeStamp called");
    return NULL;
}

void* ICMEncodedFrameGetDisplayDuration(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDisplayDuration called");
    return NULL;
}

void* ICMEncodedFrameGetDisplayNumber(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDisplayNumber called");
    return NULL;
}

void* ICMEncodedFrameGetDisplayOffset(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDisplayOffset called");
    return NULL;
}

void* ICMEncodedFrameGetDisplayTimeStamp(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetDisplayTimeStamp called");
    return NULL;
}

void* ICMEncodedFrameGetFrameType(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetFrameType called");
    return NULL;
}

void* ICMEncodedFrameGetImageDescription(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetImageDescription called");
    return NULL;
}

void* ICMEncodedFrameGetMediaSampleFlags(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetMediaSampleFlags called");
    return NULL;
}

void* ICMEncodedFrameGetSimilarity(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetSimilarity called");
    return NULL;
}

void* ICMEncodedFrameGetSourceFrameRefCon(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetSourceFrameRefCon called");
    return NULL;
}

void* ICMEncodedFrameGetTimeScale(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetTimeScale called");
    return NULL;
}

void* ICMEncodedFrameGetTypeID(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetTypeID called");
    return NULL;
}

void* ICMEncodedFrameGetValidTimeFlags(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameGetValidTimeFlags called");
    return NULL;
}

void* ICMEncodedFrameRelease(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameRelease called");
    return NULL;
}

void* ICMEncodedFrameRetain(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameRetain called");
    return NULL;
}

void* ICMEncodedFrameSetDataSize(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetDataSize called");
    return NULL;
}

void* ICMEncodedFrameSetDecodeDuration(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetDecodeDuration called");
    return NULL;
}

void* ICMEncodedFrameSetDecodeTimeStamp(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetDecodeTimeStamp called");
    return NULL;
}

void* ICMEncodedFrameSetDisplayDuration(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetDisplayDuration called");
    return NULL;
}

void* ICMEncodedFrameSetDisplayTimeStamp(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetDisplayTimeStamp called");
    return NULL;
}

void* ICMEncodedFrameSetFrameType(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetFrameType called");
    return NULL;
}

void* ICMEncodedFrameSetMediaSampleFlags(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetMediaSampleFlags called");
    return NULL;
}

void* ICMEncodedFrameSetSimilarity(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetSimilarity called");
    return NULL;
}

void* ICMEncodedFrameSetValidTimeFlags(void)
{
    if (verbose) puts("STUB: ICMEncodedFrameSetValidTimeFlags called");
    return NULL;
}

void* ICMGetPixelFormatInfo(void)
{
    if (verbose) puts("STUB: ICMGetPixelFormatInfo called");
    return NULL;
}

void* ICMImageDescriptionGetProperty(void)
{
    if (verbose) puts("STUB: ICMImageDescriptionGetProperty called");
    return NULL;
}

void* ICMImageDescriptionGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ICMImageDescriptionGetPropertyInfo called");
    return NULL;
}

void* ICMImageDescriptionSetProperty(void)
{
    if (verbose) puts("STUB: ICMImageDescriptionSetProperty called");
    return NULL;
}

void* ICMInstallVideoDecodeTask(void)
{
    if (verbose) puts("STUB: ICMInstallVideoDecodeTask called");
    return NULL;
}

void* ICMInstallVideoOutput(void)
{
    if (verbose) puts("STUB: ICMInstallVideoOutput called");
    return NULL;
}

void* ICMListWantedPixelFormats(void)
{
    if (verbose) puts("STUB: ICMListWantedPixelFormats called");
    return NULL;
}

void* ICMMultiPassStorageCopyDataAtTimeStamp(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageCopyDataAtTimeStamp called");
    return NULL;
}

void* ICMMultiPassStorageCreateWithCallbacks(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageCreateWithCallbacks called");
    return NULL;
}

void* ICMMultiPassStorageCreateWithTemporaryFile(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageCreateWithTemporaryFile called");
    return NULL;
}

void* ICMMultiPassStorageGetTimeStamp(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageGetTimeStamp called");
    return NULL;
}

void* ICMMultiPassStorageGetTypeID(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageGetTypeID called");
    return NULL;
}

void* ICMMultiPassStorageRelease(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageRelease called");
    return NULL;
}

void* ICMMultiPassStorageRetain(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageRetain called");
    return NULL;
}

void* ICMMultiPassStorageSetDataAtTimeStamp(void)
{
    if (verbose) puts("STUB: ICMMultiPassStorageSetDataAtTimeStamp called");
    return NULL;
}

void* ICMReleasePixelConversionHelper(void)
{
    if (verbose) puts("STUB: ICMReleasePixelConversionHelper called");
    return NULL;
}

void* ICMRemoveVideoOutput(void)
{
    if (verbose) puts("STUB: ICMRemoveVideoOutput called");
    return NULL;
}

void* ICMSequenceDataPtrHACK(void)
{
    if (verbose) puts("STUB: ICMSequenceDataPtrHACK called");
    return NULL;
}

void* ICMSequenceGetChainMember(void)
{
    if (verbose) puts("STUB: ICMSequenceGetChainMember called");
    return NULL;
}

void* ICMSequenceGetInfo(void)
{
    if (verbose) puts("STUB: ICMSequenceGetInfo called");
    return NULL;
}

void* ICMSequenceLockBits(void)
{
    if (verbose) puts("STUB: ICMSequenceLockBits called");
    return NULL;
}

void* ICMSequenceSetInfo(void)
{
    if (verbose) puts("STUB: ICMSequenceSetInfo called");
    return NULL;
}

void* ICMSequenceUnlockBits(void)
{
    if (verbose) puts("STUB: ICMSequenceUnlockBits called");
    return NULL;
}

void* ICMSetPixelFormatInfo(void)
{
    if (verbose) puts("STUB: ICMSetPixelFormatInfo called");
    return NULL;
}

void* ICMShieldSequenceCursor(void)
{
    if (verbose) puts("STUB: ICMShieldSequenceCursor called");
    return NULL;
}

void* ICMUninstallVideoDecodeTask(void)
{
    if (verbose) puts("STUB: ICMUninstallVideoDecodeTask called");
    return NULL;
}

void* ICMUpdateImageDescriptionForCVPixelBuffer(void)
{
    if (verbose) puts("STUB: ICMUpdateImageDescriptionForCVPixelBuffer called");
    return NULL;
}

void* INIT_QuickTimeLib(void)
{
    if (verbose) puts("STUB: INIT_QuickTimeLib called");
    return NULL;
}

void* ITextAddString(void)
{
    if (verbose) puts("STUB: ITextAddString called");
    return NULL;
}

void* ITextGetString(void)
{
    if (verbose) puts("STUB: ITextGetString called");
    return NULL;
}

void* ITextRemoveString(void)
{
    if (verbose) puts("STUB: ITextRemoveString called");
    return NULL;
}

void* ImageCodecBandCompress(void)
{
    if (verbose) puts("STUB: ImageCodecBandCompress called");
    return NULL;
}

void* ImageCodecBandDecompress(void)
{
    if (verbose) puts("STUB: ImageCodecBandDecompress called");
    return NULL;
}

void* ImageCodecBeginBand(void)
{
    if (verbose) puts("STUB: ImageCodecBeginBand called");
    return NULL;
}

void* ImageCodecBeginPass(void)
{
    if (verbose) puts("STUB: ImageCodecBeginPass called");
    return NULL;
}

void* ImageCodecBusy(void)
{
    if (verbose) puts("STUB: ImageCodecBusy called");
    return NULL;
}

void* ImageCodecCancelTrigger(void)
{
    if (verbose) puts("STUB: ImageCodecCancelTrigger called");
    return NULL;
}

void* ImageCodecChangeLastFrameDataPtrHACK(void)
{
    if (verbose) puts("STUB: ImageCodecChangeLastFrameDataPtrHACK called");
    return NULL;
}

void* ImageCodecCheckBufferAvailability(void)
{
    if (verbose) puts("STUB: ImageCodecCheckBufferAvailability called");
    return NULL;
}

void* ImageCodecCompleteFrame(void)
{
    if (verbose) puts("STUB: ImageCodecCompleteFrame called");
    return NULL;
}

void* ImageCodecCopyExtendedFrameOptions(void)
{
    if (verbose) puts("STUB: ImageCodecCopyExtendedFrameOptions called");
    return NULL;
}

void* ImageCodecCreateStandardParameterDialog(void)
{
    if (verbose) puts("STUB: ImageCodecCreateStandardParameterDialog called");
    return NULL;
}

void* ImageCodecDITLEvent(void)
{
    if (verbose) puts("STUB: ImageCodecDITLEvent called");
    return NULL;
}

void* ImageCodecDITLInstall(void)
{
    if (verbose) puts("STUB: ImageCodecDITLInstall called");
    return NULL;
}

void* ImageCodecDITLItem(void)
{
    if (verbose) puts("STUB: ImageCodecDITLItem called");
    return NULL;
}

void* ImageCodecDITLRemove(void)
{
    if (verbose) puts("STUB: ImageCodecDITLRemove called");
    return NULL;
}

void* ImageCodecDITLValidateInput(void)
{
    if (verbose) puts("STUB: ImageCodecDITLValidateInput called");
    return NULL;
}

void* ImageCodecDataMonitorBeginBand(void)
{
    if (verbose) puts("STUB: ImageCodecDataMonitorBeginBand called");
    return NULL;
}

void* ImageCodecDataMonitorEndBand(void)
{
    if (verbose) puts("STUB: ImageCodecDataMonitorEndBand called");
    return NULL;
}

void* ImageCodecDataMonitorInitialize(void)
{
    if (verbose) puts("STUB: ImageCodecDataMonitorInitialize called");
    return NULL;
}

void* ImageCodecDecodeBand(void)
{
    if (verbose) puts("STUB: ImageCodecDecodeBand called");
    return NULL;
}

void* ImageCodecDismissStandardParameterDialog(void)
{
    if (verbose) puts("STUB: ImageCodecDismissStandardParameterDialog called");
    return NULL;
}

void* ImageCodecDisplayExtraFrame(void)
{
    if (verbose) puts("STUB: ImageCodecDisplayExtraFrame called");
    return NULL;
}

void* ImageCodecDisposeExtendedFrameOptions(void)
{
    if (verbose) puts("STUB: ImageCodecDisposeExtendedFrameOptions called");
    return NULL;
}

void* ImageCodecDisposeImageGWorld(void)
{
    if (verbose) puts("STUB: ImageCodecDisposeImageGWorld called");
    return NULL;
}

void* ImageCodecDisposeMemory(void)
{
    if (verbose) puts("STUB: ImageCodecDisposeMemory called");
    return NULL;
}

void* ImageCodecDrawBand(void)
{
    if (verbose) puts("STUB: ImageCodecDrawBand called");
    return NULL;
}

void* ImageCodecDroppingFrame(void)
{
    if (verbose) puts("STUB: ImageCodecDroppingFrame called");
    return NULL;
}

void* ImageCodecEffectBegin(void)
{
    if (verbose) puts("STUB: ImageCodecEffectBegin called");
    return NULL;
}

void* ImageCodecEffectCancel(void)
{
    if (verbose) puts("STUB: ImageCodecEffectCancel called");
    return NULL;
}

void* ImageCodecEffectConvertEffectSourceToFormat(void)
{
    if (verbose) puts("STUB: ImageCodecEffectConvertEffectSourceToFormat called");
    return NULL;
}

void* ImageCodecEffectDisposeSMPTEFrame(void)
{
    if (verbose) puts("STUB: ImageCodecEffectDisposeSMPTEFrame called");
    return NULL;
}

void* ImageCodecEffectGetSpeed(void)
{
    if (verbose) puts("STUB: ImageCodecEffectGetSpeed called");
    return NULL;
}

void* ImageCodecEffectPrepareSMPTEFrame(void)
{
    if (verbose) puts("STUB: ImageCodecEffectPrepareSMPTEFrame called");
    return NULL;
}

void* ImageCodecEffectRenderFrame(void)
{
    if (verbose) puts("STUB: ImageCodecEffectRenderFrame called");
    return NULL;
}

void* ImageCodecEffectRenderSMPTEFrame(void)
{
    if (verbose) puts("STUB: ImageCodecEffectRenderSMPTEFrame called");
    return NULL;
}

void* ImageCodecEffectSetup(void)
{
    if (verbose) puts("STUB: ImageCodecEffectSetup called");
    return NULL;
}

void* ImageCodecEncodeFrame(void)
{
    if (verbose) puts("STUB: ImageCodecEncodeFrame called");
    return NULL;
}

void* ImageCodecEndBand(void)
{
    if (verbose) puts("STUB: ImageCodecEndBand called");
    return NULL;
}

void* ImageCodecEndPass(void)
{
    if (verbose) puts("STUB: ImageCodecEndPass called");
    return NULL;
}

void* ImageCodecExtendedFrameOptionsGetProperty(void)
{
    if (verbose) puts("STUB: ImageCodecExtendedFrameOptionsGetProperty called");
    return NULL;
}

void* ImageCodecExtendedFrameOptionsGetPropertyInfo(void)
{
    if (verbose) puts("STUB: ImageCodecExtendedFrameOptionsGetPropertyInfo called");
    return NULL;
}

void* ImageCodecExtendedFrameOptionsSetProperty(void)
{
    if (verbose) puts("STUB: ImageCodecExtendedFrameOptionsSetProperty called");
    return NULL;
}

void* ImageCodecExtractAndCombineFields(void)
{
    if (verbose) puts("STUB: ImageCodecExtractAndCombineFields called");
    return NULL;
}

void* ImageCodecFindAvailableBuffer(void)
{
    if (verbose) puts("STUB: ImageCodecFindAvailableBuffer called");
    return NULL;
}

void* ImageCodecFlush(void)
{
    if (verbose) puts("STUB: ImageCodecFlush called");
    return NULL;
}

void* ImageCodecFlushFrame(void)
{
    if (verbose) puts("STUB: ImageCodecFlushFrame called");
    return NULL;
}

void* ImageCodecFlushFrameAtTime(void)
{
    if (verbose) puts("STUB: ImageCodecFlushFrameAtTime called");
    return NULL;
}

void* ImageCodecFrameNotDisplayable(void)
{
    if (verbose) puts("STUB: ImageCodecFrameNotDisplayable called");
    return NULL;
}

void* ImageCodecGetBaseMPWorkFunction(void)
{
    if (verbose) puts("STUB: ImageCodecGetBaseMPWorkFunction called");
    return NULL;
}

void* ImageCodecGetCodecInfo(void)
{
    if (verbose) puts("STUB: ImageCodecGetCodecInfo called");
    return NULL;
}

void* ImageCodecGetCompressedImageSize(void)
{
    if (verbose) puts("STUB: ImageCodecGetCompressedImageSize called");
    return NULL;
}

void* ImageCodecGetCompressionTime(void)
{
    if (verbose) puts("STUB: ImageCodecGetCompressionTime called");
    return NULL;
}

void* ImageCodecGetDITLForSize(void)
{
    if (verbose) puts("STUB: ImageCodecGetDITLForSize called");
    return NULL;
}

void* ImageCodecGetDecompressLatency(void)
{
    if (verbose) puts("STUB: ImageCodecGetDecompressLatency called");
    return NULL;
}

void* ImageCodecGetInterestingTime(void)
{
    if (verbose) puts("STUB: ImageCodecGetInterestingTime called");
    return NULL;
}

void* ImageCodecGetMaxCompressionSize(void)
{
    if (verbose) puts("STUB: ImageCodecGetMaxCompressionSize called");
    return NULL;
}

void* ImageCodecGetMaxCompressionSizeWithSources(void)
{
    if (verbose) puts("STUB: ImageCodecGetMaxCompressionSizeWithSources called");
    return NULL;
}

void* ImageCodecGetNextDesiredSourceFrameDisplayTimes(void)
{
    if (verbose) puts("STUB: ImageCodecGetNextDesiredSourceFrameDisplayTimes called");
    return NULL;
}

void* ImageCodecGetParameterList(void)
{
    if (verbose) puts("STUB: ImageCodecGetParameterList called");
    return NULL;
}

void* ImageCodecGetParameterListHandle(void)
{
    if (verbose) puts("STUB: ImageCodecGetParameterListHandle called");
    return NULL;
}

void* ImageCodecGetPreferredChunkSizeAndAlignment(void)
{
    if (verbose) puts("STUB: ImageCodecGetPreferredChunkSizeAndAlignment called");
    return NULL;
}

void* ImageCodecGetSettings(void)
{
    if (verbose) puts("STUB: ImageCodecGetSettings called");
    return NULL;
}

void* ImageCodecGetSettingsAsText(void)
{
    if (verbose) puts("STUB: ImageCodecGetSettingsAsText called");
    return NULL;
}

void* ImageCodecGetSimilarity(void)
{
    if (verbose) puts("STUB: ImageCodecGetSimilarity called");
    return NULL;
}

void* ImageCodecGetSourceDataGammaLevel(void)
{
    if (verbose) puts("STUB: ImageCodecGetSourceDataGammaLevel called");
    return NULL;
}

void* ImageCodecHitTestData(void)
{
    if (verbose) puts("STUB: ImageCodecHitTestData called");
    return NULL;
}

void* ImageCodecHitTestDataWithFlags(void)
{
    if (verbose) puts("STUB: ImageCodecHitTestDataWithFlags called");
    return NULL;
}

void* ImageCodecInitialize(void)
{
    if (verbose) puts("STUB: ImageCodecInitialize called");
    return NULL;
}

void* ImageCodecIsImageDescriptionEquivalent(void)
{
    if (verbose) puts("STUB: ImageCodecIsImageDescriptionEquivalent called");
    return NULL;
}

void* ImageCodecIsStandardParameterDialogEvent(void)
{
    if (verbose) puts("STUB: ImageCodecIsStandardParameterDialogEvent called");
    return NULL;
}

void* ImageCodecLockBits(void)
{
    if (verbose) puts("STUB: ImageCodecLockBits called");
    return NULL;
}

void* ImageCodecMergeFloatingImageOntoWindow(void)
{
    if (verbose) puts("STUB: ImageCodecMergeFloatingImageOntoWindow called");
    return NULL;
}

void* ImageCodecNewExtendedFrameOptions(void)
{
    if (verbose) puts("STUB: ImageCodecNewExtendedFrameOptions called");
    return NULL;
}

void* ImageCodecNewImageBufferMemory(void)
{
    if (verbose) puts("STUB: ImageCodecNewImageBufferMemory called");
    return NULL;
}

void* ImageCodecNewImageGWorld(void)
{
    if (verbose) puts("STUB: ImageCodecNewImageGWorld called");
    return NULL;
}

void* ImageCodecNewMemory(void)
{
    if (verbose) puts("STUB: ImageCodecNewMemory called");
    return NULL;
}

void* ImageCodecPreCompress(void)
{
    if (verbose) puts("STUB: ImageCodecPreCompress called");
    return NULL;
}

void* ImageCodecPreDecompress(void)
{
    if (verbose) puts("STUB: ImageCodecPreDecompress called");
    return NULL;
}

void* ImageCodecPreflight(void)
{
    if (verbose) puts("STUB: ImageCodecPreflight called");
    return NULL;
}

void* ImageCodecPrepareToCompressFrames(void)
{
    if (verbose) puts("STUB: ImageCodecPrepareToCompressFrames called");
    return NULL;
}

void* ImageCodecProcessBetweenPasses(void)
{
    if (verbose) puts("STUB: ImageCodecProcessBetweenPasses called");
    return NULL;
}

void* ImageCodecQueueStarting(void)
{
    if (verbose) puts("STUB: ImageCodecQueueStarting called");
    return NULL;
}

void* ImageCodecQueueStopping(void)
{
    if (verbose) puts("STUB: ImageCodecQueueStopping called");
    return NULL;
}

void* ImageCodecRemoveFloatingImage(void)
{
    if (verbose) puts("STUB: ImageCodecRemoveFloatingImage called");
    return NULL;
}

void* ImageCodecRequestGammaLevel(void)
{
    if (verbose) puts("STUB: ImageCodecRequestGammaLevel called");
    return NULL;
}

void* ImageCodecRequestSettings(void)
{
    if (verbose) puts("STUB: ImageCodecRequestSettings called");
    return NULL;
}

void* ImageCodecScheduleFrame(void)
{
    if (verbose) puts("STUB: ImageCodecScheduleFrame called");
    return NULL;
}

void* ImageCodecSetDataMonitor(void)
{
    if (verbose) puts("STUB: ImageCodecSetDataMonitor called");
    return NULL;
}

void* ImageCodecSetSettings(void)
{
    if (verbose) puts("STUB: ImageCodecSetSettings called");
    return NULL;
}

void* ImageCodecSetTimeBase(void)
{
    if (verbose) puts("STUB: ImageCodecSetTimeBase called");
    return NULL;
}

void* ImageCodecSetTimeCode(void)
{
    if (verbose) puts("STUB: ImageCodecSetTimeCode called");
    return NULL;
}

void* ImageCodecSourceChanged(void)
{
    if (verbose) puts("STUB: ImageCodecSourceChanged called");
    return NULL;
}

void* ImageCodecStandardParameterDialogDoAction(void)
{
    if (verbose) puts("STUB: ImageCodecStandardParameterDialogDoAction called");
    return NULL;
}

void* ImageCodecTriggerExtraDecompress(void)
{
    if (verbose) puts("STUB: ImageCodecTriggerExtraDecompress called");
    return NULL;
}

void* ImageCodecTrimImage(void)
{
    if (verbose) puts("STUB: ImageCodecTrimImage called");
    return NULL;
}

void* ImageCodecUnlockBits(void)
{
    if (verbose) puts("STUB: ImageCodecUnlockBits called");
    return NULL;
}

void* ImageCodecValidateParameters(void)
{
    if (verbose) puts("STUB: ImageCodecValidateParameters called");
    return NULL;
}

void* ImageFieldSequenceBegin(void)
{
    if (verbose) puts("STUB: ImageFieldSequenceBegin called");
    return NULL;
}

void* ImageFieldSequenceEnd(void)
{
    if (verbose) puts("STUB: ImageFieldSequenceEnd called");
    return NULL;
}

void* ImageFieldSequenceExtractCombine(void)
{
    if (verbose) puts("STUB: ImageFieldSequenceExtractCombine called");
    return NULL;
}

void* ImageQueueVUEnqueue(void)
{
    if (verbose) puts("STUB: ImageQueueVUEnqueue called");
    return NULL;
}

void* ImageQueueVUFlush(void)
{
    if (verbose) puts("STUB: ImageQueueVUFlush called");
    return NULL;
}

void* ImageTranscodeDisposeFrameData(void)
{
    if (verbose) puts("STUB: ImageTranscodeDisposeFrameData called");
    return NULL;
}

void* ImageTranscodeFrame(void)
{
    if (verbose) puts("STUB: ImageTranscodeFrame called");
    return NULL;
}

void* ImageTranscodeSequenceBegin(void)
{
    if (verbose) puts("STUB: ImageTranscodeSequenceBegin called");
    return NULL;
}

void* ImageTranscodeSequenceEnd(void)
{
    if (verbose) puts("STUB: ImageTranscodeSequenceEnd called");
    return NULL;
}

void* ImageTranscoderBeginSequence(void)
{
    if (verbose) puts("STUB: ImageTranscoderBeginSequence called");
    return NULL;
}

void* ImageTranscoderConvert(void)
{
    if (verbose) puts("STUB: ImageTranscoderConvert called");
    return NULL;
}

void* ImageTranscoderDisposeData(void)
{
    if (verbose) puts("STUB: ImageTranscoderDisposeData called");
    return NULL;
}

void* ImageTranscoderEndSequence(void)
{
    if (verbose) puts("STUB: ImageTranscoderEndSequence called");
    return NULL;
}

void* InetDataHGetHTTPResponseHeaders(void)
{
    if (verbose) puts("STUB: InetDataHGetHTTPResponseHeaders called");
    return NULL;
}

void* InetDataHSetHTTPRequestHeaders(void)
{
    if (verbose) puts("STUB: InetDataHSetHTTPRequestHeaders called");
    return NULL;
}

void* InitializeQTS(void)
{
    if (verbose) puts("STUB: InitializeQTS called");
    return NULL;
}

void* InsertEmptyMovieSegment(void)
{
    if (verbose) puts("STUB: InsertEmptyMovieSegment called");
    return NULL;
}

void* InsertEmptyTrackSegment(void)
{
    if (verbose) puts("STUB: InsertEmptyTrackSegment called");
    return NULL;
}

void* InsertMediaIntoTrack(void)
{
    if (verbose) puts("STUB: InsertMediaIntoTrack called");
    return NULL;
}

void* InsertMovieSegment(void)
{
    if (verbose) puts("STUB: InsertMovieSegment called");
    return NULL;
}

void* InsertTrackSegment(void)
{
    if (verbose) puts("STUB: InsertTrackSegment called");
    return NULL;
}

void* InstallActionGetInfo(void)
{
    if (verbose) puts("STUB: InstallActionGetInfo called");
    return NULL;
}

void* InstallActionInit(void)
{
    if (verbose) puts("STUB: InstallActionInit called");
    return NULL;
}

void* InstallActionInstall(void)
{
    if (verbose) puts("STUB: InstallActionInstall called");
    return NULL;
}

void* InstallActionPostInstall(void)
{
    if (verbose) puts("STUB: InstallActionPostInstall called");
    return NULL;
}

void* InstallActionPreInstall(void)
{
    if (verbose) puts("STUB: InstallActionPreInstall called");
    return NULL;
}

void* InstallActionSetInfo(void)
{
    if (verbose) puts("STUB: InstallActionSetInfo called");
    return NULL;
}

void* InstallActionUninstall(void)
{
    if (verbose) puts("STUB: InstallActionUninstall called");
    return NULL;
}

void* InstrumentCloseComponentResFile(void)
{
    if (verbose) puts("STUB: InstrumentCloseComponentResFile called");
    return NULL;
}

void* InstrumentGetComponentRefCon(void)
{
    if (verbose) puts("STUB: InstrumentGetComponentRefCon called");
    return NULL;
}

void* InstrumentGetInfo(void)
{
    if (verbose) puts("STUB: InstrumentGetInfo called");
    return NULL;
}

void* InstrumentGetInst(void)
{
    if (verbose) puts("STUB: InstrumentGetInst called");
    return NULL;
}

void* InstrumentGetSynthesizerType(void)
{
    if (verbose) puts("STUB: InstrumentGetSynthesizerType called");
    return NULL;
}

void* InstrumentInitialize(void)
{
    if (verbose) puts("STUB: InstrumentInitialize called");
    return NULL;
}

void* InstrumentOpenComponentResFile(void)
{
    if (verbose) puts("STUB: InstrumentOpenComponentResFile called");
    return NULL;
}

void* InstrumentSetComponentRefCon(void)
{
    if (verbose) puts("STUB: InstrumentSetComponentRefCon called");
    return NULL;
}

void* InvalidateMovieRegion(void)
{
    if (verbose) puts("STUB: InvalidateMovieRegion called");
    return NULL;
}

void* InvalidateSprite(void)
{
    if (verbose) puts("STUB: InvalidateSprite called");
    return NULL;
}

void* InvalidateSpriteWorld(void)
{
    if (verbose) puts("STUB: InvalidateSpriteWorld called");
    return NULL;
}

void* InverseMatrix(void)
{
    if (verbose) puts("STUB: InverseMatrix called");
    return NULL;
}

void* InvokeActionsUPP(void)
{
    if (verbose) puts("STUB: InvokeActionsUPP called");
    return NULL;
}

void* InvokeBlitterUPP(void)
{
    if (verbose) puts("STUB: InvokeBlitterUPP called");
    return NULL;
}

void* InvokeCDataHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeCDataHandlerUPP called");
    return NULL;
}

void* InvokeCGNotificationUPP(void)
{
    if (verbose) puts("STUB: InvokeCGNotificationUPP called");
    return NULL;
}

void* InvokeCancelledDataUPP(void)
{
    if (verbose) puts("STUB: InvokeCancelledDataUPP called");
    return NULL;
}

void* InvokeCharDataHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeCharDataHandlerUPP called");
    return NULL;
}

void* InvokeCommentHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeCommentHandlerUPP called");
    return NULL;
}

void* InvokeDataHCompletionUPP(void)
{
    if (verbose) puts("STUB: InvokeDataHCompletionUPP called");
    return NULL;
}

void* InvokeDoMCActionUPP(void)
{
    if (verbose) puts("STUB: InvokeDoMCActionUPP called");
    return NULL;
}

void* InvokeEndDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeEndDocumentHandlerUPP called");
    return NULL;
}

void* InvokeEndElementHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeEndElementHandlerUPP called");
    return NULL;
}

void* InvokeEraseUPP(void)
{
    if (verbose) puts("STUB: InvokeEraseUPP called");
    return NULL;
}

void* InvokeExitToShellUPP(void)
{
    if (verbose) puts("STUB: InvokeExitToShellUPP called");
    return NULL;
}

void* InvokeForEachMediaUPP(void)
{
    if (verbose) puts("STUB: InvokeForEachMediaUPP called");
    return NULL;
}

void* InvokeForEachTrackUPP(void)
{
    if (verbose) puts("STUB: InvokeForEachTrackUPP called");
    return NULL;
}

void* InvokeGetAtomLeafUPP(void)
{
    if (verbose) puts("STUB: InvokeGetAtomLeafUPP called");
    return NULL;
}

void* InvokeGetMoreMovieDataUPP(void)
{
    if (verbose) puts("STUB: InvokeGetMoreMovieDataUPP called");
    return NULL;
}

void* InvokeGetMovieUPP(void)
{
    if (verbose) puts("STUB: InvokeGetMovieUPP called");
    return NULL;
}

void* InvokeICMAlignmentUPP(void)
{
    if (verbose) puts("STUB: InvokeICMAlignmentUPP called");
    return NULL;
}

void* InvokeICMCompletionUPP(void)
{
    if (verbose) puts("STUB: InvokeICMCompletionUPP called");
    return NULL;
}

void* InvokeICMConvertDataFormatUPP(void)
{
    if (verbose) puts("STUB: InvokeICMConvertDataFormatUPP called");
    return NULL;
}

void* InvokeICMCursorShieldedUPP(void)
{
    if (verbose) puts("STUB: InvokeICMCursorShieldedUPP called");
    return NULL;
}

void* InvokeICMDataUPP(void)
{
    if (verbose) puts("STUB: InvokeICMDataUPP called");
    return NULL;
}

void* InvokeICMFlushUPP(void)
{
    if (verbose) puts("STUB: InvokeICMFlushUPP called");
    return NULL;
}

void* InvokeICMMemoryDisposedUPP(void)
{
    if (verbose) puts("STUB: InvokeICMMemoryDisposedUPP called");
    return NULL;
}

void* InvokeICMProgressUPP(void)
{
    if (verbose) puts("STUB: InvokeICMProgressUPP called");
    return NULL;
}

void* InvokeImageCodecDrawBandCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeImageCodecDrawBandCompleteUPP called");
    return NULL;
}

void* InvokeImageCodecMPDrawBandUPP(void)
{
    if (verbose) puts("STUB: InvokeImageCodecMPDrawBandUPP called");
    return NULL;
}

void* InvokeImageCodecTimeTriggerUPP(void)
{
    if (verbose) puts("STUB: InvokeImageCodecTimeTriggerUPP called");
    return NULL;
}

void* InvokeMCActionFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeMCActionFilterUPP called");
    return NULL;
}

void* InvokeMCActionFilterWithRefConUPP(void)
{
    if (verbose) puts("STUB: InvokeMCActionFilterWithRefConUPP called");
    return NULL;
}

void* InvokeMCActionNotificationUPP(void)
{
    if (verbose) puts("STUB: InvokeMCActionNotificationUPP called");
    return NULL;
}

void* InvokeMovieDrawingCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieDrawingCompleteUPP called");
    return NULL;
}

void* InvokeMovieExecuteWiredActionsUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieExecuteWiredActionsUPP called");
    return NULL;
}

void* InvokeMovieExportGetDataUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieExportGetDataUPP called");
    return NULL;
}

void* InvokeMovieExportGetPropertyUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieExportGetPropertyUPP called");
    return NULL;
}

void* InvokeMovieExportStageReachedCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieExportStageReachedCallbackUPP called");
    return NULL;
}

void* InvokeMoviePrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeMoviePrePrerollCompleteUPP called");
    return NULL;
}

void* InvokeMoviePreviewCallOutUPP(void)
{
    if (verbose) puts("STUB: InvokeMoviePreviewCallOutUPP called");
    return NULL;
}

void* InvokeMovieProgressUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieProgressUPP called");
    return NULL;
}

void* InvokeMovieRgnCoverUPP(void)
{
    if (verbose) puts("STUB: InvokeMovieRgnCoverUPP called");
    return NULL;
}

void* InvokeMoviesErrorUPP(void)
{
    if (verbose) puts("STUB: InvokeMoviesErrorUPP called");
    return NULL;
}

void* InvokeMusicMIDIReadHookUPP(void)
{
    if (verbose) puts("STUB: InvokeMusicMIDIReadHookUPP called");
    return NULL;
}

void* InvokeMusicMIDISendUPP(void)
{
    if (verbose) puts("STUB: InvokeMusicMIDISendUPP called");
    return NULL;
}

void* InvokeMusicOfflineDataUPP(void)
{
    if (verbose) puts("STUB: InvokeMusicOfflineDataUPP called");
    return NULL;
}

void* InvokePBOpenUPP(void)
{
    if (verbose) puts("STUB: InvokePBOpenUPP called");
    return NULL;
}

void* InvokePrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokePrePrerollCompleteUPP called");
    return NULL;
}

void* InvokePreprocessInstructionHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokePreprocessInstructionHandlerUPP called");
    return NULL;
}

void* InvokePutNeedRoomUPP(void)
{
    if (verbose) puts("STUB: InvokePutNeedRoomUPP called");
    return NULL;
}

void* InvokePutPatchLongUPP(void)
{
    if (verbose) puts("STUB: InvokePutPatchLongUPP called");
    return NULL;
}

void* InvokeQDPixUPP(void)
{
    if (verbose) puts("STUB: InvokeQDPixUPP called");
    return NULL;
}

void* InvokeQTAllDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeQTAllDataDoneCompleteUPP called");
    return NULL;
}

void* InvokeQTAllocatorAllocateUPP(void)
{
    if (verbose) puts("STUB: InvokeQTAllocatorAllocateUPP called");
    return NULL;
}

void* InvokeQTAllocatorDisposeAllocatorUPP(void)
{
    if (verbose) puts("STUB: InvokeQTAllocatorDisposeAllocatorUPP called");
    return NULL;
}

void* InvokeQTAllocatorFreeUPP(void)
{
    if (verbose) puts("STUB: InvokeQTAllocatorFreeUPP called");
    return NULL;
}

void* InvokeQTAllocatorReallocateUPP(void)
{
    if (verbose) puts("STUB: InvokeQTAllocatorReallocateUPP called");
    return NULL;
}

void* InvokeQTBandwidthNotificationUPP(void)
{
    if (verbose) puts("STUB: InvokeQTBandwidthNotificationUPP called");
    return NULL;
}

void* InvokeQTCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTCallBackUPP called");
    return NULL;
}

void* InvokeQTComponentPropertyListenerFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeQTComponentPropertyListenerFilterUPP called");
    return NULL;
}

void* InvokeQTComponentPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: InvokeQTComponentPropertyListenerUPP called");
    return NULL;
}

void* InvokeQTDRMPrivAPICallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTDRMPrivAPICallBackUPP called");
    return NULL;
}

void* InvokeQTDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeQTDataDoneCompleteUPP called");
    return NULL;
}

void* InvokeQTDataStashValueDisposeUPP(void)
{
    if (verbose) puts("STUB: InvokeQTDataStashValueDisposeUPP called");
    return NULL;
}

void* InvokeQTEffectListFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeQTEffectListFilterUPP called");
    return NULL;
}

void* InvokeQTFlattenCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTFlattenCallBackUPP called");
    return NULL;
}

void* InvokeQTGenerateHashValueUPP(void)
{
    if (verbose) puts("STUB: InvokeQTGenerateHashValueUPP called");
    return NULL;
}

void* InvokeQTHashKeysAreEqualUPP(void)
{
    if (verbose) puts("STUB: InvokeQTHashKeysAreEqualUPP called");
    return NULL;
}

void* InvokeQTMemPoolCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTMemPoolCallBackUPP called");
    return NULL;
}

void* InvokeQTMessageDispatchUPP(void)
{
    if (verbose) puts("STUB: InvokeQTMessageDispatchUPP called");
    return NULL;
}

void* InvokeQTMissingSoftwareCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTMissingSoftwareCallbackUPP called");
    return NULL;
}

void* InvokeQTMoviePropertyListenerUPP(void)
{
    if (verbose) puts("STUB: InvokeQTMoviePropertyListenerUPP called");
    return NULL;
}

void* InvokeQTMovieStatusPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: InvokeQTMovieStatusPropertyListenerUPP called");
    return NULL;
}

void* InvokeQTNextTaskNeededSoonerCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTNextTaskNeededSoonerCallbackUPP called");
    return NULL;
}

void* InvokeQTProcessPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: InvokeQTProcessPropertyListenerUPP called");
    return NULL;
}

void* InvokeQTReleaseKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: InvokeQTReleaseKeyAndValueUPP called");
    return NULL;
}

void* InvokeQTRetainKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: InvokeQTRetainKeyAndValueUPP called");
    return NULL;
}

void* InvokeQTSDataDispatchFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSDataDispatchFilterUPP called");
    return NULL;
}

void* InvokeQTSDialogSettingsUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSDialogSettingsUPP called");
    return NULL;
}

void* InvokeQTSModalFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSModalFilterUPP called");
    return NULL;
}

void* InvokeQTSNotificationUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSNotificationUPP called");
    return NULL;
}

void* InvokeQTSPanelFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSPanelFilterUPP called");
    return NULL;
}

void* InvokeQTSSourcerDataReleaseUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSSourcerDataReleaseUPP called");
    return NULL;
}

void* InvokeQTSStreamModuleUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSStreamModuleUPP called");
    return NULL;
}

void* InvokeQTSTrackHinterProgressUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSTrackHinterProgressUPP called");
    return NULL;
}

void* InvokeQTSyncTaskUPP(void)
{
    if (verbose) puts("STUB: InvokeQTSyncTaskUPP called");
    return NULL;
}

void* InvokeQTTrackPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: InvokeQTTrackPropertyListenerUPP called");
    return NULL;
}

void* InvokeQTUpdateMovieProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTUpdateMovieProfileCallbackUPP called");
    return NULL;
}

void* InvokeQTUpdateTrackProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeQTUpdateTrackProfileCallbackUPP called");
    return NULL;
}

void* InvokeQTVRBackBufferImagingUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVRBackBufferImagingUPP called");
    return NULL;
}

void* InvokeQTVREnteringNodeUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVREnteringNodeUPP called");
    return NULL;
}

void* InvokeQTVRImagingCompleteUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVRImagingCompleteUPP called");
    return NULL;
}

void* InvokeQTVRInterceptUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVRInterceptUPP called");
    return NULL;
}

void* InvokeQTVRLeavingNodeUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVRLeavingNodeUPP called");
    return NULL;
}

void* InvokeQTVRMouseOverHotSpotUPP(void)
{
    if (verbose) puts("STUB: InvokeQTVRMouseOverHotSpotUPP called");
    return NULL;
}

void* InvokeQTWorkContextDataReleaseUPP(void)
{
    if (verbose) puts("STUB: InvokeQTWorkContextDataReleaseUPP called");
    return NULL;
}

void* InvokeRTPMPDataReleaseUPP(void)
{
    if (verbose) puts("STUB: InvokeRTPMPDataReleaseUPP called");
    return NULL;
}

void* InvokeRTPPBCallbackUPP(void)
{
    if (verbose) puts("STUB: InvokeRTPPBCallbackUPP called");
    return NULL;
}

void* InvokeSCModalFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeSCModalFilterUPP called");
    return NULL;
}

void* InvokeSCModalHookUPP(void)
{
    if (verbose) puts("STUB: InvokeSCModalHookUPP called");
    return NULL;
}

void* InvokeSFModalFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeSFModalFilterUPP called");
    return NULL;
}

void* InvokeSGAddFrameBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGAddFrameBottleUPP called");
    return NULL;
}

void* InvokeSGCompressBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGCompressBottleUPP called");
    return NULL;
}

void* InvokeSGCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGCompressCompleteBottleUPP called");
    return NULL;
}

void* InvokeSGDataUPP(void)
{
    if (verbose) puts("STUB: InvokeSGDataUPP called");
    return NULL;
}

void* InvokeSGDisplayBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGDisplayBottleUPP called");
    return NULL;
}

void* InvokeSGDisplayCompressBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGDisplayCompressBottleUPP called");
    return NULL;
}

void* InvokeSGGrabBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGGrabBottleUPP called");
    return NULL;
}

void* InvokeSGGrabCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGGrabCompleteBottleUPP called");
    return NULL;
}

void* InvokeSGGrabCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGGrabCompressCompleteBottleUPP called");
    return NULL;
}

void* InvokeSGModalFilterUPP(void)
{
    if (verbose) puts("STUB: InvokeSGModalFilterUPP called");
    return NULL;
}

void* InvokeSGTransferFrameBottleUPP(void)
{
    if (verbose) puts("STUB: InvokeSGTransferFrameBottleUPP called");
    return NULL;
}

void* InvokeSHChunkDataReleaseUPP(void)
{
    if (verbose) puts("STUB: InvokeSHChunkDataReleaseUPP called");
    return NULL;
}

void* InvokeSHNotificationUPP(void)
{
    if (verbose) puts("STUB: InvokeSHNotificationUPP called");
    return NULL;
}

void* InvokeSHSendDataUPP(void)
{
    if (verbose) puts("STUB: InvokeSHSendDataUPP called");
    return NULL;
}

void* InvokeSequenceFilterDataUPP(void)
{
    if (verbose) puts("STUB: InvokeSequenceFilterDataUPP called");
    return NULL;
}

void* InvokeStartDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeStartDocumentHandlerUPP called");
    return NULL;
}

void* InvokeStartElementHandlerUPP(void)
{
    if (verbose) puts("STUB: InvokeStartElementHandlerUPP called");
    return NULL;
}

void* InvokeStdPixUPP(void)
{
    if (verbose) puts("STUB: InvokeStdPixUPP called");
    return NULL;
}

void* InvokeTextMediaUPP(void)
{
    if (verbose) puts("STUB: InvokeTextMediaUPP called");
    return NULL;
}

void* InvokeTrackTransferUPP(void)
{
    if (verbose) puts("STUB: InvokeTrackTransferUPP called");
    return NULL;
}

void* InvokeTuneCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeTuneCallBackUPP called");
    return NULL;
}

void* InvokeTunePlayCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeTunePlayCallBackUPP called");
    return NULL;
}

void* InvokeTweenerDataUPP(void)
{
    if (verbose) puts("STUB: InvokeTweenerDataUPP called");
    return NULL;
}

void* InvokeUpdaterAppHandleMenuEventUPP(void)
{
    if (verbose) puts("STUB: InvokeUpdaterAppHandleMenuEventUPP called");
    return NULL;
}

void* InvokeUseThisTrackForTimeTableUPP(void)
{
    if (verbose) puts("STUB: InvokeUseThisTrackForTimeTableUPP called");
    return NULL;
}

void* InvokeVdigIntUPP(void)
{
    if (verbose) puts("STUB: InvokeVdigIntUPP called");
    return NULL;
}

void* InvokeYUVPtrsUPP(void)
{
    if (verbose) puts("STUB: InvokeYUVPtrsUPP called");
    return NULL;
}

void* IsMovieDone(void)
{
    if (verbose) puts("STUB: IsMovieDone called");
    return NULL;
}

void* IsQuickTimeRegistered(void)
{
    if (verbose) puts("STUB: IsQuickTimeRegistered called");
    return NULL;
}

void* IsQuickTimeVersionRegistered(void)
{
    if (verbose) puts("STUB: IsQuickTimeVersionRegistered called");
    return NULL;
}

void* IsScrapMovie(void)
{
    if (verbose) puts("STUB: IsScrapMovie called");
    return NULL;
}

void* LoadMediaIntoRam(void)
{
    if (verbose) puts("STUB: LoadMediaIntoRam called");
    return NULL;
}

void* LoadMovieIntoRam(void)
{
    if (verbose) puts("STUB: LoadMovieIntoRam called");
    return NULL;
}

void* LoadTrackIntoRam(void)
{
    if (verbose) puts("STUB: LoadTrackIntoRam called");
    return NULL;
}

void* LockRemapTable(void)
{
    if (verbose) puts("STUB: LockRemapTable called");
    return NULL;
}

void* MCActivate(void)
{
    if (verbose) puts("STUB: MCActivate called");
    return NULL;
}

void* MCAddMovieSegment(void)
{
    if (verbose) puts("STUB: MCAddMovieSegment called");
    return NULL;
}

void* MCAdjustCursor(void)
{
    if (verbose) puts("STUB: MCAdjustCursor called");
    return NULL;
}

void* MCClear(void)
{
    if (verbose) puts("STUB: MCClear called");
    return NULL;
}

void* MCClick(void)
{
    if (verbose) puts("STUB: MCClick called");
    return NULL;
}

void* MCCopy(void)
{
    if (verbose) puts("STUB: MCCopy called");
    return NULL;
}

void* MCCountMovies(void)
{
    if (verbose) puts("STUB: MCCountMovies called");
    return NULL;
}

void* MCCut(void)
{
    if (verbose) puts("STUB: MCCut called");
    return NULL;
}

void* MCDoAction(void)
{
    if (verbose) puts("STUB: MCDoAction called");
    return NULL;
}

void* MCDraw(void)
{
    if (verbose) puts("STUB: MCDraw called");
    return NULL;
}

void* MCDrawBadge(void)
{
    if (verbose) puts("STUB: MCDrawBadge called");
    return NULL;
}

void* MCDrawControllerWithCGContext(void)
{
    if (verbose) puts("STUB: MCDrawControllerWithCGContext called");
    return NULL;
}

void* MCEnableEditing(void)
{
    if (verbose) puts("STUB: MCEnableEditing called");
    return NULL;
}

void* MCGetClip(void)
{
    if (verbose) puts("STUB: MCGetClip called");
    return NULL;
}

void* MCGetControllerBoundsRect(void)
{
    if (verbose) puts("STUB: MCGetControllerBoundsRect called");
    return NULL;
}

void* MCGetControllerBoundsRgn(void)
{
    if (verbose) puts("STUB: MCGetControllerBoundsRgn called");
    return NULL;
}

void* MCGetControllerInfo(void)
{
    if (verbose) puts("STUB: MCGetControllerInfo called");
    return NULL;
}

void* MCGetControllerPort(void)
{
    if (verbose) puts("STUB: MCGetControllerPort called");
    return NULL;
}

void* MCGetCurrentTime(void)
{
    if (verbose) puts("STUB: MCGetCurrentTime called");
    return NULL;
}

void* MCGetDoActionsProc(void)
{
    if (verbose) puts("STUB: MCGetDoActionsProc called");
    return NULL;
}

void* MCGetIndMovie(void)
{
    if (verbose) puts("STUB: MCGetIndMovie called");
    return NULL;
}

void* MCGetInterfaceElement(void)
{
    if (verbose) puts("STUB: MCGetInterfaceElement called");
    return NULL;
}

void* MCGetMenuString(void)
{
    if (verbose) puts("STUB: MCGetMenuString called");
    return NULL;
}

void* MCGetVisible(void)
{
    if (verbose) puts("STUB: MCGetVisible called");
    return NULL;
}

void* MCGetWindowRgn(void)
{
    if (verbose) puts("STUB: MCGetWindowRgn called");
    return NULL;
}

void* MCHandleEvent(void)
{
    if (verbose) puts("STUB: MCHandleEvent called");
    return NULL;
}

void* MCIdle(void)
{
    if (verbose) puts("STUB: MCIdle called");
    return NULL;
}

void* MCInvalidate(void)
{
    if (verbose) puts("STUB: MCInvalidate called");
    return NULL;
}

void* MCIsControllerAttached(void)
{
    if (verbose) puts("STUB: MCIsControllerAttached called");
    return NULL;
}

void* MCIsEditingEnabled(void)
{
    if (verbose) puts("STUB: MCIsEditingEnabled called");
    return NULL;
}

void* MCIsPlayerEvent(void)
{
    if (verbose) puts("STUB: MCIsPlayerEvent called");
    return NULL;
}

void* MCKey(void)
{
    if (verbose) puts("STUB: MCKey called");
    return NULL;
}

void* MCMovieChanged(void)
{
    if (verbose) puts("STUB: MCMovieChanged called");
    return NULL;
}

void* MCNewAttachedController(void)
{
    if (verbose) puts("STUB: MCNewAttachedController called");
    return NULL;
}

void* MCNewControllerFromFile(void)
{
    if (verbose) puts("STUB: MCNewControllerFromFile called");
    return NULL;
}

void* MCPaste(void)
{
    if (verbose) puts("STUB: MCPaste called");
    return NULL;
}

void* MCPositionController(void)
{
    if (verbose) puts("STUB: MCPositionController called");
    return NULL;
}

void* MCPtInController(void)
{
    if (verbose) puts("STUB: MCPtInController called");
    return NULL;
}

void* MCRemoveAMovie(void)
{
    if (verbose) puts("STUB: MCRemoveAMovie called");
    return NULL;
}

void* MCRemoveAllMovies(void)
{
    if (verbose) puts("STUB: MCRemoveAllMovies called");
    return NULL;
}

void* MCRemoveMovie(void)
{
    if (verbose) puts("STUB: MCRemoveMovie called");
    return NULL;
}

void* MCSetActionFilter(void)
{
    if (verbose) puts("STUB: MCSetActionFilter called");
    return NULL;
}

void* MCSetActionFilterWithRefCon(void)
{
    if (verbose) puts("STUB: MCSetActionFilterWithRefCon called");
    return NULL;
}

void* MCSetBadgePict(void)
{
    if (verbose) puts("STUB: MCSetBadgePict called");
    return NULL;
}

void* MCSetClip(void)
{
    if (verbose) puts("STUB: MCSetClip called");
    return NULL;
}

void* MCSetControllerAttached(void)
{
    if (verbose) puts("STUB: MCSetControllerAttached called");
    return NULL;
}

void* MCSetControllerBoundsRect(void)
{
    if (verbose) puts("STUB: MCSetControllerBoundsRect called");
    return NULL;
}

void* MCSetControllerCapabilities(void)
{
    if (verbose) puts("STUB: MCSetControllerCapabilities called");
    return NULL;
}

void* MCSetControllerNeedsDisplayCallback(void)
{
    if (verbose) puts("STUB: MCSetControllerNeedsDisplayCallback called");
    return NULL;
}

void* MCSetControllerPort(void)
{
    if (verbose) puts("STUB: MCSetControllerPort called");
    return NULL;
}

void* MCSetDuration(void)
{
    if (verbose) puts("STUB: MCSetDuration called");
    return NULL;
}

void* MCSetIdleManager(void)
{
    if (verbose) puts("STUB: MCSetIdleManager called");
    return NULL;
}

void* MCSetMouseStateCallback(void)
{
    if (verbose) puts("STUB: MCSetMouseStateCallback called");
    return NULL;
}

void* MCSetMovie(void)
{
    if (verbose) puts("STUB: MCSetMovie called");
    return NULL;
}

void* MCSetUpEditMenu(void)
{
    if (verbose) puts("STUB: MCSetUpEditMenu called");
    return NULL;
}

void* MCSetViewTranslationCallback(void)
{
    if (verbose) puts("STUB: MCSetViewTranslationCallback called");
    return NULL;
}

void* MCSetVisible(void)
{
    if (verbose) puts("STUB: MCSetVisible called");
    return NULL;
}

void* MCTranslatePointInView(void)
{
    if (verbose) puts("STUB: MCTranslatePointInView called");
    return NULL;
}

void* MCTrimMovieSegment(void)
{
    if (verbose) puts("STUB: MCTrimMovieSegment called");
    return NULL;
}

void* MCUndo(void)
{
    if (verbose) puts("STUB: MCUndo called");
    return NULL;
}

void* MIDIImportGetSettings(void)
{
    if (verbose) puts("STUB: MIDIImportGetSettings called");
    return NULL;
}

void* MIDIImportSetSettings(void)
{
    if (verbose) puts("STUB: MIDIImportSetSettings called");
    return NULL;
}

void* MIMEPrefsUpdaterAddMIMEType(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterAddMIMEType called");
    return NULL;
}

void* MIMEPrefsUpdaterAddProtocol(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterAddProtocol called");
    return NULL;
}

void* MIMEPrefsUpdaterAddScheme(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterAddScheme called");
    return NULL;
}

void* MIMEPrefsUpdaterBeginMIMEUpdate(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterBeginMIMEUpdate called");
    return NULL;
}

void* MIMEPrefsUpdaterEndMIMEUpdate(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterEndMIMEUpdate called");
    return NULL;
}

void* MIMEPrefsUpdaterGetInfo(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterGetInfo called");
    return NULL;
}

void* MIMEPrefsUpdaterSetInfo(void)
{
    if (verbose) puts("STUB: MIMEPrefsUpdaterSetInfo called");
    return NULL;
}

void* MPHelperEncapsulateMPWorkFunctionInMPTask(void)
{
    if (verbose) puts("STUB: MPHelperEncapsulateMPWorkFunctionInMPTask called");
    return NULL;
}

void* MPHelperGetComponentMPWorkFunction(void)
{
    if (verbose) puts("STUB: MPHelperGetComponentMPWorkFunction called");
    return NULL;
}

void* MPHelperSetTaskName(void)
{
    if (verbose) puts("STUB: MPHelperSetTaskName called");
    return NULL;
}

void* MPHelperSetTaskWeight(void)
{
    if (verbose) puts("STUB: MPHelperSetTaskWeight called");
    return NULL;
}

void* MacUnicodeConvertUnicodePtrToTEHandle(void)
{
    if (verbose) puts("STUB: MacUnicodeConvertUnicodePtrToTEHandle called");
    return NULL;
}

void* MakeFilePreview(void)
{
    if (verbose) puts("STUB: MakeFilePreview called");
    return NULL;
}

void* MakeImageDescriptionForEffect(void)
{
    if (verbose) puts("STUB: MakeImageDescriptionForEffect called");
    return NULL;
}

void* MakeImageDescriptionForPixMap(void)
{
    if (verbose) puts("STUB: MakeImageDescriptionForPixMap called");
    return NULL;
}

void* MakeMediaTimeTable(void)
{
    if (verbose) puts("STUB: MakeMediaTimeTable called");
    return NULL;
}

void* MakeRemapTable(void)
{
    if (verbose) puts("STUB: MakeRemapTable called");
    return NULL;
}

void* MakeRoom(void)
{
    if (verbose) puts("STUB: MakeRoom called");
    return NULL;
}

void* MakeThumbnailFromPicture(void)
{
    if (verbose) puts("STUB: MakeThumbnailFromPicture called");
    return NULL;
}

void* MakeThumbnailFromPictureFile(void)
{
    if (verbose) puts("STUB: MakeThumbnailFromPictureFile called");
    return NULL;
}

void* MakeThumbnailFromPixMap(void)
{
    if (verbose) puts("STUB: MakeThumbnailFromPixMap called");
    return NULL;
}

void* MakeTrackTimeTable(void)
{
    if (verbose) puts("STUB: MakeTrackTimeTable called");
    return NULL;
}

void* MapMatrix(void)
{
    if (verbose) puts("STUB: MapMatrix called");
    return NULL;
}

void* MaskBlitterCreateBuffer(void)
{
    if (verbose) puts("STUB: MaskBlitterCreateBuffer called");
    return NULL;
}

void* MaskBlitterDisposeBuffer(void)
{
    if (verbose) puts("STUB: MaskBlitterDisposeBuffer called");
    return NULL;
}

void* MaskBlitterGetPixMap(void)
{
    if (verbose) puts("STUB: MaskBlitterGetPixMap called");
    return NULL;
}

void* MaskBlitterUseBuffer(void)
{
    if (verbose) puts("STUB: MaskBlitterUseBuffer called");
    return NULL;
}

void* MatteBlitterCreateBuffer(void)
{
    if (verbose) puts("STUB: MatteBlitterCreateBuffer called");
    return NULL;
}

void* MatteBlitterDisposeBuffer(void)
{
    if (verbose) puts("STUB: MatteBlitterDisposeBuffer called");
    return NULL;
}

void* MatteBlitterGetPixMap(void)
{
    if (verbose) puts("STUB: MatteBlitterGetPixMap called");
    return NULL;
}

void* MatteBlitterSetAlphaFuzz(void)
{
    if (verbose) puts("STUB: MatteBlitterSetAlphaFuzz called");
    return NULL;
}

void* MatteBlitterSetAlphaOffset(void)
{
    if (verbose) puts("STUB: MatteBlitterSetAlphaOffset called");
    return NULL;
}

void* MatteBlitterUseBuffer(void)
{
    if (verbose) puts("STUB: MatteBlitterUseBuffer called");
    return NULL;
}

void* Media3DGetCameraAngleAspect(void)
{
    if (verbose) puts("STUB: Media3DGetCameraAngleAspect called");
    return NULL;
}

void* Media3DGetCameraData(void)
{
    if (verbose) puts("STUB: Media3DGetCameraData called");
    return NULL;
}

void* Media3DGetCameraRange(void)
{
    if (verbose) puts("STUB: Media3DGetCameraRange called");
    return NULL;
}

void* Media3DGetCurrentGroup(void)
{
    if (verbose) puts("STUB: Media3DGetCurrentGroup called");
    return NULL;
}

void* Media3DGetMasterMatrix(void)
{
    if (verbose) puts("STUB: Media3DGetMasterMatrix called");
    return NULL;
}

void* Media3DGetNamedObjectList(void)
{
    if (verbose) puts("STUB: Media3DGetNamedObjectList called");
    return NULL;
}

void* Media3DGetRendererList(void)
{
    if (verbose) puts("STUB: Media3DGetRendererList called");
    return NULL;
}

void* Media3DGetViewObject(void)
{
    if (verbose) puts("STUB: Media3DGetViewObject called");
    return NULL;
}

void* Media3DRotateNamedObjectTo(void)
{
    if (verbose) puts("STUB: Media3DRotateNamedObjectTo called");
    return NULL;
}

void* Media3DScaleNamedObjectTo(void)
{
    if (verbose) puts("STUB: Media3DScaleNamedObjectTo called");
    return NULL;
}

void* Media3DSetCameraAngleAspect(void)
{
    if (verbose) puts("STUB: Media3DSetCameraAngleAspect called");
    return NULL;
}

void* Media3DSetCameraData(void)
{
    if (verbose) puts("STUB: Media3DSetCameraData called");
    return NULL;
}

void* Media3DSetCameraRange(void)
{
    if (verbose) puts("STUB: Media3DSetCameraRange called");
    return NULL;
}

void* Media3DSetMasterMatrix(void)
{
    if (verbose) puts("STUB: Media3DSetMasterMatrix called");
    return NULL;
}

void* Media3DTranslateNamedObjectTo(void)
{
    if (verbose) puts("STUB: Media3DTranslateNamedObjectTo called");
    return NULL;
}

void* MediaAddDataRefBlob(void)
{
    if (verbose) puts("STUB: MediaAddDataRefBlob called");
    return NULL;
}

void* MediaAddSample(void)
{
    if (verbose) puts("STUB: MediaAddSample called");
    return NULL;
}

void* MediaAddSampleB(void)
{
    if (verbose) puts("STUB: MediaAddSampleB called");
    return NULL;
}

void* MediaAddSampleReference(void)
{
    if (verbose) puts("STUB: MediaAddSampleReference called");
    return NULL;
}

void* MediaAddSampleReferences(void)
{
    if (verbose) puts("STUB: MediaAddSampleReferences called");
    return NULL;
}

void* MediaAddSampleReferencesS(void)
{
    if (verbose) puts("STUB: MediaAddSampleReferencesS called");
    return NULL;
}

void* MediaAddSampleTable(void)
{
    if (verbose) puts("STUB: MediaAddSampleTable called");
    return NULL;
}

void* MediaBeginEdits(void)
{
    if (verbose) puts("STUB: MediaBeginEdits called");
    return NULL;
}

void* MediaCCBlowChunks(void)
{
    if (verbose) puts("STUB: MediaCCBlowChunks called");
    return NULL;
}

void* MediaCCBlowPurgeableChunks(void)
{
    if (verbose) puts("STUB: MediaCCBlowPurgeableChunks called");
    return NULL;
}

void* MediaCCDisposeChunkCache(void)
{
    if (verbose) puts("STUB: MediaCCDisposeChunkCache called");
    return NULL;
}

void* MediaCCEmptyChunkCachePool(void)
{
    if (verbose) puts("STUB: MediaCCEmptyChunkCachePool called");
    return NULL;
}

void* MediaCCEmptyChunkH(void)
{
    if (verbose) puts("STUB: MediaCCEmptyChunkH called");
    return NULL;
}

void* MediaCCGetChunkCacheHandle(void)
{
    if (verbose) puts("STUB: MediaCCGetChunkCacheHandle called");
    return NULL;
}

void* MediaCCGetChunkDataH(void)
{
    if (verbose) puts("STUB: MediaCCGetChunkDataH called");
    return NULL;
}

void* MediaCCGetMasterChunkCacheHandle(void)
{
    if (verbose) puts("STUB: MediaCCGetMasterChunkCacheHandle called");
    return NULL;
}

void* MediaCCIncreaseChunkCache(void)
{
    if (verbose) puts("STUB: MediaCCIncreaseChunkCache called");
    return NULL;
}

void* MediaCCLockNLoadChunks(void)
{
    if (verbose) puts("STUB: MediaCCLockNLoadChunks called");
    return NULL;
}

void* MediaCCNewChunkCache(void)
{
    if (verbose) puts("STUB: MediaCCNewChunkCache called");
    return NULL;
}

void* MediaCCOKtoUsePools(void)
{
    if (verbose) puts("STUB: MediaCCOKtoUsePools called");
    return NULL;
}

void* MediaCCPrefetchChunkDataH(void)
{
    if (verbose) puts("STUB: MediaCCPrefetchChunkDataH called");
    return NULL;
}

void* MediaCCPurgeChunkH(void)
{
    if (verbose) puts("STUB: MediaCCPurgeChunkH called");
    return NULL;
}

void* MediaCCReleaseAndForgetChunkH(void)
{
    if (verbose) puts("STUB: MediaCCReleaseAndForgetChunkH called");
    return NULL;
}

void* MediaCCReleaseChunkH(void)
{
    if (verbose) puts("STUB: MediaCCReleaseChunkH called");
    return NULL;
}

void* MediaCCSetChunkHeaderSize(void)
{
    if (verbose) puts("STUB: MediaCCSetChunkHeaderSize called");
    return NULL;
}

void* MediaCCUnlockChunkH(void)
{
    if (verbose) puts("STUB: MediaCCUnlockChunkH called");
    return NULL;
}

void* MediaCCUseChunkCache(void)
{
    if (verbose) puts("STUB: MediaCCUseChunkCache called");
    return NULL;
}

void* MediaCCUseDataComponent(void)
{
    if (verbose) puts("STUB: MediaCCUseDataComponent called");
    return NULL;
}

void* MediaChangeDataHandler(void)
{
    if (verbose) puts("STUB: MediaChangeDataHandler called");
    return NULL;
}

void* MediaChangedNonPrimarySource(void)
{
    if (verbose) puts("STUB: MediaChangedNonPrimarySource called");
    return NULL;
}

void* MediaClipChanged(void)
{
    if (verbose) puts("STUB: MediaClipChanged called");
    return NULL;
}

void* MediaCloneDataRefContainer(void)
{
    if (verbose) puts("STUB: MediaCloneDataRefContainer called");
    return NULL;
}

void* MediaCloneMedia(void)
{
    if (verbose) puts("STUB: MediaCloneMedia called");
    return NULL;
}

void* MediaCompare(void)
{
    if (verbose) puts("STUB: MediaCompare called");
    return NULL;
}

void* MediaCompareDataRefBlob(void)
{
    if (verbose) puts("STUB: MediaCompareDataRefBlob called");
    return NULL;
}

void* MediaCompareForCopy(void)
{
    if (verbose) puts("STUB: MediaCompareForCopy called");
    return NULL;
}

void* MediaContainsDisplayOffsets(void)
{
    if (verbose) puts("STUB: MediaContainsDisplayOffsets called");
    return NULL;
}

void* MediaCopyAudioOutputDeviceUID(void)
{
    if (verbose) puts("STUB: MediaCopyAudioOutputDeviceUID called");
    return NULL;
}

void* MediaCopyMutableSampleTable(void)
{
    if (verbose) puts("STUB: MediaCopyMutableSampleTable called");
    return NULL;
}

void* MediaCopySettingsFromComponent(void)
{
    if (verbose) puts("STUB: MediaCopySettingsFromComponent called");
    return NULL;
}

void* MediaCurrentMediaQueuedData(void)
{
    if (verbose) puts("STUB: MediaCurrentMediaQueuedData called");
    return NULL;
}

void* MediaDecodeTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaDecodeTimeToSampleNum called");
    return NULL;
}

void* MediaDimensionChanged(void)
{
    if (verbose) puts("STUB: MediaDimensionChanged called");
    return NULL;
}

void* MediaDisplayTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaDisplayTimeToSampleNum called");
    return NULL;
}

void* MediaDisposeMediaStateCache(void)
{
    if (verbose) puts("STUB: MediaDisposeMediaStateCache called");
    return NULL;
}

void* MediaDisposeTargetRefCon(void)
{
    if (verbose) puts("STUB: MediaDisposeTargetRefCon called");
    return NULL;
}

void* MediaDoIdleActions(void)
{
    if (verbose) puts("STUB: MediaDoIdleActions called");
    return NULL;
}

void* MediaEmptyAllPurgeableChunks(void)
{
    if (verbose) puts("STUB: MediaEmptyAllPurgeableChunks called");
    return NULL;
}

void* MediaEmptySampleCache(void)
{
    if (verbose) puts("STUB: MediaEmptySampleCache called");
    return NULL;
}

void* MediaEnable(void)
{
    if (verbose) puts("STUB: MediaEnable called");
    return NULL;
}

void* MediaEndEdits(void)
{
    if (verbose) puts("STUB: MediaEndEdits called");
    return NULL;
}

void* MediaEnterEmptyEdit(void)
{
    if (verbose) puts("STUB: MediaEnterEmptyEdit called");
    return NULL;
}

void* MediaFindFirstKeySampleTime(void)
{
    if (verbose) puts("STUB: MediaFindFirstKeySampleTime called");
    return NULL;
}

void* MediaFixSampleDescription(void)
{
    if (verbose) puts("STUB: MediaFixSampleDescription called");
    return NULL;
}

void* MediaFlushNonPrimarySourceData(void)
{
    if (verbose) puts("STUB: MediaFlushNonPrimarySourceData called");
    return NULL;
}

void* MediaForceUpdate(void)
{
    if (verbose) puts("STUB: MediaForceUpdate called");
    return NULL;
}

void* MediaGGetAudioContext(void)
{
    if (verbose) puts("STUB: MediaGGetAudioContext called");
    return NULL;
}

void* MediaGGetIdleManager(void)
{
    if (verbose) puts("STUB: MediaGGetIdleManager called");
    return NULL;
}

void* MediaGGetLatency(void)
{
    if (verbose) puts("STUB: MediaGGetLatency called");
    return NULL;
}

void* MediaGGetStatus(void)
{
    if (verbose) puts("STUB: MediaGGetStatus called");
    return NULL;
}

void* MediaGGetVisualContext(void)
{
    if (verbose) puts("STUB: MediaGGetVisualContext called");
    return NULL;
}

void* MediaGSetActiveSegment(void)
{
    if (verbose) puts("STUB: MediaGSetActiveSegment called");
    return NULL;
}

void* MediaGSetIdleManager(void)
{
    if (verbose) puts("STUB: MediaGSetIdleManager called");
    return NULL;
}

void* MediaGSetParentAudioContext(void)
{
    if (verbose) puts("STUB: MediaGSetParentAudioContext called");
    return NULL;
}

void* MediaGSetVisualContext(void)
{
    if (verbose) puts("STUB: MediaGSetVisualContext called");
    return NULL;
}

void* MediaGSetVolume(void)
{
    if (verbose) puts("STUB: MediaGSetVolume called");
    return NULL;
}

void* MediaGWorldChanged(void)
{
    if (verbose) puts("STUB: MediaGWorldChanged called");
    return NULL;
}

void* MediaGenerateApertureModeDimensions(void)
{
    if (verbose) puts("STUB: MediaGenerateApertureModeDimensions called");
    return NULL;
}

void* MediaGetActionsForQTEvent(void)
{
    if (verbose) puts("STUB: MediaGetActionsForQTEvent called");
    return NULL;
}

void* MediaGetApertureModeClipRectForSampleDescriptionIndex(void)
{
    if (verbose) puts("STUB: MediaGetApertureModeClipRectForSampleDescriptionIndex called");
    return NULL;
}

void* MediaGetApertureModeMatrixForSampleDescriptionIndex(void)
{
    if (verbose) puts("STUB: MediaGetApertureModeMatrixForSampleDescriptionIndex called");
    return NULL;
}

void* MediaGetAudioContext(void)
{
    if (verbose) puts("STUB: MediaGetAudioContext called");
    return NULL;
}

void* MediaGetChunkManagementFlags(void)
{
    if (verbose) puts("STUB: MediaGetChunkManagementFlags called");
    return NULL;
}

void* MediaGetClock(void)
{
    if (verbose) puts("STUB: MediaGetClock called");
    return NULL;
}

void* MediaGetCurrentSampleTrackTime(void)
{
    if (verbose) puts("STUB: MediaGetCurrentSampleTrackTime called");
    return NULL;
}

void* MediaGetCustomMediaAtom(void)
{
    if (verbose) puts("STUB: MediaGetCustomMediaAtom called");
    return NULL;
}

void* MediaGetCustomMediaHeaderType(void)
{
    if (verbose) puts("STUB: MediaGetCustomMediaHeaderType called");
    return NULL;
}

void* MediaGetDataHandler(void)
{
    if (verbose) puts("STUB: MediaGetDataHandler called");
    return NULL;
}

void* MediaGetDataHandlerDescription(void)
{
    if (verbose) puts("STUB: MediaGetDataHandlerDescription called");
    return NULL;
}

void* MediaGetDataRefBlob(void)
{
    if (verbose) puts("STUB: MediaGetDataRefBlob called");
    return NULL;
}

void* MediaGetDataRefBlobCount(void)
{
    if (verbose) puts("STUB: MediaGetDataRefBlobCount called");
    return NULL;
}

void* MediaGetDataSize(void)
{
    if (verbose) puts("STUB: MediaGetDataSize called");
    return NULL;
}

void* MediaGetDataSize64(void)
{
    if (verbose) puts("STUB: MediaGetDataSize64 called");
    return NULL;
}

void* MediaGetDecodeDataSize64(void)
{
    if (verbose) puts("STUB: MediaGetDecodeDataSize64 called");
    return NULL;
}

void* MediaGetDisplayDataSize64(void)
{
    if (verbose) puts("STUB: MediaGetDisplayDataSize64 called");
    return NULL;
}

void* MediaGetDrawingRgn(void)
{
    if (verbose) puts("STUB: MediaGetDrawingRgn called");
    return NULL;
}

void* MediaGetEffectiveSoundBalance(void)
{
    if (verbose) puts("STUB: MediaGetEffectiveSoundBalance called");
    return NULL;
}

void* MediaGetEffectiveVolume(void)
{
    if (verbose) puts("STUB: MediaGetEffectiveVolume called");
    return NULL;
}

void* MediaGetErrorString(void)
{
    if (verbose) puts("STUB: MediaGetErrorString called");
    return NULL;
}

void* MediaGetGraphicsMode(void)
{
    if (verbose) puts("STUB: MediaGetGraphicsMode called");
    return NULL;
}

void* MediaGetIdleManager(void)
{
    if (verbose) puts("STUB: MediaGetIdleManager called");
    return NULL;
}

void* MediaGetInvalidRegion(void)
{
    if (verbose) puts("STUB: MediaGetInvalidRegion called");
    return NULL;
}

void* MediaGetLatency(void)
{
    if (verbose) puts("STUB: MediaGetLatency called");
    return NULL;
}

void* MediaGetLoadState(void)
{
    if (verbose) puts("STUB: MediaGetLoadState called");
    return NULL;
}

void* MediaGetMediaInfo(void)
{
    if (verbose) puts("STUB: MediaGetMediaInfo called");
    return NULL;
}

void* MediaGetMediaInfoAtom(void)
{
    if (verbose) puts("STUB: MediaGetMediaInfoAtom called");
    return NULL;
}

void* MediaGetMediaInfoAtomList(void)
{
    if (verbose) puts("STUB: MediaGetMediaInfoAtomList called");
    return NULL;
}

void* MediaGetMediaLoadState(void)
{
    if (verbose) puts("STUB: MediaGetMediaLoadState called");
    return NULL;
}

void* MediaGetMediaMatrix(void)
{
    if (verbose) puts("STUB: MediaGetMediaMatrix called");
    return NULL;
}

void* MediaGetMovieComplete(void)
{
    if (verbose) puts("STUB: MediaGetMovieComplete called");
    return NULL;
}

void* MediaGetName(void)
{
    if (verbose) puts("STUB: MediaGetName called");
    return NULL;
}

void* MediaGetNextBoundsChange(void)
{
    if (verbose) puts("STUB: MediaGetNextBoundsChange called");
    return NULL;
}

void* MediaGetNextSpatialChange(void)
{
    if (verbose) puts("STUB: MediaGetNextSpatialChange called");
    return NULL;
}

void* MediaGetNextStepTime(void)
{
    if (verbose) puts("STUB: MediaGetNextStepTime called");
    return NULL;
}

void* MediaGetOffscreenBufferSize(void)
{
    if (verbose) puts("STUB: MediaGetOffscreenBufferSize called");
    return NULL;
}

void* MediaGetPlayBackAbility(void)
{
    if (verbose) puts("STUB: MediaGetPlayBackAbility called");
    return NULL;
}

void* MediaGetPlayHints(void)
{
    if (verbose) puts("STUB: MediaGetPlayHints called");
    return NULL;
}

void* MediaGetPreRollTime(void)
{
    if (verbose) puts("STUB: MediaGetPreRollTime called");
    return NULL;
}

void* MediaGetPreferredChunkSize(void)
{
    if (verbose) puts("STUB: MediaGetPreferredChunkSize called");
    return NULL;
}

void* MediaGetPrivateInfo(void)
{
    if (verbose) puts("STUB: MediaGetPrivateInfo called");
    return NULL;
}

void* MediaGetPropertyAtom(void)
{
    if (verbose) puts("STUB: MediaGetPropertyAtom called");
    return NULL;
}

void* MediaGetPublicInfo(void)
{
    if (verbose) puts("STUB: MediaGetPublicInfo called");
    return NULL;
}

void* MediaGetPurgeableChunkMemoryAllowance(void)
{
    if (verbose) puts("STUB: MediaGetPurgeableChunkMemoryAllowance called");
    return NULL;
}

void* MediaGetSample(void)
{
    if (verbose) puts("STUB: MediaGetSample called");
    return NULL;
}

void* MediaGetSampleB(void)
{
    if (verbose) puts("STUB: MediaGetSampleB called");
    return NULL;
}

void* MediaGetSampleBAsHandle(void)
{
    if (verbose) puts("STUB: MediaGetSampleBAsHandle called");
    return NULL;
}

void* MediaGetSampleCount(void)
{
    if (verbose) puts("STUB: MediaGetSampleCount called");
    return NULL;
}

void* MediaGetSampleDataPointer(void)
{
    if (verbose) puts("STUB: MediaGetSampleDataPointer called");
    return NULL;
}

void* MediaGetSampleDescription(void)
{
    if (verbose) puts("STUB: MediaGetSampleDescription called");
    return NULL;
}

void* MediaGetSampleDescriptionCount(void)
{
    if (verbose) puts("STUB: MediaGetSampleDescriptionCount called");
    return NULL;
}

void* MediaGetSampleID(void)
{
    if (verbose) puts("STUB: MediaGetSampleID called");
    return NULL;
}

void* MediaGetSampleReference(void)
{
    if (verbose) puts("STUB: MediaGetSampleReference called");
    return NULL;
}

void* MediaGetSampleReferencesS(void)
{
    if (verbose) puts("STUB: MediaGetSampleReferencesS called");
    return NULL;
}

void* MediaGetSampleReferencesS64(void)
{
    if (verbose) puts("STUB: MediaGetSampleReferencesS64 called");
    return NULL;
}

void* MediaGetScheduleAheadTime(void)
{
    if (verbose) puts("STUB: MediaGetScheduleAheadTime called");
    return NULL;
}

void* MediaGetShadowSync(void)
{
    if (verbose) puts("STUB: MediaGetShadowSync called");
    return NULL;
}

void* MediaGetSoundBalance(void)
{
    if (verbose) puts("STUB: MediaGetSoundBalance called");
    return NULL;
}

void* MediaGetSoundBassAndTreble(void)
{
    if (verbose) puts("STUB: MediaGetSoundBassAndTreble called");
    return NULL;
}

void* MediaGetSoundEqualizerBandLevels(void)
{
    if (verbose) puts("STUB: MediaGetSoundEqualizerBandLevels called");
    return NULL;
}

void* MediaGetSoundEqualizerBands(void)
{
    if (verbose) puts("STUB: MediaGetSoundEqualizerBands called");
    return NULL;
}

void* MediaGetSoundLevelMeterInfo(void)
{
    if (verbose) puts("STUB: MediaGetSoundLevelMeterInfo called");
    return NULL;
}

void* MediaGetSoundLevelMeteringEnabled(void)
{
    if (verbose) puts("STUB: MediaGetSoundLevelMeteringEnabled called");
    return NULL;
}

void* MediaGetSoundOutputComponent(void)
{
    if (verbose) puts("STUB: MediaGetSoundOutputComponent called");
    return NULL;
}

void* MediaGetSrcRgn(void)
{
    if (verbose) puts("STUB: MediaGetSrcRgn called");
    return NULL;
}

void* MediaGetSrcSpatialRgn(void)
{
    if (verbose) puts("STUB: MediaGetSrcSpatialRgn called");
    return NULL;
}

void* MediaGetStatus(void)
{
    if (verbose) puts("STUB: MediaGetStatus called");
    return NULL;
}

void* MediaGetTrackOpaque(void)
{
    if (verbose) puts("STUB: MediaGetTrackOpaque called");
    return NULL;
}

void* MediaGetURLLink(void)
{
    if (verbose) puts("STUB: MediaGetURLLink called");
    return NULL;
}

void* MediaGetUserPreferredCodecs(void)
{
    if (verbose) puts("STUB: MediaGetUserPreferredCodecs called");
    return NULL;
}

void* MediaGetVideoParam(void)
{
    if (verbose) puts("STUB: MediaGetVideoParam called");
    return NULL;
}

void* MediaGetVisualContext(void)
{
    if (verbose) puts("STUB: MediaGetVisualContext called");
    return NULL;
}

void* MediaGrout(void)
{
    if (verbose) puts("STUB: MediaGrout called");
    return NULL;
}

void* MediaHandlerInit(void)
{
    if (verbose) puts("STUB: MediaHandlerInit called");
    return NULL;
}

void* MediaHasCharacteristic(void)
{
    if (verbose) puts("STUB: MediaHasCharacteristic called");
    return NULL;
}

void* MediaHitTestForTargetRefCon(void)
{
    if (verbose) puts("STUB: MediaHitTestForTargetRefCon called");
    return NULL;
}

void* MediaHitTestTargetRefCon(void)
{
    if (verbose) puts("STUB: MediaHitTestTargetRefCon called");
    return NULL;
}

void* MediaIdle(void)
{
    if (verbose) puts("STUB: MediaIdle called");
    return NULL;
}

void* MediaInitialize(void)
{
    if (verbose) puts("STUB: MediaInitialize called");
    return NULL;
}

void* MediaInvalidateRegion(void)
{
    if (verbose) puts("STUB: MediaInvalidateRegion called");
    return NULL;
}

void* MediaIsDataRefOK(void)
{
    if (verbose) puts("STUB: MediaIsDataRefOK called");
    return NULL;
}

void* MediaIsTrackOpaque(void)
{
    if (verbose) puts("STUB: MediaIsTrackOpaque called");
    return NULL;
}

void* MediaLoadIntoRam(void)
{
    if (verbose) puts("STUB: MediaLoadIntoRam called");
    return NULL;
}

void* MediaLoadIntoRamComplete(void)
{
    if (verbose) puts("STUB: MediaLoadIntoRamComplete called");
    return NULL;
}

void* MediaMCIsPlayerEvent(void)
{
    if (verbose) puts("STUB: MediaMCIsPlayerEvent called");
    return NULL;
}

void* MediaMakeMediaTimeTable(void)
{
    if (verbose) puts("STUB: MediaMakeMediaTimeTable called");
    return NULL;
}

void* MediaMakeMediaTimeTable64(void)
{
    if (verbose) puts("STUB: MediaMakeMediaTimeTable64 called");
    return NULL;
}

void* MediaMakeMediaTimeTable64Priv(void)
{
    if (verbose) puts("STUB: MediaMakeMediaTimeTable64Priv called");
    return NULL;
}

void* MediaMakeMediaTimeTableOld(void)
{
    if (verbose) puts("STUB: MediaMakeMediaTimeTableOld called");
    return NULL;
}

void* MediaMakeMediaTimeTablePriv(void)
{
    if (verbose) puts("STUB: MediaMakeMediaTimeTablePriv called");
    return NULL;
}

void* MediaMatrixChanged(void)
{
    if (verbose) puts("STUB: MediaMatrixChanged called");
    return NULL;
}

void* MediaMediaDecodeTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaMediaDecodeTimeToSampleNum called");
    return NULL;
}

void* MediaMediaDisplayTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaMediaDisplayTimeToSampleNum called");
    return NULL;
}

void* MediaMediaStateCacheMediaTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaMediaStateCacheMediaTimeToSampleNum called");
    return NULL;
}

void* MediaMediaTimeScaleChanged(void)
{
    if (verbose) puts("STUB: MediaMediaTimeScaleChanged called");
    return NULL;
}

void* MediaMediaTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaMediaTimeToSampleNum called");
    return NULL;
}

void* MediaMovieDataRefChanged(void)
{
    if (verbose) puts("STUB: MediaMovieDataRefChanged called");
    return NULL;
}

void* MediaMovieTimeScaleChanged(void)
{
    if (verbose) puts("STUB: MediaMovieTimeScaleChanged called");
    return NULL;
}

void* MediaMoviesTask(void)
{
    if (verbose) puts("STUB: MediaMoviesTask called");
    return NULL;
}

void* MediaNavigateTargetRefCon(void)
{
    if (verbose) puts("STUB: MediaNavigateTargetRefCon called");
    return NULL;
}

void* MediaNewMediaStateCache(void)
{
    if (verbose) puts("STUB: MediaNewMediaStateCache called");
    return NULL;
}

void* MediaNextInterestingTime(void)
{
    if (verbose) puts("STUB: MediaNextInterestingTime called");
    return NULL;
}

void* MediaNextInterestingTime64(void)
{
    if (verbose) puts("STUB: MediaNextInterestingTime64 called");
    return NULL;
}

void* MediaObsolete(void)
{
    if (verbose) puts("STUB: MediaObsolete called");
    return NULL;
}

void* MediaPerformance(void)
{
    if (verbose) puts("STUB: MediaPerformance called");
    return NULL;
}

void* MediaPrePrerollBegin(void)
{
    if (verbose) puts("STUB: MediaPrePrerollBegin called");
    return NULL;
}

void* MediaPrePrerollCancel(void)
{
    if (verbose) puts("STUB: MediaPrePrerollCancel called");
    return NULL;
}

void* MediaPreRoll(void)
{
    if (verbose) puts("STUB: MediaPreRoll called");
    return NULL;
}

void* MediaPreRollComplete(void)
{
    if (verbose) puts("STUB: MediaPreRollComplete called");
    return NULL;
}

void* MediaPreroll(void)
{
    if (verbose) puts("STUB: MediaPreroll called");
    return NULL;
}

void* MediaPutCustomMediaAtom(void)
{
    if (verbose) puts("STUB: MediaPutCustomMediaAtom called");
    return NULL;
}

void* MediaPutMediaInfo(void)
{
    if (verbose) puts("STUB: MediaPutMediaInfo called");
    return NULL;
}

void* MediaPutMediaInfoAtom(void)
{
    if (verbose) puts("STUB: MediaPutMediaInfoAtom called");
    return NULL;
}

void* MediaQueueNonPrimarySourceData(void)
{
    if (verbose) puts("STUB: MediaQueueNonPrimarySourceData called");
    return NULL;
}

void* MediaRateChanged(void)
{
    if (verbose) puts("STUB: MediaRateChanged called");
    return NULL;
}

void* MediaRefConGetProperty(void)
{
    if (verbose) puts("STUB: MediaRefConGetProperty called");
    return NULL;
}

void* MediaRefConSetProperty(void)
{
    if (verbose) puts("STUB: MediaRefConSetProperty called");
    return NULL;
}

void* MediaReferencesChanged(void)
{
    if (verbose) puts("STUB: MediaReferencesChanged called");
    return NULL;
}

void* MediaRefreshMediaStateCache(void)
{
    if (verbose) puts("STUB: MediaRefreshMediaStateCache called");
    return NULL;
}

void* MediaReleaseSampleDataPointer(void)
{
    if (verbose) puts("STUB: MediaReleaseSampleDataPointer called");
    return NULL;
}

void* MediaResolveTargetRefCon(void)
{
    if (verbose) puts("STUB: MediaResolveTargetRefCon called");
    return NULL;
}

void* MediaSTAddSampleDescription(void)
{
    if (verbose) puts("STUB: MediaSTAddSampleDescription called");
    return NULL;
}

void* MediaSTAddSampleReference(void)
{
    if (verbose) puts("STUB: MediaSTAddSampleReference called");
    return NULL;
}

void* MediaSTAddSampleReferences64(void)
{
    if (verbose) puts("STUB: MediaSTAddSampleReferences64 called");
    return NULL;
}

void* MediaSTAddSampleReferencesS64(void)
{
    if (verbose) puts("STUB: MediaSTAddSampleReferencesS64 called");
    return NULL;
}

void* MediaSTAddSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTAddSampleTable called");
    return NULL;
}

void* MediaSTAdjustSizeOfSamples(void)
{
    if (verbose) puts("STUB: MediaSTAdjustSizeOfSamples called");
    return NULL;
}

void* MediaSTChunkNumToChunkInfo(void)
{
    if (verbose) puts("STUB: MediaSTChunkNumToChunkInfo called");
    return NULL;
}

void* MediaSTCopyCachedSampleDataToBuffer(void)
{
    if (verbose) puts("STUB: MediaSTCopyCachedSampleDataToBuffer called");
    return NULL;
}

void* MediaSTCopyMetaData(void)
{
    if (verbose) puts("STUB: MediaSTCopyMetaData called");
    return NULL;
}

void* MediaSTCopyMutableSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTCopyMutableSampleTable called");
    return NULL;
}

void* MediaSTDecodeTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaSTDecodeTimeToSampleNum called");
    return NULL;
}

void* MediaSTDisplayTimeRangeToContainedDisplayTimeRange(void)
{
    if (verbose) puts("STUB: MediaSTDisplayTimeRangeToContainedDisplayTimeRange called");
    return NULL;
}

void* MediaSTDisplayTimeRangeToCopyBoundaryDecodeTimeRange(void)
{
    if (verbose) puts("STUB: MediaSTDisplayTimeRangeToCopyBoundaryDecodeTimeRange called");
    return NULL;
}

void* MediaSTDisplayTimeRangeToSampleNumRange(void)
{
    if (verbose) puts("STUB: MediaSTDisplayTimeRangeToSampleNumRange called");
    return NULL;
}

void* MediaSTDisplayTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaSTDisplayTimeToSampleNum called");
    return NULL;
}

void* MediaSTDisposeSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTDisposeSampleTable called");
    return NULL;
}

void* MediaSTFindFirstKeySampleDecodeTime(void)
{
    if (verbose) puts("STUB: MediaSTFindFirstKeySampleDecodeTime called");
    return NULL;
}

void* MediaSTFindFirstKeySampleDisplayTime(void)
{
    if (verbose) puts("STUB: MediaSTFindFirstKeySampleDisplayTime called");
    return NULL;
}

void* MediaSTFindFirstKeySampleTime(void)
{
    if (verbose) puts("STUB: MediaSTFindFirstKeySampleTime called");
    return NULL;
}

void* MediaSTFindPreviousCopyBoundarySampleDecodeTime(void)
{
    if (verbose) puts("STUB: MediaSTFindPreviousCopyBoundarySampleDecodeTime called");
    return NULL;
}

void* MediaSTFindPreviousCopyBoundarySampleDisplayTime(void)
{
    if (verbose) puts("STUB: MediaSTFindPreviousCopyBoundarySampleDisplayTime called");
    return NULL;
}

void* MediaSTGetCachedMinMaxOffsets(void)
{
    if (verbose) puts("STUB: MediaSTGetCachedMinMaxOffsets called");
    return NULL;
}

void* MediaSTGetChunkDataHandler(void)
{
    if (verbose) puts("STUB: MediaSTGetChunkDataHandler called");
    return NULL;
}

void* MediaSTGetChunkOffset64Size(void)
{
    if (verbose) puts("STUB: MediaSTGetChunkOffset64Size called");
    return NULL;
}

void* MediaSTGetChunkOffsetSize(void)
{
    if (verbose) puts("STUB: MediaSTGetChunkOffsetSize called");
    return NULL;
}

void* MediaSTGetMetaDataAssociation(void)
{
    if (verbose) puts("STUB: MediaSTGetMetaDataAssociation called");
    return NULL;
}

void* MediaSTGetMetaDataKeyAtIndex(void)
{
    if (verbose) puts("STUB: MediaSTGetMetaDataKeyAtIndex called");
    return NULL;
}

void* MediaSTGetMinMaxOffsets(void)
{
    if (verbose) puts("STUB: MediaSTGetMinMaxOffsets called");
    return NULL;
}

void* MediaSTGetNumChunks(void)
{
    if (verbose) puts("STUB: MediaSTGetNumChunks called");
    return NULL;
}

void* MediaSTGetQTListRootElement(void)
{
    if (verbose) puts("STUB: MediaSTGetQTListRootElement called");
    return NULL;
}

void* MediaSTGetSampleCount(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleCount called");
    return NULL;
}

void* MediaSTGetSampleData(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleData called");
    return NULL;
}

void* MediaSTGetSampleDescription(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleDescription called");
    return NULL;
}

void* MediaSTGetSampleDescriptionCnt(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleDescriptionCnt called");
    return NULL;
}

void* MediaSTGetSampleFlags(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleFlags called");
    return NULL;
}

void* MediaSTGetSampleQuanta(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleQuanta called");
    return NULL;
}

void* MediaSTGetSampleQuantaAdjustment(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleQuantaAdjustment called");
    return NULL;
}

void* MediaSTGetSampleReference(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleReference called");
    return NULL;
}

void* MediaSTGetSampleReferencesS64(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleReferencesS64 called");
    return NULL;
}

void* MediaSTGetSampleReusePolicy(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleReusePolicy called");
    return NULL;
}

void* MediaSTGetSampleTableAtom(void)
{
    if (verbose) puts("STUB: MediaSTGetSampleTableAtom called");
    return NULL;
}

void* MediaSTGetSizeOfSamples(void)
{
    if (verbose) puts("STUB: MediaSTGetSizeOfSamples called");
    return NULL;
}

void* MediaSTGetSizeOfSamples64(void)
{
    if (verbose) puts("STUB: MediaSTGetSizeOfSamples64 called");
    return NULL;
}

void* MediaSTGetTimeBoundaries(void)
{
    if (verbose) puts("STUB: MediaSTGetTimeBoundaries called");
    return NULL;
}

void* MediaSTGetTimeBoundariesExtendedInfo(void)
{
    if (verbose) puts("STUB: MediaSTGetTimeBoundariesExtendedInfo called");
    return NULL;
}

void* MediaSTGrout(void)
{
    if (verbose) puts("STUB: MediaSTGrout called");
    return NULL;
}

void* MediaSTMaxSamplesBetweenKeyFrames(void)
{
    if (verbose) puts("STUB: MediaSTMaxSamplesBetweenKeyFrames called");
    return NULL;
}

void* MediaSTMediaTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaSTMediaTimeToSampleNum called");
    return NULL;
}

void* MediaSTNewSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTNewSampleTable called");
    return NULL;
}

void* MediaSTNumPartialSyncSamples(void)
{
    if (verbose) puts("STUB: MediaSTNumPartialSyncSamples called");
    return NULL;
}

void* MediaSTNumSyncSamples(void)
{
    if (verbose) puts("STUB: MediaSTNumSyncSamples called");
    return NULL;
}

void* MediaSTPreflightAddSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTPreflightAddSampleTable called");
    return NULL;
}

void* MediaSTPrivateSampleTableGetSample(void)
{
    if (verbose) puts("STUB: MediaSTPrivateSampleTableGetSample called");
    return NULL;
}

void* MediaSTPutSampleTableAtom(void)
{
    if (verbose) puts("STUB: MediaSTPutSampleTableAtom called");
    return NULL;
}

void* MediaSTSampleDescriptionB2N(void)
{
    if (verbose) puts("STUB: MediaSTSampleDescriptionB2N called");
    return NULL;
}

void* MediaSTSampleDescriptionN2B(void)
{
    if (verbose) puts("STUB: MediaSTSampleDescriptionN2B called");
    return NULL;
}

void* MediaSTSampleDescriptionValidate(void)
{
    if (verbose) puts("STUB: MediaSTSampleDescriptionValidate called");
    return NULL;
}

void* MediaSTSampleDescriptionValidateAndRepair(void)
{
    if (verbose) puts("STUB: MediaSTSampleDescriptionValidateAndRepair called");
    return NULL;
}

void* MediaSTSampleNumRangeToDisplayTimeRange(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumRangeToDisplayTimeRange called");
    return NULL;
}

void* MediaSTSampleNumToChunkInfo(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToChunkInfo called");
    return NULL;
}

void* MediaSTSampleNumToDecodeTime(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToDecodeTime called");
    return NULL;
}

void* MediaSTSampleNumToDisplayTime(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToDisplayTime called");
    return NULL;
}

void* MediaSTSampleNumToMediaTime(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToMediaTime called");
    return NULL;
}

void* MediaSTSampleNumToNextDescription(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToNextDescription called");
    return NULL;
}

void* MediaSTSampleNumToNextDisplayTime(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToNextDisplayTime called");
    return NULL;
}

void* MediaSTSampleNumToPartialSyncSamples(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToPartialSyncSamples called");
    return NULL;
}

void* MediaSTSampleNumToShadowSyncs(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToShadowSyncs called");
    return NULL;
}

void* MediaSTSampleNumToSyncSamples(void)
{
    if (verbose) puts("STUB: MediaSTSampleNumToSyncSamples called");
    return NULL;
}

void* MediaSTSampleTableGetChunkNum(void)
{
    if (verbose) puts("STUB: MediaSTSampleTableGetChunkNum called");
    return NULL;
}

void* MediaSTSampleTableGetSampleInfo(void)
{
    if (verbose) puts("STUB: MediaSTSampleTableGetSampleInfo called");
    return NULL;
}

void* MediaSTSampleTableGetSampleNum(void)
{
    if (verbose) puts("STUB: MediaSTSampleTableGetSampleNum called");
    return NULL;
}

void* MediaSTSampleTableSync(void)
{
    if (verbose) puts("STUB: MediaSTSampleTableSync called");
    return NULL;
}

void* MediaSTSeal(void)
{
    if (verbose) puts("STUB: MediaSTSeal called");
    return NULL;
}

void* MediaSTSetMetaDataAssociation(void)
{
    if (verbose) puts("STUB: MediaSTSetMetaDataAssociation called");
    return NULL;
}

void* MediaSTSetSampleDescription(void)
{
    if (verbose) puts("STUB: MediaSTSetSampleDescription called");
    return NULL;
}

void* MediaSTUseSampleTable(void)
{
    if (verbose) puts("STUB: MediaSTUseSampleTable called");
    return NULL;
}

void* MediaSampleDescriptionB2N(void)
{
    if (verbose) puts("STUB: MediaSampleDescriptionB2N called");
    return NULL;
}

void* MediaSampleDescriptionChanged(void)
{
    if (verbose) puts("STUB: MediaSampleDescriptionChanged called");
    return NULL;
}

void* MediaSampleDescriptionN2B(void)
{
    if (verbose) puts("STUB: MediaSampleDescriptionN2B called");
    return NULL;
}

void* MediaSampleIDToSampleNumber(void)
{
    if (verbose) puts("STUB: MediaSampleIDToSampleNumber called");
    return NULL;
}

void* MediaSampleNumToDecodeTime(void)
{
    if (verbose) puts("STUB: MediaSampleNumToDecodeTime called");
    return NULL;
}

void* MediaSampleNumToDisplayTime(void)
{
    if (verbose) puts("STUB: MediaSampleNumToDisplayTime called");
    return NULL;
}

void* MediaSampleNumToMediaTime(void)
{
    if (verbose) puts("STUB: MediaSampleNumToMediaTime called");
    return NULL;
}

void* MediaSetActionsCallback(void)
{
    if (verbose) puts("STUB: MediaSetActionsCallback called");
    return NULL;
}

void* MediaSetActive(void)
{
    if (verbose) puts("STUB: MediaSetActive called");
    return NULL;
}

void* MediaSetActiveSegment(void)
{
    if (verbose) puts("STUB: MediaSetActiveSegment called");
    return NULL;
}

void* MediaSetAudioOutputDeviceUID(void)
{
    if (verbose) puts("STUB: MediaSetAudioOutputDeviceUID called");
    return NULL;
}

void* MediaSetCacheMode(void)
{
    if (verbose) puts("STUB: MediaSetCacheMode called");
    return NULL;
}

void* MediaSetChunkCache(void)
{
    if (verbose) puts("STUB: MediaSetChunkCache called");
    return NULL;
}

void* MediaSetChunkManagementFlags(void)
{
    if (verbose) puts("STUB: MediaSetChunkManagementFlags called");
    return NULL;
}

void* MediaSetClip(void)
{
    if (verbose) puts("STUB: MediaSetClip called");
    return NULL;
}

void* MediaSetDataCacheMode(void)
{
    if (verbose) puts("STUB: MediaSetDataCacheMode called");
    return NULL;
}

void* MediaSetDataHandler(void)
{
    if (verbose) puts("STUB: MediaSetDataHandler called");
    return NULL;
}

void* MediaSetDataRefBlob(void)
{
    if (verbose) puts("STUB: MediaSetDataRefBlob called");
    return NULL;
}

void* MediaSetDataRefBlobAttributes(void)
{
    if (verbose) puts("STUB: MediaSetDataRefBlobAttributes called");
    return NULL;
}

void* MediaSetDataRefContainer(void)
{
    if (verbose) puts("STUB: MediaSetDataRefContainer called");
    return NULL;
}

void* MediaSetDataScheduleMode(void)
{
    if (verbose) puts("STUB: MediaSetDataScheduleMode called");
    return NULL;
}

void* MediaSetDidDrawCallback(void)
{
    if (verbose) puts("STUB: MediaSetDidDrawCallback called");
    return NULL;
}

void* MediaSetDimensions(void)
{
    if (verbose) puts("STUB: MediaSetDimensions called");
    return NULL;
}

void* MediaSetDoMCActionCallback(void)
{
    if (verbose) puts("STUB: MediaSetDoMCActionCallback called");
    return NULL;
}

void* MediaSetForceSrcCopyTransferMode(void)
{
    if (verbose) puts("STUB: MediaSetForceSrcCopyTransferMode called");
    return NULL;
}

void* MediaSetGWorld(void)
{
    if (verbose) puts("STUB: MediaSetGWorld called");
    return NULL;
}

void* MediaSetGWorlds(void)
{
    if (verbose) puts("STUB: MediaSetGWorlds called");
    return NULL;
}

void* MediaSetGraphicsMode(void)
{
    if (verbose) puts("STUB: MediaSetGraphicsMode called");
    return NULL;
}

void* MediaSetHandlerCapabilities(void)
{
    if (verbose) puts("STUB: MediaSetHandlerCapabilities called");
    return NULL;
}

void* MediaSetHints(void)
{
    if (verbose) puts("STUB: MediaSetHints called");
    return NULL;
}

void* MediaSetIdleManager(void)
{
    if (verbose) puts("STUB: MediaSetIdleManager called");
    return NULL;
}

void* MediaSetMatrix(void)
{
    if (verbose) puts("STUB: MediaSetMatrix called");
    return NULL;
}

void* MediaSetMatteReference(void)
{
    if (verbose) puts("STUB: MediaSetMatteReference called");
    return NULL;
}

void* MediaSetMediaTimeScale(void)
{
    if (verbose) puts("STUB: MediaSetMediaTimeScale called");
    return NULL;
}

void* MediaSetMovieTimeScale(void)
{
    if (verbose) puts("STUB: MediaSetMovieTimeScale called");
    return NULL;
}

void* MediaSetNonPrimarySourceData(void)
{
    if (verbose) puts("STUB: MediaSetNonPrimarySourceData called");
    return NULL;
}

void* MediaSetOffScreenBuffer(void)
{
    if (verbose) puts("STUB: MediaSetOffScreenBuffer called");
    return NULL;
}

void* MediaSetParentAudioContext(void)
{
    if (verbose) puts("STUB: MediaSetParentAudioContext called");
    return NULL;
}

void* MediaSetPlayHints(void)
{
    if (verbose) puts("STUB: MediaSetPlayHints called");
    return NULL;
}

void* MediaSetPreferredChunkSize(void)
{
    if (verbose) puts("STUB: MediaSetPreferredChunkSize called");
    return NULL;
}

void* MediaSetPrivateInfo(void)
{
    if (verbose) puts("STUB: MediaSetPrivateInfo called");
    return NULL;
}

void* MediaSetPropertyAtom(void)
{
    if (verbose) puts("STUB: MediaSetPropertyAtom called");
    return NULL;
}

void* MediaSetPublicInfo(void)
{
    if (verbose) puts("STUB: MediaSetPublicInfo called");
    return NULL;
}

void* MediaSetPurgeableChunkMemoryAllowance(void)
{
    if (verbose) puts("STUB: MediaSetPurgeableChunkMemoryAllowance called");
    return NULL;
}

void* MediaSetRate(void)
{
    if (verbose) puts("STUB: MediaSetRate called");
    return NULL;
}

void* MediaSetSampleDescription(void)
{
    if (verbose) puts("STUB: MediaSetSampleDescription called");
    return NULL;
}

void* MediaSetSampleID(void)
{
    if (verbose) puts("STUB: MediaSetSampleID called");
    return NULL;
}

void* MediaSetSampleTable(void)
{
    if (verbose) puts("STUB: MediaSetSampleTable called");
    return NULL;
}

void* MediaSetScheduleMode(void)
{
    if (verbose) puts("STUB: MediaSetScheduleMode called");
    return NULL;
}

void* MediaSetScreenLock(void)
{
    if (verbose) puts("STUB: MediaSetScreenLock called");
    return NULL;
}

void* MediaSetScrubMode(void)
{
    if (verbose) puts("STUB: MediaSetScrubMode called");
    return NULL;
}

void* MediaSetShadowSync(void)
{
    if (verbose) puts("STUB: MediaSetShadowSync called");
    return NULL;
}

void* MediaSetSoundBalance(void)
{
    if (verbose) puts("STUB: MediaSetSoundBalance called");
    return NULL;
}

void* MediaSetSoundBassAndTreble(void)
{
    if (verbose) puts("STUB: MediaSetSoundBassAndTreble called");
    return NULL;
}

void* MediaSetSoundEqualizerBands(void)
{
    if (verbose) puts("STUB: MediaSetSoundEqualizerBands called");
    return NULL;
}

void* MediaSetSoundLevelMeteringEnabled(void)
{
    if (verbose) puts("STUB: MediaSetSoundLevelMeteringEnabled called");
    return NULL;
}

void* MediaSetSoundLocalization(void)
{
    if (verbose) puts("STUB: MediaSetSoundLocalization called");
    return NULL;
}

void* MediaSetSoundLocalizationData(void)
{
    if (verbose) puts("STUB: MediaSetSoundLocalizationData called");
    return NULL;
}

void* MediaSetSoundOutputComponent(void)
{
    if (verbose) puts("STUB: MediaSetSoundOutputComponent called");
    return NULL;
}

void* MediaSetTrackApertureModeDimensionsUsingSampleDescription(void)
{
    if (verbose) puts("STUB: MediaSetTrackApertureModeDimensionsUsingSampleDescription called");
    return NULL;
}

void* MediaSetTrackInputMapReference(void)
{
    if (verbose) puts("STUB: MediaSetTrackInputMapReference called");
    return NULL;
}

void* MediaSetUserPreferredCodecs(void)
{
    if (verbose) puts("STUB: MediaSetUserPreferredCodecs called");
    return NULL;
}

void* MediaSetVideoParam(void)
{
    if (verbose) puts("STUB: MediaSetVideoParam called");
    return NULL;
}

void* MediaSetVisualContext(void)
{
    if (verbose) puts("STUB: MediaSetVisualContext called");
    return NULL;
}

void* MediaSetVolume(void)
{
    if (verbose) puts("STUB: MediaSetVolume called");
    return NULL;
}

void* MediaShowSample(void)
{
    if (verbose) puts("STUB: MediaShowSample called");
    return NULL;
}

void* MediaSuspendResume(void)
{
    if (verbose) puts("STUB: MediaSuspendResume called");
    return NULL;
}

void* MediaTargetRefConsEqual(void)
{
    if (verbose) puts("STUB: MediaTargetRefConsEqual called");
    return NULL;
}

void* MediaTimeBaseChanged(void)
{
    if (verbose) puts("STUB: MediaTimeBaseChanged called");
    return NULL;
}

void* MediaTimeChanged(void)
{
    if (verbose) puts("STUB: MediaTimeChanged called");
    return NULL;
}

void* MediaTimeToSampleNum(void)
{
    if (verbose) puts("STUB: MediaTimeToSampleNum called");
    return NULL;
}

void* MediaTrackEdited(void)
{
    if (verbose) puts("STUB: MediaTrackEdited called");
    return NULL;
}

void* MediaTrackEditedAtTime(void)
{
    if (verbose) puts("STUB: MediaTrackEditedAtTime called");
    return NULL;
}

void* MediaTrackInputMapChanged(void)
{
    if (verbose) puts("STUB: MediaTrackInputMapChanged called");
    return NULL;
}

void* MediaTrackPropertyAtomChanged(void)
{
    if (verbose) puts("STUB: MediaTrackPropertyAtomChanged called");
    return NULL;
}

void* MediaTrackReferencesChanged(void)
{
    if (verbose) puts("STUB: MediaTrackReferencesChanged called");
    return NULL;
}

void* MediaUnusued0(void)
{
    if (verbose) puts("STUB: MediaUnusued0 called");
    return NULL;
}

void* MediaUpdateMovie(void)
{
    if (verbose) puts("STUB: MediaUpdateMovie called");
    return NULL;
}

void* MediaVideoOutputChanged(void)
{
    if (verbose) puts("STUB: MediaVideoOutputChanged called");
    return NULL;
}

void* MergeComponentInterfaceDictionaries(void)
{
    if (verbose) puts("STUB: MergeComponentInterfaceDictionaries called");
    return NULL;
}

void* MovieAudioExtractionBegin(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionBegin called");
    return NULL;
}

void* MovieAudioExtractionEnd(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionEnd called");
    return NULL;
}

void* MovieAudioExtractionFillBuffer(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionFillBuffer called");
    return NULL;
}

void* MovieAudioExtractionGetProperty(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionGetProperty called");
    return NULL;
}

void* MovieAudioExtractionGetPropertyInfo(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionGetPropertyInfo called");
    return NULL;
}

void* MovieAudioExtractionSetProperty(void)
{
    if (verbose) puts("STUB: MovieAudioExtractionSetProperty called");
    return NULL;
}

void* MovieExecuteWiredActions(void)
{
    if (verbose) puts("STUB: MovieExecuteWiredActions called");
    return NULL;
}

void* MovieExportAddDataSource(void)
{
    if (verbose) puts("STUB: MovieExportAddDataSource called");
    return NULL;
}

void* MovieExportDisposeGetDataAndPropertiesProcs(void)
{
    if (verbose) puts("STUB: MovieExportDisposeGetDataAndPropertiesProcs called");
    return NULL;
}

void* MovieExportDoUserDialog(void)
{
    if (verbose) puts("STUB: MovieExportDoUserDialog called");
    return NULL;
}

void* MovieExportFromProceduresToDataRef(void)
{
    if (verbose) puts("STUB: MovieExportFromProceduresToDataRef called");
    return NULL;
}

void* MovieExportGetAuxiliaryData(void)
{
    if (verbose) puts("STUB: MovieExportGetAuxiliaryData called");
    return NULL;
}

void* MovieExportGetCreatorType(void)
{
    if (verbose) puts("STUB: MovieExportGetCreatorType called");
    return NULL;
}

void* MovieExportGetFileNameExtension(void)
{
    if (verbose) puts("STUB: MovieExportGetFileNameExtension called");
    return NULL;
}

void* MovieExportGetSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: MovieExportGetSettingsAsAtomContainer called");
    return NULL;
}

void* MovieExportGetSettingsAsDictionary(void)
{
    if (verbose) puts("STUB: MovieExportGetSettingsAsDictionary called");
    return NULL;
}

void* MovieExportGetShortFileTypeString(void)
{
    if (verbose) puts("STUB: MovieExportGetShortFileTypeString called");
    return NULL;
}

void* MovieExportGetSourceMediaType(void)
{
    if (verbose) puts("STUB: MovieExportGetSourceMediaType called");
    return NULL;
}

void* MovieExportNewGetDataAndPropertiesProcs(void)
{
    if (verbose) puts("STUB: MovieExportNewGetDataAndPropertiesProcs called");
    return NULL;
}

void* MovieExportSetGetMoviePropertyProc(void)
{
    if (verbose) puts("STUB: MovieExportSetGetMoviePropertyProc called");
    return NULL;
}

void* MovieExportSetProgressProc(void)
{
    if (verbose) puts("STUB: MovieExportSetProgressProc called");
    return NULL;
}

void* MovieExportSetSampleDescription(void)
{
    if (verbose) puts("STUB: MovieExportSetSampleDescription called");
    return NULL;
}

void* MovieExportSetSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: MovieExportSetSettingsFromAtomContainer called");
    return NULL;
}

void* MovieExportSetSettingsFromDictionary(void)
{
    if (verbose) puts("STUB: MovieExportSetSettingsFromDictionary called");
    return NULL;
}

void* MovieExportToDataRef(void)
{
    if (verbose) puts("STUB: MovieExportToDataRef called");
    return NULL;
}

void* MovieExportToFile(void)
{
    if (verbose) puts("STUB: MovieExportToFile called");
    return NULL;
}

void* MovieExportToHandle(void)
{
    if (verbose) puts("STUB: MovieExportToHandle called");
    return NULL;
}

void* MovieExportValidate(void)
{
    if (verbose) puts("STUB: MovieExportValidate called");
    return NULL;
}

void* MovieImportDataRef(void)
{
    if (verbose) puts("STUB: MovieImportDataRef called");
    return NULL;
}

void* MovieImportDoUserDialog(void)
{
    if (verbose) puts("STUB: MovieImportDoUserDialog called");
    return NULL;
}

void* MovieImportDoUserDialogDataRef(void)
{
    if (verbose) puts("STUB: MovieImportDoUserDialogDataRef called");
    return NULL;
}

void* MovieImportEstimateCompletionTime(void)
{
    if (verbose) puts("STUB: MovieImportEstimateCompletionTime called");
    return NULL;
}

void* MovieImportFile(void)
{
    if (verbose) puts("STUB: MovieImportFile called");
    return NULL;
}

void* MovieImportGetAuxiliaryDataType(void)
{
    if (verbose) puts("STUB: MovieImportGetAuxiliaryDataType called");
    return NULL;
}

void* MovieImportGetDestinationMediaType(void)
{
    if (verbose) puts("STUB: MovieImportGetDestinationMediaType called");
    return NULL;
}

void* MovieImportGetDontBlock(void)
{
    if (verbose) puts("STUB: MovieImportGetDontBlock called");
    return NULL;
}

void* MovieImportGetFileType(void)
{
    if (verbose) puts("STUB: MovieImportGetFileType called");
    return NULL;
}

void* MovieImportGetLoadState(void)
{
    if (verbose) puts("STUB: MovieImportGetLoadState called");
    return NULL;
}

void* MovieImportGetMIMETypeList(void)
{
    if (verbose) puts("STUB: MovieImportGetMIMETypeList called");
    return NULL;
}

void* MovieImportGetMaxLoadedTime(void)
{
    if (verbose) puts("STUB: MovieImportGetMaxLoadedTime called");
    return NULL;
}

void* MovieImportGetSampleDescription(void)
{
    if (verbose) puts("STUB: MovieImportGetSampleDescription called");
    return NULL;
}

void* MovieImportGetSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: MovieImportGetSettingsAsAtomContainer called");
    return NULL;
}

void* MovieImportHandle(void)
{
    if (verbose) puts("STUB: MovieImportHandle called");
    return NULL;
}

void* MovieImportIdle(void)
{
    if (verbose) puts("STUB: MovieImportIdle called");
    return NULL;
}

void* MovieImportSetAuxiliaryData(void)
{
    if (verbose) puts("STUB: MovieImportSetAuxiliaryData called");
    return NULL;
}

void* MovieImportSetChunkSize(void)
{
    if (verbose) puts("STUB: MovieImportSetChunkSize called");
    return NULL;
}

void* MovieImportSetDimensions(void)
{
    if (verbose) puts("STUB: MovieImportSetDimensions called");
    return NULL;
}

void* MovieImportSetDontBlock(void)
{
    if (verbose) puts("STUB: MovieImportSetDontBlock called");
    return NULL;
}

void* MovieImportSetDuration(void)
{
    if (verbose) puts("STUB: MovieImportSetDuration called");
    return NULL;
}

void* MovieImportSetFromScrap(void)
{
    if (verbose) puts("STUB: MovieImportSetFromScrap called");
    return NULL;
}

void* MovieImportSetIdleManager(void)
{
    if (verbose) puts("STUB: MovieImportSetIdleManager called");
    return NULL;
}

void* MovieImportSetMediaDataRef(void)
{
    if (verbose) puts("STUB: MovieImportSetMediaDataRef called");
    return NULL;
}

void* MovieImportSetMediaFile(void)
{
    if (verbose) puts("STUB: MovieImportSetMediaFile called");
    return NULL;
}

void* MovieImportSetNewMovieFlags(void)
{
    if (verbose) puts("STUB: MovieImportSetNewMovieFlags called");
    return NULL;
}

void* MovieImportSetOffsetAndLimit(void)
{
    if (verbose) puts("STUB: MovieImportSetOffsetAndLimit called");
    return NULL;
}

void* MovieImportSetOffsetAndLimit64(void)
{
    if (verbose) puts("STUB: MovieImportSetOffsetAndLimit64 called");
    return NULL;
}

void* MovieImportSetProgressProc(void)
{
    if (verbose) puts("STUB: MovieImportSetProgressProc called");
    return NULL;
}

void* MovieImportSetSampleDescription(void)
{
    if (verbose) puts("STUB: MovieImportSetSampleDescription called");
    return NULL;
}

void* MovieImportSetSampleDuration(void)
{
    if (verbose) puts("STUB: MovieImportSetSampleDuration called");
    return NULL;
}

void* MovieImportSetSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: MovieImportSetSettingsFromAtomContainer called");
    return NULL;
}

void* MovieImportValidate(void)
{
    if (verbose) puts("STUB: MovieImportValidate called");
    return NULL;
}

void* MovieImportValidateDataRef(void)
{
    if (verbose) puts("STUB: MovieImportValidateDataRef called");
    return NULL;
}

void* MovieMediaGetChildDoMCActionCallback(void)
{
    if (verbose) puts("STUB: MovieMediaGetChildDoMCActionCallback called");
    return NULL;
}

void* MovieMediaGetChildMovieDataReference(void)
{
    if (verbose) puts("STUB: MovieMediaGetChildMovieDataReference called");
    return NULL;
}

void* MovieMediaGetCurrentMovieProperty(void)
{
    if (verbose) puts("STUB: MovieMediaGetCurrentMovieProperty called");
    return NULL;
}

void* MovieMediaGetCurrentTrackProperty(void)
{
    if (verbose) puts("STUB: MovieMediaGetCurrentTrackProperty called");
    return NULL;
}

void* MovieMediaGetDoMCActionCallback(void)
{
    if (verbose) puts("STUB: MovieMediaGetDoMCActionCallback called");
    return NULL;
}

void* MovieMediaGetMovie(void)
{
    if (verbose) puts("STUB: MovieMediaGetMovie called");
    return NULL;
}

void* MovieMediaGetMovieController(void)
{
    if (verbose) puts("STUB: MovieMediaGetMovieController called");
    return NULL;
}

void* MovieMediaLoadChildMovieFromDataReference(void)
{
    if (verbose) puts("STUB: MovieMediaLoadChildMovieFromDataReference called");
    return NULL;
}

void* MovieMediaLoadChildMovieFromDataReferenceWithQTListParams(void)
{
    if (verbose) puts("STUB: MovieMediaLoadChildMovieFromDataReferenceWithQTListParams called");
    return NULL;
}

void* MovieMediaRecompose(void)
{
    if (verbose) puts("STUB: MovieMediaRecompose called");
    return NULL;
}

void* MovieMediaSetChildMovieDataReference(void)
{
    if (verbose) puts("STUB: MovieMediaSetChildMovieDataReference called");
    return NULL;
}

void* MovieSearchText(void)
{
    if (verbose) puts("STUB: MovieSearchText called");
    return NULL;
}

void* MoviesTask(void)
{
    if (verbose) puts("STUB: MoviesTask called");
    return NULL;
}

void* MusicDerivedCloseResFile(void)
{
    if (verbose) puts("STUB: MusicDerivedCloseResFile called");
    return NULL;
}

void* MusicDerivedMIDISend(void)
{
    if (verbose) puts("STUB: MusicDerivedMIDISend called");
    return NULL;
}

void* MusicDerivedOpenResFile(void)
{
    if (verbose) puts("STUB: MusicDerivedOpenResFile called");
    return NULL;
}

void* MusicDerivedSetInstrument(void)
{
    if (verbose) puts("STUB: MusicDerivedSetInstrument called");
    return NULL;
}

void* MusicDerivedSetKnob(void)
{
    if (verbose) puts("STUB: MusicDerivedSetKnob called");
    return NULL;
}

void* MusicDerivedSetMIDI(void)
{
    if (verbose) puts("STUB: MusicDerivedSetMIDI called");
    return NULL;
}

void* MusicDerivedSetPart(void)
{
    if (verbose) puts("STUB: MusicDerivedSetPart called");
    return NULL;
}

void* MusicDerivedSetPartInstrumentNumber(void)
{
    if (verbose) puts("STUB: MusicDerivedSetPartInstrumentNumber called");
    return NULL;
}

void* MusicDerivedStorePartInstrument(void)
{
    if (verbose) puts("STUB: MusicDerivedStorePartInstrument called");
    return NULL;
}

void* MusicDeviceGetCollectionID(void)
{
    if (verbose) puts("STUB: MusicDeviceGetCollectionID called");
    return NULL;
}

void* MusicDeviceGetDLSInstrumentID(void)
{
    if (verbose) puts("STUB: MusicDeviceGetDLSInstrumentID called");
    return NULL;
}

void* MusicDeviceSetCollectionID(void)
{
    if (verbose) puts("STUB: MusicDeviceSetCollectionID called");
    return NULL;
}

void* MusicExportEvent(void)
{
    if (verbose) puts("STUB: MusicExportEvent called");
    return NULL;
}

void* MusicExportHeader(void)
{
    if (verbose) puts("STUB: MusicExportHeader called");
    return NULL;
}

void* MusicExportInitialize(void)
{
    if (verbose) puts("STUB: MusicExportInitialize called");
    return NULL;
}

void* MusicExportTerminate(void)
{
    if (verbose) puts("STUB: MusicExportTerminate called");
    return NULL;
}

void* MusicFindTone(void)
{
    if (verbose) puts("STUB: MusicFindTone called");
    return NULL;
}

void* MusicGenericConfigure(void)
{
    if (verbose) puts("STUB: MusicGenericConfigure called");
    return NULL;
}

void* MusicGenericGetKnobList(void)
{
    if (verbose) puts("STUB: MusicGenericGetKnobList called");
    return NULL;
}

void* MusicGenericGetPart(void)
{
    if (verbose) puts("STUB: MusicGenericGetPart called");
    return NULL;
}

void* MusicGenericSetResourceNumbers(void)
{
    if (verbose) puts("STUB: MusicGenericSetResourceNumbers called");
    return NULL;
}

void* MusicGetDescription(void)
{
    if (verbose) puts("STUB: MusicGetDescription called");
    return NULL;
}

void* MusicGetDeviceConnection(void)
{
    if (verbose) puts("STUB: MusicGetDeviceConnection called");
    return NULL;
}

void* MusicGetDrumKnobDescription(void)
{
    if (verbose) puts("STUB: MusicGetDrumKnobDescription called");
    return NULL;
}

void* MusicGetDrumKnobDescriptionObsolete(void)
{
    if (verbose) puts("STUB: MusicGetDrumKnobDescriptionObsolete called");
    return NULL;
}

void* MusicGetDrumNames(void)
{
    if (verbose) puts("STUB: MusicGetDrumNames called");
    return NULL;
}

void* MusicGetInfoText(void)
{
    if (verbose) puts("STUB: MusicGetInfoText called");
    return NULL;
}

void* MusicGetInstrument(void)
{
    if (verbose) puts("STUB: MusicGetInstrument called");
    return NULL;
}

void* MusicGetInstrumentAboutInfo(void)
{
    if (verbose) puts("STUB: MusicGetInstrumentAboutInfo called");
    return NULL;
}

void* MusicGetInstrumentInfo(void)
{
    if (verbose) puts("STUB: MusicGetInstrumentInfo called");
    return NULL;
}

void* MusicGetInstrumentKnobDescription(void)
{
    if (verbose) puts("STUB: MusicGetInstrumentKnobDescription called");
    return NULL;
}

void* MusicGetInstrumentKnobDescriptionObsolete(void)
{
    if (verbose) puts("STUB: MusicGetInstrumentKnobDescriptionObsolete called");
    return NULL;
}

void* MusicGetInstrumentNames(void)
{
    if (verbose) puts("STUB: MusicGetInstrumentNames called");
    return NULL;
}

void* MusicGetKnob(void)
{
    if (verbose) puts("STUB: MusicGetKnob called");
    return NULL;
}

void* MusicGetKnobDescription(void)
{
    if (verbose) puts("STUB: MusicGetKnobDescription called");
    return NULL;
}

void* MusicGetKnobDescriptionObsolete(void)
{
    if (verbose) puts("STUB: MusicGetKnobDescriptionObsolete called");
    return NULL;
}

void* MusicGetKnobSettingStrings(void)
{
    if (verbose) puts("STUB: MusicGetKnobSettingStrings called");
    return NULL;
}

void* MusicGetMIDIPorts(void)
{
    if (verbose) puts("STUB: MusicGetMIDIPorts called");
    return NULL;
}

void* MusicGetMIDIProc(void)
{
    if (verbose) puts("STUB: MusicGetMIDIProc called");
    return NULL;
}

void* MusicGetMasterTune(void)
{
    if (verbose) puts("STUB: MusicGetMasterTune called");
    return NULL;
}

void* MusicGetPart(void)
{
    if (verbose) puts("STUB: MusicGetPart called");
    return NULL;
}

void* MusicGetPartAtomicInstrument(void)
{
    if (verbose) puts("STUB: MusicGetPartAtomicInstrument called");
    return NULL;
}

void* MusicGetPartController(void)
{
    if (verbose) puts("STUB: MusicGetPartController called");
    return NULL;
}

void* MusicGetPartInstrumentNumber(void)
{
    if (verbose) puts("STUB: MusicGetPartInstrumentNumber called");
    return NULL;
}

void* MusicGetPartKnob(void)
{
    if (verbose) puts("STUB: MusicGetPartKnob called");
    return NULL;
}

void* MusicGetPartName(void)
{
    if (verbose) puts("STUB: MusicGetPartName called");
    return NULL;
}

void* MusicGetProperty(void)
{
    if (verbose) puts("STUB: MusicGetProperty called");
    return NULL;
}

void* MusicGetTimingInfo(void)
{
    if (verbose) puts("STUB: MusicGetTimingInfo called");
    return NULL;
}

void* MusicMediaGetIndexedTunePlayer(void)
{
    if (verbose) puts("STUB: MusicMediaGetIndexedTunePlayer called");
    return NULL;
}

void* MusicMediaGetPartCount(void)
{
    if (verbose) puts("STUB: MusicMediaGetPartCount called");
    return NULL;
}

void* MusicMediaGetPartInstrument(void)
{
    if (verbose) puts("STUB: MusicMediaGetPartInstrument called");
    return NULL;
}

void* MusicMediaGetPartMix(void)
{
    if (verbose) puts("STUB: MusicMediaGetPartMix called");
    return NULL;
}

void* MusicMediaSetPartInstrument(void)
{
    if (verbose) puts("STUB: MusicMediaSetPartInstrument called");
    return NULL;
}

void* MusicMediaSetPartMix(void)
{
    if (verbose) puts("STUB: MusicMediaSetPartMix called");
    return NULL;
}

void* MusicPlayNote(void)
{
    if (verbose) puts("STUB: MusicPlayNote called");
    return NULL;
}

void* MusicReceiveMIDI(void)
{
    if (verbose) puts("STUB: MusicReceiveMIDI called");
    return NULL;
}

void* MusicResetPart(void)
{
    if (verbose) puts("STUB: MusicResetPart called");
    return NULL;
}

void* MusicSendMIDI(void)
{
    if (verbose) puts("STUB: MusicSendMIDI called");
    return NULL;
}

void* MusicSetInstrument(void)
{
    if (verbose) puts("STUB: MusicSetInstrument called");
    return NULL;
}

void* MusicSetKnob(void)
{
    if (verbose) puts("STUB: MusicSetKnob called");
    return NULL;
}

void* MusicSetMIDIProc(void)
{
    if (verbose) puts("STUB: MusicSetMIDIProc called");
    return NULL;
}

void* MusicSetMasterTune(void)
{
    if (verbose) puts("STUB: MusicSetMasterTune called");
    return NULL;
}

void* MusicSetOfflineTimeTo(void)
{
    if (verbose) puts("STUB: MusicSetOfflineTimeTo called");
    return NULL;
}

void* MusicSetPart(void)
{
    if (verbose) puts("STUB: MusicSetPart called");
    return NULL;
}

void* MusicSetPartAtomicInstrument(void)
{
    if (verbose) puts("STUB: MusicSetPartAtomicInstrument called");
    return NULL;
}

void* MusicSetPartController(void)
{
    if (verbose) puts("STUB: MusicSetPartController called");
    return NULL;
}

void* MusicSetPartInstrumentNumber(void)
{
    if (verbose) puts("STUB: MusicSetPartInstrumentNumber called");
    return NULL;
}

void* MusicSetPartInstrumentNumberInterruptSafe(void)
{
    if (verbose) puts("STUB: MusicSetPartInstrumentNumberInterruptSafe called");
    return NULL;
}

void* MusicSetPartKnob(void)
{
    if (verbose) puts("STUB: MusicSetPartKnob called");
    return NULL;
}

void* MusicSetPartName(void)
{
    if (verbose) puts("STUB: MusicSetPartName called");
    return NULL;
}

void* MusicSetPartSoundLocalization(void)
{
    if (verbose) puts("STUB: MusicSetPartSoundLocalization called");
    return NULL;
}

void* MusicSetProperty(void)
{
    if (verbose) puts("STUB: MusicSetProperty called");
    return NULL;
}

void* MusicStartOffline(void)
{
    if (verbose) puts("STUB: MusicStartOffline called");
    return NULL;
}

void* MusicStorePartInstrument(void)
{
    if (verbose) puts("STUB: MusicStorePartInstrument called");
    return NULL;
}

void* MusicTask(void)
{
    if (verbose) puts("STUB: MusicTask called");
    return NULL;
}

void* MusicUseDeviceConnection(void)
{
    if (verbose) puts("STUB: MusicUseDeviceConnection called");
    return NULL;
}

void* NACopyrightDialog(void)
{
    if (verbose) puts("STUB: NACopyrightDialog called");
    return NULL;
}

void* NADisengageNoteChannel(void)
{
    if (verbose) puts("STUB: NADisengageNoteChannel called");
    return NULL;
}

void* NADisposeNoteChannel(void)
{
    if (verbose) puts("STUB: NADisposeNoteChannel called");
    return NULL;
}

void* NAEngageNoteChannel(void)
{
    if (verbose) puts("STUB: NAEngageNoteChannel called");
    return NULL;
}

void* NAFindNoteChannelTone(void)
{
    if (verbose) puts("STUB: NAFindNoteChannelTone called");
    return NULL;
}

void* NAGetController(void)
{
    if (verbose) puts("STUB: NAGetController called");
    return NULL;
}

void* NAGetDefaultMIDIInput(void)
{
    if (verbose) puts("STUB: NAGetDefaultMIDIInput called");
    return NULL;
}

void* NAGetIndNoteChannel(void)
{
    if (verbose) puts("STUB: NAGetIndNoteChannel called");
    return NULL;
}

void* NAGetKnob(void)
{
    if (verbose) puts("STUB: NAGetKnob called");
    return NULL;
}

void* NAGetMIDIPorts(void)
{
    if (verbose) puts("STUB: NAGetMIDIPorts called");
    return NULL;
}

void* NAGetNoteChannelInfo(void)
{
    if (verbose) puts("STUB: NAGetNoteChannelInfo called");
    return NULL;
}

void* NAGetNoteChannelState(void)
{
    if (verbose) puts("STUB: NAGetNoteChannelState called");
    return NULL;
}

void* NAGetNoteRequest(void)
{
    if (verbose) puts("STUB: NAGetNoteRequest called");
    return NULL;
}

void* NAGetPreferredMusicDevice(void)
{
    if (verbose) puts("STUB: NAGetPreferredMusicDevice called");
    return NULL;
}

void* NAGetRegisteredMusicDevice(void)
{
    if (verbose) puts("STUB: NAGetRegisteredMusicDevice called");
    return NULL;
}

void* NAGetStatusBlock(void)
{
    if (verbose) puts("STUB: NAGetStatusBlock called");
    return NULL;
}

void* NAILFindInstrument(void)
{
    if (verbose) puts("STUB: NAILFindInstrument called");
    return NULL;
}

void* NAILGetInstrument(void)
{
    if (verbose) puts("STUB: NAILGetInstrument called");
    return NULL;
}

void* NAILGetInstrumentCounts(void)
{
    if (verbose) puts("STUB: NAILGetInstrumentCounts called");
    return NULL;
}

void* NAILGetInstrumentList(void)
{
    if (verbose) puts("STUB: NAILGetInstrumentList called");
    return NULL;
}

void* NALoseDefaultMIDIInput(void)
{
    if (verbose) puts("STUB: NALoseDefaultMIDIInput called");
    return NULL;
}

void* NANewNoteChannel(void)
{
    if (verbose) puts("STUB: NANewNoteChannel called");
    return NULL;
}

void* NANewNoteChannelFromAtomicInstrument(void)
{
    if (verbose) puts("STUB: NANewNoteChannelFromAtomicInstrument called");
    return NULL;
}

void* NAPickArrangement(void)
{
    if (verbose) puts("STUB: NAPickArrangement called");
    return NULL;
}

void* NAPickEditInstrument(void)
{
    if (verbose) puts("STUB: NAPickEditInstrument called");
    return NULL;
}

void* NAPickInstrument(void)
{
    if (verbose) puts("STUB: NAPickInstrument called");
    return NULL;
}

void* NAPlayNote(void)
{
    if (verbose) puts("STUB: NAPlayNote called");
    return NULL;
}

void* NAPlayNote2(void)
{
    if (verbose) puts("STUB: NAPlayNote2 called");
    return NULL;
}

void* NAPrerollNoteChannel(void)
{
    if (verbose) puts("STUB: NAPrerollNoteChannel called");
    return NULL;
}

void* NARegisterMusicDevice(void)
{
    if (verbose) puts("STUB: NARegisterMusicDevice called");
    return NULL;
}

void* NAResetNoteChannel(void)
{
    if (verbose) puts("STUB: NAResetNoteChannel called");
    return NULL;
}

void* NASaveMusicConfiguration(void)
{
    if (verbose) puts("STUB: NASaveMusicConfiguration called");
    return NULL;
}

void* NASendMIDI(void)
{
    if (verbose) puts("STUB: NASendMIDI called");
    return NULL;
}

void* NASetAtomicInstrument(void)
{
    if (verbose) puts("STUB: NASetAtomicInstrument called");
    return NULL;
}

void* NASetController(void)
{
    if (verbose) puts("STUB: NASetController called");
    return NULL;
}

void* NASetDefaultMIDIInput(void)
{
    if (verbose) puts("STUB: NASetDefaultMIDIInput called");
    return NULL;
}

void* NASetInstrumentNumber(void)
{
    if (verbose) puts("STUB: NASetInstrumentNumber called");
    return NULL;
}

void* NASetInstrumentNumberInterruptSafe(void)
{
    if (verbose) puts("STUB: NASetInstrumentNumberInterruptSafe called");
    return NULL;
}

void* NASetKnob(void)
{
    if (verbose) puts("STUB: NASetKnob called");
    return NULL;
}

void* NASetNoteChannelBalance(void)
{
    if (verbose) puts("STUB: NASetNoteChannelBalance called");
    return NULL;
}

void* NASetNoteChannelSoundLocalization(void)
{
    if (verbose) puts("STUB: NASetNoteChannelSoundLocalization called");
    return NULL;
}

void* NASetNoteChannelState(void)
{
    if (verbose) puts("STUB: NASetNoteChannelState called");
    return NULL;
}

void* NASetNoteChannelVolume(void)
{
    if (verbose) puts("STUB: NASetNoteChannelVolume called");
    return NULL;
}

void* NASetPreferredMusicDevice(void)
{
    if (verbose) puts("STUB: NASetPreferredMusicDevice called");
    return NULL;
}

void* NASetSofter(void)
{
    if (verbose) puts("STUB: NASetSofter called");
    return NULL;
}

void* NAStuffToneDescription(void)
{
    if (verbose) puts("STUB: NAStuffToneDescription called");
    return NULL;
}

void* NATask(void)
{
    if (verbose) puts("STUB: NATask called");
    return NULL;
}

void* NAUnregisterMusicDevice(void)
{
    if (verbose) puts("STUB: NAUnregisterMusicDevice called");
    return NULL;
}

void* NAUnrollNoteChannel(void)
{
    if (verbose) puts("STUB: NAUnrollNoteChannel called");
    return NULL;
}

void* NAUseDefaultMIDIInput(void)
{
    if (verbose) puts("STUB: NAUseDefaultMIDIInput called");
    return NULL;
}

void* NetworkStatusGetCurrent(void)
{
    if (verbose) puts("STUB: NetworkStatusGetCurrent called");
    return NULL;
}

void* NetworkStatusGetCurrentConnectionInfo(void)
{
    if (verbose) puts("STUB: NetworkStatusGetCurrentConnectionInfo called");
    return NULL;
}

void* NewActionsUPP(void)
{
    if (verbose) puts("STUB: NewActionsUPP called");
    return NULL;
}

void* NewBlitterUPP(void)
{
    if (verbose) puts("STUB: NewBlitterUPP called");
    return NULL;
}

void* NewCDataHandlerUPP(void)
{
    if (verbose) puts("STUB: NewCDataHandlerUPP called");
    return NULL;
}

void* NewCGNotificationUPP(void)
{
    if (verbose) puts("STUB: NewCGNotificationUPP called");
    return NULL;
}

void* NewCallBack(void)
{
    if (verbose) puts("STUB: NewCallBack called");
    return NULL;
}

void* NewCancelledDataUPP(void)
{
    if (verbose) puts("STUB: NewCancelledDataUPP called");
    return NULL;
}

void* NewCharDataHandlerUPP(void)
{
    if (verbose) puts("STUB: NewCharDataHandlerUPP called");
    return NULL;
}

void* NewCommentHandlerUPP(void)
{
    if (verbose) puts("STUB: NewCommentHandlerUPP called");
    return NULL;
}

void* NewCursorNotify(void)
{
    if (verbose) puts("STUB: NewCursorNotify called");
    return NULL;
}

void* NewDataHCompletionUPP(void)
{
    if (verbose) puts("STUB: NewDataHCompletionUPP called");
    return NULL;
}

void* NewDoMCActionUPP(void)
{
    if (verbose) puts("STUB: NewDoMCActionUPP called");
    return NULL;
}

void* NewEndDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: NewEndDocumentHandlerUPP called");
    return NULL;
}

void* NewEndElementHandlerUPP(void)
{
    if (verbose) puts("STUB: NewEndElementHandlerUPP called");
    return NULL;
}

void* NewEraseUPP(void)
{
    if (verbose) puts("STUB: NewEraseUPP called");
    return NULL;
}

void* NewExitToShellUPP(void)
{
    if (verbose) puts("STUB: NewExitToShellUPP called");
    return NULL;
}

void* NewForEachMediaUPP(void)
{
    if (verbose) puts("STUB: NewForEachMediaUPP called");
    return NULL;
}

void* NewForEachTrackUPP(void)
{
    if (verbose) puts("STUB: NewForEachTrackUPP called");
    return NULL;
}

void* NewGetAtomLeafUPP(void)
{
    if (verbose) puts("STUB: NewGetAtomLeafUPP called");
    return NULL;
}

void* NewGetMoreMovieDataUPP(void)
{
    if (verbose) puts("STUB: NewGetMoreMovieDataUPP called");
    return NULL;
}

void* NewGetMovieUPP(void)
{
    if (verbose) puts("STUB: NewGetMovieUPP called");
    return NULL;
}

void* NewHandleSystemSafe(void)
{
    if (verbose) puts("STUB: NewHandleSystemSafe called");
    return NULL;
}

void* NewICMAlignmentUPP(void)
{
    if (verbose) puts("STUB: NewICMAlignmentUPP called");
    return NULL;
}

void* NewICMCompletionUPP(void)
{
    if (verbose) puts("STUB: NewICMCompletionUPP called");
    return NULL;
}

void* NewICMConvertDataFormatUPP(void)
{
    if (verbose) puts("STUB: NewICMConvertDataFormatUPP called");
    return NULL;
}

void* NewICMCursorShieldedUPP(void)
{
    if (verbose) puts("STUB: NewICMCursorShieldedUPP called");
    return NULL;
}

void* NewICMDataUPP(void)
{
    if (verbose) puts("STUB: NewICMDataUPP called");
    return NULL;
}

void* NewICMFlushUPP(void)
{
    if (verbose) puts("STUB: NewICMFlushUPP called");
    return NULL;
}

void* NewICMMemoryDisposedUPP(void)
{
    if (verbose) puts("STUB: NewICMMemoryDisposedUPP called");
    return NULL;
}

void* NewICMProgressUPP(void)
{
    if (verbose) puts("STUB: NewICMProgressUPP called");
    return NULL;
}

void* NewImageCodecDrawBandCompleteUPP(void)
{
    if (verbose) puts("STUB: NewImageCodecDrawBandCompleteUPP called");
    return NULL;
}

void* NewImageCodecMPDrawBandUPP(void)
{
    if (verbose) puts("STUB: NewImageCodecMPDrawBandUPP called");
    return NULL;
}

void* NewImageCodecTimeTriggerUPP(void)
{
    if (verbose) puts("STUB: NewImageCodecTimeTriggerUPP called");
    return NULL;
}

void* NewImageGWorld(void)
{
    if (verbose) puts("STUB: NewImageGWorld called");
    return NULL;
}

void* NewMCActionFilterUPP(void)
{
    if (verbose) puts("STUB: NewMCActionFilterUPP called");
    return NULL;
}

void* NewMCActionFilterWithRefConUPP(void)
{
    if (verbose) puts("STUB: NewMCActionFilterWithRefConUPP called");
    return NULL;
}

void* NewMCActionNotificationUPP(void)
{
    if (verbose) puts("STUB: NewMCActionNotificationUPP called");
    return NULL;
}

void* NewMovie(void)
{
    if (verbose) puts("STUB: NewMovie called");
    return NULL;
}

void* NewMovieController(void)
{
    if (verbose) puts("STUB: NewMovieController called");
    return NULL;
}

void* NewMovieDrawingCompleteUPP(void)
{
    if (verbose) puts("STUB: NewMovieDrawingCompleteUPP called");
    return NULL;
}

void* NewMovieEditState(void)
{
    if (verbose) puts("STUB: NewMovieEditState called");
    return NULL;
}

void* NewMovieExecuteWiredActionsUPP(void)
{
    if (verbose) puts("STUB: NewMovieExecuteWiredActionsUPP called");
    return NULL;
}

void* NewMovieExportGetDataUPP(void)
{
    if (verbose) puts("STUB: NewMovieExportGetDataUPP called");
    return NULL;
}

void* NewMovieExportGetPropertyUPP(void)
{
    if (verbose) puts("STUB: NewMovieExportGetPropertyUPP called");
    return NULL;
}

void* NewMovieExportStageReachedCallbackUPP(void)
{
    if (verbose) puts("STUB: NewMovieExportStageReachedCallbackUPP called");
    return NULL;
}

void* NewMovieForDataRefFromHandle(void)
{
    if (verbose) puts("STUB: NewMovieForDataRefFromHandle called");
    return NULL;
}

void* NewMovieFromDataFork(void)
{
    if (verbose) puts("STUB: NewMovieFromDataFork called");
    return NULL;
}

void* NewMovieFromDataFork64(void)
{
    if (verbose) puts("STUB: NewMovieFromDataFork64 called");
    return NULL;
}

void* NewMovieFromDataRef(void)
{
    if (verbose) puts("STUB: NewMovieFromDataRef called");
    return NULL;
}

void* NewMovieFromFile(void)
{
    if (verbose) puts("STUB: NewMovieFromFile called");
    return NULL;
}

void* NewMovieFromHandle(void)
{
    if (verbose) puts("STUB: NewMovieFromHandle called");
    return NULL;
}

void* NewMovieFromProperties(void)
{
    if (verbose) puts("STUB: NewMovieFromProperties called");
    return NULL;
}

void* NewMovieFromScrap(void)
{
    if (verbose) puts("STUB: NewMovieFromScrap called");
    return NULL;
}

void* NewMovieFromStorage(void)
{
    if (verbose) puts("STUB: NewMovieFromStorage called");
    return NULL;
}

void* NewMovieFromStorageOffset(void)
{
    if (verbose) puts("STUB: NewMovieFromStorageOffset called");
    return NULL;
}

void* NewMovieFromUserProc(void)
{
    if (verbose) puts("STUB: NewMovieFromUserProc called");
    return NULL;
}

void* NewMoviePrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: NewMoviePrePrerollCompleteUPP called");
    return NULL;
}

void* NewMoviePreviewCallOutUPP(void)
{
    if (verbose) puts("STUB: NewMoviePreviewCallOutUPP called");
    return NULL;
}

void* NewMovieProgressUPP(void)
{
    if (verbose) puts("STUB: NewMovieProgressUPP called");
    return NULL;
}

void* NewMovieRgnCoverUPP(void)
{
    if (verbose) puts("STUB: NewMovieRgnCoverUPP called");
    return NULL;
}

void* NewMovieTrack(void)
{
    if (verbose) puts("STUB: NewMovieTrack called");
    return NULL;
}

void* NewMoviesErrorUPP(void)
{
    if (verbose) puts("STUB: NewMoviesErrorUPP called");
    return NULL;
}

void* NewMusicMIDIReadHookUPP(void)
{
    if (verbose) puts("STUB: NewMusicMIDIReadHookUPP called");
    return NULL;
}

void* NewMusicMIDISendUPP(void)
{
    if (verbose) puts("STUB: NewMusicMIDISendUPP called");
    return NULL;
}

void* NewMusicOfflineDataUPP(void)
{
    if (verbose) puts("STUB: NewMusicOfflineDataUPP called");
    return NULL;
}

void* NewPBOpenUPP(void)
{
    if (verbose) puts("STUB: NewPBOpenUPP called");
    return NULL;
}

void* NewPrePrerollCompleteUPP(void)
{
    if (verbose) puts("STUB: NewPrePrerollCompleteUPP called");
    return NULL;
}

void* NewPreprocessInstructionHandlerUPP(void)
{
    if (verbose) puts("STUB: NewPreprocessInstructionHandlerUPP called");
    return NULL;
}

void* NewPutNeedRoomUPP(void)
{
    if (verbose) puts("STUB: NewPutNeedRoomUPP called");
    return NULL;
}

void* NewPutPatchLongUPP(void)
{
    if (verbose) puts("STUB: NewPutPatchLongUPP called");
    return NULL;
}

void* NewQDPixUPP(void)
{
    if (verbose) puts("STUB: NewQDPixUPP called");
    return NULL;
}

void* NewQTAllDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: NewQTAllDataDoneCompleteUPP called");
    return NULL;
}

void* NewQTAllocatorAllocateUPP(void)
{
    if (verbose) puts("STUB: NewQTAllocatorAllocateUPP called");
    return NULL;
}

void* NewQTAllocatorDisposeAllocatorUPP(void)
{
    if (verbose) puts("STUB: NewQTAllocatorDisposeAllocatorUPP called");
    return NULL;
}

void* NewQTAllocatorFreeUPP(void)
{
    if (verbose) puts("STUB: NewQTAllocatorFreeUPP called");
    return NULL;
}

void* NewQTAllocatorReallocateUPP(void)
{
    if (verbose) puts("STUB: NewQTAllocatorReallocateUPP called");
    return NULL;
}

void* NewQTBandwidthNotificationUPP(void)
{
    if (verbose) puts("STUB: NewQTBandwidthNotificationUPP called");
    return NULL;
}

void* NewQTCallBackUPP(void)
{
    if (verbose) puts("STUB: NewQTCallBackUPP called");
    return NULL;
}

void* NewQTComponentPropertyListenerFilterUPP(void)
{
    if (verbose) puts("STUB: NewQTComponentPropertyListenerFilterUPP called");
    return NULL;
}

void* NewQTComponentPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: NewQTComponentPropertyListenerUPP called");
    return NULL;
}

void* NewQTDRMPrivAPICallBackUPP(void)
{
    if (verbose) puts("STUB: NewQTDRMPrivAPICallBackUPP called");
    return NULL;
}

void* NewQTDataDoneCompleteUPP(void)
{
    if (verbose) puts("STUB: NewQTDataDoneCompleteUPP called");
    return NULL;
}

void* NewQTDataStashValueDisposeUPP(void)
{
    if (verbose) puts("STUB: NewQTDataStashValueDisposeUPP called");
    return NULL;
}

void* NewQTEffectListFilterUPP(void)
{
    if (verbose) puts("STUB: NewQTEffectListFilterUPP called");
    return NULL;
}

void* NewQTFlattenCallBackUPP(void)
{
    if (verbose) puts("STUB: NewQTFlattenCallBackUPP called");
    return NULL;
}

void* NewQTGenerateHashValueUPP(void)
{
    if (verbose) puts("STUB: NewQTGenerateHashValueUPP called");
    return NULL;
}

void* NewQTHashKeysAreEqualUPP(void)
{
    if (verbose) puts("STUB: NewQTHashKeysAreEqualUPP called");
    return NULL;
}

void* NewQTMemPoolCallBackUPP(void)
{
    if (verbose) puts("STUB: NewQTMemPoolCallBackUPP called");
    return NULL;
}

void* NewQTMessageDispatchUPP(void)
{
    if (verbose) puts("STUB: NewQTMessageDispatchUPP called");
    return NULL;
}

void* NewQTMissingSoftwareCallbackUPP(void)
{
    if (verbose) puts("STUB: NewQTMissingSoftwareCallbackUPP called");
    return NULL;
}

void* NewQTMoviePropertyListenerUPP(void)
{
    if (verbose) puts("STUB: NewQTMoviePropertyListenerUPP called");
    return NULL;
}

void* NewQTMovieStatusPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: NewQTMovieStatusPropertyListenerUPP called");
    return NULL;
}

void* NewQTNextTaskNeededSoonerCallbackUPP(void)
{
    if (verbose) puts("STUB: NewQTNextTaskNeededSoonerCallbackUPP called");
    return NULL;
}

void* NewQTProcessPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: NewQTProcessPropertyListenerUPP called");
    return NULL;
}

void* NewQTReleaseKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: NewQTReleaseKeyAndValueUPP called");
    return NULL;
}

void* NewQTRestrictions(void)
{
    if (verbose) puts("STUB: NewQTRestrictions called");
    return NULL;
}

void* NewQTRetainKeyAndValueUPP(void)
{
    if (verbose) puts("STUB: NewQTRetainKeyAndValueUPP called");
    return NULL;
}

void* NewQTSDataDispatchFilterUPP(void)
{
    if (verbose) puts("STUB: NewQTSDataDispatchFilterUPP called");
    return NULL;
}

void* NewQTSDialogSettingsUPP(void)
{
    if (verbose) puts("STUB: NewQTSDialogSettingsUPP called");
    return NULL;
}

void* NewQTSModalFilterUPP(void)
{
    if (verbose) puts("STUB: NewQTSModalFilterUPP called");
    return NULL;
}

void* NewQTSNotificationUPP(void)
{
    if (verbose) puts("STUB: NewQTSNotificationUPP called");
    return NULL;
}

void* NewQTSPanelFilterUPP(void)
{
    if (verbose) puts("STUB: NewQTSPanelFilterUPP called");
    return NULL;
}

void* NewQTSSourcerDataReleaseUPP(void)
{
    if (verbose) puts("STUB: NewQTSSourcerDataReleaseUPP called");
    return NULL;
}

void* NewQTSStreamModuleUPP(void)
{
    if (verbose) puts("STUB: NewQTSStreamModuleUPP called");
    return NULL;
}

void* NewQTSTrackHinterProgressUPP(void)
{
    if (verbose) puts("STUB: NewQTSTrackHinterProgressUPP called");
    return NULL;
}

void* NewQTSyncTaskUPP(void)
{
    if (verbose) puts("STUB: NewQTSyncTaskUPP called");
    return NULL;
}

void* NewQTTrackPropertyListenerUPP(void)
{
    if (verbose) puts("STUB: NewQTTrackPropertyListenerUPP called");
    return NULL;
}

void* NewQTUpdateMovieProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: NewQTUpdateMovieProfileCallbackUPP called");
    return NULL;
}

void* NewQTUpdateTrackProfileCallbackUPP(void)
{
    if (verbose) puts("STUB: NewQTUpdateTrackProfileCallbackUPP called");
    return NULL;
}

void* NewQTVRBackBufferImagingUPP(void)
{
    if (verbose) puts("STUB: NewQTVRBackBufferImagingUPP called");
    return NULL;
}

void* NewQTVREnteringNodeUPP(void)
{
    if (verbose) puts("STUB: NewQTVREnteringNodeUPP called");
    return NULL;
}

void* NewQTVRImagingCompleteUPP(void)
{
    if (verbose) puts("STUB: NewQTVRImagingCompleteUPP called");
    return NULL;
}

void* NewQTVRInterceptUPP(void)
{
    if (verbose) puts("STUB: NewQTVRInterceptUPP called");
    return NULL;
}

void* NewQTVRLeavingNodeUPP(void)
{
    if (verbose) puts("STUB: NewQTVRLeavingNodeUPP called");
    return NULL;
}

void* NewQTVRMouseOverHotSpotUPP(void)
{
    if (verbose) puts("STUB: NewQTVRMouseOverHotSpotUPP called");
    return NULL;
}

void* NewQTWorkContextDataReleaseUPP(void)
{
    if (verbose) puts("STUB: NewQTWorkContextDataReleaseUPP called");
    return NULL;
}

void* NewRTPMPDataReleaseUPP(void)
{
    if (verbose) puts("STUB: NewRTPMPDataReleaseUPP called");
    return NULL;
}

void* NewRTPPBCallbackUPP(void)
{
    if (verbose) puts("STUB: NewRTPPBCallbackUPP called");
    return NULL;
}

void* NewSCModalFilterUPP(void)
{
    if (verbose) puts("STUB: NewSCModalFilterUPP called");
    return NULL;
}

void* NewSCModalHookUPP(void)
{
    if (verbose) puts("STUB: NewSCModalHookUPP called");
    return NULL;
}

void* NewSFModalFilterUPP(void)
{
    if (verbose) puts("STUB: NewSFModalFilterUPP called");
    return NULL;
}

void* NewSGAddFrameBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGAddFrameBottleUPP called");
    return NULL;
}

void* NewSGCompressBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGCompressBottleUPP called");
    return NULL;
}

void* NewSGCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGCompressCompleteBottleUPP called");
    return NULL;
}

void* NewSGDataUPP(void)
{
    if (verbose) puts("STUB: NewSGDataUPP called");
    return NULL;
}

void* NewSGDisplayBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGDisplayBottleUPP called");
    return NULL;
}

void* NewSGDisplayCompressBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGDisplayCompressBottleUPP called");
    return NULL;
}

void* NewSGGrabBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGGrabBottleUPP called");
    return NULL;
}

void* NewSGGrabCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGGrabCompleteBottleUPP called");
    return NULL;
}

void* NewSGGrabCompressCompleteBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGGrabCompressCompleteBottleUPP called");
    return NULL;
}

void* NewSGModalFilterUPP(void)
{
    if (verbose) puts("STUB: NewSGModalFilterUPP called");
    return NULL;
}

void* NewSGTransferFrameBottleUPP(void)
{
    if (verbose) puts("STUB: NewSGTransferFrameBottleUPP called");
    return NULL;
}

void* NewSHChunkDataReleaseUPP(void)
{
    if (verbose) puts("STUB: NewSHChunkDataReleaseUPP called");
    return NULL;
}

void* NewSHNotificationUPP(void)
{
    if (verbose) puts("STUB: NewSHNotificationUPP called");
    return NULL;
}

void* NewSHSendDataUPP(void)
{
    if (verbose) puts("STUB: NewSHSendDataUPP called");
    return NULL;
}

void* NewSequenceFilterDataUPP(void)
{
    if (verbose) puts("STUB: NewSequenceFilterDataUPP called");
    return NULL;
}

void* NewSprite(void)
{
    if (verbose) puts("STUB: NewSprite called");
    return NULL;
}

void* NewSpriteWorld(void)
{
    if (verbose) puts("STUB: NewSpriteWorld called");
    return NULL;
}

void* NewStartDocumentHandlerUPP(void)
{
    if (verbose) puts("STUB: NewStartDocumentHandlerUPP called");
    return NULL;
}

void* NewStartElementHandlerUPP(void)
{
    if (verbose) puts("STUB: NewStartElementHandlerUPP called");
    return NULL;
}

void* NewStdPixUPP(void)
{
    if (verbose) puts("STUB: NewStdPixUPP called");
    return NULL;
}

void* NewTextMediaUPP(void)
{
    if (verbose) puts("STUB: NewTextMediaUPP called");
    return NULL;
}

void* NewTimeBase(void)
{
    if (verbose) puts("STUB: NewTimeBase called");
    return NULL;
}

void* NewTrackEditState(void)
{
    if (verbose) puts("STUB: NewTrackEditState called");
    return NULL;
}

void* NewTrackMedia(void)
{
    if (verbose) puts("STUB: NewTrackMedia called");
    return NULL;
}

void* NewTrackTransferUPP(void)
{
    if (verbose) puts("STUB: NewTrackTransferUPP called");
    return NULL;
}

void* NewTuneCallBackUPP(void)
{
    if (verbose) puts("STUB: NewTuneCallBackUPP called");
    return NULL;
}

void* NewTunePlayCallBackUPP(void)
{
    if (verbose) puts("STUB: NewTunePlayCallBackUPP called");
    return NULL;
}

void* NewTweenerDataUPP(void)
{
    if (verbose) puts("STUB: NewTweenerDataUPP called");
    return NULL;
}

void* NewTypedHandleClear(void)
{
    if (verbose) puts("STUB: NewTypedHandleClear called");
    return NULL;
}

void* NewUpdaterAppHandleMenuEventUPP(void)
{
    if (verbose) puts("STUB: NewUpdaterAppHandleMenuEventUPP called");
    return NULL;
}

void* NewUseThisTrackForTimeTableUPP(void)
{
    if (verbose) puts("STUB: NewUseThisTrackForTimeTableUPP called");
    return NULL;
}

void* NewUserData(void)
{
    if (verbose) puts("STUB: NewUserData called");
    return NULL;
}

void* NewUserDataFromHandle(void)
{
    if (verbose) puts("STUB: NewUserDataFromHandle called");
    return NULL;
}

void* NewVdigIntUPP(void)
{
    if (verbose) puts("STUB: NewVdigIntUPP called");
    return NULL;
}

void* NewYUVPtrsUPP(void)
{
    if (verbose) puts("STUB: NewYUVPtrsUPP called");
    return NULL;
}

void* OpenADataHandler(void)
{
    if (verbose) puts("STUB: OpenADataHandler called");
    return NULL;
}

void* OpenADataHandler2(void)
{
    if (verbose) puts("STUB: OpenADataHandler2 called");
    return NULL;
}

void* OpenMovieFile(void)
{
    if (verbose) puts("STUB: OpenMovieFile called");
    return NULL;
}

void* OpenMovieFileUnicode(void)
{
    if (verbose) puts("STUB: OpenMovieFileUnicode called");
    return NULL;
}

void* OpenMovieStorage(void)
{
    if (verbose) puts("STUB: OpenMovieStorage called");
    return NULL;
}

void* OpenPreviewComponentForFile(void)
{
    if (verbose) puts("STUB: OpenPreviewComponentForFile called");
    return NULL;
}

void* OptionDown(void)
{
    if (verbose) puts("STUB: OptionDown called");
    return NULL;
}

void* PasteHandleIntoMovie(void)
{
    if (verbose) puts("STUB: PasteHandleIntoMovie called");
    return NULL;
}

void* PasteMovieSelection(void)
{
    if (verbose) puts("STUB: PasteMovieSelection called");
    return NULL;
}

void* PatchAtomSize(void)
{
    if (verbose) puts("STUB: PatchAtomSize called");
    return NULL;
}

void* PlayMoviePreview(void)
{
    if (verbose) puts("STUB: PlayMoviePreview called");
    return NULL;
}

void* PluginHelperCanDo(void)
{
    if (verbose) puts("STUB: PluginHelperCanDo called");
    return NULL;
}

void* PluginHelperClose(void)
{
    if (verbose) puts("STUB: PluginHelperClose called");
    return NULL;
}

void* PluginHelperCreateMIMETypesPreferences(void)
{
    if (verbose) puts("STUB: PluginHelperCreateMIMETypesPreferences called");
    return NULL;
}

void* PluginHelperFindPotentialImporters(void)
{
    if (verbose) puts("STUB: PluginHelperFindPotentialImporters called");
    return NULL;
}

void* PluginHelperFindPotentialImportersForMIMEType(void)
{
    if (verbose) puts("STUB: PluginHelperFindPotentialImportersForMIMEType called");
    return NULL;
}

void* PluginHelperFindPotentialImportersForQTMediaType(void)
{
    if (verbose) puts("STUB: PluginHelperFindPotentialImportersForQTMediaType called");
    return NULL;
}

void* PluginHelperGetInfo(void)
{
    if (verbose) puts("STUB: PluginHelperGetInfo called");
    return NULL;
}

void* PluginHelperGetQTMIMEMappingPrefs(void)
{
    if (verbose) puts("STUB: PluginHelperGetQTMIMEMappingPrefs called");
    return NULL;
}

void* PluginHelperGetQTMIMEMappingStrings(void)
{
    if (verbose) puts("STUB: PluginHelperGetQTMIMEMappingStrings called");
    return NULL;
}

void* PluginHelperInstallFileTypesAssociation(void)
{
    if (verbose) puts("STUB: PluginHelperInstallFileTypesAssociation called");
    return NULL;
}

void* PluginHelperInstallPlugins(void)
{
    if (verbose) puts("STUB: PluginHelperInstallPlugins called");
    return NULL;
}

void* PluginHelperOpen(void)
{
    if (verbose) puts("STUB: PluginHelperOpen called");
    return NULL;
}

void* PluginHelperRegister(void)
{
    if (verbose) puts("STUB: PluginHelperRegister called");
    return NULL;
}

void* PluginHelperRegisterAllWhackers(void)
{
    if (verbose) puts("STUB: PluginHelperRegisterAllWhackers called");
    return NULL;
}

void* PluginHelperSetInfo(void)
{
    if (verbose) puts("STUB: PluginHelperSetInfo called");
    return NULL;
}

void* PluginHelperSetQTMIMEMappingPrefs(void)
{
    if (verbose) puts("STUB: PluginHelperSetQTMIMEMappingPrefs called");
    return NULL;
}

void* PluginHelperTarget(void)
{
    if (verbose) puts("STUB: PluginHelperTarget called");
    return NULL;
}

void* PluginHelperVersion(void)
{
    if (verbose) puts("STUB: PluginHelperVersion called");
    return NULL;
}

void* PrePrerollMovie(void)
{
    if (verbose) puts("STUB: PrePrerollMovie called");
    return NULL;
}

void* PreflightMovie(void)
{
    if (verbose) puts("STUB: PreflightMovie called");
    return NULL;
}

void* PrerollMovie(void)
{
    if (verbose) puts("STUB: PrerollMovie called");
    return NULL;
}

void* PreviewEvent(void)
{
    if (verbose) puts("STUB: PreviewEvent called");
    return NULL;
}

void* PreviewMakePreview(void)
{
    if (verbose) puts("STUB: PreviewMakePreview called");
    return NULL;
}

void* PreviewMakePreviewFromDataRef(void)
{
    if (verbose) puts("STUB: PreviewMakePreviewFromDataRef called");
    return NULL;
}

void* PreviewMakePreviewReference(void)
{
    if (verbose) puts("STUB: PreviewMakePreviewReference called");
    return NULL;
}

void* PreviewMakePreviewReferenceFromDataRef(void)
{
    if (verbose) puts("STUB: PreviewMakePreviewReferenceFromDataRef called");
    return NULL;
}

void* PreviewShowData(void)
{
    if (verbose) puts("STUB: PreviewShowData called");
    return NULL;
}

void* PrivateAllocateTrackAudioSubContext(void)
{
    if (verbose) puts("STUB: PrivateAllocateTrackAudioSubContext called");
    return NULL;
}

void* PrivateCreateMovieAlias(void)
{
    if (verbose) puts("STUB: PrivateCreateMovieAlias called");
    return NULL;
}

void* PrivateCreateMovieAlias_priv(void)
{
    if (verbose) puts("STUB: PrivateCreateMovieAlias_priv called");
    return NULL;
}

void* PrivateGetMaxLoadedTimeInMovie(void)
{
    if (verbose) puts("STUB: PrivateGetMaxLoadedTimeInMovie called");
    return NULL;
}

void* PrivateGetMovieLoadState(void)
{
    if (verbose) puts("STUB: PrivateGetMovieLoadState called");
    return NULL;
}

void* PrivateGetUserDataItem(void)
{
    if (verbose) puts("STUB: PrivateGetUserDataItem called");
    return NULL;
}

void* PrivateQTGetMoviePlayabilityState(void)
{
    if (verbose) puts("STUB: PrivateQTGetMoviePlayabilityState called");
    return NULL;
}

void* PrivateReleaseTrackAudioSubContext(void)
{
    if (verbose) puts("STUB: PrivateReleaseTrackAudioSubContext called");
    return NULL;
}

void* PrivateSetMovieAudioContext(void)
{
    if (verbose) puts("STUB: PrivateSetMovieAudioContext called");
    return NULL;
}

void* PrivateUpdateMovieAudioSummaryInfo(void)
{
    if (verbose) puts("STUB: PrivateUpdateMovieAudioSummaryInfo called");
    return NULL;
}

void* ProcessOneQuickTimeErrorDialogEvent(void)
{
    if (verbose) puts("STUB: ProcessOneQuickTimeErrorDialogEvent called");
    return NULL;
}

void* ProgressCloseMessage(void)
{
    if (verbose) puts("STUB: ProgressCloseMessage called");
    return NULL;
}

void* ProgressGetICMProc(void)
{
    if (verbose) puts("STUB: ProgressGetICMProc called");
    return NULL;
}

void* ProgressGetMovieProc(void)
{
    if (verbose) puts("STUB: ProgressGetMovieProc called");
    return NULL;
}

void* ProgressOpenMessage(void)
{
    if (verbose) puts("STUB: ProgressOpenMessage called");
    return NULL;
}

void* ProgressSetCenterDevice(void)
{
    if (verbose) puts("STUB: ProgressSetCenterDevice called");
    return NULL;
}

void* ProgressSetCenterPort(void)
{
    if (verbose) puts("STUB: ProgressSetCenterPort called");
    return NULL;
}

void* ProgressSetPrompt(void)
{
    if (verbose) puts("STUB: ProgressSetPrompt called");
    return NULL;
}

void* ProgressUpdateMessage(void)
{
    if (verbose) puts("STUB: ProgressUpdateMessage called");
    return NULL;
}

void* PtInDSequenceData(void)
{
    if (verbose) puts("STUB: PtInDSequenceData called");
    return NULL;
}

void* PtInMovie(void)
{
    if (verbose) puts("STUB: PtInMovie called");
    return NULL;
}

void* PtInTrack(void)
{
    if (verbose) puts("STUB: PtInTrack called");
    return NULL;
}

void* PutAtomSizeID(void)
{
    if (verbose) puts("STUB: PutAtomSizeID called");
    return NULL;
}

void* PutHandleOfSize(void)
{
    if (verbose) puts("STUB: PutHandleOfSize called");
    return NULL;
}

void* PutHandlerAtom(void)
{
    if (verbose) puts("STUB: PutHandlerAtom called");
    return NULL;
}

void* PutMovieForDataRefIntoHandle(void)
{
    if (verbose) puts("STUB: PutMovieForDataRefIntoHandle called");
    return NULL;
}

void* PutMovieForDataRefIntoHandleWithJournal(void)
{
    if (verbose) puts("STUB: PutMovieForDataRefIntoHandleWithJournal called");
    return NULL;
}

void* PutMovieIntoDataFork(void)
{
    if (verbose) puts("STUB: PutMovieIntoDataFork called");
    return NULL;
}

void* PutMovieIntoDataFork64(void)
{
    if (verbose) puts("STUB: PutMovieIntoDataFork64 called");
    return NULL;
}

void* PutMovieIntoHandle(void)
{
    if (verbose) puts("STUB: PutMovieIntoHandle called");
    return NULL;
}

void* PutMovieIntoStorage(void)
{
    if (verbose) puts("STUB: PutMovieIntoStorage called");
    return NULL;
}

void* PutMovieIntoTypedHandle(void)
{
    if (verbose) puts("STUB: PutMovieIntoTypedHandle called");
    return NULL;
}

void* PutMovieOnScrap(void)
{
    if (verbose) puts("STUB: PutMovieOnScrap called");
    return NULL;
}

void* PutSizedHandle(void)
{
    if (verbose) puts("STUB: PutSizedHandle called");
    return NULL;
}

void* PutTrackReferenceAtom(void)
{
    if (verbose) puts("STUB: PutTrackReferenceAtom called");
    return NULL;
}

void* PutUserData(void)
{
    if (verbose) puts("STUB: PutUserData called");
    return NULL;
}

void* PutUserDataAtom(void)
{
    if (verbose) puts("STUB: PutUserDataAtom called");
    return NULL;
}

void* PutUserDataIntoHandle(void)
{
    if (verbose) puts("STUB: PutUserDataIntoHandle called");
    return NULL;
}

void* QDImageQueueVUActiveStateChanged(void)
{
    if (verbose) puts("STUB: QDImageQueueVUActiveStateChanged called");
    return NULL;
}

void* QDImageQueueVUCopyAndEnqueue(void)
{
    if (verbose) puts("STUB: QDImageQueueVUCopyAndEnqueue called");
    return NULL;
}

void* QDImageQueueVUEraseOffscreen(void)
{
    if (verbose) puts("STUB: QDImageQueueVUEraseOffscreen called");
    return NULL;
}

void* QDImageQueueVUFlush(void)
{
    if (verbose) puts("STUB: QDImageQueueVUFlush called");
    return NULL;
}

void* QDImageQueueVUGetGWorld(void)
{
    if (verbose) puts("STUB: QDImageQueueVUGetGWorld called");
    return NULL;
}

void* QDImageQueueVUInitializeOffscreen(void)
{
    if (verbose) puts("STUB: QDImageQueueVUInitializeOffscreen called");
    return NULL;
}

void* QDImageQueueVUReleasePreviousOffscreen(void)
{
    if (verbose) puts("STUB: QDImageQueueVUReleasePreviousOffscreen called");
    return NULL;
}

void* QTAddComponentPropertyListener(void)
{
    if (verbose) puts("STUB: QTAddComponentPropertyListener called");
    return NULL;
}

void* QTAddDeferredAction(void)
{
    if (verbose) puts("STUB: QTAddDeferredAction called");
    return NULL;
}

void* QTAddLeafToObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTAddLeafToObjectSpecifier called");
    return NULL;
}

void* QTAddMovieError(void)
{
    if (verbose) puts("STUB: QTAddMovieError called");
    return NULL;
}

void* QTAddMovieErrorWithFlags(void)
{
    if (verbose) puts("STUB: QTAddMovieErrorWithFlags called");
    return NULL;
}

void* QTAddMoviePropertyListener(void)
{
    if (verbose) puts("STUB: QTAddMoviePropertyListener called");
    return NULL;
}

void* QTAddNewMovieRestrictions(void)
{
    if (verbose) puts("STUB: QTAddNewMovieRestrictions called");
    return NULL;
}

void* QTAddProcessPropertyListener(void)
{
    if (verbose) puts("STUB: QTAddProcessPropertyListener called");
    return NULL;
}

void* QTAddStashedData(void)
{
    if (verbose) puts("STUB: QTAddStashedData called");
    return NULL;
}

void* QTAddTrackPropertyListener(void)
{
    if (verbose) puts("STUB: QTAddTrackPropertyListener called");
    return NULL;
}

void* QTAllocateObjectToken(void)
{
    if (verbose) puts("STUB: QTAllocateObjectToken called");
    return NULL;
}

void* QTAllocateObjectTokenForComponent(void)
{
    if (verbose) puts("STUB: QTAllocateObjectTokenForComponent called");
    return NULL;
}

void* QTAltGetComputerSpeed(void)
{
    if (verbose) puts("STUB: QTAltGetComputerSpeed called");
    return NULL;
}

void* QTAreProAppsInstalledForQuickTimeProFeatures(void)
{
    if (verbose) puts("STUB: QTAreProAppsInstalledForQuickTimeProFeatures called");
    return NULL;
}

void* QTAudioAnalyzerCreate(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerCreate called");
    return NULL;
}

void* QTAudioAnalyzerCreateForTrack(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerCreateForTrack called");
    return NULL;
}

void* QTAudioAnalyzerDispose(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerDispose called");
    return NULL;
}

void* QTAudioAnalyzerGetBandFreqs(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerGetBandFreqs called");
    return NULL;
}

void* QTAudioAnalyzerGetFreqLevels(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerGetFreqLevels called");
    return NULL;
}

void* QTAudioAnalyzerGetNumBands(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerGetNumBands called");
    return NULL;
}

void* QTAudioAnalyzerSetNumBands(void)
{
    if (verbose) puts("STUB: QTAudioAnalyzerSetNumBands called");
    return NULL;
}

void* QTAudioChannelDescriptionsAreEquivalent(void)
{
    if (verbose) puts("STUB: QTAudioChannelDescriptionsAreEquivalent called");
    return NULL;
}

void* QTAudioChannelLayoutContainsDuplicateDescriptions(void)
{
    if (verbose) puts("STUB: QTAudioChannelLayoutContainsDuplicateDescriptions called");
    return NULL;
}

void* QTAudioChannelLayoutExpandIfPossible(void)
{
    if (verbose) puts("STUB: QTAudioChannelLayoutExpandIfPossible called");
    return NULL;
}

void* QTAudioChannelLayoutsAreEquivalent(void)
{
    if (verbose) puts("STUB: QTAudioChannelLayoutsAreEquivalent called");
    return NULL;
}

void* QTAudioContextAddProcessor(void)
{
    if (verbose) puts("STUB: QTAudioContextAddProcessor called");
    return NULL;
}

void* QTAudioContextCreateForAudioDevice(void)
{
    if (verbose) puts("STUB: QTAudioContextCreateForAudioDevice called");
    return NULL;
}

void* QTAudioContextCreateForCoreAudioDevice(void)
{
    if (verbose) puts("STUB: QTAudioContextCreateForCoreAudioDevice called");
    return NULL;
}

void* QTAudioContextCreateForSoundOutputComponent(void)
{
    if (verbose) puts("STUB: QTAudioContextCreateForSoundOutputComponent called");
    return NULL;
}

void* QTAudioContextCreateMovieSubContext(void)
{
    if (verbose) puts("STUB: QTAudioContextCreateMovieSubContext called");
    return NULL;
}

void* QTAudioContextCreateSubContext(void)
{
    if (verbose) puts("STUB: QTAudioContextCreateSubContext called");
    return NULL;
}

void* QTAudioContextGetClientID(void)
{
    if (verbose) puts("STUB: QTAudioContextGetClientID called");
    return NULL;
}

void* QTAudioContextGetClientInfo(void)
{
    if (verbose) puts("STUB: QTAudioContextGetClientInfo called");
    return NULL;
}

void* QTAudioContextGetDebugString(void)
{
    if (verbose) puts("STUB: QTAudioContextGetDebugString called");
    return NULL;
}

void* QTAudioContextGetParentContextFormat(void)
{
    if (verbose) puts("STUB: QTAudioContextGetParentContextFormat called");
    return NULL;
}

void* QTAudioContextInitialize(void)
{
    if (verbose) puts("STUB: QTAudioContextInitialize called");
    return NULL;
}

void* QTAudioContextRegisterInsert(void)
{
    if (verbose) puts("STUB: QTAudioContextRegisterInsert called");
    return NULL;
}

void* QTAudioContextRelease(void)
{
    if (verbose) puts("STUB: QTAudioContextRelease called");
    return NULL;
}

void* QTAudioContextRemoveProcessor(void)
{
    if (verbose) puts("STUB: QTAudioContextRemoveProcessor called");
    return NULL;
}

void* QTAudioContextReset(void)
{
    if (verbose) puts("STUB: QTAudioContextReset called");
    return NULL;
}

void* QTAudioContextRetain(void)
{
    if (verbose) puts("STUB: QTAudioContextRetain called");
    return NULL;
}

void* QTAudioContextSendMessage(void)
{
    if (verbose) puts("STUB: QTAudioContextSendMessage called");
    return NULL;
}

void* QTAudioContextSetClientID(void)
{
    if (verbose) puts("STUB: QTAudioContextSetClientID called");
    return NULL;
}

void* QTAudioContextSetClientInfo(void)
{
    if (verbose) puts("STUB: QTAudioContextSetClientInfo called");
    return NULL;
}

void* QTAudioContextSetDebugString(void)
{
    if (verbose) puts("STUB: QTAudioContextSetDebugString called");
    return NULL;
}

void* QTAudioContextSetProperty(void)
{
    if (verbose) puts("STUB: QTAudioContextSetProperty called");
    return NULL;
}

void* QTAudioContextStart(void)
{
    if (verbose) puts("STUB: QTAudioContextStart called");
    return NULL;
}

void* QTAudioContextStartRootContext(void)
{
    if (verbose) puts("STUB: QTAudioContextStartRootContext called");
    return NULL;
}

void* QTAudioContextStop(void)
{
    if (verbose) puts("STUB: QTAudioContextStop called");
    return NULL;
}

void* QTAudioContextStopRootContext(void)
{
    if (verbose) puts("STUB: QTAudioContextStopRootContext called");
    return NULL;
}

void* QTAudioContextTask(void)
{
    if (verbose) puts("STUB: QTAudioContextTask called");
    return NULL;
}

void* QTAudioContextTaskRootContext(void)
{
    if (verbose) puts("STUB: QTAudioContextTaskRootContext called");
    return NULL;
}

void* QTAudioContextUninitialize(void)
{
    if (verbose) puts("STUB: QTAudioContextUninitialize called");
    return NULL;
}

void* QTAudioDeviceContextCreate(void)
{
    if (verbose) puts("STUB: QTAudioDeviceContextCreate called");
    return NULL;
}

void* QTAudioOutputProcContextCreate(void)
{
    if (verbose) puts("STUB: QTAudioOutputProcContextCreate called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorAddOutputTime(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorAddOutputTime called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorAdvanceInputTime(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorAdvanceInputTime called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorCreate(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorCreate called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorDispose(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorDispose called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorGenerateInputTime(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorGenerateInputTime called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorGetStartInputAtZero(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorGetStartInputAtZero called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorReset(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorReset called");
    return NULL;
}

void* QTAudioPullTimeStampGeneratorSetStartInputAtZero(void)
{
    if (verbose) puts("STUB: QTAudioPullTimeStampGeneratorSetStartInputAtZero called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorAddInputTime(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorAddInputTime called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorAdvanceOutputTime(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorAdvanceOutputTime called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorCreate(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorCreate called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorDispose(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorDispose called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorGenerateOutputTime(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorGenerateOutputTime called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorGetStartOutputAtZero(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorGetStartOutputAtZero called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorReset(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorReset called");
    return NULL;
}

void* QTAudioPushTimeStampGeneratorSetStartOutputAtZero(void)
{
    if (verbose) puts("STUB: QTAudioPushTimeStampGeneratorSetStartOutputAtZero called");
    return NULL;
}

void* QTBandwidthRelease(void)
{
    if (verbose) puts("STUB: QTBandwidthRelease called");
    return NULL;
}

void* QTBandwidthRequest(void)
{
    if (verbose) puts("STUB: QTBandwidthRequest called");
    return NULL;
}

void* QTBandwidthRequestForTimeBase(void)
{
    if (verbose) puts("STUB: QTBandwidthRequestForTimeBase called");
    return NULL;
}

void* QTBeginUserInteractionSuppression(void)
{
    if (verbose) puts("STUB: QTBeginUserInteractionSuppression called");
    return NULL;
}

void* QTBuildListOfExportComponents(void)
{
    if (verbose) puts("STUB: QTBuildListOfExportComponents called");
    return NULL;
}

void* QTBuyProNow(void)
{
    if (verbose) puts("STUB: QTBuyProNow called");
    return NULL;
}

void* QTCacheHFlush(void)
{
    if (verbose) puts("STUB: QTCacheHFlush called");
    return NULL;
}

void* QTCacheHReadData(void)
{
    if (verbose) puts("STUB: QTCacheHReadData called");
    return NULL;
}

void* QTCacheHWriteData(void)
{
    if (verbose) puts("STUB: QTCacheHWriteData called");
    return NULL;
}

void* QTCallocWithError(void)
{
    if (verbose) puts("STUB: QTCallocWithError called");
    return NULL;
}

void* QTCanMoviePlayThrough(void)
{
    if (verbose) puts("STUB: QTCanMoviePlayThrough called");
    return NULL;
}

void* QTChannelLayoutAccumulatorAppend(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorAppend called");
    return NULL;
}

void* QTChannelLayoutAccumulatorCreate(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorCreate called");
    return NULL;
}

void* QTChannelLayoutAccumulatorDispose(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorDispose called");
    return NULL;
}

void* QTChannelLayoutAccumulatorGetLayout(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorGetLayout called");
    return NULL;
}

void* QTChannelLayoutAccumulatorGetLayoutSize(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorGetLayoutSize called");
    return NULL;
}

void* QTChannelLayoutAccumulatorGetNumChannels(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorGetNumChannels called");
    return NULL;
}

void* QTChannelLayoutAccumulatorRemapMonoIfPossible(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorRemapMonoIfPossible called");
    return NULL;
}

void* QTChannelLayoutAccumulatorsAreEquivalent(void)
{
    if (verbose) puts("STUB: QTChannelLayoutAccumulatorsAreEquivalent called");
    return NULL;
}

void* QTCheckMovieSecurity(void)
{
    if (verbose) puts("STUB: QTCheckMovieSecurity called");
    return NULL;
}

void* QTChunkPoolCallForEachFreePoolMember(void)
{
    if (verbose) puts("STUB: QTChunkPoolCallForEachFreePoolMember called");
    return NULL;
}

void* QTChunkPoolCallForEachPoolMember(void)
{
    if (verbose) puts("STUB: QTChunkPoolCallForEachPoolMember called");
    return NULL;
}

void* QTChunkPoolCountBlocksInUse(void)
{
    if (verbose) puts("STUB: QTChunkPoolCountBlocksInUse called");
    return NULL;
}

void* QTChunkPoolCreate(void)
{
    if (verbose) puts("STUB: QTChunkPoolCreate called");
    return NULL;
}

void* QTChunkPoolRelease(void)
{
    if (verbose) puts("STUB: QTChunkPoolRelease called");
    return NULL;
}

void* QTChunkPoolRetain(void)
{
    if (verbose) puts("STUB: QTChunkPoolRetain called");
    return NULL;
}

void* QTChunkPoolReturnBlock(void)
{
    if (verbose) puts("STUB: QTChunkPoolReturnBlock called");
    return NULL;
}

void* QTChunkPoolUseBlock(void)
{
    if (verbose) puts("STUB: QTChunkPoolUseBlock called");
    return NULL;
}

void* QTClearMovieErrorList(void)
{
    if (verbose) puts("STUB: QTClearMovieErrorList called");
    return NULL;
}

void* QTCloseDialog(void)
{
    if (verbose) puts("STUB: QTCloseDialog called");
    return NULL;
}

void* QTCodecClockGetRate(void)
{
    if (verbose) puts("STUB: QTCodecClockGetRate called");
    return NULL;
}

void* QTCodecClockSetRate(void)
{
    if (verbose) puts("STUB: QTCodecClockSetRate called");
    return NULL;
}

void* QTCodecClockSetTime(void)
{
    if (verbose) puts("STUB: QTCodecClockSetTime called");
    return NULL;
}

void* QTCompareTrackMediaCompatibility(void)
{
    if (verbose) puts("STUB: QTCompareTrackMediaCompatibility called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionAddDeferredListener(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionAddDeferredListener called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionAddListener(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionAddListener called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionCreate(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionCreate called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionDispatchDeferredIfNeeded(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionDispatchDeferredIfNeeded called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionHasListenersForProperty(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionHasListenersForProperty called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionIsEmpty(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionIsEmpty called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionMakeDeferrable(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionMakeDeferrable called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionNotifyListeners(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionNotifyListeners called");
    return NULL;
}

void* QTComponentPropertyListenerCollectionRemoveListener(void)
{
    if (verbose) puts("STUB: QTComponentPropertyListenerCollectionRemoveListener called");
    return NULL;
}

void* QTConvertBytesToCharacter(void)
{
    if (verbose) puts("STUB: QTConvertBytesToCharacter called");
    return NULL;
}

void* QTConvertCharacterToBytes(void)
{
    if (verbose) puts("STUB: QTConvertCharacterToBytes called");
    return NULL;
}

void* QTConvertLineFeeds(void)
{
    if (verbose) puts("STUB: QTConvertLineFeeds called");
    return NULL;
}

void* QTConvertSoundDescription(void)
{
    if (verbose) puts("STUB: QTConvertSoundDescription called");
    return NULL;
}

void* QTCopyAtom(void)
{
    if (verbose) puts("STUB: QTCopyAtom called");
    return NULL;
}

void* QTCopyAtomDataToHandle(void)
{
    if (verbose) puts("STUB: QTCopyAtomDataToHandle called");
    return NULL;
}

void* QTCopyAtomDataToPtr(void)
{
    if (verbose) puts("STUB: QTCopyAtomDataToPtr called");
    return NULL;
}

void* QTCopyMediaMetaData(void)
{
    if (verbose) puts("STUB: QTCopyMediaMetaData called");
    return NULL;
}

void* QTCopyMovieMetaData(void)
{
    if (verbose) puts("STUB: QTCopyMovieMetaData called");
    return NULL;
}

void* QTCopyMovieProfileInformation(void)
{
    if (verbose) puts("STUB: QTCopyMovieProfileInformation called");
    return NULL;
}

void* QTCopyObjectInterface(void)
{
    if (verbose) puts("STUB: QTCopyObjectInterface called");
    return NULL;
}

void* QTCopyObjectInterfaceByClassAndID(void)
{
    if (verbose) puts("STUB: QTCopyObjectInterfaceByClassAndID called");
    return NULL;
}

void* QTCopyObjectInterfaceByIndex(void)
{
    if (verbose) puts("STUB: QTCopyObjectInterfaceByIndex called");
    return NULL;
}

void* QTCopyObjectInterfaceByName(void)
{
    if (verbose) puts("STUB: QTCopyObjectInterfaceByName called");
    return NULL;
}

void* QTCopySampleMetaData(void)
{
    if (verbose) puts("STUB: QTCopySampleMetaData called");
    return NULL;
}

void* QTCopyTrackMetaData(void)
{
    if (verbose) puts("STUB: QTCopyTrackMetaData called");
    return NULL;
}

void* QTCopyTrackProfileInformation(void)
{
    if (verbose) puts("STUB: QTCopyTrackProfileInformation called");
    return NULL;
}

void* QTCopyUnclonedMediaIntoClonedMedia(void)
{
    if (verbose) puts("STUB: QTCopyUnclonedMediaIntoClonedMedia called");
    return NULL;
}

void* QTCoreAudioErrToMacErr(void)
{
    if (verbose) puts("STUB: QTCoreAudioErrToMacErr called");
    return NULL;
}

void* QTCountBytesForCharacters(void)
{
    if (verbose) puts("STUB: QTCountBytesForCharacters called");
    return NULL;
}

void* QTCountCharactersForBytes(void)
{
    if (verbose) puts("STUB: QTCountCharactersForBytes called");
    return NULL;
}

void* QTCountChildrenOfType(void)
{
    if (verbose) puts("STUB: QTCountChildrenOfType called");
    return NULL;
}

void* QTCountChildrenOfTypeWithSizeCheck(void)
{
    if (verbose) puts("STUB: QTCountChildrenOfTypeWithSizeCheck called");
    return NULL;
}

void* QTCountMovieErrorsOfType(void)
{
    if (verbose) puts("STUB: QTCountMovieErrorsOfType called");
    return NULL;
}

void* QTCountWarholResources(void)
{
    if (verbose) puts("STUB: QTCountWarholResources called");
    return NULL;
}

void* QTCreateChannelLayoutForUnmarkedAudio(void)
{
    if (verbose) puts("STUB: QTCreateChannelLayoutForUnmarkedAudio called");
    return NULL;
}

void* QTCreateMessage(void)
{
    if (verbose) puts("STUB: QTCreateMessage called");
    return NULL;
}

void* QTCreateNullPixelBuffer(void)
{
    if (verbose) puts("STUB: QTCreateNullPixelBuffer called");
    return NULL;
}

void* QTCreateNumericKeyForOSTypePair(void)
{
    if (verbose) puts("STUB: QTCreateNumericKeyForOSTypePair called");
    return NULL;
}

void* QTCreateObjectInterfaceDictionaryFromXMLData(void)
{
    if (verbose) puts("STUB: QTCreateObjectInterfaceDictionaryFromXMLData called");
    return NULL;
}

void* QTCreateStandardParameterDialog(void)
{
    if (verbose) puts("STUB: QTCreateStandardParameterDialog called");
    return NULL;
}

void* QTCreateUUID(void)
{
    if (verbose) puts("STUB: QTCreateUUID called");
    return NULL;
}

void* QTDRMPrivAPICheckMovieSecurity(void)
{
    if (verbose) puts("STUB: QTDRMPrivAPICheckMovieSecurity called");
    return NULL;
}

void* QTDRMPrivAPICheckTrackSecurity(void)
{
    if (verbose) puts("STUB: QTDRMPrivAPICheckTrackSecurity called");
    return NULL;
}

void* QTDRMPrivAPIInitialize(void)
{
    if (verbose) puts("STUB: QTDRMPrivAPIInitialize called");
    return NULL;
}

void* QTDataObjectCopy(void)
{
    if (verbose) puts("STUB: QTDataObjectCopy called");
    return NULL;
}

void* QTDataObjectCreateInstance(void)
{
    if (verbose) puts("STUB: QTDataObjectCreateInstance called");
    return NULL;
}

void* QTDataObjectGetClassID(void)
{
    if (verbose) puts("STUB: QTDataObjectGetClassID called");
    return NULL;
}

void* QTDataObjectGetDataPtr(void)
{
    if (verbose) puts("STUB: QTDataObjectGetDataPtr called");
    return NULL;
}

void* QTDataObjectGetPropertiesBulk(void)
{
    if (verbose) puts("STUB: QTDataObjectGetPropertiesBulk called");
    return NULL;
}

void* QTDataObjectGetProperty(void)
{
    if (verbose) puts("STUB: QTDataObjectGetProperty called");
    return NULL;
}

void* QTDataObjectGetSuperclassID(void)
{
    if (verbose) puts("STUB: QTDataObjectGetSuperclassID called");
    return NULL;
}

void* QTDataObjectIsKindOfClass(void)
{
    if (verbose) puts("STUB: QTDataObjectIsKindOfClass called");
    return NULL;
}

void* QTDataObjectIsMemberOfClass(void)
{
    if (verbose) puts("STUB: QTDataObjectIsMemberOfClass called");
    return NULL;
}

void* QTDataObjectRegisterClass(void)
{
    if (verbose) puts("STUB: QTDataObjectRegisterClass called");
    return NULL;
}

void* QTDataObjectSendMessage(void)
{
    if (verbose) puts("STUB: QTDataObjectSendMessage called");
    return NULL;
}

void* QTDataObjectSendMessageToInstanceOfClass(void)
{
    if (verbose) puts("STUB: QTDataObjectSendMessageToInstanceOfClass called");
    return NULL;
}

void* QTDataObjectSetPropertiesBulk(void)
{
    if (verbose) puts("STUB: QTDataObjectSetPropertiesBulk called");
    return NULL;
}

void* QTDataObjectSetProperty(void)
{
    if (verbose) puts("STUB: QTDataObjectSetProperty called");
    return NULL;
}

void* QTDataObjectUnregisterClass(void)
{
    if (verbose) puts("STUB: QTDataObjectUnregisterClass called");
    return NULL;
}

void* QTDataSourceDispose(void)
{
    if (verbose) puts("STUB: QTDataSourceDispose called");
    return NULL;
}

void* QTDataSourceGetProperty(void)
{
    if (verbose) puts("STUB: QTDataSourceGetProperty called");
    return NULL;
}

void* QTDataSourceGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTDataSourceGetPropertyInfo called");
    return NULL;
}

void* QTDataSourceNew(void)
{
    if (verbose) puts("STUB: QTDataSourceNew called");
    return NULL;
}

void* QTDataSourceNewWithFlags(void)
{
    if (verbose) puts("STUB: QTDataSourceNewWithFlags called");
    return NULL;
}

void* QTDataSourceSetProperty(void)
{
    if (verbose) puts("STUB: QTDataSourceSetProperty called");
    return NULL;
}

void* QTDataSourceTask(void)
{
    if (verbose) puts("STUB: QTDataSourceTask called");
    return NULL;
}

void* QTDebugKeysGetBoolean(void)
{
    if (verbose) puts("STUB: QTDebugKeysGetBoolean called");
    return NULL;
}

void* QTDebugKeysGetInteger(void)
{
    if (verbose) puts("STUB: QTDebugKeysGetInteger called");
    return NULL;
}

void* QTDebugKeysGetString(void)
{
    if (verbose) puts("STUB: QTDebugKeysGetString called");
    return NULL;
}

void* QTDebugShowMeTheBackTrace(void)
{
    if (verbose) puts("STUB: QTDebugShowMeTheBackTrace called");
    return NULL;
}

void* QTDismissStandardParameterDialog(void)
{
    if (verbose) puts("STUB: QTDismissStandardParameterDialog called");
    return NULL;
}

void* QTDispatchComponentMessage(void)
{
    if (verbose) puts("STUB: QTDispatchComponentMessage called");
    return NULL;
}

void* QTDisplayTimeRangeToCopyBoundaryDecodeTimeRange(void)
{
    if (verbose) puts("STUB: QTDisplayTimeRangeToCopyBoundaryDecodeTimeRange called");
    return NULL;
}

void* QTDisposeAtomContainer(void)
{
    if (verbose) puts("STUB: QTDisposeAtomContainer called");
    return NULL;
}

void* QTDisposeHandle(void)
{
    if (verbose) puts("STUB: QTDisposeHandle called");
    return NULL;
}

void* QTDisposeObjectToken(void)
{
    if (verbose) puts("STUB: QTDisposeObjectToken called");
    return NULL;
}

void* QTDisposePtr(void)
{
    if (verbose) puts("STUB: QTDisposePtr called");
    return NULL;
}

void* QTDisposeTween(void)
{
    if (verbose) puts("STUB: QTDisposeTween called");
    return NULL;
}

void* QTDoCallBacks(void)
{
    if (verbose) puts("STUB: QTDoCallBacks called");
    return NULL;
}

void* QTDoMetaDataCommandWithMessage(void)
{
    if (verbose) puts("STUB: QTDoMetaDataCommandWithMessage called");
    return NULL;
}

void* QTDoTween(void)
{
    if (verbose) puts("STUB: QTDoTween called");
    return NULL;
}

void* QTDoTweenPtr(void)
{
    if (verbose) puts("STUB: QTDoTweenPtr called");
    return NULL;
}

void* QTDrawErrorIntoGWorld(void)
{
    if (verbose) puts("STUB: QTDrawErrorIntoGWorld called");
    return NULL;
}

void* QTDrawGrayDialogBox(void)
{
    if (verbose) puts("STUB: QTDrawGrayDialogBox called");
    return NULL;
}

void* QTDuplicateObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTDuplicateObjectSpecifier called");
    return NULL;
}

void* QTEmptyHandle(void)
{
    if (verbose) puts("STUB: QTEmptyHandle called");
    return NULL;
}

void* QTEnableMovieFileTypeInfo(void)
{
    if (verbose) puts("STUB: QTEnableMovieFileTypeInfo called");
    return NULL;
}

void* QTEndUserInteractionSuppression(void)
{
    if (verbose) puts("STUB: QTEndUserInteractionSuppression called");
    return NULL;
}

void* QTEndianAudioChannelLayout_BtoN(void)
{
    if (verbose) puts("STUB: QTEndianAudioChannelLayout_BtoN called");
    return NULL;
}

void* QTEndianAudioChannelLayout_NtoB(void)
{
    if (verbose) puts("STUB: QTEndianAudioChannelLayout_NtoB called");
    return NULL;
}

void* QTEndianAudioStreamBasicDescription_BtoN(void)
{
    if (verbose) puts("STUB: QTEndianAudioStreamBasicDescription_BtoN called");
    return NULL;
}

void* QTEndianAudioStreamBasicDescription_NtoB(void)
{
    if (verbose) puts("STUB: QTEndianAudioStreamBasicDescription_NtoB called");
    return NULL;
}

void* QTEqualUUIDs(void)
{
    if (verbose) puts("STUB: QTEqualUUIDs called");
    return NULL;
}

void* QTEquivalentLanguageCodes(void)
{
    if (verbose) puts("STUB: QTEquivalentLanguageCodes called");
    return NULL;
}

void* QTExportSessionConvertToHandle(void)
{
    if (verbose) puts("STUB: QTExportSessionConvertToHandle called");
    return NULL;
}

void* QTExportSessionCreate(void)
{
    if (verbose) puts("STUB: QTExportSessionCreate called");
    return NULL;
}

void* QTExportSessionCreateFromHandle(void)
{
    if (verbose) puts("STUB: QTExportSessionCreateFromHandle called");
    return NULL;
}

void* QTExportSessionGetTargetDataRef(void)
{
    if (verbose) puts("STUB: QTExportSessionGetTargetDataRef called");
    return NULL;
}

void* QTExportSessionPerform(void)
{
    if (verbose) puts("STUB: QTExportSessionPerform called");
    return NULL;
}

void* QTExportSessionRelease(void)
{
    if (verbose) puts("STUB: QTExportSessionRelease called");
    return NULL;
}

void* QTExportSessionShowUI(void)
{
    if (verbose) puts("STUB: QTExportSessionShowUI called");
    return NULL;
}

void* QTFileExtensionToFileType(void)
{
    if (verbose) puts("STUB: QTFileExtensionToFileType called");
    return NULL;
}

void* QTFillOutBasicDescriptionFieldsCompletely(void)
{
    if (verbose) puts("STUB: QTFillOutBasicDescriptionFieldsCompletely called");
    return NULL;
}

void* QTFindChildByID(void)
{
    if (verbose) puts("STUB: QTFindChildByID called");
    return NULL;
}

void* QTFindChildByIndex(void)
{
    if (verbose) puts("STUB: QTFindChildByIndex called");
    return NULL;
}

void* QTFindFolder(void)
{
    if (verbose) puts("STUB: QTFindFolder called");
    return NULL;
}

void* QTFrameGrayRect(void)
{
    if (verbose) puts("STUB: QTFrameGrayRect called");
    return NULL;
}

void* QTGetAccessKeys(void)
{
    if (verbose) puts("STUB: QTGetAccessKeys called");
    return NULL;
}

void* QTGetAtomDataPtr(void)
{
    if (verbose) puts("STUB: QTGetAtomDataPtr called");
    return NULL;
}

void* QTGetAtomDataPtrWithSizeCheck(void)
{
    if (verbose) puts("STUB: QTGetAtomDataPtrWithSizeCheck called");
    return NULL;
}

void* QTGetAtomParent(void)
{
    if (verbose) puts("STUB: QTGetAtomParent called");
    return NULL;
}

void* QTGetAtomTypeAndID(void)
{
    if (verbose) puts("STUB: QTGetAtomTypeAndID called");
    return NULL;
}

void* QTGetAudioChannelCountFromLayout(void)
{
    if (verbose) puts("STUB: QTGetAudioChannelCountFromLayout called");
    return NULL;
}

void* QTGetAudioPacketInfo(void)
{
    if (verbose) puts("STUB: QTGetAudioPacketInfo called");
    return NULL;
}

void* QTGetBuyProURL(void)
{
    if (verbose) puts("STUB: QTGetBuyProURL called");
    return NULL;
}

void* QTGetComponentProperty(void)
{
    if (verbose) puts("STUB: QTGetComponentProperty called");
    return NULL;
}

void* QTGetComponentPropertyInfo(void)
{
    if (verbose) puts("STUB: QTGetComponentPropertyInfo called");
    return NULL;
}

void* QTGetConnectionSpeedFromPrefs(void)
{
    if (verbose) puts("STUB: QTGetConnectionSpeedFromPrefs called");
    return NULL;
}

void* QTGetCurrentMessageTarget(void)
{
    if (verbose) puts("STUB: QTGetCurrentMessageTarget called");
    return NULL;
}

void* QTGetDataHandlerDirectoryDataReference(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerDirectoryDataReference called");
    return NULL;
}

void* QTGetDataHandlerFullPath(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerFullPath called");
    return NULL;
}

void* QTGetDataHandlerFullPathCFString(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerFullPathCFString called");
    return NULL;
}

void* QTGetDataHandlerFullPathUnicode(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerFullPathUnicode called");
    return NULL;
}

void* QTGetDataHandlerTargetName(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerTargetName called");
    return NULL;
}

void* QTGetDataHandlerTargetNameCFString(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerTargetNameCFString called");
    return NULL;
}

void* QTGetDataHandlerTargetNameUnicode(void)
{
    if (verbose) puts("STUB: QTGetDataHandlerTargetNameUnicode called");
    return NULL;
}

void* QTGetDataRefMaxFileOffset(void)
{
    if (verbose) puts("STUB: QTGetDataRefMaxFileOffset called");
    return NULL;
}

void* QTGetDataReferenceDirectoryDataReference(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceDirectoryDataReference called");
    return NULL;
}

void* QTGetDataReferenceFullPath(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceFullPath called");
    return NULL;
}

void* QTGetDataReferenceFullPathCFString(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceFullPathCFString called");
    return NULL;
}

void* QTGetDataReferenceFullPathUnicode(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceFullPathUnicode called");
    return NULL;
}

void* QTGetDataReferenceTargetName(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceTargetName called");
    return NULL;
}

void* QTGetDataReferenceTargetNameCFString(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceTargetNameCFString called");
    return NULL;
}

void* QTGetDataReferenceTargetNameUnicode(void)
{
    if (verbose) puts("STUB: QTGetDataReferenceTargetNameUnicode called");
    return NULL;
}

void* QTGetEffectSpeed(void)
{
    if (verbose) puts("STUB: QTGetEffectSpeed called");
    return NULL;
}

void* QTGetEffectsList(void)
{
    if (verbose) puts("STUB: QTGetEffectsList called");
    return NULL;
}

void* QTGetEffectsListExtended(void)
{
    if (verbose) puts("STUB: QTGetEffectsListExtended called");
    return NULL;
}

void* QTGetFileNameExtension(void)
{
    if (verbose) puts("STUB: QTGetFileNameExtension called");
    return NULL;
}

void* QTGetHandleSize(void)
{
    if (verbose) puts("STUB: QTGetHandleSize called");
    return NULL;
}

void* QTGetLanguageCodeInfo(void)
{
    if (verbose) puts("STUB: QTGetLanguageCodeInfo called");
    return NULL;
}

void* QTGetLanguageCodeInfoFromLocale(void)
{
    if (verbose) puts("STUB: QTGetLanguageCodeInfoFromLocale called");
    return NULL;
}

void* QTGetLeafFromObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTGetLeafFromObjectSpecifier called");
    return NULL;
}

void* QTGetMIMETypeInfo(void)
{
    if (verbose) puts("STUB: QTGetMIMETypeInfo called");
    return NULL;
}

void* QTGetMediaChunkCount(void)
{
    if (verbose) puts("STUB: QTGetMediaChunkCount called");
    return NULL;
}

void* QTGetMediaChunkDataHandler(void)
{
    if (verbose) puts("STUB: QTGetMediaChunkDataHandler called");
    return NULL;
}

void* QTGetMediaChunkInfo(void)
{
    if (verbose) puts("STUB: QTGetMediaChunkInfo called");
    return NULL;
}

void* QTGetMediaSampleSizes(void)
{
    if (verbose) puts("STUB: QTGetMediaSampleSizes called");
    return NULL;
}

void* QTGetMessageAttribute(void)
{
    if (verbose) puts("STUB: QTGetMessageAttribute called");
    return NULL;
}

void* QTGetMessageBubbles(void)
{
    if (verbose) puts("STUB: QTGetMessageBubbles called");
    return NULL;
}

void* QTGetMessageDefaultActionCanceled(void)
{
    if (verbose) puts("STUB: QTGetMessageDefaultActionCanceled called");
    return NULL;
}

void* QTGetMessageIsCancelable(void)
{
    if (verbose) puts("STUB: QTGetMessageIsCancelable called");
    return NULL;
}

void* QTGetMessagePhase(void)
{
    if (verbose) puts("STUB: QTGetMessagePhase called");
    return NULL;
}

void* QTGetMessagePropagationCanceled(void)
{
    if (verbose) puts("STUB: QTGetMessagePropagationCanceled called");
    return NULL;
}

void* QTGetMessageTarget(void)
{
    if (verbose) puts("STUB: QTGetMessageTarget called");
    return NULL;
}

void* QTGetMetaDataPropertyWithMessage(void)
{
    if (verbose) puts("STUB: QTGetMetaDataPropertyWithMessage called");
    return NULL;
}

void* QTGetMissingSoftwareNotificationInfo(void)
{
    if (verbose) puts("STUB: QTGetMissingSoftwareNotificationInfo called");
    return NULL;
}

void* QTGetMovieErrorList(void)
{
    if (verbose) puts("STUB: QTGetMovieErrorList called");
    return NULL;
}

void* QTGetMovieFileTypePublicHandle(void)
{
    if (verbose) puts("STUB: QTGetMovieFileTypePublicHandle called");
    return NULL;
}

void* QTGetMovieMissingComponents(void)
{
    if (verbose) puts("STUB: QTGetMovieMissingComponents called");
    return NULL;
}

void* QTGetMovieObjectToken(void)
{
    if (verbose) puts("STUB: QTGetMovieObjectToken called");
    return NULL;
}

void* QTGetMoviePlayabilityState(void)
{
    if (verbose) puts("STUB: QTGetMoviePlayabilityState called");
    return NULL;
}

void* QTGetMoviePrivateInfo(void)
{
    if (verbose) puts("STUB: QTGetMoviePrivateInfo called");
    return NULL;
}

void* QTGetMovieProperty(void)
{
    if (verbose) puts("STUB: QTGetMovieProperty called");
    return NULL;
}

void* QTGetMoviePropertyInfo(void)
{
    if (verbose) puts("STUB: QTGetMoviePropertyInfo called");
    return NULL;
}

void* QTGetMovieRestrictions(void)
{
    if (verbose) puts("STUB: QTGetMovieRestrictions called");
    return NULL;
}

void* QTGetMovieSecurityPolicy(void)
{
    if (verbose) puts("STUB: QTGetMovieSecurityPolicy called");
    return NULL;
}

void* QTGetNewDialog(void)
{
    if (verbose) puts("STUB: QTGetNewDialog called");
    return NULL;
}

void* QTGetNextChildType(void)
{
    if (verbose) puts("STUB: QTGetNextChildType called");
    return NULL;
}

void* QTGetOSTypePairFromNumericKey(void)
{
    if (verbose) puts("STUB: QTGetOSTypePairFromNumericKey called");
    return NULL;
}

void* QTGetObjectChild(void)
{
    if (verbose) puts("STUB: QTGetObjectChild called");
    return NULL;
}

void* QTGetObjectChildCount(void)
{
    if (verbose) puts("STUB: QTGetObjectChildCount called");
    return NULL;
}

void* QTGetObjectParent(void)
{
    if (verbose) puts("STUB: QTGetObjectParent called");
    return NULL;
}

void* QTGetObjectProperty(void)
{
    if (verbose) puts("STUB: QTGetObjectProperty called");
    return NULL;
}

void* QTGetObjectSibling(void)
{
    if (verbose) puts("STUB: QTGetObjectSibling called");
    return NULL;
}

void* QTGetObjectToken(void)
{
    if (verbose) puts("STUB: QTGetObjectToken called");
    return NULL;
}

void* QTGetObjectTokenForComponent(void)
{
    if (verbose) puts("STUB: QTGetObjectTokenForComponent called");
    return NULL;
}

void* QTGetPerThreadStorage(void)
{
    if (verbose) puts("STUB: QTGetPerThreadStorage called");
    return NULL;
}

void* QTGetPixMapHandleGammaLevel(void)
{
    if (verbose) puts("STUB: QTGetPixMapHandleGammaLevel called");
    return NULL;
}

void* QTGetPixMapHandleRequestedGammaLevel(void)
{
    if (verbose) puts("STUB: QTGetPixMapHandleRequestedGammaLevel called");
    return NULL;
}

void* QTGetPixMapHandleRowBytes(void)
{
    if (verbose) puts("STUB: QTGetPixMapHandleRowBytes called");
    return NULL;
}

void* QTGetPixMapPtrGammaLevel(void)
{
    if (verbose) puts("STUB: QTGetPixMapPtrGammaLevel called");
    return NULL;
}

void* QTGetPixMapPtrRequestedGammaLevel(void)
{
    if (verbose) puts("STUB: QTGetPixMapPtrRequestedGammaLevel called");
    return NULL;
}

void* QTGetPixMapPtrRowBytes(void)
{
    if (verbose) puts("STUB: QTGetPixMapPtrRowBytes called");
    return NULL;
}

void* QTGetPixelFormatDepthForImageDescription(void)
{
    if (verbose) puts("STUB: QTGetPixelFormatDepthForImageDescription called");
    return NULL;
}

void* QTGetPixelSize(void)
{
    if (verbose) puts("STUB: QTGetPixelSize called");
    return NULL;
}

void* QTGetProcessProperty(void)
{
    if (verbose) puts("STUB: QTGetProcessProperty called");
    return NULL;
}

void* QTGetProcessPropertyInfo(void)
{
    if (verbose) puts("STUB: QTGetProcessPropertyInfo called");
    return NULL;
}

void* QTGetPtrSize(void)
{
    if (verbose) puts("STUB: QTGetPtrSize called");
    return NULL;
}

void* QTGetSampleMetaDataAssociation(void)
{
    if (verbose) puts("STUB: QTGetSampleMetaDataAssociation called");
    return NULL;
}

void* QTGetSampleMetaDataKeyAtIndex(void)
{
    if (verbose) puts("STUB: QTGetSampleMetaDataKeyAtIndex called");
    return NULL;
}

void* QTGetStashedData(void)
{
    if (verbose) puts("STUB: QTGetStashedData called");
    return NULL;
}

void* QTGetSupportedRestrictions(void)
{
    if (verbose) puts("STUB: QTGetSupportedRestrictions called");
    return NULL;
}

void* QTGetTextEncodingFromISOLangCode(void)
{
    if (verbose) puts("STUB: QTGetTextEncodingFromISOLangCode called");
    return NULL;
}

void* QTGetTextEncodingFromLangCode(void)
{
    if (verbose) puts("STUB: QTGetTextEncodingFromLangCode called");
    return NULL;
}

void* QTGetThreadGuardMode(void)
{
    if (verbose) puts("STUB: QTGetThreadGuardMode called");
    return NULL;
}

void* QTGetTimeUntilNextTask(void)
{
    if (verbose) puts("STUB: QTGetTimeUntilNextTask called");
    return NULL;
}

void* QTGetToolboxPerThreadStorage(void)
{
    if (verbose) puts("STUB: QTGetToolboxPerThreadStorage called");
    return NULL;
}

void* QTGetTrackProperty(void)
{
    if (verbose) puts("STUB: QTGetTrackProperty called");
    return NULL;
}

void* QTGetTrackPropertyInfo(void)
{
    if (verbose) puts("STUB: QTGetTrackPropertyInfo called");
    return NULL;
}

void* QTGetWallClockTimeBase(void)
{
    if (verbose) puts("STUB: QTGetWallClockTimeBase called");
    return NULL;
}

void* QTGetWarholIndResource(void)
{
    if (verbose) puts("STUB: QTGetWarholIndResource called");
    return NULL;
}

void* QTGetWarholIndString(void)
{
    if (verbose) puts("STUB: QTGetWarholIndString called");
    return NULL;
}

void* QTGetWarholResource(void)
{
    if (verbose) puts("STUB: QTGetWarholResource called");
    return NULL;
}

void* QTHGetState(void)
{
    if (verbose) puts("STUB: QTHGetState called");
    return NULL;
}

void* QTHLock(void)
{
    if (verbose) puts("STUB: QTHLock called");
    return NULL;
}

void* QTHSetState(void)
{
    if (verbose) puts("STUB: QTHSetState called");
    return NULL;
}

void* QTHUnlock(void)
{
    if (verbose) puts("STUB: QTHUnlock called");
    return NULL;
}

void* QTHandAndHand(void)
{
    if (verbose) puts("STUB: QTHandAndHand called");
    return NULL;
}

void* QTHandToHand(void)
{
    if (verbose) puts("STUB: QTHandToHand called");
    return NULL;
}

void* QTHashTableAddKeyAndValuePair(void)
{
    if (verbose) puts("STUB: QTHashTableAddKeyAndValuePair called");
    return NULL;
}

void* QTHashTableCapacity(void)
{
    if (verbose) puts("STUB: QTHashTableCapacity called");
    return NULL;
}

void* QTHashTableCreate(void)
{
    if (verbose) puts("STUB: QTHashTableCreate called");
    return NULL;
}

void* QTHashTableGetValueForKey(void)
{
    if (verbose) puts("STUB: QTHashTableGetValueForKey called");
    return NULL;
}

void* QTHashTableIteratorCreate(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorCreate called");
    return NULL;
}

void* QTHashTableIteratorCurrent(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorCurrent called");
    return NULL;
}

void* QTHashTableIteratorDispose(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorDispose called");
    return NULL;
}

void* QTHashTableIteratorFirst(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorFirst called");
    return NULL;
}

void* QTHashTableIteratorNext(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorNext called");
    return NULL;
}

void* QTHashTableIteratorReset(void)
{
    if (verbose) puts("STUB: QTHashTableIteratorReset called");
    return NULL;
}

void* QTHashTableOccupiedCount(void)
{
    if (verbose) puts("STUB: QTHashTableOccupiedCount called");
    return NULL;
}

void* QTHashTableRelease(void)
{
    if (verbose) puts("STUB: QTHashTableRelease called");
    return NULL;
}

void* QTHashTableRemoveAllKeys(void)
{
    if (verbose) puts("STUB: QTHashTableRemoveAllKeys called");
    return NULL;
}

void* QTHashTableRemoveKeyValuePair(void)
{
    if (verbose) puts("STUB: QTHashTableRemoveKeyValuePair called");
    return NULL;
}

void* QTHashTableRetain(void)
{
    if (verbose) puts("STUB: QTHashTableRetain called");
    return NULL;
}

void* QTHideSheetWindow(void)
{
    if (verbose) puts("STUB: QTHideSheetWindow called");
    return NULL;
}

void* QTIdentifyTextBufferFormat(void)
{
    if (verbose) puts("STUB: QTIdentifyTextBufferFormat called");
    return NULL;
}

void* QTIdleManagerClose(void)
{
    if (verbose) puts("STUB: QTIdleManagerClose called");
    return NULL;
}

void* QTIdleManagerGetNextIdleTime(void)
{
    if (verbose) puts("STUB: QTIdleManagerGetNextIdleTime called");
    return NULL;
}

void* QTIdleManagerNeedsAnIdle(void)
{
    if (verbose) puts("STUB: QTIdleManagerNeedsAnIdle called");
    return NULL;
}

void* QTIdleManagerOpen(void)
{
    if (verbose) puts("STUB: QTIdleManagerOpen called");
    return NULL;
}

void* QTIdleManagerSetNextIdleTime(void)
{
    if (verbose) puts("STUB: QTIdleManagerSetNextIdleTime called");
    return NULL;
}

void* QTIdleManagerSetNextIdleTimeDelta(void)
{
    if (verbose) puts("STUB: QTIdleManagerSetNextIdleTimeDelta called");
    return NULL;
}

void* QTIdleManagerSetNextIdleTimeNever(void)
{
    if (verbose) puts("STUB: QTIdleManagerSetNextIdleTimeNever called");
    return NULL;
}

void* QTIdleManagerSetNextIdleTimeNow(void)
{
    if (verbose) puts("STUB: QTIdleManagerSetNextIdleTimeNow called");
    return NULL;
}

void* QTIdleManagerSetParent(void)
{
    if (verbose) puts("STUB: QTIdleManagerSetParent called");
    return NULL;
}

void* QTImageQueueConnect(void)
{
    if (verbose) puts("STUB: QTImageQueueConnect called");
    return NULL;
}

void* QTImageQueueCreate(void)
{
    if (verbose) puts("STUB: QTImageQueueCreate called");
    return NULL;
}

void* QTImageQueueDispose(void)
{
    if (verbose) puts("STUB: QTImageQueueDispose called");
    return NULL;
}

void* QTImageQueueEnqueue(void)
{
    if (verbose) puts("STUB: QTImageQueueEnqueue called");
    return NULL;
}

void* QTImageQueueFlush(void)
{
    if (verbose) puts("STUB: QTImageQueueFlush called");
    return NULL;
}

void* QTImageQueueGetTrackingCallback(void)
{
    if (verbose) puts("STUB: QTImageQueueGetTrackingCallback called");
    return NULL;
}

void* QTImageQueueSetTrackingCallback(void)
{
    if (verbose) puts("STUB: QTImageQueueSetTrackingCallback called");
    return NULL;
}

void* QTInsertChild(void)
{
    if (verbose) puts("STUB: QTInsertChild called");
    return NULL;
}

void* QTInsertChildren(void)
{
    if (verbose) puts("STUB: QTInsertChildren called");
    return NULL;
}

void* QTInstallDialogEventHandlers(void)
{
    if (verbose) puts("STUB: QTInstallDialogEventHandlers called");
    return NULL;
}

void* QTInstallFrequentTimer(void)
{
    if (verbose) puts("STUB: QTInstallFrequentTimer called");
    return NULL;
}

void* QTInstallMessageHandler(void)
{
    if (verbose) puts("STUB: QTInstallMessageHandler called");
    return NULL;
}

void* QTInstallMessageHandlerForComponent(void)
{
    if (verbose) puts("STUB: QTInstallMessageHandlerForComponent called");
    return NULL;
}

void* QTInstallNextTaskNeededSoonerCallback(void)
{
    if (verbose) puts("STUB: QTInstallNextTaskNeededSoonerCallback called");
    return NULL;
}

void* QTInternalGetAudioInfo(void)
{
    if (verbose) puts("STUB: QTInternalGetAudioInfo called");
    return NULL;
}

void* QTIsMovieFileTypeInfoEnabled(void)
{
    if (verbose) puts("STUB: QTIsMovieFileTypeInfoEnabled called");
    return NULL;
}

void* QTIsNullPixelBuffer(void)
{
    if (verbose) puts("STUB: QTIsNullPixelBuffer called");
    return NULL;
}

void* QTIsSheet(void)
{
    if (verbose) puts("STUB: QTIsSheet called");
    return NULL;
}

void* QTIsStandardParameterDialogEvent(void)
{
    if (verbose) puts("STUB: QTIsStandardParameterDialogEvent called");
    return NULL;
}

void* QTIsUserInteractionAllowed(void)
{
    if (verbose) puts("STUB: QTIsUserInteractionAllowed called");
    return NULL;
}

void* QTIsUserInteractionAllowed2(void)
{
    if (verbose) puts("STUB: QTIsUserInteractionAllowed2 called");
    return NULL;
}

void* QTListAttributeRefClone(void)
{
    if (verbose) puts("STUB: QTListAttributeRefClone called");
    return NULL;
}

void* QTListAttributeRefDispose(void)
{
    if (verbose) puts("STUB: QTListAttributeRefDispose called");
    return NULL;
}

void* QTListAttributeRefNew(void)
{
    if (verbose) puts("STUB: QTListAttributeRefNew called");
    return NULL;
}

void* QTListCopyAttributeDataToPtr(void)
{
    if (verbose) puts("STUB: QTListCopyAttributeDataToPtr called");
    return NULL;
}

void* QTListCopyElementDataToPtr(void)
{
    if (verbose) puts("STUB: QTListCopyElementDataToPtr called");
    return NULL;
}

void* QTListCopySublist(void)
{
    if (verbose) puts("STUB: QTListCopySublist called");
    return NULL;
}

void* QTListCountAttributes(void)
{
    if (verbose) puts("STUB: QTListCountAttributes called");
    return NULL;
}

void* QTListCountChildElements(void)
{
    if (verbose) puts("STUB: QTListCountChildElements called");
    return NULL;
}

void* QTListCutSublist(void)
{
    if (verbose) puts("STUB: QTListCutSublist called");
    return NULL;
}

void* QTListDeleteAttribute(void)
{
    if (verbose) puts("STUB: QTListDeleteAttribute called");
    return NULL;
}

void* QTListDeleteElement(void)
{
    if (verbose) puts("STUB: QTListDeleteElement called");
    return NULL;
}

void* QTListDeleteSublist(void)
{
    if (verbose) puts("STUB: QTListDeleteSublist called");
    return NULL;
}

void* QTListDispose(void)
{
    if (verbose) puts("STUB: QTListDispose called");
    return NULL;
}

void* QTListElementRefClone(void)
{
    if (verbose) puts("STUB: QTListElementRefClone called");
    return NULL;
}

void* QTListElementRefDispose(void)
{
    if (verbose) puts("STUB: QTListElementRefDispose called");
    return NULL;
}

void* QTListElementRefGetList(void)
{
    if (verbose) puts("STUB: QTListElementRefGetList called");
    return NULL;
}

void* QTListElementRefNew(void)
{
    if (verbose) puts("STUB: QTListElementRefNew called");
    return NULL;
}

void* QTListFindAttributeByIndex(void)
{
    if (verbose) puts("STUB: QTListFindAttributeByIndex called");
    return NULL;
}

void* QTListFindAttributeByName(void)
{
    if (verbose) puts("STUB: QTListFindAttributeByName called");
    return NULL;
}

void* QTListFindAttributeParentElement(void)
{
    if (verbose) puts("STUB: QTListFindAttributeParentElement called");
    return NULL;
}

void* QTListFindChildElementByPathName(void)
{
    if (verbose) puts("STUB: QTListFindChildElementByPathName called");
    return NULL;
}

void* QTListFindElementByIndex(void)
{
    if (verbose) puts("STUB: QTListFindElementByIndex called");
    return NULL;
}

void* QTListFindElementByName(void)
{
    if (verbose) puts("STUB: QTListFindElementByName called");
    return NULL;
}

void* QTListFindParentElement(void)
{
    if (verbose) puts("STUB: QTListFindParentElement called");
    return NULL;
}

void* QTListFindRootElement(void)
{
    if (verbose) puts("STUB: QTListFindRootElement called");
    return NULL;
}

void* QTListGetAttributeIndex(void)
{
    if (verbose) puts("STUB: QTListGetAttributeIndex called");
    return NULL;
}

void* QTListGetAttributeName(void)
{
    if (verbose) puts("STUB: QTListGetAttributeName called");
    return NULL;
}

void* QTListGetAttributeTypeAndSize(void)
{
    if (verbose) puts("STUB: QTListGetAttributeTypeAndSize called");
    return NULL;
}

void* QTListGetElementIndex(void)
{
    if (verbose) puts("STUB: QTListGetElementIndex called");
    return NULL;
}

void* QTListGetElementName(void)
{
    if (verbose) puts("STUB: QTListGetElementName called");
    return NULL;
}

void* QTListGetElementPathName(void)
{
    if (verbose) puts("STUB: QTListGetElementPathName called");
    return NULL;
}

void* QTListGetElementTypeAndSize(void)
{
    if (verbose) puts("STUB: QTListGetElementTypeAndSize called");
    return NULL;
}

void* QTListInsertNewAttribute(void)
{
    if (verbose) puts("STUB: QTListInsertNewAttribute called");
    return NULL;
}

void* QTListInsertNewElement(void)
{
    if (verbose) puts("STUB: QTListInsertNewElement called");
    return NULL;
}

void* QTListNew(void)
{
    if (verbose) puts("STUB: QTListNew called");
    return NULL;
}

void* QTListPasteSublist(void)
{
    if (verbose) puts("STUB: QTListPasteSublist called");
    return NULL;
}

void* QTListReadFromXML(void)
{
    if (verbose) puts("STUB: QTListReadFromXML called");
    return NULL;
}

void* QTListSetAttributeDataAs(void)
{
    if (verbose) puts("STUB: QTListSetAttributeDataAs called");
    return NULL;
}

void* QTListSetAttributeName(void)
{
    if (verbose) puts("STUB: QTListSetAttributeName called");
    return NULL;
}

void* QTListSetElementDataAs(void)
{
    if (verbose) puts("STUB: QTListSetElementDataAs called");
    return NULL;
}

void* QTListSetElementName(void)
{
    if (verbose) puts("STUB: QTListSetElementName called");
    return NULL;
}

void* QTListSetMatchingElementData(void)
{
    if (verbose) puts("STUB: QTListSetMatchingElementData called");
    return NULL;
}

void* QTListValidateDataType(void)
{
    if (verbose) puts("STUB: QTListValidateDataType called");
    return NULL;
}

void* QTListWriteToXML(void)
{
    if (verbose) puts("STUB: QTListWriteToXML called");
    return NULL;
}

void* QTLockContainer(void)
{
    if (verbose) puts("STUB: QTLockContainer called");
    return NULL;
}

void* QTMIDIGetMIDIPorts(void)
{
    if (verbose) puts("STUB: QTMIDIGetMIDIPorts called");
    return NULL;
}

void* QTMIDISendMIDI(void)
{
    if (verbose) puts("STUB: QTMIDISendMIDI called");
    return NULL;
}

void* QTMIDIUseReceivePort(void)
{
    if (verbose) puts("STUB: QTMIDIUseReceivePort called");
    return NULL;
}

void* QTMIDIUseSendPort(void)
{
    if (verbose) puts("STUB: QTMIDIUseSendPort called");
    return NULL;
}

void* QTMLCreateMutex(void)
{
    if (verbose) puts("STUB: QTMLCreateMutex called");
    return NULL;
}

void* QTMLDestroyMutex(void)
{
    if (verbose) puts("STUB: QTMLDestroyMutex called");
    return NULL;
}

void* QTMLGrabMutex(void)
{
    if (verbose) puts("STUB: QTMLGrabMutex called");
    return NULL;
}

void* QTMLReturnMutex(void)
{
    if (verbose) puts("STUB: QTMLReturnMutex called");
    return NULL;
}

void* QTMLTryGrabMutex(void)
{
    if (verbose) puts("STUB: QTMLTryGrabMutex called");
    return NULL;
}

void* QTMLYieldCPU(void)
{
    if (verbose) puts("STUB: QTMLYieldCPU called");
    return NULL;
}

void* QTMLYieldCPUTime(void)
{
    if (verbose) puts("STUB: QTMLYieldCPUTime called");
    return NULL;
}

void* QTMallocWithError(void)
{
    if (verbose) puts("STUB: QTMallocWithError called");
    return NULL;
}

void* QTMapToISOLanguageCode(void)
{
    if (verbose) puts("STUB: QTMapToISOLanguageCode called");
    return NULL;
}

void* QTMediaKeyDisposeList(void)
{
    if (verbose) puts("STUB: QTMediaKeyDisposeList called");
    return NULL;
}

void* QTMediaKeyGetBooleanValue(void)
{
    if (verbose) puts("STUB: QTMediaKeyGetBooleanValue called");
    return NULL;
}

void* QTMediaKeyIsSet(void)
{
    if (verbose) puts("STUB: QTMediaKeyIsSet called");
    return NULL;
}

void* QTMetaDataAddItem(void)
{
    if (verbose) puts("STUB: QTMetaDataAddItem called");
    return NULL;
}

void* QTMetaDataCopyItemValueAsCFType(void)
{
    if (verbose) puts("STUB: QTMetaDataCopyItemValueAsCFType called");
    return NULL;
}

void* QTMetaDataCreateDataAsCFType(void)
{
    if (verbose) puts("STUB: QTMetaDataCreateDataAsCFType called");
    return NULL;
}

void* QTMetaDataCreateFromBuffer(void)
{
    if (verbose) puts("STUB: QTMetaDataCreateFromBuffer called");
    return NULL;
}

void* QTMetaDataGetBytes(void)
{
    if (verbose) puts("STUB: QTMetaDataGetBytes called");
    return NULL;
}

void* QTMetaDataGetContainerProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataGetContainerProperty called");
    return NULL;
}

void* QTMetaDataGetContainerPropertyInfo(void)
{
    if (verbose) puts("STUB: QTMetaDataGetContainerPropertyInfo called");
    return NULL;
}

void* QTMetaDataGetItemCountWithKey(void)
{
    if (verbose) puts("STUB: QTMetaDataGetItemCountWithKey called");
    return NULL;
}

void* QTMetaDataGetItemProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataGetItemProperty called");
    return NULL;
}

void* QTMetaDataGetItemPropertyInfo(void)
{
    if (verbose) puts("STUB: QTMetaDataGetItemPropertyInfo called");
    return NULL;
}

void* QTMetaDataGetItemValue(void)
{
    if (verbose) puts("STUB: QTMetaDataGetItemValue called");
    return NULL;
}

void* QTMetaDataGetItemWithPersistentID(void)
{
    if (verbose) puts("STUB: QTMetaDataGetItemWithPersistentID called");
    return NULL;
}

void* QTMetaDataGetNextItem(void)
{
    if (verbose) puts("STUB: QTMetaDataGetNextItem called");
    return NULL;
}

void* QTMetaDataGetProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataGetProperty called");
    return NULL;
}

void* QTMetaDataGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTMetaDataGetPropertyInfo called");
    return NULL;
}

void* QTMetaDataNew(void)
{
    if (verbose) puts("STUB: QTMetaDataNew called");
    return NULL;
}

void* QTMetaDataNewFromBuffer(void)
{
    if (verbose) puts("STUB: QTMetaDataNewFromBuffer called");
    return NULL;
}

void* QTMetaDataNewFromBufferWithOwner(void)
{
    if (verbose) puts("STUB: QTMetaDataNewFromBufferWithOwner called");
    return NULL;
}

void* QTMetaDataNewFromQTMetaData(void)
{
    if (verbose) puts("STUB: QTMetaDataNewFromQTMetaData called");
    return NULL;
}

void* QTMetaDataRelease(void)
{
    if (verbose) puts("STUB: QTMetaDataRelease called");
    return NULL;
}

void* QTMetaDataRemoveItem(void)
{
    if (verbose) puts("STUB: QTMetaDataRemoveItem called");
    return NULL;
}

void* QTMetaDataRemoveItemsWithKey(void)
{
    if (verbose) puts("STUB: QTMetaDataRemoveItemsWithKey called");
    return NULL;
}

void* QTMetaDataRetain(void)
{
    if (verbose) puts("STUB: QTMetaDataRetain called");
    return NULL;
}

void* QTMetaDataSetContainerProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataSetContainerProperty called");
    return NULL;
}

void* QTMetaDataSetItem(void)
{
    if (verbose) puts("STUB: QTMetaDataSetItem called");
    return NULL;
}

void* QTMetaDataSetItemProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataSetItemProperty called");
    return NULL;
}

void* QTMetaDataSetProperty(void)
{
    if (verbose) puts("STUB: QTMetaDataSetProperty called");
    return NULL;
}

void* QTMovieExportSessionCopySerializedState(void)
{
    if (verbose) puts("STUB: QTMovieExportSessionCopySerializedState called");
    return NULL;
}

void* QTMovieExportSessionDoUserDialog(void)
{
    if (verbose) puts("STUB: QTMovieExportSessionDoUserDialog called");
    return NULL;
}

void* QTMovieExportSessionInitializeFromSerializedState(void)
{
    if (verbose) puts("STUB: QTMovieExportSessionInitializeFromSerializedState called");
    return NULL;
}

void* QTMovieExportSessionPerform(void)
{
    if (verbose) puts("STUB: QTMovieExportSessionPerform called");
    return NULL;
}

void* QTMovieExportSessionTask(void)
{
    if (verbose) puts("STUB: QTMovieExportSessionTask called");
    return NULL;
}

void* QTMovieNeedsTimeTable(void)
{
    if (verbose) puts("STUB: QTMovieNeedsTimeTable called");
    return NULL;
}

void* QTMovieNeedsTimeToExtentTable(void)
{
    if (verbose) puts("STUB: QTMovieNeedsTimeToExtentTable called");
    return NULL;
}

void* QTMovieStatusAddPropertyListener(void)
{
    if (verbose) puts("STUB: QTMovieStatusAddPropertyListener called");
    return NULL;
}

void* QTMovieStatusGetDataContainerProperty(void)
{
    if (verbose) puts("STUB: QTMovieStatusGetDataContainerProperty called");
    return NULL;
}

void* QTMovieStatusGetDataContainerPropertyInfo(void)
{
    if (verbose) puts("STUB: QTMovieStatusGetDataContainerPropertyInfo called");
    return NULL;
}

void* QTMovieStatusGetProperty(void)
{
    if (verbose) puts("STUB: QTMovieStatusGetProperty called");
    return NULL;
}

void* QTMovieStatusGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTMovieStatusGetPropertyInfo called");
    return NULL;
}

void* QTMovieStatusRemovePropertyListener(void)
{
    if (verbose) puts("STUB: QTMovieStatusRemovePropertyListener called");
    return NULL;
}

void* QTMovieStatusSetDataContainerProperty(void)
{
    if (verbose) puts("STUB: QTMovieStatusSetDataContainerProperty called");
    return NULL;
}

void* QTMovieStatusSetProperty(void)
{
    if (verbose) puts("STUB: QTMovieStatusSetProperty called");
    return NULL;
}

void* QTMovieStatusTask(void)
{
    if (verbose) puts("STUB: QTMovieStatusTask called");
    return NULL;
}

void* QTMunger(void)
{
    if (verbose) puts("STUB: QTMunger called");
    return NULL;
}

void* QTNewAlias(void)
{
    if (verbose) puts("STUB: QTNewAlias called");
    return NULL;
}

void* QTNewAtomContainer(void)
{
    if (verbose) puts("STUB: QTNewAtomContainer called");
    return NULL;
}

void* QTNewDataReferenceFromCFURL(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromCFURL called");
    return NULL;
}

void* QTNewDataReferenceFromFSRef(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFSRef called");
    return NULL;
}

void* QTNewDataReferenceFromFSRefCFString(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFSRefCFString called");
    return NULL;
}

void* QTNewDataReferenceFromFSRefUnicode(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFSRefUnicode called");
    return NULL;
}

void* QTNewDataReferenceFromFSSpec(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFSSpec called");
    return NULL;
}

void* QTNewDataReferenceFromFullPath(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFullPath called");
    return NULL;
}

void* QTNewDataReferenceFromFullPathCFString(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFullPathCFString called");
    return NULL;
}

void* QTNewDataReferenceFromFullPathUnicode(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromFullPathUnicode called");
    return NULL;
}

void* QTNewDataReferenceFromRelativePath(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromRelativePath called");
    return NULL;
}

void* QTNewDataReferenceFromRelativePathCFString(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromRelativePathCFString called");
    return NULL;
}

void* QTNewDataReferenceFromRelativePathUnicode(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromRelativePathUnicode called");
    return NULL;
}

void* QTNewDataReferenceFromURL(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromURL called");
    return NULL;
}

void* QTNewDataReferenceFromURLCFString(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromURLCFString called");
    return NULL;
}

void* QTNewDataReferenceFromURLUnicode(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceFromURLUnicode called");
    return NULL;
}

void* QTNewDataReferenceWithDirectory(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceWithDirectory called");
    return NULL;
}

void* QTNewDataReferenceWithDirectoryCFString(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceWithDirectoryCFString called");
    return NULL;
}

void* QTNewDataReferenceWithDirectoryUnicode(void)
{
    if (verbose) puts("STUB: QTNewDataReferenceWithDirectoryUnicode called");
    return NULL;
}

void* QTNewEmptyHandle(void)
{
    if (verbose) puts("STUB: QTNewEmptyHandle called");
    return NULL;
}

void* QTNewGWorld(void)
{
    if (verbose) puts("STUB: QTNewGWorld called");
    return NULL;
}

void* QTNewGWorldFromPtr(void)
{
    if (verbose) puts("STUB: QTNewGWorldFromPtr called");
    return NULL;
}

void* QTNewHandle(void)
{
    if (verbose) puts("STUB: QTNewHandle called");
    return NULL;
}

void* QTNewHandleClear(void)
{
    if (verbose) puts("STUB: QTNewHandleClear called");
    return NULL;
}

void* QTNewObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTNewObjectSpecifier called");
    return NULL;
}

void* QTNewPtr(void)
{
    if (verbose) puts("STUB: QTNewPtr called");
    return NULL;
}

void* QTNewPtrClear(void)
{
    if (verbose) puts("STUB: QTNewPtrClear called");
    return NULL;
}

void* QTNewRegionForStorage(void)
{
    if (verbose) puts("STUB: QTNewRegionForStorage called");
    return NULL;
}

void* QTNewRegionFromStorage(void)
{
    if (verbose) puts("STUB: QTNewRegionFromStorage called");
    return NULL;
}

void* QTNewTween(void)
{
    if (verbose) puts("STUB: QTNewTween called");
    return NULL;
}

void* QTNewTypedHandle(void)
{
    if (verbose) puts("STUB: QTNewTypedHandle called");
    return NULL;
}

void* QTNewTypedHandleClear(void)
{
    if (verbose) puts("STUB: QTNewTypedHandleClear called");
    return NULL;
}

void* QTNextChildAnyType(void)
{
    if (verbose) puts("STUB: QTNextChildAnyType called");
    return NULL;
}

void* QTNotifyMovieHasClosed(void)
{
    if (verbose) puts("STUB: QTNotifyMovieHasClosed called");
    return NULL;
}

void* QTOpenControlPanel(void)
{
    if (verbose) puts("STUB: QTOpenControlPanel called");
    return NULL;
}

void* QTOpenGLTextureContextCreate(void)
{
    if (verbose) puts("STUB: QTOpenGLTextureContextCreate called");
    return NULL;
}

void* QTOpenSelection(void)
{
    if (verbose) puts("STUB: QTOpenSelection called");
    return NULL;
}

void* QTOpenURLInPlayer(void)
{
    if (verbose) puts("STUB: QTOpenURLInPlayer called");
    return NULL;
}

void* QTOpenUpdater(void)
{
    if (verbose) puts("STUB: QTOpenUpdater called");
    return NULL;
}

void* QTParseReferenceAtom(void)
{
    if (verbose) puts("STUB: QTParseReferenceAtom called");
    return NULL;
}

void* QTParseTextHREF(void)
{
    if (verbose) puts("STUB: QTParseTextHREF called");
    return NULL;
}

void* QTPhotoDefineHuffmanTable(void)
{
    if (verbose) puts("STUB: QTPhotoDefineHuffmanTable called");
    return NULL;
}

void* QTPhotoDefineQuantizationTable(void)
{
    if (verbose) puts("STUB: QTPhotoDefineQuantizationTable called");
    return NULL;
}

void* QTPhotoSetRestartInterval(void)
{
    if (verbose) puts("STUB: QTPhotoSetRestartInterval called");
    return NULL;
}

void* QTPhotoSetSampling(void)
{
    if (verbose) puts("STUB: QTPhotoSetSampling called");
    return NULL;
}

void* QTPixelBufferContextCreate(void)
{
    if (verbose) puts("STUB: QTPixelBufferContextCreate called");
    return NULL;
}

void* QTPostNotification(void)
{
    if (verbose) puts("STUB: QTPostNotification called");
    return NULL;
}

void* QTPostNotificationMessage(void)
{
    if (verbose) puts("STUB: QTPostNotificationMessage called");
    return NULL;
}

void* QTPreventDefaultMessageAction(void)
{
    if (verbose) puts("STUB: QTPreventDefaultMessageAction called");
    return NULL;
}

void* QTPrintAudioBufferList(void)
{
    if (verbose) puts("STUB: QTPrintAudioBufferList called");
    return NULL;
}

void* QTPrintAudioChannelLayout(void)
{
    if (verbose) puts("STUB: QTPrintAudioChannelLayout called");
    return NULL;
}

void* QTPrintAudioTimeStamp(void)
{
    if (verbose) puts("STUB: QTPrintAudioTimeStamp called");
    return NULL;
}

void* QTPrintBasicDescription(void)
{
    if (verbose) puts("STUB: QTPrintBasicDescription called");
    return NULL;
}

void* QTPrintHALDeviceDataSources(void)
{
    if (verbose) puts("STUB: QTPrintHALDeviceDataSources called");
    return NULL;
}

void* QTPrintMagicCookie(void)
{
    if (verbose) puts("STUB: QTPrintMagicCookie called");
    return NULL;
}

void* QTPrintStreamBasicDescriptionWithFormat(void)
{
    if (verbose) puts("STUB: QTPrintStreamBasicDescriptionWithFormat called");
    return NULL;
}

void* QTPtrAndHand(void)
{
    if (verbose) puts("STUB: QTPtrAndHand called");
    return NULL;
}

void* QTPtrToHand(void)
{
    if (verbose) puts("STUB: QTPtrToHand called");
    return NULL;
}

void* QTPtrToXHand(void)
{
    if (verbose) puts("STUB: QTPtrToXHand called");
    return NULL;
}

void* QTReEncodeTextBuff(void)
{
    if (verbose) puts("STUB: QTReEncodeTextBuff called");
    return NULL;
}

void* QTReEncodeTextHandle(void)
{
    if (verbose) puts("STUB: QTReEncodeTextHandle called");
    return NULL;
}

void* QTReEncodeTextHandleFromNative(void)
{
    if (verbose) puts("STUB: QTReEncodeTextHandleFromNative called");
    return NULL;
}

void* QTReEncodeTextHandleToNative(void)
{
    if (verbose) puts("STUB: QTReEncodeTextHandleToNative called");
    return NULL;
}

void* QTReEncodeTextHandleToNativeUsingCF(void)
{
    if (verbose) puts("STUB: QTReEncodeTextHandleToNativeUsingCF called");
    return NULL;
}

void* QTReallocateHandle(void)
{
    if (verbose) puts("STUB: QTReallocateHandle called");
    return NULL;
}

void* QTRecoverHandle(void)
{
    if (verbose) puts("STUB: QTRecoverHandle called");
    return NULL;
}

void* QTRegisterAccessKey(void)
{
    if (verbose) puts("STUB: QTRegisterAccessKey called");
    return NULL;
}

void* QTRegisterCooperativeThread(void)
{
    if (verbose) puts("STUB: QTRegisterCooperativeThread called");
    return NULL;
}

void* QTRegisterThreadWithGarbageCollector(void)
{
    if (verbose) puts("STUB: QTRegisterThreadWithGarbageCollector called");
    return NULL;
}

void* QTReleaseLicensedTechnology(void)
{
    if (verbose) puts("STUB: QTReleaseLicensedTechnology called");
    return NULL;
}

void* QTReleaseObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTReleaseObjectSpecifier called");
    return NULL;
}

void* QTReleaseToolboxPerThreadStorage(void)
{
    if (verbose) puts("STUB: QTReleaseToolboxPerThreadStorage called");
    return NULL;
}

void* QTRemoveAtom(void)
{
    if (verbose) puts("STUB: QTRemoveAtom called");
    return NULL;
}

void* QTRemoveChildren(void)
{
    if (verbose) puts("STUB: QTRemoveChildren called");
    return NULL;
}

void* QTRemoveComponentPropertyListener(void)
{
    if (verbose) puts("STUB: QTRemoveComponentPropertyListener called");
    return NULL;
}

void* QTRemoveFrequentTimer(void)
{
    if (verbose) puts("STUB: QTRemoveFrequentTimer called");
    return NULL;
}

void* QTRemoveMessageHandler(void)
{
    if (verbose) puts("STUB: QTRemoveMessageHandler called");
    return NULL;
}

void* QTRemoveMoviePropertyListener(void)
{
    if (verbose) puts("STUB: QTRemoveMoviePropertyListener called");
    return NULL;
}

void* QTRemoveProcessPropertyListener(void)
{
    if (verbose) puts("STUB: QTRemoveProcessPropertyListener called");
    return NULL;
}

void* QTRemoveStashedData(void)
{
    if (verbose) puts("STUB: QTRemoveStashedData called");
    return NULL;
}

void* QTRemoveTrackPropertyListener(void)
{
    if (verbose) puts("STUB: QTRemoveTrackPropertyListener called");
    return NULL;
}

void* QTReplaceAtom(void)
{
    if (verbose) puts("STUB: QTReplaceAtom called");
    return NULL;
}

void* QTReportMissingSoftware(void)
{
    if (verbose) puts("STUB: QTReportMissingSoftware called");
    return NULL;
}

void* QTRequestLicensedTechnology(void)
{
    if (verbose) puts("STUB: QTRequestLicensedTechnology called");
    return NULL;
}

void* QTResolveObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTResolveObjectSpecifier called");
    return NULL;
}

void* QTRestrictionsGetIndClass(void)
{
    if (verbose) puts("STUB: QTRestrictionsGetIndClass called");
    return NULL;
}

void* QTRestrictionsGetInfo(void)
{
    if (verbose) puts("STUB: QTRestrictionsGetInfo called");
    return NULL;
}

void* QTRestrictionsGetItem(void)
{
    if (verbose) puts("STUB: QTRestrictionsGetItem called");
    return NULL;
}

void* QTRestrictionsSetItem(void)
{
    if (verbose) puts("STUB: QTRestrictionsSetItem called");
    return NULL;
}

void* QTRetainObjectSpecifier(void)
{
    if (verbose) puts("STUB: QTRetainObjectSpecifier called");
    return NULL;
}

void* QTSAllocBuffer(void)
{
    if (verbose) puts("STUB: QTSAllocBuffer called");
    return NULL;
}

void* QTSAllocMemPtr(void)
{
    if (verbose) puts("STUB: QTSAllocMemPtr called");
    return NULL;
}

void* QTSAutoConfigCancel(void)
{
    if (verbose) puts("STUB: QTSAutoConfigCancel called");
    return NULL;
}

void* QTSAutoConfigConnection(void)
{
    if (verbose) puts("STUB: QTSAutoConfigConnection called");
    return NULL;
}

void* QTSAutoConfigIdle(void)
{
    if (verbose) puts("STUB: QTSAutoConfigIdle called");
    return NULL;
}

void* QTSBandwidthCacheDispose(void)
{
    if (verbose) puts("STUB: QTSBandwidthCacheDispose called");
    return NULL;
}

void* QTSBandwidthCacheInitialize(void)
{
    if (verbose) puts("STUB: QTSBandwidthCacheInitialize called");
    return NULL;
}

void* QTSBandwidthCacheLookup(void)
{
    if (verbose) puts("STUB: QTSBandwidthCacheLookup called");
    return NULL;
}

void* QTSBandwidthCacheSave(void)
{
    if (verbose) puts("STUB: QTSBandwidthCacheSave called");
    return NULL;
}

void* QTSBandwidthCacheUpdate(void)
{
    if (verbose) puts("STUB: QTSBandwidthCacheUpdate called");
    return NULL;
}

void* QTSBase64Decode(void)
{
    if (verbose) puts("STUB: QTSBase64Decode called");
    return NULL;
}

void* QTSBase64DecodeToBuffer(void)
{
    if (verbose) puts("STUB: QTSBase64DecodeToBuffer called");
    return NULL;
}

void* QTSBase64Encode(void)
{
    if (verbose) puts("STUB: QTSBase64Encode called");
    return NULL;
}

void* QTSBase64EncodeToBuffer(void)
{
    if (verbose) puts("STUB: QTSBase64EncodeToBuffer called");
    return NULL;
}

void* QTSCondAboutToClose(void)
{
    if (verbose) puts("STUB: QTSCondAboutToClose called");
    return NULL;
}

void* QTSCondAddLeafSettings(void)
{
    if (verbose) puts("STUB: QTSCondAddLeafSettings called");
    return NULL;
}

void* QTSCondAddNextConductor(void)
{
    if (verbose) puts("STUB: QTSCondAddNextConductor called");
    return NULL;
}

void* QTSCondAddSourcer(void)
{
    if (verbose) puts("STUB: QTSCondAddSourcer called");
    return NULL;
}

void* QTSCondDisposeStream(void)
{
    if (verbose) puts("STUB: QTSCondDisposeStream called");
    return NULL;
}

void* QTSCondGetActiveSegment(void)
{
    if (verbose) puts("STUB: QTSCondGetActiveSegment called");
    return NULL;
}

void* QTSCondGetClip(void)
{
    if (verbose) puts("STUB: QTSCondGetClip called");
    return NULL;
}

void* QTSCondGetDimensions(void)
{
    if (verbose) puts("STUB: QTSCondGetDimensions called");
    return NULL;
}

void* QTSCondGetEnable(void)
{
    if (verbose) puts("STUB: QTSCondGetEnable called");
    return NULL;
}

void* QTSCondGetFlags(void)
{
    if (verbose) puts("STUB: QTSCondGetFlags called");
    return NULL;
}

void* QTSCondGetGWorld(void)
{
    if (verbose) puts("STUB: QTSCondGetGWorld called");
    return NULL;
}

void* QTSCondGetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSCondGetGraphicsMode called");
    return NULL;
}

void* QTSCondGetIndSourcer(void)
{
    if (verbose) puts("STUB: QTSCondGetIndSourcer called");
    return NULL;
}

void* QTSCondGetIndStream(void)
{
    if (verbose) puts("STUB: QTSCondGetIndStream called");
    return NULL;
}

void* QTSCondGetInfo(void)
{
    if (verbose) puts("STUB: QTSCondGetInfo called");
    return NULL;
}

void* QTSCondGetMatrix(void)
{
    if (verbose) puts("STUB: QTSCondGetMatrix called");
    return NULL;
}

void* QTSCondGetNextIndConductor(void)
{
    if (verbose) puts("STUB: QTSCondGetNextIndConductor called");
    return NULL;
}

void* QTSCondGetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSCondGetNotificationProc called");
    return NULL;
}

void* QTSCondGetNumSourcers(void)
{
    if (verbose) puts("STUB: QTSCondGetNumSourcers called");
    return NULL;
}

void* QTSCondGetNumStreams(void)
{
    if (verbose) puts("STUB: QTSCondGetNumStreams called");
    return NULL;
}

void* QTSCondGetPicture(void)
{
    if (verbose) puts("STUB: QTSCondGetPicture called");
    return NULL;
}

void* QTSCondGetPlayHints(void)
{
    if (verbose) puts("STUB: QTSCondGetPlayHints called");
    return NULL;
}

void* QTSCondGetPreferredRate(void)
{
    if (verbose) puts("STUB: QTSCondGetPreferredRate called");
    return NULL;
}

void* QTSCondGetPresenting(void)
{
    if (verbose) puts("STUB: QTSCondGetPresenting called");
    return NULL;
}

void* QTSCondGetProperty(void)
{
    if (verbose) puts("STUB: QTSCondGetProperty called");
    return NULL;
}

void* QTSCondGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSCondGetPropertyInfo called");
    return NULL;
}

void* QTSCondGetSettings(void)
{
    if (verbose) puts("STUB: QTSCondGetSettings called");
    return NULL;
}

void* QTSCondGetSettingsAsText(void)
{
    if (verbose) puts("STUB: QTSCondGetSettingsAsText called");
    return NULL;
}

void* QTSCondGetStreamMediaComponent(void)
{
    if (verbose) puts("STUB: QTSCondGetStreamMediaComponent called");
    return NULL;
}

void* QTSCondGetTimeBase(void)
{
    if (verbose) puts("STUB: QTSCondGetTimeBase called");
    return NULL;
}

void* QTSCondGetTimeScale(void)
{
    if (verbose) puts("STUB: QTSCondGetTimeScale called");
    return NULL;
}

void* QTSCondGetVisualContext(void)
{
    if (verbose) puts("STUB: QTSCondGetVisualContext called");
    return NULL;
}

void* QTSCondGetVolumes(void)
{
    if (verbose) puts("STUB: QTSCondGetVolumes called");
    return NULL;
}

void* QTSCondHasCharacteristic(void)
{
    if (verbose) puts("STUB: QTSCondHasCharacteristic called");
    return NULL;
}

void* QTSCondIdle(void)
{
    if (verbose) puts("STUB: QTSCondIdle called");
    return NULL;
}

void* QTSCondInitialize(void)
{
    if (verbose) puts("STUB: QTSCondInitialize called");
    return NULL;
}

void* QTSCondInvalidateRegion(void)
{
    if (verbose) puts("STUB: QTSCondInvalidateRegion called");
    return NULL;
}

void* QTSCondNewStream(void)
{
    if (verbose) puts("STUB: QTSCondNewStream called");
    return NULL;
}

void* QTSCondPreroll(void)
{
    if (verbose) puts("STUB: QTSCondPreroll called");
    return NULL;
}

void* QTSCondPreview(void)
{
    if (verbose) puts("STUB: QTSCondPreview called");
    return NULL;
}

void* QTSCondProcessNotification(void)
{
    if (verbose) puts("STUB: QTSCondProcessNotification called");
    return NULL;
}

void* QTSCondRemoveNextConductor(void)
{
    if (verbose) puts("STUB: QTSCondRemoveNextConductor called");
    return NULL;
}

void* QTSCondRemoveSourcer(void)
{
    if (verbose) puts("STUB: QTSCondRemoveSourcer called");
    return NULL;
}

void* QTSCondReset(void)
{
    if (verbose) puts("STUB: QTSCondReset called");
    return NULL;
}

void* QTSCondSetActiveSegment(void)
{
    if (verbose) puts("STUB: QTSCondSetActiveSegment called");
    return NULL;
}

void* QTSCondSetClip(void)
{
    if (verbose) puts("STUB: QTSCondSetClip called");
    return NULL;
}

void* QTSCondSetDimensions(void)
{
    if (verbose) puts("STUB: QTSCondSetDimensions called");
    return NULL;
}

void* QTSCondSetEnable(void)
{
    if (verbose) puts("STUB: QTSCondSetEnable called");
    return NULL;
}

void* QTSCondSetFlags(void)
{
    if (verbose) puts("STUB: QTSCondSetFlags called");
    return NULL;
}

void* QTSCondSetGWorld(void)
{
    if (verbose) puts("STUB: QTSCondSetGWorld called");
    return NULL;
}

void* QTSCondSetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSCondSetGraphicsMode called");
    return NULL;
}

void* QTSCondSetInfo(void)
{
    if (verbose) puts("STUB: QTSCondSetInfo called");
    return NULL;
}

void* QTSCondSetLeafSettings(void)
{
    if (verbose) puts("STUB: QTSCondSetLeafSettings called");
    return NULL;
}

void* QTSCondSetMatrix(void)
{
    if (verbose) puts("STUB: QTSCondSetMatrix called");
    return NULL;
}

void* QTSCondSetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSCondSetNotificationProc called");
    return NULL;
}

void* QTSCondSetPlayHints(void)
{
    if (verbose) puts("STUB: QTSCondSetPlayHints called");
    return NULL;
}

void* QTSCondSetPreferredRate(void)
{
    if (verbose) puts("STUB: QTSCondSetPreferredRate called");
    return NULL;
}

void* QTSCondSetPresenting(void)
{
    if (verbose) puts("STUB: QTSCondSetPresenting called");
    return NULL;
}

void* QTSCondSetProperty(void)
{
    if (verbose) puts("STUB: QTSCondSetProperty called");
    return NULL;
}

void* QTSCondSetRate(void)
{
    if (verbose) puts("STUB: QTSCondSetRate called");
    return NULL;
}

void* QTSCondSetSettings(void)
{
    if (verbose) puts("STUB: QTSCondSetSettings called");
    return NULL;
}

void* QTSCondSetTimeBase(void)
{
    if (verbose) puts("STUB: QTSCondSetTimeBase called");
    return NULL;
}

void* QTSCondSetTimeScale(void)
{
    if (verbose) puts("STUB: QTSCondSetTimeScale called");
    return NULL;
}

void* QTSCondSetVisualContext(void)
{
    if (verbose) puts("STUB: QTSCondSetVisualContext called");
    return NULL;
}

void* QTSCondSetVolumes(void)
{
    if (verbose) puts("STUB: QTSCondSetVolumes called");
    return NULL;
}

void* QTSCondSettingsDialog(void)
{
    if (verbose) puts("STUB: QTSCondSettingsDialog called");
    return NULL;
}

void* QTSCondSettingsDialogGetPanelList(void)
{
    if (verbose) puts("STUB: QTSCondSettingsDialogGetPanelList called");
    return NULL;
}

void* QTSCondStart(void)
{
    if (verbose) puts("STUB: QTSCondStart called");
    return NULL;
}

void* QTSCondStop(void)
{
    if (verbose) puts("STUB: QTSCondStop called");
    return NULL;
}

void* QTSConvertBufferToUTF8(void)
{
    if (verbose) puts("STUB: QTSConvertBufferToUTF8 called");
    return NULL;
}

void* QTSConvertUTF8ToBuffer(void)
{
    if (verbose) puts("STUB: QTSConvertUTF8ToBuffer called");
    return NULL;
}

void* QTSCopyMessage(void)
{
    if (verbose) puts("STUB: QTSCopyMessage called");
    return NULL;
}

void* QTSDisposePresentation(void)
{
    if (verbose) puts("STUB: QTSDisposePresentation called");
    return NULL;
}

void* QTSDisposeStatHelper(void)
{
    if (verbose) puts("STUB: QTSDisposeStatHelper called");
    return NULL;
}

void* QTSDisposeStream(void)
{
    if (verbose) puts("STUB: QTSDisposeStream called");
    return NULL;
}

void* QTSDupMessage(void)
{
    if (verbose) puts("STUB: QTSDupMessage called");
    return NULL;
}

void* QTSDuplicateMessage(void)
{
    if (verbose) puts("STUB: QTSDuplicateMessage called");
    return NULL;
}

void* QTSExporterExport(void)
{
    if (verbose) puts("STUB: QTSExporterExport called");
    return NULL;
}

void* QTSExporterGetInfo(void)
{
    if (verbose) puts("STUB: QTSExporterGetInfo called");
    return NULL;
}

void* QTSExporterSetInfo(void)
{
    if (verbose) puts("STUB: QTSExporterSetInfo called");
    return NULL;
}

void* QTSFindMediaPacketizer(void)
{
    if (verbose) puts("STUB: QTSFindMediaPacketizer called");
    return NULL;
}

void* QTSFindMediaPacketizerForPayloadID(void)
{
    if (verbose) puts("STUB: QTSFindMediaPacketizerForPayloadID called");
    return NULL;
}

void* QTSFindMediaPacketizerForPayloadName(void)
{
    if (verbose) puts("STUB: QTSFindMediaPacketizerForPayloadName called");
    return NULL;
}

void* QTSFindMediaPacketizerForTrack(void)
{
    if (verbose) puts("STUB: QTSFindMediaPacketizerForTrack called");
    return NULL;
}

void* QTSFindNetworkComponent(void)
{
    if (verbose) puts("STUB: QTSFindNetworkComponent called");
    return NULL;
}

void* QTSFindReassemblerForPayloadID(void)
{
    if (verbose) puts("STUB: QTSFindReassemblerForPayloadID called");
    return NULL;
}

void* QTSFindReassemblerForPayloadName(void)
{
    if (verbose) puts("STUB: QTSFindReassemblerForPayloadName called");
    return NULL;
}

void* QTSFlattenMessage(void)
{
    if (verbose) puts("STUB: QTSFlattenMessage called");
    return NULL;
}

void* QTSFreeMessage(void)
{
    if (verbose) puts("STUB: QTSFreeMessage called");
    return NULL;
}

void* QTSGetBase64MaxDecodeLength(void)
{
    if (verbose) puts("STUB: QTSGetBase64MaxDecodeLength called");
    return NULL;
}

void* QTSGetBase64MaxEncodeLength(void)
{
    if (verbose) puts("STUB: QTSGetBase64MaxEncodeLength called");
    return NULL;
}

void* QTSGetConductorPresentation(void)
{
    if (verbose) puts("STUB: QTSGetConductorPresentation called");
    return NULL;
}

void* QTSGetErrorString(void)
{
    if (verbose) puts("STUB: QTSGetErrorString called");
    return NULL;
}

void* QTSGetLocalUniqueID(void)
{
    if (verbose) puts("STUB: QTSGetLocalUniqueID called");
    return NULL;
}

void* QTSGetNetworkAppName(void)
{
    if (verbose) puts("STUB: QTSGetNetworkAppName called");
    return NULL;
}

void* QTSGetOrMakeStatAtomForStream(void)
{
    if (verbose) puts("STUB: QTSGetOrMakeStatAtomForStream called");
    return NULL;
}

void* QTSGetQTSUserAgentExtraTags(void)
{
    if (verbose) puts("STUB: QTSGetQTSUserAgentExtraTags called");
    return NULL;
}

void* QTSGetQTSVersionString(void)
{
    if (verbose) puts("STUB: QTSGetQTSVersionString called");
    return NULL;
}

void* QTSGetStreamMediaComponent(void)
{
    if (verbose) puts("STUB: QTSGetStreamMediaComponent called");
    return NULL;
}

void* QTSGetStreamPresentation(void)
{
    if (verbose) puts("STUB: QTSGetStreamPresentation called");
    return NULL;
}

void* QTSGetTaggedHandle(void)
{
    if (verbose) puts("STUB: QTSGetTaggedHandle called");
    return NULL;
}

void* QTSImporterGetInfo(void)
{
    if (verbose) puts("STUB: QTSImporterGetInfo called");
    return NULL;
}

void* QTSImporterNewAtomContainerFromData(void)
{
    if (verbose) puts("STUB: QTSImporterNewAtomContainerFromData called");
    return NULL;
}

void* QTSImporterNewAtomContainerFromDataRef(void)
{
    if (verbose) puts("STUB: QTSImporterNewAtomContainerFromDataRef called");
    return NULL;
}

void* QTSImporterNewAtomContainerFromFile(void)
{
    if (verbose) puts("STUB: QTSImporterNewAtomContainerFromFile called");
    return NULL;
}

void* QTSImporterSetInfo(void)
{
    if (verbose) puts("STUB: QTSImporterSetInfo called");
    return NULL;
}

void* QTSInitializeMediaParams(void)
{
    if (verbose) puts("STUB: QTSInitializeMediaParams called");
    return NULL;
}

void* QTSInsertStatistic(void)
{
    if (verbose) puts("STUB: QTSInsertStatistic called");
    return NULL;
}

void* QTSInsertStatisticName(void)
{
    if (verbose) puts("STUB: QTSInsertStatisticName called");
    return NULL;
}

void* QTSInsertStatisticUnits(void)
{
    if (verbose) puts("STUB: QTSInsertStatisticUnits called");
    return NULL;
}

void* QTSLanguageTagStringToRegionTag(void)
{
    if (verbose) puts("STUB: QTSLanguageTagStringToRegionTag called");
    return NULL;
}

void* QTSLogCopyGlobalLog(void)
{
    if (verbose) puts("STUB: QTSLogCopyGlobalLog called");
    return NULL;
}

void* QTSLogDisposeLog(void)
{
    if (verbose) puts("STUB: QTSLogDisposeLog called");
    return NULL;
}

void* QTSLogFlush(void)
{
    if (verbose) puts("STUB: QTSLogFlush called");
    return NULL;
}

void* QTSLogGetContentFlags(void)
{
    if (verbose) puts("STUB: QTSLogGetContentFlags called");
    return NULL;
}

void* QTSLogGetContentType(void)
{
    if (verbose) puts("STUB: QTSLogGetContentType called");
    return NULL;
}

void* QTSLogGetRefCount(void)
{
    if (verbose) puts("STUB: QTSLogGetRefCount called");
    return NULL;
}

void* QTSLogNewLog(void)
{
    if (verbose) puts("STUB: QTSLogNewLog called");
    return NULL;
}

void* QTSLogNewLogForInstance(void)
{
    if (verbose) puts("STUB: QTSLogNewLogForInstance called");
    return NULL;
}

void* QTSLogNewLogFromSpec(void)
{
    if (verbose) puts("STUB: QTSLogNewLogFromSpec called");
    return NULL;
}

void* QTSLogPrintf(void)
{
    if (verbose) puts("STUB: QTSLogPrintf called");
    return NULL;
}

void* QTSLogRelease(void)
{
    if (verbose) puts("STUB: QTSLogRelease called");
    return NULL;
}

void* QTSLogRetain(void)
{
    if (verbose) puts("STUB: QTSLogRetain called");
    return NULL;
}

void* QTSLogSetContentFlags(void)
{
    if (verbose) puts("STUB: QTSLogSetContentFlags called");
    return NULL;
}

void* QTSLogSetContentType(void)
{
    if (verbose) puts("STUB: QTSLogSetContentType called");
    return NULL;
}

void* QTSLogSetMaxBufferSize(void)
{
    if (verbose) puts("STUB: QTSLogSetMaxBufferSize called");
    return NULL;
}

void* QTSLogWriteChars(void)
{
    if (verbose) puts("STUB: QTSLogWriteChars called");
    return NULL;
}

void* QTSLogWriteEOL(void)
{
    if (verbose) puts("STUB: QTSLogWriteEOL called");
    return NULL;
}

void* QTSLogWriteHexData(void)
{
    if (verbose) puts("STUB: QTSLogWriteHexData called");
    return NULL;
}

void* QTSLogWriteNum(void)
{
    if (verbose) puts("STUB: QTSLogWriteNum called");
    return NULL;
}

void* QTSLogWritePString(void)
{
    if (verbose) puts("STUB: QTSLogWritePString called");
    return NULL;
}

void* QTSLogWriteString(void)
{
    if (verbose) puts("STUB: QTSLogWriteString called");
    return NULL;
}

void* QTSLogWriteTime(void)
{
    if (verbose) puts("STUB: QTSLogWriteTime called");
    return NULL;
}

void* QTSLogWriteToDiskIfBufferFull(void)
{
    if (verbose) puts("STUB: QTSLogWriteToDiskIfBufferFull called");
    return NULL;
}

void* QTSMediaGetIndStreamInfo(void)
{
    if (verbose) puts("STUB: QTSMediaGetIndStreamInfo called");
    return NULL;
}

void* QTSMediaGetInfo(void)
{
    if (verbose) puts("STUB: QTSMediaGetInfo called");
    return NULL;
}

void* QTSMediaSetIndStreamInfo(void)
{
    if (verbose) puts("STUB: QTSMediaSetIndStreamInfo called");
    return NULL;
}

void* QTSMediaSetInfo(void)
{
    if (verbose) puts("STUB: QTSMediaSetInfo called");
    return NULL;
}

void* QTSMemAllocGetInfo(void)
{
    if (verbose) puts("STUB: QTSMemAllocGetInfo called");
    return NULL;
}

void* QTSMemAllocSetHints(void)
{
    if (verbose) puts("STUB: QTSMemAllocSetHints called");
    return NULL;
}

void* QTSMemAllocSetInfo(void)
{
    if (verbose) puts("STUB: QTSMemAllocSetInfo called");
    return NULL;
}

void* QTSMessageLength(void)
{
    if (verbose) puts("STUB: QTSMessageLength called");
    return NULL;
}

void* QTSModCreateStream(void)
{
    if (verbose) puts("STUB: QTSModCreateStream called");
    return NULL;
}

void* QTSModDestroyStream(void)
{
    if (verbose) puts("STUB: QTSModDestroyStream called");
    return NULL;
}

void* QTSModGetComponent(void)
{
    if (verbose) puts("STUB: QTSModGetComponent called");
    return NULL;
}

void* QTSModGetLinkEndRefCon(void)
{
    if (verbose) puts("STUB: QTSModGetLinkEndRefCon called");
    return NULL;
}

void* QTSModGetNextLinkEnd(void)
{
    if (verbose) puts("STUB: QTSModGetNextLinkEnd called");
    return NULL;
}

void* QTSModGetNumLinkEnds(void)
{
    if (verbose) puts("STUB: QTSModGetNumLinkEnds called");
    return NULL;
}

void* QTSModGetOtherLinkEnd(void)
{
    if (verbose) puts("STUB: QTSModGetOtherLinkEnd called");
    return NULL;
}

void* QTSModIdleStream(void)
{
    if (verbose) puts("STUB: QTSModIdleStream called");
    return NULL;
}

void* QTSModInsertModule(void)
{
    if (verbose) puts("STUB: QTSModInsertModule called");
    return NULL;
}

void* QTSModRemoveModule(void)
{
    if (verbose) puts("STUB: QTSModRemoveModule called");
    return NULL;
}

void* QTSModSendControl(void)
{
    if (verbose) puts("STUB: QTSModSendControl called");
    return NULL;
}

void* QTSModSendControlToLinkEndType(void)
{
    if (verbose) puts("STUB: QTSModSendControlToLinkEndType called");
    return NULL;
}

void* QTSModSendData(void)
{
    if (verbose) puts("STUB: QTSModSendData called");
    return NULL;
}

void* QTSModSendDataToLinkEndType(void)
{
    if (verbose) puts("STUB: QTSModSendDataToLinkEndType called");
    return NULL;
}

void* QTSModSetLinkEndRefCon(void)
{
    if (verbose) puts("STUB: QTSModSetLinkEndRefCon called");
    return NULL;
}

void* QTSNetworkGetInfo(void)
{
    if (verbose) puts("STUB: QTSNetworkGetInfo called");
    return NULL;
}

void* QTSNetworkGetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSNetworkGetNotificationProc called");
    return NULL;
}

void* QTSNetworkIdle(void)
{
    if (verbose) puts("STUB: QTSNetworkIdle called");
    return NULL;
}

void* QTSNetworkInitialize(void)
{
    if (verbose) puts("STUB: QTSNetworkInitialize called");
    return NULL;
}

void* QTSNetworkLookupAddress(void)
{
    if (verbose) puts("STUB: QTSNetworkLookupAddress called");
    return NULL;
}

void* QTSNetworkLookupLocal(void)
{
    if (verbose) puts("STUB: QTSNetworkLookupLocal called");
    return NULL;
}

void* QTSNetworkReconnectLocal(void)
{
    if (verbose) puts("STUB: QTSNetworkReconnectLocal called");
    return NULL;
}

void* QTSNetworkReconnectRemote(void)
{
    if (verbose) puts("STUB: QTSNetworkReconnectRemote called");
    return NULL;
}

void* QTSNetworkSendData(void)
{
    if (verbose) puts("STUB: QTSNetworkSendData called");
    return NULL;
}

void* QTSNetworkSetInfo(void)
{
    if (verbose) puts("STUB: QTSNetworkSetInfo called");
    return NULL;
}

void* QTSNetworkSetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSNetworkSetNotificationProc called");
    return NULL;
}

void* QTSNetworkTerminate(void)
{
    if (verbose) puts("STUB: QTSNetworkTerminate called");
    return NULL;
}

void* QTSNetworkThreadAddCallback(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadAddCallback called");
    return NULL;
}

void* QTSNetworkThreadCancelCallback(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadCancelCallback called");
    return NULL;
}

void* QTSNetworkThreadCreate(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadCreate called");
    return NULL;
}

void* QTSNetworkThreadGetProperty(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadGetProperty called");
    return NULL;
}

void* QTSNetworkThreadRelease(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadRelease called");
    return NULL;
}

void* QTSNetworkThreadRetain(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadRetain called");
    return NULL;
}

void* QTSNetworkThreadSetProperty(void)
{
    if (verbose) puts("STUB: QTSNetworkThreadSetProperty called");
    return NULL;
}

void* QTSNewHandle(void)
{
    if (verbose) puts("STUB: QTSNewHandle called");
    return NULL;
}

void* QTSNewPresentation(void)
{
    if (verbose) puts("STUB: QTSNewPresentation called");
    return NULL;
}

void* QTSNewPresentationFromAtomContainer(void)
{
    if (verbose) puts("STUB: QTSNewPresentationFromAtomContainer called");
    return NULL;
}

void* QTSNewPresentationFromData(void)
{
    if (verbose) puts("STUB: QTSNewPresentationFromData called");
    return NULL;
}

void* QTSNewPresentationFromDataRef(void)
{
    if (verbose) puts("STUB: QTSNewPresentationFromDataRef called");
    return NULL;
}

void* QTSNewPresentationFromFile(void)
{
    if (verbose) puts("STUB: QTSNewPresentationFromFile called");
    return NULL;
}

void* QTSNewPtr(void)
{
    if (verbose) puts("STUB: QTSNewPtr called");
    return NULL;
}

void* QTSNewSourcer(void)
{
    if (verbose) puts("STUB: QTSNewSourcer called");
    return NULL;
}

void* QTSNewStatHelper(void)
{
    if (verbose) puts("STUB: QTSNewStatHelper called");
    return NULL;
}

void* QTSNewStreamBuffer(void)
{
    if (verbose) puts("STUB: QTSNewStreamBuffer called");
    return NULL;
}

void* QTSPrefsAddConnectionSetting(void)
{
    if (verbose) puts("STUB: QTSPrefsAddConnectionSetting called");
    return NULL;
}

void* QTSPrefsAddProxySetting(void)
{
    if (verbose) puts("STUB: QTSPrefsAddProxySetting called");
    return NULL;
}

void* QTSPrefsAddProxyUserInfo(void)
{
    if (verbose) puts("STUB: QTSPrefsAddProxyUserInfo called");
    return NULL;
}

void* QTSPrefsFindConnectionByType(void)
{
    if (verbose) puts("STUB: QTSPrefsFindConnectionByType called");
    return NULL;
}

void* QTSPrefsFindProxyByType(void)
{
    if (verbose) puts("STUB: QTSPrefsFindProxyByType called");
    return NULL;
}

void* QTSPrefsFindProxyUserInfoByType(void)
{
    if (verbose) puts("STUB: QTSPrefsFindProxyUserInfoByType called");
    return NULL;
}

void* QTSPrefsGetActiveConnection(void)
{
    if (verbose) puts("STUB: QTSPrefsGetActiveConnection called");
    return NULL;
}

void* QTSPrefsGetInstantOnSettings(void)
{
    if (verbose) puts("STUB: QTSPrefsGetInstantOnSettings called");
    return NULL;
}

void* QTSPrefsGetNoProxyURLs(void)
{
    if (verbose) puts("STUB: QTSPrefsGetNoProxyURLs called");
    return NULL;
}

void* QTSPrefsReset(void)
{
    if (verbose) puts("STUB: QTSPrefsReset called");
    return NULL;
}

void* QTSPrefsSetActiveConnection(void)
{
    if (verbose) puts("STUB: QTSPrefsSetActiveConnection called");
    return NULL;
}

void* QTSPrefsSetInstantOnSettings(void)
{
    if (verbose) puts("STUB: QTSPrefsSetInstantOnSettings called");
    return NULL;
}

void* QTSPrefsSetNoProxyURLs(void)
{
    if (verbose) puts("STUB: QTSPrefsSetNoProxyURLs called");
    return NULL;
}

void* QTSPresAddSourcer(void)
{
    if (verbose) puts("STUB: QTSPresAddSourcer called");
    return NULL;
}

void* QTSPresExport(void)
{
    if (verbose) puts("STUB: QTSPresExport called");
    return NULL;
}

void* QTSPresGetActiveSegment(void)
{
    if (verbose) puts("STUB: QTSPresGetActiveSegment called");
    return NULL;
}

void* QTSPresGetClip(void)
{
    if (verbose) puts("STUB: QTSPresGetClip called");
    return NULL;
}

void* QTSPresGetConductor(void)
{
    if (verbose) puts("STUB: QTSPresGetConductor called");
    return NULL;
}

void* QTSPresGetDimensions(void)
{
    if (verbose) puts("STUB: QTSPresGetDimensions called");
    return NULL;
}

void* QTSPresGetEnable(void)
{
    if (verbose) puts("STUB: QTSPresGetEnable called");
    return NULL;
}

void* QTSPresGetFlags(void)
{
    if (verbose) puts("STUB: QTSPresGetFlags called");
    return NULL;
}

void* QTSPresGetGWorld(void)
{
    if (verbose) puts("STUB: QTSPresGetGWorld called");
    return NULL;
}

void* QTSPresGetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSPresGetGraphicsMode called");
    return NULL;
}

void* QTSPresGetIndSourcer(void)
{
    if (verbose) puts("STUB: QTSPresGetIndSourcer called");
    return NULL;
}

void* QTSPresGetIndStream(void)
{
    if (verbose) puts("STUB: QTSPresGetIndStream called");
    return NULL;
}

void* QTSPresGetInfo(void)
{
    if (verbose) puts("STUB: QTSPresGetInfo called");
    return NULL;
}

void* QTSPresGetMatrix(void)
{
    if (verbose) puts("STUB: QTSPresGetMatrix called");
    return NULL;
}

void* QTSPresGetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSPresGetNotificationProc called");
    return NULL;
}

void* QTSPresGetNumSourcers(void)
{
    if (verbose) puts("STUB: QTSPresGetNumSourcers called");
    return NULL;
}

void* QTSPresGetNumStreams(void)
{
    if (verbose) puts("STUB: QTSPresGetNumStreams called");
    return NULL;
}

void* QTSPresGetPicture(void)
{
    if (verbose) puts("STUB: QTSPresGetPicture called");
    return NULL;
}

void* QTSPresGetPlayHints(void)
{
    if (verbose) puts("STUB: QTSPresGetPlayHints called");
    return NULL;
}

void* QTSPresGetPreferredRate(void)
{
    if (verbose) puts("STUB: QTSPresGetPreferredRate called");
    return NULL;
}

void* QTSPresGetPresenting(void)
{
    if (verbose) puts("STUB: QTSPresGetPresenting called");
    return NULL;
}

void* QTSPresGetProperty(void)
{
    if (verbose) puts("STUB: QTSPresGetProperty called");
    return NULL;
}

void* QTSPresGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSPresGetPropertyInfo called");
    return NULL;
}

void* QTSPresGetSettings(void)
{
    if (verbose) puts("STUB: QTSPresGetSettings called");
    return NULL;
}

void* QTSPresGetSettingsAsText(void)
{
    if (verbose) puts("STUB: QTSPresGetSettingsAsText called");
    return NULL;
}

void* QTSPresGetStatusHandler(void)
{
    if (verbose) puts("STUB: QTSPresGetStatusHandler called");
    return NULL;
}

void* QTSPresGetTimeBase(void)
{
    if (verbose) puts("STUB: QTSPresGetTimeBase called");
    return NULL;
}

void* QTSPresGetTimeScale(void)
{
    if (verbose) puts("STUB: QTSPresGetTimeScale called");
    return NULL;
}

void* QTSPresGetVisualContext(void)
{
    if (verbose) puts("STUB: QTSPresGetVisualContext called");
    return NULL;
}

void* QTSPresGetVolumes(void)
{
    if (verbose) puts("STUB: QTSPresGetVolumes called");
    return NULL;
}

void* QTSPresHasCharacteristic(void)
{
    if (verbose) puts("STUB: QTSPresHasCharacteristic called");
    return NULL;
}

void* QTSPresIdle(void)
{
    if (verbose) puts("STUB: QTSPresIdle called");
    return NULL;
}

void* QTSPresInvalidateRegion(void)
{
    if (verbose) puts("STUB: QTSPresInvalidateRegion called");
    return NULL;
}

void* QTSPresNewStream(void)
{
    if (verbose) puts("STUB: QTSPresNewStream called");
    return NULL;
}

void* QTSPresPreroll(void)
{
    if (verbose) puts("STUB: QTSPresPreroll called");
    return NULL;
}

void* QTSPresPreroll64(void)
{
    if (verbose) puts("STUB: QTSPresPreroll64 called");
    return NULL;
}

void* QTSPresPreview(void)
{
    if (verbose) puts("STUB: QTSPresPreview called");
    return NULL;
}

void* QTSPresPreview64(void)
{
    if (verbose) puts("STUB: QTSPresPreview64 called");
    return NULL;
}

void* QTSPresRemoveSourcer(void)
{
    if (verbose) puts("STUB: QTSPresRemoveSourcer called");
    return NULL;
}

void* QTSPresSetActiveSegment(void)
{
    if (verbose) puts("STUB: QTSPresSetActiveSegment called");
    return NULL;
}

void* QTSPresSetClip(void)
{
    if (verbose) puts("STUB: QTSPresSetClip called");
    return NULL;
}

void* QTSPresSetDimensions(void)
{
    if (verbose) puts("STUB: QTSPresSetDimensions called");
    return NULL;
}

void* QTSPresSetEnable(void)
{
    if (verbose) puts("STUB: QTSPresSetEnable called");
    return NULL;
}

void* QTSPresSetFlags(void)
{
    if (verbose) puts("STUB: QTSPresSetFlags called");
    return NULL;
}

void* QTSPresSetGWorld(void)
{
    if (verbose) puts("STUB: QTSPresSetGWorld called");
    return NULL;
}

void* QTSPresSetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSPresSetGraphicsMode called");
    return NULL;
}

void* QTSPresSetInfo(void)
{
    if (verbose) puts("STUB: QTSPresSetInfo called");
    return NULL;
}

void* QTSPresSetMatrix(void)
{
    if (verbose) puts("STUB: QTSPresSetMatrix called");
    return NULL;
}

void* QTSPresSetNotificationProc(void)
{
    if (verbose) puts("STUB: QTSPresSetNotificationProc called");
    return NULL;
}

void* QTSPresSetPlayHints(void)
{
    if (verbose) puts("STUB: QTSPresSetPlayHints called");
    return NULL;
}

void* QTSPresSetPreferredRate(void)
{
    if (verbose) puts("STUB: QTSPresSetPreferredRate called");
    return NULL;
}

void* QTSPresSetPresenting(void)
{
    if (verbose) puts("STUB: QTSPresSetPresenting called");
    return NULL;
}

void* QTSPresSetProperty(void)
{
    if (verbose) puts("STUB: QTSPresSetProperty called");
    return NULL;
}

void* QTSPresSetSettings(void)
{
    if (verbose) puts("STUB: QTSPresSetSettings called");
    return NULL;
}

void* QTSPresSetStatusFlags(void)
{
    if (verbose) puts("STUB: QTSPresSetStatusFlags called");
    return NULL;
}

void* QTSPresSetVisualContext(void)
{
    if (verbose) puts("STUB: QTSPresSetVisualContext called");
    return NULL;
}

void* QTSPresSetVolumes(void)
{
    if (verbose) puts("STUB: QTSPresSetVolumes called");
    return NULL;
}

void* QTSPresSettingsDialog(void)
{
    if (verbose) puts("STUB: QTSPresSettingsDialog called");
    return NULL;
}

void* QTSPresSettingsDialogWithFilters(void)
{
    if (verbose) puts("STUB: QTSPresSettingsDialogWithFilters called");
    return NULL;
}

void* QTSPresSkipTo(void)
{
    if (verbose) puts("STUB: QTSPresSkipTo called");
    return NULL;
}

void* QTSPresSkipTo64(void)
{
    if (verbose) puts("STUB: QTSPresSkipTo64 called");
    return NULL;
}

void* QTSPresStart(void)
{
    if (verbose) puts("STUB: QTSPresStart called");
    return NULL;
}

void* QTSPresStop(void)
{
    if (verbose) puts("STUB: QTSPresStop called");
    return NULL;
}

void* QTSRegionTagToLanguageTagString(void)
{
    if (verbose) puts("STUB: QTSRegionTagToLanguageTagString called");
    return NULL;
}

void* QTSReleaseMemPtr(void)
{
    if (verbose) puts("STUB: QTSReleaseMemPtr called");
    return NULL;
}

void* QTSSMAddLinkEnd(void)
{
    if (verbose) puts("STUB: QTSSMAddLinkEnd called");
    return NULL;
}

void* QTSSMGetInfo(void)
{
    if (verbose) puts("STUB: QTSSMGetInfo called");
    return NULL;
}

void* QTSSMGetStreamModuleProc(void)
{
    if (verbose) puts("STUB: QTSSMGetStreamModuleProc called");
    return NULL;
}

void* QTSSMHasCharacteristic(void)
{
    if (verbose) puts("STUB: QTSSMHasCharacteristic called");
    return NULL;
}

void* QTSSMIdle(void)
{
    if (verbose) puts("STUB: QTSSMIdle called");
    return NULL;
}

void* QTSSMRcvControl(void)
{
    if (verbose) puts("STUB: QTSSMRcvControl called");
    return NULL;
}

void* QTSSMRcvData(void)
{
    if (verbose) puts("STUB: QTSSMRcvData called");
    return NULL;
}

void* QTSSMRemoveLinkEnd(void)
{
    if (verbose) puts("STUB: QTSSMRemoveLinkEnd called");
    return NULL;
}

void* QTSSMSendDataContents(void)
{
    if (verbose) puts("STUB: QTSSMSendDataContents called");
    return NULL;
}

void* QTSSMSetInfo(void)
{
    if (verbose) puts("STUB: QTSSMSetInfo called");
    return NULL;
}

void* QTSSMSetStreamModuleProc(void)
{
    if (verbose) puts("STUB: QTSSMSetStreamModuleProc called");
    return NULL;
}

void* QTSSURLNeedsNoProxy(void)
{
    if (verbose) puts("STUB: QTSSURLNeedsNoProxy called");
    return NULL;
}

void* QTSSetNetworkAppName(void)
{
    if (verbose) puts("STUB: QTSSetNetworkAppName called");
    return NULL;
}

void* QTSSetTaggedHandle(void)
{
    if (verbose) puts("STUB: QTSSetTaggedHandle called");
    return NULL;
}

void* QTSSourcerAddLeafSettings(void)
{
    if (verbose) puts("STUB: QTSSourcerAddLeafSettings called");
    return NULL;
}

void* QTSSourcerFlush(void)
{
    if (verbose) puts("STUB: QTSSourcerFlush called");
    return NULL;
}

void* QTSSourcerGetClip(void)
{
    if (verbose) puts("STUB: QTSSourcerGetClip called");
    return NULL;
}

void* QTSSourcerGetDimensions(void)
{
    if (verbose) puts("STUB: QTSSourcerGetDimensions called");
    return NULL;
}

void* QTSSourcerGetEnable(void)
{
    if (verbose) puts("STUB: QTSSourcerGetEnable called");
    return NULL;
}

void* QTSSourcerGetGWorld(void)
{
    if (verbose) puts("STUB: QTSSourcerGetGWorld called");
    return NULL;
}

void* QTSSourcerGetIndSourcer(void)
{
    if (verbose) puts("STUB: QTSSourcerGetIndSourcer called");
    return NULL;
}

void* QTSSourcerGetInfo(void)
{
    if (verbose) puts("STUB: QTSSourcerGetInfo called");
    return NULL;
}

void* QTSSourcerGetMatrix(void)
{
    if (verbose) puts("STUB: QTSSourcerGetMatrix called");
    return NULL;
}

void* QTSSourcerGetMediaType(void)
{
    if (verbose) puts("STUB: QTSSourcerGetMediaType called");
    return NULL;
}

void* QTSSourcerGetNumSourcers(void)
{
    if (verbose) puts("STUB: QTSSourcerGetNumSourcers called");
    return NULL;
}

void* QTSSourcerGetPicture(void)
{
    if (verbose) puts("STUB: QTSSourcerGetPicture called");
    return NULL;
}

void* QTSSourcerGetSampleDescription(void)
{
    if (verbose) puts("STUB: QTSSourcerGetSampleDescription called");
    return NULL;
}

void* QTSSourcerGetSettings(void)
{
    if (verbose) puts("STUB: QTSSourcerGetSettings called");
    return NULL;
}

void* QTSSourcerGetSettingsAsText(void)
{
    if (verbose) puts("STUB: QTSSourcerGetSettingsAsText called");
    return NULL;
}

void* QTSSourcerGetSourcerEnable(void)
{
    if (verbose) puts("STUB: QTSSourcerGetSourcerEnable called");
    return NULL;
}

void* QTSSourcerGetSrcRgn(void)
{
    if (verbose) puts("STUB: QTSSourcerGetSrcRgn called");
    return NULL;
}

void* QTSSourcerGetTimeBase(void)
{
    if (verbose) puts("STUB: QTSSourcerGetTimeBase called");
    return NULL;
}

void* QTSSourcerGetTimeScale(void)
{
    if (verbose) puts("STUB: QTSSourcerGetTimeScale called");
    return NULL;
}

void* QTSSourcerGetVolumes(void)
{
    if (verbose) puts("STUB: QTSSourcerGetVolumes called");
    return NULL;
}

void* QTSSourcerHasCharacteristic(void)
{
    if (verbose) puts("STUB: QTSSourcerHasCharacteristic called");
    return NULL;
}

void* QTSSourcerIdle(void)
{
    if (verbose) puts("STUB: QTSSourcerIdle called");
    return NULL;
}

void* QTSSourcerInitialize(void)
{
    if (verbose) puts("STUB: QTSSourcerInitialize called");
    return NULL;
}

void* QTSSourcerInvalidateRegion(void)
{
    if (verbose) puts("STUB: QTSSourcerInvalidateRegion called");
    return NULL;
}

void* QTSSourcerMakeSourcerData(void)
{
    if (verbose) puts("STUB: QTSSourcerMakeSourcerData called");
    return NULL;
}

void* QTSSourcerPreroll(void)
{
    if (verbose) puts("STUB: QTSSourcerPreroll called");
    return NULL;
}

void* QTSSourcerPreview(void)
{
    if (verbose) puts("STUB: QTSSourcerPreview called");
    return NULL;
}

void* QTSSourcerSendDataIn(void)
{
    if (verbose) puts("STUB: QTSSourcerSendDataIn called");
    return NULL;
}

void* QTSSourcerSendDataOut(void)
{
    if (verbose) puts("STUB: QTSSourcerSendDataOut called");
    return NULL;
}

void* QTSSourcerSendGetInfo(void)
{
    if (verbose) puts("STUB: QTSSourcerSendGetInfo called");
    return NULL;
}

void* QTSSourcerSendMessage(void)
{
    if (verbose) puts("STUB: QTSSourcerSendMessage called");
    return NULL;
}

void* QTSSourcerSendSampleDescChangedIn(void)
{
    if (verbose) puts("STUB: QTSSourcerSendSampleDescChangedIn called");
    return NULL;
}

void* QTSSourcerSendSampleDescChangedOut(void)
{
    if (verbose) puts("STUB: QTSSourcerSendSampleDescChangedOut called");
    return NULL;
}

void* QTSSourcerSendSetInfo(void)
{
    if (verbose) puts("STUB: QTSSourcerSendSetInfo called");
    return NULL;
}

void* QTSSourcerSetClip(void)
{
    if (verbose) puts("STUB: QTSSourcerSetClip called");
    return NULL;
}

void* QTSSourcerSetDimensions(void)
{
    if (verbose) puts("STUB: QTSSourcerSetDimensions called");
    return NULL;
}

void* QTSSourcerSetEnable(void)
{
    if (verbose) puts("STUB: QTSSourcerSetEnable called");
    return NULL;
}

void* QTSSourcerSetGWorld(void)
{
    if (verbose) puts("STUB: QTSSourcerSetGWorld called");
    return NULL;
}

void* QTSSourcerSetHints(void)
{
    if (verbose) puts("STUB: QTSSourcerSetHints called");
    return NULL;
}

void* QTSSourcerSetInfo(void)
{
    if (verbose) puts("STUB: QTSSourcerSetInfo called");
    return NULL;
}

void* QTSSourcerSetLeafSettings(void)
{
    if (verbose) puts("STUB: QTSSourcerSetLeafSettings called");
    return NULL;
}

void* QTSSourcerSetMatrix(void)
{
    if (verbose) puts("STUB: QTSSourcerSetMatrix called");
    return NULL;
}

void* QTSSourcerSetMediaType(void)
{
    if (verbose) puts("STUB: QTSSourcerSetMediaType called");
    return NULL;
}

void* QTSSourcerSetOutputSourcer(void)
{
    if (verbose) puts("STUB: QTSSourcerSetOutputSourcer called");
    return NULL;
}

void* QTSSourcerSetSampleDescription(void)
{
    if (verbose) puts("STUB: QTSSourcerSetSampleDescription called");
    return NULL;
}

void* QTSSourcerSetSettings(void)
{
    if (verbose) puts("STUB: QTSSourcerSetSettings called");
    return NULL;
}

void* QTSSourcerSetSourcerEnable(void)
{
    if (verbose) puts("STUB: QTSSourcerSetSourcerEnable called");
    return NULL;
}

void* QTSSourcerSetTimeBase(void)
{
    if (verbose) puts("STUB: QTSSourcerSetTimeBase called");
    return NULL;
}

void* QTSSourcerSetTimeScale(void)
{
    if (verbose) puts("STUB: QTSSourcerSetTimeScale called");
    return NULL;
}

void* QTSSourcerSetVolumes(void)
{
    if (verbose) puts("STUB: QTSSourcerSetVolumes called");
    return NULL;
}

void* QTSSourcerSettingsDialog(void)
{
    if (verbose) puts("STUB: QTSSourcerSettingsDialog called");
    return NULL;
}

void* QTSSourcerStart(void)
{
    if (verbose) puts("STUB: QTSSourcerStart called");
    return NULL;
}

void* QTSSourcerStop(void)
{
    if (verbose) puts("STUB: QTSSourcerStop called");
    return NULL;
}

void* QTSStatHelperGetNumStats(void)
{
    if (verbose) puts("STUB: QTSStatHelperGetNumStats called");
    return NULL;
}

void* QTSStatHelperGetStats(void)
{
    if (verbose) puts("STUB: QTSStatHelperGetStats called");
    return NULL;
}

void* QTSStatHelperNext(void)
{
    if (verbose) puts("STUB: QTSStatHelperNext called");
    return NULL;
}

void* QTSStatHelperResetIter(void)
{
    if (verbose) puts("STUB: QTSStatHelperResetIter called");
    return NULL;
}

void* QTSStatusHDisposeHandler(void)
{
    if (verbose) puts("STUB: QTSStatusHDisposeHandler called");
    return NULL;
}

void* QTSStatusHGetClip(void)
{
    if (verbose) puts("STUB: QTSStatusHGetClip called");
    return NULL;
}

void* QTSStatusHGetDimensions(void)
{
    if (verbose) puts("STUB: QTSStatusHGetDimensions called");
    return NULL;
}

void* QTSStatusHGetFlags(void)
{
    if (verbose) puts("STUB: QTSStatusHGetFlags called");
    return NULL;
}

void* QTSStatusHGetGWorld(void)
{
    if (verbose) puts("STUB: QTSStatusHGetGWorld called");
    return NULL;
}

void* QTSStatusHGetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSStatusHGetGraphicsMode called");
    return NULL;
}

void* QTSStatusHGetIndPresentation(void)
{
    if (verbose) puts("STUB: QTSStatusHGetIndPresentation called");
    return NULL;
}

void* QTSStatusHGetInfo(void)
{
    if (verbose) puts("STUB: QTSStatusHGetInfo called");
    return NULL;
}

void* QTSStatusHGetMatrix(void)
{
    if (verbose) puts("STUB: QTSStatusHGetMatrix called");
    return NULL;
}

void* QTSStatusHGetNumPresentations(void)
{
    if (verbose) puts("STUB: QTSStatusHGetNumPresentations called");
    return NULL;
}

void* QTSStatusHGetPicture(void)
{
    if (verbose) puts("STUB: QTSStatusHGetPicture called");
    return NULL;
}

void* QTSStatusHGetVolumes(void)
{
    if (verbose) puts("STUB: QTSStatusHGetVolumes called");
    return NULL;
}

void* QTSStatusHIdle(void)
{
    if (verbose) puts("STUB: QTSStatusHIdle called");
    return NULL;
}

void* QTSStatusHInvalidateRegion(void)
{
    if (verbose) puts("STUB: QTSStatusHInvalidateRegion called");
    return NULL;
}

void* QTSStatusHNewHandler(void)
{
    if (verbose) puts("STUB: QTSStatusHNewHandler called");
    return NULL;
}

void* QTSStatusHRemoveAllPresentations(void)
{
    if (verbose) puts("STUB: QTSStatusHRemoveAllPresentations called");
    return NULL;
}

void* QTSStatusHRemovePresentation(void)
{
    if (verbose) puts("STUB: QTSStatusHRemovePresentation called");
    return NULL;
}

void* QTSStatusHSetClip(void)
{
    if (verbose) puts("STUB: QTSStatusHSetClip called");
    return NULL;
}

void* QTSStatusHSetCurrentError(void)
{
    if (verbose) puts("STUB: QTSStatusHSetCurrentError called");
    return NULL;
}

void* QTSStatusHSetCurrentStatus(void)
{
    if (verbose) puts("STUB: QTSStatusHSetCurrentStatus called");
    return NULL;
}

void* QTSStatusHSetDimensions(void)
{
    if (verbose) puts("STUB: QTSStatusHSetDimensions called");
    return NULL;
}

void* QTSStatusHSetFlags(void)
{
    if (verbose) puts("STUB: QTSStatusHSetFlags called");
    return NULL;
}

void* QTSStatusHSetGWorld(void)
{
    if (verbose) puts("STUB: QTSStatusHSetGWorld called");
    return NULL;
}

void* QTSStatusHSetGraphicsMode(void)
{
    if (verbose) puts("STUB: QTSStatusHSetGraphicsMode called");
    return NULL;
}

void* QTSStatusHSetInfo(void)
{
    if (verbose) puts("STUB: QTSStatusHSetInfo called");
    return NULL;
}

void* QTSStatusHSetMatrix(void)
{
    if (verbose) puts("STUB: QTSStatusHSetMatrix called");
    return NULL;
}

void* QTSStatusHSetPresentation(void)
{
    if (verbose) puts("STUB: QTSStatusHSetPresentation called");
    return NULL;
}

void* QTSStatusHSetSendDataProc(void)
{
    if (verbose) puts("STUB: QTSStatusHSetSendDataProc called");
    return NULL;
}

void* QTSStatusHSetVisualContext(void)
{
    if (verbose) puts("STUB: QTSStatusHSetVisualContext called");
    return NULL;
}

void* QTSStatusHSetVolumes(void)
{
    if (verbose) puts("STUB: QTSStatusHSetVolumes called");
    return NULL;
}

void* QTSStreamBufferDataInfo(void)
{
    if (verbose) puts("STUB: QTSStreamBufferDataInfo called");
    return NULL;
}

void* QTSTimerCreate(void)
{
    if (verbose) puts("STUB: QTSTimerCreate called");
    return NULL;
}

void* QTSTimerGetProperty(void)
{
    if (verbose) puts("STUB: QTSTimerGetProperty called");
    return NULL;
}

void* QTSTimerGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSTimerGetPropertyInfo called");
    return NULL;
}

void* QTSTimerRelease(void)
{
    if (verbose) puts("STUB: QTSTimerRelease called");
    return NULL;
}

void* QTSTimerRetain(void)
{
    if (verbose) puts("STUB: QTSTimerRetain called");
    return NULL;
}

void* QTSTimerSetNextFireTimeFromNow(void)
{
    if (verbose) puts("STUB: QTSTimerSetNextFireTimeFromNow called");
    return NULL;
}

void* QTSTimerSetProperty(void)
{
    if (verbose) puts("STUB: QTSTimerSetProperty called");
    return NULL;
}

void* QTSTimerStart(void)
{
    if (verbose) puts("STUB: QTSTimerStart called");
    return NULL;
}

void* QTSTimerStop(void)
{
    if (verbose) puts("STUB: QTSTimerStop called");
    return NULL;
}

void* QTSTrackHinterAddLeafSettings(void)
{
    if (verbose) puts("STUB: QTSTrackHinterAddLeafSettings called");
    return NULL;
}

void* QTSTrackHinterDoUserDialog(void)
{
    if (verbose) puts("STUB: QTSTrackHinterDoUserDialog called");
    return NULL;
}

void* QTSTrackHinterGetInfo(void)
{
    if (verbose) puts("STUB: QTSTrackHinterGetInfo called");
    return NULL;
}

void* QTSTrackHinterGetSettingsAsText(void)
{
    if (verbose) puts("STUB: QTSTrackHinterGetSettingsAsText called");
    return NULL;
}

void* QTSTrackHinterGetSettingsIntoAtomContainerAtAtom(void)
{
    if (verbose) puts("STUB: QTSTrackHinterGetSettingsIntoAtomContainerAtAtom called");
    return NULL;
}

void* QTSTrackHinterHasCharacteristic(void)
{
    if (verbose) puts("STUB: QTSTrackHinterHasCharacteristic called");
    return NULL;
}

void* QTSTrackHinterHintTrack(void)
{
    if (verbose) puts("STUB: QTSTrackHinterHintTrack called");
    return NULL;
}

void* QTSTrackHinterSetInfo(void)
{
    if (verbose) puts("STUB: QTSTrackHinterSetInfo called");
    return NULL;
}

void* QTSTrackHinterSetLeafSettings(void)
{
    if (verbose) puts("STUB: QTSTrackHinterSetLeafSettings called");
    return NULL;
}

void* QTSTrackHinterSetSettingsFromAtomContainerAtAtom(void)
{
    if (verbose) puts("STUB: QTSTrackHinterSetSettingsFromAtomContainerAtAtom called");
    return NULL;
}

void* QTSTrackHinterValidate(void)
{
    if (verbose) puts("STUB: QTSTrackHinterValidate called");
    return NULL;
}

void* QTSVersionToString(void)
{
    if (verbose) puts("STUB: QTSVersionToString called");
    return NULL;
}

void* QTSampleNumToChunkNum(void)
{
    if (verbose) puts("STUB: QTSampleNumToChunkNum called");
    return NULL;
}

void* QTSampleTableAddDataRef(void)
{
    if (verbose) puts("STUB: QTSampleTableAddDataRef called");
    return NULL;
}

void* QTSampleTableAddEntriesFromSampleRefs(void)
{
    if (verbose) puts("STUB: QTSampleTableAddEntriesFromSampleRefs called");
    return NULL;
}

void* QTSampleTableAddEntry(void)
{
    if (verbose) puts("STUB: QTSampleTableAddEntry called");
    return NULL;
}

void* QTSampleTableAddSampleDescription(void)
{
    if (verbose) puts("STUB: QTSampleTableAddSampleDescription called");
    return NULL;
}

void* QTSampleTableAddSampleReferences(void)
{
    if (verbose) puts("STUB: QTSampleTableAddSampleReferences called");
    return NULL;
}

void* QTSampleTableCopyDataRef(void)
{
    if (verbose) puts("STUB: QTSampleTableCopyDataRef called");
    return NULL;
}

void* QTSampleTableCopyMetaData(void)
{
    if (verbose) puts("STUB: QTSampleTableCopyMetaData called");
    return NULL;
}

void* QTSampleTableCopySampleDescription(void)
{
    if (verbose) puts("STUB: QTSampleTableCopySampleDescription called");
    return NULL;
}

void* QTSampleTableCreateMutable(void)
{
    if (verbose) puts("STUB: QTSampleTableCreateMutable called");
    return NULL;
}

void* QTSampleTableCreateMutableCopy(void)
{
    if (verbose) puts("STUB: QTSampleTableCreateMutableCopy called");
    return NULL;
}

void* QTSampleTableCreateMutableCopyFromRange(void)
{
    if (verbose) puts("STUB: QTSampleTableCreateMutableCopyFromRange called");
    return NULL;
}

void* QTSampleTableDecodeTimeToSampleNum(void)
{
    if (verbose) puts("STUB: QTSampleTableDecodeTimeToSampleNum called");
    return NULL;
}

void* QTSampleTableEntryIndexToSampleNum(void)
{
    if (verbose) puts("STUB: QTSampleTableEntryIndexToSampleNum called");
    return NULL;
}

void* QTSampleTableEntryRangeToSampleNumRange(void)
{
    if (verbose) puts("STUB: QTSampleTableEntryRangeToSampleNumRange called");
    return NULL;
}

void* QTSampleTableGetAny64BitDataOffsetsExist(void)
{
    if (verbose) puts("STUB: QTSampleTableGetAny64BitDataOffsetsExist called");
    return NULL;
}

void* QTSampleTableGetAnyMatchingSampleFlagsExist(void)
{
    if (verbose) puts("STUB: QTSampleTableGetAnyMatchingSampleFlagsExist called");
    return NULL;
}

void* QTSampleTableGetAnyNonZeroDisplayOffsetsExist(void)
{
    if (verbose) puts("STUB: QTSampleTableGetAnyNonZeroDisplayOffsetsExist called");
    return NULL;
}

void* QTSampleTableGetDataOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableGetDataOffset called");
    return NULL;
}

void* QTSampleTableGetDataSizePerSample(void)
{
    if (verbose) puts("STUB: QTSampleTableGetDataSizePerSample called");
    return NULL;
}

void* QTSampleTableGetDecodeDuration(void)
{
    if (verbose) puts("STUB: QTSampleTableGetDecodeDuration called");
    return NULL;
}

void* QTSampleTableGetDisplayOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableGetDisplayOffset called");
    return NULL;
}

void* QTSampleTableGetEntriesDataOffsets(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesDataOffsets called");
    return NULL;
}

void* QTSampleTableGetEntriesDataSizes(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesDataSizes called");
    return NULL;
}

void* QTSampleTableGetEntriesDecodeDurations(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesDecodeDurations called");
    return NULL;
}

void* QTSampleTableGetEntriesDisplayOffsets(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesDisplayOffsets called");
    return NULL;
}

void* QTSampleTableGetEntriesNumbersOfSamples(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesNumbersOfSamples called");
    return NULL;
}

void* QTSampleTableGetEntriesPtr(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesPtr called");
    return NULL;
}

void* QTSampleTableGetEntriesSampleFlags(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntriesSampleFlags called");
    return NULL;
}

void* QTSampleTableGetEntryDataOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryDataOffset called");
    return NULL;
}

void* QTSampleTableGetEntryDataSize(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryDataSize called");
    return NULL;
}

void* QTSampleTableGetEntryDecodeDuration(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryDecodeDuration called");
    return NULL;
}

void* QTSampleTableGetEntryDescriptionID(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryDescriptionID called");
    return NULL;
}

void* QTSampleTableGetEntryDisplayOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryDisplayOffset called");
    return NULL;
}

void* QTSampleTableGetEntryInfo(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryInfo called");
    return NULL;
}

void* QTSampleTableGetEntryNumberOfSamples(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntryNumberOfSamples called");
    return NULL;
}

void* QTSampleTableGetEntrySampleFlags(void)
{
    if (verbose) puts("STUB: QTSampleTableGetEntrySampleFlags called");
    return NULL;
}

void* QTSampleTableGetMinAndMaxDisplayOffsets(void)
{
    if (verbose) puts("STUB: QTSampleTableGetMinAndMaxDisplayOffsets called");
    return NULL;
}

void* QTSampleTableGetMinAndMaxRelativeDisplayTimes(void)
{
    if (verbose) puts("STUB: QTSampleTableGetMinAndMaxRelativeDisplayTimes called");
    return NULL;
}

void* QTSampleTableGetNextAttributeChange(void)
{
    if (verbose) puts("STUB: QTSampleTableGetNextAttributeChange called");
    return NULL;
}

void* QTSampleTableGetNumberOfEntries(void)
{
    if (verbose) puts("STUB: QTSampleTableGetNumberOfEntries called");
    return NULL;
}

void* QTSampleTableGetNumberOfSamples(void)
{
    if (verbose) puts("STUB: QTSampleTableGetNumberOfSamples called");
    return NULL;
}

void* QTSampleTableGetProperty(void)
{
    if (verbose) puts("STUB: QTSampleTableGetProperty called");
    return NULL;
}

void* QTSampleTableGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSampleTableGetPropertyInfo called");
    return NULL;
}

void* QTSampleTableGetSampleDescriptionID(void)
{
    if (verbose) puts("STUB: QTSampleTableGetSampleDescriptionID called");
    return NULL;
}

void* QTSampleTableGetSampleFlags(void)
{
    if (verbose) puts("STUB: QTSampleTableGetSampleFlags called");
    return NULL;
}

void* QTSampleTableGetTimeScale(void)
{
    if (verbose) puts("STUB: QTSampleTableGetTimeScale called");
    return NULL;
}

void* QTSampleTableGetTotalDecodeDuration(void)
{
    if (verbose) puts("STUB: QTSampleTableGetTotalDecodeDuration called");
    return NULL;
}

void* QTSampleTableGetTypeID(void)
{
    if (verbose) puts("STUB: QTSampleTableGetTypeID called");
    return NULL;
}

void* QTSampleTableRelease(void)
{
    if (verbose) puts("STUB: QTSampleTableRelease called");
    return NULL;
}

void* QTSampleTableReplaceEntries(void)
{
    if (verbose) puts("STUB: QTSampleTableReplaceEntries called");
    return NULL;
}

void* QTSampleTableReplaceRange(void)
{
    if (verbose) puts("STUB: QTSampleTableReplaceRange called");
    return NULL;
}

void* QTSampleTableRetain(void)
{
    if (verbose) puts("STUB: QTSampleTableRetain called");
    return NULL;
}

void* QTSampleTableSampleNumRangeToDisplayTimeRange(void)
{
    if (verbose) puts("STUB: QTSampleTableSampleNumRangeToDisplayTimeRange called");
    return NULL;
}

void* QTSampleTableSampleNumToDecodeTime(void)
{
    if (verbose) puts("STUB: QTSampleTableSampleNumToDecodeTime called");
    return NULL;
}

void* QTSampleTableSampleNumToEntryIndex(void)
{
    if (verbose) puts("STUB: QTSampleTableSampleNumToEntryIndex called");
    return NULL;
}

void* QTSampleTableSetDataRef(void)
{
    if (verbose) puts("STUB: QTSampleTableSetDataRef called");
    return NULL;
}

void* QTSampleTableSetEntriesDataOffsets(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesDataOffsets called");
    return NULL;
}

void* QTSampleTableSetEntriesDataSizes(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesDataSizes called");
    return NULL;
}

void* QTSampleTableSetEntriesDecodeDurations(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesDecodeDurations called");
    return NULL;
}

void* QTSampleTableSetEntriesDisplayOffsets(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesDisplayOffsets called");
    return NULL;
}

void* QTSampleTableSetEntriesNumbersOfSamples(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesNumbersOfSamples called");
    return NULL;
}

void* QTSampleTableSetEntriesSampleFlags(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntriesSampleFlags called");
    return NULL;
}

void* QTSampleTableSetEntryDataOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryDataOffset called");
    return NULL;
}

void* QTSampleTableSetEntryDataSize(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryDataSize called");
    return NULL;
}

void* QTSampleTableSetEntryDecodeDuration(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryDecodeDuration called");
    return NULL;
}

void* QTSampleTableSetEntryDescriptionID(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryDescriptionID called");
    return NULL;
}

void* QTSampleTableSetEntryDisplayOffset(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryDisplayOffset called");
    return NULL;
}

void* QTSampleTableSetEntryInfo(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryInfo called");
    return NULL;
}

void* QTSampleTableSetEntryNumberOfSamples(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntryNumberOfSamples called");
    return NULL;
}

void* QTSampleTableSetEntrySampleFlags(void)
{
    if (verbose) puts("STUB: QTSampleTableSetEntrySampleFlags called");
    return NULL;
}

void* QTSampleTableSetNumberOfEntries(void)
{
    if (verbose) puts("STUB: QTSampleTableSetNumberOfEntries called");
    return NULL;
}

void* QTSampleTableSetProperty(void)
{
    if (verbose) puts("STUB: QTSampleTableSetProperty called");
    return NULL;
}

void* QTSampleTableSetSampleDescription(void)
{
    if (verbose) puts("STUB: QTSampleTableSetSampleDescription called");
    return NULL;
}

void* QTSampleTableSetTimeScale(void)
{
    if (verbose) puts("STUB: QTSampleTableSetTimeScale called");
    return NULL;
}

void* QTScheduledBandwidthRelease(void)
{
    if (verbose) puts("STUB: QTScheduledBandwidthRelease called");
    return NULL;
}

void* QTScheduledBandwidthRequest(void)
{
    if (verbose) puts("STUB: QTScheduledBandwidthRequest called");
    return NULL;
}

void* QTSecurityPolicyCanUseDataRef(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyCanUseDataRef called");
    return NULL;
}

void* QTSecurityPolicyCanUseDataRefWithParentDataRef(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyCanUseDataRefWithParentDataRef called");
    return NULL;
}

void* QTSecurityPolicyCanUseFile(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyCanUseFile called");
    return NULL;
}

void* QTSecurityPolicyCreate(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyCreate called");
    return NULL;
}

void* QTSecurityPolicyCreateCopy(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyCreateCopy called");
    return NULL;
}

void* QTSecurityPolicyGetProperty(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyGetProperty called");
    return NULL;
}

void* QTSecurityPolicyGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyGetPropertyInfo called");
    return NULL;
}

void* QTSecurityPolicyGetTypeID(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyGetTypeID called");
    return NULL;
}

void* QTSecurityPolicyRelease(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyRelease called");
    return NULL;
}

void* QTSecurityPolicyRetain(void)
{
    if (verbose) puts("STUB: QTSecurityPolicyRetain called");
    return NULL;
}

void* QTSecurityPolicySetProperty(void)
{
    if (verbose) puts("STUB: QTSecurityPolicySetProperty called");
    return NULL;
}

void* QTSendMessageToObject(void)
{
    if (verbose) puts("STUB: QTSendMessageToObject called");
    return NULL;
}

void* QTSendToObject(void)
{
    if (verbose) puts("STUB: QTSendToObject called");
    return NULL;
}

void* QTSetAtomData(void)
{
    if (verbose) puts("STUB: QTSetAtomData called");
    return NULL;
}

void* QTSetAtomID(void)
{
    if (verbose) puts("STUB: QTSetAtomID called");
    return NULL;
}

void* QTSetComponentProperty(void)
{
    if (verbose) puts("STUB: QTSetComponentProperty called");
    return NULL;
}

void* QTSetHandleSize(void)
{
    if (verbose) puts("STUB: QTSetHandleSize called");
    return NULL;
}

void* QTSetMetaDataPropertyWithMessage(void)
{
    if (verbose) puts("STUB: QTSetMetaDataPropertyWithMessage called");
    return NULL;
}

void* QTSetMovieFileTypePublicHandle(void)
{
    if (verbose) puts("STUB: QTSetMovieFileTypePublicHandle called");
    return NULL;
}

void* QTSetMoviePrivateInfo(void)
{
    if (verbose) puts("STUB: QTSetMoviePrivateInfo called");
    return NULL;
}

void* QTSetMovieProfileInformation(void)
{
    if (verbose) puts("STUB: QTSetMovieProfileInformation called");
    return NULL;
}

void* QTSetMovieProperty(void)
{
    if (verbose) puts("STUB: QTSetMovieProperty called");
    return NULL;
}

void* QTSetObjectProperty(void)
{
    if (verbose) puts("STUB: QTSetObjectProperty called");
    return NULL;
}

void* QTSetPixMapHandleGammaLevel(void)
{
    if (verbose) puts("STUB: QTSetPixMapHandleGammaLevel called");
    return NULL;
}

void* QTSetPixMapHandleRequestedGammaLevel(void)
{
    if (verbose) puts("STUB: QTSetPixMapHandleRequestedGammaLevel called");
    return NULL;
}

void* QTSetPixMapHandleRowBytes(void)
{
    if (verbose) puts("STUB: QTSetPixMapHandleRowBytes called");
    return NULL;
}

void* QTSetPixMapPtrGammaLevel(void)
{
    if (verbose) puts("STUB: QTSetPixMapPtrGammaLevel called");
    return NULL;
}

void* QTSetPixMapPtrRequestedGammaLevel(void)
{
    if (verbose) puts("STUB: QTSetPixMapPtrRequestedGammaLevel called");
    return NULL;
}

void* QTSetPixMapPtrRowBytes(void)
{
    if (verbose) puts("STUB: QTSetPixMapPtrRowBytes called");
    return NULL;
}

void* QTSetProcessProperty(void)
{
    if (verbose) puts("STUB: QTSetProcessProperty called");
    return NULL;
}

void* QTSetPtrSize(void)
{
    if (verbose) puts("STUB: QTSetPtrSize called");
    return NULL;
}

void* QTSetSampleMetaDataAssociation(void)
{
    if (verbose) puts("STUB: QTSetSampleMetaDataAssociation called");
    return NULL;
}

void* QTSetTrackProfileInformation(void)
{
    if (verbose) puts("STUB: QTSetTrackProfileInformation called");
    return NULL;
}

void* QTSetTrackProperty(void)
{
    if (verbose) puts("STUB: QTSetTrackProperty called");
    return NULL;
}

void* QTShowSheetWindow(void)
{
    if (verbose) puts("STUB: QTShowSheetWindow called");
    return NULL;
}

void* QTSndNewChannel(void)
{
    if (verbose) puts("STUB: QTSndNewChannel called");
    return NULL;
}

void* QTSoundDescriptionConvert(void)
{
    if (verbose) puts("STUB: QTSoundDescriptionConvert called");
    return NULL;
}

void* QTSoundDescriptionCreate(void)
{
    if (verbose) puts("STUB: QTSoundDescriptionCreate called");
    return NULL;
}

void* QTSoundDescriptionGetProperty(void)
{
    if (verbose) puts("STUB: QTSoundDescriptionGetProperty called");
    return NULL;
}

void* QTSoundDescriptionGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTSoundDescriptionGetPropertyInfo called");
    return NULL;
}

void* QTSoundDescriptionSetProperty(void)
{
    if (verbose) puts("STUB: QTSoundDescriptionSetProperty called");
    return NULL;
}

void* QTStandardParameterDialogDoAction(void)
{
    if (verbose) puts("STUB: QTStandardParameterDialogDoAction called");
    return NULL;
}

void* QTStartUsingPrivAPI(void)
{
    if (verbose) puts("STUB: QTStartUsingPrivAPI called");
    return NULL;
}

void* QTStdDialogFilterProc(void)
{
    if (verbose) puts("STUB: QTStdDialogFilterProc called");
    return NULL;
}

void* QTStopMessagePropagation(void)
{
    if (verbose) puts("STUB: QTStopMessagePropagation called");
    return NULL;
}

void* QTStopUsingPrivAPI(void)
{
    if (verbose) puts("STUB: QTStopUsingPrivAPI called");
    return NULL;
}

void* QTStreamBasicDescriptionIsCanonical(void)
{
    if (verbose) puts("STUB: QTStreamBasicDescriptionIsCanonical called");
    return NULL;
}

void* QTStreamBasicDescriptionIsMovieSafe(void)
{
    if (verbose) puts("STUB: QTStreamBasicDescriptionIsMovieSafe called");
    return NULL;
}

void* QTStreamBasicDescriptionSetCanonical(void)
{
    if (verbose) puts("STUB: QTStreamBasicDescriptionSetCanonical called");
    return NULL;
}

void* QTStreamBasicDescriptionSetMovieSafe(void)
{
    if (verbose) puts("STUB: QTStreamBasicDescriptionSetMovieSafe called");
    return NULL;
}

void* QTStreamingQOSLogPause(void)
{
    if (verbose) puts("STUB: QTStreamingQOSLogPause called");
    return NULL;
}

void* QTStreamingQOSLogPauseToBuffer(void)
{
    if (verbose) puts("STUB: QTStreamingQOSLogPauseToBuffer called");
    return NULL;
}

void* QTStreamingQOSLogPlay(void)
{
    if (verbose) puts("STUB: QTStreamingQOSLogPlay called");
    return NULL;
}

void* QTSwapAtoms(void)
{
    if (verbose) puts("STUB: QTSwapAtoms called");
    return NULL;
}

void* QTSyslogPrintAudioTimeStamp(void)
{
    if (verbose) puts("STUB: QTSyslogPrintAudioTimeStamp called");
    return NULL;
}

void* QTTIFFBuildExifDirectory(void)
{
    if (verbose) puts("STUB: QTTIFFBuildExifDirectory called");
    return NULL;
}

void* QTTempNewHandle(void)
{
    if (verbose) puts("STUB: QTTempNewHandle called");
    return NULL;
}

void* QTTextConvertBufferToHandle(void)
{
    if (verbose) puts("STUB: QTTextConvertBufferToHandle called");
    return NULL;
}

void* QTTextToNativeText(void)
{
    if (verbose) puts("STUB: QTTextToNativeText called");
    return NULL;
}

void* QTTimeBaseGetProperty(void)
{
    if (verbose) puts("STUB: QTTimeBaseGetProperty called");
    return NULL;
}

void* QTTimeBaseGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTTimeBaseGetPropertyInfo called");
    return NULL;
}

void* QTTimeBaseSetProperty(void)
{
    if (verbose) puts("STUB: QTTimeBaseSetProperty called");
    return NULL;
}

void* QTTrackStateCacheCreate(void)
{
    if (verbose) puts("STUB: QTTrackStateCacheCreate called");
    return NULL;
}

void* QTTrackStateCacheRefresh(void)
{
    if (verbose) puts("STUB: QTTrackStateCacheRefresh called");
    return NULL;
}

void* QTTrackStateCacheRelease(void)
{
    if (verbose) puts("STUB: QTTrackStateCacheRelease called");
    return NULL;
}

void* QTTrackStateCacheRetain(void)
{
    if (verbose) puts("STUB: QTTrackStateCacheRetain called");
    return NULL;
}

void* QTTrackStateMediaDisplayTimeToSampleNum(void)
{
    if (verbose) puts("STUB: QTTrackStateMediaDisplayTimeToSampleNum called");
    return NULL;
}

void* QTTrackStateMediaDisplayTimeToSampleTime(void)
{
    if (verbose) puts("STUB: QTTrackStateMediaDisplayTimeToSampleTime called");
    return NULL;
}

void* QTTrackStateTrackTimeToMediaDisplayTime(void)
{
    if (verbose) puts("STUB: QTTrackStateTrackTimeToMediaDisplayTime called");
    return NULL;
}

void* QTUIPanelLockNotifications(void)
{
    if (verbose) puts("STUB: QTUIPanelLockNotifications called");
    return NULL;
}

void* QTUIPanelPostNotification(void)
{
    if (verbose) puts("STUB: QTUIPanelPostNotification called");
    return NULL;
}

void* QTUIPanelSendCommand(void)
{
    if (verbose) puts("STUB: QTUIPanelSendCommand called");
    return NULL;
}

void* QTUIPanelUnlockNotifications(void)
{
    if (verbose) puts("STUB: QTUIPanelUnlockNotifications called");
    return NULL;
}

void* QTUninstallNextTaskNeededSoonerCallback(void)
{
    if (verbose) puts("STUB: QTUninstallNextTaskNeededSoonerCallback called");
    return NULL;
}

void* QTUnlockContainer(void)
{
    if (verbose) puts("STUB: QTUnlockContainer called");
    return NULL;
}

void* QTUnregisterAccessKey(void)
{
    if (verbose) puts("STUB: QTUnregisterAccessKey called");
    return NULL;
}

void* QTUpdateGWorld(void)
{
    if (verbose) puts("STUB: QTUpdateGWorld called");
    return NULL;
}

void* QTUpdateMovieProfileInformation(void)
{
    if (verbose) puts("STUB: QTUpdateMovieProfileInformation called");
    return NULL;
}

void* QTUpdateTrackProfileInformation(void)
{
    if (verbose) puts("STUB: QTUpdateTrackProfileInformation called");
    return NULL;
}

void* QTVCAddVisualContextPropertyListener(void)
{
    if (verbose) puts("STUB: QTVCAddVisualContextPropertyListener called");
    return NULL;
}

void* QTVCAttributeChanged(void)
{
    if (verbose) puts("STUB: QTVCAttributeChanged called");
    return NULL;
}

void* QTVCCopyDescription(void)
{
    if (verbose) puts("STUB: QTVCCopyDescription called");
    return NULL;
}

void* QTVCCopyImageForTime(void)
{
    if (verbose) puts("STUB: QTVCCopyImageForTime called");
    return NULL;
}

void* QTVCCopyPixelBufferOptions(void)
{
    if (verbose) puts("STUB: QTVCCopyPixelBufferOptions called");
    return NULL;
}

void* QTVCGetAttribute(void)
{
    if (verbose) puts("STUB: QTVCGetAttribute called");
    return NULL;
}

void* QTVCGetExpectedReadAhead(void)
{
    if (verbose) puts("STUB: QTVCGetExpectedReadAhead called");
    return NULL;
}

void* QTVCGetVisualGraph(void)
{
    if (verbose) puts("STUB: QTVCGetVisualGraph called");
    return NULL;
}

void* QTVCInitialize(void)
{
    if (verbose) puts("STUB: QTVCInitialize called");
    return NULL;
}

void* QTVCInitializeOpenGLTextureContext(void)
{
    if (verbose) puts("STUB: QTVCInitializeOpenGLTextureContext called");
    return NULL;
}

void* QTVCInitializePixelBufferContext(void)
{
    if (verbose) puts("STUB: QTVCInitializePixelBufferContext called");
    return NULL;
}

void* QTVCIsNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVCIsNewImageAvailable called");
    return NULL;
}

void* QTVCNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVCNewImageAvailable called");
    return NULL;
}

void* QTVCRemoveVisualContextPropertyListener(void)
{
    if (verbose) puts("STUB: QTVCRemoveVisualContextPropertyListener called");
    return NULL;
}

void* QTVCReportError(void)
{
    if (verbose) puts("STUB: QTVCReportError called");
    return NULL;
}

void* QTVCSetAttribute(void)
{
    if (verbose) puts("STUB: QTVCSetAttribute called");
    return NULL;
}

void* QTVCSetAttributeChangedCallback(void)
{
    if (verbose) puts("STUB: QTVCSetAttributeChangedCallback called");
    return NULL;
}

void* QTVCSetGenerateImageCallback(void)
{
    if (verbose) puts("STUB: QTVCSetGenerateImageCallback called");
    return NULL;
}

void* QTVCSetGetAttributeCallback(void)
{
    if (verbose) puts("STUB: QTVCSetGetAttributeCallback called");
    return NULL;
}

void* QTVCSetImageAvailableCallback(void)
{
    if (verbose) puts("STUB: QTVCSetImageAvailableCallback called");
    return NULL;
}

void* QTVCSetIsNewImageAvailableCallback(void)
{
    if (verbose) puts("STUB: QTVCSetIsNewImageAvailableCallback called");
    return NULL;
}

void* QTVCSetVisualGraph(void)
{
    if (verbose) puts("STUB: QTVCSetVisualGraph called");
    return NULL;
}

void* QTVCTask(void)
{
    if (verbose) puts("STUB: QTVCTask called");
    return NULL;
}

void* QTVRAnglesToCoord(void)
{
    if (verbose) puts("STUB: QTVRAnglesToCoord called");
    return NULL;
}

void* QTVRBeginUpdateStream(void)
{
    if (verbose) puts("STUB: QTVRBeginUpdateStream called");
    return NULL;
}

void* QTVRCallInterceptedProc(void)
{
    if (verbose) puts("STUB: QTVRCallInterceptedProc called");
    return NULL;
}

void* QTVRColumnToPan(void)
{
    if (verbose) puts("STUB: QTVRColumnToPan called");
    return NULL;
}

void* QTVRCoordToAngles(void)
{
    if (verbose) puts("STUB: QTVRCoordToAngles called");
    return NULL;
}

void* QTVREnableFrameAnimation(void)
{
    if (verbose) puts("STUB: QTVREnableFrameAnimation called");
    return NULL;
}

void* QTVREnableHotSpot(void)
{
    if (verbose) puts("STUB: QTVREnableHotSpot called");
    return NULL;
}

void* QTVREnableTransition(void)
{
    if (verbose) puts("STUB: QTVREnableTransition called");
    return NULL;
}

void* QTVREnableViewAnimation(void)
{
    if (verbose) puts("STUB: QTVREnableViewAnimation called");
    return NULL;
}

void* QTVREndUpdateStream(void)
{
    if (verbose) puts("STUB: QTVREndUpdateStream called");
    return NULL;
}

void* QTVRGetAngularUnits(void)
{
    if (verbose) puts("STUB: QTVRGetAngularUnits called");
    return NULL;
}

void* QTVRGetAnimationSetting(void)
{
    if (verbose) puts("STUB: QTVRGetAnimationSetting called");
    return NULL;
}

void* QTVRGetAvailableResolutions(void)
{
    if (verbose) puts("STUB: QTVRGetAvailableResolutions called");
    return NULL;
}

void* QTVRGetBackBufferMemInfo(void)
{
    if (verbose) puts("STUB: QTVRGetBackBufferMemInfo called");
    return NULL;
}

void* QTVRGetBackBufferSettings(void)
{
    if (verbose) puts("STUB: QTVRGetBackBufferSettings called");
    return NULL;
}

void* QTVRGetConstraintStatus(void)
{
    if (verbose) puts("STUB: QTVRGetConstraintStatus called");
    return NULL;
}

void* QTVRGetConstraints(void)
{
    if (verbose) puts("STUB: QTVRGetConstraints called");
    return NULL;
}

void* QTVRGetControlSetting(void)
{
    if (verbose) puts("STUB: QTVRGetControlSetting called");
    return NULL;
}

void* QTVRGetCurrentMouseMode(void)
{
    if (verbose) puts("STUB: QTVRGetCurrentMouseMode called");
    return NULL;
}

void* QTVRGetCurrentNodeID(void)
{
    if (verbose) puts("STUB: QTVRGetCurrentNodeID called");
    return NULL;
}

void* QTVRGetCurrentViewDuration(void)
{
    if (verbose) puts("STUB: QTVRGetCurrentViewDuration called");
    return NULL;
}

void* QTVRGetFieldOfView(void)
{
    if (verbose) puts("STUB: QTVRGetFieldOfView called");
    return NULL;
}

void* QTVRGetFrameAnimation(void)
{
    if (verbose) puts("STUB: QTVRGetFrameAnimation called");
    return NULL;
}

void* QTVRGetFrameRate(void)
{
    if (verbose) puts("STUB: QTVRGetFrameRate called");
    return NULL;
}

void* QTVRGetHotSpotRegion(void)
{
    if (verbose) puts("STUB: QTVRGetHotSpotRegion called");
    return NULL;
}

void* QTVRGetHotSpotType(void)
{
    if (verbose) puts("STUB: QTVRGetHotSpotType called");
    return NULL;
}

void* QTVRGetImagingProperty(void)
{
    if (verbose) puts("STUB: QTVRGetImagingProperty called");
    return NULL;
}

void* QTVRGetInteractionProperty(void)
{
    if (verbose) puts("STUB: QTVRGetInteractionProperty called");
    return NULL;
}

void* QTVRGetMouseDownTracking(void)
{
    if (verbose) puts("STUB: QTVRGetMouseDownTracking called");
    return NULL;
}

void* QTVRGetMouseOverTracking(void)
{
    if (verbose) puts("STUB: QTVRGetMouseOverTracking called");
    return NULL;
}

void* QTVRGetNodeInfo(void)
{
    if (verbose) puts("STUB: QTVRGetNodeInfo called");
    return NULL;
}

void* QTVRGetNodeType(void)
{
    if (verbose) puts("STUB: QTVRGetNodeType called");
    return NULL;
}

void* QTVRGetPanAngle(void)
{
    if (verbose) puts("STUB: QTVRGetPanAngle called");
    return NULL;
}

void* QTVRGetQTVRInstance(void)
{
    if (verbose) puts("STUB: QTVRGetQTVRInstance called");
    return NULL;
}

void* QTVRGetQTVRTrack(void)
{
    if (verbose) puts("STUB: QTVRGetQTVRTrack called");
    return NULL;
}

void* QTVRGetTiltAngle(void)
{
    if (verbose) puts("STUB: QTVRGetTiltAngle called");
    return NULL;
}

void* QTVRGetVRWorld(void)
{
    if (verbose) puts("STUB: QTVRGetVRWorld called");
    return NULL;
}

void* QTVRGetViewAnimation(void)
{
    if (verbose) puts("STUB: QTVRGetViewAnimation called");
    return NULL;
}

void* QTVRGetViewCenter(void)
{
    if (verbose) puts("STUB: QTVRGetViewCenter called");
    return NULL;
}

void* QTVRGetViewCurrentTime(void)
{
    if (verbose) puts("STUB: QTVRGetViewCurrentTime called");
    return NULL;
}

void* QTVRGetViewParameter(void)
{
    if (verbose) puts("STUB: QTVRGetViewParameter called");
    return NULL;
}

void* QTVRGetViewRate(void)
{
    if (verbose) puts("STUB: QTVRGetViewRate called");
    return NULL;
}

void* QTVRGetViewState(void)
{
    if (verbose) puts("STUB: QTVRGetViewState called");
    return NULL;
}

void* QTVRGetViewStateCount(void)
{
    if (verbose) puts("STUB: QTVRGetViewStateCount called");
    return NULL;
}

void* QTVRGetViewingLimits(void)
{
    if (verbose) puts("STUB: QTVRGetViewingLimits called");
    return NULL;
}

void* QTVRGetVisible(void)
{
    if (verbose) puts("STUB: QTVRGetVisible called");
    return NULL;
}

void* QTVRGetVisibleHotSpots(void)
{
    if (verbose) puts("STUB: QTVRGetVisibleHotSpots called");
    return NULL;
}

void* QTVRGoToNodeID(void)
{
    if (verbose) puts("STUB: QTVRGoToNodeID called");
    return NULL;
}

void* QTVRInstallInterceptProc(void)
{
    if (verbose) puts("STUB: QTVRInstallInterceptProc called");
    return NULL;
}

void* QTVRInteractionNudge(void)
{
    if (verbose) puts("STUB: QTVRInteractionNudge called");
    return NULL;
}

void* QTVRMouseDown(void)
{
    if (verbose) puts("STUB: QTVRMouseDown called");
    return NULL;
}

void* QTVRMouseEnter(void)
{
    if (verbose) puts("STUB: QTVRMouseEnter called");
    return NULL;
}

void* QTVRMouseLeave(void)
{
    if (verbose) puts("STUB: QTVRMouseLeave called");
    return NULL;
}

void* QTVRMouseStillDown(void)
{
    if (verbose) puts("STUB: QTVRMouseStillDown called");
    return NULL;
}

void* QTVRMouseStillDownExtended(void)
{
    if (verbose) puts("STUB: QTVRMouseStillDownExtended called");
    return NULL;
}

void* QTVRMouseUp(void)
{
    if (verbose) puts("STUB: QTVRMouseUp called");
    return NULL;
}

void* QTVRMouseUpExtended(void)
{
    if (verbose) puts("STUB: QTVRMouseUpExtended called");
    return NULL;
}

void* QTVRMouseWithin(void)
{
    if (verbose) puts("STUB: QTVRMouseWithin called");
    return NULL;
}

void* QTVRNudge(void)
{
    if (verbose) puts("STUB: QTVRNudge called");
    return NULL;
}

void* QTVRPanToColumn(void)
{
    if (verbose) puts("STUB: QTVRPanToColumn called");
    return NULL;
}

void* QTVRPtToAngles(void)
{
    if (verbose) puts("STUB: QTVRPtToAngles called");
    return NULL;
}

void* QTVRPtToHotSpotID(void)
{
    if (verbose) puts("STUB: QTVRPtToHotSpotID called");
    return NULL;
}

void* QTVRRefreshBackBuffer(void)
{
    if (verbose) puts("STUB: QTVRRefreshBackBuffer called");
    return NULL;
}

void* QTVRReplaceCursor(void)
{
    if (verbose) puts("STUB: QTVRReplaceCursor called");
    return NULL;
}

void* QTVRRowToTilt(void)
{
    if (verbose) puts("STUB: QTVRRowToTilt called");
    return NULL;
}

void* QTVRSetAngularUnits(void)
{
    if (verbose) puts("STUB: QTVRSetAngularUnits called");
    return NULL;
}

void* QTVRSetAnimationSetting(void)
{
    if (verbose) puts("STUB: QTVRSetAnimationSetting called");
    return NULL;
}

void* QTVRSetBackBufferImagingProc(void)
{
    if (verbose) puts("STUB: QTVRSetBackBufferImagingProc called");
    return NULL;
}

void* QTVRSetBackBufferPrefs(void)
{
    if (verbose) puts("STUB: QTVRSetBackBufferPrefs called");
    return NULL;
}

void* QTVRSetConstraints(void)
{
    if (verbose) puts("STUB: QTVRSetConstraints called");
    return NULL;
}

void* QTVRSetControlSetting(void)
{
    if (verbose) puts("STUB: QTVRSetControlSetting called");
    return NULL;
}

void* QTVRSetEnteringNodeProc(void)
{
    if (verbose) puts("STUB: QTVRSetEnteringNodeProc called");
    return NULL;
}

void* QTVRSetFieldOfView(void)
{
    if (verbose) puts("STUB: QTVRSetFieldOfView called");
    return NULL;
}

void* QTVRSetFrameRate(void)
{
    if (verbose) puts("STUB: QTVRSetFrameRate called");
    return NULL;
}

void* QTVRSetImagingProperty(void)
{
    if (verbose) puts("STUB: QTVRSetImagingProperty called");
    return NULL;
}

void* QTVRSetInteractionProperty(void)
{
    if (verbose) puts("STUB: QTVRSetInteractionProperty called");
    return NULL;
}

void* QTVRSetLeavingNodeProc(void)
{
    if (verbose) puts("STUB: QTVRSetLeavingNodeProc called");
    return NULL;
}

void* QTVRSetMouseDownTracking(void)
{
    if (verbose) puts("STUB: QTVRSetMouseDownTracking called");
    return NULL;
}

void* QTVRSetMouseOverHotSpotProc(void)
{
    if (verbose) puts("STUB: QTVRSetMouseOverHotSpotProc called");
    return NULL;
}

void* QTVRSetMouseOverTracking(void)
{
    if (verbose) puts("STUB: QTVRSetMouseOverTracking called");
    return NULL;
}

void* QTVRSetPanAngle(void)
{
    if (verbose) puts("STUB: QTVRSetPanAngle called");
    return NULL;
}

void* QTVRSetPrescreenImagingCompleteProc(void)
{
    if (verbose) puts("STUB: QTVRSetPrescreenImagingCompleteProc called");
    return NULL;
}

void* QTVRSetTiltAngle(void)
{
    if (verbose) puts("STUB: QTVRSetTiltAngle called");
    return NULL;
}

void* QTVRSetTransitionProperty(void)
{
    if (verbose) puts("STUB: QTVRSetTransitionProperty called");
    return NULL;
}

void* QTVRSetViewCenter(void)
{
    if (verbose) puts("STUB: QTVRSetViewCenter called");
    return NULL;
}

void* QTVRSetViewCurrentTime(void)
{
    if (verbose) puts("STUB: QTVRSetViewCurrentTime called");
    return NULL;
}

void* QTVRSetViewParameter(void)
{
    if (verbose) puts("STUB: QTVRSetViewParameter called");
    return NULL;
}

void* QTVRSetViewRate(void)
{
    if (verbose) puts("STUB: QTVRSetViewRate called");
    return NULL;
}

void* QTVRSetViewState(void)
{
    if (verbose) puts("STUB: QTVRSetViewState called");
    return NULL;
}

void* QTVRSetVisible(void)
{
    if (verbose) puts("STUB: QTVRSetVisible called");
    return NULL;
}

void* QTVRShowDefaultView(void)
{
    if (verbose) puts("STUB: QTVRShowDefaultView called");
    return NULL;
}

void* QTVRTiltToRow(void)
{
    if (verbose) puts("STUB: QTVRTiltToRow called");
    return NULL;
}

void* QTVRTriggerHotSpot(void)
{
    if (verbose) puts("STUB: QTVRTriggerHotSpot called");
    return NULL;
}

void* QTVRUpdate(void)
{
    if (verbose) puts("STUB: QTVRUpdate called");
    return NULL;
}

void* QTVRWrapAndConstrain(void)
{
    if (verbose) puts("STUB: QTVRWrapAndConstrain called");
    return NULL;
}

void* QTVUAttributeChanged(void)
{
    if (verbose) puts("STUB: QTVUAttributeChanged called");
    return NULL;
}

void* QTVUCreateInputVisualContext(void)
{
    if (verbose) puts("STUB: QTVUCreateInputVisualContext called");
    return NULL;
}

void* QTVUGenerateImage(void)
{
    if (verbose) puts("STUB: QTVUGenerateImage called");
    return NULL;
}

void* QTVUGetAttribute(void)
{
    if (verbose) puts("STUB: QTVUGetAttribute called");
    return NULL;
}

void* QTVUGetOutputVisualContext(void)
{
    if (verbose) puts("STUB: QTVUGetOutputVisualContext called");
    return NULL;
}

void* QTVUGetTrackingCallback(void)
{
    if (verbose) puts("STUB: QTVUGetTrackingCallback called");
    return NULL;
}

void* QTVUGrabConfiguringMutex(void)
{
    if (verbose) puts("STUB: QTVUGrabConfiguringMutex called");
    return NULL;
}

void* QTVUGrabProcessingMutex(void)
{
    if (verbose) puts("STUB: QTVUGrabProcessingMutex called");
    return NULL;
}

void* QTVUInitialize(void)
{
    if (verbose) puts("STUB: QTVUInitialize called");
    return NULL;
}

void* QTVUIsNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVUIsNewImageAvailable called");
    return NULL;
}

void* QTVUNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVUNewImageAvailable called");
    return NULL;
}

void* QTVUPortAddPropertyListener(void)
{
    if (verbose) puts("STUB: QTVUPortAddPropertyListener called");
    return NULL;
}

void* QTVUPortGetProperty(void)
{
    if (verbose) puts("STUB: QTVUPortGetProperty called");
    return NULL;
}

void* QTVUPortGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTVUPortGetPropertyInfo called");
    return NULL;
}

void* QTVUPortRemovePropertyListener(void)
{
    if (verbose) puts("STUB: QTVUPortRemovePropertyListener called");
    return NULL;
}

void* QTVUPortSetProperty(void)
{
    if (verbose) puts("STUB: QTVUPortSetProperty called");
    return NULL;
}

void* QTVUReleaseInputVisualContext(void)
{
    if (verbose) puts("STUB: QTVUReleaseInputVisualContext called");
    return NULL;
}

void* QTVUReturnConfiguringMutex(void)
{
    if (verbose) puts("STUB: QTVUReturnConfiguringMutex called");
    return NULL;
}

void* QTVUReturnProcessingMutex(void)
{
    if (verbose) puts("STUB: QTVUReturnProcessingMutex called");
    return NULL;
}

void* QTVUSetAttributeChangedCallback(void)
{
    if (verbose) puts("STUB: QTVUSetAttributeChangedCallback called");
    return NULL;
}

void* QTVUSetOutputVisualContext(void)
{
    if (verbose) puts("STUB: QTVUSetOutputVisualContext called");
    return NULL;
}

void* QTVUSetPassThruVisualContext(void)
{
    if (verbose) puts("STUB: QTVUSetPassThruVisualContext called");
    return NULL;
}

void* QTVUSetTrackingCallback(void)
{
    if (verbose) puts("STUB: QTVUSetTrackingCallback called");
    return NULL;
}

void* QTValidateAtomContainer(void)
{
    if (verbose) puts("STUB: QTValidateAtomContainer called");
    return NULL;
}

void* QTValidateAudioChannelLayout(void)
{
    if (verbose) puts("STUB: QTValidateAudioChannelLayout called");
    return NULL;
}

void* QTVideoOutputBaseSetEchoPort(void)
{
    if (verbose) puts("STUB: QTVideoOutputBaseSetEchoPort called");
    return NULL;
}

void* QTVideoOutputBegin(void)
{
    if (verbose) puts("STUB: QTVideoOutputBegin called");
    return NULL;
}

void* QTVideoOutputCopyIndAudioOutputDeviceUID(void)
{
    if (verbose) puts("STUB: QTVideoOutputCopyIndAudioOutputDeviceUID called");
    return NULL;
}

void* QTVideoOutputCustomConfigureDisplay(void)
{
    if (verbose) puts("STUB: QTVideoOutputCustomConfigureDisplay called");
    return NULL;
}

void* QTVideoOutputEnd(void)
{
    if (verbose) puts("STUB: QTVideoOutputEnd called");
    return NULL;
}

void* QTVideoOutputGetClientName(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetClientName called");
    return NULL;
}

void* QTVideoOutputGetClock(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetClock called");
    return NULL;
}

void* QTVideoOutputGetCurrentClientName(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetCurrentClientName called");
    return NULL;
}

void* QTVideoOutputGetDisplayMode(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetDisplayMode called");
    return NULL;
}

void* QTVideoOutputGetDisplayModeList(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetDisplayModeList called");
    return NULL;
}

void* QTVideoOutputGetGWorld(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetGWorld called");
    return NULL;
}

void* QTVideoOutputGetGWorldParameters(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetGWorldParameters called");
    return NULL;
}

void* QTVideoOutputGetIndImageDecompressor(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetIndImageDecompressor called");
    return NULL;
}

void* QTVideoOutputGetIndSoundOutput(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetIndSoundOutput called");
    return NULL;
}

void* QTVideoOutputGetVideoOutputTransferCodecList(void)
{
    if (verbose) puts("STUB: QTVideoOutputGetVideoOutputTransferCodecList called");
    return NULL;
}

void* QTVideoOutputPostSetEchoPort(void)
{
    if (verbose) puts("STUB: QTVideoOutputPostSetEchoPort called");
    return NULL;
}

void* QTVideoOutputPreSetEchoPort(void)
{
    if (verbose) puts("STUB: QTVideoOutputPreSetEchoPort called");
    return NULL;
}

void* QTVideoOutputRestoreState(void)
{
    if (verbose) puts("STUB: QTVideoOutputRestoreState called");
    return NULL;
}

void* QTVideoOutputSaveState(void)
{
    if (verbose) puts("STUB: QTVideoOutputSaveState called");
    return NULL;
}

void* QTVideoOutputSetClientName(void)
{
    if (verbose) puts("STUB: QTVideoOutputSetClientName called");
    return NULL;
}

void* QTVideoOutputSetDisplayMode(void)
{
    if (verbose) puts("STUB: QTVideoOutputSetDisplayMode called");
    return NULL;
}

void* QTVideoOutputSetEchoPort(void)
{
    if (verbose) puts("STUB: QTVideoOutputSetEchoPort called");
    return NULL;
}

void* QTVisualContextAddPropertyListener(void)
{
    if (verbose) puts("STUB: QTVisualContextAddPropertyListener called");
    return NULL;
}

void* QTVisualContextAttributeChanged(void)
{
    if (verbose) puts("STUB: QTVisualContextAttributeChanged called");
    return NULL;
}

void* QTVisualContextCopyImageForTime(void)
{
    if (verbose) puts("STUB: QTVisualContextCopyImageForTime called");
    return NULL;
}

void* QTVisualContextCopyPixelBufferOptions(void)
{
    if (verbose) puts("STUB: QTVisualContextCopyPixelBufferOptions called");
    return NULL;
}

void* QTVisualContextCreate(void)
{
    if (verbose) puts("STUB: QTVisualContextCreate called");
    return NULL;
}

void* QTVisualContextGetAttribute(void)
{
    if (verbose) puts("STUB: QTVisualContextGetAttribute called");
    return NULL;
}

void* QTVisualContextGetExpectedReadAhead(void)
{
    if (verbose) puts("STUB: QTVisualContextGetExpectedReadAhead called");
    return NULL;
}

void* QTVisualContextGetProperty(void)
{
    if (verbose) puts("STUB: QTVisualContextGetProperty called");
    return NULL;
}

void* QTVisualContextGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTVisualContextGetPropertyInfo called");
    return NULL;
}

void* QTVisualContextGetTypeID(void)
{
    if (verbose) puts("STUB: QTVisualContextGetTypeID called");
    return NULL;
}

void* QTVisualContextGetVisualGraph(void)
{
    if (verbose) puts("STUB: QTVisualContextGetVisualGraph called");
    return NULL;
}

void* QTVisualContextIsNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVisualContextIsNewImageAvailable called");
    return NULL;
}

void* QTVisualContextNewImageAvailable(void)
{
    if (verbose) puts("STUB: QTVisualContextNewImageAvailable called");
    return NULL;
}

void* QTVisualContextRelease(void)
{
    if (verbose) puts("STUB: QTVisualContextRelease called");
    return NULL;
}

void* QTVisualContextRemovePropertyListener(void)
{
    if (verbose) puts("STUB: QTVisualContextRemovePropertyListener called");
    return NULL;
}

void* QTVisualContextRetain(void)
{
    if (verbose) puts("STUB: QTVisualContextRetain called");
    return NULL;
}

void* QTVisualContextSetAttribute(void)
{
    if (verbose) puts("STUB: QTVisualContextSetAttribute called");
    return NULL;
}

void* QTVisualContextSetAttributeChangedCallback(void)
{
    if (verbose) puts("STUB: QTVisualContextSetAttributeChangedCallback called");
    return NULL;
}

void* QTVisualContextSetGenerateImageCallback(void)
{
    if (verbose) puts("STUB: QTVisualContextSetGenerateImageCallback called");
    return NULL;
}

void* QTVisualContextSetGetAttributeCallback(void)
{
    if (verbose) puts("STUB: QTVisualContextSetGetAttributeCallback called");
    return NULL;
}

void* QTVisualContextSetImageAvailableCallback(void)
{
    if (verbose) puts("STUB: QTVisualContextSetImageAvailableCallback called");
    return NULL;
}

void* QTVisualContextSetIsNewImageAvailableCallback(void)
{
    if (verbose) puts("STUB: QTVisualContextSetIsNewImageAvailableCallback called");
    return NULL;
}

void* QTVisualContextSetProperty(void)
{
    if (verbose) puts("STUB: QTVisualContextSetProperty called");
    return NULL;
}

void* QTVisualContextSetVisualGraph(void)
{
    if (verbose) puts("STUB: QTVisualContextSetVisualGraph called");
    return NULL;
}

void* QTVisualContextTask(void)
{
    if (verbose) puts("STUB: QTVisualContextTask called");
    return NULL;
}

void* QTVisualGraphCreate(void)
{
    if (verbose) puts("STUB: QTVisualGraphCreate called");
    return NULL;
}

void* QTVisualGraphGetTypeID(void)
{
    if (verbose) puts("STUB: QTVisualGraphGetTypeID called");
    return NULL;
}

void* QTVisualGraphGrabConfiguringMutex(void)
{
    if (verbose) puts("STUB: QTVisualGraphGrabConfiguringMutex called");
    return NULL;
}

void* QTVisualGraphGrabProcessingMutex(void)
{
    if (verbose) puts("STUB: QTVisualGraphGrabProcessingMutex called");
    return NULL;
}

void* QTVisualGraphRelease(void)
{
    if (verbose) puts("STUB: QTVisualGraphRelease called");
    return NULL;
}

void* QTVisualGraphRetain(void)
{
    if (verbose) puts("STUB: QTVisualGraphRetain called");
    return NULL;
}

void* QTVisualGraphReturnConfiguringMutex(void)
{
    if (verbose) puts("STUB: QTVisualGraphReturnConfiguringMutex called");
    return NULL;
}

void* QTVisualGraphReturnProcessingMutex(void)
{
    if (verbose) puts("STUB: QTVisualGraphReturnProcessingMutex called");
    return NULL;
}

void* QTWorkContextCopyForMediaContextID(void)
{
    if (verbose) puts("STUB: QTWorkContextCopyForMediaContextID called");
    return NULL;
}

void* QTWorkContextCreate(void)
{
    if (verbose) puts("STUB: QTWorkContextCreate called");
    return NULL;
}

void* QTWorkContextGetProperty(void)
{
    if (verbose) puts("STUB: QTWorkContextGetProperty called");
    return NULL;
}

void* QTWorkContextGetPropertyInfo(void)
{
    if (verbose) puts("STUB: QTWorkContextGetPropertyInfo called");
    return NULL;
}

void* QTWorkContextGetReleaseInfoForCFType(void)
{
    if (verbose) puts("STUB: QTWorkContextGetReleaseInfoForCFType called");
    return NULL;
}

void* QTWorkContextGetReleaseInfoForHandle(void)
{
    if (verbose) puts("STUB: QTWorkContextGetReleaseInfoForHandle called");
    return NULL;
}

void* QTWorkContextGetReleaseInfoForQTCalloc(void)
{
    if (verbose) puts("STUB: QTWorkContextGetReleaseInfoForQTCalloc called");
    return NULL;
}

void* QTWorkContextGetTypeID(void)
{
    if (verbose) puts("STUB: QTWorkContextGetTypeID called");
    return NULL;
}

void* QTWorkContextGetValue(void)
{
    if (verbose) puts("STUB: QTWorkContextGetValue called");
    return NULL;
}

void* QTWorkContextHasValue(void)
{
    if (verbose) puts("STUB: QTWorkContextHasValue called");
    return NULL;
}

void* QTWorkContextRelease(void)
{
    if (verbose) puts("STUB: QTWorkContextRelease called");
    return NULL;
}

void* QTWorkContextRetain(void)
{
    if (verbose) puts("STUB: QTWorkContextRetain called");
    return NULL;
}

void* QTWorkContextSetProperty(void)
{
    if (verbose) puts("STUB: QTWorkContextSetProperty called");
    return NULL;
}

void* QTWorkContextSetValue(void)
{
    if (verbose) puts("STUB: QTWorkContextSetValue called");
    return NULL;
}

void* QTrHandToHand(void)
{
    if (verbose) puts("STUB: QTrHandToHand called");
    return NULL;
}

void* QTsyscall(void)
{
    if (verbose) puts("STUB: QTsyscall called");
    return NULL;
}

void* QuadToQuadMatrix(void)
{
    if (verbose) puts("STUB: QuadToQuadMatrix called");
    return NULL;
}

void* QueueSyncTask(void)
{
    if (verbose) puts("STUB: QueueSyncTask called");
    return NULL;
}

void* QueueSyncTaskOnTimeBaseThread(void)
{
    if (verbose) puts("STUB: QueueSyncTaskOnTimeBaseThread called");
    return NULL;
}

void* QuickTimeExtensionLoaded(void)
{
    if (verbose) puts("STUB: QuickTimeExtensionLoaded called");
    return NULL;
}

void* RTPMPAddLeafSettings(void)
{
    if (verbose) puts("STUB: RTPMPAddLeafSettings called");
    return NULL;
}

void* RTPMPDoUserDialog(void)
{
    if (verbose) puts("STUB: RTPMPDoUserDialog called");
    return NULL;
}

void* RTPMPFlush(void)
{
    if (verbose) puts("STUB: RTPMPFlush called");
    return NULL;
}

void* RTPMPGetInfo(void)
{
    if (verbose) puts("STUB: RTPMPGetInfo called");
    return NULL;
}

void* RTPMPGetMaxPacketDuration(void)
{
    if (verbose) puts("STUB: RTPMPGetMaxPacketDuration called");
    return NULL;
}

void* RTPMPGetMaxPacketSize(void)
{
    if (verbose) puts("STUB: RTPMPGetMaxPacketSize called");
    return NULL;
}

void* RTPMPGetMediaType(void)
{
    if (verbose) puts("STUB: RTPMPGetMediaType called");
    return NULL;
}

void* RTPMPGetPacketBuilder(void)
{
    if (verbose) puts("STUB: RTPMPGetPacketBuilder called");
    return NULL;
}

void* RTPMPGetPayloadRegistrationFromResource(void)
{
    if (verbose) puts("STUB: RTPMPGetPayloadRegistrationFromResource called");
    return NULL;
}

void* RTPMPGetSettings(void)
{
    if (verbose) puts("STUB: RTPMPGetSettings called");
    return NULL;
}

void* RTPMPGetSettingsAsText(void)
{
    if (verbose) puts("STUB: RTPMPGetSettingsAsText called");
    return NULL;
}

void* RTPMPGetSettingsIntoAtomContainerAtAtom(void)
{
    if (verbose) puts("STUB: RTPMPGetSettingsIntoAtomContainerAtAtom called");
    return NULL;
}

void* RTPMPGetTimeBase(void)
{
    if (verbose) puts("STUB: RTPMPGetTimeBase called");
    return NULL;
}

void* RTPMPGetTimeScale(void)
{
    if (verbose) puts("STUB: RTPMPGetTimeScale called");
    return NULL;
}

void* RTPMPHasCharacteristic(void)
{
    if (verbose) puts("STUB: RTPMPHasCharacteristic called");
    return NULL;
}

void* RTPMPIdle(void)
{
    if (verbose) puts("STUB: RTPMPIdle called");
    return NULL;
}

void* RTPMPInitialize(void)
{
    if (verbose) puts("STUB: RTPMPInitialize called");
    return NULL;
}

void* RTPMPPreflightMedia(void)
{
    if (verbose) puts("STUB: RTPMPPreflightMedia called");
    return NULL;
}

void* RTPMPReset(void)
{
    if (verbose) puts("STUB: RTPMPReset called");
    return NULL;
}

void* RTPMPSetInfo(void)
{
    if (verbose) puts("STUB: RTPMPSetInfo called");
    return NULL;
}

void* RTPMPSetLeafSettings(void)
{
    if (verbose) puts("STUB: RTPMPSetLeafSettings called");
    return NULL;
}

void* RTPMPSetMaxPacketDuration(void)
{
    if (verbose) puts("STUB: RTPMPSetMaxPacketDuration called");
    return NULL;
}

void* RTPMPSetMaxPacketSize(void)
{
    if (verbose) puts("STUB: RTPMPSetMaxPacketSize called");
    return NULL;
}

void* RTPMPSetMediaType(void)
{
    if (verbose) puts("STUB: RTPMPSetMediaType called");
    return NULL;
}

void* RTPMPSetPacketBuilder(void)
{
    if (verbose) puts("STUB: RTPMPSetPacketBuilder called");
    return NULL;
}

void* RTPMPSetSampleData(void)
{
    if (verbose) puts("STUB: RTPMPSetSampleData called");
    return NULL;
}

void* RTPMPSetSettings(void)
{
    if (verbose) puts("STUB: RTPMPSetSettings called");
    return NULL;
}

void* RTPMPSetSettingsFromAtomContainerAtAtom(void)
{
    if (verbose) puts("STUB: RTPMPSetSettingsFromAtomContainerAtAtom called");
    return NULL;
}

void* RTPMPSetTimeBase(void)
{
    if (verbose) puts("STUB: RTPMPSetTimeBase called");
    return NULL;
}

void* RTPMPSetTimeScale(void)
{
    if (verbose) puts("STUB: RTPMPSetTimeScale called");
    return NULL;
}

void* RTPPBAddPacketLiteralData(void)
{
    if (verbose) puts("STUB: RTPPBAddPacketLiteralData called");
    return NULL;
}

void* RTPPBAddPacketRepeatedData(void)
{
    if (verbose) puts("STUB: RTPPBAddPacketRepeatedData called");
    return NULL;
}

void* RTPPBAddPacketSampleData(void)
{
    if (verbose) puts("STUB: RTPPBAddPacketSampleData called");
    return NULL;
}

void* RTPPBAddPacketSampleData64(void)
{
    if (verbose) puts("STUB: RTPPBAddPacketSampleData64 called");
    return NULL;
}

void* RTPPBAddRepeatPacket(void)
{
    if (verbose) puts("STUB: RTPPBAddRepeatPacket called");
    return NULL;
}

void* RTPPBBeginPacket(void)
{
    if (verbose) puts("STUB: RTPPBBeginPacket called");
    return NULL;
}

void* RTPPBBeginPacketGroup(void)
{
    if (verbose) puts("STUB: RTPPBBeginPacketGroup called");
    return NULL;
}

void* RTPPBEndPacket(void)
{
    if (verbose) puts("STUB: RTPPBEndPacket called");
    return NULL;
}

void* RTPPBEndPacketGroup(void)
{
    if (verbose) puts("STUB: RTPPBEndPacketGroup called");
    return NULL;
}

void* RTPPBGetCallback(void)
{
    if (verbose) puts("STUB: RTPPBGetCallback called");
    return NULL;
}

void* RTPPBGetInfo(void)
{
    if (verbose) puts("STUB: RTPPBGetInfo called");
    return NULL;
}

void* RTPPBGetPacketSequenceNumber(void)
{
    if (verbose) puts("STUB: RTPPBGetPacketSequenceNumber called");
    return NULL;
}

void* RTPPBGetPacketTimeStampOffset(void)
{
    if (verbose) puts("STUB: RTPPBGetPacketTimeStampOffset called");
    return NULL;
}

void* RTPPBGetSampleData(void)
{
    if (verbose) puts("STUB: RTPPBGetSampleData called");
    return NULL;
}

void* RTPPBReleaseRepeatedData(void)
{
    if (verbose) puts("STUB: RTPPBReleaseRepeatedData called");
    return NULL;
}

void* RTPPBSetCallback(void)
{
    if (verbose) puts("STUB: RTPPBSetCallback called");
    return NULL;
}

void* RTPPBSetInfo(void)
{
    if (verbose) puts("STUB: RTPPBSetInfo called");
    return NULL;
}

void* RTPPBSetPacketSequenceNumber(void)
{
    if (verbose) puts("STUB: RTPPBSetPacketSequenceNumber called");
    return NULL;
}

void* RTPPBSetPacketTimeStampOffset(void)
{
    if (verbose) puts("STUB: RTPPBSetPacketTimeStampOffset called");
    return NULL;
}

void* RTPPayloadMapGetDynamicMappingAsContainer(void)
{
    if (verbose) puts("STUB: RTPPayloadMapGetDynamicMappingAsContainer called");
    return NULL;
}

void* RTPPayloadMapGetInfo(void)
{
    if (verbose) puts("STUB: RTPPayloadMapGetInfo called");
    return NULL;
}

void* RTPPayloadMapGetPayloadComponent(void)
{
    if (verbose) puts("STUB: RTPPayloadMapGetPayloadComponent called");
    return NULL;
}

void* RTPPayloadMapInitialize(void)
{
    if (verbose) puts("STUB: RTPPayloadMapInitialize called");
    return NULL;
}

void* RTPPayloadMapSetDynamicMapping(void)
{
    if (verbose) puts("STUB: RTPPayloadMapSetDynamicMapping called");
    return NULL;
}

void* RTPPayloadMapSetDynamicMappingFromContainer(void)
{
    if (verbose) puts("STUB: RTPPayloadMapSetDynamicMappingFromContainer called");
    return NULL;
}

void* RTPPayloadMapSetInfo(void)
{
    if (verbose) puts("STUB: RTPPayloadMapSetInfo called");
    return NULL;
}

void* RTPRssmAdjustPacketParams(void)
{
    if (verbose) puts("STUB: RTPRssmAdjustPacketParams called");
    return NULL;
}

void* RTPRssmClearCachedPackets(void)
{
    if (verbose) puts("STUB: RTPRssmClearCachedPackets called");
    return NULL;
}

void* RTPRssmComputeChunkSize(void)
{
    if (verbose) puts("STUB: RTPRssmComputeChunkSize called");
    return NULL;
}

void* RTPRssmCopyDataToChunk(void)
{
    if (verbose) puts("STUB: RTPRssmCopyDataToChunk called");
    return NULL;
}

void* RTPRssmDecrChunkRefCount(void)
{
    if (verbose) puts("STUB: RTPRssmDecrChunkRefCount called");
    return NULL;
}

void* RTPRssmFillPacketListParams(void)
{
    if (verbose) puts("STUB: RTPRssmFillPacketListParams called");
    return NULL;
}

void* RTPRssmGetCapabilities(void)
{
    if (verbose) puts("STUB: RTPRssmGetCapabilities called");
    return NULL;
}

void* RTPRssmGetChunkAndIncrRefCount(void)
{
    if (verbose) puts("STUB: RTPRssmGetChunkAndIncrRefCount called");
    return NULL;
}

void* RTPRssmGetExtChunkAndIncrRefCount(void)
{
    if (verbose) puts("STUB: RTPRssmGetExtChunkAndIncrRefCount called");
    return NULL;
}

void* RTPRssmGetInfo(void)
{
    if (verbose) puts("STUB: RTPRssmGetInfo called");
    return NULL;
}

void* RTPRssmGetPacketDensityFromPacket(void)
{
    if (verbose) puts("STUB: RTPRssmGetPacketDensityFromPacket called");
    return NULL;
}

void* RTPRssmGetPayloadHeaderLength(void)
{
    if (verbose) puts("STUB: RTPRssmGetPayloadHeaderLength called");
    return NULL;
}

void* RTPRssmGetPayloadRegistrationFromResource(void)
{
    if (verbose) puts("STUB: RTPRssmGetPayloadRegistrationFromResource called");
    return NULL;
}

void* RTPRssmGetStreamHandler(void)
{
    if (verbose) puts("STUB: RTPRssmGetStreamHandler called");
    return NULL;
}

void* RTPRssmGetTimeScale(void)
{
    if (verbose) puts("STUB: RTPRssmGetTimeScale called");
    return NULL;
}

void* RTPRssmGetTimeScaleFromPacket(void)
{
    if (verbose) puts("STUB: RTPRssmGetTimeScaleFromPacket called");
    return NULL;
}

void* RTPRssmHandleNewPacket(void)
{
    if (verbose) puts("STUB: RTPRssmHandleNewPacket called");
    return NULL;
}

void* RTPRssmHasCharacteristic(void)
{
    if (verbose) puts("STUB: RTPRssmHasCharacteristic called");
    return NULL;
}

void* RTPRssmIncrChunkRefCount(void)
{
    if (verbose) puts("STUB: RTPRssmIncrChunkRefCount called");
    return NULL;
}

void* RTPRssmInitialize(void)
{
    if (verbose) puts("STUB: RTPRssmInitialize called");
    return NULL;
}

void* RTPRssmNewStreamHandler(void)
{
    if (verbose) puts("STUB: RTPRssmNewStreamHandler called");
    return NULL;
}

void* RTPRssmReleasePacketList(void)
{
    if (verbose) puts("STUB: RTPRssmReleasePacketList called");
    return NULL;
}

void* RTPRssmReset(void)
{
    if (verbose) puts("STUB: RTPRssmReset called");
    return NULL;
}

void* RTPRssmSendChunkAndDecrRefCount(void)
{
    if (verbose) puts("STUB: RTPRssmSendChunkAndDecrRefCount called");
    return NULL;
}

void* RTPRssmSendLostChunk(void)
{
    if (verbose) puts("STUB: RTPRssmSendLostChunk called");
    return NULL;
}

void* RTPRssmSendPacketList(void)
{
    if (verbose) puts("STUB: RTPRssmSendPacketList called");
    return NULL;
}

void* RTPRssmSendStreamBufferRange(void)
{
    if (verbose) puts("STUB: RTPRssmSendStreamBufferRange called");
    return NULL;
}

void* RTPRssmSendStreamHandlerChanged(void)
{
    if (verbose) puts("STUB: RTPRssmSendStreamHandlerChanged called");
    return NULL;
}

void* RTPRssmSetCapabilities(void)
{
    if (verbose) puts("STUB: RTPRssmSetCapabilities called");
    return NULL;
}

void* RTPRssmSetInfo(void)
{
    if (verbose) puts("STUB: RTPRssmSetInfo called");
    return NULL;
}

void* RTPRssmSetPayloadHeaderLength(void)
{
    if (verbose) puts("STUB: RTPRssmSetPayloadHeaderLength called");
    return NULL;
}

void* RTPRssmSetSampleDescription(void)
{
    if (verbose) puts("STUB: RTPRssmSetSampleDescription called");
    return NULL;
}

void* RTPRssmSetStreamHandler(void)
{
    if (verbose) puts("STUB: RTPRssmSetStreamHandler called");
    return NULL;
}

void* RTPRssmSetTimeScale(void)
{
    if (verbose) puts("STUB: RTPRssmSetTimeScale called");
    return NULL;
}

void* RectMatrix(void)
{
    if (verbose) puts("STUB: RectMatrix called");
    return NULL;
}

void* ReleaseQuickTimePreferences(void)
{
    if (verbose) puts("STUB: ReleaseQuickTimePreferences called");
    return NULL;
}

void* ReleaseQuickTimePreferencesForReading(void)
{
    if (verbose) puts("STUB: ReleaseQuickTimePreferencesForReading called");
    return NULL;
}

void* ReleaseQuickTimePreferencesForWriting(void)
{
    if (verbose) puts("STUB: ReleaseQuickTimePreferencesForWriting called");
    return NULL;
}

void* RemoveCallBackFromTimeBase(void)
{
    if (verbose) puts("STUB: RemoveCallBackFromTimeBase called");
    return NULL;
}

void* RemoveImageDescriptionExtension(void)
{
    if (verbose) puts("STUB: RemoveImageDescriptionExtension called");
    return NULL;
}

void* RemoveMovieApertureModeDimensions(void)
{
    if (verbose) puts("STUB: RemoveMovieApertureModeDimensions called");
    return NULL;
}

void* RemoveMovieExecuteWiredActionsProc(void)
{
    if (verbose) puts("STUB: RemoveMovieExecuteWiredActionsProc called");
    return NULL;
}

void* RemoveMovieFromStorage(void)
{
    if (verbose) puts("STUB: RemoveMovieFromStorage called");
    return NULL;
}

void* RemoveMovieResource(void)
{
    if (verbose) puts("STUB: RemoveMovieResource called");
    return NULL;
}

void* RemoveSoundDescriptionExtension(void)
{
    if (verbose) puts("STUB: RemoveSoundDescriptionExtension called");
    return NULL;
}

void* RemoveTrackApertureModeDimensions(void)
{
    if (verbose) puts("STUB: RemoveTrackApertureModeDimensions called");
    return NULL;
}

void* RemoveUserData(void)
{
    if (verbose) puts("STUB: RemoveUserData called");
    return NULL;
}

void* RemoveUserDataText(void)
{
    if (verbose) puts("STUB: RemoveUserDataText called");
    return NULL;
}

void* ReplaceDSequenceImageDescription(void)
{
    if (verbose) puts("STUB: ReplaceDSequenceImageDescription called");
    return NULL;
}

void* ReplyComponentMessage(void)
{
    if (verbose) puts("STUB: ReplyComponentMessage called");
    return NULL;
}

void* ResetComponentMessage(void)
{
    if (verbose) puts("STUB: ResetComponentMessage called");
    return NULL;
}

void* RotateMatrix(void)
{
    if (verbose) puts("STUB: RotateMatrix called");
    return NULL;
}

void* SCAddSpecial(void)
{
    if (verbose) puts("STUB: SCAddSpecial called");
    return NULL;
}

void* SCAsyncIdle(void)
{
    if (verbose) puts("STUB: SCAsyncIdle called");
    return NULL;
}

void* SCAudioFillBuffer(void)
{
    if (verbose) puts("STUB: SCAudioFillBuffer called");
    return NULL;
}

void* SCAudioInvokeLegacyCodecOptionsDialog(void)
{
    if (verbose) puts("STUB: SCAudioInvokeLegacyCodecOptionsDialog called");
    return NULL;
}

void* SCAudioReset(void)
{
    if (verbose) puts("STUB: SCAudioReset called");
    return NULL;
}

void* SCCompressImage(void)
{
    if (verbose) puts("STUB: SCCompressImage called");
    return NULL;
}

void* SCCompressPicture(void)
{
    if (verbose) puts("STUB: SCCompressPicture called");
    return NULL;
}

void* SCCompressPictureFile(void)
{
    if (verbose) puts("STUB: SCCompressPictureFile called");
    return NULL;
}

void* SCCompressSequenceBegin(void)
{
    if (verbose) puts("STUB: SCCompressSequenceBegin called");
    return NULL;
}

void* SCCompressSequenceEnd(void)
{
    if (verbose) puts("STUB: SCCompressSequenceEnd called");
    return NULL;
}

void* SCCompressSequenceFrame(void)
{
    if (verbose) puts("STUB: SCCompressSequenceFrame called");
    return NULL;
}

void* SCCompressSequenceFrameAsync(void)
{
    if (verbose) puts("STUB: SCCompressSequenceFrameAsync called");
    return NULL;
}

void* SCCopyCompressionSessionOptions(void)
{
    if (verbose) puts("STUB: SCCopyCompressionSessionOptions called");
    return NULL;
}

void* SCCopySettingsAsCFString(void)
{
    if (verbose) puts("STUB: SCCopySettingsAsCFString called");
    return NULL;
}

void* SCDefaultPictFileSettings(void)
{
    if (verbose) puts("STUB: SCDefaultPictFileSettings called");
    return NULL;
}

void* SCDefaultPictHandleSettings(void)
{
    if (verbose) puts("STUB: SCDefaultPictHandleSettings called");
    return NULL;
}

void* SCDefaultPixMapSettings(void)
{
    if (verbose) puts("STUB: SCDefaultPixMapSettings called");
    return NULL;
}

void* SCEvent(void)
{
    if (verbose) puts("STUB: SCEvent called");
    return NULL;
}

void* SCGetBestDeviceRect(void)
{
    if (verbose) puts("STUB: SCGetBestDeviceRect called");
    return NULL;
}

void* SCGetCompressFlags(void)
{
    if (verbose) puts("STUB: SCGetCompressFlags called");
    return NULL;
}

void* SCGetCompressionExtended(void)
{
    if (verbose) puts("STUB: SCGetCompressionExtended called");
    return NULL;
}

void* SCGetInfo(void)
{
    if (verbose) puts("STUB: SCGetInfo called");
    return NULL;
}

void* SCGetSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: SCGetSettingsAsAtomContainer called");
    return NULL;
}

void* SCGetSettingsAsText(void)
{
    if (verbose) puts("STUB: SCGetSettingsAsText called");
    return NULL;
}

void* SCItem(void)
{
    if (verbose) puts("STUB: SCItem called");
    return NULL;
}

void* SCNewGWorld(void)
{
    if (verbose) puts("STUB: SCNewGWorld called");
    return NULL;
}

void* SCPanelCanRun(void)
{
    if (verbose) puts("STUB: SCPanelCanRun called");
    return NULL;
}

void* SCPanelGetDITLForSize(void)
{
    if (verbose) puts("STUB: SCPanelGetDITLForSize called");
    return NULL;
}

void* SCPanelGetDitl(void)
{
    if (verbose) puts("STUB: SCPanelGetDitl called");
    return NULL;
}

void* SCPanelGetSettings(void)
{
    if (verbose) puts("STUB: SCPanelGetSettings called");
    return NULL;
}

void* SCPanelInstall(void)
{
    if (verbose) puts("STUB: SCPanelInstall called");
    return NULL;
}

void* SCPanelSetEventFilter(void)
{
    if (verbose) puts("STUB: SCPanelSetEventFilter called");
    return NULL;
}

void* SCPanelSetSettings(void)
{
    if (verbose) puts("STUB: SCPanelSetSettings called");
    return NULL;
}

void* SCPanelValidateInput(void)
{
    if (verbose) puts("STUB: SCPanelValidateInput called");
    return NULL;
}

void* SCPositionDialog(void)
{
    if (verbose) puts("STUB: SCPositionDialog called");
    return NULL;
}

void* SCPositionRect(void)
{
    if (verbose) puts("STUB: SCPositionRect called");
    return NULL;
}

void* SCRemove(void)
{
    if (verbose) puts("STUB: SCRemove called");
    return NULL;
}

void* SCRequestImageSettings(void)
{
    if (verbose) puts("STUB: SCRequestImageSettings called");
    return NULL;
}

void* SCRequestSequenceSettings(void)
{
    if (verbose) puts("STUB: SCRequestSequenceSettings called");
    return NULL;
}

void* SCSetCompressFlags(void)
{
    if (verbose) puts("STUB: SCSetCompressFlags called");
    return NULL;
}

void* SCSetInfo(void)
{
    if (verbose) puts("STUB: SCSetInfo called");
    return NULL;
}

void* SCSetSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: SCSetSettingsFromAtomContainer called");
    return NULL;
}

void* SCSetTestImagePictFile(void)
{
    if (verbose) puts("STUB: SCSetTestImagePictFile called");
    return NULL;
}

void* SCSetTestImagePictHandle(void)
{
    if (verbose) puts("STUB: SCSetTestImagePictHandle called");
    return NULL;
}

void* SCSetTestImagePixMap(void)
{
    if (verbose) puts("STUB: SCSetTestImagePixMap called");
    return NULL;
}

void* SCVideoConductSizeDialog(void)
{
    if (verbose) puts("STUB: SCVideoConductSizeDialog called");
    return NULL;
}

void* SGAddExtendedFrameReference(void)
{
    if (verbose) puts("STUB: SGAddExtendedFrameReference called");
    return NULL;
}

void* SGAddExtendedMovieData(void)
{
    if (verbose) puts("STUB: SGAddExtendedMovieData called");
    return NULL;
}

void* SGAddFrame(void)
{
    if (verbose) puts("STUB: SGAddFrame called");
    return NULL;
}

void* SGAddFrameReference(void)
{
    if (verbose) puts("STUB: SGAddFrameReference called");
    return NULL;
}

void* SGAddMovieData(void)
{
    if (verbose) puts("STUB: SGAddMovieData called");
    return NULL;
}

void* SGAddOutputDataRefToMedia(void)
{
    if (verbose) puts("STUB: SGAddOutputDataRefToMedia called");
    return NULL;
}

void* SGAlignChannelRect(void)
{
    if (verbose) puts("STUB: SGAlignChannelRect called");
    return NULL;
}

void* SGAppendDeviceListToMenu(void)
{
    if (verbose) puts("STUB: SGAppendDeviceListToMenu called");
    return NULL;
}

void* SGChangedSource(void)
{
    if (verbose) puts("STUB: SGChangedSource called");
    return NULL;
}

void* SGChannelGetCodecSettings(void)
{
    if (verbose) puts("STUB: SGChannelGetCodecSettings called");
    return NULL;
}

void* SGChannelGetDataSourceName(void)
{
    if (verbose) puts("STUB: SGChannelGetDataSourceName called");
    return NULL;
}

void* SGChannelGetPreferredClock(void)
{
    if (verbose) puts("STUB: SGChannelGetPreferredClock called");
    return NULL;
}

void* SGChannelGetRequestedDataRate(void)
{
    if (verbose) puts("STUB: SGChannelGetRequestedDataRate called");
    return NULL;
}

void* SGChannelPutPicture(void)
{
    if (verbose) puts("STUB: SGChannelPutPicture called");
    return NULL;
}

void* SGChannelSetCodecSettings(void)
{
    if (verbose) puts("STUB: SGChannelSetCodecSettings called");
    return NULL;
}

void* SGChannelSetDataSourceName(void)
{
    if (verbose) puts("STUB: SGChannelSetDataSourceName called");
    return NULL;
}

void* SGChannelSetRequestedDataRate(void)
{
    if (verbose) puts("STUB: SGChannelSetRequestedDataRate called");
    return NULL;
}

void* SGCompressFrame(void)
{
    if (verbose) puts("STUB: SGCompressFrame called");
    return NULL;
}

void* SGCompressFrameComplete(void)
{
    if (verbose) puts("STUB: SGCompressFrameComplete called");
    return NULL;
}

void* SGDisplayCompress(void)
{
    if (verbose) puts("STUB: SGDisplayCompress called");
    return NULL;
}

void* SGDisplayCompressS(void)
{
    if (verbose) puts("STUB: SGDisplayCompressS called");
    return NULL;
}

void* SGDisplayFrame(void)
{
    if (verbose) puts("STUB: SGDisplayFrame called");
    return NULL;
}

void* SGDisposeChannel(void)
{
    if (verbose) puts("STUB: SGDisposeChannel called");
    return NULL;
}

void* SGDisposeDeviceList(void)
{
    if (verbose) puts("STUB: SGDisposeDeviceList called");
    return NULL;
}

void* SGDisposeOutput(void)
{
    if (verbose) puts("STUB: SGDisposeOutput called");
    return NULL;
}

void* SGGetAdditionalSoundRates(void)
{
    if (verbose) puts("STUB: SGGetAdditionalSoundRates called");
    return NULL;
}

void* SGGetAlignmentProc(void)
{
    if (verbose) puts("STUB: SGGetAlignmentProc called");
    return NULL;
}

void* SGGetBufferInfo(void)
{
    if (verbose) puts("STUB: SGGetBufferInfo called");
    return NULL;
}

void* SGGetChannelBounds(void)
{
    if (verbose) puts("STUB: SGGetChannelBounds called");
    return NULL;
}

void* SGGetChannelClip(void)
{
    if (verbose) puts("STUB: SGGetChannelClip called");
    return NULL;
}

void* SGGetChannelDeviceAndInputNames(void)
{
    if (verbose) puts("STUB: SGGetChannelDeviceAndInputNames called");
    return NULL;
}

void* SGGetChannelDeviceList(void)
{
    if (verbose) puts("STUB: SGGetChannelDeviceList called");
    return NULL;
}

void* SGGetChannelInfo(void)
{
    if (verbose) puts("STUB: SGGetChannelInfo called");
    return NULL;
}

void* SGGetChannelMatrix(void)
{
    if (verbose) puts("STUB: SGGetChannelMatrix called");
    return NULL;
}

void* SGGetChannelMaxFrames(void)
{
    if (verbose) puts("STUB: SGGetChannelMaxFrames called");
    return NULL;
}

void* SGGetChannelPlayFlags(void)
{
    if (verbose) puts("STUB: SGGetChannelPlayFlags called");
    return NULL;
}

void* SGGetChannelRefCon(void)
{
    if (verbose) puts("STUB: SGGetChannelRefCon called");
    return NULL;
}

void* SGGetChannelSampleDescription(void)
{
    if (verbose) puts("STUB: SGGetChannelSampleDescription called");
    return NULL;
}

void* SGGetChannelSettings(void)
{
    if (verbose) puts("STUB: SGGetChannelSettings called");
    return NULL;
}

void* SGGetChannelTimeBase(void)
{
    if (verbose) puts("STUB: SGGetChannelTimeBase called");
    return NULL;
}

void* SGGetChannelTimeScale(void)
{
    if (verbose) puts("STUB: SGGetChannelTimeScale called");
    return NULL;
}

void* SGGetChannelUsage(void)
{
    if (verbose) puts("STUB: SGGetChannelUsage called");
    return NULL;
}

void* SGGetChannelVolume(void)
{
    if (verbose) puts("STUB: SGGetChannelVolume called");
    return NULL;
}

void* SGGetCompressBuffer(void)
{
    if (verbose) puts("STUB: SGGetCompressBuffer called");
    return NULL;
}

void* SGGetDataOutput(void)
{
    if (verbose) puts("STUB: SGGetDataOutput called");
    return NULL;
}

void* SGGetDataOutputStorageSpaceRemaining(void)
{
    if (verbose) puts("STUB: SGGetDataOutputStorageSpaceRemaining called");
    return NULL;
}

void* SGGetDataOutputStorageSpaceRemaining64(void)
{
    if (verbose) puts("STUB: SGGetDataOutputStorageSpaceRemaining64 called");
    return NULL;
}

void* SGGetDataRate(void)
{
    if (verbose) puts("STUB: SGGetDataRate called");
    return NULL;
}

void* SGGetDataRef(void)
{
    if (verbose) puts("STUB: SGGetDataRef called");
    return NULL;
}

void* SGGetFlags(void)
{
    if (verbose) puts("STUB: SGGetFlags called");
    return NULL;
}

void* SGGetFrameRate(void)
{
    if (verbose) puts("STUB: SGGetFrameRate called");
    return NULL;
}

void* SGGetGWorld(void)
{
    if (verbose) puts("STUB: SGGetGWorld called");
    return NULL;
}

void* SGGetIndChannel(void)
{
    if (verbose) puts("STUB: SGGetIndChannel called");
    return NULL;
}

void* SGGetInstrument(void)
{
    if (verbose) puts("STUB: SGGetInstrument called");
    return NULL;
}

void* SGGetLastMovieResID(void)
{
    if (verbose) puts("STUB: SGGetLastMovieResID called");
    return NULL;
}

void* SGGetMaximumRecordTime(void)
{
    if (verbose) puts("STUB: SGGetMaximumRecordTime called");
    return NULL;
}

void* SGGetMode(void)
{
    if (verbose) puts("STUB: SGGetMode called");
    return NULL;
}

void* SGGetMovie(void)
{
    if (verbose) puts("STUB: SGGetMovie called");
    return NULL;
}

void* SGGetNextExtendedFrameReference(void)
{
    if (verbose) puts("STUB: SGGetNextExtendedFrameReference called");
    return NULL;
}

void* SGGetNextFrameReference(void)
{
    if (verbose) puts("STUB: SGGetNextFrameReference called");
    return NULL;
}

void* SGGetOutputDataReference(void)
{
    if (verbose) puts("STUB: SGGetOutputDataReference called");
    return NULL;
}

void* SGGetOutputMaximumOffset(void)
{
    if (verbose) puts("STUB: SGGetOutputMaximumOffset called");
    return NULL;
}

void* SGGetOutputNextOutput(void)
{
    if (verbose) puts("STUB: SGGetOutputNextOutput called");
    return NULL;
}

void* SGGetPause(void)
{
    if (verbose) puts("STUB: SGGetPause called");
    return NULL;
}

void* SGGetPreferredPacketSize(void)
{
    if (verbose) puts("STUB: SGGetPreferredPacketSize called");
    return NULL;
}

void* SGGetSettings(void)
{
    if (verbose) puts("STUB: SGGetSettings called");
    return NULL;
}

void* SGGetSoundInputDriver(void)
{
    if (verbose) puts("STUB: SGGetSoundInputDriver called");
    return NULL;
}

void* SGGetSoundInputParameters(void)
{
    if (verbose) puts("STUB: SGGetSoundInputParameters called");
    return NULL;
}

void* SGGetSoundInputRate(void)
{
    if (verbose) puts("STUB: SGGetSoundInputRate called");
    return NULL;
}

void* SGGetSoundRecordChunkSize(void)
{
    if (verbose) puts("STUB: SGGetSoundRecordChunkSize called");
    return NULL;
}

void* SGGetSrcVideoBounds(void)
{
    if (verbose) puts("STUB: SGGetSrcVideoBounds called");
    return NULL;
}

void* SGGetStorageSpaceRemaining(void)
{
    if (verbose) puts("STUB: SGGetStorageSpaceRemaining called");
    return NULL;
}

void* SGGetStorageSpaceRemaining64(void)
{
    if (verbose) puts("STUB: SGGetStorageSpaceRemaining64 called");
    return NULL;
}

void* SGGetTextReturnToSpaceValue(void)
{
    if (verbose) puts("STUB: SGGetTextReturnToSpaceValue called");
    return NULL;
}

void* SGGetTimeBase(void)
{
    if (verbose) puts("STUB: SGGetTimeBase called");
    return NULL;
}

void* SGGetTimeRemaining(void)
{
    if (verbose) puts("STUB: SGGetTimeRemaining called");
    return NULL;
}

void* SGGetUseScreenBuffer(void)
{
    if (verbose) puts("STUB: SGGetUseScreenBuffer called");
    return NULL;
}

void* SGGetUserVideoCompressorList(void)
{
    if (verbose) puts("STUB: SGGetUserVideoCompressorList called");
    return NULL;
}

void* SGGetVideoBottlenecks(void)
{
    if (verbose) puts("STUB: SGGetVideoBottlenecks called");
    return NULL;
}

void* SGGetVideoCompressor(void)
{
    if (verbose) puts("STUB: SGGetVideoCompressor called");
    return NULL;
}

void* SGGetVideoCompressorType(void)
{
    if (verbose) puts("STUB: SGGetVideoCompressorType called");
    return NULL;
}

void* SGGetVideoDigitizerComponent(void)
{
    if (verbose) puts("STUB: SGGetVideoDigitizerComponent called");
    return NULL;
}

void* SGGetVideoRect(void)
{
    if (verbose) puts("STUB: SGGetVideoRect called");
    return NULL;
}

void* SGGrabCompressComplete(void)
{
    if (verbose) puts("STUB: SGGrabCompressComplete called");
    return NULL;
}

void* SGGrabFrame(void)
{
    if (verbose) puts("STUB: SGGrabFrame called");
    return NULL;
}

void* SGGrabFrameComplete(void)
{
    if (verbose) puts("STUB: SGGrabFrameComplete called");
    return NULL;
}

void* SGGrabPict(void)
{
    if (verbose) puts("STUB: SGGrabPict called");
    return NULL;
}

void* SGHandleUpdateEvent(void)
{
    if (verbose) puts("STUB: SGHandleUpdateEvent called");
    return NULL;
}

void* SGIdle(void)
{
    if (verbose) puts("STUB: SGIdle called");
    return NULL;
}

void* SGInitChannel(void)
{
    if (verbose) puts("STUB: SGInitChannel called");
    return NULL;
}

void* SGInitialize(void)
{
    if (verbose) puts("STUB: SGInitialize called");
    return NULL;
}

void* SGNewChannel(void)
{
    if (verbose) puts("STUB: SGNewChannel called");
    return NULL;
}

void* SGNewChannelFromComponent(void)
{
    if (verbose) puts("STUB: SGNewChannelFromComponent called");
    return NULL;
}

void* SGNewOutput(void)
{
    if (verbose) puts("STUB: SGNewOutput called");
    return NULL;
}

void* SGPanelCanRun(void)
{
    if (verbose) puts("STUB: SGPanelCanRun called");
    return NULL;
}

void* SGPanelEvent(void)
{
    if (verbose) puts("STUB: SGPanelEvent called");
    return NULL;
}

void* SGPanelGetDITLForSize(void)
{
    if (verbose) puts("STUB: SGPanelGetDITLForSize called");
    return NULL;
}

void* SGPanelGetDitl(void)
{
    if (verbose) puts("STUB: SGPanelGetDitl called");
    return NULL;
}

void* SGPanelGetSettings(void)
{
    if (verbose) puts("STUB: SGPanelGetSettings called");
    return NULL;
}

void* SGPanelGetTitle(void)
{
    if (verbose) puts("STUB: SGPanelGetTitle called");
    return NULL;
}

void* SGPanelInstall(void)
{
    if (verbose) puts("STUB: SGPanelInstall called");
    return NULL;
}

void* SGPanelItem(void)
{
    if (verbose) puts("STUB: SGPanelItem called");
    return NULL;
}

void* SGPanelRemove(void)
{
    if (verbose) puts("STUB: SGPanelRemove called");
    return NULL;
}

void* SGPanelSetEventFilter(void)
{
    if (verbose) puts("STUB: SGPanelSetEventFilter called");
    return NULL;
}

void* SGPanelSetGrabber(void)
{
    if (verbose) puts("STUB: SGPanelSetGrabber called");
    return NULL;
}

void* SGPanelSetResFile(void)
{
    if (verbose) puts("STUB: SGPanelSetResFile called");
    return NULL;
}

void* SGPanelSetSettings(void)
{
    if (verbose) puts("STUB: SGPanelSetSettings called");
    return NULL;
}

void* SGPanelValidateInput(void)
{
    if (verbose) puts("STUB: SGPanelValidateInput called");
    return NULL;
}

void* SGPause(void)
{
    if (verbose) puts("STUB: SGPause called");
    return NULL;
}

void* SGPrepare(void)
{
    if (verbose) puts("STUB: SGPrepare called");
    return NULL;
}

void* SGRelease(void)
{
    if (verbose) puts("STUB: SGRelease called");
    return NULL;
}

void* SGSetAdditionalSoundRates(void)
{
    if (verbose) puts("STUB: SGSetAdditionalSoundRates called");
    return NULL;
}

void* SGSetChannelBounds(void)
{
    if (verbose) puts("STUB: SGSetChannelBounds called");
    return NULL;
}

void* SGSetChannelClip(void)
{
    if (verbose) puts("STUB: SGSetChannelClip called");
    return NULL;
}

void* SGSetChannelDevice(void)
{
    if (verbose) puts("STUB: SGSetChannelDevice called");
    return NULL;
}

void* SGSetChannelDeviceInput(void)
{
    if (verbose) puts("STUB: SGSetChannelDeviceInput called");
    return NULL;
}

void* SGSetChannelMatrix(void)
{
    if (verbose) puts("STUB: SGSetChannelMatrix called");
    return NULL;
}

void* SGSetChannelMaxFrames(void)
{
    if (verbose) puts("STUB: SGSetChannelMaxFrames called");
    return NULL;
}

void* SGSetChannelOutput(void)
{
    if (verbose) puts("STUB: SGSetChannelOutput called");
    return NULL;
}

void* SGSetChannelPlayFlags(void)
{
    if (verbose) puts("STUB: SGSetChannelPlayFlags called");
    return NULL;
}

void* SGSetChannelRefCon(void)
{
    if (verbose) puts("STUB: SGSetChannelRefCon called");
    return NULL;
}

void* SGSetChannelSettings(void)
{
    if (verbose) puts("STUB: SGSetChannelSettings called");
    return NULL;
}

void* SGSetChannelSettingsStateChanging(void)
{
    if (verbose) puts("STUB: SGSetChannelSettingsStateChanging called");
    return NULL;
}

void* SGSetChannelUsage(void)
{
    if (verbose) puts("STUB: SGSetChannelUsage called");
    return NULL;
}

void* SGSetChannelVolume(void)
{
    if (verbose) puts("STUB: SGSetChannelVolume called");
    return NULL;
}

void* SGSetCompressBuffer(void)
{
    if (verbose) puts("STUB: SGSetCompressBuffer called");
    return NULL;
}

void* SGSetDataOutput(void)
{
    if (verbose) puts("STUB: SGSetDataOutput called");
    return NULL;
}

void* SGSetDataProc(void)
{
    if (verbose) puts("STUB: SGSetDataProc called");
    return NULL;
}

void* SGSetDataRef(void)
{
    if (verbose) puts("STUB: SGSetDataRef called");
    return NULL;
}

void* SGSetFlags(void)
{
    if (verbose) puts("STUB: SGSetFlags called");
    return NULL;
}

void* SGSetFontName(void)
{
    if (verbose) puts("STUB: SGSetFontName called");
    return NULL;
}

void* SGSetFontSize(void)
{
    if (verbose) puts("STUB: SGSetFontSize called");
    return NULL;
}

void* SGSetFrameRate(void)
{
    if (verbose) puts("STUB: SGSetFrameRate called");
    return NULL;
}

void* SGSetGWorld(void)
{
    if (verbose) puts("STUB: SGSetGWorld called");
    return NULL;
}

void* SGSetInstrument(void)
{
    if (verbose) puts("STUB: SGSetInstrument called");
    return NULL;
}

void* SGSetJustification(void)
{
    if (verbose) puts("STUB: SGSetJustification called");
    return NULL;
}

void* SGSetMaximumRecordTime(void)
{
    if (verbose) puts("STUB: SGSetMaximumRecordTime called");
    return NULL;
}

void* SGSetOutputFlags(void)
{
    if (verbose) puts("STUB: SGSetOutputFlags called");
    return NULL;
}

void* SGSetOutputMaximumOffset(void)
{
    if (verbose) puts("STUB: SGSetOutputMaximumOffset called");
    return NULL;
}

void* SGSetOutputNextOutput(void)
{
    if (verbose) puts("STUB: SGSetOutputNextOutput called");
    return NULL;
}

void* SGSetPreferredPacketSize(void)
{
    if (verbose) puts("STUB: SGSetPreferredPacketSize called");
    return NULL;
}

void* SGSetSettings(void)
{
    if (verbose) puts("STUB: SGSetSettings called");
    return NULL;
}

void* SGSetSettingsSummary(void)
{
    if (verbose) puts("STUB: SGSetSettingsSummary called");
    return NULL;
}

void* SGSetSoundInputDriver(void)
{
    if (verbose) puts("STUB: SGSetSoundInputDriver called");
    return NULL;
}

void* SGSetSoundInputParameters(void)
{
    if (verbose) puts("STUB: SGSetSoundInputParameters called");
    return NULL;
}

void* SGSetSoundInputRate(void)
{
    if (verbose) puts("STUB: SGSetSoundInputRate called");
    return NULL;
}

void* SGSetSoundRecordChunkSize(void)
{
    if (verbose) puts("STUB: SGSetSoundRecordChunkSize called");
    return NULL;
}

void* SGSetTextBackColor(void)
{
    if (verbose) puts("STUB: SGSetTextBackColor called");
    return NULL;
}

void* SGSetTextForeColor(void)
{
    if (verbose) puts("STUB: SGSetTextForeColor called");
    return NULL;
}

void* SGSetTextReturnToSpaceValue(void)
{
    if (verbose) puts("STUB: SGSetTextReturnToSpaceValue called");
    return NULL;
}

void* SGSetUseScreenBuffer(void)
{
    if (verbose) puts("STUB: SGSetUseScreenBuffer called");
    return NULL;
}

void* SGSetUserVideoCompressorList(void)
{
    if (verbose) puts("STUB: SGSetUserVideoCompressorList called");
    return NULL;
}

void* SGSetVideoBottlenecks(void)
{
    if (verbose) puts("STUB: SGSetVideoBottlenecks called");
    return NULL;
}

void* SGSetVideoCompressor(void)
{
    if (verbose) puts("STUB: SGSetVideoCompressor called");
    return NULL;
}

void* SGSetVideoCompressorType(void)
{
    if (verbose) puts("STUB: SGSetVideoCompressorType called");
    return NULL;
}

void* SGSetVideoDigitizerComponent(void)
{
    if (verbose) puts("STUB: SGSetVideoDigitizerComponent called");
    return NULL;
}

void* SGSetVideoRect(void)
{
    if (verbose) puts("STUB: SGSetVideoRect called");
    return NULL;
}

void* SGSettingsDialog(void)
{
    if (verbose) puts("STUB: SGSettingsDialog called");
    return NULL;
}

void* SGSortDeviceList(void)
{
    if (verbose) puts("STUB: SGSortDeviceList called");
    return NULL;
}

void* SGSoundInputDriverChanged(void)
{
    if (verbose) puts("STUB: SGSoundInputDriverChanged called");
    return NULL;
}

void* SGStartPreview(void)
{
    if (verbose) puts("STUB: SGStartPreview called");
    return NULL;
}

void* SGStartRecord(void)
{
    if (verbose) puts("STUB: SGStartRecord called");
    return NULL;
}

void* SGStop(void)
{
    if (verbose) puts("STUB: SGStop called");
    return NULL;
}

void* SGTransferFrameForCompress(void)
{
    if (verbose) puts("STUB: SGTransferFrameForCompress called");
    return NULL;
}

void* SGUpdate(void)
{
    if (verbose) puts("STUB: SGUpdate called");
    return NULL;
}

void* SGVideoDigitizerChanged(void)
{
    if (verbose) puts("STUB: SGVideoDigitizerChanged called");
    return NULL;
}

void* SGVideoDisableDrawing(void)
{
    if (verbose) puts("STUB: SGVideoDisableDrawing called");
    return NULL;
}

void* SGVideoDrawGraphics(void)
{
    if (verbose) puts("STUB: SGVideoDrawGraphics called");
    return NULL;
}

void* SGVideoDrawVideo(void)
{
    if (verbose) puts("STUB: SGVideoDrawVideo called");
    return NULL;
}

void* SGVideoEnableDrawing(void)
{
    if (verbose) puts("STUB: SGVideoEnableDrawing called");
    return NULL;
}

void* SGVideoStopDrawing(void)
{
    if (verbose) puts("STUB: SGVideoStopDrawing called");
    return NULL;
}

void* SGWriteExtendedMovieData(void)
{
    if (verbose) puts("STUB: SGWriteExtendedMovieData called");
    return NULL;
}

void* SGWriteMovieData(void)
{
    if (verbose) puts("STUB: SGWriteMovieData called");
    return NULL;
}

void* SGWriteSamples(void)
{
    if (verbose) puts("STUB: SGWriteSamples called");
    return NULL;
}

void* SHDecodeIdle(void)
{
    if (verbose) puts("STUB: SHDecodeIdle called");
    return NULL;
}

void* SHDerivedRenderIdle(void)
{
    if (verbose) puts("STUB: SHDerivedRenderIdle called");
    return NULL;
}

void* SHDerivedSetDimensions(void)
{
    if (verbose) puts("STUB: SHDerivedSetDimensions called");
    return NULL;
}

void* SHDerivedSetGWorld(void)
{
    if (verbose) puts("STUB: SHDerivedSetGWorld called");
    return NULL;
}

void* SHDerivedSetMatrix(void)
{
    if (verbose) puts("STUB: SHDerivedSetMatrix called");
    return NULL;
}

void* SHDerivedSetVisualContext(void)
{
    if (verbose) puts("STUB: SHDerivedSetVisualContext called");
    return NULL;
}

void* SHDerivedSwitchOffVisualContext(void)
{
    if (verbose) puts("STUB: SHDerivedSwitchOffVisualContext called");
    return NULL;
}

void* SHDispatchChunk(void)
{
    if (verbose) puts("STUB: SHDispatchChunk called");
    return NULL;
}

void* SHDisposePtr(void)
{
    if (verbose) puts("STUB: SHDisposePtr called");
    return NULL;
}

void* SHDoneWithChunk(void)
{
    if (verbose) puts("STUB: SHDoneWithChunk called");
    return NULL;
}

void* SHGetClip(void)
{
    if (verbose) puts("STUB: SHGetClip called");
    return NULL;
}

void* SHGetClock(void)
{
    if (verbose) puts("STUB: SHGetClock called");
    return NULL;
}

void* SHGetCurrentChunk(void)
{
    if (verbose) puts("STUB: SHGetCurrentChunk called");
    return NULL;
}

void* SHGetDimensions(void)
{
    if (verbose) puts("STUB: SHGetDimensions called");
    return NULL;
}

void* SHGetEmptyInterval(void)
{
    if (verbose) puts("STUB: SHGetEmptyInterval called");
    return NULL;
}

void* SHGetEnable(void)
{
    if (verbose) puts("STUB: SHGetEnable called");
    return NULL;
}

void* SHGetGWorld(void)
{
    if (verbose) puts("STUB: SHGetGWorld called");
    return NULL;
}

void* SHGetGraphicsMode(void)
{
    if (verbose) puts("STUB: SHGetGraphicsMode called");
    return NULL;
}

void* SHGetIndChunk(void)
{
    if (verbose) puts("STUB: SHGetIndChunk called");
    return NULL;
}

void* SHGetIndSubstreamHandler(void)
{
    if (verbose) puts("STUB: SHGetIndSubstreamHandler called");
    return NULL;
}

void* SHGetMatrix(void)
{
    if (verbose) puts("STUB: SHGetMatrix called");
    return NULL;
}

void* SHGetName(void)
{
    if (verbose) puts("STUB: SHGetName called");
    return NULL;
}

void* SHGetNextStepTime(void)
{
    if (verbose) puts("STUB: SHGetNextStepTime called");
    return NULL;
}

void* SHGetNotificationProc(void)
{
    if (verbose) puts("STUB: SHGetNotificationProc called");
    return NULL;
}

void* SHGetNumChunks(void)
{
    if (verbose) puts("STUB: SHGetNumChunks called");
    return NULL;
}

void* SHGetPicture(void)
{
    if (verbose) puts("STUB: SHGetPicture called");
    return NULL;
}

void* SHGetPlayHints(void)
{
    if (verbose) puts("STUB: SHGetPlayHints called");
    return NULL;
}

void* SHGetPresenting(void)
{
    if (verbose) puts("STUB: SHGetPresenting called");
    return NULL;
}

void* SHGetSendDataProc(void)
{
    if (verbose) puts("STUB: SHGetSendDataProc called");
    return NULL;
}

void* SHGetSoundOutputComponent(void)
{
    if (verbose) puts("STUB: SHGetSoundOutputComponent called");
    return NULL;
}

void* SHGetStreamHandlerCapabilities(void)
{
    if (verbose) puts("STUB: SHGetStreamHandlerCapabilities called");
    return NULL;
}

void* SHGetStreamInfo(void)
{
    if (verbose) puts("STUB: SHGetStreamInfo called");
    return NULL;
}

void* SHGetTimeParameters(void)
{
    if (verbose) puts("STUB: SHGetTimeParameters called");
    return NULL;
}

void* SHGetVisualContext(void)
{
    if (verbose) puts("STUB: SHGetVisualContext called");
    return NULL;
}

void* SHGetVolumes(void)
{
    if (verbose) puts("STUB: SHGetVolumes called");
    return NULL;
}

void* SHHasCharacteristic(void)
{
    if (verbose) puts("STUB: SHHasCharacteristic called");
    return NULL;
}

void* SHInitialize(void)
{
    if (verbose) puts("STUB: SHInitialize called");
    return NULL;
}

void* SHInvalidateRegion(void)
{
    if (verbose) puts("STUB: SHInvalidateRegion called");
    return NULL;
}

void* SHNewPtr(void)
{
    if (verbose) puts("STUB: SHNewPtr called");
    return NULL;
}

void* SHPlayNow(void)
{
    if (verbose) puts("STUB: SHPlayNow called");
    return NULL;
}

void* SHPreflightChunk(void)
{
    if (verbose) puts("STUB: SHPreflightChunk called");
    return NULL;
}

void* SHPreflightHWMC(void)
{
    if (verbose) puts("STUB: SHPreflightHWMC called");
    return NULL;
}

void* SHRenderIdle(void)
{
    if (verbose) puts("STUB: SHRenderIdle called");
    return NULL;
}

void* SHReset(void)
{
    if (verbose) puts("STUB: SHReset called");
    return NULL;
}

void* SHRestartDispatchReason(void)
{
    if (verbose) puts("STUB: SHRestartDispatchReason called");
    return NULL;
}

void* SHSampleDescriptionB2N(void)
{
    if (verbose) puts("STUB: SHSampleDescriptionB2N called");
    return NULL;
}

void* SHSampleDescriptionN2B(void)
{
    if (verbose) puts("STUB: SHSampleDescriptionN2B called");
    return NULL;
}

void* SHSetActiveSegment(void)
{
    if (verbose) puts("STUB: SHSetActiveSegment called");
    return NULL;
}

void* SHSetAdjustedPlayRate(void)
{
    if (verbose) puts("STUB: SHSetAdjustedPlayRate called");
    return NULL;
}

void* SHSetCapabilities(void)
{
    if (verbose) puts("STUB: SHSetCapabilities called");
    return NULL;
}

void* SHSetClip(void)
{
    if (verbose) puts("STUB: SHSetClip called");
    return NULL;
}

void* SHSetDimensions(void)
{
    if (verbose) puts("STUB: SHSetDimensions called");
    return NULL;
}

void* SHSetDispatchReason(void)
{
    if (verbose) puts("STUB: SHSetDispatchReason called");
    return NULL;
}

void* SHSetEnable(void)
{
    if (verbose) puts("STUB: SHSetEnable called");
    return NULL;
}

void* SHSetGWorld(void)
{
    if (verbose) puts("STUB: SHSetGWorld called");
    return NULL;
}

void* SHSetGraphicsMode(void)
{
    if (verbose) puts("STUB: SHSetGraphicsMode called");
    return NULL;
}

void* SHSetMatrix(void)
{
    if (verbose) puts("STUB: SHSetMatrix called");
    return NULL;
}

void* SHSetMpegTimeBase(void)
{
    if (verbose) puts("STUB: SHSetMpegTimeBase called");
    return NULL;
}

void* SHSetNotificationProc(void)
{
    if (verbose) puts("STUB: SHSetNotificationProc called");
    return NULL;
}

void* SHSetPlayHints(void)
{
    if (verbose) puts("STUB: SHSetPlayHints called");
    return NULL;
}

void* SHSetPresenting(void)
{
    if (verbose) puts("STUB: SHSetPresenting called");
    return NULL;
}

void* SHSetRate(void)
{
    if (verbose) puts("STUB: SHSetRate called");
    return NULL;
}

void* SHSetSendDataProc(void)
{
    if (verbose) puts("STUB: SHSetSendDataProc called");
    return NULL;
}

void* SHSetSoundOutputComponent(void)
{
    if (verbose) puts("STUB: SHSetSoundOutputComponent called");
    return NULL;
}

void* SHSetStreamInfo(void)
{
    if (verbose) puts("STUB: SHSetStreamInfo called");
    return NULL;
}

void* SHSetTimeParameters(void)
{
    if (verbose) puts("STUB: SHSetTimeParameters called");
    return NULL;
}

void* SHSetVisualContext(void)
{
    if (verbose) puts("STUB: SHSetVisualContext called");
    return NULL;
}

void* SHSetVolumes(void)
{
    if (verbose) puts("STUB: SHSetVolumes called");
    return NULL;
}

void* SHStartupHWMC(void)
{
    if (verbose) puts("STUB: SHStartupHWMC called");
    return NULL;
}

void* SHSwitchOffVisualContext(void)
{
    if (verbose) puts("STUB: SHSwitchOffVisualContext called");
    return NULL;
}

void* SHTeardownHWMC(void)
{
    if (verbose) puts("STUB: SHTeardownHWMC called");
    return NULL;
}

void* SSMusicValidateAtomicInstrument(void)
{
    if (verbose) puts("STUB: SSMusicValidateAtomicInstrument called");
    return NULL;
}

void* SampleNumToMediaDecodeTime(void)
{
    if (verbose) puts("STUB: SampleNumToMediaDecodeTime called");
    return NULL;
}

void* SampleNumToMediaDisplayTime(void)
{
    if (verbose) puts("STUB: SampleNumToMediaDisplayTime called");
    return NULL;
}

void* SampleNumToMediaTime(void)
{
    if (verbose) puts("STUB: SampleNumToMediaTime called");
    return NULL;
}

void* ScaleMatrix(void)
{
    if (verbose) puts("STUB: ScaleMatrix called");
    return NULL;
}

void* ScaleMovieSegment(void)
{
    if (verbose) puts("STUB: ScaleMovieSegment called");
    return NULL;
}

void* ScaleTrackSegment(void)
{
    if (verbose) puts("STUB: ScaleTrackSegment called");
    return NULL;
}

void* ScalingCodecClientGetBlitterProc(void)
{
    if (verbose) puts("STUB: ScalingCodecClientGetBlitterProc called");
    return NULL;
}

void* ScalingCodecClientGetYUVPtrsProc(void)
{
    if (verbose) puts("STUB: ScalingCodecClientGetYUVPtrsProc called");
    return NULL;
}

void* ScamFSRefFromOpenFile(void)
{
    if (verbose) puts("STUB: ScamFSRefFromOpenFile called");
    return NULL;
}

void* ScamFSSpecFromOpenFile(void)
{
    if (verbose) puts("STUB: ScamFSSpecFromOpenFile called");
    return NULL;
}

void* SelectMovieAlternates(void)
{
    if (verbose) puts("STUB: SelectMovieAlternates called");
    return NULL;
}

void* SendComponentMessage(void)
{
    if (verbose) puts("STUB: SendComponentMessage called");
    return NULL;
}

void* SequenceFilterBegin(void)
{
    if (verbose) puts("STUB: SequenceFilterBegin called");
    return NULL;
}

void* SequenceFilterDoUserDialog(void)
{
    if (verbose) puts("STUB: SequenceFilterDoUserDialog called");
    return NULL;
}

void* SequenceFilterEnd(void)
{
    if (verbose) puts("STUB: SequenceFilterEnd called");
    return NULL;
}

void* SequenceFilterFeedSampleData(void)
{
    if (verbose) puts("STUB: SequenceFilterFeedSampleData called");
    return NULL;
}

void* SequenceFilterGetInputSampleInfo(void)
{
    if (verbose) puts("STUB: SequenceFilterGetInputSampleInfo called");
    return NULL;
}

void* SequenceFilterGetOutputSampleInfo(void)
{
    if (verbose) puts("STUB: SequenceFilterGetOutputSampleInfo called");
    return NULL;
}

void* SequenceFilterGetSettingsAsAtomContainer(void)
{
    if (verbose) puts("STUB: SequenceFilterGetSettingsAsAtomContainer called");
    return NULL;
}

void* SequenceFilterSetDataProc(void)
{
    if (verbose) puts("STUB: SequenceFilterSetDataProc called");
    return NULL;
}

void* SequenceFilterSetInputSampleInfo(void)
{
    if (verbose) puts("STUB: SequenceFilterSetInputSampleInfo called");
    return NULL;
}

void* SequenceFilterSetSettingsFromAtomContainer(void)
{
    if (verbose) puts("STUB: SequenceFilterSetSettingsFromAtomContainer called");
    return NULL;
}

void* SetAutoTrackAlternatesEnabled(void)
{
    if (verbose) puts("STUB: SetAutoTrackAlternatesEnabled called");
    return NULL;
}

void* SetCSequenceDataRateParams(void)
{
    if (verbose) puts("STUB: SetCSequenceDataRateParams called");
    return NULL;
}

void* SetCSequenceFlushProc(void)
{
    if (verbose) puts("STUB: SetCSequenceFlushProc called");
    return NULL;
}

void* SetCSequenceFrameNumber(void)
{
    if (verbose) puts("STUB: SetCSequenceFrameNumber called");
    return NULL;
}

void* SetCSequenceKeyFrameRate(void)
{
    if (verbose) puts("STUB: SetCSequenceKeyFrameRate called");
    return NULL;
}

void* SetCSequencePreferredPacketSize(void)
{
    if (verbose) puts("STUB: SetCSequencePreferredPacketSize called");
    return NULL;
}

void* SetCSequencePrev(void)
{
    if (verbose) puts("STUB: SetCSequencePrev called");
    return NULL;
}

void* SetCSequenceQuality(void)
{
    if (verbose) puts("STUB: SetCSequenceQuality called");
    return NULL;
}

void* SetComponentPropertiesBulk(void)
{
    if (verbose) puts("STUB: SetComponentPropertiesBulk called");
    return NULL;
}

void* SetComponentPropertyAsType(void)
{
    if (verbose) puts("STUB: SetComponentPropertyAsType called");
    return NULL;
}

void* SetCompressedPixMapInfo(void)
{
    if (verbose) puts("STUB: SetCompressedPixMapInfo called");
    return NULL;
}

void* SetCursorNotifyRect(void)
{
    if (verbose) puts("STUB: SetCursorNotifyRect called");
    return NULL;
}

void* SetDSequenceAccuracy(void)
{
    if (verbose) puts("STUB: SetDSequenceAccuracy called");
    return NULL;
}

void* SetDSequenceDataProc(void)
{
    if (verbose) puts("STUB: SetDSequenceDataProc called");
    return NULL;
}

void* SetDSequenceFlags(void)
{
    if (verbose) puts("STUB: SetDSequenceFlags called");
    return NULL;
}

void* SetDSequenceMask(void)
{
    if (verbose) puts("STUB: SetDSequenceMask called");
    return NULL;
}

void* SetDSequenceMatrix(void)
{
    if (verbose) puts("STUB: SetDSequenceMatrix called");
    return NULL;
}

void* SetDSequenceMatte(void)
{
    if (verbose) puts("STUB: SetDSequenceMatte called");
    return NULL;
}

void* SetDSequenceNonScheduledDisplayDirection(void)
{
    if (verbose) puts("STUB: SetDSequenceNonScheduledDisplayDirection called");
    return NULL;
}

void* SetDSequenceNonScheduledDisplayTime(void)
{
    if (verbose) puts("STUB: SetDSequenceNonScheduledDisplayTime called");
    return NULL;
}

void* SetDSequenceSrcRect(void)
{
    if (verbose) puts("STUB: SetDSequenceSrcRect called");
    return NULL;
}

void* SetDSequenceTimeCode(void)
{
    if (verbose) puts("STUB: SetDSequenceTimeCode called");
    return NULL;
}

void* SetDSequenceTransferMode(void)
{
    if (verbose) puts("STUB: SetDSequenceTransferMode called");
    return NULL;
}

void* SetIdentityMatrix(void)
{
    if (verbose) puts("STUB: SetIdentityMatrix called");
    return NULL;
}

void* SetImageDescriptionCTable(void)
{
    if (verbose) puts("STUB: SetImageDescriptionCTable called");
    return NULL;
}

void* SetMediaBoundaryTimes(void)
{
    if (verbose) puts("STUB: SetMediaBoundaryTimes called");
    return NULL;
}

void* SetMediaDataHandler(void)
{
    if (verbose) puts("STUB: SetMediaDataHandler called");
    return NULL;
}

void* SetMediaDataRef(void)
{
    if (verbose) puts("STUB: SetMediaDataRef called");
    return NULL;
}

void* SetMediaDataRefAttributes(void)
{
    if (verbose) puts("STUB: SetMediaDataRefAttributes called");
    return NULL;
}

void* SetMediaDefaultDataRefIndex(void)
{
    if (verbose) puts("STUB: SetMediaDefaultDataRefIndex called");
    return NULL;
}

void* SetMediaDuration(void)
{
    if (verbose) puts("STUB: SetMediaDuration called");
    return NULL;
}

void* SetMediaHandler(void)
{
    if (verbose) puts("STUB: SetMediaHandler called");
    return NULL;
}

void* SetMediaInputMap(void)
{
    if (verbose) puts("STUB: SetMediaInputMap called");
    return NULL;
}

void* SetMediaLanguage(void)
{
    if (verbose) puts("STUB: SetMediaLanguage called");
    return NULL;
}

void* SetMediaPlayHints(void)
{
    if (verbose) puts("STUB: SetMediaPlayHints called");
    return NULL;
}

void* SetMediaPreferredChunkSize(void)
{
    if (verbose) puts("STUB: SetMediaPreferredChunkSize called");
    return NULL;
}

void* SetMediaPropertyAtom(void)
{
    if (verbose) puts("STUB: SetMediaPropertyAtom called");
    return NULL;
}

void* SetMediaQuality(void)
{
    if (verbose) puts("STUB: SetMediaQuality called");
    return NULL;
}

void* SetMediaSampleDescription(void)
{
    if (verbose) puts("STUB: SetMediaSampleDescription called");
    return NULL;
}

void* SetMediaShadowSync(void)
{
    if (verbose) puts("STUB: SetMediaShadowSync called");
    return NULL;
}

void* SetMediaTimeScale(void)
{
    if (verbose) puts("STUB: SetMediaTimeScale called");
    return NULL;
}

void* SetMovieActive(void)
{
    if (verbose) puts("STUB: SetMovieActive called");
    return NULL;
}

void* SetMovieActiveSegment(void)
{
    if (verbose) puts("STUB: SetMovieActiveSegment called");
    return NULL;
}

void* SetMovieAnchorDataRef(void)
{
    if (verbose) puts("STUB: SetMovieAnchorDataRef called");
    return NULL;
}

void* SetMovieAudioBalance(void)
{
    if (verbose) puts("STUB: SetMovieAudioBalance called");
    return NULL;
}

void* SetMovieAudioContext(void)
{
    if (verbose) puts("STUB: SetMovieAudioContext called");
    return NULL;
}

void* SetMovieAudioFade(void)
{
    if (verbose) puts("STUB: SetMovieAudioFade called");
    return NULL;
}

void* SetMovieAudioFrequencyMeteringNumBands(void)
{
    if (verbose) puts("STUB: SetMovieAudioFrequencyMeteringNumBands called");
    return NULL;
}

void* SetMovieAudioGain(void)
{
    if (verbose) puts("STUB: SetMovieAudioGain called");
    return NULL;
}

void* SetMovieAudioMute(void)
{
    if (verbose) puts("STUB: SetMovieAudioMute called");
    return NULL;
}

void* SetMovieAudioVolumeMeteringEnabled(void)
{
    if (verbose) puts("STUB: SetMovieAudioVolumeMeteringEnabled called");
    return NULL;
}

void* SetMovieBox(void)
{
    if (verbose) puts("STUB: SetMovieBox called");
    return NULL;
}

void* SetMovieBufferGWorld(void)
{
    if (verbose) puts("STUB: SetMovieBufferGWorld called");
    return NULL;
}

void* SetMovieClipRgn(void)
{
    if (verbose) puts("STUB: SetMovieClipRgn called");
    return NULL;
}

void* SetMovieColorTable(void)
{
    if (verbose) puts("STUB: SetMovieColorTable called");
    return NULL;
}

void* SetMovieCompositeBufferFlags(void)
{
    if (verbose) puts("STUB: SetMovieCompositeBufferFlags called");
    return NULL;
}

void* SetMovieCoverProcs(void)
{
    if (verbose) puts("STUB: SetMovieCoverProcs called");
    return NULL;
}

void* SetMovieDefaultDataRef(void)
{
    if (verbose) puts("STUB: SetMovieDefaultDataRef called");
    return NULL;
}

void* SetMovieDisplayClipRgn(void)
{
    if (verbose) puts("STUB: SetMovieDisplayClipRgn called");
    return NULL;
}

void* SetMovieDrawingCompleteProc(void)
{
    if (verbose) puts("STUB: SetMovieDrawingCompleteProc called");
    return NULL;
}

void* SetMovieGWorld(void)
{
    if (verbose) puts("STUB: SetMovieGWorld called");
    return NULL;
}

void* SetMovieLanguage(void)
{
    if (verbose) puts("STUB: SetMovieLanguage called");
    return NULL;
}

void* SetMovieMasterClock(void)
{
    if (verbose) puts("STUB: SetMovieMasterClock called");
    return NULL;
}

void* SetMovieMasterTimeBase(void)
{
    if (verbose) puts("STUB: SetMovieMasterTimeBase called");
    return NULL;
}

void* SetMovieMatrix(void)
{
    if (verbose) puts("STUB: SetMovieMatrix called");
    return NULL;
}

void* SetMovieMovieControllerAssociationForIdleManager(void)
{
    if (verbose) puts("STUB: SetMovieMovieControllerAssociationForIdleManager called");
    return NULL;
}

void* SetMovieParentTrack(void)
{
    if (verbose) puts("STUB: SetMovieParentTrack called");
    return NULL;
}

void* SetMoviePlayHints(void)
{
    if (verbose) puts("STUB: SetMoviePlayHints called");
    return NULL;
}

void* SetMoviePosterTime(void)
{
    if (verbose) puts("STUB: SetMoviePosterTime called");
    return NULL;
}

void* SetMoviePreferredRate(void)
{
    if (verbose) puts("STUB: SetMoviePreferredRate called");
    return NULL;
}

void* SetMoviePreferredVolume(void)
{
    if (verbose) puts("STUB: SetMoviePreferredVolume called");
    return NULL;
}

void* SetMoviePreviewMode(void)
{
    if (verbose) puts("STUB: SetMoviePreviewMode called");
    return NULL;
}

void* SetMoviePreviewTime(void)
{
    if (verbose) puts("STUB: SetMoviePreviewTime called");
    return NULL;
}

void* SetMovieProgressProc(void)
{
    if (verbose) puts("STUB: SetMovieProgressProc called");
    return NULL;
}

void* SetMoviePropertyAtom(void)
{
    if (verbose) puts("STUB: SetMoviePropertyAtom called");
    return NULL;
}

void* SetMovieRate(void)
{
    if (verbose) puts("STUB: SetMovieRate called");
    return NULL;
}

void* SetMovieSelection(void)
{
    if (verbose) puts("STUB: SetMovieSelection called");
    return NULL;
}

void* SetMovieTime(void)
{
    if (verbose) puts("STUB: SetMovieTime called");
    return NULL;
}

void* SetMovieTimeScale(void)
{
    if (verbose) puts("STUB: SetMovieTimeScale called");
    return NULL;
}

void* SetMovieTimeValue(void)
{
    if (verbose) puts("STUB: SetMovieTimeValue called");
    return NULL;
}

void* SetMovieUseTrackForTimeTableCallBack(void)
{
    if (verbose) puts("STUB: SetMovieUseTrackForTimeTableCallBack called");
    return NULL;
}

void* SetMovieVideoOutput(void)
{
    if (verbose) puts("STUB: SetMovieVideoOutput called");
    return NULL;
}

void* SetMovieVisualBrightness(void)
{
    if (verbose) puts("STUB: SetMovieVisualBrightness called");
    return NULL;
}

void* SetMovieVisualContext(void)
{
    if (verbose) puts("STUB: SetMovieVisualContext called");
    return NULL;
}

void* SetMovieVisualContrast(void)
{
    if (verbose) puts("STUB: SetMovieVisualContrast called");
    return NULL;
}

void* SetMovieVisualHue(void)
{
    if (verbose) puts("STUB: SetMovieVisualHue called");
    return NULL;
}

void* SetMovieVisualSaturation(void)
{
    if (verbose) puts("STUB: SetMovieVisualSaturation called");
    return NULL;
}

void* SetMovieVolume(void)
{
    if (verbose) puts("STUB: SetMovieVolume called");
    return NULL;
}

void* SetMoviesErrorProc(void)
{
    if (verbose) puts("STUB: SetMoviesErrorProc called");
    return NULL;
}

void* SetNamedComponentMessageParameter(void)
{
    if (verbose) puts("STUB: SetNamedComponentMessageParameter called");
    return NULL;
}

void* SetNamedComponentMessageParameters(void)
{
    if (verbose) puts("STUB: SetNamedComponentMessageParameters called");
    return NULL;
}

void* SetPosterBox(void)
{
    if (verbose) puts("STUB: SetPosterBox called");
    return NULL;
}

void* SetQTContextProperty(void)
{
    if (verbose) puts("STUB: SetQTContextProperty called");
    return NULL;
}

void* SetQuickTimePreference(void)
{
    if (verbose) puts("STUB: SetQuickTimePreference called");
    return NULL;
}

void* SetSequenceProgressProc(void)
{
    if (verbose) puts("STUB: SetSequenceProgressProc called");
    return NULL;
}

void* SetSpriteProperty(void)
{
    if (verbose) puts("STUB: SetSpriteProperty called");
    return NULL;
}

void* SetSpriteWorldClip(void)
{
    if (verbose) puts("STUB: SetSpriteWorldClip called");
    return NULL;
}

void* SetSpriteWorldFlags(void)
{
    if (verbose) puts("STUB: SetSpriteWorldFlags called");
    return NULL;
}

void* SetSpriteWorldGraphicsMode(void)
{
    if (verbose) puts("STUB: SetSpriteWorldGraphicsMode called");
    return NULL;
}

void* SetSpriteWorldMatrix(void)
{
    if (verbose) puts("STUB: SetSpriteWorldMatrix called");
    return NULL;
}

void* SetTimeBaseFlags(void)
{
    if (verbose) puts("STUB: SetTimeBaseFlags called");
    return NULL;
}

void* SetTimeBaseMasterClock(void)
{
    if (verbose) puts("STUB: SetTimeBaseMasterClock called");
    return NULL;
}

void* SetTimeBaseMasterTimeBase(void)
{
    if (verbose) puts("STUB: SetTimeBaseMasterTimeBase called");
    return NULL;
}

void* SetTimeBaseOffsetTimeBase(void)
{
    if (verbose) puts("STUB: SetTimeBaseOffsetTimeBase called");
    return NULL;
}

void* SetTimeBasePrerollRate(void)
{
    if (verbose) puts("STUB: SetTimeBasePrerollRate called");
    return NULL;
}

void* SetTimeBaseRate(void)
{
    if (verbose) puts("STUB: SetTimeBaseRate called");
    return NULL;
}

void* SetTimeBaseStartTime(void)
{
    if (verbose) puts("STUB: SetTimeBaseStartTime called");
    return NULL;
}

void* SetTimeBaseStopTime(void)
{
    if (verbose) puts("STUB: SetTimeBaseStopTime called");
    return NULL;
}

void* SetTimeBaseTime(void)
{
    if (verbose) puts("STUB: SetTimeBaseTime called");
    return NULL;
}

void* SetTimeBaseValue(void)
{
    if (verbose) puts("STUB: SetTimeBaseValue called");
    return NULL;
}

void* SetTimeBaseZero(void)
{
    if (verbose) puts("STUB: SetTimeBaseZero called");
    return NULL;
}

void* SetTrackAlternate(void)
{
    if (verbose) puts("STUB: SetTrackAlternate called");
    return NULL;
}

void* SetTrackApertureModeDimensionsUsingSampleDescription(void)
{
    if (verbose) puts("STUB: SetTrackApertureModeDimensionsUsingSampleDescription called");
    return NULL;
}

void* SetTrackApertureModeDimensionsUsingTrack(void)
{
    if (verbose) puts("STUB: SetTrackApertureModeDimensionsUsingTrack called");
    return NULL;
}

void* SetTrackAudioGain(void)
{
    if (verbose) puts("STUB: SetTrackAudioGain called");
    return NULL;
}

void* SetTrackAudioMute(void)
{
    if (verbose) puts("STUB: SetTrackAudioMute called");
    return NULL;
}

void* SetTrackAudioVolumeMeteringEnabled(void)
{
    if (verbose) puts("STUB: SetTrackAudioVolumeMeteringEnabled called");
    return NULL;
}

void* SetTrackClipRgn(void)
{
    if (verbose) puts("STUB: SetTrackClipRgn called");
    return NULL;
}

void* SetTrackDimensions(void)
{
    if (verbose) puts("STUB: SetTrackDimensions called");
    return NULL;
}

void* SetTrackEnabled(void)
{
    if (verbose) puts("STUB: SetTrackEnabled called");
    return NULL;
}

void* SetTrackGWorld(void)
{
    if (verbose) puts("STUB: SetTrackGWorld called");
    return NULL;
}

void* SetTrackID(void)
{
    if (verbose) puts("STUB: SetTrackID called");
    return NULL;
}

void* SetTrackLayer(void)
{
    if (verbose) puts("STUB: SetTrackLayer called");
    return NULL;
}

void* SetTrackLoadSettings(void)
{
    if (verbose) puts("STUB: SetTrackLoadSettings called");
    return NULL;
}

void* SetTrackMatrix(void)
{
    if (verbose) puts("STUB: SetTrackMatrix called");
    return NULL;
}

void* SetTrackMatte(void)
{
    if (verbose) puts("STUB: SetTrackMatte called");
    return NULL;
}

void* SetTrackOffset(void)
{
    if (verbose) puts("STUB: SetTrackOffset called");
    return NULL;
}

void* SetTrackPrecompositeFlags(void)
{
    if (verbose) puts("STUB: SetTrackPrecompositeFlags called");
    return NULL;
}

void* SetTrackPrecompositeFlagsWithMask(void)
{
    if (verbose) puts("STUB: SetTrackPrecompositeFlagsWithMask called");
    return NULL;
}

void* SetTrackReference(void)
{
    if (verbose) puts("STUB: SetTrackReference called");
    return NULL;
}

void* SetTrackSoundLocalizationSettings(void)
{
    if (verbose) puts("STUB: SetTrackSoundLocalizationSettings called");
    return NULL;
}

void* SetTrackUsage(void)
{
    if (verbose) puts("STUB: SetTrackUsage called");
    return NULL;
}

void* SetTrackVolume(void)
{
    if (verbose) puts("STUB: SetTrackVolume called");
    return NULL;
}

void* SetUserDataItem(void)
{
    if (verbose) puts("STUB: SetUserDataItem called");
    return NULL;
}

void* ShiftDown(void)
{
    if (verbose) puts("STUB: ShiftDown called");
    return NULL;
}

void* ShowMovieInformation(void)
{
    if (verbose) puts("STUB: ShowMovieInformation called");
    return NULL;
}

void* ShowMoviePoster(void)
{
    if (verbose) puts("STUB: ShowMoviePoster called");
    return NULL;
}

void* SizedDataHEqual(void)
{
    if (verbose) puts("STUB: SizedDataHEqual called");
    return NULL;
}

void* SkewMatrix(void)
{
    if (verbose) puts("STUB: SkewMatrix called");
    return NULL;
}

void* SkipAtom(void)
{
    if (verbose) puts("STUB: SkipAtom called");
    return NULL;
}

void* SpriteHitTest(void)
{
    if (verbose) puts("STUB: SpriteHitTest called");
    return NULL;
}

void* SpriteMediaCountImages(void)
{
    if (verbose) puts("STUB: SpriteMediaCountImages called");
    return NULL;
}

void* SpriteMediaCountSprites(void)
{
    if (verbose) puts("STUB: SpriteMediaCountSprites called");
    return NULL;
}

void* SpriteMediaDisposeImage(void)
{
    if (verbose) puts("STUB: SpriteMediaDisposeImage called");
    return NULL;
}

void* SpriteMediaDisposeSprite(void)
{
    if (verbose) puts("STUB: SpriteMediaDisposeSprite called");
    return NULL;
}

void* SpriteMediaGetActionVariable(void)
{
    if (verbose) puts("STUB: SpriteMediaGetActionVariable called");
    return NULL;
}

void* SpriteMediaGetActionVariableAsString(void)
{
    if (verbose) puts("STUB: SpriteMediaGetActionVariableAsString called");
    return NULL;
}

void* SpriteMediaGetDisplayedSampleNumber(void)
{
    if (verbose) puts("STUB: SpriteMediaGetDisplayedSampleNumber called");
    return NULL;
}

void* SpriteMediaGetImageName(void)
{
    if (verbose) puts("STUB: SpriteMediaGetImageName called");
    return NULL;
}

void* SpriteMediaGetIndImageDescription(void)
{
    if (verbose) puts("STUB: SpriteMediaGetIndImageDescription called");
    return NULL;
}

void* SpriteMediaGetIndImageProperty(void)
{
    if (verbose) puts("STUB: SpriteMediaGetIndImageProperty called");
    return NULL;
}

void* SpriteMediaGetProperty(void)
{
    if (verbose) puts("STUB: SpriteMediaGetProperty called");
    return NULL;
}

void* SpriteMediaGetSpriteActionsForQTEvent(void)
{
    if (verbose) puts("STUB: SpriteMediaGetSpriteActionsForQTEvent called");
    return NULL;
}

void* SpriteMediaGetSpriteName(void)
{
    if (verbose) puts("STUB: SpriteMediaGetSpriteName called");
    return NULL;
}

void* SpriteMediaGetSpriteProperty(void)
{
    if (verbose) puts("STUB: SpriteMediaGetSpriteProperty called");
    return NULL;
}

void* SpriteMediaHitTestAllSprites(void)
{
    if (verbose) puts("STUB: SpriteMediaHitTestAllSprites called");
    return NULL;
}

void* SpriteMediaHitTestOneSprite(void)
{
    if (verbose) puts("STUB: SpriteMediaHitTestOneSprite called");
    return NULL;
}

void* SpriteMediaHitTestSprites(void)
{
    if (verbose) puts("STUB: SpriteMediaHitTestSprites called");
    return NULL;
}

void* SpriteMediaImageIDToIndex(void)
{
    if (verbose) puts("STUB: SpriteMediaImageIDToIndex called");
    return NULL;
}

void* SpriteMediaImageIndexToID(void)
{
    if (verbose) puts("STUB: SpriteMediaImageIndexToID called");
    return NULL;
}

void* SpriteMediaNewImage(void)
{
    if (verbose) puts("STUB: SpriteMediaNewImage called");
    return NULL;
}

void* SpriteMediaNewSprite(void)
{
    if (verbose) puts("STUB: SpriteMediaNewSprite called");
    return NULL;
}

void* SpriteMediaSetActionVariable(void)
{
    if (verbose) puts("STUB: SpriteMediaSetActionVariable called");
    return NULL;
}

void* SpriteMediaSetActionVariableToString(void)
{
    if (verbose) puts("STUB: SpriteMediaSetActionVariableToString called");
    return NULL;
}

void* SpriteMediaSetProperty(void)
{
    if (verbose) puts("STUB: SpriteMediaSetProperty called");
    return NULL;
}

void* SpriteMediaSetSpriteProperty(void)
{
    if (verbose) puts("STUB: SpriteMediaSetSpriteProperty called");
    return NULL;
}

void* SpriteMediaSpriteIDToIndex(void)
{
    if (verbose) puts("STUB: SpriteMediaSpriteIDToIndex called");
    return NULL;
}

void* SpriteMediaSpriteIndexToID(void)
{
    if (verbose) puts("STUB: SpriteMediaSpriteIndexToID called");
    return NULL;
}

void* SpriteWorldHitTest(void)
{
    if (verbose) puts("STUB: SpriteWorldHitTest called");
    return NULL;
}

void* SpriteWorldIdle(void)
{
    if (verbose) puts("STUB: SpriteWorldIdle called");
    return NULL;
}

void* StandardGetFilePreview(void)
{
    if (verbose) puts("STUB: StandardGetFilePreview called");
    return NULL;
}

void* StartMovie(void)
{
    if (verbose) puts("STUB: StartMovie called");
    return NULL;
}

void* StartUsingWarholResFile(void)
{
    if (verbose) puts("STUB: StartUsingWarholResFile called");
    return NULL;
}

void* StdPix(void)
{
    if (verbose) puts("STUB: StdPix called");
    return NULL;
}

void* StopMovie(void)
{
    if (verbose) puts("STUB: StopMovie called");
    return NULL;
}

void* StopUsingWarholResFile(void)
{
    if (verbose) puts("STUB: StopUsingWarholResFile called");
    return NULL;
}

void* SubAtom(void)
{
    if (verbose) puts("STUB: SubAtom called");
    return NULL;
}

void* SubLeafAtom(void)
{
    if (verbose) puts("STUB: SubLeafAtom called");
    return NULL;
}

void* SubLeafAtomNoSkip(void)
{
    if (verbose) puts("STUB: SubLeafAtomNoSkip called");
    return NULL;
}

void* SubtractTime(void)
{
    if (verbose) puts("STUB: SubtractTime called");
    return NULL;
}

void* SuggestIntermediatePixelFormatForRecompression(void)
{
    if (verbose) puts("STUB: SuggestIntermediatePixelFormatForRecompression called");
    return NULL;
}

void* TCFrameNumberToTimeCode(void)
{
    if (verbose) puts("STUB: TCFrameNumberToTimeCode called");
    return NULL;
}

void* TCFrameNumberToTimeCodeCounter(void)
{
    if (verbose) puts("STUB: TCFrameNumberToTimeCodeCounter called");
    return NULL;
}

void* TCFrameNumberToTimeCodeTime(void)
{
    if (verbose) puts("STUB: TCFrameNumberToTimeCodeTime called");
    return NULL;
}

void* TCGetCurrentFrameAndTimeCodeDef(void)
{
    if (verbose) puts("STUB: TCGetCurrentFrameAndTimeCodeDef called");
    return NULL;
}

void* TCGetCurrentTimeCode(void)
{
    if (verbose) puts("STUB: TCGetCurrentTimeCode called");
    return NULL;
}

void* TCGetDisplayOptions(void)
{
    if (verbose) puts("STUB: TCGetDisplayOptions called");
    return NULL;
}

void* TCGetFrameAndTimeCodeDefAtTime(void)
{
    if (verbose) puts("STUB: TCGetFrameAndTimeCodeDefAtTime called");
    return NULL;
}

void* TCGetSourceRef(void)
{
    if (verbose) puts("STUB: TCGetSourceRef called");
    return NULL;
}

void* TCGetTimeCodeAtTime(void)
{
    if (verbose) puts("STUB: TCGetTimeCodeAtTime called");
    return NULL;
}

void* TCGetTimeCodeFlags(void)
{
    if (verbose) puts("STUB: TCGetTimeCodeFlags called");
    return NULL;
}

void* TCSetDisplayOptions(void)
{
    if (verbose) puts("STUB: TCSetDisplayOptions called");
    return NULL;
}

void* TCSetSourceRef(void)
{
    if (verbose) puts("STUB: TCSetSourceRef called");
    return NULL;
}

void* TCSetTimeCodeFlags(void)
{
    if (verbose) puts("STUB: TCSetTimeCodeFlags called");
    return NULL;
}

void* TCTimeCodeCounterToFrameNumber(void)
{
    if (verbose) puts("STUB: TCTimeCodeCounterToFrameNumber called");
    return NULL;
}

void* TCTimeCodeCounterToString(void)
{
    if (verbose) puts("STUB: TCTimeCodeCounterToString called");
    return NULL;
}

void* TCTimeCodeTimeToFrameNumber(void)
{
    if (verbose) puts("STUB: TCTimeCodeTimeToFrameNumber called");
    return NULL;
}

void* TCTimeCodeTimeToString(void)
{
    if (verbose) puts("STUB: TCTimeCodeTimeToString called");
    return NULL;
}

void* TCTimeCodeToFrameNumber(void)
{
    if (verbose) puts("STUB: TCTimeCodeToFrameNumber called");
    return NULL;
}

void* TCTimeCodeToString(void)
{
    if (verbose) puts("STUB: TCTimeCodeToString called");
    return NULL;
}

void* TaskMovie(void)
{
    if (verbose) puts("STUB: TaskMovie called");
    return NULL;
}

void* TerminateQTS(void)
{
    if (verbose) puts("STUB: TerminateQTS called");
    return NULL;
}

void* TextExportGetDisplayData(void)
{
    if (verbose) puts("STUB: TextExportGetDisplayData called");
    return NULL;
}

void* TextExportGetSettings(void)
{
    if (verbose) puts("STUB: TextExportGetSettings called");
    return NULL;
}

void* TextExportGetTimeFraction(void)
{
    if (verbose) puts("STUB: TextExportGetTimeFraction called");
    return NULL;
}

void* TextExportSetSettings(void)
{
    if (verbose) puts("STUB: TextExportSetSettings called");
    return NULL;
}

void* TextExportSetTimeFraction(void)
{
    if (verbose) puts("STUB: TextExportSetTimeFraction called");
    return NULL;
}

void* TextMediaAddHiliteSample(void)
{
    if (verbose) puts("STUB: TextMediaAddHiliteSample called");
    return NULL;
}

void* TextMediaAddTESample(void)
{
    if (verbose) puts("STUB: TextMediaAddTESample called");
    return NULL;
}

void* TextMediaAddTextSample(void)
{
    if (verbose) puts("STUB: TextMediaAddTextSample called");
    return NULL;
}

void* TextMediaDrawRaw(void)
{
    if (verbose) puts("STUB: TextMediaDrawRaw called");
    return NULL;
}

void* TextMediaFindNextText(void)
{
    if (verbose) puts("STUB: TextMediaFindNextText called");
    return NULL;
}

void* TextMediaFindNextTextCFString(void)
{
    if (verbose) puts("STUB: TextMediaFindNextTextCFString called");
    return NULL;
}

void* TextMediaGetMetrics(void)
{
    if (verbose) puts("STUB: TextMediaGetMetrics called");
    return NULL;
}

void* TextMediaGetTextProperty(void)
{
    if (verbose) puts("STUB: TextMediaGetTextProperty called");
    return NULL;
}

void* TextMediaHiliteTextSample(void)
{
    if (verbose) puts("STUB: TextMediaHiliteTextSample called");
    return NULL;
}

void* TextMediaRawIdle(void)
{
    if (verbose) puts("STUB: TextMediaRawIdle called");
    return NULL;
}

void* TextMediaRawSetup(void)
{
    if (verbose) puts("STUB: TextMediaRawSetup called");
    return NULL;
}

void* TextMediaSetTextProc(void)
{
    if (verbose) puts("STUB: TextMediaSetTextProc called");
    return NULL;
}

void* TextMediaSetTextProperty(void)
{
    if (verbose) puts("STUB: TextMediaSetTextProperty called");
    return NULL;
}

void* TextMediaSetTextSampleData(void)
{
    if (verbose) puts("STUB: TextMediaSetTextSampleData called");
    return NULL;
}

void* TrackCompositingChanged(void)
{
    if (verbose) puts("STUB: TrackCompositingChanged called");
    return NULL;
}

void* TrackIsActive(void)
{
    if (verbose) puts("STUB: TrackIsActive called");
    return NULL;
}

void* TrackTimeToMediaDisplayTime(void)
{
    if (verbose) puts("STUB: TrackTimeToMediaDisplayTime called");
    return NULL;
}

void* TrackTimeToMediaTime(void)
{
    if (verbose) puts("STUB: TrackTimeToMediaTime called");
    return NULL;
}

void* TrackTimeToMediaTimePriv(void)
{
    if (verbose) puts("STUB: TrackTimeToMediaTimePriv called");
    return NULL;
}

void* TransformFixedPoints(void)
{
    if (verbose) puts("STUB: TransformFixedPoints called");
    return NULL;
}

void* TransformFixedRect(void)
{
    if (verbose) puts("STUB: TransformFixedRect called");
    return NULL;
}

void* TransformPoints(void)
{
    if (verbose) puts("STUB: TransformPoints called");
    return NULL;
}

void* TransformRect(void)
{
    if (verbose) puts("STUB: TransformRect called");
    return NULL;
}

void* TransformRgn(void)
{
    if (verbose) puts("STUB: TransformRgn called");
    return NULL;
}

void* TranslateMatrix(void)
{
    if (verbose) puts("STUB: TranslateMatrix called");
    return NULL;
}

void* TranslateMatrixTo(void)
{
    if (verbose) puts("STUB: TranslateMatrixTo called");
    return NULL;
}

void* TrimImage(void)
{
    if (verbose) puts("STUB: TrimImage called");
    return NULL;
}

void* TuneFlush(void)
{
    if (verbose) puts("STUB: TuneFlush called");
    return NULL;
}

void* TuneGetIndexedNoteChannel(void)
{
    if (verbose) puts("STUB: TuneGetIndexedNoteChannel called");
    return NULL;
}

void* TuneGetNoteAllocator(void)
{
    if (verbose) puts("STUB: TuneGetNoteAllocator called");
    return NULL;
}

void* TuneGetPartMix(void)
{
    if (verbose) puts("STUB: TuneGetPartMix called");
    return NULL;
}

void* TuneGetState(void)
{
    if (verbose) puts("STUB: TuneGetState called");
    return NULL;
}

void* TuneGetStatus(void)
{
    if (verbose) puts("STUB: TuneGetStatus called");
    return NULL;
}

void* TuneGetTimeBase(void)
{
    if (verbose) puts("STUB: TuneGetTimeBase called");
    return NULL;
}

void* TuneGetTimeScale(void)
{
    if (verbose) puts("STUB: TuneGetTimeScale called");
    return NULL;
}

void* TuneGetVolume(void)
{
    if (verbose) puts("STUB: TuneGetVolume called");
    return NULL;
}

void* TuneInstant(void)
{
    if (verbose) puts("STUB: TuneInstant called");
    return NULL;
}

void* TunePreroll(void)
{
    if (verbose) puts("STUB: TunePreroll called");
    return NULL;
}

void* TuneQueue(void)
{
    if (verbose) puts("STUB: TuneQueue called");
    return NULL;
}

void* TuneQueueWithSize(void)
{
    if (verbose) puts("STUB: TuneQueueWithSize called");
    return NULL;
}

void* TuneResume(void)
{
    if (verbose) puts("STUB: TuneResume called");
    return NULL;
}

void* TuneSetBalance(void)
{
    if (verbose) puts("STUB: TuneSetBalance called");
    return NULL;
}

void* TuneSetHeader(void)
{
    if (verbose) puts("STUB: TuneSetHeader called");
    return NULL;
}

void* TuneSetHeaderWithSize(void)
{
    if (verbose) puts("STUB: TuneSetHeaderWithSize called");
    return NULL;
}

void* TuneSetNoteChannels(void)
{
    if (verbose) puts("STUB: TuneSetNoteChannels called");
    return NULL;
}

void* TuneSetPartMix(void)
{
    if (verbose) puts("STUB: TuneSetPartMix called");
    return NULL;
}

void* TuneSetPartTranspose(void)
{
    if (verbose) puts("STUB: TuneSetPartTranspose called");
    return NULL;
}

void* TuneSetSofter(void)
{
    if (verbose) puts("STUB: TuneSetSofter called");
    return NULL;
}

void* TuneSetSoundLocalization(void)
{
    if (verbose) puts("STUB: TuneSetSoundLocalization called");
    return NULL;
}

void* TuneSetTimeScale(void)
{
    if (verbose) puts("STUB: TuneSetTimeScale called");
    return NULL;
}

void* TuneSetVolume(void)
{
    if (verbose) puts("STUB: TuneSetVolume called");
    return NULL;
}

void* TuneStop(void)
{
    if (verbose) puts("STUB: TuneStop called");
    return NULL;
}

void* TuneTask(void)
{
    if (verbose) puts("STUB: TuneTask called");
    return NULL;
}

void* TuneUnroll(void)
{
    if (verbose) puts("STUB: TuneUnroll called");
    return NULL;
}

void* TweenerDoTween(void)
{
    if (verbose) puts("STUB: TweenerDoTween called");
    return NULL;
}

void* TweenerInitialize(void)
{
    if (verbose) puts("STUB: TweenerInitialize called");
    return NULL;
}

void* TweenerReset(void)
{
    if (verbose) puts("STUB: TweenerReset called");
    return NULL;
}

void* TypeCoercionCoerceData(void)
{
    if (verbose) puts("STUB: TypeCoercionCoerceData called");
    return NULL;
}

void* UnsignedFixMulDiv(void)
{
    if (verbose) puts("STUB: UnsignedFixMulDiv called");
    return NULL;
}

void* UpdateLogFileCheck(void)
{
    if (verbose) puts("STUB: UpdateLogFileCheck called");
    return NULL;
}

void* UpdateLogFileClose(void)
{
    if (verbose) puts("STUB: UpdateLogFileClose called");
    return NULL;
}

void* UpdateLogFileOpen(void)
{
    if (verbose) puts("STUB: UpdateLogFileOpen called");
    return NULL;
}

void* UpdateLogFilePrintfGuts(void)
{
    if (verbose) puts("STUB: UpdateLogFilePrintfGuts called");
    return NULL;
}

void* UpdateMovie(void)
{
    if (verbose) puts("STUB: UpdateMovie called");
    return NULL;
}

void* UpdateMovieInStorage(void)
{
    if (verbose) puts("STUB: UpdateMovieInStorage called");
    return NULL;
}

void* UpdateMovieResource(void)
{
    if (verbose) puts("STUB: UpdateMovieResource called");
    return NULL;
}

void* UpdateRemapTable(void)
{
    if (verbose) puts("STUB: UpdateRemapTable called");
    return NULL;
}

void* UseMovieEditState(void)
{
    if (verbose) puts("STUB: UseMovieEditState called");
    return NULL;
}

void* UseTrackEditState(void)
{
    if (verbose) puts("STUB: UseTrackEditState called");
    return NULL;
}

void* VDAddKeyColor(void)
{
    if (verbose) puts("STUB: VDAddKeyColor called");
    return NULL;
}

void* VDCaptureStateChanging(void)
{
    if (verbose) puts("STUB: VDCaptureStateChanging called");
    return NULL;
}

void* VDClearClipRgn(void)
{
    if (verbose) puts("STUB: VDClearClipRgn called");
    return NULL;
}

void* VDCompressDone(void)
{
    if (verbose) puts("STUB: VDCompressDone called");
    return NULL;
}

void* VDCompressOneFrameAsync(void)
{
    if (verbose) puts("STUB: VDCompressOneFrameAsync called");
    return NULL;
}

void* VDCopyPreferredAudioDevice(void)
{
    if (verbose) puts("STUB: VDCopyPreferredAudioDevice called");
    return NULL;
}

void* VDDone(void)
{
    if (verbose) puts("STUB: VDDone called");
    return NULL;
}

void* VDGetActiveSrcRect(void)
{
    if (verbose) puts("STUB: VDGetActiveSrcRect called");
    return NULL;
}

void* VDGetBlackLevelValue(void)
{
    if (verbose) puts("STUB: VDGetBlackLevelValue called");
    return NULL;
}

void* VDGetBrightness(void)
{
    if (verbose) puts("STUB: VDGetBrightness called");
    return NULL;
}

void* VDGetCLUTInUse(void)
{
    if (verbose) puts("STUB: VDGetCLUTInUse called");
    return NULL;
}

void* VDGetClipState(void)
{
    if (verbose) puts("STUB: VDGetClipState called");
    return NULL;
}

void* VDGetCompressionTime(void)
{
    if (verbose) puts("STUB: VDGetCompressionTime called");
    return NULL;
}

void* VDGetCompressionTypes(void)
{
    if (verbose) puts("STUB: VDGetCompressionTypes called");
    return NULL;
}

void* VDGetContrast(void)
{
    if (verbose) puts("STUB: VDGetContrast called");
    return NULL;
}

void* VDGetCurrentFlags(void)
{
    if (verbose) puts("STUB: VDGetCurrentFlags called");
    return NULL;
}

void* VDGetDMADepths(void)
{
    if (verbose) puts("STUB: VDGetDMADepths called");
    return NULL;
}

void* VDGetDataRate(void)
{
    if (verbose) puts("STUB: VDGetDataRate called");
    return NULL;
}

void* VDGetDeviceNameAndFlags(void)
{
    if (verbose) puts("STUB: VDGetDeviceNameAndFlags called");
    return NULL;
}

void* VDGetDigitizerInfo(void)
{
    if (verbose) puts("STUB: VDGetDigitizerInfo called");
    return NULL;
}

void* VDGetDigitizerRect(void)
{
    if (verbose) puts("STUB: VDGetDigitizerRect called");
    return NULL;
}

void* VDGetFieldPreference(void)
{
    if (verbose) puts("STUB: VDGetFieldPreference called");
    return NULL;
}

void* VDGetHue(void)
{
    if (verbose) puts("STUB: VDGetHue called");
    return NULL;
}

void* VDGetImageDescription(void)
{
    if (verbose) puts("STUB: VDGetImageDescription called");
    return NULL;
}

void* VDGetInput(void)
{
    if (verbose) puts("STUB: VDGetInput called");
    return NULL;
}

void* VDGetInputColorSpaceMode(void)
{
    if (verbose) puts("STUB: VDGetInputColorSpaceMode called");
    return NULL;
}

void* VDGetInputFormat(void)
{
    if (verbose) puts("STUB: VDGetInputFormat called");
    return NULL;
}

void* VDGetInputGammaRecord(void)
{
    if (verbose) puts("STUB: VDGetInputGammaRecord called");
    return NULL;
}

void* VDGetInputGammaValue(void)
{
    if (verbose) puts("STUB: VDGetInputGammaValue called");
    return NULL;
}

void* VDGetInputName(void)
{
    if (verbose) puts("STUB: VDGetInputName called");
    return NULL;
}

void* VDGetKeyColor(void)
{
    if (verbose) puts("STUB: VDGetKeyColor called");
    return NULL;
}

void* VDGetKeyColorRange(void)
{
    if (verbose) puts("STUB: VDGetKeyColorRange called");
    return NULL;
}

void* VDGetMaskPixMap(void)
{
    if (verbose) puts("STUB: VDGetMaskPixMap called");
    return NULL;
}

void* VDGetMaskandValue(void)
{
    if (verbose) puts("STUB: VDGetMaskandValue called");
    return NULL;
}

void* VDGetMaxAuxBuffer(void)
{
    if (verbose) puts("STUB: VDGetMaxAuxBuffer called");
    return NULL;
}

void* VDGetMaxSrcRect(void)
{
    if (verbose) puts("STUB: VDGetMaxSrcRect called");
    return NULL;
}

void* VDGetNextKeyColor(void)
{
    if (verbose) puts("STUB: VDGetNextKeyColor called");
    return NULL;
}

void* VDGetNumberOfInputs(void)
{
    if (verbose) puts("STUB: VDGetNumberOfInputs called");
    return NULL;
}

void* VDGetPLLFilterType(void)
{
    if (verbose) puts("STUB: VDGetPLLFilterType called");
    return NULL;
}

void* VDGetPlayThruDestination(void)
{
    if (verbose) puts("STUB: VDGetPlayThruDestination called");
    return NULL;
}

void* VDGetPreferredClock(void)
{
    if (verbose) puts("STUB: VDGetPreferredClock called");
    return NULL;
}

void* VDGetPreferredImageDimensions(void)
{
    if (verbose) puts("STUB: VDGetPreferredImageDimensions called");
    return NULL;
}

void* VDGetPreferredTimeScale(void)
{
    if (verbose) puts("STUB: VDGetPreferredTimeScale called");
    return NULL;
}

void* VDGetSaturation(void)
{
    if (verbose) puts("STUB: VDGetSaturation called");
    return NULL;
}

void* VDGetSharpness(void)
{
    if (verbose) puts("STUB: VDGetSharpness called");
    return NULL;
}

void* VDGetSoundInputDriver(void)
{
    if (verbose) puts("STUB: VDGetSoundInputDriver called");
    return NULL;
}

void* VDGetSoundInputSource(void)
{
    if (verbose) puts("STUB: VDGetSoundInputSource called");
    return NULL;
}

void* VDGetTimeCode(void)
{
    if (verbose) puts("STUB: VDGetTimeCode called");
    return NULL;
}

void* VDGetUniqueIDs(void)
{
    if (verbose) puts("STUB: VDGetUniqueIDs called");
    return NULL;
}

void* VDGetVBlankRect(void)
{
    if (verbose) puts("STUB: VDGetVBlankRect called");
    return NULL;
}

void* VDGetVideoDefaults(void)
{
    if (verbose) puts("STUB: VDGetVideoDefaults called");
    return NULL;
}

void* VDGetWhiteLevelValue(void)
{
    if (verbose) puts("STUB: VDGetWhiteLevelValue called");
    return NULL;
}

void* VDGrabOneFrame(void)
{
    if (verbose) puts("STUB: VDGrabOneFrame called");
    return NULL;
}

void* VDGrabOneFrameAsync(void)
{
    if (verbose) puts("STUB: VDGrabOneFrameAsync called");
    return NULL;
}

void* VDIIDCGetCSRData(void)
{
    if (verbose) puts("STUB: VDIIDCGetCSRData called");
    return NULL;
}

void* VDIIDCGetDefaultFeatures(void)
{
    if (verbose) puts("STUB: VDIIDCGetDefaultFeatures called");
    return NULL;
}

void* VDIIDCGetFeatures(void)
{
    if (verbose) puts("STUB: VDIIDCGetFeatures called");
    return NULL;
}

void* VDIIDCGetFeaturesForSpecifier(void)
{
    if (verbose) puts("STUB: VDIIDCGetFeaturesForSpecifier called");
    return NULL;
}

void* VDIIDCSetCSRData(void)
{
    if (verbose) puts("STUB: VDIIDCSetCSRData called");
    return NULL;
}

void* VDIIDCSetFeatures(void)
{
    if (verbose) puts("STUB: VDIIDCSetFeatures called");
    return NULL;
}

void* VDPreflightDestination(void)
{
    if (verbose) puts("STUB: VDPreflightDestination called");
    return NULL;
}

void* VDPreflightGlobalRect(void)
{
    if (verbose) puts("STUB: VDPreflightGlobalRect called");
    return NULL;
}

void* VDReleaseAsyncBuffers(void)
{
    if (verbose) puts("STUB: VDReleaseAsyncBuffers called");
    return NULL;
}

void* VDReleaseCompressBuffer(void)
{
    if (verbose) puts("STUB: VDReleaseCompressBuffer called");
    return NULL;
}

void* VDResetCompressSequence(void)
{
    if (verbose) puts("STUB: VDResetCompressSequence called");
    return NULL;
}

void* VDSelectUniqueIDs(void)
{
    if (verbose) puts("STUB: VDSelectUniqueIDs called");
    return NULL;
}

void* VDSetBlackLevelValue(void)
{
    if (verbose) puts("STUB: VDSetBlackLevelValue called");
    return NULL;
}

void* VDSetBrightness(void)
{
    if (verbose) puts("STUB: VDSetBrightness called");
    return NULL;
}

void* VDSetClipRgn(void)
{
    if (verbose) puts("STUB: VDSetClipRgn called");
    return NULL;
}

void* VDSetClipState(void)
{
    if (verbose) puts("STUB: VDSetClipState called");
    return NULL;
}

void* VDSetCompression(void)
{
    if (verbose) puts("STUB: VDSetCompression called");
    return NULL;
}

void* VDSetCompressionOnOff(void)
{
    if (verbose) puts("STUB: VDSetCompressionOnOff called");
    return NULL;
}

void* VDSetContrast(void)
{
    if (verbose) puts("STUB: VDSetContrast called");
    return NULL;
}

void* VDSetDataRate(void)
{
    if (verbose) puts("STUB: VDSetDataRate called");
    return NULL;
}

void* VDSetDestinationPort(void)
{
    if (verbose) puts("STUB: VDSetDestinationPort called");
    return NULL;
}

void* VDSetDigitizerRect(void)
{
    if (verbose) puts("STUB: VDSetDigitizerRect called");
    return NULL;
}

void* VDSetDigitizerUserInterrupt(void)
{
    if (verbose) puts("STUB: VDSetDigitizerUserInterrupt called");
    return NULL;
}

void* VDSetFieldPreference(void)
{
    if (verbose) puts("STUB: VDSetFieldPreference called");
    return NULL;
}

void* VDSetFrameRate(void)
{
    if (verbose) puts("STUB: VDSetFrameRate called");
    return NULL;
}

void* VDSetHue(void)
{
    if (verbose) puts("STUB: VDSetHue called");
    return NULL;
}

void* VDSetInput(void)
{
    if (verbose) puts("STUB: VDSetInput called");
    return NULL;
}

void* VDSetInputColorSpaceMode(void)
{
    if (verbose) puts("STUB: VDSetInputColorSpaceMode called");
    return NULL;
}

void* VDSetInputGammaRecord(void)
{
    if (verbose) puts("STUB: VDSetInputGammaRecord called");
    return NULL;
}

void* VDSetInputGammaValue(void)
{
    if (verbose) puts("STUB: VDSetInputGammaValue called");
    return NULL;
}

void* VDSetInputStandard(void)
{
    if (verbose) puts("STUB: VDSetInputStandard called");
    return NULL;
}

void* VDSetKeyColor(void)
{
    if (verbose) puts("STUB: VDSetKeyColor called");
    return NULL;
}

void* VDSetKeyColorRange(void)
{
    if (verbose) puts("STUB: VDSetKeyColorRange called");
    return NULL;
}

void* VDSetMasterBlendLevel(void)
{
    if (verbose) puts("STUB: VDSetMasterBlendLevel called");
    return NULL;
}

void* VDSetPLLFilterType(void)
{
    if (verbose) puts("STUB: VDSetPLLFilterType called");
    return NULL;
}

void* VDSetPlayThruDestination(void)
{
    if (verbose) puts("STUB: VDSetPlayThruDestination called");
    return NULL;
}

void* VDSetPlayThruGlobalRect(void)
{
    if (verbose) puts("STUB: VDSetPlayThruGlobalRect called");
    return NULL;
}

void* VDSetPlayThruOnOff(void)
{
    if (verbose) puts("STUB: VDSetPlayThruOnOff called");
    return NULL;
}

void* VDSetPreferredImageDimensions(void)
{
    if (verbose) puts("STUB: VDSetPreferredImageDimensions called");
    return NULL;
}

void* VDSetPreferredPacketSize(void)
{
    if (verbose) puts("STUB: VDSetPreferredPacketSize called");
    return NULL;
}

void* VDSetSaturation(void)
{
    if (verbose) puts("STUB: VDSetSaturation called");
    return NULL;
}

void* VDSetSharpness(void)
{
    if (verbose) puts("STUB: VDSetSharpness called");
    return NULL;
}

void* VDSetTimeBase(void)
{
    if (verbose) puts("STUB: VDSetTimeBase called");
    return NULL;
}

void* VDSetWhiteLevelValue(void)
{
    if (verbose) puts("STUB: VDSetWhiteLevelValue called");
    return NULL;
}

void* VDSetupBuffers(void)
{
    if (verbose) puts("STUB: VDSetupBuffers called");
    return NULL;
}

void* VDUseSafeBuffers(void)
{
    if (verbose) puts("STUB: VDUseSafeBuffers called");
    return NULL;
}

void* VDUseThisCLUT(void)
{
    if (verbose) puts("STUB: VDUseThisCLUT called");
    return NULL;
}

void* ValidateQuickTimeRegistration(void)
{
    if (verbose) puts("STUB: ValidateQuickTimeRegistration called");
    return NULL;
}

void* ValidateQuickTimeVersionRegistration(void)
{
    if (verbose) puts("STUB: ValidateQuickTimeVersionRegistration called");
    return NULL;
}

void* VideoMediaGetCodecParameter(void)
{
    if (verbose) puts("STUB: VideoMediaGetCodecParameter called");
    return NULL;
}

void* VideoMediaGetDecompressor(void)
{
    if (verbose) puts("STUB: VideoMediaGetDecompressor called");
    return NULL;
}

void* VideoMediaGetFlags(void)
{
    if (verbose) puts("STUB: VideoMediaGetFlags called");
    return NULL;
}

void* VideoMediaGetGraphicsMode(void)
{
    if (verbose) puts("STUB: VideoMediaGetGraphicsMode called");
    return NULL;
}

void* VideoMediaGetPerformanceStats(void)
{
    if (verbose) puts("STUB: VideoMediaGetPerformanceStats called");
    return NULL;
}

void* VideoMediaGetStallCount(void)
{
    if (verbose) puts("STUB: VideoMediaGetStallCount called");
    return NULL;
}

void* VideoMediaGetStatistics(void)
{
    if (verbose) puts("STUB: VideoMediaGetStatistics called");
    return NULL;
}

void* VideoMediaResetStatistics(void)
{
    if (verbose) puts("STUB: VideoMediaResetStatistics called");
    return NULL;
}

void* VideoMediaSetCacheDebug(void)
{
    if (verbose) puts("STUB: VideoMediaSetCacheDebug called");
    return NULL;
}

void* VideoMediaSetCodecParameter(void)
{
    if (verbose) puts("STUB: VideoMediaSetCodecParameter called");
    return NULL;
}

void* VideoMediaSetDebug(void)
{
    if (verbose) puts("STUB: VideoMediaSetDebug called");
    return NULL;
}

void* VideoMediaSetDecompressor(void)
{
    if (verbose) puts("STUB: VideoMediaSetDecompressor called");
    return NULL;
}

void* VideoMediaSetFlags(void)
{
    if (verbose) puts("STUB: VideoMediaSetFlags called");
    return NULL;
}

void* VideoMediaSetGraphicsMode(void)
{
    if (verbose) puts("STUB: VideoMediaSetGraphicsMode called");
    return NULL;
}

void* VideoMediaSetPerformanceStats(void)
{
    if (verbose) puts("STUB: VideoMediaSetPerformanceStats called");
    return NULL;
}

void* VideoMediaSetTimeOutMaximum(void)
{
    if (verbose) puts("STUB: VideoMediaSetTimeOutMaximum called");
    return NULL;
}

void* VisualCompositorVUBeginConfiguration(void)
{
    if (verbose) puts("STUB: VisualCompositorVUBeginConfiguration called");
    return NULL;
}

void* VisualCompositorVUCommitConfiguration(void)
{
    if (verbose) puts("STUB: VisualCompositorVUCommitConfiguration called");
    return NULL;
}

void* WFAppend(void)
{
    if (verbose) puts("STUB: WFAppend called");
    return NULL;
}

void* WFAppend64(void)
{
    if (verbose) puts("STUB: WFAppend64 called");
    return NULL;
}

void* WFClose(void)
{
    if (verbose) puts("STUB: WFClose called");
    return NULL;
}

void* WFFlush(void)
{
    if (verbose) puts("STUB: WFFlush called");
    return NULL;
}

void* WFGetEOF(void)
{
    if (verbose) puts("STUB: WFGetEOF called");
    return NULL;
}

void* WFGetEOF64(void)
{
    if (verbose) puts("STUB: WFGetEOF64 called");
    return NULL;
}

void* WFGetEOF64_priv(void)
{
    if (verbose) puts("STUB: WFGetEOF64_priv called");
    return NULL;
}

void* WFOpen(void)
{
    if (verbose) puts("STUB: WFOpen called");
    return NULL;
}

void* WFOpenRes(void)
{
    if (verbose) puts("STUB: WFOpenRes called");
    return NULL;
}

void* WFOpenResWithFlags(void)
{
    if (verbose) puts("STUB: WFOpenResWithFlags called");
    return NULL;
}

void* WFOpenUnicodeResWithFlags(void)
{
    if (verbose) puts("STUB: WFOpenUnicodeResWithFlags called");
    return NULL;
}

void* WFOpenUnicodeWithFlags(void)
{
    if (verbose) puts("STUB: WFOpenUnicodeWithFlags called");
    return NULL;
}

void* WFOpenWithFlags(void)
{
    if (verbose) puts("STUB: WFOpenWithFlags called");
    return NULL;
}

void* WFOpen_priv(void)
{
    if (verbose) puts("STUB: WFOpen_priv called");
    return NULL;
}

void* WFRead(void)
{
    if (verbose) puts("STUB: WFRead called");
    return NULL;
}

void* WFRead64(void)
{
    if (verbose) puts("STUB: WFRead64 called");
    return NULL;
}

void* WFRead64WithFlags(void)
{
    if (verbose) puts("STUB: WFRead64WithFlags called");
    return NULL;
}

void* WFReadWithFlags(void)
{
    if (verbose) puts("STUB: WFReadWithFlags called");
    return NULL;
}

void* WFResolveAlias(void)
{
    if (verbose) puts("STUB: WFResolveAlias called");
    return NULL;
}

void* WFWrite(void)
{
    if (verbose) puts("STUB: WFWrite called");
    return NULL;
}

void* WFWrite64(void)
{
    if (verbose) puts("STUB: WFWrite64 called");
    return NULL;
}

void* WFWrite64_priv(void)
{
    if (verbose) puts("STUB: WFWrite64_priv called");
    return NULL;
}

void* WarholFSResolveAlias(void)
{
    if (verbose) puts("STUB: WarholFSResolveAlias called");
    return NULL;
}

void* WarholFSResolveAlias2(void)
{
    if (verbose) puts("STUB: WarholFSResolveAlias2 called");
    return NULL;
}

void* WarholFSResolveAlias2WithFlags(void)
{
    if (verbose) puts("STUB: WarholFSResolveAlias2WithFlags called");
    return NULL;
}

void* WarholFSResolveAliasWithFlags(void)
{
    if (verbose) puts("STUB: WarholFSResolveAliasWithFlags called");
    return NULL;
}

void* WarholResolveAlias(void)
{
    if (verbose) puts("STUB: WarholResolveAlias called");
    return NULL;
}

void* WarholResolveAlias2(void)
{
    if (verbose) puts("STUB: WarholResolveAlias2 called");
    return NULL;
}

void* WarholResolveAlias2WithFlags(void)
{
    if (verbose) puts("STUB: WarholResolveAlias2WithFlags called");
    return NULL;
}

void* WarholResolveAliasWithFlags(void)
{
    if (verbose) puts("STUB: WarholResolveAliasWithFlags called");
    return NULL;
}

void* WarholResolveAlias_priv(void)
{
    if (verbose) puts("STUB: WarholResolveAlias_priv called");
    return NULL;
}

void* XMLParseAddAttribute(void)
{
    if (verbose) puts("STUB: XMLParseAddAttribute called");
    return NULL;
}

void* XMLParseAddAttributeAndValue(void)
{
    if (verbose) puts("STUB: XMLParseAddAttributeAndValue called");
    return NULL;
}

void* XMLParseAddAttributeValueKind(void)
{
    if (verbose) puts("STUB: XMLParseAddAttributeValueKind called");
    return NULL;
}

void* XMLParseAddElement(void)
{
    if (verbose) puts("STUB: XMLParseAddElement called");
    return NULL;
}

void* XMLParseAddMultipleAttributes(void)
{
    if (verbose) puts("STUB: XMLParseAddMultipleAttributes called");
    return NULL;
}

void* XMLParseAddMultipleAttributesAndValues(void)
{
    if (verbose) puts("STUB: XMLParseAddMultipleAttributesAndValues called");
    return NULL;
}

void* XMLParseAddNameSpace(void)
{
    if (verbose) puts("STUB: XMLParseAddNameSpace called");
    return NULL;
}

void* XMLParseDataRef(void)
{
    if (verbose) puts("STUB: XMLParseDataRef called");
    return NULL;
}

void* XMLParseDisposeXMLDoc(void)
{
    if (verbose) puts("STUB: XMLParseDisposeXMLDoc called");
    return NULL;
}

void* XMLParseFile(void)
{
    if (verbose) puts("STUB: XMLParseFile called");
    return NULL;
}

void* XMLParseGetDetailedParseError(void)
{
    if (verbose) puts("STUB: XMLParseGetDetailedParseError called");
    return NULL;
}

void* XMLParseSetCDataHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetCDataHandler called");
    return NULL;
}

void* XMLParseSetCharDataHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetCharDataHandler called");
    return NULL;
}

void* XMLParseSetCommentHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetCommentHandler called");
    return NULL;
}

void* XMLParseSetEndDocumentHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetEndDocumentHandler called");
    return NULL;
}

void* XMLParseSetEndElementHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetEndElementHandler called");
    return NULL;
}

void* XMLParseSetEventParseRefCon(void)
{
    if (verbose) puts("STUB: XMLParseSetEventParseRefCon called");
    return NULL;
}

void* XMLParseSetOffsetAndLimit(void)
{
    if (verbose) puts("STUB: XMLParseSetOffsetAndLimit called");
    return NULL;
}

void* XMLParseSetPreprocessInstructionHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetPreprocessInstructionHandler called");
    return NULL;
}

void* XMLParseSetStartDocumentHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetStartDocumentHandler called");
    return NULL;
}

void* XMLParseSetStartElementHandler(void)
{
    if (verbose) puts("STUB: XMLParseSetStartElementHandler called");
    return NULL;
}

void* _DebugQTMLCreateMutex(void)
{
    if (verbose) puts("STUB: _DebugQTMLCreateMutex called");
    return NULL;
}

void* _DebugQTMLDestroyMutex(void)
{
    if (verbose) puts("STUB: _DebugQTMLDestroyMutex called");
    return NULL;
}

void* _DebugQTMLGrabMutex(void)
{
    if (verbose) puts("STUB: _DebugQTMLGrabMutex called");
    return NULL;
}

void* _DebugQTMLReturnMutex(void)
{
    if (verbose) puts("STUB: _DebugQTMLReturnMutex called");
    return NULL;
}

void* _DebugQTMLTryGrabMutex(void)
{
    if (verbose) puts("STUB: _DebugQTMLTryGrabMutex called");
    return NULL;
}

void* _DebugQTVisualGraphGrabConfiguringMutex(void)
{
    if (verbose) puts("STUB: _DebugQTVisualGraphGrabConfiguringMutex called");
    return NULL;
}

void* _DebugQTVisualGraphGrabProcessingMutex(void)
{
    if (verbose) puts("STUB: _DebugQTVisualGraphGrabProcessingMutex called");
    return NULL;
}

void* _DebugQTVisualGraphReturnConfiguringMutex(void)
{
    if (verbose) puts("STUB: _DebugQTVisualGraphReturnConfiguringMutex called");
    return NULL;
}

void* _DebugQTVisualGraphReturnProcessingMutex(void)
{
    if (verbose) puts("STUB: _DebugQTVisualGraphReturnProcessingMutex called");
    return NULL;
}

void* _QTPrintMessage(void)
{
    if (verbose) puts("STUB: _QTPrintMessage called");
    return NULL;
}

void* _QTPrintObjectTree(void)
{
    if (verbose) puts("STUB: _QTPrintObjectTree called");
    return NULL;
}

void* _QTSetJavaThreadHook(void)
{
    if (verbose) puts("STUB: _QTSetJavaThreadHook called");
    return NULL;
}

void* __QTInitializeGestaltValues(void)
{
    if (verbose) puts("STUB: __QTInitializeGestaltValues called");
    return NULL;
}

void* findAtomByIDIgnoreAliases(void)
{
    if (verbose) puts("STUB: findAtomByIDIgnoreAliases called");
    return NULL;
}

void* findAtomByIndexIgnoreAliases(void)
{
    if (verbose) puts("STUB: findAtomByIndexIgnoreAliases called");
    return NULL;
}

void* getAtomDataFlags(void)
{
    if (verbose) puts("STUB: getAtomDataFlags called");
    return NULL;
}

void* getAtomSize(void)
{
    if (verbose) puts("STUB: getAtomSize called");
    return NULL;
}

void* icmGammaLevelIgnoringYCbCrMatrixDouble(void)
{
    if (verbose) puts("STUB: icmGammaLevelIgnoringYCbCrMatrixDouble called");
    return NULL;
}

void* icmTestCopyFromPixelBufferToPixelBuffer(void)
{
    if (verbose) puts("STUB: icmTestCopyFromPixelBufferToPixelBuffer called");
    return NULL;
}

void* isQTTraceEnabled(void)
{
    if (verbose) puts("STUB: isQTTraceEnabled called");
    return NULL;
}

void* privFindMediaPacketizer(void)
{
    if (verbose) puts("STUB: privFindMediaPacketizer called");
    return NULL;
}

void* qtAppendColonString(void)
{
    if (verbose) puts("STUB: qtAppendColonString called");
    return NULL;
}

void* qtConvertDoubleToString(void)
{
    if (verbose) puts("STUB: qtConvertDoubleToString called");
    return NULL;
}

void* qtCreateCannonicalFormat(void)
{
    if (verbose) puts("STUB: qtCreateCannonicalFormat called");
    return NULL;
}

void* qtDoDrawPictureFile(void)
{
    if (verbose) puts("STUB: qtDoDrawPictureFile called");
    return NULL;
}

void* qtGetModKeys(void)
{
    if (verbose) puts("STUB: qtGetModKeys called");
    return NULL;
}

void* qtReplaceDialogItem(void)
{
    if (verbose) puts("STUB: qtReplaceDialogItem called");
    return NULL;
}

void* qtSetItemFixed(void)
{
    if (verbose) puts("STUB: qtSetItemFixed called");
    return NULL;
}

void* qtSetItemNumber(void)
{
    if (verbose) puts("STUB: qtSetItemNumber called");
    return NULL;
}

void* qtSetItemText(void)
{
    if (verbose) puts("STUB: qtSetItemText called");
    return NULL;
}

void* qtaddLogEntry(void)
{
    if (verbose) puts("STUB: qtaddLogEntry called");
    return NULL;
}

void* qtrHandToHand(void)
{
    if (verbose) puts("STUB: qtrHandToHand called");
    return NULL;
}

void* qtsdebugf(void)
{
    if (verbose) puts("STUB: qtsdebugf called");
    return NULL;
}

void* rPtrToHand(void)
{
    if (verbose) puts("STUB: rPtrToHand called");
    return NULL;
}

void* rPurgeSpace(void)
{
    if (verbose) puts("STUB: rPurgeSpace called");
    return NULL;
}

void* rSndSoundManagerVersion(void)
{
    if (verbose) puts("STUB: rSndSoundManagerVersion called");
    return NULL;
}

void* setAtomAlias(void)
{
    if (verbose) puts("STUB: setAtomAlias called");
    return NULL;
}

void* setAtomDataFlags(void)
{
    if (verbose) puts("STUB: setAtomDataFlags called");
    return NULL;
}
