// Copyright (C) 2026 Lubos Dolezel
//
// Typecheck of basic SwiftUI API: View protocol conformance, Text, and Body.
// Build: swiftc -typecheck swiftui_import.swift -target x86_64-apple-macos10.15 -sdk <SDK>

import Foundation
import SwiftUI

struct MyTestView: View {
    var body: some View {
        Text("Hello, Darling!")
    }
}

func testSwiftUI() {
    let view = MyTestView()
    _ = view.body
}
testSwiftUI()
