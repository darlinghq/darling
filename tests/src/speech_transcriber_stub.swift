// Copyright (C) 2026 Lubos Dolezel
//
// SpeechTranscriber on a machine without on-device speech models: isAvailable is false and both locale lists are empty.
// Build (x86_64): swiftc -frontend -c -parse-as-library with the Darling Swift overlay modules, then link Speech.framework.
// 64-bit only: the Swift runtime has no i386 slice.

import Darwin
import Foundation
import Speech

// Contract checks for SpeechTranscriber on a machine without on-device speech models.
// Only API that the macOS 26 SDK declares is used, so this also builds and runs against the real framework.

private func fail(_ message: String) -> Never {
    fputs("FAIL: \(message)\n", stderr)
    exit(1)
}

private func requireSendable<T: Sendable>(_: T.Type) {}

@main
struct SpeechTranscriberStubTest {
    static func main() async {
        requireSendable(SpeechTranscriber.self)

        if SpeechTranscriber.isAvailable { fail("isAvailable should be false") }

        let supported = await SpeechTranscriber.supportedLocales
        if !supported.isEmpty { fail("supportedLocales should be empty, got \(supported)") }

        let installed = await SpeechTranscriber.installedLocales
        if !installed.isEmpty { fail("installedLocales should be empty, got \(installed)") }

        // A locale that no speech module supports has no equivalent.
        let none = await SpeechTranscriber.supportedLocale(equivalentTo: Locale(identifier: "xx_YY"))
        if none != nil { fail("unknown locale should have no equivalent, got \(String(describing: none))") }

        print("PASS")
    }
}
