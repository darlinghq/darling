import Foundation
import Observation

// On-device Apple Intelligence models have no Linux backend, so every model instance
// reports .unavailable(.deviceNotEligible), as macOS does on hardware that cannot run the model.
public final class SystemLanguageModel: Observable, Sendable {
    public struct UseCase: Equatable, Sendable {
        private let kind: UInt8

        private init(kind: UInt8) {
            self.kind = kind
        }

        public static let general = UseCase(kind: 0)
        public static let contentTagging = UseCase(kind: 1)
    }

    public struct Guardrails: Sendable {
        private let kind: UInt8

        private init(kind: UInt8) {
            self.kind = kind
        }

        public static let `default` = Guardrails(kind: 0)
        public static let permissiveContentTransformations = Guardrails(kind: 1)
    }

    @frozen public enum Availability: Equatable, Sendable {
        public enum UnavailableReason: Equatable, Hashable, Sendable {
            case deviceNotEligible
            case appleIntelligenceNotEnabled
            case modelNotReady
        }

        case available
        case unavailable(UnavailableReason)
    }

    public static let `default` = SystemLanguageModel()

    public init(useCase: UseCase = .general, guardrails: Guardrails = .default) {}

    public var availability: Availability {
        .unavailable(.deviceNotEligible)
    }

    public var isAvailable: Bool {
        false
    }

    // The set of languages the model supports is a property of the model, not of the hardware:
    // macOS reports it on machines where the model is unavailable.
    public var supportedLanguages: Set<Locale.Language> {
        Set(Self.supportedLanguageIdentifiers.map { Locale.Language(identifier: $0) })
    }

    private static let supportedLanguageIdentifiers = [
        "da-DK", "de-DE", "en-AU", "en-GB", "en-US", "es-419", "es-ES", "es-US",
        "fr-CA", "fr-FR", "it-IT", "ja-JP", "ko-KR", "nb-NO", "nl-NL", "pt-BR",
        "pt-PT", "sv-SE", "tr-TR", "vi-VN", "zh-CN", "zh-HK", "zh-TW",
    ]
}
