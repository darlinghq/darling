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


#include <SpeechRecognition/SpeechRecognition.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DisposeSRCallBackUPP(void)
{
    if (verbose) puts("STUB: DisposeSRCallBackUPP called");
    return NULL;
}

void* InvokeSRCallBackUPP(void)
{
    if (verbose) puts("STUB: InvokeSRCallBackUPP called");
    return NULL;
}

void* NewSRCallBackUPP(void)
{
    if (verbose) puts("STUB: NewSRCallBackUPP called");
    return NULL;
}

void* SFSAvatarCaptionVisible(void)
{
    if (verbose) puts("STUB: SFSAvatarCaptionVisible called");
    return NULL;
}

void* SFSCopyActiveRecognizer(void)
{
    if (verbose) puts("STUB: SFSCopyActiveRecognizer called");
    return NULL;
}

void* SFSCopyCurrentAvatarIdentifier(void)
{
    if (verbose) puts("STUB: SFSCopyCurrentAvatarIdentifier called");
    return NULL;
}

void* SFSCopyIndAvatarInfo(void)
{
    if (verbose) puts("STUB: SFSCopyIndAvatarInfo called");
    return NULL;
}

void* SFSCopyIndRecognizerPluginBundleRef(void)
{
    if (verbose) puts("STUB: SFSCopyIndRecognizerPluginBundleRef called");
    return NULL;
}

void* SFSCopyNamedAvatarInfo(void)
{
    if (verbose) puts("STUB: SFSCopyNamedAvatarInfo called");
    return NULL;
}

void* SFSCopyProperty(void)
{
    if (verbose) puts("STUB: SFSCopyProperty called");
    return NULL;
}

void* SFSCountAvatars(void)
{
    if (verbose) puts("STUB: SFSCountAvatars called");
    return NULL;
}

void* SFSCountRecognizerPlugins(void)
{
    if (verbose) puts("STUB: SFSCountRecognizerPlugins called");
    return NULL;
}

void* SFSCreateSpeechFeedbackService(void)
{
    if (verbose) puts("STUB: SFSCreateSpeechFeedbackService called");
    return NULL;
}

void* SFSCreateSpeechFeedbackServiceFromRecognizerIdentifier(void)
{
    if (verbose) puts("STUB: SFSCreateSpeechFeedbackServiceFromRecognizerIdentifier called");
    return NULL;
}

void* SFSDeminimizeFeedbackWindow(void)
{
    if (verbose) puts("STUB: SFSDeminimizeFeedbackWindow called");
    return NULL;
}

void* SFSDeminimizeUtilityWindow(void)
{
    if (verbose) puts("STUB: SFSDeminimizeUtilityWindow called");
    return NULL;
}

void* SFSFeedbackWindowVisible(void)
{
    if (verbose) puts("STUB: SFSFeedbackWindowVisible called");
    return NULL;
}

void* SFSGetAvatarExpression(void)
{
    if (verbose) puts("STUB: SFSGetAvatarExpression called");
    return NULL;
}

void* SFSGetTargetProcess(void)
{
    if (verbose) puts("STUB: SFSGetTargetProcess called");
    return NULL;
}

void* SFSHideAvatarCaption(void)
{
    if (verbose) puts("STUB: SFSHideAvatarCaption called");
    return NULL;
}

void* SFSHideFeedbackWindow(void)
{
    if (verbose) puts("STUB: SFSHideFeedbackWindow called");
    return NULL;
}

void* SFSHideRecognizerArea(void)
{
    if (verbose) puts("STUB: SFSHideRecognizerArea called");
    return NULL;
}

void* SFSHideTextArea(void)
{
    if (verbose) puts("STUB: SFSHideTextArea called");
    return NULL;
}

void* SFSHideUtilityWindow(void)
{
    if (verbose) puts("STUB: SFSHideUtilityWindow called");
    return NULL;
}

void* SFSIsRecognizerAreaVisible(void)
{
    if (verbose) puts("STUB: SFSIsRecognizerAreaVisible called");
    return NULL;
}

void* SFSIsTextAreaVisible(void)
{
    if (verbose) puts("STUB: SFSIsTextAreaVisible called");
    return NULL;
}

void* SFSMinimizeFeedbackWindow(void)
{
    if (verbose) puts("STUB: SFSMinimizeFeedbackWindow called");
    return NULL;
}

void* SFSMinimizeUtilityWindow(void)
{
    if (verbose) puts("STUB: SFSMinimizeUtilityWindow called");
    return NULL;
}

void* SFSReleaseSpeechFeedbackService(void)
{
    if (verbose) puts("STUB: SFSReleaseSpeechFeedbackService called");
    return NULL;
}

