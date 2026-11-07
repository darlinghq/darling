// Copyright (C) 2026 Lubos Dolezel
//
// Exported string and number constants of the media frameworks have the
// values macOS gives them. Apps compare these strings, store them in
// files and send them across processes, so a wrong value fails silently
// where a missing symbol would fail loudly.
//
// The expected values were captured on macOS 27.0.1 by printing each
// constant; this program passes unchanged there. Symbols are looked up
// with dlsym() so that a missing one is reported instead of failing the
// link.
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const char* const frameworks[] = {
	"/System/Library/Frameworks/CoreMedia.framework/CoreMedia",
	"/System/Library/Frameworks/CoreVideo.framework/CoreVideo",
	"/System/Library/Frameworks/VideoToolbox.framework/VideoToolbox",
	"/System/Library/Frameworks/Vision.framework/Vision",
	"/System/Library/Frameworks/ImageCaptureCore.framework/ImageCaptureCore",
	"/System/Library/Frameworks/MediaAccessibility.framework/MediaAccessibility",
	"/System/Library/Frameworks/CoreLocation.framework/CoreLocation",
};

struct string_constant
{
	const char* name;
	const char* value;
};

static const struct string_constant strings[] = {
	{ "kCMFormatDescriptionTransferFunction_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCMFormatDescriptionTransferFunction_SMPTE_240M_1995", "SMPTE_240M_1995" },
	{ "kCMFormatDescriptionTransferFunction_UseGamma", "UseGamma" },
	{ "kCMFormatDescriptionTransferFunction_ITU_R_2020", "ITU_R_2020" },
	{ "kCMFormatDescriptionTransferFunction_ITU_R_2100_HLG", "ITU_R_2100_HLG" },
	{ "kCMFormatDescriptionTransferFunction_SMPTE_ST_2084_PQ", "SMPTE_ST_2084_PQ" },
	{ "kCMFormatDescriptionTransferFunction_SMPTE_ST_428_1", "SMPTE_ST_428_1" },
	{ "kCMFormatDescriptionTransferFunction_Linear", "Linear" },
	{ "kCMFormatDescriptionTransferFunction_sRGB", "IEC_sRGB" },
	{ "kCMFormatDescriptionColorPrimaries_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCMFormatDescriptionColorPrimaries_EBU_3213", "EBU_3213" },
	{ "kCMFormatDescriptionColorPrimaries_SMPTE_C", "SMPTE_C" },
	{ "kCMFormatDescriptionColorPrimaries_ITU_R_2020", "ITU_R_2020" },
	{ "kCMFormatDescriptionColorPrimaries_P3_D65", "P3_D65" },
	{ "kCMFormatDescriptionColorPrimaries_DCI_P3", "DCI_P3" },
	{ "kCMFormatDescriptionExtension_ContentLightLevelInfo", "ContentLightLevelInfo" },
	{ "kCMFormatDescriptionExtension_Depth", "Depth" },
	{ "kCMFormatDescriptionExtension_FormatName", "FormatName" },
	{ "kCMFormatDescriptionExtension_FullRangeVideo", "FullRangeVideo" },
	{ "kCMFormatDescriptionExtension_MasteringDisplayColorVolume", "MasteringDisplayColorVolume" },
	{ "kCMSampleAttachmentKey_IsDependedOnByOthers", "IsDependedOnByOthers" },
	{ "kCMFormatDescriptionYCbCrMatrix_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCMFormatDescriptionYCbCrMatrix_ITU_R_601_4", "ITU_R_601_4" },
	{ "kCMFormatDescriptionYCbCrMatrix_SMPTE_240M_1995", "SMPTE_240M_1995" },
	{ "kCMFormatDescriptionYCbCrMatrix_ITU_R_2020", "ITU_R_2020" },
	{ "kCMSampleAttachmentKey_NotSync", "NotSync" },
	{ "kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms", "SampleDescriptionExtensionAtoms" },
	{ "kCVImageBufferTransferFunction_sRGB", "IEC_sRGB" },
	{ "kCVBufferPropagatedAttachmentsKey", "PropagatedAttachments" },
	{ "kCVImageBufferContentLightLevelInfoKey", "ContentLightLevelInfo" },
	{ "kCVImageBufferMasteringDisplayColorVolumeKey", "MasteringDisplayColorVolume" },
	{ "kCVPixelBufferPoolAllocationThresholdKey", "BufferPoolAllocationThreshold" },
	{ "kCVImageBufferColorPrimaries_P3_D65", "P3_D65" },
	{ "kCVImageBufferColorPrimaries_DCI_P3", "DCI_P3" },
	{ "kCVImageBufferColorPrimaries_ITU_R_2020", "ITU_R_2020" },
	{ "kCVImageBufferYCbCrMatrix_ITU_R_2020", "ITU_R_2020" },
	{ "kCVImageBufferCGColorSpaceKey", "CGColorSpace" },
	{ "kCVImageBufferChromaLocation_Bottom", "Bottom" },
	{ "kCVImageBufferChromaLocationBottomFieldKey", "CVImageBufferChromaLocationBottomField" },
	{ "kCVImageBufferChromaLocation_BottomLeft", "BottomLeft" },
	{ "kCVImageBufferChromaLocation_Center", "Center" },
	{ "kCVImageBufferChromaLocation_DV420", "DV 4:2:0" },
	{ "kCVImageBufferChromaLocation_Left", "Left" },
	{ "kCVImageBufferChromaLocation_Top", "Top" },
	{ "kCVImageBufferChromaLocationTopFieldKey", "CVImageBufferChromaLocationTopField" },
	{ "kCVImageBufferChromaLocation_TopLeft", "TopLeft" },
	{ "kCVImageBufferColorPrimaries_EBU_3213", "EBU_3213" },
	{ "kCVImageBufferColorPrimaries_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCVImageBufferColorPrimariesKey", "CVImageBufferColorPrimaries" },
	{ "kCVImageBufferColorPrimaries_SMPTE_C", "SMPTE_C" },
	{ "kCVImageBufferGammaLevelKey", "CVImageBufferGammaLevel" },
	{ "kCVImageBufferPixelAspectRatioHorizontalSpacingKey", "HorizontalSpacing" },
	{ "kCVImageBufferPixelAspectRatioKey", "CVPixelAspectRatio" },
	{ "kCVImageBufferPixelAspectRatioVerticalSpacingKey", "VerticalSpacing" },
	{ "kCVImageBufferTransferFunction_ITU_R_2020", "ITU_R_2020" },
	{ "kCVImageBufferTransferFunction_ITU_R_2100_HLG", "ITU_R_2100_HLG" },
	{ "kCVImageBufferTransferFunction_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCVImageBufferTransferFunctionKey", "CVImageBufferTransferFunction" },
	{ "kCVImageBufferTransferFunction_Linear", "Linear" },
	{ "kCVImageBufferTransferFunction_SMPTE_240M_1995", "SMPTE_240M_1995" },
	{ "kCVImageBufferTransferFunction_SMPTE_C", "SMPTE_C" },
	{ "kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ", "SMPTE_ST_2084_PQ" },
	{ "kCVImageBufferTransferFunction_SMPTE_ST_428_1", "SMPTE_ST_428_1" },
	{ "kCVImageBufferTransferFunction_UseGamma", "UseGamma" },
	{ "kCVImageBufferYCbCrMatrix_ITU_R_601_4", "ITU_R_601_4" },
	{ "kCVImageBufferYCbCrMatrix_ITU_R_709_2", "ITU_R_709_2" },
	{ "kCVImageBufferYCbCrMatrixKey", "CVImageBufferYCbCrMatrix" },
	{ "kCVImageBufferYCbCrMatrix_SMPTE_240M_1995", "SMPTE_240M_1995" },
	{ "kCVPixelBufferBytesPerRowAlignmentKey", "BytesPerRowAlignment" },
	{ "kCVPixelBufferHeightKey", "Height" },
	{ "kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey", "IOSurfaceOpenGLTextureCompatibility" },
	{ "kCVPixelBufferIOSurfacePropertiesKey", "IOSurfaceProperties" },
	{ "kCVPixelBufferMetalCompatibilityKey", "MetalCompatibility" },
	{ "kCVPixelBufferOpenGLCompatibilityKey", "OpenGLCompatibility" },
	{ "kCVPixelBufferPixelFormatTypeKey", "PixelFormatType" },
	{ "kCVPixelBufferPoolMaximumBufferAgeKey", "MaximumBufferAge" },
	{ "kCVPixelBufferPoolMinimumBufferCountKey", "MinimumBufferCount" },
	{ "kCVPixelBufferWidthKey", "Width" },
	{ "kVTCompressionPropertyKey_BaseLayerFrameRateFraction", "BaseLayerFrameRateFraction" },
	{ "kVTCompressionPropertyKey_ColorPrimaries", "ColorPrimaries" },
	{ "kVTCompressionPropertyKey_TransferFunction", "TransferFunction" },
	{ "kVTCompressionPropertyKey_YCbCrMatrix", "YCbCrMatrix" },
	{ "kVTVideoEncoderSpecification_EnableLowLatencyRateControl", "EnableLowLatencyRateControl" },
	{ "VNBarcodeSymbologyAztec", "VNBarcodeSymbologyAztec" },
	{ "VNBarcodeSymbologyCode128", "VNBarcodeSymbologyCode128" },
	{ "VNBarcodeSymbologyCode39", "VNBarcodeSymbologyCode39" },
	{ "VNBarcodeSymbologyCode39Checksum", "VNBarcodeSymbologyCode39Checksum" },
	{ "VNBarcodeSymbologyCode39FullASCII", "VNBarcodeSymbologyCode39FullASCII" },
	{ "VNBarcodeSymbologyCode39FullASCIIChecksum", "VNBarcodeSymbologyCode39FullASCIIChecksum" },
	{ "VNBarcodeSymbologyCode93", "VNBarcodeSymbologyCode93" },
	{ "VNBarcodeSymbologyCode93i", "VNBarcodeSymbologyCode93i" },
	{ "VNBarcodeSymbologyDataMatrix", "VNBarcodeSymbologyDataMatrix" },
	{ "VNBarcodeSymbologyEAN13", "VNBarcodeSymbologyEAN13" },
	{ "VNBarcodeSymbologyEAN8", "VNBarcodeSymbologyEAN8" },
	{ "VNBarcodeSymbologyI2of5", "VNBarcodeSymbologyI2of5" },
	{ "VNBarcodeSymbologyI2of5Checksum", "VNBarcodeSymbologyI2of5Checksum" },
	{ "VNBarcodeSymbologyITF14", "VNBarcodeSymbologyITF14" },
	{ "VNBarcodeSymbologyPDF417", "VNBarcodeSymbologyPDF417" },
	{ "VNBarcodeSymbologyQR", "VNBarcodeSymbologyQR" },
	{ "VNBarcodeSymbologyUPCE", "VNBarcodeSymbologyUPCE" },
	{ "ICOverwrite", "ICOverwrite" },
	{ "ICSaveAsFilename", "ICSaveAsFilename" },
	{ "ICSavedFilename", "ICSavedFilename" },
	{ "ICTransportTypeMassStorage", "ICTransportTypeMassStorage" },
	{ "ICDownloadsDirectoryURL", "ICDownloadsDirectoryURL" },
	{ "ICDownloadSidecarFiles", "ICDownloadSidecarFiles" },
	{ "kMACaptionAppearanceSettingsChangedNotification", "com.apple.mediaaccessibility.captionAppearanceSettingsChanged" },
};

