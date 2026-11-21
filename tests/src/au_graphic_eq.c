// Copyright (C) 2026 Lubos Dolezel
//
// The graphic equalizer effect unit (kAudioUnitType_Effect,
// kAudioUnitSubType_GraphicEQ, kAudioUnitManufacturer_Apple):
// - it can be found, instantiated and added to an AUGraph;
// - its parameters: kGraphicEQParam_NumberOfBands (0 = 10 bands, 1 = 31),
//   one gain per band, -20..+20 dB, named after the band frequency, and
//   a change of the band count announced as a ParameterList change;
// - its frequency response, measured with sine tones rendered through it:
//   a band's gain is exact at its centre, and the response around it is
//   that of Apple's unit (values measured on macOS 26.7.3) within 0.15 dB.
// The program passes unchanged on macOS 26.7.3 (x86_64).
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

enum { N = 512 };
static float g_tone[2][N];

static OSStatus input(void* ref, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* ts,
	UInt32 bus, UInt32 frames, AudioBufferList* io)
{
	(void) ref; (void) flags; (void) ts; (void) bus;
	for (UInt32 c = 0; c < io->mNumberBuffers; c++)
		memcpy(io->mBuffers[c].mData, g_tone[c], frames * sizeof(float));
	return noErr;
}

// Gain in dB of the unit for a sine tone, after it has settled
static double tone_gain(AudioUnit u, double freq, double rate)
{
	static float out[2][N];
	double phase = 0, in2 = 0, out2 = 0;
	for (int block = 0; block < 600; block++)
	{
		for (int i = 0; i < N; i++)
		{
			g_tone[0][i] = g_tone[1][i] = 0.25f * (float) sin(phase);
			phase += 2 * M_PI * freq / rate;
		}
		struct { AudioBufferList list; AudioBuffer second; } abl;
		abl.list.mNumberBuffers = 2;
		for (int c = 0; c < 2; c++)
		{
			abl.list.mBuffers[c].mNumberChannels = 1;
			abl.list.mBuffers[c].mDataByteSize = N * sizeof(float);
			abl.list.mBuffers[c].mData = out[c];
		}
		AudioTimeStamp ts;
		memset(&ts, 0, sizeof(ts));
		ts.mFlags = kAudioTimeStampSampleTimeValid;
		ts.mSampleTime = block * N;
		AudioUnitRenderActionFlags flags = 0;
		OSStatus s = AudioUnitRender(u, &flags, &ts, 0, N, &abl.list);
		if (s != noErr)
		{
			printf("FAIL: AudioUnitRender %d\n", (int) s);
			g_failures++;
			return 0;
		}
		if (block >= 300)
		{
			for (int i = 0; i < N; i++)
			{
				in2 += g_tone[0][i] * g_tone[0][i];
				out2 += out[1][i] * out[1][i];
			}
		}
	}
	return 10 * log10(out2 / in2);
}

static void check_response(AudioUnit u, const char* what, const double* freqs, const double* expected, int n)
{
	for (int i = 0; i < n; i++)
	{
		double g = tone_gain(u, freqs[i], 44100);
		if (fabs(g - expected[i]) > 0.15)
		{
			printf("FAIL: %s at %g Hz: %.3f dB, expected %.3f dB\n", what, freqs[i], g, expected[i]);
			g_failures++;
		}
	}
}

static int g_listChanges;
static void property_changed(void* ref, AudioUnit u, AudioUnitPropertyID id, AudioUnitScope scope, AudioUnitElement el)
{
	(void) ref; (void) u; (void) scope; (void) el;
	if (id == kAudioUnitProperty_ParameterList)
		g_listChanges++;
}

static int parameter_count(AudioUnit u)
{
	UInt32 size = 0;
	Boolean writable;
	if (AudioUnitGetPropertyInfo(u, kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0, &size, &writable) != noErr)
		return -1;
	return size / sizeof(AudioUnitParameterID);
}

