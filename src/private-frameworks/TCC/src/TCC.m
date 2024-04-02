/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <TCC/TCC.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


void* kTCCAccessCheckOptionPrompt = (void*)0;
void* kTCCAccessCheckOptionPurposeRequired = (void*)0;
void* kTCCAccessFlags = (void*)0;
void* kTCCAccessRequestOptionPurpose = (void*)0;
void* kTCCCodeIdentityAuditToken = (void*)0;
void* kTCCCodeIdentityAuthority = (void*)0;
void* kTCCCodeIdentityBundleID = (void*)0;
void* kTCCCodeIdentityCSFlags = (void*)0;
void* kTCCCodeIdentityCanSendToAnyTarget = (void*)0;
void* kTCCCodeIdentityDesignatedRequirementAuthority = (void*)0;
void* kTCCCodeIdentityDesignatedRequirementData = (void*)0;
void* kTCCCodeIdentityDevIDAuthority = (void*)0;
void* kTCCCodeIdentityEntitlementName = (void*)0;
void* kTCCCodeIdentityExecutableURL = (void*)0;
void* kTCCCodeIdentityIdentifier = (void*)0;
void* kTCCCodeIdentityIdentifierType = (void*)0;
void* kTCCCodeIdentityPlatformType = (void*)0;
void* kTCCCodeIdentityPromptPolicy = (void*)0;
void* kTCCCodeIdentityRequestingProcessAuditToken = (void*)0;
void* kTCCCodeIdentitySDKVersion = (void*)0;
void* kTCCCodeIdentitySigningID = (void*)0;
void* kTCCCodeIdentityTeamID = (void*)0;
void* const kTCCErrorDomain = (void*)0;
void* kTCCIndirectObjectAuditToken = (void*)0;
void* kTCCIndirectObjectFileProviderDomainID = (void*)0;
void* kTCCIndirectObjectType = (void*)0;
void* kTCCInfoAllowStandardUserToSetSystemService = (void*)0;
void* kTCCInfoBundle = (void*)0;
void* kTCCInfoCodeRequirement = (void*)0;
void* kTCCInfoExpiredAt = (void*)0;
void* kTCCInfoGranted = (void*)0;
void* kTCCInfoIndirectObjectIdentityDictionary = (void*)0;
void* kTCCInfoLastModified = (void*)0;
void* kTCCInfoLastUsed = (void*)0;
void* kTCCInfoPath = (void*)0;
void* kTCCInfoService = (void*)0;
void* kTCCInfoSubjectIdentityDictionary = (void*)0;
void* const kTCCObjectDictionaryKeyCallBacks = (void*)0;
void* const kTCCObjectDictionaryValueCallBacks = (void*)0;
void* kTCCServiceAccessibility = (void*)0;
void* kTCCServiceAddressBook = (void*)0;
void* kTCCServiceAll = (void*)0;
void* kTCCServiceAppleEvents = (void*)0;
void* kTCCServiceBluetoothAlways = (void*)0;
void* kTCCServiceCalendar = (void*)0;
void* kTCCServiceCamera = (void*)0;
void* kTCCServiceContactsFull = (void*)0;
void* kTCCServiceContactsLimited = (void*)0;
void* kTCCServiceDeveloperTool = (void*)0;
void* kTCCServiceEndpointSecurityClient = (void*)0;
void* kTCCServiceFacebook = (void*)0;
void* kTCCServiceFileProviderDomain = (void*)0;
void* kTCCServiceFileProviderPresence = (void*)0;
void* kTCCServiceFocusStatus = (void*)0;
void* kTCCServiceGameCenterFriends = (void*)0;
void* kTCCServiceLinkedIn = (void*)0;
void* kTCCServiceListenEvent = (void*)0;
void* kTCCServiceLiverpool = (void*)0;
void* kTCCServiceLocation = (void*)0;
void* kTCCServiceMediaLibrary = (void*)0;
void* kTCCServiceMicrophone = (void*)0;
void* kTCCServiceMotion = (void*)0;
void* kTCCServicePhotos = (void*)0;
void* kTCCServicePhotosAdd = (void*)0;
void* kTCCServicePostEvent = (void*)0;
void* kTCCServicePrototype3Rights = (void*)0;
void* kTCCServicePrototype4Rights = (void*)0;
void* kTCCServiceReminders = (void*)0;
void* kTCCServiceScreenCapture = (void*)0;
void* kTCCServiceShareKit = (void*)0;
void* kTCCServiceSinaWeibo = (void*)0;
void* kTCCServiceSiri = (void*)0;
void* kTCCServiceSpeechRecognition = (void*)0;
void* kTCCServiceSystemPolicyAllFiles = (void*)0;
void* kTCCServiceSystemPolicyAppBundles = (void*)0;
void* kTCCServiceSystemPolicyDesktopFolder = (void*)0;
void* kTCCServiceSystemPolicyDeveloperFiles = (void*)0;
void* kTCCServiceSystemPolicyDocumentsFolder = (void*)0;
void* kTCCServiceSystemPolicyDownloadsFolder = (void*)0;
void* kTCCServiceSystemPolicyNetworkVolumes = (void*)0;
void* kTCCServiceSystemPolicyRemovableVolumes = (void*)0;
void* kTCCServiceSystemPolicySysAdminFiles = (void*)0;
void* kTCCServiceTencentWeibo = (void*)0;
void* kTCCServiceTwitter = (void*)0;
void* kTCCServiceUbiquity = (void*)0;
void* kTCCServiceUserAvailability = (void*)0;
void* kTCCServiceUserTracking = (void*)0;
void* kTCCServiceWebBrowserPublicKeyCredential = (void*)0;
void* kTCCServiceWebKitIntelligentTrackingPrevention = (void*)0;
void* kTCCServiceWillow = (void*)0;
void* kTCCSetNoKill = (void*)0;

