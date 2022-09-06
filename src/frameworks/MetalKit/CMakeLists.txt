project(MetalKit)

remove_sdk_framework(MetalKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "113.0.0")

set(FRAMEWORK_VERSION "A")

generate_sdk_framework(MetalKit
    VERSION ${FRAMEWORK_VERSION}
    HEADER "include/MetalKit"
)

add_framework(MetalKit
    FAT
    CURRENT_VERSION
    VERSION ${FRAMEWORK_VERSION}

    SOURCES
        src/MetalKit.m
        src/MTKTextureLoaderPVR3.m
        src/MTKTextureUploader.m
        src/MTKTextureLoaderMDL.m
        src/MTKTextureIOBufferMap.m
        src/MTKTextureIOBuffer.m
        src/MTKTextureIOBufferAllocator.m
        src/MTKTextureLoader.m
        src/MTKTextureLoaderData.m
        src/MTKTextureLoaderImageIO.m
        src/MTKTextureLoaderKTX.m
        src/MTKTextureLoaderPVR.m
        src/MTKOffscreenDrawable.m
        src/MTKMeshBufferAllocator.m
        src/MTKSubmesh.m
        src/MTKMesh.m
        src/MTKMeshBufferHolder.m
        src/MTKMeshBufferZone.m
        src/MTKMeshBuffer.m
        src/MTKView.m

    DEPENDENCIES
        AppKit
        system
        objc
        Foundation
)
