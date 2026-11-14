// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift Photos overlay (apple/swift 5.2.2, stdlib/public/Darwin/Photos/PHProjectChangeRequest.swift)
// needs from the Photos Clang module: a generic PHFetchResult<PHAsset> and the swift_private
// __removeAssets accessor on PHProjectChangeRequest taking an NSFastEnumeration collection.
// Build: swiftc -typecheck photos_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>

import Foundation
import Photos

// What the overlay's extension does, spelled out (the overlay itself needs the ObjectiveC overlay module).
extension PHProjectChangeRequest {
    func typedRemoveAssets<T>(_ assets: T) where T: Collection, T.Element == PHAsset {
        let array = Array(assets) as NSArray
        self.__removeAssets(array)
    }
    func typedRemoveAssets(_ assets: PHFetchResult<PHAsset>) {
        self.__removeAssets(assets)
    }
}

func use(_ result: PHFetchResult<PHAsset>, _ request: PHProjectChangeRequest, _ assets: [PHAsset]) {
    let n: Int = result.count
    let first: PHAsset? = result.firstObject
    let last: PHAsset? = result.lastObject
    let one: PHAsset = result.object(at: 0)
    let idx: Int = result.index(of: one)
    let has: Bool = result.contains(one)
    result.enumerateObjects { (asset: PHAsset, i: Int, stop: UnsafeMutablePointer<ObjCBool>) in
        stop.pointee = true
    }
    request.typedRemoveAssets(assets)
    request.typedRemoveAssets(result)
    _ = (n, first, last, idx, has)
}
