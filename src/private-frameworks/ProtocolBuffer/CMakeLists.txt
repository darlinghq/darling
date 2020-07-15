project(ProtocolBuffer)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "274.24.9")

add_framework(ProtocolBuffer
    FAT
    CURRENT_VERSION
    PRIVATE
    VERSION "A"

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
