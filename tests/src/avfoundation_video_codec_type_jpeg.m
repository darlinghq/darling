// CFLAGS: -framework AVFoundation -framework Foundation
// Copyright (C) 2026 Lubos Dolezel
//
// AVVideoCodecTypeJPEG is the string "jpeg". Observed on macOS 26.7.3; passes unchanged there.
#import <AVFoundation/AVFoundation.h>
#include <stdio.h>

int main(void)
{
	int ok = [AVVideoCodecTypeJPEG isEqualToString:@"jpeg"];
	printf(ok ? "PASS\n" : "FAIL: AVVideoCodecTypeJPEG\n");
	return !ok;
}
