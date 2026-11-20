// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of the Metal Clang module and Swift API:
// device creation, queues, types, and basic texture descriptors.
// Build: swiftc -typecheck metal_import.swift -target x86_64-apple-macosx10.13 -sdk <SDK>

import Foundation
import Metal

func testMetal() {
    let origin = MTLOrigin(x: 0, y: 0, z: 0)
    let size = MTLSize(width: 64, height: 64, depth: 1)
    let pixelFormat: MTLPixelFormat = .rgba8Unorm
    let textureType: MTLTextureType = .type2D
    _ = (origin, size, pixelFormat, textureType)

    if let device = MTLCreateSystemDefaultDevice() {
        let queue = device.makeCommandQueue()
        _ = queue
    }
}
testMetal()
