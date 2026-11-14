// Copyright (C) 2026 Lubos Dolezel
//
// QTKit exports its NSString constants and the QTTime constants as data
// symbols; applications import them (for example QTMovieDidEndNotification),
// so a missing one stops the program from loading. The expected values are
// what macOS reports. Works for i386 and x86_64:
//   guest-cc.sh -a i386 -o qtkit-constants tests/qtkit-constants.m -framework Foundation -framework QTKit
// Exits non-zero on failure.
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void checkString(const char *name, NSString *value, NSString *expected) {
	if (!value || ![value isEqualToString:expected]) {
		printf("FAIL: %s is '%s', expected '%s'\n", name, [value UTF8String] ?: "(null)", [expected UTF8String]);
		failures++;
	}
}

#define CHECK(sym, expected) checkString(#sym, sym, expected)

static void checkTime(const char *name, QTTime t, long flags) {
	if (t.timeValue != 0 || t.timeScale != 1 || t.flags != flags) {
		printf("FAIL: %s is {%lld, %ld, %ld}, expected {0, 1, %ld}\n", name, t.timeValue, t.timeScale, t.flags, flags);
		failures++;
	}
}

int main(void) {
	@autoreleasepool {
		CHECK(QTMovieDidEndNotification, @"QTMovieDidEndNotification");
		CHECK(QTMovieRateDidChangeNotification, @"QTMovieRateDidChangeNotification");
		CHECK(QTMovieLoadStateDidChangeNotification, @"QTMovieLoadStateDidChangeNotification");
		CHECK(QTMovieTimeDidChangeNotification, @"QTMovieTimeDidChangeNotification");
		CHECK(QTMovieVolumeDidChangeNotification, @"QTMovieVolumeDidChangeNotification");
		CHECK(QTMovieURLAttribute, @"QTMovieURLAttribute");
		CHECK(QTMovieOpenAsyncOKAttribute, @"QTMovieOpenAsyncOKAttribute");
		CHECK(QTMovieDurationAttribute, @"QTMovieDurationAttribute");
		CHECK(QTMovieFrameImageType, @"QTMovieFrameImageType");
		CHECK(QTMovieModernizerOutputFormat_H264, @"OutputFormat_H264");
		CHECK(QTMovieViewMovieBinding, @"movie");
		CHECK(QTTrackEnabledAttribute, @"QTTrackEnabledAttribute");
		CHECK(QTTrackMediaTypeAttribute, @"QTTrackMediaTypeAttribute");
		CHECK(QTMediaTypeVideo, @"vide");
		CHECK(QTMediaTypeSound, @"soun");
		CHECK(QTMediaTypeBase, @"gnrc");
		CHECK(QTMediaDurationAttribute, @"QTMediaDurationAttribute");
		CHECK(QTKitErrorDomain, @"QTKitErrorDomain");
		CHECK(QTErrorTimeKey, @"QTErrorTimeKey");
		CHECK(QTCaptureAnyMediaType, @"QTCaptureAnyMediaType");
		CHECK(QTCaptureDeviceWasConnectedNotification, @"QTCaptureDeviceWasConnectedNotification");
		CHECK(QTCaptureSessionRuntimeErrorNotification, @"QTCaptureSessionRuntimeErrorNotification");
		CHECK(QTExportOptionsAppleM4A, @"QTExportOptionsAppleM4A");
		CHECK(QTCommonMetadataTitle, @"title");
		CHECK(QTCommonMetadataArtist, @"artist");
		CHECK(QTMetadataKeySpaceCommon, @"comn");
		CHECK(QTDataReferenceTypeFile, @"alis");
		CHECK(QTSampleBufferHostTimeAttribute, @"hostTime");
		CHECK(QTFormatDescriptionVideoEncodedPixelsSizeAttribute, @"videoEncodedPixelsSize");
		CHECK(QTUISizeRegular, @"QTUISizeRegular");
		CHECK(QTCarbonMoviePasteboardType, @"CorePasteboardFlavorType 0x6D6F6F76");
		CHECK(QTMovieStdControllerType, @"std");

#if defined(__i386__)
		if (sizeof(QTTime) != 16) { printf("FAIL: sizeof(QTTime) is %zu\n", sizeof(QTTime)); failures++; }
		CHECK(QTMovieMovieStructureAvailableCallbackAttribute, @"QTMovieMovieStructureAvailableCallbackAttribute");
#else
		if (sizeof(QTTime) != 24) { printf("FAIL: sizeof(QTTime) is %zu\n", sizeof(QTTime)); failures++; }
		CHECK(QTKitServerDidDieNotification, @"QTKitServerDidDieNotification");
#endif
		checkTime("QTZeroTime", QTZeroTime, 0);
		checkTime("QTIndefiniteTime", QTIndefiniteTime, kQTTimeIsIndefinite);
	}
	if (failures == 0)
		printf("PASS: QTKit constants\n");
	return failures != 0;
}
