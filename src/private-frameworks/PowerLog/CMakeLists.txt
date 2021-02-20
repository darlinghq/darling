project(PowerLog)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(PowerLog
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

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
