//  -framework foundation -framework corefoundation -framework coreservices
// http://www.cocoabuilder.com/archive/cocoa/144537-human-readable-locale-names.html

#import <Foundation/Foundation.h>
#import <CoreServices/CoreServices.h>
#include <stdio.h>

int main (int argc, const char * argv[]) {

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    //NSString *identifier = [[NSLocale currentLocale] localeIdentifier];
    //char localeString[12];
    //BOOL ok = [identifier getCString:localeString maxLength:12 encoding:NSASCIIStringEncoding];

    OSStatus status;
    LocaleRef locale;
    // status = LocaleRefFromLocaleString (localeString, &locale);
    status = LocaleRefFromLocaleString ("es_ES", &locale);

    LocaleRef displayLocale;
    // status = LocaleRefFromLocaleString(localeString, &displayLocale);
    status = LocaleRefFromLocaleString("es_ES", &displayLocale);

    UniCharCount maxNameLen = 256;
    UniCharCount actualNameLen;
    UniChar displayName[256];

    status = LocaleGetName (locale,
            0, // LocaleOperationVariant opVariant
            kLocaleAndVariantNameMask, // LocaleNameMask nameMask
            displayLocale,
            maxNameLen, // UniCharCount maxNameLen
            &actualNameLen, // UniCharCount * actualNameLen
            displayName); // UniChar displayName[]

    NSString *displayString =
        [NSString stringWithCharacters:displayName
length:actualNameLen];
    //NSLog(@"identifier: %@", identifier);
    //NSLog(@"localeString: %s", localeString);
    //NSLog(@"displayString: %@", displayString);
    puts([displayString UTF8String]);

    [pool release];
    return 0;
}

