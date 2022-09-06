project(LoggingSupport)

remove_sdk_framework(LoggingSupport
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "906.220.1")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(LoggingSupport
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/LoggingSupport"
	PRIVATE
)

add_framework(LoggingSupport
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/LoggingSupport.m
        src/LoggingSupport_OSLogCoder.m
        src/OSLogEventProxy.m
        src/OSLogEventDecomposedMessage.m
        src/OSLogMessagePlaceholder.m
        src/OSLogEventMessageArgument.m
        src/OSLogEventBacktraceFrame.m
        src/OSLogEventBacktrace.m
        src/OSLogDevice.m
        src/OSActivityStream.m
        src/OSActivityCreateEvent.m
        src/OSActivityTraceMessageEvent.m
        src/OSActivityLogMessageEvent.m
        src/OSActivitySignpostEvent.m
        src/OSActivityUserActionEvent.m
        src/OSActivityTransitionEvent.m
        src/OSActivityStatedumpEvent.m
        src/OSActivityTimesyncEvent.m
        src/OSActivityLossEvent.m
        src/OSActivityEvent.m
        src/OSActivityEventMessage.m
        src/OSLogEventLiveSource.m
        src/_OSLogChunkStore.m
        src/_OSLogChunkMemory.m
        src/_OSLogChunkFile.m
        src/_OSLogChunkFileReference.m
        src/_OSLogIndexFile.m
        src/_OSLogEnumeratorOversizeChunk.m
        src/_OSLogChunkBuffer.m
        src/_OSLogEnumeratorCatalogSubchunk.m
        src/_OSLogEnumeratorCatalog.m
        src/_OSLogTracepointBuffer.m
        src/_OSLogIndexEnumerator.m
        src/_OSLogIndex.m
        src/_OSLogPredicateMapper.m
        src/_OSLogLegacyPredicateMapper.m
        src/OSLogPersistence.m
        src/OSLogEventLiveStream.m
        src/OSLogTermDumper.m
        src/OSLogEventStreamBase.m
        src/_OSLogCatalogFilter.m
        src/_OSLogDirectoryReference.m
        src/_OSLogCollectionReference.m
        src/OSLogEventLiveStore.m
        src/OSLogEventStream.m
        src/OSLogEventStreamPosition.m
        src/_OSLogVersioning.m
        src/_OSLogSimplePredicate.m
        src/_OSLogStreamFilter.m
        src/OSLogEventSource.m
        src/OSLogPreferencesManager.m
        src/OSLogPreferencesProcess.m
        src/OSLogPreferencesSubsystem.m
        src/OSLogPreferencesCategory.m
        src/_OSLogEventStoreTimeRef.m
        src/_OSLogEventStoreMetadata.m
        src/OSLogEventStore.m
        src/OSLogEventLocalStore.m

    DEPENDENCIES
        system
        objc
        Foundation
)
