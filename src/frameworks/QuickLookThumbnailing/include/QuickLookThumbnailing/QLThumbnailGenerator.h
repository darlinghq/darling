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

#ifndef _QLTHUMBNAILGENERATOR_H_
#define _QLTHUMBNAILGENERATOR_H_

#import <Foundation/Foundation.h>
#import <UniformTypeIdentifiers/UTType.h>
#import <QuickLookThumbnailing/QLThumbnailError.h>
#import <QuickLookThumbnailing/QLThumbnailGenerationRequest.h>
#import <QuickLookThumbnailing/QLThumbnailRepresentation.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(macos(10.15))
@interface QLThumbnailGenerator : NSObject

@property (class, readonly) QLThumbnailGenerator *sharedGenerator;

- (void)generateBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                           completionHandler:(void (^)(QLThumbnailRepresentation *_Nullable thumbnail, NSError *_Nullable error))completionHandler;

- (void)generateRepresentationsForRequest:(QLThumbnailGenerationRequest *)request
                            updateHandler:(void (^_Nullable)(QLThumbnailRepresentation *_Nullable thumbnail, QLThumbnailRepresentationType type, NSError *_Nullable error))updateHandler;

- (void)saveBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                             toFileAtURL:(NSURL *)fileURL
                         withContentType:(NSString *)contentType
                       completionHandler:(void (^)(NSError *_Nullable error))completionHandler
    API_DEPRECATED_WITH_REPLACEMENT("-saveBestRepresentationForRequest:toFileAtURL:asContentType:completionHandler:", macos(11.0, 15.0));

- (void)saveBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                             toFileAtURL:(NSURL *)fileURL
                           asContentType:(UTType *)contentType
                       completionHandler:(void (^)(NSError *_Nullable error))completionHandler;

- (void)cancelRequest:(QLThumbnailGenerationRequest *)request;

@end

NS_ASSUME_NONNULL_END

#endif
