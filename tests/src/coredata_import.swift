// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of what the Swift CoreData overlay (apple/swift 5.2.2, stdlib/public/Darwin/CoreData/NSManagedObjectContext.swift)
// needs from the CoreData Clang module: a generic NSFetchRequest<ResultType: NSFetchRequestResult>, the throwing
// fetch(_:) and count(for:) members of NSManagedObjectContext, and the NSAffected*ErrorKey constants.
// Build: swiftc -typecheck coredata_import.swift -target x86_64-apple-macosx26.0 -sdk <SDK>
// (needs the CoreData headers of the darling-cocotron branch in the SDK's CoreData.framework/Headers)

import CoreData
import Foundation

// The overlay's two extension methods, spelled out.
extension NSManagedObjectContext {
    func typedFetch<T>(_ request: NSFetchRequest<T>) throws -> [T] {
        return try fetch(unsafeDowncast(request, to: NSFetchRequest<NSFetchRequestResult>.self)) as! [T]
    }
    func typedCount<T>(for request: NSFetchRequest<T>) throws -> Int {
        return try count(for: unsafeDowncast(request, to: NSFetchRequest<NSFetchRequestResult>.self))
    }
}

func use(_ context: NSManagedObjectContext, _ request: NSFetchRequest<NSFetchRequestResult>,
         _ objects: NSFetchRequest<NSManagedObject>) throws {
    let count: Int = try context.count(for: request)
    let all: [Any] = try context.fetch(request)
    let typed: [NSManagedObject] = try context.typedFetch(objects)
    let typedCount: Int = try context.typedCount(for: objects)
    let type: NSFetchRequestResultType = .managedObjectResultType
    _ = (count, all, typed, typedCount, type, NSAffectedStoresErrorKey, NSAffectedObjectsErrorKey)
}

// NSManagedObject and NSManagedObjectID are valid result types.
let _: NSFetchRequest<NSManagedObjectID>? = nil
let _: NSFetchRequest<NSManagedObject>? = nil
