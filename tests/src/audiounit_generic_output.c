// Copyright (C) 2026 Lubos Dolezel
//
// The generic output unit ('auou'/'genr'): found as an audio component, usable as the
// output node of an AUGraph (a graph has just one output node), and rendered by pulling
// its output bus. Expected values were observed on macOS 26.7.3 (x86_64) and 10.14.4.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)
#define CHECK_NEAR(a, b, tol) do { double _a = (a), _b = (b); if (fabs(_a - _b) > (tol)) { printf("FAIL %d: %s == %.4f, expected %.4f\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static AudioStreamBasicDescription float_format(int channels, int noninterleaved, double rate)
{
	AudioStreamBasicDescription f;
	memset(&f, 0, sizeof(f));
	f.mSampleRate = rate;
	f.mFormatID = kAudioFormatLinearPCM;
	f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | (noninterleaved ? kAudioFormatFlagIsNonInterleaved : 0);
	f.mChannelsPerFrame = channels;
	f.mBitsPerChannel = 32;
	f.mFramesPerPacket = 1;
	f.mBytesPerFrame = noninterleaved ? 4 : 4 * channels;
	f.mBytesPerPacket = f.mBytesPerFrame;
	return f;
}

static int g_calls;
static UInt32 g_bus, g_frames;
static Float64 g_sampleTime;

// Source: every channel's sample is (sample time + frame) / 1000.
static OSStatus source(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* ts, UInt32 bus, UInt32 frames, AudioBufferList* io)
{
	(void) refCon; (void) flags;
	g_calls++;
	g_bus = bus;
	g_frames = frames;
	g_sampleTime = ts->mSampleTime;
	for (UInt32 b = 0; b < io->mNumberBuffers; b++)
	{
		float* p = (float*) io->mBuffers[b].mData;
		UInt32 ch = io->mBuffers[b].mNumberChannels;
		for (UInt32 i = 0; i < frames; i++)
			for (UInt32 c = 0; c < ch; c++)
				p[i * ch + c] = (float) (ts->mSampleTime + i) / 1000.0f;
	}
	return noErr;
}

typedef struct
{
	AudioBufferList list;
	AudioBuffer extra;
} StereoList;

