// Copyright (C) 2026 Lubos Dolezel
//
// ImageIO property, auxiliary data and destination option names: the strings
// are the ones read from the constants on macOS 26.7.3.
#include <ImageIO/ImageIO.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(const char *name, CFStringRef value, const char *expected) {
    char buffer[128] = "";
    if (value == NULL || !CFStringGetCString(value, buffer, sizeof buffer, kCFStringEncodingUTF8) ||
        strcmp(buffer, expected) != 0) {
        printf("FAIL: %s is '%s', expected '%s'\n", name, buffer, expected);
        failures++;
    }
}

#define CHECK_NAME(n, v) check(#n, n, v)

int main(void) {
    CHECK_NAME(kCGImagePropertyWidth, "Width");
    CHECK_NAME(kCGImagePropertyHeight, "Height");
    CHECK_NAME(kCGImagePropertyDepth, "Depth");
    CHECK_NAME(kCGImagePropertyPixelFormat, "PixelFormat");
    CHECK_NAME(kCGImagePropertyColorModel, "ColorModel");
    CHECK_NAME(kCGImagePropertyColorModelRGB, "RGB");
    CHECK_NAME(kCGImagePropertyColorModelGray, "Gray");
    CHECK_NAME(kCGImagePropertyColorModelCMYK, "CMYK");
    CHECK_NAME(kCGImagePropertyGPSDictionary, "{GPS}");
    CHECK_NAME(kCGImagePropertyGPSLatitude, "Latitude");
    CHECK_NAME(kCGImagePropertyGPSLatitudeRef, "LatitudeRef");
    CHECK_NAME(kCGImagePropertyGPSLongitude, "Longitude");
    CHECK_NAME(kCGImagePropertyGPSLongitudeRef, "LongitudeRef");
    CHECK_NAME(kCGImagePropertyExifDateTimeOriginal, "DateTimeOriginal");
    CHECK_NAME(kCGImagePropertyExifOffsetTimeOriginal, "OffsetTimeOriginal");
    CHECK_NAME(kCGImagePropertyExifUserComment, "UserComment");
    CHECK_NAME(kCGImagePropertyHEICSDictionary, "{HEICS}");
    CHECK_NAME(kCGImagePropertyHEICSDelayTime, "DelayTime");
    CHECK_NAME(kCGImagePropertyHEICSUnclampedDelayTime, "UnclampedDelayTime");
    CHECK_NAME(kCGImagePropertyIPTCCity, "City");
    CHECK_NAME(kCGImagePropertyIPTCCredit, "Credit");
    CHECK_NAME(kCGImagePropertyIPTCSubLocation, "SubLocation");
    CHECK_NAME(kCGImagePropertyIPTCExtDigitalSourceType, "DigitalSourceType");
    CHECK_NAME(kCGImagePropertyIPTCExtLocationGPSAltitude, "GPSAltitude");
    CHECK_NAME(kCGImagePropertyIPTCExtLocationGPSLatitude, "GPSLatitude");
    CHECK_NAME(kCGImagePropertyIPTCExtLocationGPSLongitude, "GPSLongitude");
    CHECK_NAME(kCGImagePropertyTIFFArtist, "Artist");
    CHECK_NAME(kCGImagePropertyTIFFCopyright, "Copyright");
    CHECK_NAME(kCGImagePropertyTIFFDocumentName, "DocumentName");
    CHECK_NAME(kCGImagePropertyTIFFImageDescription, "ImageDescription");
    CHECK_NAME(kCGImagePropertyMakerAppleDictionary, "{MakerApple}");
    CHECK_NAME(kCGImageDestinationImageMaxPixelSize, "kCGImageDestinationImageMaxPixelSize");
    CHECK_NAME(kCGImageDestinationOptimizeColorForSharing, "kCGImageDestinationOptimizeColorForSharing");
    CHECK_NAME(kCGImageDestinationPreserveGainMap, "kCGImageDestinationPreserveGainMap");
    CHECK_NAME(kCGImageAuxiliaryDataTypeDepth, "kCGImageAuxiliaryDataTypeDepth");
    CHECK_NAME(kCGImageAuxiliaryDataTypeDisparity, "kCGImageAuxiliaryDataTypeDisparity");
    CHECK_NAME(kCGImageAuxiliaryDataTypeHDRGainMap, "kCGImageAuxiliaryDataTypeHDRGainMap");
    CHECK_NAME(kCGImageAuxiliaryDataTypeISOGainMap, "kCGImageAuxiliaryDataTypeISOGainMap");
    CHECK_NAME(kCGImageAuxiliaryDataInfoData, "kCGImageAuxiliaryDataInfoData");
    CHECK_NAME(kCGImageAuxiliaryDataInfoDataDescription, "kCGImageAuxiliaryDataInfoDataDescription");
    // not read on macOS: the TIFF and Exif tag names, and the WebP dictionary
    // named like the other format dictionaries
    CHECK_NAME(kCGImagePropertyTIFFResolutionUnit, "ResolutionUnit");
    CHECK_NAME(kCGImagePropertyExifPixelXDimension, "PixelXDimension");
    CHECK_NAME(kCGImagePropertyExifPixelYDimension, "PixelYDimension");
    CHECK_NAME(kCGImagePropertyWebPDictionary, "{WebP}");
    if (failures == 0)
        printf("PASS: ImageIO names\n");
    return failures ? 1 : 0;
}
