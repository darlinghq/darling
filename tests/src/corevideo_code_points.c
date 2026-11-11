// Copyright (C) 2026 Lubos Dolezel
//
// CoreVideo maps the integer code points of ITU-T H.273 to the string constants of
// CVImageBuffer.h and back. The expected values were captured on macOS 27.0.1.
//
// The quirk_ checks pin undocumented behaviour: a code point without a named constant
// maps to a synthesized "<Kind>#<n>" string, and a string that is not recognised maps
// to code point 2.
#include <CoreVideo/CoreVideo.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int is(CFStringRef value, const char* expected)
{
	if (value == NULL)
		return expected == NULL;
	if (expected == NULL)
		return 0;
	char buffer[128];
	return CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8) && strcmp(buffer, expected) == 0;
}

static void quirk_unnamed_code_point_gets_synthesized_string(void)
{
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(3), "ColorPrimaries#3"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(4), "ColorPrimaries#4"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(5), "TransferFunction#5"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(5), "YCbCrMatrix#5"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(31), "YCbCrMatrix#31"));

	// The same string object is returned each time
	CHECK(CVColorPrimariesGetStringForIntegerCodePoint(3) == CVColorPrimariesGetStringForIntegerCodePoint(3));
}

static void quirk_unrecognised_string_is_unspecified(void)
{
	CHECK(CVColorPrimariesGetIntegerCodePointForString(NULL) == 2);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(NULL) == 2);
	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(NULL) == 2);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(CFSTR("bogus")) == 2);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(CFSTR("bogus")) == 2);
	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(CFSTR("bogus")) == 2);

	// ITU_R_2020 as a transfer function maps back to 1
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_ITU_R_2020) == 1);
}

int main(void)
{
	CHECK(CVColorPrimariesGetStringForIntegerCodePoint(0) == NULL);
	CHECK(CVColorPrimariesGetStringForIntegerCodePoint(2) == NULL);
	CHECK(CVTransferFunctionGetStringForIntegerCodePoint(0) == NULL);
	CHECK(CVTransferFunctionGetStringForIntegerCodePoint(2) == NULL);
	CHECK(CVYCbCrMatrixGetStringForIntegerCodePoint(0) == NULL);
	CHECK(CVYCbCrMatrixGetStringForIntegerCodePoint(2) == NULL);

	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(1), "ITU_R_709_2"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(5), "EBU_3213"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(6), "SMPTE_C"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(9), "ITU_R_2020"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(11), "DCI_P3"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(12), "P3_D65"));
	CHECK(is(CVColorPrimariesGetStringForIntegerCodePoint(22), "P22"));

	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(1), "ITU_R_709_2"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(6), "ITU_R_709_2"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(14), "ITU_R_709_2"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(15), "ITU_R_709_2"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(7), "SMPTE_240M_1995"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(8), "Linear"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(13), "IEC_sRGB"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(16), "SMPTE_ST_2084_PQ"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(17), "SMPTE_ST_428_1"));
	CHECK(is(CVTransferFunctionGetStringForIntegerCodePoint(18), "ITU_R_2100_HLG"));

	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(1), "ITU_R_709_2"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(6), "ITU_R_601_4"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(7), "SMPTE_240M_1995"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(9), "ITU_R_2020"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(14), "ITU_R_2100_ICtCp"));
	CHECK(is(CVYCbCrMatrixGetStringForIntegerCodePoint(15), "IPT_C2"));

	// String to code point, for every public constant
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_ITU_R_709_2) == 1);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_EBU_3213) == 5);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_SMPTE_C) == 6);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_ITU_R_2020) == 9);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_DCI_P3) == 11);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_P3_D65) == 12);
	CHECK(CVColorPrimariesGetIntegerCodePointForString(kCVImageBufferColorPrimaries_P22) == 22);

	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_ITU_R_709_2) == 1);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_UseGamma) == 2);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_SMPTE_240M_1995) == 7);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_Linear) == 8);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_sRGB) == 13);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ) == 16);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_SMPTE_ST_428_1) == 17);
	CHECK(CVTransferFunctionGetIntegerCodePointForString(kCVImageBufferTransferFunction_ITU_R_2100_HLG) == 18);

	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(kCVImageBufferYCbCrMatrix_ITU_R_709_2) == 1);
	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(kCVImageBufferYCbCrMatrix_ITU_R_601_4) == 6);
	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(kCVImageBufferYCbCrMatrix_SMPTE_240M_1995) == 7);
	CHECK(CVYCbCrMatrixGetIntegerCodePointForString(kCVImageBufferYCbCrMatrix_ITU_R_2020) == 9);

	// Clean aperture keys
	CHECK(is(kCVImageBufferCleanApertureKey, "CVCleanAperture"));
	CHECK(is(kCVImageBufferCleanApertureWidthKey, "Width"));
	CHECK(is(kCVImageBufferCleanApertureHeightKey, "Height"));
	CHECK(is(kCVImageBufferCleanApertureHorizontalOffsetKey, "HorizontalOffset"));
	CHECK(is(kCVImageBufferCleanApertureVerticalOffsetKey, "VerticalOffset"));

	quirk_unnamed_code_point_gets_synthesized_string();
	quirk_unrecognised_string_is_unspecified();

	if (g_failures)
	{
		printf("%d failures\n", g_failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
