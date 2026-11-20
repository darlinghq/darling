// Copyright (C) 2026 Lubos Dolezel
//
// The system's declarations of common audio types. Values observed black-box on macOS 26.7.3 (x86_64) and macOS
// 10.14.4: public.mp3 (mp3), org.xiph.flac (flac, audio/flac), com.apple.m4a-audio (m4a, audio/x-m4a; conforms to
// public.mpeg-4-audio, which keeps audio/mp4), com.microsoft.waveform-audio and public.aiff-audio are declared, and
// every one conforms to public.audio, public.audiovisual-content, public.data and public.item. The Ogg audio type
// org.xiph.ogg-audio (ogg, oga, opus, audio/ogg) exists on macOS 26.7.3 only; macOS 10.14.4 gives a dyn.* identifier
// for ogg, so the "ogg" checks are expected to fail there.
#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)
#define CHECK_STR(got, want) do { NSString *g_ = (got); NSString *w_ = (want); \
    if (!(g_ == w_ || [g_ isEqualToString: w_])) { fprintf(stderr, "FAIL line %d: %s = %s, want %s\n", __LINE__, #got, \
        g_ ? [g_ UTF8String] : "(nil)", w_ ? [w_ UTF8String] : "(nil)"); failures++; } } while (0)

static NSString *TypeForTag(CFStringRef tagClass, NSString *tag) {
    return [(NSString *) UTTypeCreatePreferredIdentifierForTag(tagClass, (CFStringRef) tag, NULL) autorelease];
}

static void CheckAudioType(NSString *ext, NSString *type, NSString *description, NSString *conformsDirectly) {
    CHECK_STR(TypeForTag(kUTTagClassFilenameExtension, ext), type);
    NSDictionary *declaration = [(NSDictionary *) UTTypeCopyDeclaration((CFStringRef) type) autorelease];
    CHECK(declaration != nil);
    CHECK_STR([declaration objectForKey: (NSString *) kUTTypeIdentifierKey], type);
    CHECK_STR([declaration objectForKey: (NSString *) kUTTypeDescriptionKey], description);
    id conforms = [declaration objectForKey: (NSString *) kUTTypeConformsToKey];
    NSArray *list = [conforms isKindOfClass: [NSArray class]] ? conforms : (conforms ? @[conforms] : @[]);
    CHECK([list containsObject: conformsDirectly]);
    for (NSString *parent in @[@"public.audio", @"public.audiovisual-content", @"public.data", @"public.item"])
        CHECK(UTTypeConformsTo((CFStringRef) type, (CFStringRef) parent));
    CHECK(!UTTypeConformsTo((CFStringRef) type, CFSTR("public.image")));
}

int main(void) {
    @autoreleasepool {
        CheckAudioType(@"mp3", @"public.mp3", @"MP3 audio", @"public.audio");
        CheckAudioType(@"flac", @"org.xiph.flac", @"FLAC audio", @"public.audio");
        CheckAudioType(@"m4a", @"com.apple.m4a-audio", @"Apple MPEG-4 audio", @"public.mpeg-4-audio");
        CheckAudioType(@"wav", @"com.microsoft.waveform-audio", @"Waveform audio", @"public.audio");
        CheckAudioType(@"aiff", @"public.aiff-audio", @"AIFF audio", @"public.aifc-audio");

        // macOS 26.7.3 only (see the header)
        CheckAudioType(@"ogg", @"org.xiph.ogg-audio", @"Ogg Audio", @"public.audio");
        CHECK_STR(TypeForTag(kUTTagClassFilenameExtension, @"oga"), @"org.xiph.ogg-audio");
        CHECK_STR(TypeForTag(kUTTagClassFilenameExtension, @"opus"), @"org.xiph.ogg-audio");
        CHECK_STR(TypeForTag(kUTTagClassMIMEType, @"audio/ogg"), @"org.xiph.ogg-audio");

        CHECK_STR(TypeForTag(kUTTagClassMIMEType, @"audio/flac"), @"org.xiph.flac");
        CHECK_STR(TypeForTag(kUTTagClassMIMEType, @"audio/x-m4a"), @"com.apple.m4a-audio");
        CHECK_STR(TypeForTag(kUTTagClassMIMEType, @"audio/mp4"), @"public.mpeg-4-audio");
        CHECK(UTTypeConformsTo(CFSTR("com.apple.m4a-audio"), CFSTR("public.mpeg-4-audio")));
        CHECK(!UTTypeConformsTo(CFSTR("public.mpeg-4-audio"), CFSTR("com.apple.m4a-audio")));
        CHECK_STR([(NSString *) UTTypeCopyPreferredTagWithClass(CFSTR("org.xiph.flac"), kUTTagClassFilenameExtension) autorelease], @"flac");

        // a conformance filter still finds the audio type
        NSString *flac = [(NSString *) UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, CFSTR("flac"), CFSTR("public.audio")) autorelease];
        CHECK_STR(flac, @"org.xiph.flac");

        // a type that is not audio stays apart
        CHECK(![TypeForTag(kUTTagClassFilenameExtension, @"zzqqunknown") isEqualToString: @"org.xiph.flac"]);
    }
    if (failures == 0)
        printf("PASS\n");
    return failures != 0;
}
