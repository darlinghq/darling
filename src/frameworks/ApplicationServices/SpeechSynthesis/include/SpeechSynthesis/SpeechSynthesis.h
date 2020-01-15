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


#ifndef _SpeechSynthesis_H_
#define _SpeechSynthesis_H_

void* CancelSpokenNotification(void);
void* CancelSpokenNotificationForProcess(void);
void* CancelTalkingAlert(void);
void* ContinueSpeech(void);
void* CopyAvailableIntervals(void);
void* CopyAvailablePhrases(void);
void* CopyIdentifierStringForVoiceSpec(void);
void* CopyPhonemesFromText(void);
void* CopySpeechProperty(void);
void* CopySpeechSynthesisVoicesForMode(void);
void* CopyTimeAnnouncementsPreferences(void);
void* CountVoices(void);
void* CreateSpeechLanguageLocale(void);
void* DisposeSpeechChannel(void);
void* GetIdentifierStringForPreferredVoiceInListWithLocale(void);
void* GetIdentifierStringForPreferredVoiceWithLocale(void);
void* GetIndVoice(void);
void* GetSpeechInfo(void);
void* GetSpeechPitch(void);
void* GetSpeechRate(void);
void* GetVoiceDescription(void);
void* GetVoiceInfo(void);
void* MakeVoiceSpec(void);
void* MakeVoiceSpecForIdentifierString(void);
void* NewSpeechChannel(void);
void* PauseSpeechAt(void);
void* RegisterSpokenNotificationForProcess(void);
void* RegisterTalkingAlertWithProperties(void);
void* SRSetSecurityFlags(void);
void* SUAddAppPathToLoginStartupList(void);
void* SUCopyAXUIElementExceptionsDictionary(void);
void* SUCopyKeyNamesPropertyList(void);
void* SUCopyPhraseArrayFromDisk(void);
void* SUIsMainBundleUsingEnglishLocalization(void);
void* SUIsProcessUsingEnglishLocalization(void);
void* SUPostKeyCodeWithModifiers(void);
void* SURemoveAppPathFromLoginStartupList(void);
void* SendMessageToSpeechSynthesisServer(void);
void* SetSpeechInfo(void);
void* SetSpeechPitch(void);
void* SetSpeechProperty(void);
void* SetSpeechRate(void);
void* SetTimeAnnouncementsPreferences(void);
void* SpeakBuffer(void);
void* SpeakCFString(void);
void* SpeakString(void);
void* SpeakText(void);
void* SpeechBusy(void);
void* SpeechBusySystemWide(void);
void* SpeechDaemonEnvironment(void);
void* SpeechDaemonSetLogging(void);
void* SpeechDaemonShutdown(void);
void* SpeechManagerVersion(void);
void* SpeechSynthesisConsoleLogCFString(void);
void* SpeechSynthesisConsoleLogTextBuffer(void);
void* SpeechSynthesisRegisterModuleURL(void);
void* SpeechSynthesisUnregisterModuleURL(void);
void* SpokenNotificationsEnabled(void);
void* SpokenNotificationsEnabledForProcess(void);
void* StopSpeech(void);
void* StopSpeechAt(void);
void* TTSIsSpeakingSelection(void);
void* TTSStartSpeakingSelection(void);
void* TTSStopSpeakingSelection(void);
void* TalkingAlertsEnabled(void);
void* TextToPhonemes(void);
void* UseDictionary(void);
void* UseSpeechDictionary(void);

#endif
