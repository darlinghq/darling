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

#ifndef _SFUniversalLink_H_
#define _SFUniversalLink_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/* Describes the app that would handle a web page URL as a universal link. */
@interface SFUniversalLink : NSObject {
@private
	NSURL *_webpageURL;
	NSURL *_applicationURL;
	BOOL _enabled;
}

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/* Answers nil when no app handles the URL as a universal link. */
- (nullable instancetype)initWithWebpageURL:(NSURL *)url;

@property(readonly) NSURL *webpageURL;
@property(readonly) NSURL *applicationURL;
@property(getter=isEnabled) BOOL enabled;

@end

NS_ASSUME_NONNULL_END

#endif
