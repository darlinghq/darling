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

#ifndef _VTFRAMEPROCESSORCONFIGURATION_H_
#define _VTFRAMEPROCESSORCONFIGURATION_H_

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

@protocol VTFrameProcessorConfiguration <NSObject>
@required
@property (class, nonatomic, readonly, getter=isSupported) BOOL supported;
@property (nonatomic, readonly) NSArray<NSNumber *> *frameSupportedPixelFormats;
@property (nonatomic, readonly) NSDictionary<NSString *, id> *sourcePixelBufferAttributes;
@property (nonatomic, readonly) NSDictionary<NSString *, id> *destinationPixelBufferAttributes;
@optional
@property (nonatomic, readonly) NSInteger nextFrameCount;
@property (nonatomic, readonly) NSInteger previousFrameCount;
@property (class, nonatomic, readonly) CMVideoDimensions maximumDimensions;
@property (class, nonatomic, readonly) CMVideoDimensions minimumDimensions;
@end
#endif

#endif
