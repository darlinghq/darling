import Darwin
import Foundation
import Speech

private func requireSendable<T: Sendable>(_: T.Type) {}

private func runChecks() async {
    requireSendable(SpeechTranscriber.self)

    guard !SpeechTranscriber.isAvailable else {
        fputs("FAIL: expected SpeechTranscriber.isAvailable == false\n", stderr)
        exit(1)
    }

    let supported = await SpeechTranscriber.supportedLocales
    guard supported.isEmpty else {
        fputs("FAIL: expected empty supportedLocales, got \(supported)\n", stderr)
        exit(1)
    }

    let installed = await SpeechTranscriber.installedLocales
    guard installed.isEmpty else {
        fputs("FAIL: expected empty installedLocales, got \(installed)\n", stderr)
        exit(1)
    }

    let matched = await SpeechTranscriber.supportedLocale(equivalentTo: Locale(identifier: "en_US"))
    guard matched == nil else {
        fputs("FAIL: expected nil supportedLocale(equivalentTo:), got \(String(describing: matched))\n", stderr)
        exit(1)
    }

    print("PASS")
}

@main
struct SpeechTranscriberStubTest {
    static func main() async {
        await runChecks()
    }
}
