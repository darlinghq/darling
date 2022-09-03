project(ProtocolBuffer)

remove_sdk_framework(ProtocolBuffer
	PRIVATE
)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "274.24.9")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(ProtocolBuffer
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/ProtocolBuffer"
	PRIVATE
)

add_framework(ProtocolBuffer
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/ProtocolBuffer.m
        src/PBDataReader.m
        src/PBTextWriter.m
        src/PBMutableData.m
        src/PBDataWriter.m
        src/PBTextReader.m
        src/PBCodable.m
        src/PBRequest.m
        src/PBStreamWriter.m
        src/PBMessageStreamReader.m
        src/PBMessageStreamWriter.m
        src/PBStreamReader.m
        src/_PBProperty.m
        src/PBUnknownFields.m
        src/PBSessionRequester.m

    DEPENDENCIES
        system
        objc
        Foundation
)
