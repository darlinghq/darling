import Observation

// On-device Apple Intelligence models have no Linux backend, so every model instance
// reports .unavailable(.deviceNotEligible).
public final class SystemLanguageModel: Observable, Sendable {
    public struct UseCase: Equatable, Hashable, Sendable {
        private let kind: UInt8

        private init(kind: UInt8) {
            self.kind = kind
        }

        public static let general = UseCase(kind: 0)
        public static let contentTagging = UseCase(kind: 1)
    }

    public struct Guardrails: Equatable, Hashable, Sendable {
        private let kind: UInt8

        private init(kind: UInt8) {
            self.kind = kind
        }

        public static let `default` = Guardrails(kind: 0)
        public static let permissiveContentTransformations = Guardrails(kind: 1)
    }

    @frozen public enum Availability: Equatable, Hashable, Sendable {
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
}
