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

#ifndef _SHAZAMCORE_H_
#define _SHAZAMCORE_H_

#import <Foundation/Foundation.h>

#import <ShazamCore/NSData+Compression.h>
#import <ShazamCore/NSError+SHCoreError.h>
#import <ShazamCore/SHAnalytics.h>
#import <ShazamCore/SHBagContract.h>
#import <ShazamCore/SHCampaignTokens.h>
#import <ShazamCore/SHCompression.h>
#import <ShazamCore/SHCoreError.h>
#import <ShazamCore/SHDataCompression.h>
#import <ShazamCore/SHDataStream.h>
#import <ShazamCore/SHDataStreamInput.h>
#import <ShazamCore/SHDataStreamOutput.h>
#import <ShazamCore/SHDefaultConfigurationValues.h>
#import <ShazamCore/SHEndpoints.h>
#import <ShazamCore/SHEntitlements.h>
#import <ShazamCore/SHFeatureFlags.h>
#import <ShazamCore/SHHost.h>
#import <ShazamCore/SHJSONLDataDetokenizer.h>
#import <ShazamCore/SHJSONLFileReader.h>
#import <ShazamCore/SHJSONLFileWriter.h>
#import <ShazamCore/SHLocalization.h>
#import <ShazamCore/SHMusicSubscriptionStatus.h>
#import <ShazamCore/SHOffers.h>
#import <ShazamCore/SHOffersNetworkRequester.h>
#import <ShazamCore/SHRemoteConfiguration.h>
#import <ShazamCore/SHStorefront.h>
#import <ShazamCore/SHTokenizedURL.h>


void *sh_log_object(void);


extern void* const SHAnalyticsActionAppleMusic;
extern void* const SHAnalyticsActionNotificationTap;
extern void* const SHAnalyticsEventMusicRecognitionAlert;
extern void* const SHAnalyticsEventMusicRecognitionEnabled;
extern void* const SHAnalyticsEventMusicRecognitionHistory;
extern void* const SHAnalyticsEventMusicRecognitionList;
extern void* const SHAnalyticsEventMusicRecognitionResult;
extern void* const SHAnalyticsEventMusicRecognitionResultAction;
extern void* const SHAnalyticsEventMusicRecognitionResultExpanded;
extern void* const SHAnalyticsPayloadActionKey;
extern void* const SHAnalyticsPayloadEnabledKey;
extern void* const SHAnalyticsPayloadErrorCodeKey;
extern void* const SHAnalyticsPayloadMatchKey;
extern void* const SHAnalyticsPayloadNumberShazamsKey;
extern void* const SHAnalyticsPayloadSecureAccountKey;
extern void* const SHAnalyticsPayloadSourceKey;
extern void* const SHCampaignTokenDefaultCampaignValue;
extern void* const SHCoreErrorDomain;
extern void* const SHDefaultWorkerTimeout;
extern void* const SHRecordingIntermissionDefaultValue;
extern void* const SHShazamKitExternalAudioRecordingEntitlement;
extern void* const SHShazamKitInternalAudioRecordingEntitlement;
extern void* const SHShazamKitInternalClientEntitlement;
extern void* const SHShazamKitTCCEntitlement;
extern void* const SHShazamKitTCCEntitlementMediaLibrary;
extern void* const SHShazamKitTCCEntitlementMicrophone;

#endif

