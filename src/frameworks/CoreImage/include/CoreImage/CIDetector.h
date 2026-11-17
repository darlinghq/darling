/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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

#import <Foundation/Foundation.h>

@class CIImage;
@class CIContext;
@class CIFeature;

NS_ASSUME_NONNULL_BEGIN

@interface CIDetector : NSObject

+ (nullable CIDetector *)detectorOfType:(NSString *)type
                                context:(nullable CIContext *)context
                                options:(nullable NSDictionary<NSString *, id> *)options;

- (NSArray<CIFeature *> *)featuresInImage:(CIImage *)image;
- (NSArray<CIFeature *> *)featuresInImage:(CIImage *)image
                                  options:(nullable NSDictionary<NSString *, id> *)options;

@end

extern NSString *const CIDetectorTypeFace;
extern NSString *const CIDetectorTypeRectangle;
extern NSString *const CIDetectorTypeQRCode;
extern NSString *const CIDetectorTypeText;

extern NSString *const CIDetectorAccuracy;
extern NSString *const CIDetectorAccuracyLow;
extern NSString *const CIDetectorAccuracyHigh;
extern NSString *const CIDetectorTracking;
extern NSString *const CIDetectorMinFeatureSize;
extern NSString *const CIDetectorImageOrientation;

NS_ASSUME_NONNULL_END
