/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <Foundation/Foundation.h>
#include <darling-dmg/darling_dmg.h>

@class DIMImageOpener;

// How an opener attaches an image: darling_dmg_attach, or a stand-in in tests.
typedef int (*DIMAttachFunction)(darling_dmg_image *image, const struct darling_dmg_attach_options *options,
                                 char *mountPoint, size_t mountPointSize);

// What an opener asks its user: whether they agree to the image's licence agreement, and to see an error.
@protocol DIMImageOpenerUI <NSObject>
// Calls reply exactly once, on the main queue. The image stays open until then.
- (void)imageOpener:(DIMImageOpener *)opener askAgreementForImage:(darling_dmg_image *)image
              reply:(void (^)(BOOL agreed))reply;
// Calls done once the error has been seen.
- (void)imageOpener:(DIMImageOpener *)opener showError:(NSString *)message
        informative:(NSString *)informative completion:(void (^)(void))done;
@end

// Opens one disk image: asks for its licence agreement when it has one, attaches it (unless it is attached
// already) and opens its volume. Used on the main thread.
@interface DIMImageOpener : NSObject
- (instancetype)initWithURL:(NSURL *)imageURL ui:(id<DIMImageOpenerUI>)ui;
@property (nonatomic, readonly) NSURL *imageURL;
@property (nonatomic) DIMAttachFunction attachFunction;        // darling_dmg_attach unless changed
@property (nonatomic, copy) void (^openVolume)(NSURL *volume); // opens it with NSWorkspace unless changed
// Calls done on the main queue with the opened volume's URL, or nil when nothing was opened (the agreement was
// declined, or an error was shown).
- (void)startWithCompletion:(void (^)(NSURL *volume))done;
@end
