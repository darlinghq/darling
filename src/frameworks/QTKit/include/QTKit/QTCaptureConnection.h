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

@interface QTCaptureConnection : NSObject

@end

QTKIT_EXTERN NSString * const QTCaptureConnectionAttributeDidChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureConnectionAttributeWillChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureConnectionAudioAveragePowerLevelsAttribute;
QTKIT_EXTERN NSString * const QTCaptureConnectionAudioMasterVolumeAttribute;
QTKIT_EXTERN NSString * const QTCaptureConnectionAudioPeakHoldLevelsAttribute;
QTKIT_EXTERN NSString * const QTCaptureConnectionAudioVolumesAttribute;
QTKIT_EXTERN NSString * const QTCaptureConnectionChangedAttributeKey;
QTKIT_EXTERN NSString * const QTCaptureConnectionEnabledAudioChannelsAttribute;
QTKIT_EXTERN NSString * const QTCaptureConnectionFormatDescriptionDidChangeNotification;
QTKIT_EXTERN NSString * const QTCaptureConnectionFormatDescriptionWillChangeNotification;
