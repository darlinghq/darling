project(ShazamInsights)

remove_sdk_framework(ShazamInsights
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "195.12.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ShazamInsights
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/ShazamInsights"
)


add_framework(ShazamInsights
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SHSQLiteUtils.m
		src/SHCDNDataFetcher.m
		src/SHInsightsNetworkRequester.m
		src/SHDataFetcherFileInfo.m
		src/SHLocalDataFetcher.m
		src/SHClusterController.m
		src/SHClusterQuery.m
		src/SHInsightsConfiguration.m
		src/SHClusterLoader.m
		src/SHStubbedNetworkRequester.m
		src/SHRegion.m
		src/SHInsightsError.m
		src/SHTimeAndPlaceController.m
		src/SHClusterJSONLReader.m
		src/SHClusterMetadataQuery.m
		src/SHMediaItemPropertyCollection.m
		src/SHTimeAndPlaceAffinityGroup.m
		src/SHAffinityGroup.m
		src/SHClusterImporterUtils.m
		src/SHTimeAndPlaceServerResponseParser.m
		src/SHAffinityGroupQuery.m
		src/SHJSONLClusterImporter.m
		src/SHClusterStatus.m
		src/SHSQLiteClusterImporter.m
		src/SHClusterSQLiteDataStore.m
		src/SHClusterStatementRunner.m
		src/SHFileChecksum.m
		src/SHClusterMetadata.m
		src/SHClusterLoaderRequest.m
		src/CLLocation+Geohash.m
		src/ShazamInsights.m

	DEPENDENCIES
		system
		objc
		Foundation
)