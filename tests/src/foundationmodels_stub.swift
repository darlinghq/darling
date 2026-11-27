import FoundationModels
import Observation

let defaultModel = SystemLanguageModel.default
let defaultInitModel = SystemLanguageModel()
let useCaseOnlyModel = SystemLanguageModel(useCase: .contentTagging)
let guardrailsOnlyModel = SystemLanguageModel(guardrails: .permissiveContentTransformations)
let taggingModel = SystemLanguageModel(useCase: .contentTagging, guardrails: .default)
let permissiveModel = SystemLanguageModel(useCase: .general, guardrails: .permissiveContentTransformations)

let reasons: Set<SystemLanguageModel.Availability.UnavailableReason> = [
    .deviceNotEligible,
    .appleIntelligenceNotEnabled,
    .modelNotReady,
]
let useCases: Set<SystemLanguageModel.UseCase> = [.general, .contentTagging]
let guardrails: Set<SystemLanguageModel.Guardrails> = [.default, .permissiveContentTransformations]

let observableModel: any Observable = defaultModel
let sendableModel: any Sendable = defaultModel
_ = (observableModel, sendableModel)

let matchedDeviceNotEligible: Bool
switch defaultModel.availability {
case .available:
    matchedDeviceNotEligible = false
case .unavailable(let reason):
    switch reason {
    case .deviceNotEligible:
        matchedDeviceNotEligible = true
    case .appleIntelligenceNotEnabled, .modelNotReady:
        matchedDeviceNotEligible = false
    @unknown default:
        matchedDeviceNotEligible = false
    }
}

guard matchedDeviceNotEligible,
      defaultModel.availability == .unavailable(.deviceNotEligible),
      !defaultModel.isAvailable,
      defaultInitModel.availability == .unavailable(.deviceNotEligible),
      !defaultInitModel.isAvailable,
      useCaseOnlyModel.availability == .unavailable(.deviceNotEligible),
      guardrailsOnlyModel.availability == .unavailable(.deviceNotEligible),
      taggingModel.availability == .unavailable(.deviceNotEligible),
      !taggingModel.isAvailable,
      permissiveModel.availability == .unavailable(.deviceNotEligible),
      !permissiveModel.isAvailable,
      defaultModel.availability != .available,
      defaultModel.availability != .unavailable(.appleIntelligenceNotEnabled),
      defaultModel.availability != .unavailable(.modelNotReady),
      reasons.count == 3,
      useCases.count == 2,
      guardrails.count == 2
else {
    fatalError("FoundationModels stub verification failed")
}

print("ALL PASSED")
