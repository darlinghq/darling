// Probe 0109: the numeric and string values behind the public ImageIO names for image metadata and thumbnail
// options, and how the public CGImageMetadata functions behave on a few simple cases.
#import "probe_common.h"
#import <ImageIO/ImageIO.h>
#import <CoreGraphics/CoreGraphics.h>
#include <string.h>

#define S(name) probe_line("0109.string", @ #name, PF(@"\"%@\"", (__bridge NSString *)(name)))
#define N(name) probe_line("0109.enum", @ #name, PF(@"%d", (int)(name)))

static NSString *d(id o) { return o == nil ? @"nil" : PF(@"%@", o); }

int main(void) {
    @autoreleasepool {
        probe_env("0109");
        N(kCGImageMetadataTypeInvalid);
        N(kCGImageMetadataTypeDefault);
        N(kCGImageMetadataTypeString);
        N(kCGImageMetadataTypeArrayUnordered);
        N(kCGImageMetadataTypeArrayOrdered);
        N(kCGImageMetadataTypeAlternateArray);
        N(kCGImageMetadataTypeAlternateText);
        N(kCGImageMetadataTypeStructure);

        S(kCGImageSourceCreateThumbnailFromImageIfAbsent);
        S(kCGImageSourceCreateThumbnailFromImageAlways);
        S(kCGImageSourceThumbnailMaxPixelSize);
        S(kCGImageSourceCreateThumbnailWithTransform);
        S(kCGImageDestinationMetadata);
        S(kCGImageDestinationMergeMetadata);
        S(kCGImageMetadataNamespaceExif);
        S(kCGImageMetadataNamespaceExifEX);
        S(kCGImageMetadataNamespaceExifAux);
        S(kCGImageMetadataNamespaceTIFF);
        S(kCGImageMetadataNamespaceDublinCore);
        S(kCGImageMetadataNamespaceIPTCCore);
        S(kCGImageMetadataNamespacePhotoshop);
        S(kCGImageMetadataNamespaceXMPBasic);
        S(kCGImageMetadataNamespaceXMPRights);
        S(kCGImageMetadataPrefixExif);
        S(kCGImageMetadataPrefixExifEX);
        S(kCGImageMetadataPrefixExifAux);
        S(kCGImageMetadataPrefixTIFF);
        S(kCGImageMetadataPrefixDublinCore);
        S(kCGImageMetadataPrefixIPTCCore);
        S(kCGImageMetadataPrefixPhotoshop);
        S(kCGImageMetadataPrefixXMPBasic);
        S(kCGImageMetadataPrefixXMPRights);

        // Tags: namespace and prefix handling, default type inference, values.
        CGImageMetadataTagRef t1 = CGImageMetadataTagCreate(kCGImageMetadataNamespaceExif, kCGImageMetadataPrefixExif,
                                                            CFSTR("DateTimeOriginal"), kCGImageMetadataTypeString, CFSTR("2020:01:02 03:04:05"));
        probe_line("0109.tag", @"create exif:DateTimeOriginal type String",
                   t1 ? PF(@"type=%d prefix=%@ ns=%@ value=%@", (int)CGImageMetadataTagGetType(t1),
                           (__bridge_transfer NSString *)CGImageMetadataTagCopyPrefix(t1),
                           (__bridge_transfer NSString *)CGImageMetadataTagCopyNamespace(t1),
                           (__bridge_transfer id)CGImageMetadataTagCopyValue(t1)) : @"NULL");
        CGImageMetadataTagRef t2 = CGImageMetadataTagCreate(kCGImageMetadataNamespaceExif, kCGImageMetadataPrefixExif,
                                                            CFSTR("Flash"), kCGImageMetadataTypeDefault, CFSTR("x"));
        probe_line("0109.tag", @"create with type Default and a string value", t2 ? PF(@"type=%d", (int)CGImageMetadataTagGetType(t2)) : @"NULL");
        CGImageMetadataTagRef t3 = CGImageMetadataTagCreate(kCGImageMetadataNamespaceDublinCore, kCGImageMetadataPrefixDublinCore,
                                                            CFSTR("creator"), kCGImageMetadataTypeDefault, (__bridge CFTypeRef)@[@"a", @"b"]);
        probe_line("0109.tag", @"create with type Default and an array value", t3 ? PF(@"type=%d", (int)CGImageMetadataTagGetType(t3)) : @"NULL");
        CGImageMetadataTagRef t4 = CGImageMetadataTagCreate(CFSTR("http://example.invalid/ns/"), CFSTR("zz"), CFSTR("k"),
                                                            kCGImageMetadataTypeString, CFSTR("v"));
        probe_line("0109.tag", @"create with an unregistered namespace and prefix zz", t4 ? @"non-NULL" : @"NULL");
        CGImageMetadataTagRef t5 = CGImageMetadataTagCreate(NULL, CFSTR("exif"), CFSTR("k"), kCGImageMetadataTypeString, CFSTR("v"));
        probe_line("0109.tag", @"create with NULL namespace, prefix exif", t5 ? @"non-NULL" : @"NULL");
        CGImageMetadataTagRef t6 = CGImageMetadataTagCreate(NULL, CFSTR("zz"), CFSTR("k"), kCGImageMetadataTypeString, CFSTR("v"));
        probe_line("0109.tag", @"create with NULL namespace, unregistered prefix zz", t6 ? @"non-NULL" : @"NULL");

        // Containers: order of tags, paths, structure tags.
        CGMutableImageMetadataRef m = CGImageMetadataCreateMutable();
        probe_line("0109.metadata", @"CopyTags of an empty metadata", CGImageMetadataCopyTags(m) ? @"non-NULL" : @"NULL");
        BOOL ok1 = CGImageMetadataSetTagWithPath(m, NULL, CFSTR("exif:DateTimeOriginal"), t1);
        BOOL ok2 = t3 ? CGImageMetadataSetTagWithPath(m, NULL, CFSTR("dc:creator"), t3) : NO;
        BOOL ok3 = CGImageMetadataSetValueWithPath(m, NULL, CFSTR("xmp:Rating"), CFSTR("3"));
        probe_line("0109.metadata", @"SetTagWithPath exif:DateTimeOriginal, SetTagWithPath dc:creator, SetValueWithPath xmp:Rating",
                   PF(@"%d %d %d", ok1, ok2, ok3));
        CFArrayRef tags = CGImageMetadataCopyTags(m);
        NSMutableArray *names = [NSMutableArray array];
        for (id t in (__bridge NSArray *)tags)
            [names addObject:PF(@"%@:%@", (__bridge_transfer NSString *)CGImageMetadataTagCopyPrefix((__bridge CGImageMetadataTagRef)t),
                                (__bridge_transfer NSString *)CGImageMetadataTagCopyName((__bridge CGImageMetadataTagRef)t))];
        probe_line("0109.metadata", @"order of CopyTags after the three sets", [names componentsJoinedByString:@" , "]);
        if (tags) CFRelease(tags);
        CFStringRef sv = CGImageMetadataCopyStringValueWithPath(m, NULL, CFSTR("dc:creator"));
        probe_line("0109.metadata", @"CopyStringValueWithPath dc:creator (an array tag)", d((__bridge_transfer id)sv));
        sv = CGImageMetadataCopyStringValueWithPath(m, NULL, CFSTR("xmp:Rating"));
        probe_line("0109.metadata", @"CopyStringValueWithPath xmp:Rating", d((__bridge_transfer id)sv));
        CGImageMetadataTagRef got = CGImageMetadataCopyTagWithPath(m, NULL, CFSTR("xmp:Rating"));
        probe_line("0109.metadata", @"type of the tag SetValueWithPath made for xmp:Rating", got ? PF(@"%d", (int)CGImageMetadataTagGetType(got)) : @"NULL");
        probe_line("0109.metadata", @"CopyTagWithPath missing path", CGImageMetadataCopyTagWithPath(m, NULL, CFSTR("xmp:Nope")) ? @"non-NULL" : @"NULL");
        probe_line("0109.metadata", @"RemoveTagWithPath xmp:Rating then again",
                   PF(@"%d %d", CGImageMetadataRemoveTagWithPath(m, NULL, CFSTR("xmp:Rating")),
                      CGImageMetadataRemoveTagWithPath(m, NULL, CFSTR("xmp:Rating"))));
        probe_line("0109.metadata", @"SetValueWithPath with an unregistered prefix",
                   PF(@"%d", CGImageMetadataSetValueWithPath(m, NULL, CFSTR("qq:Thing"), CFSTR("v"))));

        // Metadata read from a real image: write a tiny JPEG with EXIF and TIFF properties, read it back.
        CFMutableDataRef out = CFDataCreateMutable(NULL, 0);
        CGImageDestinationRef dst = CGImageDestinationCreateWithData(out, CFSTR("public.jpeg"), 1, NULL);
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef bc = CGBitmapContextCreate(NULL, 40, 20, 8, 0, cs, kCGImageAlphaPremultipliedLast);
        CGContextSetRGBFillColor(bc, 0, 0.5, 1, 1);
        CGContextFillRect(bc, CGRectMake(0, 0, 40, 20));
        CGImageRef img = CGBitmapContextCreateImage(bc);
        NSDictionary *props = @{(__bridge id)kCGImagePropertyOrientation: @6,
                                (__bridge id)kCGImagePropertyExifDictionary: @{(__bridge id)kCGImagePropertyExifDateTimeOriginal: @"2020:01:02 03:04:05"},
                                (__bridge id)kCGImagePropertyTIFFDictionary: @{(__bridge id)kCGImagePropertyTIFFMake: @"ProbeMake"}};
        CGImageDestinationAddImage(dst, img, (__bridge CFDictionaryRef)props);
        probe_line("0109.write", @"JPEG 40x20 with orientation 6, an EXIF date and a TIFF make", PF(@"finalize=%d bytes>0=%d", CGImageDestinationFinalize(dst), CFDataGetLength(out) > 0));
        CGImageSourceRef src = CGImageSourceCreateWithData(out, NULL);
        CGImageMetadataRef md = src ? CGImageSourceCopyMetadataAtIndex(src, 0, NULL) : NULL;
        probe_line("0109.read", @"CGImageSourceCopyMetadataAtIndex", md ? @"non-NULL" : @"NULL");
        if (md) {
            CFArrayRef rt = CGImageMetadataCopyTags(md);
            NSMutableArray *paths = [NSMutableArray array];
            for (id t in (__bridge NSArray *)rt)
                [paths addObject:PF(@"%@:%@(type %d)", (__bridge_transfer NSString *)CGImageMetadataTagCopyPrefix((__bridge CGImageMetadataTagRef)t),
                                    (__bridge_transfer NSString *)CGImageMetadataTagCopyName((__bridge CGImageMetadataTagRef)t),
                                    (int)CGImageMetadataTagGetType((__bridge CGImageMetadataTagRef)t))];
            probe_line("0109.read", @"top-level tags (prefix:name(type))", [paths componentsJoinedByString:@" , "]);
            CFStringRef dt = CGImageMetadataCopyStringValueWithPath(md, NULL, CFSTR("exif:DateTimeOriginal"));
            probe_line("0109.read", @"exif:DateTimeOriginal", d((__bridge_transfer id)dt));
            CFStringRef mk = CGImageMetadataCopyStringValueWithPath(md, NULL, CFSTR("tiff:Make"));
            probe_line("0109.read", @"tiff:Make", d((__bridge_transfer id)mk));
            CFStringRef orn = CGImageMetadataCopyStringValueWithPath(md, NULL, CFSTR("tiff:Orientation"));
            probe_line("0109.read", @"tiff:Orientation", d((__bridge_transfer id)orn));
            if (rt) CFRelease(rt);
            CFRelease(md);
        }
        // Thumbnails: size rules and the transform option.
        if (src) {
            NSDictionary *o1 = @{(__bridge id)kCGImageSourceCreateThumbnailFromImageAlways: @YES,
                                 (__bridge id)kCGImageSourceThumbnailMaxPixelSize: @10};
            CGImageRef th = CGImageSourceCreateThumbnailAtIndex(src, 0, (__bridge CFDictionaryRef)o1);
            probe_line("0109.thumbnail", @"FromImageAlways, MaxPixelSize 10, no transform", th ? PF(@"%zux%zu", CGImageGetWidth(th), CGImageGetHeight(th)) : @"NULL");
            if (th) CGImageRelease(th);
            NSDictionary *o2 = @{(__bridge id)kCGImageSourceCreateThumbnailFromImageAlways: @YES,
                                 (__bridge id)kCGImageSourceThumbnailMaxPixelSize: @10,
                                 (__bridge id)kCGImageSourceCreateThumbnailWithTransform: @YES};
            th = CGImageSourceCreateThumbnailAtIndex(src, 0, (__bridge CFDictionaryRef)o2);
            probe_line("0109.thumbnail", @"FromImageAlways, MaxPixelSize 10, WithTransform (orientation 6)", th ? PF(@"%zux%zu", CGImageGetWidth(th), CGImageGetHeight(th)) : @"NULL");
            if (th) CGImageRelease(th);
            NSDictionary *o3 = @{(__bridge id)kCGImageSourceCreateThumbnailFromImageAlways: @YES,
                                 (__bridge id)kCGImageSourceThumbnailMaxPixelSize: @100};
            th = CGImageSourceCreateThumbnailAtIndex(src, 0, (__bridge CFDictionaryRef)o3);
            probe_line("0109.thumbnail", @"FromImageAlways, MaxPixelSize 100 (larger than the image)", th ? PF(@"%zux%zu", CGImageGetWidth(th), CGImageGetHeight(th)) : @"NULL");
            if (th) CGImageRelease(th);
            th = CGImageSourceCreateThumbnailAtIndex(src, 0, NULL);
            probe_line("0109.thumbnail", @"options NULL", th ? PF(@"%zux%zu", CGImageGetWidth(th), CGImageGetHeight(th)) : @"NULL");
            if (th) CGImageRelease(th);
        }
    }
    return 0;
}
