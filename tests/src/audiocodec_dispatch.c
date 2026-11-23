// Copyright (C) 2026 Lubos Dolezel
//
// The AudioCodec entry points of AudioToolbox hand each call, with its arguments, to the
// selector of the codec component instance (AudioCodec.h). A component registered with
// AudioComponentRegister() implements the codec selectors; every AudioCodec call must reach
// the matching one with the caller's arguments and return its result. A selector the component
// does not implement makes the call fail.
#include <AudioToolbox/AudioComponent.h>
#include <AudioToolbox/AudioCodec.h>
#include <CoreFoundation/CoreFoundation.h>
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

typedef struct
{
	AudioComponentPlugInInterface iface;
	void* self_seen[16];
} Codec;

static struct { int calls; SInt16 last; void* a[6]; UInt32 u[6]; } g_seen;

#define RESULT(sel) ((OSStatus) (1000 + (sel)))

static OSStatus open_cb(void* self, AudioComponentInstance inst) { (void) self; (void) inst; return noErr; }
static OSStatus close_cb(void* self) { (void) self; return noErr; }

static OSStatus m_getinfo(void* self, UInt32 prop, UInt32* size, Boolean* writable)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecGetPropertyInfoSelect; g_seen.u[0] = prop;
	g_seen.a[0] = size; g_seen.a[1] = writable;
	return RESULT(kAudioCodecGetPropertyInfoSelect);
}
static OSStatus m_get(void* self, UInt32 prop, UInt32* size, void* data)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecGetPropertySelect; g_seen.u[0] = prop;
	g_seen.a[0] = size; g_seen.a[1] = data;
	return RESULT(kAudioCodecGetPropertySelect);
}
static OSStatus m_set(void* self, UInt32 prop, UInt32 size, const void* data)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecSetPropertySelect; g_seen.u[0] = prop; g_seen.u[1] = size;
	g_seen.a[0] = (void*) data;
	return RESULT(kAudioCodecSetPropertySelect);
}
static OSStatus m_init(void* self, const AudioStreamBasicDescription* in, const AudioStreamBasicDescription* out, const void* cookie, UInt32 cookieSize)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecInitializeSelect; g_seen.a[0] = (void*) in; g_seen.a[1] = (void*) out;
	g_seen.a[2] = (void*) cookie; g_seen.u[0] = cookieSize;
	return RESULT(kAudioCodecInitializeSelect);
}
static OSStatus m_uninit(void* self) { (void) self; g_seen.calls++; g_seen.last = kAudioCodecUninitializeSelect; return RESULT(kAudioCodecUninitializeSelect); }
static OSStatus m_append(void* self, const void* data, UInt32* size, UInt32* packets, const AudioStreamPacketDescription* desc)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecAppendInputDataSelect; g_seen.a[0] = (void*) data; g_seen.a[1] = size;
	g_seen.a[2] = packets; g_seen.a[3] = (void*) desc;
	return RESULT(kAudioCodecAppendInputDataSelect);
}
static OSStatus m_produce(void* self, void* data, UInt32* size, UInt32* packets, AudioStreamPacketDescription* desc, UInt32* status)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecProduceOutputDataSelect; g_seen.a[0] = data; g_seen.a[1] = size;
	g_seen.a[2] = packets; g_seen.a[3] = desc; g_seen.a[4] = status;
	return RESULT(kAudioCodecProduceOutputDataSelect);
}
static OSStatus m_appendlist(void* self, const AudioBufferList* list, UInt32* packets, const AudioStreamPacketDescription* desc, UInt32* consumed)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecAppendInputBufferListSelect; g_seen.a[0] = (void*) list; g_seen.a[1] = packets;
	g_seen.a[2] = (void*) desc; g_seen.a[3] = consumed;
	return RESULT(kAudioCodecAppendInputBufferListSelect);
}
static OSStatus m_producelist(void* self, AudioBufferList* list, UInt32* packets, AudioStreamPacketDescription* desc, UInt32* status)
{
	(void) self; g_seen.calls++; g_seen.last = kAudioCodecProduceOutputBufferListSelect; g_seen.a[0] = list; g_seen.a[1] = packets;
	g_seen.a[2] = desc; g_seen.a[3] = status;
	return RESULT(kAudioCodecProduceOutputBufferListSelect);
}
static OSStatus m_reset(void* self) { (void) self; g_seen.calls++; g_seen.last = kAudioCodecResetSelect; return RESULT(kAudioCodecResetSelect); }

static AudioComponentMethod lookup_cb(SInt16 sel)
{
	switch (sel)
	{
		case kAudioCodecGetPropertyInfoSelect: return (AudioComponentMethod) m_getinfo;
		case kAudioCodecGetPropertySelect: return (AudioComponentMethod) m_get;
		case kAudioCodecSetPropertySelect: return (AudioComponentMethod) m_set;
		case kAudioCodecInitializeSelect: return (AudioComponentMethod) m_init;
		case kAudioCodecUninitializeSelect: return (AudioComponentMethod) m_uninit;
		case kAudioCodecAppendInputDataSelect: return (AudioComponentMethod) m_append;
		case kAudioCodecProduceOutputDataSelect: return (AudioComponentMethod) m_produce;
		case kAudioCodecAppendInputBufferListSelect: return (AudioComponentMethod) m_appendlist;
		case kAudioCodecProduceOutputBufferListSelect: return (AudioComponentMethod) m_producelist;
		case kAudioCodecResetSelect: return (AudioComponentMethod) m_reset;
	}
	return NULL;
}

