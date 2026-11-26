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

#ifndef _QLTHUMBNAILREPRESENTATION_H_
#define _QLTHUMBNAILREPRESENTATION_H_

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

@class NSImage;

// The documentation gives the cases but not their numeric values. They are 0, 1 and 2 in the order
// the documentation lists the cases, as macOS 26.7.3 reports (black-box observation).
// Test: tests/src/quicklookthumbnailing_placeholder.m
typedef NS_ENUM(NSInteger, QLThumbnailRepresentationType) {
    QLThumbnailRepresentationTypeIcon = 0,
    QLThumbnailRepresentationTypeLowQualityThumbnail = 1,
    QLThumbnailRepresentationTypeThumbnail = 2,
} API_AVAILABLE(macos(10.15));

NS_ASSUME_NONNULL_BEGIN

// The instance variables are declared here, and the properties are synthesized by name, because the
// i386 slice uses the legacy Objective-C runtime: no automatic property synthesis and no instance
// variables in @implementation.
API_AVAILABLE(macos(10.15))
@interface QLThumbnailRepresentation : NSObject {
@private
    CGImageRef _CGImage;
    NSImage *_NSImage;
    QLThumbnailRepresentationType _type;
    CGRect _contentRect;
}

@property (nonatomic, readonly) CGImageRef CGImage;
@property (nonatomic, strong, readonly) NSImage *NSImage;
@property (nonatomic, readonly) QLThumbnailRepresentationType type;
@property (nonatomic, readonly) CGRect contentRect API_AVAILABLE(macos(12.0));

@end

NS_ASSUME_NONNULL_END

#endif
