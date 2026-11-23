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

#import <QuickLookThumbnailing/QLThumbnailGenerator.h>
#import <dispatch/dispatch.h>

// Placeholder: thumbnail generation is not implemented. Every generate and save method reports an error
// in QLThumbnailErrorDomain with no representation, so a caller takes its ordinary failure path instead of
// waiting for a handler that never runs. The case is the documented one for the operation (generationFailed
// for generate, savingToURLFailed for save), and NSUnderlyingErrorKey holds NSFeatureUnsupportedError in
// NSCocoaErrorDomain so the real reason stays visible. cancelRequest: has nothing to cancel.
//
// GUESS: the documentation does not say on which queue handlers run; a global queue is used.
// GUESS: generateRepresentationsForRequest:updateHandler: reports its error with the highest-quality type in
// the request's representationTypes (Thumbnail, then LowQualityThumbnail, then Icon), or Thumbnail when the set
// is empty; the documentation names no type for an error.

static NSError *QLThumbnailingUnsupportedError(QLThumbnailError code)
{
    NSError *reason = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFeatureUnsupportedError userInfo:nil];
    return [NSError errorWithDomain:QLThumbnailErrorDomain code:code userInfo:@{NSUnderlyingErrorKey: reason}];
}

static QLThumbnailRepresentationType QLThumbnailingErrorType(QLThumbnailGenerationRequestRepresentationTypes types)
{
    if (types & QLThumbnailGenerationRequestRepresentationTypeThumbnail) {
        return QLThumbnailRepresentationTypeThumbnail;
    }
    if (types & QLThumbnailGenerationRequestRepresentationTypeLowQualityThumbnail) {
        return QLThumbnailRepresentationTypeLowQualityThumbnail;
    }
    if (types & QLThumbnailGenerationRequestRepresentationTypeIcon) {
        return QLThumbnailRepresentationTypeIcon;
    }
    return QLThumbnailRepresentationTypeThumbnail;
}

@implementation QLThumbnailGenerator

+ (QLThumbnailGenerator *)sharedGenerator
{
    static QLThumbnailGenerator *generator;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        generator = [[QLThumbnailGenerator alloc] init];
    });
    return generator;
}

- (void)generateBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                           completionHandler:(void (^)(QLThumbnailRepresentation *thumbnail, NSError *error))completionHandler
{
    if (!completionHandler) {
        return;
    }
    NSError *error = QLThumbnailingUnsupportedError(QLThumbnailErrorGenerationFailed);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        completionHandler(nil, error);
    });
}

- (void)generateRepresentationsForRequest:(QLThumbnailGenerationRequest *)request
                            updateHandler:(void (^)(QLThumbnailRepresentation *thumbnail, QLThumbnailRepresentationType type, NSError *error))updateHandler
{
    if (!updateHandler) {
        return;
    }
    NSError *error = QLThumbnailingUnsupportedError(QLThumbnailErrorGenerationFailed);
    QLThumbnailRepresentationType type = QLThumbnailingErrorType(request.representationTypes);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        updateHandler(nil, type, error);
    });
}

- (void)saveBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                             toFileAtURL:(NSURL *)fileURL
                         withContentType:(NSString *)contentType
                       completionHandler:(void (^)(NSError *error))completionHandler
{
    if (!completionHandler) {
        return;
    }
    NSError *error = QLThumbnailingUnsupportedError(QLThumbnailErrorSavingToURLFailed);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        completionHandler(error);
    });
}

- (void)saveBestRepresentationForRequest:(QLThumbnailGenerationRequest *)request
                             toFileAtURL:(NSURL *)fileURL
                           asContentType:(UTType *)contentType
                       completionHandler:(void (^)(NSError *error))completionHandler
{
    if (!completionHandler) {
        return;
    }
    NSError *error = QLThumbnailingUnsupportedError(QLThumbnailErrorSavingToURLFailed);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        completionHandler(error);
    });
}

- (void)cancelRequest:(QLThumbnailGenerationRequest *)request
{
}

@end
