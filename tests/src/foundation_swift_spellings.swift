// Copyright (C) 2026 Lubos Dolezel
//
// Swift spellings of a few Foundation APIs, written the way applications use them:
// FileManager.urls(for:in:) returning [URL], UserDefaults.set(_:forKey:) for each value type,
// NSCoder.encodeCInt(_:forKey:) / decodeCInt(forKey:) for C int, IndexPath bridged to NSIndexPath, and the
// settable requiresSecureCoding of NSKeyedArchiver. Typecheck against the SDK with the Foundation overlay
// module; running it needs the Foundation framework.
import Foundation

var failures = 0
func check(_ ok: Bool, _ what: String) {
    if !ok {
        print("FAIL: \(what)")
        failures += 1
    }
}

let urls: [URL] = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)
check(urls.count <= 1, "one documents directory in the user domain at most")

let suite = "org.darlinghq.test.foundation-swift-spellings"
let defaults = UserDefaults(suiteName: suite)!
defaults.set(7, forKey: "int")
defaults.set(Float(1.5), forKey: "float")
defaults.set(2.25, forKey: "double")
defaults.set(true, forKey: "bool")
defaults.set(URL(fileURLWithPath: "/tmp/x"), forKey: "url")
defaults.set("text" as Any, forKey: "object")
check(defaults.integer(forKey: "int") == 7, "set(Int)")
check(defaults.float(forKey: "float") == 1.5, "set(Float)")
check(defaults.double(forKey: "double") == 2.25, "set(Double)")
check(defaults.bool(forKey: "bool"), "set(Bool)")
check(defaults.url(forKey: "url")?.path == "/tmp/x", "set(URL)")
check(defaults.string(forKey: "object") == "text", "set(Any)")
defaults.removePersistentDomain(forName: suite)

let archiver = NSKeyedArchiver(requiringSecureCoding: false)
archiver.requiresSecureCoding = true
check(archiver.requiresSecureCoding, "NSKeyedArchiver.requiresSecureCoding is settable")
archiver.requiresSecureCoding = false
archiver.encodeCInt(Int32(77), forKey: "c")
archiver.encode(Int32(78), forKey: "i32")
archiver.finishEncoding()
let unarchiver = try NSKeyedUnarchiver(forReadingFrom: archiver.encodedData)
check(unarchiver.decodeCInt(forKey: "c") == 77, "decodeCInt")
check(unarchiver.decodeInt32(forKey: "i32") == 78, "decodeInt32")
unarchiver.finishDecoding()

let path = IndexPath(indexes: [1, 2, 3])
let nsPath: NSIndexPath = path as NSIndexPath
check(path.count == 3 && nsPath.index(atPosition: 1) == 2, "IndexPath as NSIndexPath")
let back: IndexPath = nsPath as IndexPath
check(back == path, "NSIndexPath as IndexPath")

if failures == 0 { print("PASS") }
exit(failures == 0 ? 0 : 1)
