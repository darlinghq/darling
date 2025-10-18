/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <CoreFoundation/CoreFoundation.h>

const CFStringRef kCGImagePropertyPixelHeight=(CFStringRef)@"PixelHeight";
const CFStringRef kCGImagePropertyOrientation=(CFStringRef)@"Orientation";

const CFStringRef kCGImagePropertyTIFFDictionary=(CFStringRef)@"{TIFF}";
const CFStringRef kCGImagePropertyExifDictionary=(CFStringRef)@"{Exif}";

const CFStringRef kCGImagePropertyTIFFXResolution=(CFStringRef)@"XResolution";
const CFStringRef kCGImagePropertyTIFFYResolution=(CFStringRef)@"YResolution";
const CFStringRef kCGImagePropertyTIFFOrientation=(CFStringRef)@"Orientation";

const CFStringRef kCGImagePropertyDPIHeight = CFSTR("DPIHeight");
const CFStringRef kCGImagePropertyDPIWidth = CFSTR("DPIWidth");
const CFStringRef kCGImagePropertyPixelWidth = CFSTR("PixelWidth");
const CFStringRef kCGImagePropertyPNGDictionary = CFSTR("{PNG}");
const CFStringRef kCGImagePropertyPNGGamma = CFSTR("Gamma");
const CFStringRef kCGImagePropertyGIFDelayTime = CFSTR("DelayTime");
const CFStringRef kCGImagePropertyGIFDictionary = CFSTR("{GIF}");
const CFStringRef kCGImagePropertyGIFLoopCount = CFSTR("LoopCount");
const CFStringRef kCGImagePropertyGIFUnclampedDelayTime = CFSTR("UnclampedDelayTime");

const CFStringRef kCGImagePropertyHasAlpha = CFSTR("HasAlpha");
const CFStringRef kCGImagePropertyProfileName = CFSTR("ProfileName"); // guessed value

const CFStringRef kCGImagePropertyAPNGDelayTime = CFSTR("APNGDelayTime"); // ditto
const CFStringRef kCGImagePropertyAPNGLoopCount = CFSTR("APNGLoopCount"); // ditto