static void check_name(AudioUnit u, AudioUnitParameterID id, const char* expected)
{
	AudioUnitParameterInfo info;
	UInt32 size = sizeof(info);
	memset(&info, 0, sizeof(info));
	OSStatus s = AudioUnitGetProperty(u, kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, id, &info, &size);
	char name[64] = "";
	if (s == noErr && info.cfNameString)
		CFStringGetCString(info.cfNameString, name, sizeof(name), kCFStringEncodingUTF8);
	if (s != noErr || strcmp(name, expected) != 0)
	{
		printf("FAIL: parameter %u: status %d, name '%s', expected '%s'\n", (unsigned) id, (int) s, name, expected);
		g_failures++;
	}
	if (s == noErr && info.unit == kAudioUnitParameterUnit_Decibels)
		CHECK(info.minValue == -20 && info.maxValue == 20 && info.defaultValue == 0);
	if (s == noErr && (info.flags & kAudioUnitParameterFlag_CFNameRelease) && info.cfNameString)
		CFRelease(info.cfNameString);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(120);

	AudioComponentDescription d = { kAudioUnitType_Effect, kAudioUnitSubType_GraphicEQ, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent c = AudioComponentFindNext(NULL, &d);
	CHECK(c != NULL);
	if (!c)
	{
		printf("FAIL: no graphic EQ unit\n");
		return 1;
	}

	// It can be a node of a graph
	{
		AUGraph graph;
		AUNode node;
		CHECK_EQ(NewAUGraph(&graph), noErr);
		CHECK_EQ(AUGraphOpen(graph), noErr);
		CHECK_EQ(AUGraphAddNode(graph, &d, &node), noErr);
		AudioUnit unit = NULL;
		CHECK_EQ(AUGraphNodeInfo(graph, node, NULL, &unit), noErr);
		CHECK(unit != NULL);
		CHECK_EQ(AUGraphClose(graph), noErr);
		CHECK_EQ(DisposeAUGraph(graph), noErr);
	}

	AudioUnit u = NULL;
	CHECK_EQ(AudioComponentInstanceNew(c, &u), noErr);
	if (!u)
		return 1;

	AudioUnitParameterValue v = -1;
	CHECK_EQ(AudioUnitGetParameter(u, kGraphicEQParam_NumberOfBands, kAudioUnitScope_Global, 0, &v), noErr);
	CHECK(v == 1);
	CHECK_EQ(parameter_count(u), 32);
	check_name(u, 0, "20.0 Hz");
	check_name(u, 2, "31.5 Hz");
	check_name(u, 17, "1000.0 Hz");
	check_name(u, 28, "12000.0 Hz");
	check_name(u, 30, "20000.0 Hz");
	check_name(u, kGraphicEQParam_NumberOfBands, "Number of Bands");

	{
		AudioUnitParameterInfo info;
		UInt32 size = sizeof(info);
		CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, kGraphicEQParam_NumberOfBands, &info, &size), noErr);
		CHECK(info.unit == kAudioUnitParameterUnit_Indexed && info.minValue == 0 && info.maxValue == 1 && info.defaultValue == 1);
		CHECK(info.flags & kAudioUnitParameterFlag_IsGlobalMeta);

		CFArrayRef strings = NULL;
		size = sizeof(strings);
		CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_ParameterValueStrings, kAudioUnitScope_Global, kGraphicEQParam_NumberOfBands, &strings, &size), noErr);
		CHECK(strings != NULL && CFArrayGetCount(strings) == 2);
		if (strings && CFArrayGetCount(strings) == 2)
		{
			CHECK(CFStringCompare(CFArrayGetValueAtIndex(strings, 0), CFSTR("10 Bands"), 0) == kCFCompareEqualTo);
			CHECK(CFStringCompare(CFArrayGetValueAtIndex(strings, 1), CFSTR("31 Bands"), 0) == kCFCompareEqualTo);
		}
		if (strings)
			CFRelease(strings);

		Float64 tail = -1, latency = -1;
		size = sizeof(Float64);
		CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_TailTime, kAudioUnitScope_Global, 0, &tail, &size), noErr);
		size = sizeof(Float64);
		CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0, &latency, &size), noErr);
		CHECK(tail > 0 && tail < 0.01);
		CHECK(latency == 0);
	}

	// 10 bands: the list shrinks, and listeners hear about it
	CHECK_EQ(AudioUnitAddPropertyListener(u, kAudioUnitProperty_ParameterList, property_changed, NULL), noErr);
	CHECK_EQ(AudioUnitSetParameter(u, kGraphicEQParam_NumberOfBands, kAudioUnitScope_Global, 0, 0, 0), noErr);
	CHECK(g_listChanges >= 1);
	CHECK_EQ(parameter_count(u), 11);
	check_name(u, 0, "32 Hz");
	check_name(u, 5, "1024 Hz");
	check_name(u, 9, "16384 Hz");
	{
		AudioUnitParameterInfo info;
		UInt32 size = sizeof(info);
		CHECK_EQ(AudioUnitGetProperty(u, kAudioUnitProperty_ParameterInfo, kAudioUnitScope_Global, 12, &info, &size), kAudioUnitErr_InvalidParameter);
	}

	// Frequency response at 44.1 kHz, stereo float
	AURenderCallbackStruct cb = { input, NULL };
	CHECK_EQ(AudioUnitSetProperty(u, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof(cb)), noErr);
	CHECK_EQ(AudioUnitInitialize(u), noErr);

	static const double flat[] = { 32, 1000, 16000 };
	static const double zero[] = { 0, 0, 0 };
	check_response(u, "flat", flat, zero, 3);

	static const double f10[] = { 362, 512, 724, 861, 1024, 1218, 1448, 2048, 2896 };
	static const double g10p12[] = { 2.420, 4.576, 8.345, 10.717, 12.000, 10.713, 8.336, 4.543, 2.372 };
	static const double g10m20[] = { -3.143, -5.841, -11.001, -15.558, -20.000, -15.548, -10.986, -5.800, -3.082 };
	static const double g10p3[] = { 0.730, 1.394, 2.381, 2.821, 3.000, 2.820, 2.379, 1.384, 0.715 };
	AudioUnitSetParameter(u, 5, kAudioUnitScope_Global, 0, 12, 0);
	check_response(u, "10 bands, 1024 Hz +12 dB", f10, g10p12, 9);
	AudioUnitSetParameter(u, 5, kAudioUnitScope_Global, 0, -20, 0);
	check_response(u, "10 bands, 1024 Hz -20 dB", f10, g10m20, 9);
	AudioUnitSetParameter(u, 5, kAudioUnitScope_Global, 0, 3, 0);
	check_response(u, "10 bands, 1024 Hz +3 dB", f10, g10p3, 9);
	AudioUnitSetParameter(u, 5, kAudioUnitScope_Global, 0, 0, 0);

	static const double f10lo[] = { 16, 32, 64 };
	static const double g10lo[] = { 4.56, 12.0, 4.56 };
	AudioUnitSetParameter(u, 0, kAudioUnitScope_Global, 0, 12, 0);
	check_response(u, "10 bands, 32 Hz +12 dB", f10lo, g10lo, 3);
	AudioUnitSetParameter(u, 0, kAudioUnitScope_Global, 0, 0, 0);

	// Back to 31 bands, also while initialized
	CHECK_EQ(AudioUnitSetParameter(u, kGraphicEQParam_NumberOfBands, kAudioUnitScope_Global, 0, 1, 0), noErr);
	CHECK_EQ(parameter_count(u), 32);
	static const double f31[] = { 707, 794, 891, 1000, 1122, 1260, 1414 };
	static const double g31p12[] = { 2.997, 4.927, 8.425, 12.000, 8.433, 4.909, 2.990 };
	static const double g31p6[] = { 1.742, 2.870, 4.678, 6.000, 4.681, 2.860, 1.738 };
	AudioUnitSetParameter(u, 17, kAudioUnitScope_Global, 0, 12, 0);
	check_response(u, "31 bands, 1000 Hz +12 dB", f31, g31p12, 7);
	AudioUnitSetParameter(u, 17, kAudioUnitScope_Global, 0, 6, 0);
	check_response(u, "31 bands, 1000 Hz +6 dB", f31, g31p6, 7);
	AudioUnitSetParameter(u, 17, kAudioUnitScope_Global, 0, 0, 0);

	// Band centres are exact up high too
	static const double f31hi[] = { 12000, 20000 };
	static const double g31hi[] = { 12.0, 12.0 };
	AudioUnitSetParameter(u, 28, kAudioUnitScope_Global, 0, 12, 0);
	check_response(u, "31 bands, 12 kHz +12 dB", f31hi, g31hi, 1);
	AudioUnitSetParameter(u, 28, kAudioUnitScope_Global, 0, 0, 0);
	AudioUnitSetParameter(u, 30, kAudioUnitScope_Global, 0, 12, 0);
	check_response(u, "31 bands, 20 kHz +12 dB", f31hi + 1, g31hi + 1, 1);
	AudioUnitSetParameter(u, 30, kAudioUnitScope_Global, 0, 0, 0);

	CHECK_EQ(AudioUnitUninitialize(u), noErr);
	CHECK_EQ(AudioComponentInstanceDispose(u), noErr);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	fflush(stdout);
	return g_failures ? 1 : 0;
}
