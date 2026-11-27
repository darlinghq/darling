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

#ifndef _VTFRAMEPROCESSORFRAME_H_
#define _VTFRAMEPROCESSORFRAME_H_

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CMTime.h>

// A pixel buffer with the time it is presented at.
@interface VTFrameProcessorFrame : NSObject {
@private
    CVPixelBufferRef _buffer;
    CMTime _presentationTimeStamp;
}
- (nullable instancetype)initWithBuffer:(CVPixelBufferRef)buffer presentationTimeStamp:(CMTime)presentationTimeStamp;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@property (nonatomic, readonly) CVPixelBufferRef buffer;
@property (nonatomic, readonly) CMTime presentationTimeStamp;
@end
#endif

#endif
