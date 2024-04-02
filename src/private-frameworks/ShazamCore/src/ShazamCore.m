/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <ShazamCore/ShazamCore.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


void* const SHAnalyticsActionAppleMusic = (void*)0;
void* const SHAnalyticsActionNotificationTap = (void*)0;
void* const SHAnalyticsEventMusicRecognitionAlert = (void*)0;
void* const SHAnalyticsEventMusicRecognitionEnabled = (void*)0;
void* const SHAnalyticsEventMusicRecognitionHistory = (void*)0;
void* const SHAnalyticsEventMusicRecognitionList = (void*)0;
void* const SHAnalyticsEventMusicRecognitionResult = (void*)0;
void* const SHAnalyticsEventMusicRecognitionResultAction = (void*)0;
void* const SHAnalyticsEventMusicRecognitionResultExpanded = (void*)0;
void* const SHAnalyticsPayloadActionKey = (void*)0;
void* const SHAnalyticsPayloadEnabledKey = (void*)0;
void* const SHAnalyticsPayloadErrorCodeKey = (void*)0;
void* const SHAnalyticsPayloadMatchKey = (void*)0;
void* const SHAnalyticsPayloadNumberShazamsKey = (void*)0;
void* const SHAnalyticsPayloadSecureAccountKey = (void*)0;
void* const SHAnalyticsPayloadSourceKey = (void*)0;
void* const SHCampaignTokenDefaultCampaignValue = (void*)0;
void* const SHCoreErrorDomain = (void*)0;
void* const SHDefaultWorkerTimeout = (void*)0;
void* const SHRecordingIntermissionDefaultValue = (void*)0;
void* const SHShazamKitExternalAudioRecordingEntitlement = (void*)0;
void* const SHShazamKitInternalAudioRecordingEntitlement = (void*)0;
void* const SHShazamKitInternalClientEntitlement = (void*)0;
void* const SHShazamKitTCCEntitlement = (void*)0;
void* const SHShazamKitTCCEntitlementMediaLibrary = (void*)0;
void* const SHShazamKitTCCEntitlementMicrophone = (void*)0;

void *sh_log_object(void) {
    if (verbose) puts("STUB: sh_log_object called");
    return NULL;
}

