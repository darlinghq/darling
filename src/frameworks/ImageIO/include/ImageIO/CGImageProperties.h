/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <ImageIO/ImageIOBase.h>
#include <CoreFoundation/CoreFoundation.h>

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyDPIWidth;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyDPIHeight;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPixelHeight;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPixelWidth;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyOrientation;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyExifDictionary;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFXResolution;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFYResolution;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFOrientation;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyHasAlpha;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPNGAuthor;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPNGDescription;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPNGDictionary;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFDelayTime;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFLoopCount;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFUnclampedDelayTime;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFHasGlobalColorMap;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGIFImageColorMap;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCKeywords;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyAPNGDelayTime;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyAPNGLoopCount;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyAPNGUnclampedDelayTime;

IMAGEIO_EXTERN const CFStringRef kCGImageMetadataNamespaceExif;
IMAGEIO_EXTERN const CFStringRef kCGImageMetadataPrefixExif;

IMAGEIO_EXTERN const CFStringRef kCGImagePropertyWidth;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyHeight;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyDepth;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyPixelFormat;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyColorModel;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyColorModelRGB;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyColorModelGray;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyColorModelCMYK;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGPSDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGPSLatitude;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGPSLatitudeRef;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGPSLongitude;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyGPSLongitudeRef;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyExifDateTimeOriginal;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyExifOffsetTimeOriginal;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyExifUserComment;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyHEICSDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyHEICSDelayTime;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyHEICSUnclampedDelayTime;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCCity;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCCredit;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCSubLocation;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCExtDigitalSourceType;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCExtLocationGPSAltitude;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCExtLocationGPSLatitude;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCExtLocationGPSLongitude;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFArtist;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFCopyright;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFDocumentName;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFImageDescription;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyMakerAppleDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyExifDateTimeDigitized;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCDateCreated;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyIPTCDigitalCreationDate;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyRawDictionary;
IMAGEIO_EXTERN const CFStringRef kCGImagePropertyTIFFDateTime;
