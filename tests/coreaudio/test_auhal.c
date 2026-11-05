// tests/coreaudio/test_auhal.c
// End-to-end check of the output and input units against a running PulseAudio server.
// Needs an installed build: run it inside Darling. Exits non-zero on the first failure.
//
// Only silence is played. Use a real sink for output: a null sink consumes audio in bursts
// of up to two seconds, which fails the real-time check. ./fixture.sh up provides a source,
// selected with PULSE_SOURCE=darling-test-source, so that no microphone has to be opened.
#include <AudioToolbox/AUGraph.h>
#include <AudioToolbox/AudioUnitParameters.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreAudio/HostTime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(expr) do { OSStatus s_ = (expr); if (s_ != noErr) { printf("FAIL %s:%d: %s = %d\n", __FILE__, __LINE__, #expr, (int) s_); exit(1); } } while (0)
#define REQUIRE(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); exit(1); } } while (0)

static volatile int g_renders, g_badTimestamps, g_inputCallbacks, g_inputRenderFailures;
static volatile OSStatus g_inputRenderError;
static volatile UInt64 g_framesRendered;

static OSStatus renderSilence(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
	UInt32 bus, UInt32 frames, AudioBufferList* data)
{
	if (!(time->mFlags & kAudioTimeStampHostTimeValid) || !time->mHostTime)
		g_badTimestamps++;
	for (UInt32 i = 0; i < data->mNumberBuffers; i++)
		memset(data->mBuffers[i].mData, 0, data->mBuffers[i].mDataByteSize);
	g_renders++;
	g_framesRendered += frames;
	return noErr;
}

static AudioUnit newUnit(OSType subType)
{
	AudioComponentDescription desc = { kAudioUnitType_Output, subType, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	REQUIRE(component != NULL);

	AudioUnit unit = NULL;
	CHECK(AudioComponentInstanceNew(component, &unit));
	return unit;
}

static AudioStreamBasicDescription format(double rate, UInt32 channels, UInt32 flags, UInt32 bits)
{
	const UInt32 frameBytes = (flags & kAudioFormatFlagIsNonInterleaved) ? bits / 8 : channels * bits / 8;
	AudioStreamBasicDescription asbd = { rate, kAudioFormatLinearPCM, flags, frameBytes, 1, frameBytes, channels, bits, 0 };
	return asbd;
}

// The format is set before AudioUnitInitialize, and the unit must play again after a stop.
static void testPlayback(AudioStreamBasicDescription asbd, const char* name)
{
	AudioUnit unit = newUnit(kAudioUnitSubType_DefaultOutput);
	AURenderCallbackStruct callback = { renderSilence, NULL };

	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)));
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback)));
	CHECK(AudioUnitInitialize(unit));

	for (int round = 0; round < 2; round++)
	{
		g_renders = 0;
		g_framesRendered = 0;
		CHECK(AudioOutputUnitStart(unit));
		sleep(3);
		CHECK(AudioOutputUnitStop(unit));

		const double seconds = g_framesRendered / asbd.mSampleRate;
		printf("  %s, round %d: %d renders, %.2fs of audio in 3s\n", name, round, g_renders, seconds);

		// Real time, give or take startup and what the server has buffered.
		REQUIRE(seconds > 2.0 && seconds < 4.0);
	}
	REQUIRE(g_badTimestamps == 0);

	CHECK(AudioUnitUninitialize(unit));
	CHECK(AudioComponentInstanceDispose(unit));
}

static AudioUnit g_inputUnit;
static AudioBufferList* g_inputBuffers;
static UInt32 g_inputBufferBytes;

static OSStatus inputAvailable(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
	UInt32 bus, UInt32 frames, AudioBufferList* data)
{
	// A render leaves the slice's size here, and the next slice may be larger.
	g_inputBuffers->mBuffers[0].mDataByteSize = g_inputBufferBytes;

	OSStatus status = AudioUnitRender(g_inputUnit, flags, time, 1, frames, g_inputBuffers);
	if (status != noErr)
	{
		g_inputRenderError = status;
		g_inputRenderFailures++;
	}
	g_inputCallbacks++;
	return noErr;
}

static void testInput(void)
{
	AudioUnit unit = g_inputUnit = newUnit(kAudioUnitSubType_HALOutput);
	UInt32 on = 1, off = 0, value = 99;
	UInt32 size = sizeof(value);

	CHECK(AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &on, sizeof(on)));
	CHECK(AudioUnitSetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &off, sizeof(off)));
	CHECK(AudioUnitGetProperty(unit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &value, &size));
	REQUIRE(value == 1);

	// Interleaved 16-bit mono, at a rate the device doesn't advertise.
	AudioStreamBasicDescription asbd = format(16000, 1, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 16);
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &asbd, sizeof(asbd)));

	AURenderCallbackStruct callback = { inputAvailable, NULL };
	CHECK(AudioUnitSetProperty(unit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback)));

	static int16_t samples[8192];
	AudioBufferList list = { 1, { { 1, sizeof(samples), samples } } };
	g_inputBuffers = &list;
	g_inputBufferBytes = sizeof(samples);

	CHECK(AudioUnitInitialize(unit));
	CHECK(AudioOutputUnitStart(unit));
	sleep(1);
	CHECK(AudioOutputUnitStop(unit));

	printf("  input: %d callbacks, %d render failures (last error %d)\n", g_inputCallbacks, g_inputRenderFailures, (int) g_inputRenderError);
	REQUIRE(g_inputCallbacks > 0);
	REQUIRE(g_inputRenderFailures == 0);

	CHECK(AudioComponentInstanceDispose(unit));
}

