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

// An item that a Quick Look preview shows: the URL of its contents, and
// optionally a title and a state of the preview's own.
@protocol QLPreviewItem <NSObject>

@required
@property (readonly, nullable) NSURL *previewItemURL;

@optional
@property (readonly, nullable) NSString *previewItemTitle;
@property (readonly, nullable) id previewItemDisplayState;

@end

// NSURL is a preview item itself, showing the file it refers to.
@interface NSURL (QLPreviewConvenienceAdditions) <QLPreviewItem>
@end
