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

#ifndef _BAURLDOWNLOAD_H_
#define _BAURLDOWNLOAD_H_

#import <Foundation/Foundation.h>
#import <BackgroundAssets/BADownload.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(13.0))
@interface BAURLDownload : BADownload <NSCopying>

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
                         essential:(BOOL)essential
                          fileSize:(NSUInteger)fileSize
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                          priority:(BADownloaderPriority)priority API_AVAILABLE(macos(13.3));

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
                          fileSize:(NSUInteger)fileSize
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier API_AVAILABLE(macos(13.3));

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier API_DEPRECATED("", macos(13.0, 13.3));

- (instancetype)initWithIdentifier:(NSString *)identifier
                           request:(NSURLRequest *)request
        applicationGroupIdentifier:(NSString *)applicationGroupIdentifier
                          priority:(BADownloaderPriority)priority API_DEPRECATED("", macos(13.0, 13.3));

@end

NS_ASSUME_NONNULL_END

#endif
