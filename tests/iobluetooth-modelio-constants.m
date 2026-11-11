// Copyright (C) 2026 Lubos Dolezel
//
// IOBluetooth OBEX file transfer dictionary keys and ModelIO default vertex
// attribute names. The expected strings are the values macOS reports.
//   clang -framework Foundation -framework IOBluetooth -framework ModelIO tests/iobluetooth-modelio-constants.m -o /tmp/iobluetooth-modelio-constants
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <IOBluetooth/OBEXFileTransferServices.h>
#import <ModelIO/MDLVertexDescriptor.h>
#include <stdio.h>

static int failures;

static void checkString(const char *name, NSString *value, NSString *expected) {
	if (!value || ![value isEqualToString:expected]) {
		printf("FAIL: %s is '%s', expected '%s'\n", name, [value UTF8String] ?: "(null)", [expected UTF8String]);
		failures++;
	}
}

#define CHECK(sym, expected) checkString(#sym, sym, expected)

int main(void) {
	@autoreleasepool {
		CHECK(kFTSListingNameKey, @"name");
		CHECK(kFTSListingTypeKey, @"file-type");
		CHECK(kFTSListingSizeKey, @"size");
		CHECK(kFTSProgressBytesTransferredKey, @"kFTSProgressBytesTransferredKey");
		CHECK(kFTSProgressBytesTotalKey, @"kFTSProgressBytesTotalKey");
		CHECK(kFTSProgressPercentageKey, @"kFTSProgressPercentageKey");
		CHECK(kFTSProgressPrecentageKey, @"kFTSProgressPrecentageKey");
		CHECK(kFTSProgressEstimatedTimeKey, @"kFTSProgressEstimatedTimeKey");
		CHECK(kFTSProgressTimeElapsedKey, @"kFTSProgressTimeElapsedKey");
		CHECK(kFTSProgressTransferRateKey, @"kFTSProgressTransferRateKey");

		CHECK(MDLVertexAttributePosition, @"position");
		CHECK(MDLVertexAttributeNormal, @"normal");
		CHECK(MDLVertexAttributeTangent, @"tangent");
		CHECK(MDLVertexAttributeBitangent, @"bitangent");
		CHECK(MDLVertexAttributeTextureCoordinate, @"textureCoordinate");
	}
	if (failures == 0)
		printf("PASS: OBEX and ModelIO constants\n");
	return failures != 0;
}
