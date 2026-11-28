import Darwin
import Foundation
import FoundationModels
import Observation

// Contract checks for the unavailable-model stub. Only API that the macOS 26 SDK declares is used,
// so this file also builds and passes against the real framework on a Mac without Apple Intelligence.

private func fail(_ message: String) -> Never {
    fputs("FAIL: \(message)\n", stderr)
    exit(1)
}

private func requireSendable<T: Sendable>(_: T.Type) {}
private func requireObservable<T: Observable>(_: T.Type) {}

requireSendable(SystemLanguageModel.self)
requireSendable(SystemLanguageModel.UseCase.self)
requireSendable(SystemLanguageModel.Guardrails.self)
requireSendable(SystemLanguageModel.Availability.self)
requireObservable(SystemLanguageModel.self)

let models: [(String, SystemLanguageModel)] = [
    ("default", SystemLanguageModel.default),
    ("init()", SystemLanguageModel()),
    ("contentTagging", SystemLanguageModel(useCase: .contentTagging)),
    ("permissive", SystemLanguageModel(guardrails: .permissiveContentTransformations)),
    ("tagging+default", SystemLanguageModel(useCase: .contentTagging, guardrails: .default)),
    ("general+permissive", SystemLanguageModel(useCase: .general, guardrails: .permissiveContentTransformations)),
]

for (name, model) in models {
    if model.isAvailable { fail("\(name): isAvailable should be false") }
    if model.availability != .unavailable(.deviceNotEligible) {
        fail("\(name): availability should be unavailable(deviceNotEligible), got \(model.availability)")
    }
    if model.availability == .available { fail("\(name): availability should not be available") }
}

if SystemLanguageModel.UseCase.general == SystemLanguageModel.UseCase.contentTagging {
    fail("use cases must differ")
}

let reasons: Set<SystemLanguageModel.Availability.UnavailableReason> = [
    .deviceNotEligible, .appleIntelligenceNotEnabled, .modelNotReady,
]
if reasons.count != 3 { fail("UnavailableReason cases must be distinct and hashable") }

switch SystemLanguageModel.default.availability {
case .available:
    fail("exhaustive switch reached .available")
case .unavailable(let reason):
    switch reason {
    case .deviceNotEligible: break
    case .appleIntelligenceNotEnabled, .modelNotReady:
        fail("unexpected unavailable reason \(reason)")
    @unknown default:
        fail("unknown unavailable reason")
    }
}

// The supported-language list does not depend on the device being eligible.
let languages = SystemLanguageModel.default.supportedLanguages
let expected = ["da-DK", "de-DE", "en-AU", "en-GB", "en-US", "es-419", "es-ES", "es-US",
                "fr-CA", "fr-FR", "it-IT", "ja-JP", "ko-KR", "nb-NO", "nl-NL", "pt-BR",
                "pt-PT", "sv-SE", "tr-TR", "vi-VN", "zh-CN", "zh-HK", "zh-TW"]
let expectedSet = Set(expected.map { Locale.Language(identifier: $0) })
if languages != expectedSet { fail("supportedLanguages mismatch: \(languages)") }
if !languages.contains(Locale.Language(identifier: "en-US")) { fail("en-US missing") }
if languages.contains(Locale.Language(identifier: "en")) { fail("bare en should not be a member") }

print("PASS")
