// Copyright (C) 2026 Lubos Dolezel
//
// AVAudioPlayerNode conforms to AVAudioMixing and keeps the volume, pan and 3D mixing
// properties with their documented defaults and ranges (clamping as observed on macOS 27.0.1,
// arm64 and x86_64: volume only below 0, pan -1..1, rate 0.5..2, reverbBlend 0..1,
// obstruction and occlusion -100..0), and an unconnected node has no mixer destination.
// Passes unchanged on macOS 27.0.1.
#import <AVFAudio/AVFAudio.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	@autoreleasepool {
		AVAudioPlayerNode* node = [[AVAudioPlayerNode alloc] init];
		CHECK([node conformsToProtocol:@protocol(AVAudioMixing)]);
		CHECK([node conformsToProtocol:@protocol(AVAudioStereoMixing)]);
		CHECK([node conformsToProtocol:@protocol(AVAudio3DMixing)]);

		CHECK(node.volume == 1.0f);
		CHECK(node.pan == 0.0f);
		CHECK(node.rate == 1.0f);
		CHECK(node.reverbBlend == 0.0f);
		CHECK(node.obstruction == 0.0f && node.occlusion == 0.0f);
		CHECK(node.renderingAlgorithm == AVAudio3DMixingRenderingAlgorithmEqualPowerPanning);
		CHECK(node.sourceMode == AVAudio3DMixingSourceModeSpatializeIfMono);
		CHECK(node.pointSourceInHeadMode == AVAudio3DMixingPointSourceInHeadModeMono);
		CHECK(node.position.x == 0 && node.position.y == 0 && node.position.z == 0);

		node.volume = 0.5f;
		CHECK(node.volume == 0.5f);
		node.volume = 2.0f;
		CHECK(node.volume == 2.0f);
		node.volume = -1.0f;
		CHECK(node.volume == 0.0f);

		node.pan = -0.5f;
		CHECK(node.pan == -0.5f);
		node.pan = 2.0f;
		CHECK(node.pan == 1.0f);
		node.pan = -2.0f;
		CHECK(node.pan == -1.0f);

		node.rate = 0.1f;
		CHECK(node.rate == 0.5f);
		node.rate = 5.0f;
		CHECK(node.rate == 2.0f);
		node.rate = 1.5f;
		CHECK(node.rate == 1.5f);

		node.reverbBlend = -1.0f;
		CHECK(node.reverbBlend == 0.0f);
		node.reverbBlend = 2.0f;
		CHECK(node.reverbBlend == 1.0f);

		node.obstruction = -200.0f;
		CHECK(node.obstruction == -100.0f);
		node.obstruction = 1.0f;
		CHECK(node.obstruction == 0.0f);
		node.occlusion = -10.0f;
		CHECK(node.occlusion == -10.0f);
		node.occlusion = -120.0f;
		CHECK(node.occlusion == -100.0f);

		AVAudio3DPoint point = { 1.0f, 2.0f, 3.0f };
		node.position = point;
		CHECK(node.position.x == 1.0f && node.position.y == 2.0f && node.position.z == 3.0f);
		node.renderingAlgorithm = AVAudio3DMixingRenderingAlgorithmHRTF;
		CHECK(node.renderingAlgorithm == AVAudio3DMixingRenderingAlgorithmHRTF);
		node.sourceMode = AVAudio3DMixingSourceModeBypass;
		CHECK(node.sourceMode == AVAudio3DMixingSourceModeBypass);
		node.pointSourceInHeadMode = AVAudio3DMixingPointSourceInHeadModeBypass;
		CHECK(node.pointSourceInHeadMode == AVAudio3DMixingPointSourceInHeadModeBypass);

		// Properties belong to the node.
		AVAudioPlayerNode* other = [[AVAudioPlayerNode alloc] init];
		CHECK(other.volume == 1.0f && other.pan == 0.0f);

		// An unconnected node has no destination.
		AVAudioMixerNode* mixer = [[AVAudioMixerNode alloc] init];
		CHECK([node destinationForMixer:mixer bus:0] == nil);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
