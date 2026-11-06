/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Team

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

#ifndef _ScreenCaptureKit_H_
#define _ScreenCaptureKit_H_

#import <Foundation/Foundation.h>

#import <ScreenCaptureKit/SCContentFilter.h>
#import <ScreenCaptureKit/SCContentSharingPicker.h>
#import <ScreenCaptureKit/SCScreenshotManager.h>
#import <ScreenCaptureKit/SCShareableContent.h>
#import <ScreenCaptureKit/SCStream.h>
#import <ScreenCaptureKit/SCStreamConfiguration.h>

FOUNDATION_EXTERN NSString * const SCStreamFrameInfoBoundingRect;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoContentRect;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoContentScale;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoDirtyRects;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoPresenterOverlayContentRect;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoScaleFactor;
FOUNDATION_EXTERN NSString * const SCStreamFrameInfoStatus;

#endif
