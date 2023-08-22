project(ShazamKit)

remove_sdk_framework(ShazamKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ShazamKit
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/ShazamKit"
)


add_framework(ShazamKit
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SHCustomCatalog.m
		src/SHAudioConverter.m
		src/SHSessionDriverSignatureSlot.m
		src/SHStreamingSessionDriver.m
		src/SHMatch.m
		src/SHShazamKey.m
		src/SHCustomCatalogMatcherResultBuilder.m
		src/SHCustomCatalogMemoryStorage.m
		src/SHMediaLibraryTrackBuilder.m
		src/SHMediaLibrary.m
		src/SHMediaLibraryTrack.m
		src/SigCrop.m
		src/SHContinuityTracker.m
		src/SHCustomCatalogMatcher.m
		src/SHReferenceSignature.m
		src/SHMutableSignature.m
		src/MRE.m
		src/SignatureAlignmentTracker.m
		src/SHEventSignaller.m
		src/SHManagedSessionDriverDaemonDelegate.m
		src/SHAudioUtilities.m
		src/SHMediaItem.m
		src/SHShazamKitUIServiceInterfaces.m
		src/SHCatalog.m
		src/SHSyncedLyrics.m
		src/SHAttribution.m
		src/SHSignatureDataRepresentationValidator.m
		src/SHMediaItemPresentationSettings.m
		src/SHLyricLine.m
		src/SHMediaLibraryTrackUpdate.m
		src/SHShazamKitServiceConnection.m
		src/SigX.m
		src/SHMatchedMediaItem.m
		src/SHMediaItemPropertyUtilities.m
		src/SHSignatureBuffer.m
		src/SHError.m
		src/SHShazamKitServiceConnectionClient.m
		src/SHMediaTimelineCustomCatalogMatcher.m
		src/SHShazamKitServiceInterfaces.m
		src/SigSonify.m
		src/SHMediaLibraryItemMetadata.m
		src/SHCustomCatalogTracker.m
		src/SHMediaItemSerialization.m
		src/SHMediaLibraryGroup.m
		src/SHManagedSessionDriver.m
		src/SHCustomCatalogContainer.m
		src/SHSignature.m
		src/SHManagedSession.m
		src/SHJSONLCustomCatalogTransformer.m
		src/SHMediaTimeline.m
		src/SHTrackMO.m
		src/SHMediaLibraryLabel.m
		src/SHSignatureChunker.m
		src/SHRotatingInstallationID.m
		src/SHSignatureGenerator.m
		src/SHSignatureMetrics.m
		src/SHMediaLibraryRequest.m
		src/SHMediaLibrarySnapshot.m
		src/SHSession.m
		src/SHRange.m
		src/SignatureAlignmentTrackerResult.m
		src/SHGroupMO.m
		src/SHCatalogConfiguration.m
		src/SHContinuityTrackerResult.m
		src/SHShazamKitServiceConnectionProvider.m
		src/MRESignature.m
		src/SHMetadataMO.m
		src/SHCustomCatalogConfiguration.m
		src/SHMatcherResponse.m
		src/SHMediaLibraryInfo.m
		src/SHShazamCatalog.m
		src/MREResult.m
		src/SHMediaLibraryChange.m
		src/SHMatcherRequest.m
		src/SHMediaLibraryQueryParameters.m
		src/UTType+SHShazamAdditions.m
		src/NSError+SHError.m
		src/ShazamKit.m

	DEPENDENCIES
		system
		objc
		Foundation
)
