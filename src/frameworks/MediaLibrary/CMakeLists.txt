project(MediaLibrary)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "764.0.0")
set(FRAMEWORK_VERSION "A")

remove_sdk_framework(MediaLibrary
)

generate_sdk_framework(MediaLibrary
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MediaLibrary"
)

add_framework(MediaLibrary
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MediaLibrary.m
        src/MLMediaLibrary.m
        src/MLMediaLibraryImpl.m
        src/MLMediaGroup.m
        src/MLMediaGroupImpl.m
        src/MLMediaObject.m
        src/MLMediaObjectImpl.m
        src/MLMediaSource.m
        src/MLMediaSourceImpl.m
        src/MLPMR.m
        src/MLPMRContextImpl.m
        src/MLPMRContext.m

    DEPENDENCIES
        system
        objc
        Foundation
)
