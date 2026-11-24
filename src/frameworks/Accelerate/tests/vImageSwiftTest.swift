// Copyright (C) 2026 Lubos Dolezel
//
// A program built on a Mac (swiftc -target x86_64-apple-macosx10.15 vImageSwiftTest.swift) that uses the vImage part of the Accelerate
// Swift overlay starts and runs in the guest with no DYLD variable: dyld binds the overlay's imports (vImageBuffer_Init,
// vImageBuffer_GetSize, vImageCGImageFormat_GetComponentCount, vImageConverter_CreateWithCGImageFormat, vImageConvert_AnyToAny,
// vImageConverter_MustOperateOutOfPlace, vImageCreateCGImageFromBuffer, ...) to Accelerate by library ordinal. Prints PASS.
import Accelerate
import CoreGraphics
import Foundation

var failures = 0
func expect(_ condition: Bool, _ what: String) { if !condition { print("FAIL \(what)"); failures += 1 } }

do {
    let srgb = CGColorSpaceCreateDeviceRGB()
    guard let premult = vImage_CGImageFormat(bitsPerComponent: 8, bitsPerPixel: 32, colorSpace: srgb,
                                             bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue)),
          let straight = vImage_CGImageFormat(bitsPerComponent: 8, bitsPerPixel: 32, colorSpace: srgb,
                                              bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)) else {
        print("FAIL formats"); exit(1)
    }
    expect(premult.componentCount == 4, "componentCount of RGBA is 4, got \(premult.componentCount)")

    var source = try vImage_Buffer(width: 4, height: 2, bitsPerPixel: 32)
    var dest = try vImage_Buffer(width: 4, height: 2, bitsPerPixel: 32)
    defer { source.free(); dest.free() }
    expect(source.width == 4 && source.height == 2 && source.rowBytes == 16, "Init: \(source.width)x\(source.height) rowBytes \(source.rowBytes)")
    expect(source.size == CGSize(width: 4, height: 2), "size")

    // 0x40 in every byte: straight RGBA to premultiplied scales the colour by 0x40/255
    memset(source.data, 0x40, 2 * source.rowBytes)
    let converter = try vImageConverter.make(sourceFormat: straight, destinationFormat: premult)
    expect(converter.sourceBufferCount == 1 && converter.destinationBufferCount == 1, "buffer counts")
    expect((try? converter.mustOperateOutOfPlace(source: source, destination: dest)) == false, "RGBA8 to RGBA8P works in place")
    try converter.convert(source: source, destination: &dest)
    let bytes = dest.data.assumingMemoryBound(to: UInt8.self)
    expect(bytes[0] == 0x10 && bytes[1] == 0x10 && bytes[2] == 0x10 && bytes[3] == 0x40, "converted pixel \(bytes[0]) \(bytes[1]) \(bytes[2]) \(bytes[3])")

    // an image made from a buffer reads back
    let image = try source.createCGImage(format: straight)
    expect(image.width == 4 && image.height == 2, "CGImage size")
    var back = try vImage_Buffer(cgImage: image, format: straight)
    defer { back.free() }
    expect(back.width == 4 && back.height == 2, "buffer from the CGImage")
    expect(back.data.assumingMemoryBound(to: UInt8.self)[0] == 0x40, "pixel of the buffer from the CGImage")
} catch {
    print("FAIL error \(error)")
    failures += 1
}
print(failures == 0 ? "PASS" : "FAIL (\(failures))")
exit(failures == 0 ? 0 : 1)
