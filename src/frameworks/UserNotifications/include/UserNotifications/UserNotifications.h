/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _UserNotifications_H_
#define _UserNotifications_H_

#import <Foundation/Foundation.h>

#import <UserNotifications/BSXPCCoding.h>
#import <UserNotifications/NSExtensionRequestHandling.h>
#import <UserNotifications/NSXPCListenerDelegate.h>
#import <UserNotifications/UNNotificationAttachmentThumbnailOptions.h>
#import <UserNotifications/UNUserNotificationCenterDelegateConnectionListenerDelegate.h>
#import <UserNotifications/UNUserNotificationCenterDelegateServiceProtocol.h>
#import <UserNotifications/UNUserNotificationClientProtocol.h>
#import <UserNotifications/UNUserNotificationServerProtocol.h>
#import <UserNotifications/UNUserNotificationServiceConnectionObserver.h>
#import <UserNotifications/_UNNotificationServiceExtensionHostXPCInterface.h>
#import <UserNotifications/_UNNotificationServiceExtensionRemoteXPCInterface.h>
#import <UserNotifications/_UNNotificationServiceExtensionXPCInterface.h>
#import <UserNotifications/UNLocalizedStringFactory.h>
#import <UserNotifications/UNNotificationTrigger.h>
#import <UserNotifications/UNCalendarNotificationTrigger.h>
#import <UserNotifications/UNLegacyNotificationTrigger.h>
#import <UserNotifications/UNLocationNotificationTrigger.h>
#import <UserNotifications/UNPushNotificationTrigger.h>
#import <UserNotifications/UNTimeIntervalNotificationTrigger.h>
#import <UserNotifications/_UNNotificationServiceExtensionRemoteContext.h>
#import <UserNotifications/UNNotificationAction.h>
#import <UserNotifications/UNTextInputNotificationAction.h>
#import <UserNotifications/UNNotificationSettings.h>
#import <UserNotifications/UNMutableNotificationSettings.h>
#import <UserNotifications/UNNotificationAttachmentOptions.h>
#import <UserNotifications/UNNotificationIcon.h>
#import <UserNotifications/UNMovieNotificationAttachmentOptions.h>
#import <UserNotifications/UNUserNotificationCenterDelegateConnectionListener.h>
#import <UserNotifications/UNSecurityScopedURL.h>
#import <UserNotifications/UNNotificationServiceExtension.h>
#import <UserNotifications/_UNNotificationServiceExtensionHostContext.h>
#import <UserNotifications/UNNotificationRequest.h>
#import <UserNotifications/UNNotification.h>
#import <UserNotifications/UNNotificationSound.h>
#import <UserNotifications/UNMutableNotificationSound.h>
#import <UserNotifications/_UNNotificationServiceExtensionContext.h>
#import <UserNotifications/UNAudioNotificationAttachmentOptions.h>
#import <UserNotifications/UNLocalizedString.h>
#import <UserNotifications/UNPushNotificationRequestBuilder.h>
#import <UserNotifications/UNImageNotificationAttachmentOptions.h>
#import <UserNotifications/UNNotificationAttachment.h>
#import <UserNotifications/UNUserNotificationService.h>
#import <UserNotifications/UNNotificationResponse.h>
#import <UserNotifications/UNTextInputNotificationResponse.h>
#import <UserNotifications/UNNotificationContent.h>
#import <UserNotifications/UNMutableNotificationContent.h>
#import <UserNotifications/UNUserNotificationCenterDelegateService.h>
#import <UserNotifications/UNNotificationCategory.h>
#import <UserNotifications/UNMutableNotificationCategory.h>
#import <UserNotifications/UNUserNotificationCenter.h>
#import <UserNotifications/UNUserNotificationServiceConnection.h>
#import <UserNotifications/UNNotificationTopic.h>
#import <UserNotifications/UNNotificationTopicRequest.h>

void* UNDoubleIsZero(void);
void* UNEqualBools(void);
void* UNEqualDoubles(void);
void* UNEqualFloats(void);
void* UNEqualIntegers(void);
void* UNEqualObjects(void);
void* UNEqualStrings(void);
void* UNFloatIsZero(void);
void* UNFormatLocalizedString(void);
void* UNFormatLocalizedStringInBundleWithDefaultValue(void);
void* UNIsInternalInstall(void);
void* UNNotificationAttachmentFamilyFromTypeIdentifier(void);
void* UNNotificationAttachmentFamilyMaximumSize(void);
void* UNNotificationAttachmentFamilyToString(void);
void* UNNotificationGroupingSettingString(void);
void* UNNotificationSettingString(void);
void* UNRegisterUserNotificationsLogging(void);
void* UNSafeCast(void);
void* UNSafeCastAny(void);
void* UNSafeConforms(void);
void* UNShowPreviewsSettingString(void);
void* UNSimilarSets(void);
void* UNSimilarStrings(void);
void* un_CGRectContainsRect(void);
void* un_CGRectCreateDictionaryRepresentation(void);
void* un_CGRectEqualToRect(void);
void* un_CGRectIsEmpty(void);
void* un_CGRectMakeWithDictionaryRepresentation(void);
void* un_CGRectNull(void);
void* un_CMTimeCompare(void);
void* un_CMTimeCopyAsDictionary(void);
void* un_CMTimeMakeFromDictionary(void);
void* un_CMTimeMakeWithSeconds(void);
void* un_kCMTimeInvalid(void);

#endif
