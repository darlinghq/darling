project(OpenDirectory)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

add_framework(OpenDirectory
    FAT
    CURRENT_VERSION
    VERSION "A"

    SOURCES
        src/OpenDirectory.m
        src/NSODSession.m
        src/ODSession.m
        src/NSODNode.m
        src/ODNode.m
        src/NSODRecord.m
        src/ODRecord.m
        src/NSODQuery.m
        src/ODQuery.m
        src/NSODContext.m
        src/ODContext.m
        src/ODConfiguration.m
        src/ODModuleEntry.m
        src/ODMappings.m
        src/ODAttributeMap.m
        src/ODRecordMap.m

    DEPENDENCIES
        system
        objc
        Foundation
)

reexport(OpenDirectory CFOpenDirectory ${CMAKE_BINARY_DIR}/src/frameworks/CFOpenDirectory/CFOpenDirectory)
