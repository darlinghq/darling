/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Where Foundation's error-domain and error-key constants (NSError.h), its exception
// names and the other NS* data symbols that macOS defines in Foundation live, and
// where the NSStringTransform* constants and NSThumbnail1024x1024SizeKey live
// (CoreFoundation).  Binaries built for macOS bind each of these to the image that
// defines it there, so a different defining image on Darling means a program built on
// one system fails to start on the other.  The expectations were observed with dladdr
// and dlsym on macOS 26.7.3 and 10.14.4 (x86_64).
//
// Each name is checked in two ways: through a direct reference, which the linker binds
// to the defining image, and through dlsym.  The expected values are the ones macOS
// has: CoreFoundation's kCFErrorDomainPOSIX and friends are separate objects from the
// Foundation constants; the NSStringTransform* constants are the kCFStringTransform*
// objects themselves.

#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static const char *image_base(const void *p)
{
	Dl_info info;
	if (!p || !dladdr(p, &info) || !info.dli_fname)
		return "(none)";
	const char *slash = strrchr(info.dli_fname, '/');
	return slash ? slash + 1 : info.dli_fname;
}

static void expect_image(const char *name, const void *direct, const char *want)
{
	const char *got = image_base(direct);
	CHECK(strcmp(got, want) == 0, "&%s is defined in %s, expected %s", name, got, want);
}

static void expect_dlsym(void *foundation, void *corefoundation, const char *name, const char *want)
{
	void *viaFoundation = dlsym(foundation, name);
	void *viaCF = dlsym(corefoundation, name);
	CHECK(viaFoundation != NULL, "dlsym(Foundation, %s) is NULL", name);
	CHECK(strcmp(image_base(viaFoundation), want) == 0, "dlsym(Foundation, %s) is in %s, expected %s",
		name, image_base(viaFoundation), want);
	if (strcmp(want, "Foundation") == 0)
		CHECK(viaCF == NULL, "CoreFoundation exports %s (macOS exports it from Foundation only)", name);
	else
		CHECK(viaCF == viaFoundation, "CoreFoundation and Foundation resolve %s to different addresses", name);
}