// Every enumerated device answers its basic properties, and a volume read returns.
static void testDevices(void)
{
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	UInt32 size = 0;
	CHECK(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, NULL, &size));

	AudioDeviceID* devices = malloc(size);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, devices));

	for (UInt32 i = 0; i < size / sizeof(AudioDeviceID); i++)
	{
		AudioObjectPropertyAddress uidAddress = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		CFStringRef uid = NULL;
		UInt32 uidSize = sizeof(uid);
		CHECK(AudioObjectGetPropertyData(devices[i], &uidAddress, 0, NULL, &uidSize, &uid));

		AudioDeviceID found = kAudioObjectUnknown;
		AudioValueTranslation translation = { &uid, sizeof(uid), &found, sizeof(found) };
		AudioObjectPropertyAddress forUID = { kAudioHardwarePropertyDeviceForUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		UInt32 translationSize = sizeof(translation);
		CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &forUID, 0, NULL, &translationSize, &translation));
		REQUIRE(found == devices[i]);
		CFRelease(uid);

		Float32 volume = -1;
		UInt32 volumeSize = sizeof(volume);
		AudioObjectPropertyAddress volumeAddress = { kAudioDevicePropertyVolumeScalar, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		CHECK(AudioObjectGetPropertyData(devices[i], &volumeAddress, 0, NULL, &volumeSize, &volume));
		REQUIRE(volume >= 0);

		AudioStreamID stream = 0;
		UInt32 streamSize = sizeof(stream);
		AudioObjectPropertyAddress streams = { kAudioDevicePropertyStreams, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		CHECK(AudioObjectGetPropertyData(devices[i], &streams, 0, NULL, &streamSize, &stream));
		REQUIRE(streamSize == sizeof(stream));

		UInt32 direction = 99, directionSize = sizeof(direction);
		AudioObjectPropertyAddress directionAddress = { kAudioStreamPropertyDirection, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
		CHECK(AudioObjectGetPropertyData(stream, &directionAddress, 0, NULL, &directionSize, &direction));
		REQUIRE(direction <= 1);
	}
	free(devices);
}

// A graph fed by an input callback through a mixer, whose gain must arrive intact.
static void testGraph(void)
{
	AUGraph graph;
	AUNode mixerNode, outputNode;
	AudioComponentDescription mixerDesc = { kAudioUnitType_Mixer, kAudioUnitSubType_MultiChannelMixer, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponentDescription outputDesc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };

	CHECK(NewAUGraph(&graph));
	CHECK(AUGraphAddNode(graph, &mixerDesc, &mixerNode));
	CHECK(AUGraphAddNode(graph, &outputDesc, &outputNode));
	CHECK(AUGraphOpen(graph));
	CHECK(AUGraphConnectNodeInput(graph, mixerNode, 0, outputNode, 0));

	AURenderCallbackStruct callback = { renderSilence, NULL };
	CHECK(AUGraphSetNodeInputCallback(graph, mixerNode, 0, &callback));

	UInt32 interactions = 0;
	CHECK(AUGraphGetNumberOfInteractions(graph, &interactions));
	REQUIRE(interactions == 2);

	AudioUnit mixer;
	CHECK(AUGraphNodeInfo(graph, mixerNode, NULL, &mixer));
	CHECK(AudioUnitSetParameter(mixer, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, 0.25f, 0));

	AudioUnitParameterValue gain = 0;
	CHECK(AudioUnitGetParameter(mixer, kMultiChannelMixerParam_Volume, kAudioUnitScope_Input, 0, &gain));
	REQUIRE(gain == 0.25f);

	g_renders = 0;
	CHECK(AUGraphInitialize(graph));
	CHECK(AUGraphStart(graph));
	sleep(1);
	CHECK(AUGraphStop(graph));
	printf("  graph: %d renders\n", g_renders);
	REQUIRE(g_renders > 0);

	CHECK(DisposeAUGraph(graph));
}

int main(void)
{
	REQUIRE(AudioGetCurrentHostTime() != 0);

	printf("devices\n");
	testDevices();

	printf("playback\n");
	testPlayback(format(44100, 2, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved, 32), "planar float");
	testPlayback(format(48000, 2, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 32), "interleaved float");
	testPlayback(format(22050, 1, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked, 16), "mono int16");

	printf("input\n");
	testInput();

	printf("graph\n");
	testGraph();

	printf("test_auhal: OK\n");
	return 0;
}
