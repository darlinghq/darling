project(PowerLog)

remove_sdk_framework(PowerLog
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(PowerLog
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/PowerLog"
	PRIVATE
)

add_framework(PowerLog
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/PowerLog.m
        src/PLCoreDuetEventInterval.m
        src/PLCoreDuetEvent.m
        src/PLEnergyBucket.m
        src/PLUsageSnapshot.m
        src/PLDuetActivitySchedulerUsageSnapshot.m
        src/PLNetworkEnergyModel.m
        src/PLModelingUtilities.m
        src/PLClientLogAggregator.m
        src/PLClientLogger.m

    DEPENDENCIES
        system
        objc
        Foundation
)
