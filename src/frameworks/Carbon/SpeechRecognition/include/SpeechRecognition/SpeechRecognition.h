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


#ifndef _SpeechRecognition_H_
#define _SpeechRecognition_H_

void* DisposeSRCallBackUPP(void);
void* InvokeSRCallBackUPP(void);
void* NewSRCallBackUPP(void);
void* SFSAvatarCaptionVisible(void);
void* SFSCopyActiveRecognizer(void);
void* SFSCopyCurrentAvatarIdentifier(void);
void* SFSCopyIndAvatarInfo(void);
void* SFSCopyIndRecognizerPluginBundleRef(void);
void* SFSCopyNamedAvatarInfo(void);
void* SFSCopyProperty(void);
void* SFSCountAvatars(void);
void* SFSCountRecognizerPlugins(void);
void* SFSCreateSpeechFeedbackService(void);
void* SFSCreateSpeechFeedbackServiceFromRecognizerIdentifier(void);
void* SFSDeminimizeFeedbackWindow(void);
void* SFSDeminimizeUtilityWindow(void);
void* SFSFeedbackWindowVisible(void);
void* SFSGetAvatarExpression(void);
void* SFSGetTargetProcess(void);
void* SFSHideAvatarCaption(void);
void* SFSHideFeedbackWindow(void);
void* SFSHideRecognizerArea(void);
void* SFSHideTextArea(void);
void* SFSHideUtilityWindow(void);
void* SFSIsRecognizerAreaVisible(void);
void* SFSIsTextAreaVisible(void);
void* SFSMinimizeFeedbackWindow(void);
void* SFSMinimizeUtilityWindow(void);
void* SFSReleaseSpeechFeedbackService(void);
void* SFSRequestMicrophone(void);
void* SFSReturnMicrophone(void);
void* SFSSendMessageToRecognizerArea(void);
void* SFSSetActiveRecognizer(void);
void* SFSSetAvatarCaption(void);
void* SFSSetAvatarExpression(void);
void* SFSSetAvatarSpeakingState(void);
void* SFSSetIdentityBadge(void);
void* SFSSetMicrophoneMeterStatus(void);
void* SFSSetProperty(void);
void* SFSSetRecognizerStartupAction(void);
void* SFSSetSpeakableCommandsData(void);
void* SFSShowAvatarCaption(void);
void* SFSShowFeedbackWindow(void);
void* SFSShowRecognizerArea(void);
void* SFSShowTextArea(void);
void* SFSShowUtilityWindow(void);
void* SFSSpeakAndDrawString(void);
void* SFSSpeakAndDrawStringUsingVoice(void);
void* SFSSpecifyMicrophoneToMeter(void);
void* SFSSpeechBusy(void);
void* SFSStopSpeech(void);
void* SFSSyncFeedbackWindowWithPrefs(void);
void* SFSUseAvatar(void);
void* SFSUseAvatarBundle(void);
void* SFSUseRecognizerAreaBundle(void);
void* SFSUtilityWindowVisible(void);
void* SRAddLanguageObject(void);
void* SRAddText(void);
void* SRCancelRecognition(void);
void* SRChangeLanguageObject(void);
void* SRCloseRecognitionSystem(void);
void* SRContinueRecognition(void);
void* SRCountItems(void);
void* SRDrawRecognizedText(void);
void* SRDrawText(void);
void* SREmptyLanguageObject(void);
void* SRGetIndexedItem(void);
void* SRGetLanguageModel(void);
void* SRGetProperty(void);
void* SRGetReference(void);
void* SRIdle(void);
void* SRNewLanguageModel(void);
void* SRNewLanguageObjectFromDataFile(void);
void* SRNewLanguageObjectFromHandle(void);
void* SRNewPath(void);
void* SRNewPhrase(void);
void* SRNewRecognizer(void);
void* SRNewWord(void);
void* SROpenRecognitionSystem(void);
void* SRProcessBegin(void);
void* SRProcessEnd(void);
void* SRPutLanguageObjectIntoDataFile(void);
void* SRPutLanguageObjectIntoHandle(void);
void* SRReleaseObject(void);
void* SRRemoveIndexedItem(void);
void* SRRemoveLanguageObject(void);
void* SRSetIndexedItem(void);
void* SRSetLanguageModel(void);
void* SRSetProperty(void);
void* SRSpeakAndDrawText(void);
void* SRSpeakText(void);
void* SRSpeechBusy(void);
void* SRStartListening(void);
void* SRStopListening(void);
void* SRStopSpeech(void);

#endif
