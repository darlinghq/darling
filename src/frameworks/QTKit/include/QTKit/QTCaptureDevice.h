/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

@interface QTCaptureDevice : NSObject

@end

QTKIT_EXTERN NSString * const QTCaptureAnyMediaType;
QTKIT_EXTERN NSString * const QTCaptureDeviceAVCTransportControlsAttribute;
QTKIT_EXTERN NSString * const QTCaptureDeviceAVCTransportControlsPlaybackModeKey;
QTKIT_EXTERN NSString * const QTCaptureDeviceAVCTransportControlsSpeedKey;
QTKIT_EXTERN NSString * const QTCaptureDeviceAttributeDidChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureDeviceAttributeWillChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureDeviceAvailableInputSourcesAttribute;
QTKIT_EXTERN NSString * const QTCaptureDeviceChangedAttributeKey;
QTKIT_EXTERN NSString * const QTCaptureDeviceFormatDescriptionsDidChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureDeviceFormatDescriptionsWillChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureDeviceInputSourceIdentifierAttribute;
QTKIT_EXTERN NSString * const QTCaptureDeviceInputSourceIdentifierKey;
QTKIT_EXTERN NSString * const QTCaptureDeviceInputSourceLocalizedDisplayNameKey;
#if defined(__i386__)
QTKIT_EXTERN NSString * const QTCaptureDeviceLegacySequenceGrabberAttribute;
#endif
QTKIT_EXTERN NSString * const QTCaptureDeviceLinkedDevicesAttribute;
QTKIT_EXTERN NSString * const QTCaptureDeviceSuspendedAttribute;
QTKIT_EXTERN NSString * const QTCaptureDeviceWasConnectedNotification;
QTKIT_EXTERN NSString * const QTCaptureDeviceWasDisconnectedNotification;