static void test_unit(void)
{
	AudioComponentDescription d = { kAudioUnitType_Output, kAudioUnitSubType_GenericOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent c = AudioComponentFindNext(NULL, &d);
	CHECK(c != NULL);
	if (!c)
		return;
	AudioComponentDescription got;
	CHECK_EQ(AudioComponentGetDescription(c, &got), noErr);
	CHECK_EQ(got.componentType, kAudioUnitType_Output);
	CHECK_EQ(got.componentSubType, kAudioUnitSubType_GenericOutput);
	CHECK(AudioComponentFindNext(c, &d) == NULL);

	AudioUnit u;
	CHECK_EQ(AudioComponentInstanceNew(c, &u), noErr);
	UInt32 n = 0, size = sizeof(n);

	// One input and one output element; the default format is 44.1 kHz stereo non-interleaved float.
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &n, &size), noErr);
	CHECK_EQ(n, 1);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_ElementCount, kAudioUnitScope_Output, 0, &n, &size), noErr);
	CHECK_EQ(n, 1);
	AudioStreamBasicDescription f, def = float_format(2, 1, 44100);
	size = sizeof(f);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &f, &size), noErr);
	CHECK(memcmp(&f, &def, sizeof(f)) == 0);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &f, &size), noErr);
	CHECK(memcmp(&f, &def, sizeof(f)) == 0);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &f, &size), kAudioUnitErr_InvalidElement);
	AudioUnitGetProperty(u, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0, &n, &size);
	CHECK_EQ(n, 1156);
	UInt32 running = 9;
	size = sizeof(running);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &running, &size), noErr);
	CHECK_EQ(running, 0);
	// No device to choose.
	UInt32 sz = 0;
	Boolean w;
	CHECK_EQ(AudioUnitGetPropertyInfo(u, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &sz, &w), kAudioUnitErr_InvalidProperty);

	static float a[1024], b[1024];
	StereoList sl;
	AudioBufferList* io = &sl.list;
	io->mNumberBuffers = 2;
	io->mBuffers[0].mNumberChannels = 1;
	io->mBuffers[0].mDataByteSize = 512 * 4;
	io->mBuffers[0].mData = a;
	io->mBuffers[1].mNumberChannels = 1;
	io->mBuffers[1].mDataByteSize = 512 * 4;
	io->mBuffers[1].mData = b;
	AudioUnitRenderActionFlags flags = 0;
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mFlags = kAudioTimeStampSampleTimeValid;

	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), kAudioUnitErr_Uninitialized);
	CHECK_EQ(AudioUnitInitialize(u), noErr);
	// Nothing feeds the input.
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), kAudioUnitErr_NoConnection);
	CHECK_EQ(AudioUnitUninitialize(u), noErr);

	AURenderCallbackStruct cb = { source, NULL };
	CHECK_EQ(AudioUnitSetProperty(u, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof(cb)), noErr);
	CHECK_EQ(AudioUnitInitialize(u), noErr);

	// Rendering pulls the source for the same time stamp and frame count.
	ts.mSampleTime = 1000;
	g_calls = 0;
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), noErr);
	CHECK_EQ(g_calls, 1);
	CHECK_EQ(g_bus, 0);
	CHECK_EQ(g_frames, 512);
	CHECK_NEAR(g_sampleTime, 1000, 0);
	CHECK_EQ(io->mBuffers[0].mDataByteSize, 2048);
	CHECK_NEAR(a[0], 1.0, 1e-5);
	CHECK_NEAR(a[511], 1.511, 1e-5);
	CHECK_NEAR(b[3], 1.003, 1e-5);

	// Output buffers with no memory of their own are supplied by the unit.
	io->mBuffers[0].mData = NULL;
	io->mBuffers[1].mData = NULL;
	io->mBuffers[0].mDataByteSize = 512 * 4;
	io->mBuffers[1].mDataByteSize = 512 * 4;
	ts.mSampleTime = 1512;
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), noErr);
	CHECK(io->mBuffers[0].mData != NULL);
	if (io->mBuffers[0].mData)
		CHECK_NEAR(((float*) io->mBuffers[0].mData)[0], 1.512, 1e-5);
	io->mBuffers[0].mData = a;
	io->mBuffers[1].mData = b;
	io->mBuffers[0].mDataByteSize = 512 * 4;
	io->mBuffers[1].mDataByteSize = 512 * 4;

	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 1, 512, io), kAudioUnitErr_InvalidElement);
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 100000, io), kAudioUnitErr_TooManyFramesToProcess);
	io->mNumberBuffers = 1;
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), kAudio_ParamError);
	io->mNumberBuffers = 2;

	// "Running" is only a state to read back; Start and Stop are always accepted.
	CHECK_EQ(AudioOutputUnitStart(u), noErr);
	size = sizeof(running);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &running, &size), noErr);
	CHECK_EQ(running, 1);
	g_calls = 0;
	ts.mSampleTime = 3000;
	CHECK_EQ(AudioUnitRender(u, &flags, &ts, 0, 512, io), noErr);
	CHECK_EQ(g_calls, 1);
	CHECK_EQ(AudioOutputUnitStop(u), noErr);
	CHECK_EQ(AudioOutputUnitStop(u), noErr);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &running, &size), noErr);
	CHECK_EQ(running, 0);
	CHECK_EQ(AudioUnitUninitialize(u), noErr);

	// The two sides can have different formats (here a mono output); the formats can
	// only be changed while uninitialized.
	AudioStreamBasicDescription mono = float_format(1, 1, 44100);
	CHECK_EQ(AudioUnitSetProperty(u, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &mono, sizeof(mono)), noErr);
	size = sizeof(f);
	CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &f, &size), noErr);
	CHECK_EQ(f.mChannelsPerFrame, 1);
	AudioComponentInstanceDispose(u);
}

