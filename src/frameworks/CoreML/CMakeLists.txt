project(CoreML)

remove_sdk_framework(CoreML)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "1.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(CoreML
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/CoreML"
)

add_framework(CoreML
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MLModel.m
        src/MLModelConfiguration.m
        src/MLMultiArray.m
        src/MLFeatureValue.m
        src/MLPredictionOptions.m
        src/MLComputePlan.m
        src/MLCPUComputeDevice.m
        src/MLGPUComputeDevice.m
        src/MLNeuralEngineComputeDevice.m
        src/CoreML.m

    DEPENDENCIES
        system
        objc
        Foundation
)
