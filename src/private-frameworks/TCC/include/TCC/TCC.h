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

#ifndef _TCC_H_
#define _TCC_H_

#import <Foundation/Foundation.h>

#import <TCC/OS_tcc_attributed_entity.h>
#import <TCC/OS_tcc_attributed_entity.h>
#import <TCC/OS_tcc_authorization_cache.h>
#import <TCC/OS_tcc_authorization_cache.h>
#import <TCC/OS_tcc_authorization_change_monitor.h>
#import <TCC/OS_tcc_authorization_change_monitor.h>
#import <TCC/OS_tcc_authorization_record.h>
#import <TCC/OS_tcc_authorization_record.h>
#import <TCC/OS_tcc_credential.h>
#import <TCC/OS_tcc_credential.h>
#import <TCC/OS_tcc_events_filter.h>
#import <TCC/OS_tcc_events_filter.h>
#import <TCC/OS_tcc_events_subscription.h>
#import <TCC/OS_tcc_events_subscription.h>
#import <TCC/OS_tcc_identity.h>
#import <TCC/OS_tcc_identity.h>
#import <TCC/OS_tcc_message_options.h>
#import <TCC/OS_tcc_message_options.h>
#import <TCC/OS_tcc_object.h>
#import <TCC/OS_tcc_object.h>
#import <TCC/OS_tcc_server.h>
#import <TCC/OS_tcc_server.h>
#import <TCC/OS_tcc_service.h>
#import <TCC/OS_tcc_service.h>


