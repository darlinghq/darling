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


#include <SpeechSynthesis/SpeechSynthesis.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CancelSpokenNotification(void)
{
    if (verbose) puts("STUB: CancelSpokenNotification called");
    return NULL;
}

void* CancelSpokenNotificationForProcess(void)
{
    if (verbose) puts("STUB: CancelSpokenNotificationForProcess called");
    return NULL;
}

void* CancelTalkingAlert(void)
{
    if (verbose) puts("STUB: CancelTalkingAlert called");
    return NULL;
}

void* ContinueSpeech(void)
{
    if (verbose) puts("STUB: ContinueSpeech called");
    return NULL;
}

void* CopyAvailableIntervals(void)
{
    if (verbose) puts("STUB: CopyAvailableIntervals called");
    return NULL;
}

void* CopyAvailablePhrases(void)
{
    if (verbose) puts("STUB: CopyAvailablePhrases called");
    return NULL;
}

void* CopyIdentifierStringForVoiceSpec(void)
{
    if (verbose) puts("STUB: CopyIdentifierStringForVoiceSpec called");
    return NULL;
}

void* CopyPhonemesFromText(void)
{
    if (verbose) puts("STUB: CopyPhonemesFromText called");
    return NULL;
}

void* CopySpeechProperty(void)
{
    if (verbose) puts("STUB: CopySpeechProperty called");
    return NULL;
}

void* CopySpeechSynthesisVoicesForMode(void)
{
    if (verbose) puts("STUB: CopySpeechSynthesisVoicesForMode called");
    return NULL;
}

void* CopyTimeAnnouncementsPreferences(void)
{
    if (verbose) puts("STUB: CopyTimeAnnouncementsPreferences called");
    return NULL;
}

void* CountVoices(void)
{
    if (verbose) puts("STUB: CountVoices called");
    return NULL;
}

void* CreateSpeechLanguageLocale(void)
{
    if (verbose) puts("STUB: CreateSpeechLanguageLocale called");
    return NULL;
}

void* DisposeSpeechChannel(void)
{
    if (verbose) puts("STUB: DisposeSpeechChannel called");
    return NULL;
}

void* GetIdentifierStringForPreferredVoiceInListWithLocale(void)
{
    if (verbose) puts("STUB: GetIdentifierStringForPreferredVoiceInListWithLocale called");
    return NULL;
}

void* GetIdentifierStringForPreferredVoiceWithLocale(void)
{
    if (verbose) puts("STUB: GetIdentifierStringForPreferredVoiceWithLocale called");
    return NULL;
}

void* GetIndVoice(void)
{
    if (verbose) puts("STUB: GetIndVoice called");
    return NULL;
}

void* GetSpeechInfo(void)
{
    if (verbose) puts("STUB: GetSpeechInfo called");
    return NULL;
}

void* GetSpeechPitch(void)
{
    if (verbose) puts("STUB: GetSpeechPitch called");
    return NULL;
}

void* GetSpeechRate(void)
{
    if (verbose) puts("STUB: GetSpeechRate called");
    return NULL;
}

void* GetVoiceDescription(void)
{
    if (verbose) puts("STUB: GetVoiceDescription called");
    return NULL;
}

void* GetVoiceInfo(void)
{
    if (verbose) puts("STUB: GetVoiceInfo called");
    return NULL;
}

void* MakeVoiceSpec(void)
{
    if (verbose) puts("STUB: MakeVoiceSpec called");
    return NULL;
}

void* MakeVoiceSpecForIdentifierString(void)
{
    if (verbose) puts("STUB: MakeVoiceSpecForIdentifierString called");
    return NULL;
}

void* NewSpeechChannel(void)
{
    if (verbose) puts("STUB: NewSpeechChannel called");
    return NULL;
}

void* PauseSpeechAt(void)
{
    if (verbose) puts("STUB: PauseSpeechAt called");
    return NULL;
}

void* RegisterSpokenNotificationForProcess(void)
{
    if (verbose) puts("STUB: RegisterSpokenNotificationForProcess called");
    return NULL;
}

void* RegisterTalkingAlertWithProperties(void)
{
    if (verbose) puts("STUB: RegisterTalkingAlertWithProperties called");
    return NULL;
}

void* SRSetSecurityFlags(void)
{
    if (verbose) puts("STUB: SRSetSecurityFlags called");
    return NULL;
}

void* SUAddAppPathToLoginStartupList(void)
{
    if (verbose) puts("STUB: SUAddAppPathToLoginStartupList called");
    return NULL;
}

void* SUCopyAXUIElementExceptionsDictionary(void)
{
    if (verbose) puts("STUB: SUCopyAXUIElementExceptionsDictionary called");
    return NULL;
}

void* SUCopyKeyNamesPropertyList(void)
{
    if (verbose) puts("STUB: SUCopyKeyNamesPropertyList called");
    return NULL;
}

void* SUCopyPhraseArrayFromDisk(void)
{
    if (verbose) puts("STUB: SUCopyPhraseArrayFromDisk called");
    return NULL;
}

