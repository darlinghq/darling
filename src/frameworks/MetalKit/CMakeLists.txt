project(MetalKit)

set(DYLIB_COMPAT_VERSION "1.0.0")
set(DYLIB_CURRENT_VERSION "113.0.0")

add_framework(MetalKit
    FAT
    CURRENT_VERSION
    VERSION "A"

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
        system
        objc
        Foundation
)
