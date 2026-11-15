// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift Vision overlay (apple/swift 5.2.2, stdlib/public/Darwin/Vision/Vision.swift)
// needs from the Vision Clang module: VNFaceLandmarkRegion2D.pointCount (from VNFaceLandmarkRegion) and the
// swift_private __normalizedPoints and __pointsInImage(imageSize:) accessors.
// Build: swiftc -typecheck vision_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import CoreGraphics
import Foundation
import Vision

// The overlay's extension, spelled out.
extension VNFaceLandmarkRegion2D {
    var pointsFromOverlay: [CGPoint] {
        let buffer = UnsafeBufferPointer<CGPoint>(start: self.__normalizedPoints, count: Int(self.pointCount))
        return Array(buffer)
    }
    func pointsInImageFromOverlay(imageSize: CGSize) -> [CGPoint] {
        let buffer = UnsafeBufferPointer<CGPoint>(start: self.__pointsInImage(imageSize: imageSize), count: Int(self.pointCount))
        return Array(buffer)
    }
}

func use(_ region: VNFaceLandmarkRegion2D) {
    let n: Int = Int(region.pointCount)
    let base: VNFaceLandmarkRegion = region
    _ = (n, base.pointCount, region.pointsFromOverlay, region.pointsInImageFromOverlay(imageSize: CGSize(width: 10, height: 10)))
}
