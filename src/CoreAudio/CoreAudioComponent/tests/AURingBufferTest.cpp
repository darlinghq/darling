// src/CoreAudio/CoreAudioComponent/tests/AURingBufferTest.cpp
#include "../AURingBuffer.h"
#include <cassert>
#include <cstdio>
#include <vector>

int main()
{
	AURingBuffer rb(/*capacityFrames=*/8, /*channels=*/2);

	std::vector<float> in = {1,1, 2,2, 3,3, 4,4}; // 4 frames, 2 channels interleaved
	size_t written = rb.write(in.data(), 4);
	assert(written == 4);

	std::vector<float> out(4 * 2, -1.0f);
	size_t read = rb.read(out.data(), 4);
	assert(read == 4);
	assert(out == in);

	// Reading again with nothing new returns 0 and leaves the buffer untouched (silence
	// is the caller's responsibility, matching how AUHAL callbacks handle underflow).
	size_t read2 = rb.read(out.data(), 4);
	assert(read2 == 0);

	// Writing more than capacity caps at capacity (no overflow UB).
	std::vector<float> big(20 * 2, 9.0f);
	size_t written2 = rb.write(big.data(), 20);
	assert(written2 == 8); // capacityFrames

	printf("AURingBufferTest: OK\n");
	return 0;
}
