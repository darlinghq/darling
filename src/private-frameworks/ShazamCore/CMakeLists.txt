project(ShazamCore)

remove_sdk_framework(ShazamCore
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "0.0.0")
set(DYLIB_CURRENT_VERSION "207.9.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ShazamCore
	PRIVATE
	VERSION ${FRAMEWORK_VERSION}
	HEADER "include/ShazamCore"
)


add_framework(ShazamCore
	PRIVATE
	FAT
	CURRENT_VERSION
	VERSION ${FRAMEWORK_VERSION}

	SOURCES
		src/SHJSONLDataDetokenizer.m
		src/SHCampaignTokens.m
		src/SHJSONLFileReader.m
		src/SHCompression.m
		src/SHOffersNetworkRequester.m
		src/SHDefaultConfigurationValues.m
		src/SHRemoteConfiguration.m
		src/SHDataStreamOutput.m
		src/SHJSONLFileWriter.m
		src/SHHost.m
		src/SHTokenizedURL.m
		src/SHLocalization.m
		src/SHEndpoints.m
		src/SHAnalytics.m
		src/SHMusicSubscriptionStatus.m
		src/SHEntitlements.m
		src/SHFeatureFlags.m
		src/SHDataStreamInput.m
		src/SHCoreError.m
		src/SHOffers.m
		src/SHDataCompression.m
		src/SHBagContract.m
		src/SHStorefront.m
		src/NSData+Compression.m
		src/NSError+SHCoreError.m
		src/ShazamCore.m

	DEPENDENCIES
		system
		objc
		Foundation
)
