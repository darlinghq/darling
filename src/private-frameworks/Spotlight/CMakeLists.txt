project(Spotlight)

remove_sdk_framework(Spotlight
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1191.2.6")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(Spotlight
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/Spotlight"
	PRIVATE
)

add_framework(Spotlight
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/Spotlight.m
        src/SPShortcuts.m
        src/PRSDictionaryResult.m
        src/SPLogging.m
        src/SPLoggingSearchSession.m
        src/SPDefaults.m
        src/SPAppDefaults.m
        src/SPSearchQueryContext.m
        src/SPClientSession.m
        src/SPKCalculatorQuery.m
        src/TestingFeedbackListener.m
        src/QueryController.m
        src/SPKCoreParsecInterface.m
        src/PRSPattern.m
        src/SPStatistics.m
        src/SPURLResult.m
        src/SPURLQuery.m
        src/SPKParsecPattern.m
        src/SPKParsecQuery.m
        src/SPWebHistoryResult.m
        src/SPWebHistoryQuery.m
        src/SPApplicationQueryResult.m
        src/SPApplicationQuery.m
        src/SPFolderQuery.m
        src/SPMetadataQuery.m
        src/SPQueryTask.m
        src/SPKDictionaryQuery.m
        src/SPCoreSpotlightResult.m
        src/SPCoreSpotlightQuery.m
        src/SPMetadataPattern.m
        src/SPGroupHeadingResult.m
        src/SPMetadataGroupHeadingResult.m
        src/SPParsecGroupHeadingResult.m
        src/PRSResult.m
        src/SPShortcutPattern.m
        src/PRSParsecResult.m
        src/PRSParsecSimpleResult.m
        src/PRSParsecGenericResult.m
        src/PRSParsecMapsResult.m
        src/_SPResultsArrays.m
        src/PRSCalculatorResult.m
        src/PRSCalculationResult.m
        src/PRSConversion.m
        src/PRSConversionResult.m
        src/PRSUnitConversionResult.m
        src/PRSCurrencyConversionResult.m
        src/SPKResponse.m
        src/PRSPurchaseHistoryManager.m
        src/SPKQuery.m
        src/SPMetadataResult.m
        src/SPFusedResult.m
        src/SPAlbumResult.m

    DEPENDENCIES
        system
        objc
        Foundation
)
