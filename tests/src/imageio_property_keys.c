// Copyright (C) 2026 Darling developers
//
// ImageIO property keys that the documentation lists without their strings:
// kCGImagePropertyExifDateTimeDigitized, kCGImagePropertyIPTCDateCreated,
// kCGImagePropertyIPTCDigitalCreationDate, kCGImagePropertyRawDictionary and
// kCGImagePropertyTIFFDateTime. Each must be a non-NULL CFString, and the keys
// that share a dictionary must be pairwise distinct. The string values are not
// documented and are not pinned. Prints OK, or the failures with a non-zero
// exit status.
#include <ImageIO/ImageIO.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

#define IS_STRING(name) \
    CHECK((name) != NULL && CFGetTypeID(name) == CFStringGetTypeID(), #name " is not a CFString")

static void distinct(const char *group, const CFStringRef *keys, const char *const *names, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (keys[i] != NULL && keys[j] != NULL && CFEqual(keys[i], keys[j])) {
                printf("FAIL: %s and %s are equal in the %s group\n", names[i], names[j], group);
                failures++;
            }
        }
    }
}

int main(void) {
    IS_STRING(kCGImagePropertyExifDateTimeDigitized);
    IS_STRING(kCGImagePropertyIPTCDateCreated);
    IS_STRING(kCGImagePropertyIPTCDigitalCreationDate);
    IS_STRING(kCGImagePropertyRawDictionary);
    IS_STRING(kCGImagePropertyTIFFDateTime);

    const CFStringRef exif[] = {
        kCGImagePropertyExifDateTimeDigitized, kCGImagePropertyExifDateTimeOriginal,
        kCGImagePropertyExifOffsetTimeOriginal, kCGImagePropertyExifUserComment,
    };
    const char *const exifNames[] = {
        "kCGImagePropertyExifDateTimeDigitized", "kCGImagePropertyExifDateTimeOriginal",
        "kCGImagePropertyExifOffsetTimeOriginal", "kCGImagePropertyExifUserComment",
    };
    distinct("Exif", exif, exifNames, 4);

    const CFStringRef iptc[] = {
        kCGImagePropertyIPTCDateCreated, kCGImagePropertyIPTCDigitalCreationDate,
        kCGImagePropertyIPTCKeywords, kCGImagePropertyIPTCCity,
        kCGImagePropertyIPTCCredit, kCGImagePropertyIPTCSubLocation,
    };
    const char *const iptcNames[] = {
        "kCGImagePropertyIPTCDateCreated", "kCGImagePropertyIPTCDigitalCreationDate",
        "kCGImagePropertyIPTCKeywords", "kCGImagePropertyIPTCCity",
        "kCGImagePropertyIPTCCredit", "kCGImagePropertyIPTCSubLocation",
    };
    distinct("IPTC", iptc, iptcNames, 6);

    const CFStringRef tiff[] = {
        kCGImagePropertyTIFFDateTime, kCGImagePropertyTIFFArtist, kCGImagePropertyTIFFCopyright,
        kCGImagePropertyTIFFDocumentName, kCGImagePropertyTIFFImageDescription,
        kCGImagePropertyTIFFXResolution, kCGImagePropertyTIFFYResolution,
        kCGImagePropertyTIFFOrientation,
    };
    const char *const tiffNames[] = {
        "kCGImagePropertyTIFFDateTime", "kCGImagePropertyTIFFArtist", "kCGImagePropertyTIFFCopyright",
        "kCGImagePropertyTIFFDocumentName", "kCGImagePropertyTIFFImageDescription",
        "kCGImagePropertyTIFFXResolution", "kCGImagePropertyTIFFYResolution",
        "kCGImagePropertyTIFFOrientation",
    };
    distinct("TIFF", tiff, tiffNames, 8);

    const CFStringRef dictionaries[] = {
        kCGImagePropertyRawDictionary, kCGImagePropertyTIFFDictionary, kCGImagePropertyExifDictionary,
        kCGImagePropertyIPTCDictionary, kCGImagePropertyGPSDictionary, kCGImagePropertyPNGDictionary,
        kCGImagePropertyGIFDictionary, kCGImagePropertyHEICSDictionary, kCGImagePropertyMakerAppleDictionary,
    };
    const char *const dictionaryNames[] = {
        "kCGImagePropertyRawDictionary", "kCGImagePropertyTIFFDictionary", "kCGImagePropertyExifDictionary",
        "kCGImagePropertyIPTCDictionary", "kCGImagePropertyGPSDictionary", "kCGImagePropertyPNGDictionary",
        "kCGImagePropertyGIFDictionary", "kCGImagePropertyHEICSDictionary", "kCGImagePropertyMakerAppleDictionary",
    };
    distinct("dictionary", dictionaries, dictionaryNames, 9);

    if (failures == 0) {
        printf("OK\n");
    }
    return failures != 0;
}