void *TCCAccessCheckAuditToken(void) {
    if (verbose) puts("STUB: TCCAccessCheckAuditToken called");
    return NULL;
}

void *TCCAccessCopyBundleIdentifiersDisabledForService(void) {
    if (verbose) puts("STUB: TCCAccessCopyBundleIdentifiersDisabledForService called");
    return NULL;
}

void *TCCAccessCopyBundleIdentifiersForService(void) {
    if (verbose) puts("STUB: TCCAccessCopyBundleIdentifiersForService called");
    return NULL;
}

void *TCCAccessCopyInformation(void) {
    if (verbose) puts("STUB: TCCAccessCopyInformation called");
    return NULL;
}

void *TCCAccessCopyInformationForBundle(void) {
    if (verbose) puts("STUB: TCCAccessCopyInformationForBundle called");
    return NULL;
}

void *TCCAccessCopyInformationForBundleId(void) {
    if (verbose) puts("STUB: TCCAccessCopyInformationForBundleId called");
    return NULL;
}

void *TCCAccessCopyOverrides(void) {
    if (verbose) puts("STUB: TCCAccessCopyOverrides called");
    return NULL;
}

void *TCCAccessDeclarePolicy(void) {
    if (verbose) puts("STUB: TCCAccessDeclarePolicy called");
    return NULL;
}

void *TCCAccessGetOverride(void) {
    if (verbose) puts("STUB: TCCAccessGetOverride called");
    return NULL;
}

void *TCCAccessGetOverrides(void) {
    if (verbose) puts("STUB: TCCAccessGetOverrides called");
    return NULL;
}

void *TCCAccessOverride(void) {
    if (verbose) puts("STUB: TCCAccessOverride called");
    return NULL;
}

void *TCCAccessPreflight(void) {
    if (verbose) puts("STUB: TCCAccessPreflight called");
    return NULL;
}

void *TCCAccessPreflightWithAuditToken(void) {
    if (verbose) puts("STUB: TCCAccessPreflightWithAuditToken called");
    return NULL;
}

void *TCCAccessRequest(void) {
    if (verbose) puts("STUB: TCCAccessRequest called");
    return NULL;
}

void *TCCAccessRequestIndirect(void) {
    if (verbose) puts("STUB: TCCAccessRequestIndirect called");
    return NULL;
}

void *TCCAccessRequestIndirectWithOptions(void) {
    if (verbose) puts("STUB: TCCAccessRequestIndirectWithOptions called");
    return NULL;
}