void* SFSRequestMicrophone(void)
{
    if (verbose) puts("STUB: SFSRequestMicrophone called");
    return NULL;
}

void* SFSReturnMicrophone(void)
{
    if (verbose) puts("STUB: SFSReturnMicrophone called");
    return NULL;
}

void* SFSSendMessageToRecognizerArea(void)
{
    if (verbose) puts("STUB: SFSSendMessageToRecognizerArea called");
    return NULL;
}

void* SFSSetActiveRecognizer(void)
{
    if (verbose) puts("STUB: SFSSetActiveRecognizer called");
    return NULL;
}

void* SFSSetAvatarCaption(void)
{
    if (verbose) puts("STUB: SFSSetAvatarCaption called");
    return NULL;
}

void* SFSSetAvatarExpression(void)
{
    if (verbose) puts("STUB: SFSSetAvatarExpression called");
    return NULL;
}

void* SFSSetAvatarSpeakingState(void)
{
    if (verbose) puts("STUB: SFSSetAvatarSpeakingState called");
    return NULL;
}

void* SFSSetIdentityBadge(void)
{
    if (verbose) puts("STUB: SFSSetIdentityBadge called");
    return NULL;
}

void* SFSSetMicrophoneMeterStatus(void)
{
    if (verbose) puts("STUB: SFSSetMicrophoneMeterStatus called");
    return NULL;
}

void* SFSSetProperty(void)
{
    if (verbose) puts("STUB: SFSSetProperty called");
    return NULL;
}

void* SFSSetRecognizerStartupAction(void)
{
    if (verbose) puts("STUB: SFSSetRecognizerStartupAction called");
    return NULL;
}

void* SFSSetSpeakableCommandsData(void)
{
    if (verbose) puts("STUB: SFSSetSpeakableCommandsData called");
    return NULL;
}

void* SFSShowAvatarCaption(void)
{
    if (verbose) puts("STUB: SFSShowAvatarCaption called");
    return NULL;
}

void* SFSShowFeedbackWindow(void)
{
    if (verbose) puts("STUB: SFSShowFeedbackWindow called");
    return NULL;
}

void* SFSShowRecognizerArea(void)
{
    if (verbose) puts("STUB: SFSShowRecognizerArea called");
    return NULL;
}

void* SFSShowTextArea(void)
{
    if (verbose) puts("STUB: SFSShowTextArea called");
    return NULL;
}

void* SFSShowUtilityWindow(void)
{
    if (verbose) puts("STUB: SFSShowUtilityWindow called");
    return NULL;
}

void* SFSSpeakAndDrawString(void)
{
    if (verbose) puts("STUB: SFSSpeakAndDrawString called");
    return NULL;
}

void* SFSSpeakAndDrawStringUsingVoice(void)
{
    if (verbose) puts("STUB: SFSSpeakAndDrawStringUsingVoice called");
    return NULL;
}

void* SFSSpecifyMicrophoneToMeter(void)
{
    if (verbose) puts("STUB: SFSSpecifyMicrophoneToMeter called");
    return NULL;
}

void* SFSSpeechBusy(void)
{
    if (verbose) puts("STUB: SFSSpeechBusy called");
    return NULL;
}

void* SFSStopSpeech(void)
{
    if (verbose) puts("STUB: SFSStopSpeech called");
    return NULL;
}

void* SFSSyncFeedbackWindowWithPrefs(void)
{
    if (verbose) puts("STUB: SFSSyncFeedbackWindowWithPrefs called");
    return NULL;
}

void* SFSUseAvatar(void)
{
    if (verbose) puts("STUB: SFSUseAvatar called");
    return NULL;
}

void* SFSUseAvatarBundle(void)
{
    if (verbose) puts("STUB: SFSUseAvatarBundle called");
    return NULL;
}

void* SFSUseRecognizerAreaBundle(void)
{
    if (verbose) puts("STUB: SFSUseRecognizerAreaBundle called");
    return NULL;
}

void* SFSUtilityWindowVisible(void)
{
    if (verbose) puts("STUB: SFSUtilityWindowVisible called");
    return NULL;
}

void* SRAddLanguageObject(void)
{
    if (verbose) puts("STUB: SRAddLanguageObject called");
    return NULL;
}

void* SRAddText(void)
{
    if (verbose) puts("STUB: SRAddText called");
    return NULL;
}

void* SRCancelRecognition(void)
{
    if (verbose) puts("STUB: SRCancelRecognition called");
    return NULL;
}

void* SRChangeLanguageObject(void)
{
    if (verbose) puts("STUB: SRChangeLanguageObject called");
    return NULL;
}

