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
const CFStringRef kCGImagePropertyGIFHasGlobalColorMap = CFSTR("HasGlobalColorMap");
const CFStringRef kCGImagePropertyGIFImageColorMap = CFSTR("ImageColorMap");

const CFStringRef kCGImagePropertyIPTCDictionary = CFSTR("{IPTC}");
const CFStringRef kCGImagePropertyIPTCKeywords = CFSTR("Keywords");

const CFStringRef kCGImagePropertyHasAlpha = CFSTR("HasAlpha");
const CFStringRef kCGImagePropertyProfileName = CFSTR("ProfileName"); // guessed value

const CFStringRef kCGImagePropertyAPNGDelayTime = CFSTR("DelayTime");
const CFStringRef kCGImagePropertyAPNGLoopCount = CFSTR("LoopCount");
const CFStringRef kCGImagePropertyAPNGUnclampedDelayTime = CFSTR("UnclampedDelayTime");

const CFStringRef kCGImageMetadataNamespaceExif = CFSTR("http://ns.adobe.com/exif/1.0/");
const CFStringRef kCGImageMetadataPrefixExif = CFSTR("exif");
const CFStringRef kCGImagePropertyPNGAuthor = CFSTR("Author");
const CFStringRef kCGImagePropertyPNGDescription = CFSTR("Description");

// The documentation names these keys but not their strings. The strings were
// read from the constants on macOS 26.7.3: a dictionary key is the key name
// without its kCGImageProperty<Dictionary> prefix, a dictionary is "{Name}",
// and the IPTC Ext keys drop "Ext" and "ExtLocation".
// Test: tests/src/imageio_property_names.c
const CFStringRef kCGImagePropertyWidth = CFSTR("Width");
const CFStringRef kCGImagePropertyHeight = CFSTR("Height");
const CFStringRef kCGImagePropertyDepth = CFSTR("Depth");
const CFStringRef kCGImagePropertyPixelFormat = CFSTR("PixelFormat");
const CFStringRef kCGImagePropertyColorModel = CFSTR("ColorModel");
const CFStringRef kCGImagePropertyColorModelRGB = CFSTR("RGB");
const CFStringRef kCGImagePropertyColorModelGray = CFSTR("Gray");
const CFStringRef kCGImagePropertyColorModelCMYK = CFSTR("CMYK");
const CFStringRef kCGImagePropertyGPSDictionary = CFSTR("{GPS}");
const CFStringRef kCGImagePropertyGPSLatitude = CFSTR("Latitude");
const CFStringRef kCGImagePropertyGPSLatitudeRef = CFSTR("LatitudeRef");
const CFStringRef kCGImagePropertyGPSLongitude = CFSTR("Longitude");
const CFStringRef kCGImagePropertyGPSLongitudeRef = CFSTR("LongitudeRef");
const CFStringRef kCGImagePropertyExifDateTimeOriginal = CFSTR("DateTimeOriginal");
const CFStringRef kCGImagePropertyExifOffsetTimeOriginal = CFSTR("OffsetTimeOriginal");
const CFStringRef kCGImagePropertyExifUserComment = CFSTR("UserComment");
const CFStringRef kCGImagePropertyHEICSDictionary = CFSTR("{HEICS}");
const CFStringRef kCGImagePropertyHEICSDelayTime = CFSTR("DelayTime");
const CFStringRef kCGImagePropertyHEICSUnclampedDelayTime = CFSTR("UnclampedDelayTime");
const CFStringRef kCGImagePropertyIPTCCity = CFSTR("City");
const CFStringRef kCGImagePropertyIPTCCredit = CFSTR("Credit");
const CFStringRef kCGImagePropertyIPTCSubLocation = CFSTR("SubLocation");
const CFStringRef kCGImagePropertyIPTCExtDigitalSourceType = CFSTR("DigitalSourceType");
const CFStringRef kCGImagePropertyIPTCExtLocationGPSAltitude = CFSTR("GPSAltitude");
const CFStringRef kCGImagePropertyIPTCExtLocationGPSLatitude = CFSTR("GPSLatitude");
const CFStringRef kCGImagePropertyIPTCExtLocationGPSLongitude = CFSTR("GPSLongitude");
const CFStringRef kCGImagePropertyTIFFArtist = CFSTR("Artist");
const CFStringRef kCGImagePropertyTIFFCopyright = CFSTR("Copyright");
const CFStringRef kCGImagePropertyTIFFDocumentName = CFSTR("DocumentName");
const CFStringRef kCGImagePropertyTIFFImageDescription = CFSTR("ImageDescription");
const CFStringRef kCGImagePropertyMakerAppleDictionary = CFSTR("{MakerApple}");
