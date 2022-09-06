project(DiscRecording)

remove_sdk_framework(DiscRecording)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(DiscRecording
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/DiscRecording"
)

add_framework(DiscRecording
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/DiscRecording.m
        src/DRBurn.m
        src/DRDevice.m
        src/DRErase.m
        src/DRNotificationCenter.m
        src/DRTask.m
        src/DRCDTextBlock.m
        src/DRMSF.m
        src/DRMSFFormatter.m
        src/DRTrack.m
        src/DRPrepare.m
        src/DRSession.m
        src/DRSessionCDTextProducer.m
        src/DRSessionProducerPlaceholder.m
        src/DRCallbackDevice.m
        src/DRAudioTrack.m
        src/DRFilesystemTrack.m
        src/DRFile.m
        src/DRFolder.m
        src/DRFSObject.m
        src/DRBINFileProducer.m
        src/DRSilenceGenerator.m
        src/DRDataGenerator.m
        src/DRAudioGenerator.m
        src/DRCueFileParser.m
        src/DRPartialFileAudioProducer.m
        src/DRTOCFileParser.m
        src/DRTOCProducer.m
        src/DRDeviceFilter.m
        src/DRDefaultDeviceFilter.m

    DEPENDENCIES
        system
        objc
        Foundation
)
