/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef _QTKitDefines_H_
#define _QTKitDefines_H_

#include <Foundation/Foundation.h>

#ifdef __cplusplus
#define QTKIT_EXTERN extern "C"
#else
#define QTKIT_EXTERN extern
#endif

QTKIT_EXTERN NSString * const QTDisallowedForInitializationPurposeException;
QTKIT_EXTERN NSString * const QTErrorCaptureInputKey;
QTKIT_EXTERN NSString * const QTErrorCaptureOutputKey;
QTKIT_EXTERN NSString * const QTErrorDeviceKey;
QTKIT_EXTERN NSString * const QTErrorDiscontinuityFlagsKey;
QTKIT_EXTERN NSString * const QTErrorExcludingDeviceKey;
QTKIT_EXTERN NSString * const QTErrorFileSizeKey;
QTKIT_EXTERN NSString * const QTErrorRecordingSuccesfullyFinishedKey;
QTKIT_EXTERN NSString * const QTErrorTimeKey;
QTKIT_EXTERN NSString * const QTKitErrorDomain;
QTKIT_EXTERN NSString * const QTKitErrorException;
QTKIT_EXTERN NSString * const QTKitErrorValue;
#if defined(__x86_64__)
QTKIT_EXTERN NSString * const QTKitServerDidDieNotification;
QTKIT_EXTERN NSString * const QTKitServerDidDieProcessIDKey;
#endif

#endif
