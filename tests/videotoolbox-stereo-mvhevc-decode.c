// Copyright (C) 2026 Lubos Dolezel
//
// VTIsStereoMVHEVCDecodeSupported reports whether the system can decode
// stereo MV-HEVC. Observed on macOS 26.7.3 x86_64: false. Darling has no
// MV-HEVC decoder, so it answers false as well.

#include <VideoToolbox/VideoToolbox.h>
#include <stdio.h>

int main(void)
{
	Boolean supported = VTIsStereoMVHEVCDecodeSupported();
	if (supported) {
		printf("FAIL: VTIsStereoMVHEVCDecodeSupported() returned true\n");
		printf("FAILED\n");
		return 1;
	}
	printf("PASSED\n");
	return 0;
}