void *TCCAccessReset(void) {
    if (verbose) puts("STUB: TCCAccessReset called");
    return NULL;
}

void *TCCAccessResetForBundle(void) {
    if (verbose) puts("STUB: TCCAccessResetForBundle called");
    return NULL;
}

void *TCCAccessResetForBundleId(void) {
    if (verbose) puts("STUB: TCCAccessResetForBundleId called");
    return NULL;
}

void *TCCAccessResetForBundleIdWithAuditToken(void) {
    if (verbose) puts("STUB: TCCAccessResetForBundleIdWithAuditToken called");
    return NULL;
}

void *TCCAccessResetForPath(void) {
    if (verbose) puts("STUB: TCCAccessResetForPath called");
    return NULL;
}

void *TCCAccessResetPoliciesExcept(void) {
    if (verbose) puts("STUB: TCCAccessResetPoliciesExcept called");
    return NULL;
}

void *TCCAccessRestricted(void) {
    if (verbose) puts("STUB: TCCAccessRestricted called");
    return NULL;
}

void *TCCAccessSelectPolicyForExtensionWithIdentifier(void) {
    if (verbose) puts("STUB: TCCAccessSelectPolicyForExtensionWithIdentifier called");
    return NULL;
}

void *TCCAccessSet(void) {
    if (verbose) puts("STUB: TCCAccessSet called");
    return NULL;
}

void *TCCAccessSetForAuditToken(void) {
    if (verbose) puts("STUB: TCCAccessSetForAuditToken called");
    return NULL;
}

void *TCCAccessSetForAuditTokenWithOptions(void) {
    if (verbose) puts("STUB: TCCAccessSetForAuditTokenWithOptions called");
    return NULL;
}

void *TCCAccessSetForBundle(void) {
    if (verbose) puts("STUB: TCCAccessSetForBundle called");
    return NULL;
}

void *TCCAccessSetForBundleId(void) {
    if (verbose) puts("STUB: TCCAccessSetForBundleId called");
    return NULL;
}

void *TCCAccessSetForBundleIdAndCodeRequirement(void) {
    if (verbose) puts("STUB: TCCAccessSetForBundleIdAndCodeRequirement called");
    return NULL;
}

void *TCCAccessSetForBundleIdWithOptions(void) {
    if (verbose) puts("STUB: TCCAccessSetForBundleIdWithOptions called");
    return NULL;
}

void *TCCAccessSetForCodeIdentity(void) {
    if (verbose) puts("STUB: TCCAccessSetForCodeIdentity called");
    return NULL;
}

void *TCCAccessSetForIndirectObject(void) {
    if (verbose) puts("STUB: TCCAccessSetForIndirectObject called");
    return NULL;
}

void *TCCAccessSetForPath(void) {
    if (verbose) puts("STUB: TCCAccessSetForPath called");
    return NULL;
}

void *TCCAccessSetForPathWithOptions(void) {
    if (verbose) puts("STUB: TCCAccessSetForPathWithOptions called");
    return NULL;
}

void *TCCAccessSetOverride(void) {
    if (verbose) puts("STUB: TCCAccessSetOverride called");
    return NULL;
}

void *TCCAccessSetPidResponsibleForPid(void) {
    if (verbose) puts("STUB: TCCAccessSetPidResponsibleForPid called");
    return NULL;
}

void *TCCAuthCopyDescription(void) {
    if (verbose) puts("STUB: TCCAuthCopyDescription called");
    return NULL;
}

void *TCCAuthGetReasonDescription(void) {
    if (verbose) puts("STUB: TCCAuthGetReasonDescription called");
    return NULL;
}

void *TCCCreateDesignatedRequirementIdentityFromAuditToken(void) {
    if (verbose) puts("STUB: TCCCreateDesignatedRequirementIdentityFromAuditToken called");
    return NULL;
}

void *TCCCreateDesignatedRequirementIdentityFromAuditTokenForService(void) {
    if (verbose) puts("STUB: TCCCreateDesignatedRequirementIdentityFromAuditTokenForService called");
    return NULL;
}

void *TCCCreateDesignatedRequirementIdentityFromPath(void) {
    if (verbose) puts("STUB: TCCCreateDesignatedRequirementIdentityFromPath called");
    return NULL;
}

