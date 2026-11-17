/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// VideoToolbox's session declarations from C: the error codes, the decode and encode flag values,
// and the documented failures when no codec exists for a fourcc (no decoder: kVTCouldNotFindVideoDecoderErr,
// no encoder: kVTCouldNotFindVideoEncoderErr, with a NULL session returned). The expected values
// were observed on macOS 10.14.4 and 26.7.3. Prints PASS when everything holds; the exit code is
// the number of failed checks.

#include <VideoToolbox/VideoToolbox.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { failures++; printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

#define UNKNOWN_CODEC 0x78787878 /* 'xxxx' */

static void outputCallback(void *refCon, void *frameRefCon, OSStatus status, VTDecodeInfoFlags flags,
	CVImageBufferRef image, CMTime pts, CMTime duration)
{
}

static void encodeCallback(void *refCon, void *frameRefCon, OSStatus status, VTEncodeInfoFlags flags,
	CMSampleBufferRef sampleBuffer)
{
}

int main(void)
{
	CHECK(kVTInvalidSessionErr == -12903);
	CHECK(kVTCouldNotFindVideoDecoderErr == -12906);
	CHECK(kVTCouldNotFindVideoEncoderErr == -12908);

	CHECK(kVTDecodeFrame_EnableAsynchronousDecompression == 1);
	CHECK(kVTDecodeFrame_DoNotOutputFrame == 2);
	CHECK(kVTDecodeFrame_1xRealTimePlayback == 4);
	CHECK(kVTDecodeFrame_EnableTemporalProcessing == 8);
	CHECK(kVTDecodeInfo_Asynchronous == 1 && kVTDecodeInfo_FrameDropped == 2 && kVTDecodeInfo_ImageBufferModifiable == 4);
	CHECK(kVTEncodeInfo_Asynchronous == 1 && kVTEncodeInfo_FrameDropped == 2);

	CMVideoFormatDescriptionRef format = NULL;
	CHECK(CMVideoFormatDescriptionCreate(NULL, UNKNOWN_CODEC, 64, 32, NULL, &format) == noErr);

	VTDecompressionOutputCallbackRecord record = { outputCallback, NULL };
	VTDecompressionSessionRef decoder = (VTDecompressionSessionRef)1;
	CHECK(VTDecompressionSessionCreate(NULL, format, NULL, NULL, &record, &decoder) == kVTCouldNotFindVideoDecoderErr);
	CHECK(decoder == NULL);

	VTCompressionSessionRef encoder = (VTCompressionSessionRef)1;
	CHECK(VTCompressionSessionCreate(NULL, 64, 32, UNKNOWN_CODEC, NULL, NULL, NULL, encodeCallback, NULL, &encoder)
		== kVTCouldNotFindVideoEncoderErr);
	CHECK(encoder == NULL);

	CHECK(!VTIsHardwareDecodeSupported(UNKNOWN_CODEC));

	CFRelease(format);
	if (failures == 0)
		printf("PASS\n");
	return failures;
}
