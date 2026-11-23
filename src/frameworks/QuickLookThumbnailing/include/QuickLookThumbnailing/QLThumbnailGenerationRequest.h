/*
 This file is part of Darling.

 Copyright (C) 2026 Darling developers

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

#ifndef _QLTHUMBNAILGENERATIONREQUEST_H_
#define _QLTHUMBNAILGENERATIONREQUEST_H_

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UniformTypeIdentifiers/UTType.h>

// The documentation names the members, including "all", but gives no numeric values. One bit per
// member in the documented order (icon, low quality, thumbnail), All the union: the values macOS
// 26.7.3 reports (black-box observation). Test: tests/src/quicklookthumbnailing_placeholder.m
typedef NS_OPTIONS(NSUInteger, QLThumbnailGenerationRequestRepresentationTypes) {
    QLThumbnailGenerationRequestRepresentationTypeIcon = 1 << 0,
    QLThumbnailGenerationRequestRepresentationTypeLowQualityThumbnail = 1 << 1,
    QLThumbnailGenerationRequestRepresentationTypeThumbnail = 1 << 2,
    QLThumbnailGenerationRequestRepresentationTypeAll = (1 << 0) | (1 << 1) | (1 << 2),
} API_AVAILABLE(macos(10.15));

NS_ASSUME_NONNULL_BEGIN

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.15))
@interface QLThumbnailGenerationRequest : NSObject {
@private
    NSURL *_fileURL;
    UTType *_contentType;
    CGSize _size;
    CGFloat _scale;
    QLThumbnailGenerationRequestRepresentationTypes _representationTypes;
    CGFloat _minimumDimension;
    BOOL _iconMode;
}

- (instancetype)initWithFileAtURL:(NSURL *)url
                             size:(CGSize)size
                            scale:(CGFloat)scale
              representationTypes:(QLThumbnailGenerationRequestRepresentationTypes)representationTypes;

@property (nonatomic, readonly) CGSize size;
@property (nonatomic, readonly) CGFloat scale;
@property (nonatomic, readonly) QLThumbnailGenerationRequestRepresentationTypes representationTypes;
@property (nonatomic, copy, null_resettable) UTType *contentType API_AVAILABLE(macos(11.0));
@property (nonatomic, assign) CGFloat minimumDimension;
@property (nonatomic, assign) BOOL iconMode;

@end

NS_ASSUME_NONNULL_END

#endif
