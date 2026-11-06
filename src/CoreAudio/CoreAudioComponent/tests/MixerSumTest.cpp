// src/CoreAudio/CoreAudioComponent/tests/MixerSumTest.cpp
#include "../MixerSum.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

int main()
{
	std::vector<float> a = {0.1f, 0.2f, 0.3f};
	std::vector<float> b = {0.4f, -0.1f, 0.05f};
	std::vector<float> out(3, 0.0f);

	float gains[2] = {1.0f, 0.5f};
	const float* inputs[2] = {a.data(), b.data()};
	mixerSum(inputs, gains, 2, out.data(), 3);

	assert(std::abs(out[0] - (0.1f + 0.4f*0.5f)) < 1e-6f);
	assert(std::abs(out[1] - (0.2f + -0.1f*0.5f)) < 1e-6f);
	assert(std::abs(out[2] - (0.3f + 0.05f*0.5f)) < 1e-6f);

	printf("MixerSumTest: OK\n");
	return 0;
}
