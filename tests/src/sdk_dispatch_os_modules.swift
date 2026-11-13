// Copyright (C) 2026 Lubos Dolezel
import Dispatch
import os.log

// The Dispatch and os Clang modules import with the Swift names given by their API notes.
let queue: DispatchQueue = DispatchQueue(__label: "sdk-test", attr: nil)
let log: OSLog = OSLog(__subsystem: "sdk.test", category: "modules")
let type: OSLogType = OSLogType(rawValue: 0)
if log.isEnabled(type: type) || !log.isEnabled(type: type) {
    print("ALL PASSED")
}
_ = queue