void *TCCCreateIndirectObjectIdentityForFileProviderDomainFromPath(void) {
    if (verbose) puts("STUB: TCCCreateIndirectObjectIdentityForFileProviderDomainFromPath called");
    return NULL;
}

void *TCCCreateIndirectObjectIdentityFromAuditToken(void) {
    if (verbose) puts("STUB: TCCCreateIndirectObjectIdentityFromAuditToken called");
    return NULL;
}

void *TCCCreateIndirectObjectIdentityFromFileProviderDomainID(void) {
    if (verbose) puts("STUB: TCCCreateIndirectObjectIdentityFromFileProviderDomainID called");
    return NULL;
}

void *TCCExpirationCopyInformationForService(void) {
    if (verbose) puts("STUB: TCCExpirationCopyInformationForService called");
    return NULL;
}

void *TCCExpirationResetService(void) {
    if (verbose) puts("STUB: TCCExpirationResetService called");
    return NULL;
}

void *TCCExpirationResetServiceForBundleIdentifier(void) {
    if (verbose) puts("STUB: TCCExpirationResetServiceForBundleIdentifier called");
    return NULL;
}

void *TCCExpirationResetServiceForPath(void) {
    if (verbose) puts("STUB: TCCExpirationResetServiceForPath called");
    return NULL;
}

void *TCCServiceCopyDescriptions(void) {
    if (verbose) puts("STUB: TCCServiceCopyDescriptions called");
    return NULL;
}

void *TCCServiceCopyNames(void) {
    if (verbose) puts("STUB: TCCServiceCopyNames called");
    return NULL;
}

void *TCCTestInternal(void) {
    if (verbose) puts("STUB: TCCTestInternal called");
    return NULL;
}

void *TCCTestInternalWithReplyHander(void) {
    if (verbose) puts("STUB: TCCTestInternalWithReplyHander called");
    return NULL;
}

void *TCCTestInternalWithReplyHander4(void) {
    if (verbose) puts("STUB: TCCTestInternalWithReplyHander4 called");
    return NULL;
}

void *_tcc_util_create_CFError_with_user_info(void) {
    if (verbose) puts("STUB: _tcc_util_create_CFError_with_user_info called");
    return NULL;
}

void *tcc_attributed_entity_create(void) {
    if (verbose) puts("STUB: tcc_attributed_entity_create called");
    return NULL;
}

void *tcc_attributed_entity_get_path(void) {
    if (verbose) puts("STUB: tcc_attributed_entity_get_path called");
    return NULL;
}

void *tcc_attributed_entity_get_type(void) {
    if (verbose) puts("STUB: tcc_attributed_entity_get_type called");
    return NULL;
}

void *tcc_authorization_cache_copy_authorization(void) {
    if (verbose) puts("STUB: tcc_authorization_cache_copy_authorization called");
    return NULL;
}

void *tcc_authorization_cache_create(void) {
    if (verbose) puts("STUB: tcc_authorization_cache_create called");
    return NULL;
}

void *tcc_authorization_cache_handle_authorization_changed_for_service(void) {
    if (verbose) puts("STUB: tcc_authorization_cache_handle_authorization_changed_for_service called");
    return NULL;
}

void *tcc_authorization_cache_set_authorization(void) {
    if (verbose) puts("STUB: tcc_authorization_cache_set_authorization called");
    return NULL;
}

void *tcc_authorization_cache_shared_instance(void) {
    if (verbose) puts("STUB: tcc_authorization_cache_shared_instance called");
    return NULL;
}

void *tcc_authorization_change_monitor_create(void) {
    if (verbose) puts("STUB: tcc_authorization_change_monitor_create called");
    return NULL;
}

void *tcc_authorization_change_monitor_start_monitoring_service(void) {
    if (verbose) puts("STUB: tcc_authorization_change_monitor_start_monitoring_service called");
    return NULL;
}

void *tcc_authorization_changed_for_service_notification_name(void) {
    if (verbose) puts("STUB: tcc_authorization_changed_for_service_notification_name called");
    return NULL;
}

void *tcc_authorization_check_audit_token(void) {
    if (verbose) puts("STUB: tcc_authorization_check_audit_token called");
    return NULL;
}