static void test_graph(void)
{
	AudioComponentDescription gen = { kAudioUnitType_Output, kAudioUnitSubType_GenericOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription def = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription mixer = { kAudioUnitType_Mixer, kAudioUnitSubType_MultiChannelMixer, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription bogus = { kAudioUnitType_Effect, 'nope', kAudioUnitManufacturer_Apple, 0, 0 };
	AUGraph g;
	AUNode n;
	CHECK_EQ(NewAUGraph(&g), noErr);

	// Before the graph is open nodes are only recorded, but a graph holds one output node.
	CHECK_EQ(AUGraphAddNode(g, &def, &n), noErr);
	CHECK_EQ(AUGraphAddNode(g, &gen, &n), kAUGraphErr_OutputNodeErr);
	CHECK_EQ(AUGraphAddNode(g, &mixer, &n), noErr);
	CHECK_EQ(DisposeAUGraph(g), noErr);

	CHECK_EQ(NewAUGraph(&g), noErr);
	CHECK_EQ(AUGraphOpen(g), noErr);
	// In an open graph the unit is created at once; a component that does not exist is an error.
	CHECK_EQ(AUGraphAddNode(g, &bogus, &n), -2005);
	AUNode output;
	CHECK_EQ(AUGraphAddNode(g, &gen, &output), noErr);
	CHECK_EQ(AUGraphAddNode(g, &gen, &n), kAUGraphErr_OutputNodeErr);

	AudioUnit unit = NULL;
	CHECK_EQ(AUGraphNodeInfo(g, output, NULL, &unit), noErr);
	CHECK(unit != NULL);
	if (unit)
	{
		AudioStreamBasicDescription f = float_format(2, 1, 44100);
		AURenderCallbackStruct cb = { source, NULL };
		CHECK_EQ(AUGraphSetNodeInputCallback(g, output, 0, &cb), noErr);
		CHECK_EQ(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &f, sizeof(f)), noErr);
		CHECK_EQ(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &f, sizeof(f)), noErr);
		CHECK_EQ(AUGraphInitialize(g), noErr);
		CHECK_EQ(AUGraphStart(g), noErr);
		Boolean running = 0;
		CHECK_EQ(AUGraphIsRunning(g, &running), noErr);
		CHECK(running);

		// Offline rendering through the graph: pull the output unit.
		static float a[1024], b[1024];
		StereoList sl;
		AudioBufferList* io = &sl.list;
		io->mNumberBuffers = 2;
		io->mBuffers[0].mNumberChannels = 1;
		io->mBuffers[0].mDataByteSize = 512 * 4;
		io->mBuffers[0].mData = a;
		io->mBuffers[1].mNumberChannels = 1;
		io->mBuffers[1].mDataByteSize = 512 * 4;
		io->mBuffers[1].mData = b;
		AudioUnitRenderActionFlags flags = 0;
		AudioTimeStamp ts;
		memset(&ts, 0, sizeof(ts));
		ts.mFlags = kAudioTimeStampSampleTimeValid;
		g_calls = 0;
		CHECK_EQ(AudioUnitRender(unit, &flags, &ts, 0, 512, io), noErr);
		CHECK_EQ(g_calls, 1);
		CHECK_NEAR(a[10], 0.010, 1e-5);
		CHECK_EQ(AUGraphStop(g), noErr);
		CHECK_EQ(AUGraphUninitialize(g), noErr);
	}
	CHECK_EQ(AUGraphClose(g), noErr);
	CHECK_EQ(DisposeAUGraph(g), noErr);

	// An open graph whose nodes include a missing component cannot be opened.
	CHECK_EQ(NewAUGraph(&g), noErr);
	CHECK_EQ(AUGraphAddNode(g, &bogus, &n), noErr);
	CHECK_EQ(AUGraphOpen(g), -2005);
	CHECK_EQ(DisposeAUGraph(g), noErr);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);
	test_unit();
	test_graph();
	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