int main(void)
{
	@autoreleasepool {
		void *foundation = dlopen("/System/Library/Frameworks/Foundation.framework/Foundation", RTLD_NOW);
		void *corefoundation = dlopen("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation", RTLD_NOW);
		if (!foundation || !corefoundation) {
			printf("FAIL: cannot open Foundation or CoreFoundation\n");
			return 1;
		}

		// Defined by Foundation.
		struct { const char *name; const void *direct; } inFoundation[] = {
			{ "NSCocoaErrorDomain", &NSCocoaErrorDomain },
			{ "NSPOSIXErrorDomain", &NSPOSIXErrorDomain },
			{ "NSOSStatusErrorDomain", &NSOSStatusErrorDomain },
			{ "NSMachErrorDomain", &NSMachErrorDomain },
			{ "NSUnderlyingErrorKey", &NSUnderlyingErrorKey },
			{ "NSLocalizedDescriptionKey", &NSLocalizedDescriptionKey },
			{ "NSLocalizedFailureReasonErrorKey", &NSLocalizedFailureReasonErrorKey },
			{ "NSLocalizedRecoverySuggestionErrorKey", &NSLocalizedRecoverySuggestionErrorKey },
			{ "NSLocalizedRecoveryOptionsErrorKey", &NSLocalizedRecoveryOptionsErrorKey },
			{ "NSRecoveryAttempterErrorKey", &NSRecoveryAttempterErrorKey },
			{ "NSHelpAnchorErrorKey", &NSHelpAnchorErrorKey },
			{ "NSStringEncodingErrorKey", &NSStringEncodingErrorKey },
			{ "NSURLErrorKey", &NSURLErrorKey },
			{ "NSFilePathErrorKey", &NSFilePathErrorKey },
			{ "NSDebugDescriptionErrorKey", &NSDebugDescriptionErrorKey },
			{ "NSLocalizedFailureErrorKey", &NSLocalizedFailureErrorKey },
			{ "NSCharacterConversionException", &NSCharacterConversionException },
			{ "NSDestinationInvalidException", &NSDestinationInvalidException },
			{ "NSFileHandleOperationException", &NSFileHandleOperationException },
			{ "NSInvalidReceivePortException", &NSInvalidReceivePortException },
			{ "NSInvalidSendPortException", &NSInvalidSendPortException },
			{ "NSObjectInaccessibleException", &NSObjectInaccessibleException },
			{ "NSObjectNotAvailableException", &NSObjectNotAvailableException },
			{ "NSPortReceiveException", &NSPortReceiveException },
			{ "NSPortSendException", &NSPortSendException },
			{ "NSPortTimeoutException", &NSPortTimeoutException },
			{ "NSFoundationVersionNumber", &NSFoundationVersionNumber },
			{ "NSLocaleLanguageConfidenceHigh", &NSLocaleLanguageConfidenceHigh },
			{ "NSLocaleLanguageConfidenceMedium", &NSLocaleLanguageConfidenceMedium },
			{ "NSStreamNetworkServiceType", &NSStreamNetworkServiceType },
			{ "NSStreamNetworkServiceTypeBackground", &NSStreamNetworkServiceTypeBackground },
			{ "NSStreamNetworkServiceTypeVideo", &NSStreamNetworkServiceTypeVideo },
			{ "NSStreamNetworkServiceTypeVoice", &NSStreamNetworkServiceTypeVoice },
			{ "NSStreamNetworkServiceTypeVoIP", &NSStreamNetworkServiceTypeVoIP },
			{ "NSStreamSocketSSLErrorDomain", &NSStreamSocketSSLErrorDomain },
			{ "NSStreamSOCKSErrorDomain", &NSStreamSOCKSErrorDomain },
			{ "NSURLErrorFailingURLErrorKey", &NSURLErrorFailingURLErrorKey },
			{ "NSURLErrorFailingURLPeerTrustErrorKey", &NSURLErrorFailingURLPeerTrustErrorKey },
			{ "NSURLErrorFailingURLStringErrorKey", &NSURLErrorFailingURLStringErrorKey },
			{ "NSURLFileScheme", &NSURLFileScheme },
		};
		for (unsigned i = 0; i < sizeof inFoundation / sizeof *inFoundation; i++) {
			expect_image(inFoundation[i].name, inFoundation[i].direct, "Foundation");
			expect_dlsym(foundation, corefoundation, inFoundation[i].name, "Foundation");
		}

		// Defined by CoreFoundation (and visible through Foundation, which re-exports it).
		struct { const char *name; const void *direct; } inCoreFoundation[] = {
			{ "NSThumbnail1024x1024SizeKey", &NSThumbnail1024x1024SizeKey },
			{ "NSDefaultRunLoopMode", &NSDefaultRunLoopMode },
			{ "NSRunLoopCommonModes", &NSRunLoopCommonModes },
			{ "NSGenericException", &NSGenericException },
			{ "NSCalendarIdentifierGregorian", &NSCalendarIdentifierGregorian },
			{ "NSStringTransformLatinToKatakana", &NSStringTransformLatinToKatakana },
			{ "NSStringTransformLatinToHiragana", &NSStringTransformLatinToHiragana },
			{ "NSStringTransformLatinToHangul", &NSStringTransformLatinToHangul },
			{ "NSStringTransformLatinToArabic", &NSStringTransformLatinToArabic },
			{ "NSStringTransformLatinToHebrew", &NSStringTransformLatinToHebrew },
			{ "NSStringTransformLatinToThai", &NSStringTransformLatinToThai },
			{ "NSStringTransformLatinToCyrillic", &NSStringTransformLatinToCyrillic },
			{ "NSStringTransformLatinToGreek", &NSStringTransformLatinToGreek },
			{ "NSStringTransformToLatin", &NSStringTransformToLatin },
			{ "NSStringTransformMandarinToLatin", &NSStringTransformMandarinToLatin },
			{ "NSStringTransformHiraganaToKatakana", &NSStringTransformHiraganaToKatakana },
			{ "NSStringTransformFullwidthToHalfwidth", &NSStringTransformFullwidthToHalfwidth },
			{ "NSStringTransformToXMLHex", &NSStringTransformToXMLHex },
			{ "NSStringTransformToUnicodeName", &NSStringTransformToUnicodeName },
			{ "NSStringTransformStripCombiningMarks", &NSStringTransformStripCombiningMarks },
			{ "NSStringTransformStripDiacritics", &NSStringTransformStripDiacritics },
		};
		for (unsigned i = 0; i < sizeof inCoreFoundation / sizeof *inCoreFoundation; i++) {
			expect_image(inCoreFoundation[i].name, inCoreFoundation[i].direct, "CoreFoundation");
			expect_dlsym(foundation, corefoundation, inCoreFoundation[i].name, "CoreFoundation");
		}

		// NSStringTransform* are the kCFStringTransform* objects.
		struct { NSString *ns; CFStringRef cf; } transforms[] = {
			{ NSStringTransformLatinToKatakana, kCFStringTransformLatinKatakana },
			{ NSStringTransformLatinToHiragana, kCFStringTransformLatinHiragana },
			{ NSStringTransformLatinToHangul, kCFStringTransformLatinHangul },
			{ NSStringTransformLatinToArabic, kCFStringTransformLatinArabic },
			{ NSStringTransformLatinToHebrew, kCFStringTransformLatinHebrew },
			{ NSStringTransformLatinToThai, kCFStringTransformLatinThai },
			{ NSStringTransformLatinToCyrillic, kCFStringTransformLatinCyrillic },
			{ NSStringTransformLatinToGreek, kCFStringTransformLatinGreek },
			{ NSStringTransformToLatin, kCFStringTransformToLatin },
			{ NSStringTransformMandarinToLatin, kCFStringTransformMandarinLatin },
			{ NSStringTransformHiraganaToKatakana, kCFStringTransformHiraganaKatakana },
			{ NSStringTransformFullwidthToHalfwidth, kCFStringTransformFullwidthHalfwidth },
			{ NSStringTransformToXMLHex, kCFStringTransformToXMLHex },
			{ NSStringTransformToUnicodeName, kCFStringTransformToUnicodeName },
			{ NSStringTransformStripCombiningMarks, kCFStringTransformStripCombiningMarks },
			{ NSStringTransformStripDiacritics, kCFStringTransformStripDiacritics },
		};
		for (unsigned i = 0; i < sizeof transforms / sizeof *transforms; i++)
			CHECK((CFStringRef)transforms[i].ns == transforms[i].cf, "NSStringTransform %u is not the kCFStringTransform object", i);

		// The NSError domain and key strings equal CoreFoundation's, but are distinct objects.
		CHECK([NSPOSIXErrorDomain isEqual:(NSString *)kCFErrorDomainPOSIX], "NSPOSIXErrorDomain != kCFErrorDomainPOSIX");
		CHECK([NSCocoaErrorDomain isEqual:(NSString *)kCFErrorDomainCocoa], "NSCocoaErrorDomain != kCFErrorDomainCocoa");
		CHECK([NSOSStatusErrorDomain isEqual:(NSString *)kCFErrorDomainOSStatus], "NSOSStatusErrorDomain != kCFErrorDomainOSStatus");
		CHECK([NSMachErrorDomain isEqual:(NSString *)kCFErrorDomainMach], "NSMachErrorDomain != kCFErrorDomainMach");
		CHECK([NSUnderlyingErrorKey isEqual:(NSString *)kCFErrorUnderlyingErrorKey], "NSUnderlyingErrorKey != kCFErrorUnderlyingErrorKey");
		CHECK([NSLocalizedDescriptionKey isEqual:(NSString *)kCFErrorLocalizedDescriptionKey], "NSLocalizedDescriptionKey != kCFErrorLocalizedDescriptionKey");
		CHECK([NSFilePathErrorKey isEqual:(NSString *)kCFErrorFilePathKey], "NSFilePathErrorKey != kCFErrorFilePathKey");
		CHECK([NSURLErrorKey isEqual:(NSString *)kCFErrorURLKey], "NSURLErrorKey != kCFErrorURLKey");
		CHECK((CFStringRef)NSPOSIXErrorDomain != kCFErrorDomainPOSIX, "NSPOSIXErrorDomain is the kCFErrorDomainPOSIX object (distinct objects on macOS)");
		CHECK((CFStringRef)NSUnderlyingErrorKey != kCFErrorUnderlyingErrorKey, "NSUnderlyingErrorKey is the kCFErrorUnderlyingErrorKey object (distinct objects on macOS)");

		// NSError itself is looked up by name: its class symbol is not part of this check.
		Class errorClass = NSClassFromString(@"NSError");

		// An NSError and a CFError describe the same error through either layer's constants.
		NSError *inner = [errorClass errorWithDomain:NSPOSIXErrorDomain code:2 userInfo:nil];
		NSError *outer = [errorClass errorWithDomain:NSCocoaErrorDomain code:260
			userInfo:@{ NSUnderlyingErrorKey: inner, NSFilePathErrorKey: @"/nonexistent", NSLocalizedDescriptionKey: @"described" }];
		CFDictionaryRef info = CFErrorCopyUserInfo((CFErrorRef)outer);
		CHECK(CFErrorGetCode((CFErrorRef)outer) == 260, "CFErrorGetCode of an NSError");
		CHECK([(NSString *)CFErrorGetDomain((CFErrorRef)outer) isEqual:NSCocoaErrorDomain], "CFErrorGetDomain of an NSError");
		CHECK(info && CFDictionaryGetValue(info, kCFErrorUnderlyingErrorKey) == (const void *)inner, "kCFErrorUnderlyingErrorKey finds the NSUnderlyingErrorKey value");
		CHECK(info && [(NSString *)CFDictionaryGetValue(info, kCFErrorFilePathKey) isEqual:@"/nonexistent"], "kCFErrorFilePathKey finds the NSFilePathErrorKey value");
		if (info)
			CFRelease(info);
		CHECK([[outer localizedDescription] isEqual:@"described"], "localizedDescription reads NSLocalizedDescriptionKey");

		CFErrorRef cfe = CFErrorCreate(NULL, kCFErrorDomainPOSIX, 13, NULL);
		NSError *bridged = [(NSError *)cfe autorelease];
		CHECK([[bridged domain] isEqual:NSPOSIXErrorDomain] && [bridged code] == 13, "a CFError read as an NSError has the NSPOSIXErrorDomain domain");
	}
	if (failures)
		printf("%d failure(s)\n", failures);
	else
		printf("PASS\n");
	return failures != 0;
}
