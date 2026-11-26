/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

NS_ASSUME_NONNULL_BEGIN

// What is known about a link: its URLs, title and the providers of its
// icon, image and video.
@interface LPLinkMetadata : NSObject <NSCopying, NSSecureCoding>

@property (nonatomic, retain, nullable) NSURL *originalURL;
@property (nonatomic, retain, nullable) NSURL *URL;
@property (nonatomic, copy, nullable) NSString *title;
@property (nonatomic, retain, nullable) NSItemProvider *iconProvider;
@property (nonatomic, retain, nullable) NSItemProvider *imageProvider;
@property (nonatomic, retain, nullable) NSItemProvider *videoProvider;
@property (nonatomic, retain, nullable) NSURL *remoteVideoURL;

@end

NS_ASSUME_NONNULL_END
