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

#ifndef _SHAZAMKIT_H_
#define _SHAZAMKIT_H_

#import <Foundation/Foundation.h>

#import <ShazamKit/MRE.h>
#import <ShazamKit/MREResult.h>
#import <ShazamKit/MRESignature.h>
#import <ShazamKit/NSError+SHError.h>
#import <ShazamKit/SHAttribution.h>
#import <ShazamKit/SHAudioConverter.h>
#import <ShazamKit/SHAudioUtilities.h>
#import <ShazamKit/SHCatalog.h>
#import <ShazamKit/SHCatalogConfiguration.h>
#import <ShazamKit/SHContinuityTracker.h>
#import <ShazamKit/SHContinuityTrackerResult.h>
#import <ShazamKit/SHCustomCatalog.h>
#import <ShazamKit/SHCustomCatalogConfiguration.h>
#import <ShazamKit/SHCustomCatalogContainer.h>
#import <ShazamKit/SHCustomCatalogMatcher.h>
#import <ShazamKit/SHCustomCatalogMatcherResultBuilder.h>
#import <ShazamKit/SHCustomCatalogMemoryStorage.h>
#import <ShazamKit/SHCustomCatalogStorage.h>
#import <ShazamKit/SHCustomCatalogTracker.h>
#import <ShazamKit/SHError.h>
#import <ShazamKit/SHEventSignaller.h>
#import <ShazamKit/SHEventSignalling.h>
#import <ShazamKit/SHGroupMO.h>
#import <ShazamKit/SHJSONLCustomCatalogTransformer.h>
#import <ShazamKit/SHJSONLCustomCatalogTransformerDelegate.h>
#import <ShazamKit/SHJSONLDataDetokenizerDelegate.h>
#import <ShazamKit/SHLyricLine.h>
#import <ShazamKit/SHManagedSession.h>
#import <ShazamKit/SHManagedSessionDriver.h>
#import <ShazamKit/SHManagedSessionDriverDaemonDelegate.h>
#import <ShazamKit/SHMatch.h>
#import <ShazamKit/SHMatchedMediaItem.h>
#import <ShazamKit/SHMatcher.h>
#import <ShazamKit/SHMatcherDelegate.h>
#import <ShazamKit/SHMatcherRequest.h>
#import <ShazamKit/SHMatcherResponse.h>
#import <ShazamKit/SHMediaItem.h>
#import <ShazamKit/SHMediaItemPresentation.h>
#import <ShazamKit/SHMediaItemPresentationSettings.h>
#import <ShazamKit/SHMediaItemPropertyUtilities.h>
#import <ShazamKit/SHMediaItemProvider.h>
#import <ShazamKit/SHMediaItemSerialization.h>
#import <ShazamKit/SHMediaLibrary.h>
#import <ShazamKit/SHMediaLibrary.h>
#import <ShazamKit/SHMediaLibraryChange.h>
#import <ShazamKit/SHMediaLibraryGroup.h>
#import <ShazamKit/SHMediaLibraryInfo.h>
#import <ShazamKit/SHMediaLibraryItem.h>
#import <ShazamKit/SHMediaLibraryItemMetadata.h>
#import <ShazamKit/SHMediaLibraryLabel.h>
#import <ShazamKit/SHMediaLibraryQuery.h>
#import <ShazamKit/SHMediaLibraryQueryParameters.h>
#import <ShazamKit/SHMediaLibraryRequest.h>
#import <ShazamKit/SHMediaLibrarySnapshot.h>
#import <ShazamKit/SHMediaLibrarySync.h>
#import <ShazamKit/SHMediaLibrarySyncDelegate.h>
#import <ShazamKit/SHMediaLibraryTrack.h>
#import <ShazamKit/SHMediaLibraryTrackBuilder.h>
#import <ShazamKit/SHMediaLibraryTrackUpdate.h>
#import <ShazamKit/SHMediaTimeline.h>
#import <ShazamKit/SHMediaTimelineCustomCatalogMatcher.h>
#import <ShazamKit/SHMediaTimelineDelegate.h>
#import <ShazamKit/SHMetadataMO.h>
#import <ShazamKit/SHMutableSignature.h>
#import <ShazamKit/SHRange.h>
#import <ShazamKit/SHReferenceSignature.h>
#import <ShazamKit/SHRotatingInstallationID.h>
#import <ShazamKit/SHSession.h>
#import <ShazamKit/SHSessionDriver.h>
#import <ShazamKit/SHSessionDriverDelegate.h>
#import <ShazamKit/SHSessionDriverSignatureSlot.h>
#import <ShazamKit/SHShazamCatalog.h>
#import <ShazamKit/SHShazamKey.h>
#import <ShazamKit/SHShazamKitClient.h>
#import <ShazamKit/SHShazamKitService.h>
#import <ShazamKit/SHShazamKitServiceConnection.h>
#import <ShazamKit/SHShazamKitServiceConnectionClient.h>
#import <ShazamKit/SHShazamKitServiceConnectionProvider.h>
#import <ShazamKit/SHShazamKitServiceConnectionProvider.h>
#import <ShazamKit/SHShazamKitServiceInterfaces.h>
#import <ShazamKit/SHShazamKitUIService.h>
#import <ShazamKit/SHShazamKitUIServiceInterfaces.h>
#import <ShazamKit/SHSignature.h>
#import <ShazamKit/SHSignatureBuffer.h>
#import <ShazamKit/SHSignatureChunker.h>
#import <ShazamKit/SHSignatureDataRepresentationValidator.h>
#import <ShazamKit/SHSignatureGenerator.h>
#import <ShazamKit/SHSignatureMetrics.h>
#import <ShazamKit/SHStreamingSessionDriver.h>
#import <ShazamKit/SHSyncedLyrics.h>
#import <ShazamKit/SHTrackMO.h>
#import <ShazamKit/SigCrop.h>
#import <ShazamKit/SigSonify.h>
#import <ShazamKit/SigX.h>
#import <ShazamKit/SignatureAlignmentTracker.h>
#import <ShazamKit/SignatureAlignmentTrackerResult.h>
#import <ShazamKit/UTType+SHShazamAdditions.h>