// A component that implements nothing.
static AudioComponentMethod lookup_none_cb(SInt16 sel)
{
	(void) sel;
	return NULL;
}

static AudioComponentPlugInInterface g_iface = { open_cb, close_cb, lookup_cb, NULL };

static AudioComponentPlugInInterface g_iface_none = { open_cb, close_cb, lookup_none_cb, NULL };

static AudioComponentPlugInInterface* factory(void)
{
	return &g_iface;
}

static AudioComponentPlugInInterface* factory_none(void)
{
	return &g_iface_none;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	AudioComponentDescription desc = { 'adec', 'dcts', 'Dtst', 0, 0 };
	AudioComponent comp = AudioComponentRegister(&desc, CFSTR("Darling test: codec"), 0x00010000, (AudioComponentFactoryFunction) factory);
	CHECK(comp != NULL);
	AudioComponentInstance codec = NULL;
	CHECK(AudioComponentInstanceNew(comp, &codec) == noErr && codec != NULL);
	if (codec == NULL)
		return 1;

	UInt32 u1 = 11, u2 = 22, u3 = 33, u4 = 44;
	Boolean writable = 0;
	char data[8] = "abc";
	AudioStreamBasicDescription in, out;
	memset(&in, 0, sizeof in);
	memset(&out, 0, sizeof out);
	AudioStreamPacketDescription pd;
	memset(&pd, 0, sizeof pd);
	AudioBufferList list;
	memset(&list, 0, sizeof list);

	memset(&g_seen, 0, sizeof g_seen);
	CHECK(AudioCodecGetPropertyInfo(codec, 0x12345678, &u1, &writable) == RESULT(kAudioCodecGetPropertyInfoSelect));
	CHECK(g_seen.last == kAudioCodecGetPropertyInfoSelect && g_seen.u[0] == 0x12345678 && g_seen.a[0] == &u1 && g_seen.a[1] == &writable);

	CHECK(AudioCodecGetProperty(codec, 0x01020304, &u2, data) == RESULT(kAudioCodecGetPropertySelect));
	CHECK(g_seen.last == kAudioCodecGetPropertySelect && g_seen.u[0] == 0x01020304 && g_seen.a[0] == &u2 && g_seen.a[1] == data);

	CHECK(AudioCodecSetProperty(codec, 0x0A0B0C0D, 8, data) == RESULT(kAudioCodecSetPropertySelect));
	CHECK(g_seen.last == kAudioCodecSetPropertySelect && g_seen.u[0] == 0x0A0B0C0D && g_seen.u[1] == 8 && g_seen.a[0] == data);

	CHECK(AudioCodecInitialize(codec, &in, &out, data, 3) == RESULT(kAudioCodecInitializeSelect));
	CHECK(g_seen.last == kAudioCodecInitializeSelect && g_seen.a[0] == &in && g_seen.a[1] == &out && g_seen.a[2] == data && g_seen.u[0] == 3);

	CHECK(AudioCodecAppendInputData(codec, data, &u1, &u2, &pd) == RESULT(kAudioCodecAppendInputDataSelect));
	CHECK(g_seen.last == kAudioCodecAppendInputDataSelect && g_seen.a[0] == data && g_seen.a[1] == &u1 && g_seen.a[2] == &u2 && g_seen.a[3] == &pd);

	CHECK(AudioCodecProduceOutputPackets(codec, data, &u1, &u2, &pd, &u3) == RESULT(kAudioCodecProduceOutputDataSelect));
	CHECK(g_seen.last == kAudioCodecProduceOutputDataSelect && g_seen.a[0] == data && g_seen.a[1] == &u1 && g_seen.a[2] == &u2 && g_seen.a[3] == &pd && g_seen.a[4] == &u3);

	CHECK(AudioCodecAppendInputBufferList(codec, &list, &u1, &pd, &u4) == RESULT(kAudioCodecAppendInputBufferListSelect));
	CHECK(g_seen.last == kAudioCodecAppendInputBufferListSelect && g_seen.a[0] == &list && g_seen.a[1] == &u1 && g_seen.a[2] == &pd && g_seen.a[3] == &u4);

	CHECK(AudioCodecProduceOutputBufferList(codec, &list, &u1, &pd, &u3) == RESULT(kAudioCodecProduceOutputBufferListSelect));
	CHECK(g_seen.last == kAudioCodecProduceOutputBufferListSelect && g_seen.a[0] == &list && g_seen.a[1] == &u1 && g_seen.a[2] == &pd && g_seen.a[3] == &u3);

	CHECK(AudioCodecUninitialize(codec) == RESULT(kAudioCodecUninitializeSelect));
	CHECK(AudioCodecReset(codec) == RESULT(kAudioCodecResetSelect));
	CHECK(g_seen.calls == 10);

	// A selector the component does not implement is an error and never reaches the component.
	AudioComponentDescription descNone = { 'adec', 'none', 'Dtst', 0, 0 };
	AudioComponent compNone = AudioComponentRegister(&descNone, CFSTR("Darling test: empty codec"), 0x00010000, (AudioComponentFactoryFunction) factory_none);
	AudioComponentInstance empty = NULL;
	CHECK(compNone != NULL && AudioComponentInstanceNew(compNone, &empty) == noErr && empty != NULL);
	int before = g_seen.calls;
	OSStatus unimplemented = AudioCodecReset(empty);
	printf("unimplemented selector returned %d\n", (int) unimplemented);
	CHECK(unimplemented != noErr && g_seen.calls == before);
	if (empty != NULL)
		AudioComponentInstanceDispose(empty);

	AudioComponentInstanceDispose(codec);
	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
