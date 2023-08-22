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


#include <ShazamKit/ShazamKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* const SHErrorDeviceToDeviceEncryptionKey = (void*)0;
extern void* const SHErrorDiscardedAudioDurationKey = (void*)0;
extern void* const SHErrorDomain = (void*)0;
extern void* const SHEventSignalIdentifierMusicRecognitionHistoryActivated = (void*)0;
extern void* const SHEventSignalIdentifierMusicRecognitionRecognitionActivated = (void*)0;
extern void* const SHMaximumSignatureDuration = (void*)0;
extern void* const SHMediaItemAlbumName = (void*)0;
extern void* const SHMediaItemAppleMusicID = (void*)0;
extern void* const SHMediaItemAppleMusicURL = (void*)0;
extern void* const SHMediaItemArtist = (void*)0;
extern void* const SHMediaItemArtworkURL = (void*)0;
extern void* const SHMediaItemAudioStartDate = (void*)0;
extern void* const SHMediaItemCreationDate = (void*)0;
extern void* const SHMediaItemExplicitContent = (void*)0;
extern void* const SHMediaItemFrequencySkew = (void*)0;
extern void* const SHMediaItemFrequencySkewRanges = (void*)0;
extern void* const SHMediaItemFrequencySkewRangesSwift = (void*)0;
extern void* const SHMediaItemGenres = (void*)0;
extern void* const SHMediaItemISRC = (void*)0;
extern void* const SHMediaItemLyricsSnippet = (void*)0;
extern void* const SHMediaItemMatchOffset = (void*)0;
extern void* const SHMediaItemPresentationSettingOpenResultInBrowserOnFailure = (void*)0;
extern void* const SHMediaItemQueryMatchOffsetRanges = (void*)0;
extern void* const SHMediaItemReferenceMatchOffsetRanges = (void*)0;
extern void* const SHMediaItemReleaseDate = (void*)0;
extern void* const SHMediaItemShazamCount = (void*)0;
extern void* const SHMediaItemShazamID = (void*)0;
extern void* const SHMediaItemSpeedSkew = (void*)0;
extern void* const SHMediaItemStaticLyricLines = (void*)0;
extern void* const SHMediaItemSubtitle = (void*)0;
extern void* const SHMediaItemSyncedLyricsSnippet = (void*)0;
extern void* const SHMediaItemTimeRanges = (void*)0;
extern void* const SHMediaItemTimeRangesSwift = (void*)0;
extern void* const SHMediaItemTitle = (void*)0;
extern void* const SHMediaItemVideoURL = (void*)0;
extern void* const SHMediaItemWebURL = (void*)0;
extern void* const SHMediaLibraryErrorDomain = (void*)0;
extern void* const SHMediaLibrarySyncStartConditionInitialFetch = (void*)0;
extern void* const SHMediaLibrarySyncStartConditionInitialUpload = (void*)0;
extern void* const SHMediaLibrarySyncStartConditionMigration = (void*)0;
extern void* const SHMediaLibrarySyncStartConditionRemotePush = (void*)0;
extern void* const SHMediaLibrarySyncStartConditionUserAction = (void*)0;
extern void* const SHNotificationKeySessionError = (void*)0;
extern void* const SHNotificationKeySessionMatch = (void*)0;
extern void* const SHNotificationKeySessionSignature = (void*)0;
extern void* const SHNotificationNameSessionResult = (void*)0;
extern void* const SHShazamKitServiceName = (void*)0;
extern void* const SHShazamKitUIServiceName = (void*)0;
extern void* const _ZTSNSt3__113basic_fstreamIcNS_11char_traitsIcEEEE = (void*)0;
extern void* const _ZTSNSt3__117bad_function_callE = (void*)0;

void *SHShazamKitClientInterface(void) {
    if (verbose) puts("STUB: SHShazamKitClientInterface called");
    return NULL;
}

void *SHShazamKitServiceInterface(void) {
    if (verbose) puts("STUB: SHShazamKitServiceInterface called");
    return NULL;
}

void *SHShazamKitUIClientInterface(void) {
    if (verbose) puts("STUB: SHShazamKitUIClientInterface called");
    return NULL;
}

void *SHShazamKitUIServiceInterface(void) {
    if (verbose) puts("STUB: SHShazamKitUIServiceInterface called");
    return NULL;
}