void *tcc_authorization_preflight(void) {
    if (verbose) puts("STUB: tcc_authorization_preflight called");
    return NULL;
}

void *tcc_authorization_preflight_with_endpoint(void) {
    if (verbose) puts("STUB: tcc_authorization_preflight_with_endpoint called");
    return NULL;
}

void *tcc_authorization_record_create(void) {
    if (verbose) puts("STUB: tcc_authorization_record_create called");
    return NULL;
}

void *tcc_authorization_record_get_allow_standard_user_to_set_system_service(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_allow_standard_user_to_set_system_service called");
    return NULL;
}

void *tcc_authorization_record_get_authorization_reason(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_authorization_reason called");
    return NULL;
}

void *tcc_authorization_record_get_authorization_right(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_authorization_right called");
    return NULL;
}

void *tcc_authorization_record_get_authorization_value(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_authorization_value called");
    return NULL;
}

void *tcc_authorization_record_get_expired_at_date(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_expired_at_date called");
    return NULL;
}

void *tcc_authorization_record_get_indirect_object_attributed_entity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_indirect_object_attributed_entity called");
    return NULL;
}

void *tcc_authorization_record_get_indirect_object_identity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_indirect_object_identity called");
    return NULL;
}

void *tcc_authorization_record_get_last_modified_date(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_last_modified_date called");
    return NULL;
}

void *tcc_authorization_record_get_service(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_service called");
    return NULL;
}

void *tcc_authorization_record_get_subject_attributed_entity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_subject_attributed_entity called");
    return NULL;
}

void *tcc_authorization_record_get_subject_identity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_subject_identity called");
    return NULL;
}

void *tcc_authorization_record_get_version(void) {
    if (verbose) puts("STUB: tcc_authorization_record_get_version called");
    return NULL;
}

void *tcc_authorization_record_set_allow_standard_user_to_set_system_service(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_allow_standard_user_to_set_system_service called");
    return NULL;
}

void *tcc_authorization_record_set_authorization_reason(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_authorization_reason called");
    return NULL;
}

void *tcc_authorization_record_set_authorization_right(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_authorization_right called");
    return NULL;
}

void *tcc_authorization_record_set_authorization_value(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_authorization_value called");
    return NULL;
}

void *tcc_authorization_record_set_expired_at_date(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_expired_at_date called");
    return NULL;
}

void *tcc_authorization_record_set_indirect_object_attributed_entity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_indirect_object_attributed_entity called");
    return NULL;
}

void *tcc_authorization_record_set_indirect_object_identity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_indirect_object_identity called");
    return NULL;
}

void *tcc_authorization_record_set_last_modified_date(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_last_modified_date called");
    return NULL;
}

void *tcc_authorization_record_set_service(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_service called");
    return NULL;
}

void *tcc_authorization_record_set_subject_attributed_entity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_subject_attributed_entity called");
    return NULL;
}

void *tcc_authorization_record_set_subject_identity(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_subject_identity called");
    return NULL;
}

void *tcc_authorization_record_set_version(void) {
    if (verbose) puts("STUB: tcc_authorization_record_set_version called");
    return NULL;
}

void *tcc_authorization_request(void) {
    if (verbose) puts("STUB: tcc_authorization_request called");
    return NULL;
}

void *tcc_authorization_request_with_endpoint(void) {
    if (verbose) puts("STUB: tcc_authorization_request_with_endpoint called");
    return NULL;
}

void *tcc_credential_create_data_from_process_with_audit_token(void) {
    if (verbose) puts("STUB: tcc_credential_create_data_from_process_with_audit_token called");
    return NULL;
}

void *tcc_credential_create_for_file_provider_domain_identifier(void) {
    if (verbose) puts("STUB: tcc_credential_create_for_file_provider_domain_identifier called");
    return NULL;
}

void *tcc_credential_create_for_process_with_audit_token(void) {
    if (verbose) puts("STUB: tcc_credential_create_for_process_with_audit_token called");
    return NULL;
}

void *tcc_credential_create_for_process_with_audit_token_and_assumed_identity(void) {
    if (verbose) puts("STUB: tcc_credential_create_for_process_with_audit_token_and_assumed_identity called");
    return NULL;
}

