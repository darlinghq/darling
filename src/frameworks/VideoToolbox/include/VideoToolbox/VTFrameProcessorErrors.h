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

#ifndef _VTFRAMEPROCESSORERRORS_H_
#define _VTFRAMEPROCESSORERRORS_H_

#ifdef __OBJC__
#import <Foundation/Foundation.h>

extern NSErrorDomain const VTFrameProcessorErrorDomain;

typedef NS_ERROR_ENUM(VTFrameProcessorErrorDomain, VTFrameProcessorError)
{
    VTFrameProcessorUnknownError            = -19730,
    VTFrameProcessorUnsupportedResolution   = -19731,
    VTFrameProcessorSessionNotStarted       = -19732,
    VTFrameProcessorSessionAlreadyActive    = -19733,
    VTFrameProcessorFatalError              = -19734,
    VTFrameProcessorSessionLevelError       = -19735,
    VTFrameProcessorInitializationFailed    = -19736,
    VTFrameProcessorUnsupportedInput        = -19737,
    VTFrameProcessorMemoryAllocationFailure = -19738,
    VTFrameProcessorRevisionNotSupported    = -19739,
    VTFrameProcessorProcessingError         = -19740,
    VTFrameProcessorInvalidParameterError   = -19741,
    VTFrameProcessorInvalidFrameTiming      = -19742,
    VTFrameProcessorAssetDownloadFailed     = -19743,
};
#endif

#endif