void *TCCAccessCheckAuditToken(void);
void *TCCAccessCopyBundleIdentifiersDisabledForService(void);
void *TCCAccessCopyBundleIdentifiersForService(void);
void *TCCAccessCopyInformation(void);
void *TCCAccessCopyInformationForBundle(void);
void *TCCAccessCopyInformationForBundleId(void);
void *TCCAccessCopyOverrides(void);
void *TCCAccessDeclarePolicy(void);
void *TCCAccessGetOverride(void);
void *TCCAccessGetOverrides(void);
void *TCCAccessOverride(void);
void *TCCAccessPreflight(void);
void *TCCAccessPreflightWithAuditToken(void);
void *TCCAccessRequest(void);
void *TCCAccessRequestIndirect(void);
void *TCCAccessRequestIndirectWithOptions(void);
void *TCCAccessReset(void);
void *TCCAccessResetForBundle(void);
void *TCCAccessResetForBundleId(void);
void *TCCAccessResetForBundleIdWithAuditToken(void);
void *TCCAccessResetForPath(void);
void *TCCAccessResetPoliciesExcept(void);
void *TCCAccessRestricted(void);
void *TCCAccessSelectPolicyForExtensionWithIdentifier(void);
void *TCCAccessSet(void);
void *TCCAccessSetForAuditToken(void);
void *TCCAccessSetForAuditTokenWithOptions(void);
void *TCCAccessSetForBundle(void);
void *TCCAccessSetForBundleId(void);
void *TCCAccessSetForBundleIdAndCodeRequirement(void);
void *TCCAccessSetForBundleIdWithOptions(void);
void *TCCAccessSetForCodeIdentity(void);
void *TCCAccessSetForIndirectObject(void);
void *TCCAccessSetForPath(void);
void *TCCAccessSetForPathWithOptions(void);
void *TCCAccessSetOverride(void);
void *TCCAccessSetPidResponsibleForPid(void);
void *TCCAuthCopyDescription(void);
void *TCCAuthGetReasonDescription(void);
void *TCCCreateDesignatedRequirementIdentityFromAuditToken(void);
void *TCCCreateDesignatedRequirementIdentityFromAuditTokenForService(void);
void *TCCCreateDesignatedRequirementIdentityFromPath(void);
void *TCCCreateIndirectObjectIdentityForFileProviderDomainFromPath(void);
void *TCCCreateIndirectObjectIdentityFromAuditToken(void);
void *TCCCreateIndirectObjectIdentityFromFileProviderDomainID(void);
void *TCCExpirationCopyInformationForService(void);
void *TCCExpirationResetService(void);
void *TCCExpirationResetServiceForBundleIdentifier(void);
void *TCCExpirationResetServiceForPath(void);
void *TCCServiceCopyDescriptions(void);
void *TCCServiceCopyNames(void);
void *TCCTestInternal(void);
void *TCCTestInternalWithReplyHander(void);
void *TCCTestInternalWithReplyHander4(void);
void *_tcc_util_create_CFError_with_user_info(void);
void *tcc_attributed_entity_create(void);
void *tcc_attributed_entity_get_path(void);
void *tcc_attributed_entity_get_type(void);
void *tcc_authorization_cache_copy_authorization(void);
void *tcc_authorization_cache_create(void);
void *tcc_authorization_cache_handle_authorization_changed_for_service(void);
void *tcc_authorization_cache_set_authorization(void);
void *tcc_authorization_cache_shared_instance(void);
void *tcc_authorization_change_monitor_create(void);
void *tcc_authorization_change_monitor_start_monitoring_service(void);
void *tcc_authorization_changed_for_service_notification_name(void);
void *tcc_authorization_check_audit_token(void);
void *tcc_authorization_preflight(void);
void *tcc_authorization_preflight_with_endpoint(void);
void *tcc_authorization_record_create(void);
void *tcc_authorization_record_get_allow_standard_user_to_set_system_service(void);
void *tcc_authorization_record_get_authorization_reason(void);
void *tcc_authorization_record_get_authorization_right(void);
void *tcc_authorization_record_get_authorization_value(void);
void *tcc_authorization_record_get_expired_at_date(void);
void *tcc_authorization_record_get_indirect_object_attributed_entity(void);
void *tcc_authorization_record_get_indirect_object_identity(void);
void *tcc_authorization_record_get_last_modified_date(void);
void *tcc_authorization_record_get_service(void);
void *tcc_authorization_record_get_subject_attributed_entity(void);
void *tcc_authorization_record_get_subject_identity(void);
void *tcc_authorization_record_get_version(void);
void *tcc_authorization_record_set_allow_standard_user_to_set_system_service(void);
void *tcc_authorization_record_set_authorization_reason(void);
void *tcc_authorization_record_set_authorization_right(void);
void *tcc_authorization_record_set_authorization_value(void);
void *tcc_authorization_record_set_expired_at_date(void);
void *tcc_authorization_record_set_indirect_object_attributed_entity(void);
void *tcc_authorization_record_set_indirect_object_identity(void);
void *tcc_authorization_record_set_last_modified_date(void);
void *tcc_authorization_record_set_service(void);
void *tcc_authorization_record_set_subject_attributed_entity(void);
void *tcc_authorization_record_set_subject_identity(void);
void *tcc_authorization_record_set_version(void);
void *tcc_authorization_request(void);
void *tcc_authorization_request_with_endpoint(void);
void *tcc_credential_create_data_from_process_with_audit_token(void);
void *tcc_credential_create_for_file_provider_domain_identifier(void);
void *tcc_credential_create_for_process_with_audit_token(void);
void *tcc_credential_create_for_process_with_audit_token_and_assumed_identity(void);
void *tcc_credential_create_for_self_with_assumed_identity(void);
void *tcc_credential_singleton_for_self(void);
void *tcc_events_filter_create_with_criteria(void);
void *tcc_events_filter_singleton_for_check_in(void);
void *tcc_events_subscribe(void);
void *tcc_events_unsubscribe(void);
void *tcc_identity_copy_external_representation(void);
void *tcc_identity_create(void);
void *tcc_identity_create_from_external_representation(void);
void *tcc_identity_get_identifier(void);
void *tcc_identity_get_type(void);
void *tcc_identity_get_verifier_code_requirement_string(void);
void *tcc_identity_get_verifier_type(void);
void *tcc_identity_set_verifier_code_requirement_data(void);
void *tcc_identity_set_verifier_code_requirement_string(void);
void *tcc_identity_type_and_identifiers_are_equal(void);
void *tcc_message_options_create(void);
void *tcc_message_options_get_cache_policy(void);
void *tcc_message_options_get_reply_handler_policy(void);
void *tcc_message_options_get_request_prompt_policy(void);
void *tcc_message_options_get_request_prompt_rights_mask(void);
void *tcc_message_options_get_request_record_upgrade_policy(void);
void *tcc_message_options_get_request_usage_string_policy(void);
void *tcc_message_options_set_cache_policy(void);
void *tcc_message_options_set_reply_handler_policy(void);
void *tcc_message_options_set_request_prompt_policy(void);
void *tcc_message_options_set_request_prompt_rights_mask(void);
void *tcc_message_options_set_request_record_upgrade_policy(void);
void *tcc_message_options_set_request_usage_string_policy(void);
void *tcc_object_copy_description(void);
void *tcc_object_equal(void);
void *tcc_object_hash(void);
void *tcc_release(void);
void *tcc_release_internal(void);
void *tcc_retain(void);
void *tcc_retain_internal(void);
void *tcc_server_cache_authorization(void);
void *tcc_server_copy_cached_authorization(void);
void *tcc_server_create(void);
void *tcc_server_create_with_endpoint(void);
void *tcc_server_get_reply_handler_queue(void);
void *tcc_server_message_get_authorization_records_by_identity(void);
void *tcc_server_message_get_authorization_records_by_service(void);
void *tcc_server_message_get_identity_for_credential(void);
void *tcc_server_message_request_authorization(void);
void *tcc_server_message_set_authorization_change_policy(void);
void *tcc_server_message_set_authorization_value(void);
void *tcc_server_set_authorization_change_handler(void);
void *tcc_server_singleton_default(void);
void *tcc_service_get_CF_name(void);
void *tcc_service_get_authorization_prompt_rights_mask(void);
void *tcc_service_get_authorization_rights_mask(void);
void *tcc_service_get_authorization_value_type(void);
void *tcc_service_get_is_system_service(void);
void *tcc_service_get_name(void);
void *tcc_service_singleton_for_CF_name(void);
void *tcc_service_singleton_for_name(void);


