import Foundation

// Darling ships no on-device speech recognition models, so report unavailability.
public final class SpeechTranscriber: Sendable {
    public static var isAvailable: Bool { false }
    public static var supportedLocales: [Locale] { get async { [] } }
    public static var installedLocales: [Locale] { get async { [] } }
    public static func supportedLocale(equivalentTo locale: Locale) async -> Locale? { nil }

    private init() {}
}
