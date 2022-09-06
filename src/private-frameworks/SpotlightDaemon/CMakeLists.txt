project(SpotlightDaemon)

remove_sdk_framework(SpotlightDaemon
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(SpotlightDaemon
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/SpotlightDaemon"
	PRIVATE
)

add_framework(SpotlightDaemon
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/SpotlightDaemon.m
        src/SPQueryJob.m
        src/SPQueryResultsQueue.m
        src/SPCoreSpotlightTask.m
        src/MDAgent.m
        src/SPCoreSpotlightIndexerTask.m
        src/_SPBucketSet.m
        src/SPConcreteCoreSpotlightIndexer.m
        src/SPCoreSpotlightIndexer.m
        src/SpotlightReceiverConnection.m
        src/SpotlightSender.m
        src/SDTraceItem.m
        src/SDTrace.m
        src/SDEventMonitor.m
        src/SDMigrationMonitor.m
        src/CSSearchClientConnectionKey.m
        src/CSSearchClientConnection.m
        src/CSSearchAgent.m
        src/CSIndexClientConnectionKey.m
        src/CSIndexClientConnection.m
        src/CSIndexAgent.m
        src/SPCoreSpotlightExtensionsTask.m
        src/CSRecieverState.m
        src/MDSearchableIndexService.m

    DEPENDENCIES
        system
        objc
        Foundation
)