extern void* kTCCAccessCheckOptionPrompt;
extern void* kTCCAccessCheckOptionPurposeRequired;
extern void* kTCCAccessFlags;
extern void* kTCCAccessRequestOptionPurpose;
extern void* kTCCCodeIdentityAuditToken;
extern void* kTCCCodeIdentityAuthority;
extern void* kTCCCodeIdentityBundleID;
extern void* kTCCCodeIdentityCSFlags;
extern void* kTCCCodeIdentityCanSendToAnyTarget;
extern void* kTCCCodeIdentityDesignatedRequirementAuthority;
extern void* kTCCCodeIdentityDesignatedRequirementData;
extern void* kTCCCodeIdentityDevIDAuthority;
extern void* kTCCCodeIdentityEntitlementName;
extern void* kTCCCodeIdentityExecutableURL;
extern void* kTCCCodeIdentityIdentifier;
extern void* kTCCCodeIdentityIdentifierType;
extern void* kTCCCodeIdentityPlatformType;
extern void* kTCCCodeIdentityPromptPolicy;
extern void* kTCCCodeIdentityRequestingProcessAuditToken;
extern void* kTCCCodeIdentitySDKVersion;
extern void* kTCCCodeIdentitySigningID;
extern void* kTCCCodeIdentityTeamID;
extern void* const kTCCErrorDomain;
extern void* kTCCIndirectObjectAuditToken;
extern void* kTCCIndirectObjectFileProviderDomainID;
extern void* kTCCIndirectObjectType;
extern void* kTCCInfoAllowStandardUserToSetSystemService;
extern void* kTCCInfoBundle;
extern void* kTCCInfoCodeRequirement;
extern void* kTCCInfoExpiredAt;
extern void* kTCCInfoGranted;
extern void* kTCCInfoIndirectObjectIdentityDictionary;
extern void* kTCCInfoLastModified;
extern void* kTCCInfoLastUsed;
extern void* kTCCInfoPath;
extern void* kTCCInfoService;
extern void* kTCCInfoSubjectIdentityDictionary;
extern void* const kTCCObjectDictionaryKeyCallBacks;
extern void* const kTCCObjectDictionaryValueCallBacks;
extern void* kTCCServiceAccessibility;
extern void* kTCCServiceAddressBook;
extern void* kTCCServiceAll;
extern void* kTCCServiceAppleEvents;
extern void* kTCCServiceBluetoothAlways;
extern void* kTCCServiceCalendar;
extern void* kTCCServiceCamera;
extern void* kTCCServiceContactsFull;
extern void* kTCCServiceContactsLimited;
extern void* kTCCServiceDeveloperTool;
extern void* kTCCServiceEndpointSecurityClient;
extern void* kTCCServiceFacebook;
extern void* kTCCServiceFileProviderDomain;
extern void* kTCCServiceFileProviderPresence;
extern void* kTCCServiceFocusStatus;
extern void* kTCCServiceGameCenterFriends;
extern void* kTCCServiceLinkedIn;
extern void* kTCCServiceListenEvent;
extern void* kTCCServiceLiverpool;
extern void* kTCCServiceLocation;
extern void* kTCCServiceMediaLibrary;
extern void* kTCCServiceMicrophone;
extern void* kTCCServiceMotion;
extern void* kTCCServicePhotos;
extern void* kTCCServicePhotosAdd;
extern void* kTCCServicePostEvent;
extern void* kTCCServicePrototype3Rights;
extern void* kTCCServicePrototype4Rights;
extern void* kTCCServiceReminders;
extern void* kTCCServiceScreenCapture;
extern void* kTCCServiceShareKit;
extern void* kTCCServiceSinaWeibo;
extern void* kTCCServiceSiri;
extern void* kTCCServiceSpeechRecognition;
extern void* kTCCServiceSystemPolicyAllFiles;
extern void* kTCCServiceSystemPolicyAppBundles;
extern void* kTCCServiceSystemPolicyDesktopFolder;
extern void* kTCCServiceSystemPolicyDeveloperFiles;
extern void* kTCCServiceSystemPolicyDocumentsFolder;
extern void* kTCCServiceSystemPolicyDownloadsFolder;
extern void* kTCCServiceSystemPolicyNetworkVolumes;
extern void* kTCCServiceSystemPolicyRemovableVolumes;
extern void* kTCCServiceSystemPolicySysAdminFiles;
extern void* kTCCServiceTencentWeibo;
extern void* kTCCServiceTwitter;
extern void* kTCCServiceUbiquity;
extern void* kTCCServiceUserAvailability;
extern void* kTCCServiceUserTracking;
extern void* kTCCServiceWebBrowserPublicKeyCredential;
extern void* kTCCServiceWebKitIntelligentTrackingPrevention;
extern void* kTCCServiceWillow;
extern void* kTCCSetNoKill;

#endif