void *SHShazamKitClientInterface(void);
void *SHShazamKitServiceInterface(void);
void *SHShazamKitUIClientInterface(void);
void *SHShazamKitUIServiceInterface(void);


extern void* const SHErrorDeviceToDeviceEncryptionKey;
extern void* const SHErrorDiscardedAudioDurationKey;
extern void* const SHErrorDomain;
extern void* const SHEventSignalIdentifierMusicRecognitionHistoryActivated;
extern void* const SHEventSignalIdentifierMusicRecognitionRecognitionActivated;
extern void* const SHMaximumSignatureDuration;
extern void* const SHMediaItemAlbumName;
extern void* const SHMediaItemAppleMusicID;
extern void* const SHMediaItemAppleMusicURL;
extern void* const SHMediaItemArtist;
extern void* const SHMediaItemArtworkURL;
extern void* const SHMediaItemAudioStartDate;
extern void* const SHMediaItemCreationDate;
extern void* const SHMediaItemExplicitContent;
extern void* const SHMediaItemFrequencySkew;
extern void* const SHMediaItemFrequencySkewRanges;
extern void* const SHMediaItemFrequencySkewRangesSwift;
extern void* const SHMediaItemGenres;
extern void* const SHMediaItemISRC;
extern void* const SHMediaItemLyricsSnippet;
extern void* const SHMediaItemMatchOffset;
extern void* const SHMediaItemPresentationSettingOpenResultInBrowserOnFailure;
extern void* const SHMediaItemQueryMatchOffsetRanges;
extern void* const SHMediaItemReferenceMatchOffsetRanges;
extern void* const SHMediaItemReleaseDate;
extern void* const SHMediaItemShazamCount;
extern void* const SHMediaItemShazamID;
extern void* const SHMediaItemSpeedSkew;
extern void* const SHMediaItemStaticLyricLines;
extern void* const SHMediaItemSubtitle;
extern void* const SHMediaItemSyncedLyricsSnippet;
extern void* const SHMediaItemTimeRanges;
extern void* const SHMediaItemTimeRangesSwift;
extern void* const SHMediaItemTitle;
extern void* const SHMediaItemVideoURL;
extern void* const SHMediaItemWebURL;
extern void* const SHMediaLibraryErrorDomain;
extern void* const SHMediaLibrarySyncStartConditionInitialFetch;
extern void* const SHMediaLibrarySyncStartConditionInitialUpload;
extern void* const SHMediaLibrarySyncStartConditionMigration;
extern void* const SHMediaLibrarySyncStartConditionRemotePush;
extern void* const SHMediaLibrarySyncStartConditionUserAction;
extern void* const SHNotificationKeySessionError;
extern void* const SHNotificationKeySessionMatch;
extern void* const SHNotificationKeySessionSignature;
extern void* const SHNotificationNameSessionResult;
extern void* const SHShazamKitServiceName;
extern void* const SHShazamKitUIServiceName;
extern void* const _ZTSNSt3__113basic_fstreamIcNS_11char_traitsIcEEEE;
extern void* const _ZTSNSt3__117bad_function_callE;

#endif