void *tcc_credential_create_for_self_with_assumed_identity(void) {
    if (verbose) puts("STUB: tcc_credential_create_for_self_with_assumed_identity called");
    return NULL;
}

void *tcc_credential_singleton_for_self(void) {
    if (verbose) puts("STUB: tcc_credential_singleton_for_self called");
    return NULL;
}

void *tcc_events_filter_create_with_criteria(void) {
    if (verbose) puts("STUB: tcc_events_filter_create_with_criteria called");
    return NULL;
}

void *tcc_events_filter_singleton_for_check_in(void) {
    if (verbose) puts("STUB: tcc_events_filter_singleton_for_check_in called");
    return NULL;
}

void *tcc_events_subscribe(void) {
    if (verbose) puts("STUB: tcc_events_subscribe called");
    return NULL;
}

void *tcc_events_unsubscribe(void) {
    if (verbose) puts("STUB: tcc_events_unsubscribe called");
    return NULL;
}

void *tcc_identity_copy_external_representation(void) {
    if (verbose) puts("STUB: tcc_identity_copy_external_representation called");
    return NULL;
}

void *tcc_identity_create(void) {
    if (verbose) puts("STUB: tcc_identity_create called");
    return NULL;
}

void *tcc_identity_create_from_external_representation(void) {
    if (verbose) puts("STUB: tcc_identity_create_from_external_representation called");
    return NULL;
}

void *tcc_identity_get_identifier(void) {
    if (verbose) puts("STUB: tcc_identity_get_identifier called");
    return NULL;
}

void *tcc_identity_get_type(void) {
    if (verbose) puts("STUB: tcc_identity_get_type called");
    return NULL;
}

void *tcc_identity_get_verifier_code_requirement_string(void) {
    if (verbose) puts("STUB: tcc_identity_get_verifier_code_requirement_string called");
    return NULL;
}

void *tcc_identity_get_verifier_type(void) {
    if (verbose) puts("STUB: tcc_identity_get_verifier_type called");
    return NULL;
}

void *tcc_identity_set_verifier_code_requirement_data(void) {
    if (verbose) puts("STUB: tcc_identity_set_verifier_code_requirement_data called");
    return NULL;
}

void *tcc_identity_set_verifier_code_requirement_string(void) {
    if (verbose) puts("STUB: tcc_identity_set_verifier_code_requirement_string called");
    return NULL;
}

void *tcc_identity_type_and_identifiers_are_equal(void) {
    if (verbose) puts("STUB: tcc_identity_type_and_identifiers_are_equal called");
    return NULL;
}

void *tcc_message_options_create(void) {
    if (verbose) puts("STUB: tcc_message_options_create called");
    return NULL;
}

void *tcc_message_options_get_cache_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_get_cache_policy called");
    return NULL;
}

void *tcc_message_options_get_reply_handler_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_get_reply_handler_policy called");
    return NULL;
}

void *tcc_message_options_get_request_prompt_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_get_request_prompt_policy called");
    return NULL;
}

void *tcc_message_options_get_request_prompt_rights_mask(void) {
    if (verbose) puts("STUB: tcc_message_options_get_request_prompt_rights_mask called");
    return NULL;
}

void *tcc_message_options_get_request_record_upgrade_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_get_request_record_upgrade_policy called");
    return NULL;
}

void *tcc_message_options_get_request_usage_string_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_get_request_usage_string_policy called");
    return NULL;
}

void *tcc_message_options_set_cache_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_set_cache_policy called");
    return NULL;
}

void *tcc_message_options_set_reply_handler_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_set_reply_handler_policy called");
    return NULL;
}

void *tcc_message_options_set_request_prompt_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_set_request_prompt_policy called");
    return NULL;
}

void *tcc_message_options_set_request_prompt_rights_mask(void) {
    if (verbose) puts("STUB: tcc_message_options_set_request_prompt_rights_mask called");
    return NULL;
}

void *tcc_message_options_set_request_record_upgrade_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_set_request_record_upgrade_policy called");
    return NULL;
}

void *tcc_message_options_set_request_usage_string_policy(void) {
    if (verbose) puts("STUB: tcc_message_options_set_request_usage_string_policy called");
    return NULL;
}