void* SUIsMainBundleUsingEnglishLocalization(void)
{
    if (verbose) puts("STUB: SUIsMainBundleUsingEnglishLocalization called");
    return NULL;
}

void* SUIsProcessUsingEnglishLocalization(void)
{
    if (verbose) puts("STUB: SUIsProcessUsingEnglishLocalization called");
    return NULL;
}

void* SUPostKeyCodeWithModifiers(void)
{
    if (verbose) puts("STUB: SUPostKeyCodeWithModifiers called");
    return NULL;
}

void* SURemoveAppPathFromLoginStartupList(void)
{
    if (verbose) puts("STUB: SURemoveAppPathFromLoginStartupList called");
    return NULL;
}

void* SendMessageToSpeechSynthesisServer(void)
{
    if (verbose) puts("STUB: SendMessageToSpeechSynthesisServer called");
    return NULL;
}

void* SetSpeechInfo(void)
{
    if (verbose) puts("STUB: SetSpeechInfo called");
    return NULL;
}

void* SetSpeechPitch(void)
{
    if (verbose) puts("STUB: SetSpeechPitch called");
    return NULL;
}

void* SetSpeechProperty(void)
{
    if (verbose) puts("STUB: SetSpeechProperty called");
    return NULL;
}

void* SetSpeechRate(void)
{
    if (verbose) puts("STUB: SetSpeechRate called");
    return NULL;
}

void* SetTimeAnnouncementsPreferences(void)
{
    if (verbose) puts("STUB: SetTimeAnnouncementsPreferences called");
    return NULL;
}

void* SpeakBuffer(void)
{
    if (verbose) puts("STUB: SpeakBuffer called");
    return NULL;
}

void* SpeakCFString(void)
{
    if (verbose) puts("STUB: SpeakCFString called");
    return NULL;
}

void* SpeakString(void)
{
    if (verbose) puts("STUB: SpeakString called");
    return NULL;
}

void* SpeakText(void)
{
    if (verbose) puts("STUB: SpeakText called");
    return NULL;
}

void* SpeechBusy(void)
{
    if (verbose) puts("STUB: SpeechBusy called");
    return NULL;
}

void* SpeechBusySystemWide(void)
{
    if (verbose) puts("STUB: SpeechBusySystemWide called");
    return NULL;
}

void* SpeechDaemonEnvironment(void)
{
    if (verbose) puts("STUB: SpeechDaemonEnvironment called");
    return NULL;
}

void* SpeechDaemonSetLogging(void)
{
    if (verbose) puts("STUB: SpeechDaemonSetLogging called");
    return NULL;
}

void* SpeechDaemonShutdown(void)
{
    if (verbose) puts("STUB: SpeechDaemonShutdown called");
    return NULL;
}

void* SpeechManagerVersion(void)
{
    if (verbose) puts("STUB: SpeechManagerVersion called");
    return NULL;
}

void* SpeechSynthesisConsoleLogCFString(void)
{
    if (verbose) puts("STUB: SpeechSynthesisConsoleLogCFString called");
    return NULL;
}

void* SpeechSynthesisConsoleLogTextBuffer(void)
{
    if (verbose) puts("STUB: SpeechSynthesisConsoleLogTextBuffer called");
    return NULL;
}

void* SpeechSynthesisRegisterModuleURL(void)
{
    if (verbose) puts("STUB: SpeechSynthesisRegisterModuleURL called");
    return NULL;
}

void* SpeechSynthesisUnregisterModuleURL(void)
{
    if (verbose) puts("STUB: SpeechSynthesisUnregisterModuleURL called");
    return NULL;
}

void* SpokenNotificationsEnabled(void)
{
    if (verbose) puts("STUB: SpokenNotificationsEnabled called");
    return NULL;
}

void* SpokenNotificationsEnabledForProcess(void)
{
    if (verbose) puts("STUB: SpokenNotificationsEnabledForProcess called");
    return NULL;
}

void* StopSpeech(void)
{
    if (verbose) puts("STUB: StopSpeech called");
    return NULL;
}

void* StopSpeechAt(void)
{
    if (verbose) puts("STUB: StopSpeechAt called");
    return NULL;
}

void* TTSIsSpeakingSelection(void)
{
    if (verbose) puts("STUB: TTSIsSpeakingSelection called");
    return NULL;
}

void* TTSStartSpeakingSelection(void)
{
    if (verbose) puts("STUB: TTSStartSpeakingSelection called");
    return NULL;
}

void* TTSStopSpeakingSelection(void)
{
    if (verbose) puts("STUB: TTSStopSpeakingSelection called");
    return NULL;
}

void* TalkingAlertsEnabled(void)
{
    if (verbose) puts("STUB: TalkingAlertsEnabled called");
    return NULL;
}

void* TextToPhonemes(void)
{
    if (verbose) puts("STUB: TextToPhonemes called");
    return NULL;
}

void* UseDictionary(void)
{
    if (verbose) puts("STUB: UseDictionary called");
    return NULL;
}

void* UseSpeechDictionary(void)
{
    if (verbose) puts("STUB: UseSpeechDictionary called");
    return NULL;
}
