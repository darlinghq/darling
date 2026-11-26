// Copyright (C) 2026 Lubos Dolezel
//
// kAudioUnitProperty_RenderQuality on the HAL output units (HALOutput, DefaultOutput, SystemOutput).
// The program passes unchanged on macOS 26.7.3 and 10.14.4.
// No audio is started. The expectations follow what AudioUnitGetProperty/AudioUnitSetProperty return
// for these units on macOS 26.7.3 and 10.14.4 (black-box observation).
#include <AudioToolbox/AudioComponent.h>
#include <AudioUnit/AudioUnit.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static OSStatus setQuality(AudioUnit unit, AudioUnitScope scope, AudioUnitElement element, UInt32 value)
{
	return AudioUnitSetProperty(unit, kAudioUnitProperty_RenderQuality, scope, element, &value, sizeof(value));
}

static UInt32 getQuality(AudioUnit unit, AudioUnitScope scope, AudioUnitElement element, OSStatus* status)
{
	UInt32 value = 0xdeadbeef, size = sizeof(value);

	*status = AudioUnitGetProperty(unit, kAudioUnitProperty_RenderQuality, scope, element, &value, &size);
	return value;
}

static void check_unit(const char* name, OSType subtype)
{
	AudioComponentDescription desc = { kAudioUnitType_Output, subtype, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	AudioUnit unit = NULL;
	OSStatus status;
	UInt32 size = 0, value;
	Boolean writable = false;

	printf("-- %s\n", name);
	CHECK(component != NULL);
	if (component == NULL || AudioComponentInstanceNew(component, &unit) != noErr)
	{
		printf("FAIL: cannot instantiate %s\n", name);
		g_failures++;
		return;
	}

	// Property info and the initial value (Medium).
	status = AudioUnitGetPropertyInfo(unit, kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, &size, &writable);
	CHECK(status == noErr && size == sizeof(UInt32) && writable);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_Medium);

	// Every documented quality level reads back.
	static const UInt32 levels[] = { kRenderQuality_Min, kRenderQuality_Low, kRenderQuality_Medium,
		kRenderQuality_High, kRenderQuality_Max };
	for (size_t i = 0; i < sizeof(levels) / sizeof(levels[0]); i++)
	{
		CHECK(setQuality(unit, kAudioUnitScope_Global, 0, levels[i]) == noErr);
		value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
		CHECK(status == noErr && value == levels[i]);
	}

	// Values above kRenderQuality_Max are stored as given, up to INT32_MAX.
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, 200) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == 200);
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, 0x7fffffff) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == 0x7fffffff);

	// QUIRK check (see AUHAL.cpp): a value above INT32_MAX succeeds and restores the initial value.
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, kRenderQuality_Low) == noErr);
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, 0xffffffffu) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_Medium);
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, kRenderQuality_Low) == noErr);
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, 0x80000000u) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_Medium);

	// The value is per unit, not per element: any element of the global scope is the same property.
	CHECK(setQuality(unit, kAudioUnitScope_Global, 1, 77) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == 77);
	value = getQuality(unit, kAudioUnitScope_Global, 1, &status);
	CHECK(status == noErr && value == 77);

	// Other scopes are not supported.
	for (AudioUnitScope scope = kAudioUnitScope_Input; scope <= kAudioUnitScope_Output; scope++)
	{
		for (AudioUnitElement element = 0; element <= 1; element++)
		{
			CHECK(AudioUnitGetPropertyInfo(unit, kAudioUnitProperty_RenderQuality, scope, element, &size, &writable)
				== kAudioUnitErr_InvalidScope);
			(void) getQuality(unit, scope, element, &status);
			CHECK(status == kAudioUnitErr_InvalidScope);
			CHECK(setQuality(unit, scope, element, kRenderQuality_Low) == kAudioUnitErr_InvalidScope);
		}
	}

	// A missing value is a parameter error, and nothing changes.
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, kRenderQuality_High) == noErr);
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, NULL,
		sizeof(UInt32)) == kAudio_ParamError);
	value = 5;
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, &value, 0)
		== kAudio_ParamError);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_High);

	// The value survives initialization and can be changed while initialized.
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, kRenderQuality_Max) == noErr);
	CHECK(AudioUnitInitialize(unit) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_Max);
	CHECK(setQuality(unit, kAudioUnitScope_Global, 0, kRenderQuality_Low) == noErr);
	value = getQuality(unit, kAudioUnitScope_Global, 0, &status);
	CHECK(status == noErr && value == kRenderQuality_Low);
	AudioUnitUninitialize(unit);
	AudioComponentInstanceDispose(unit);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	check_unit("HALOutput", kAudioUnitSubType_HALOutput);
	check_unit("DefaultOutput", kAudioUnitSubType_DefaultOutput);
	check_unit("SystemOutput", kAudioUnitSubType_SystemOutput);

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("PASS: kAudioUnitProperty_RenderQuality\n");
	return 0;
}
