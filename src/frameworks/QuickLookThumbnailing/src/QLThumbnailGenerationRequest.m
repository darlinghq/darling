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

#import <QuickLookThumbnailing/QLThumbnailGenerationRequest.h>
#import <UniformTypeIdentifiers/UTCoreTypes.h>

// Placeholder: the documented parameters and properties are stored. While contentType is unset, or after it
// is reset to nil, it is derived from the file extension, as documented.
@implementation QLThumbnailGenerationRequest

@synthesize size = _size;
@synthesize scale = _scale;
@synthesize representationTypes = _representationTypes;
@synthesize minimumDimension = _minimumDimension;
@synthesize iconMode = _iconMode;

- (instancetype)initWithFileAtURL:(NSURL *)url
                             size:(CGSize)size
                            scale:(CGFloat)scale
              representationTypes:(QLThumbnailGenerationRequestRepresentationTypes)representationTypes
{
    if ((self = [super init])) {
        _fileURL = [url copy];
        _size = size;
        _scale = scale;
        _representationTypes = representationTypes;
    }
    return self;
}

- (void)dealloc
{
    [_fileURL release];
    [_contentType release];
    [super dealloc];
}

- (UTType *)contentType
{
    if (_contentType) {
        return _contentType;
    }
    // GUESS: the documentation does not say what a URL with no extension yields; the generic data type is used.
    return [UTType typeWithFilenameExtension:[_fileURL pathExtension]] ?: UTTypeData;
}

- (void)setContentType:(UTType *)contentType
{
    if (_contentType != contentType) {
        [_contentType release];
        _contentType = [contentType copy];
    }
}

@end
