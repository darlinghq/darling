/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <ImageIO/ImageIOBase.h>
#import <CoreFoundation/CFDictionary.h>
#import <CoreFoundation/CFURL.h>

@class O2ImageSource;
typedef O2ImageSource *CGImageSourceRef;

#import <CoreGraphics/CGImage.h>

IMAGEIO_EXTERN CGImageSourceRef CGImageSourceCreateWithData(CFDataRef data, CFDictionaryRef options);
IMAGEIO_EXTERN CGImageSourceRef CGImageSourceCreateWithURL(CFURLRef url, CFDictionaryRef options);

IMAGEIO_EXTERN CFStringRef CGImageSourceGetType(CGImageSourceRef self);

IMAGEIO_EXTERN size_t CGImageSourceGetCount(CGImageSourceRef self);

IMAGEIO_EXTERN CGImageRef CGImageSourceCreateImageAtIndex(CGImageSourceRef self, size_t index, CFDictionaryRef options);
IMAGEIO_EXTERN CFDictionaryRef CGImageSourceCopyPropertiesAtIndex(CGImageSourceRef self, size_t index, CFDictionaryRef options);