void *tcc_object_copy_description(void) {
    if (verbose) puts("STUB: tcc_object_copy_description called");
    return NULL;
}

void *tcc_object_equal(void) {
    if (verbose) puts("STUB: tcc_object_equal called");
    return NULL;
}

void *tcc_object_hash(void) {
    if (verbose) puts("STUB: tcc_object_hash called");
    return NULL;
}

void *tcc_release(void) {
    if (verbose) puts("STUB: tcc_release called");
    return NULL;
}

void *tcc_release_internal(void) {
    if (verbose) puts("STUB: tcc_release_internal called");
    return NULL;
}

void *tcc_retain(void) {
    if (verbose) puts("STUB: tcc_retain called");
    return NULL;
}

void *tcc_retain_internal(void) {
    if (verbose) puts("STUB: tcc_retain_internal called");
    return NULL;
}

void *tcc_server_cache_authorization(void) {
    if (verbose) puts("STUB: tcc_server_cache_authorization called");
    return NULL;
}

void *tcc_server_copy_cached_authorization(void) {
    if (verbose) puts("STUB: tcc_server_copy_cached_authorization called");
    return NULL;
}

void *tcc_server_create(void) {
    if (verbose) puts("STUB: tcc_server_create called");
    return NULL;
}

void *tcc_server_create_with_endpoint(void) {
    if (verbose) puts("STUB: tcc_server_create_with_endpoint called");
    return NULL;
}

void *tcc_server_get_reply_handler_queue(void) {
    if (verbose) puts("STUB: tcc_server_get_reply_handler_queue called");
    return NULL;
}

void *tcc_server_message_get_authorization_records_by_identity(void) {
    if (verbose) puts("STUB: tcc_server_message_get_authorization_records_by_identity called");
    return NULL;
}

void *tcc_server_message_get_authorization_records_by_service(void) {
    if (verbose) puts("STUB: tcc_server_message_get_authorization_records_by_service called");
    return NULL;
}

void *tcc_server_message_get_identity_for_credential(void) {
    if (verbose) puts("STUB: tcc_server_message_get_identity_for_credential called");
    return NULL;
}

void *tcc_server_message_request_authorization(void) {
    if (verbose) puts("STUB: tcc_server_message_request_authorization called");
    return NULL;
}

void *tcc_server_message_set_authorization_change_policy(void) {
    if (verbose) puts("STUB: tcc_server_message_set_authorization_change_policy called");
    return NULL;
}

void *tcc_server_message_set_authorization_value(void) {
    if (verbose) puts("STUB: tcc_server_message_set_authorization_value called");
    return NULL;
}

void *tcc_server_set_authorization_change_handler(void) {
    if (verbose) puts("STUB: tcc_server_set_authorization_change_handler called");
    return NULL;
}

void *tcc_server_singleton_default(void) {
    if (verbose) puts("STUB: tcc_server_singleton_default called");
    return NULL;
}

void *tcc_service_get_CF_name(void) {
    if (verbose) puts("STUB: tcc_service_get_CF_name called");
    return NULL;
}

void *tcc_service_get_authorization_prompt_rights_mask(void) {
    if (verbose) puts("STUB: tcc_service_get_authorization_prompt_rights_mask called");
    return NULL;
}

void *tcc_service_get_authorization_rights_mask(void) {
    if (verbose) puts("STUB: tcc_service_get_authorization_rights_mask called");
    return NULL;
}

void *tcc_service_get_authorization_value_type(void) {
    if (verbose) puts("STUB: tcc_service_get_authorization_value_type called");
    return NULL;
}

void *tcc_service_get_is_system_service(void) {
    if (verbose) puts("STUB: tcc_service_get_is_system_service called");
    return NULL;
}

void *tcc_service_get_name(void) {
    if (verbose) puts("STUB: tcc_service_get_name called");
    return NULL;
}

void *tcc_service_singleton_for_CF_name(void) {
    if (verbose) puts("STUB: tcc_service_singleton_for_CF_name called");
    return NULL;
}

void *tcc_service_singleton_for_name(void) {
    if (verbose) puts("STUB: tcc_service_singleton_for_name called");
    return NULL;
}