void* SRCloseRecognitionSystem(void)
{
    if (verbose) puts("STUB: SRCloseRecognitionSystem called");
    return NULL;
}

void* SRContinueRecognition(void)
{
    if (verbose) puts("STUB: SRContinueRecognition called");
    return NULL;
}

void* SRCountItems(void)
{
    if (verbose) puts("STUB: SRCountItems called");
    return NULL;
}

void* SRDrawRecognizedText(void)
{
    if (verbose) puts("STUB: SRDrawRecognizedText called");
    return NULL;
}

void* SRDrawText(void)
{
    if (verbose) puts("STUB: SRDrawText called");
    return NULL;
}

void* SREmptyLanguageObject(void)
{
    if (verbose) puts("STUB: SREmptyLanguageObject called");
    return NULL;
}

void* SRGetIndexedItem(void)
{
    if (verbose) puts("STUB: SRGetIndexedItem called");
    return NULL;
}

void* SRGetLanguageModel(void)
{
    if (verbose) puts("STUB: SRGetLanguageModel called");
    return NULL;
}

void* SRGetProperty(void)
{
    if (verbose) puts("STUB: SRGetProperty called");
    return NULL;
}

void* SRGetReference(void)
{
    if (verbose) puts("STUB: SRGetReference called");
    return NULL;
}

void* SRIdle(void)
{
    if (verbose) puts("STUB: SRIdle called");
    return NULL;
}

void* SRNewLanguageModel(void)
{
    if (verbose) puts("STUB: SRNewLanguageModel called");
    return NULL;
}

void* SRNewLanguageObjectFromDataFile(void)
{
    if (verbose) puts("STUB: SRNewLanguageObjectFromDataFile called");
    return NULL;
}

void* SRNewLanguageObjectFromHandle(void)
{
    if (verbose) puts("STUB: SRNewLanguageObjectFromHandle called");
    return NULL;
}

void* SRNewPath(void)
{
    if (verbose) puts("STUB: SRNewPath called");
    return NULL;
}

void* SRNewPhrase(void)
{
    if (verbose) puts("STUB: SRNewPhrase called");
    return NULL;
}

void* SRNewRecognizer(void)
{
    if (verbose) puts("STUB: SRNewRecognizer called");
    return NULL;
}

void* SRNewWord(void)
{
    if (verbose) puts("STUB: SRNewWord called");
    return NULL;
}

void* SROpenRecognitionSystem(void)
{
    if (verbose) puts("STUB: SROpenRecognitionSystem called");
    return NULL;
}

void* SRProcessBegin(void)
{
    if (verbose) puts("STUB: SRProcessBegin called");
    return NULL;
}

void* SRProcessEnd(void)
{
    if (verbose) puts("STUB: SRProcessEnd called");
    return NULL;
}

void* SRPutLanguageObjectIntoDataFile(void)
{
    if (verbose) puts("STUB: SRPutLanguageObjectIntoDataFile called");
    return NULL;
}

void* SRPutLanguageObjectIntoHandle(void)
{
    if (verbose) puts("STUB: SRPutLanguageObjectIntoHandle called");
    return NULL;
}

void* SRReleaseObject(void)
{
    if (verbose) puts("STUB: SRReleaseObject called");
    return NULL;
}

void* SRRemoveIndexedItem(void)
{
    if (verbose) puts("STUB: SRRemoveIndexedItem called");
    return NULL;
}

void* SRRemoveLanguageObject(void)
{
    if (verbose) puts("STUB: SRRemoveLanguageObject called");
    return NULL;
}

void* SRSetIndexedItem(void)
{
    if (verbose) puts("STUB: SRSetIndexedItem called");
    return NULL;
}

void* SRSetLanguageModel(void)
{
    if (verbose) puts("STUB: SRSetLanguageModel called");
    return NULL;
}

void* SRSetProperty(void)
{
    if (verbose) puts("STUB: SRSetProperty called");
    return NULL;
}

void* SRSpeakAndDrawText(void)
{
    if (verbose) puts("STUB: SRSpeakAndDrawText called");
    return NULL;
}

void* SRSpeakText(void)
{
    if (verbose) puts("STUB: SRSpeakText called");
    return NULL;
}

void* SRSpeechBusy(void)
{
    if (verbose) puts("STUB: SRSpeechBusy called");
    return NULL;
}

void* SRStartListening(void)
{
    if (verbose) puts("STUB: SRStartListening called");
    return NULL;
}

void* SRStopListening(void)
{
    if (verbose) puts("STUB: SRStopListening called");
    return NULL;
}

void* SRStopSpeech(void)
{
    if (verbose) puts("STUB: SRStopSpeech called");
    return NULL;
}