struct number_constant
{
	const char* name;
	double value;
};

static const struct number_constant numbers[] = {
	{ "kCLLocationAccuracyBestForNavigation", -2 },
	{ "kCLLocationAccuracyBest", -1 },
	{ "kCLLocationAccuracyNearestTenMeters", 10 },
	{ "kCLLocationAccuracyHundredMeters", 100 },
	{ "kCLLocationAccuracyKilometer", 1000 },
	{ "kCLLocationAccuracyThreeKilometers", 3000 },
};

static int g_failures = 0;

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

static void check_string(const struct string_constant* c)
{
	const CFStringRef* sym = (const CFStringRef*) dlsym(RTLD_DEFAULT, c->name);
	if (sym == NULL)
	{
		printf("FAIL: %s is not exported\n", c->name);
		g_failures++;
		return;
	}
	if (*sym == NULL || CFGetTypeID(*sym) != CFStringGetTypeID())
	{
		printf("FAIL: %s is not a CFString\n", c->name);
		g_failures++;
		return;
	}

	char buf[256];
	if (!CFStringGetCString(*sym, buf, sizeof(buf), kCFStringEncodingUTF8))
		strcpy(buf, "(unconvertible)");
	if (strcmp(buf, c->value) != 0)
	{
		printf("FAIL: %s = \"%s\", expected \"%s\"\n", c->name, buf, c->value);
		g_failures++;
	}
}

static void check_number(const struct number_constant* c)
{
	const double* sym = (const double*) dlsym(RTLD_DEFAULT, c->name);
	if (sym == NULL)
	{
		printf("FAIL: %s is not exported\n", c->name);
		g_failures++;
	}
	else if (*sym != c->value)
	{
		printf("FAIL: %s = %g, expected %g\n", c->name, *sym, c->value);
		g_failures++;
	}
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	for (size_t i = 0; i < sizeof(frameworks) / sizeof(frameworks[0]); i++)
	{
		if (dlopen(frameworks[i], RTLD_NOW | RTLD_GLOBAL) == NULL)
		{
			printf("FAIL: cannot load %s: %s\n", frameworks[i], dlerror());
			g_failures++;
		}
	}

	size_t total = 0;
	for (size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); i++, total++)
		check_string(&strings[i]);
	for (size_t i = 0; i < sizeof(numbers) / sizeof(numbers[0]); i++, total++)
		check_number(&numbers[i]);

	if (g_failures)
		printf("FAIL: %d of %zu check(s) failed\n", g_failures, total);
	else
		printf("PASS: %zu constants\n", total);
	finish(g_failures ? 1 : 0);
}
