// Copyright (C) 2026 Lubos Dolezel
//
// Parameter and event listeners of AudioToolbox (AudioUnitUtilities.h):
// AUEventListenerCreate(WithDispatchQueue), AUListenerCreate,
// AUEventListenerAddEventType/RemoveEventType, AUEventListenerNotify,
// AUParameterSet, AUParameterListenerNotify and AUListenerDispose.
//
// The events come from a small audio unit registered in the process with
// AudioComponentRegister: two global parameters, and a property it announces
// to its property listeners on request.
//
// The expected behaviour was observed on macOS 26.7.3 (x86_64) with Apple's
// graphic EQ unit, and this program passes unchanged there:
// - an event listener is always called on its run loop (or queue), also with
//   a notification interval of 0; a parameter listener with an interval of 0
//   is called synchronously; AUListenerCreate with no run loop uses the
//   creating thread's;
// - the event listener functions need a run loop and mode (kAudio_ParamError);
// - changes of a parameter queued within one interval are all delivered,
//   unless they follow each other within the value-change granularity, in
//   which case the later replaces the earlier;
// - a sender is not told about its own change: none of its objects when the
//   sending object is NULL, else only that object;
// - kAUParameterListener_AnyParameter reaches every registered parameter of
//   the unit, each with its current value;
// - gestures go only to gesture registrations, with a value of 0; property
//   changes cannot be sent with AUEventListenerNotify (kAudio_ParamError) and reach
//   the listener with the scope and element the unit announced;
// - registering a parameter the unit doesn't have returns the unit's error;
//   registering twice, or removing what isn't registered, returns kAudio_ParamError;
// - events still queued when the listener is disposed of are dropped.
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioUnitUtilities.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

// --- test audio unit ---

enum { kParamA = 0, kParamB = 1, kTestProperty = 64000 };

typedef struct
{
	AudioUnitPropertyListenerProc proc;
	void* userData;
	AudioUnitPropertyID property;
} PropListener;

typedef struct
{
	AudioComponentPlugInInterface iface;
	AudioComponentInstance instance;
	AudioUnitParameterValue values[2];
	PropListener listeners[16];
	int listenerCount;
} TestUnit;

static TestUnit* g_unit; // the most recently opened instance

static OSStatus tu_open(void* self, AudioComponentInstance ci)
{
	((TestUnit*) self)->instance = ci;
	g_unit = self;
	return noErr;
}

static OSStatus tu_close(void* self)
{
	free(self);
	return noErr;
}

static OSStatus tu_get_parameter(void* self, AudioUnitParameterID id, AudioUnitScope scope,
	AudioUnitElement element, AudioUnitParameterValue* value)
{
	TestUnit* u = self;
	if (scope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;
	if (element != 0)
		return kAudioUnitErr_InvalidElement;
	if (id > kParamB)
		return kAudioUnitErr_InvalidParameter;
	*value = u->values[id];
	return noErr;
}

static OSStatus tu_set_parameter(void* self, AudioUnitParameterID id, AudioUnitScope scope,
	AudioUnitElement element, AudioUnitParameterValue value, UInt32 offset)
{
	TestUnit* u = self;
	(void) offset;
	if (scope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;
	if (element != 0)
		return kAudioUnitErr_InvalidElement;
	if (id > kParamB)
		return kAudioUnitErr_InvalidParameter;
	u->values[id] = value;
	return noErr;
}

static OSStatus tu_add_listener(void* self, AudioUnitPropertyID id, AudioUnitPropertyListenerProc proc, void* userData)
{
	TestUnit* u = self;
	if (u->listenerCount == 16)
		return kAudioUnitErr_TooManyFramesToProcess;
	u->listeners[u->listenerCount++] = (PropListener) { proc, userData, id };
	return noErr;
}

static OSStatus tu_remove_listener(void* self, AudioUnitPropertyID id, AudioUnitPropertyListenerProc proc, void* userData)
{
	TestUnit* u = self;
	for (int i = 0; i < u->listenerCount; i++)
	{
		if (u->listeners[i].property == id && u->listeners[i].proc == proc && u->listeners[i].userData == userData)
		{
			u->listeners[i] = u->listeners[--u->listenerCount];
			return noErr;
		}
	}
	return noErr;
}

// A real unit describes itself: one global element with two parameters.
static OSStatus tu_property_size(AudioUnitPropertyID id, AudioUnitScope scope, AudioUnitElement element, UInt32* size)
{
	switch (id)
	{
		case kAudioUnitProperty_ElementCount:
			*size = sizeof(UInt32);
			return noErr;
		case kAudioUnitProperty_ParameterList:
			*size = scope == kAudioUnitScope_Global ? 2 * sizeof(AudioUnitParameterID) : 0;
			return noErr;
		case kAudioUnitProperty_ParameterInfo:
			if (scope != kAudioUnitScope_Global || element > kParamB)
				return kAudioUnitErr_InvalidParameter;
			*size = sizeof(AudioUnitParameterInfo);
			return noErr;
		default:
			return kAudioUnitErr_InvalidProperty;
	}
}

static OSStatus tu_get_property_info(void* self, AudioUnitPropertyID id, AudioUnitScope scope,
	AudioUnitElement element, UInt32* size, Boolean* writable)
{
	UInt32 s;
	(void) self;
	OSStatus status = tu_property_size(id, scope, element, &s);
	if (status == noErr)
	{
		if (size)
			*size = s;
		if (writable)
			*writable = false;
	}
	return status;
}

static OSStatus tu_get_property(void* self, AudioUnitPropertyID id, AudioUnitScope scope,
	AudioUnitElement element, void* data, UInt32* size)
{
	UInt32 s;
	(void) self;
	OSStatus status = tu_property_size(id, scope, element, &s);
	if (status != noErr)
		return status;
	if (*size < s)
		return kAudioUnitErr_InvalidPropertyValue;
	*size = s;

	switch (id)
	{
		case kAudioUnitProperty_ElementCount:
			*(UInt32*) data = scope == kAudioUnitScope_Global ? 1 : 0;
			break;
		case kAudioUnitProperty_ParameterList:
			if (s)
			{
				((AudioUnitParameterID*) data)[0] = kParamA;
				((AudioUnitParameterID*) data)[1] = kParamB;
			}
			break;
		case kAudioUnitProperty_ParameterInfo:
		{
			AudioUnitParameterInfo* info = data;
			memset(info, 0, sizeof(*info));
			snprintf(info->name, sizeof(info->name), "%s", element == kParamA ? "A" : "B");
			info->unit = kAudioUnitParameterUnit_Generic;
			info->minValue = 0;
			info->maxValue = 100;
			info->flags = kAudioUnitParameterFlag_IsReadable | kAudioUnitParameterFlag_IsWritable;
			break;
		}
	}
	return noErr;
}

static AudioComponentMethod tu_lookup(SInt16 selector)
{
	switch (selector)
	{
		case kAudioUnitGetPropertyInfoSelect: return (AudioComponentMethod) tu_get_property_info;
		case kAudioUnitGetPropertySelect: return (AudioComponentMethod) tu_get_property;
		case kAudioUnitGetParameterSelect: return (AudioComponentMethod) tu_get_parameter;
		case kAudioUnitSetParameterSelect: return (AudioComponentMethod) tu_set_parameter;
		case kAudioUnitAddPropertyListenerSelect: return (AudioComponentMethod) tu_add_listener;
		case kAudioUnitRemovePropertyListenerWithUserDataSelect: return (AudioComponentMethod) tu_remove_listener;
		default: return NULL;
	}
}

static AudioComponentPlugInInterface* tu_factory(const AudioComponentDescription* desc)
{
	(void) desc;
	TestUnit* u = calloc(1, sizeof(TestUnit));
	u->iface.Open = tu_open;
	u->iface.Close = tu_close;
	u->iface.Lookup = tu_lookup;
	return &u->iface;
}

// The unit announces a change of one of its properties
static int test_property_listeners(TestUnit* u)
{
	int n = 0;
	for (int i = 0; i < u->listenerCount; i++)
		if (u->listeners[i].property == kTestProperty)
			n++;
	return n;
}

static void announce(AudioUnit unit, AudioUnitPropertyID id, AudioUnitScope scope, AudioUnitElement element)
{
	int n = g_unit->listenerCount;
	PropListener copy[16];
	memcpy(copy, g_unit->listeners, sizeof(copy));
	for (int i = 0; i < n; i++)
		if (copy[i].property == id)
			copy[i].proc(copy[i].userData, unit, id, scope, element);
}

// --- recording callbacks ---

typedef struct
{
	void* object;
	AudioUnitEventType type;
	UInt32 id;           // parameter or property ID
	AudioUnitScope scope;
	AudioUnitElement element;
	AudioUnitParameterValue value;
	UInt64 hostTime;
	int onMain;
} Record;

static Record g_records[64];
static volatile int g_count;
static pthread_t g_main;
static AUEventListenerRef g_selfDisposing;

static void record(void* object, const AudioUnitEvent* e, UInt64 hostTime, AudioUnitParameterValue value)
{
	if (g_count >= 64)
		return;
	Record* r = &g_records[g_count];
	r->object = object;
	r->type = e->mEventType;
	if (e->mEventType == kAudioUnitEvent_PropertyChange)
	{
		r->id = e->mArgument.mProperty.mPropertyID;
		r->scope = e->mArgument.mProperty.mScope;
		r->element = e->mArgument.mProperty.mElement;
	}
	else
	{
		r->id = e->mArgument.mParameter.mParameterID;
		r->scope = e->mArgument.mParameter.mScope;
		r->element = e->mArgument.mParameter.mElement;
	}
	r->value = value;
	r->hostTime = hostTime;
	r->onMain = pthread_equal(pthread_self(), g_main);
	__sync_synchronize();
	g_count++;
}

static void event_proc(void* userData, void* object, const AudioUnitEvent* e, UInt64 hostTime, AudioUnitParameterValue value)
{
	record(object, e, hostTime, value);
	if (userData == (void*) 1 && g_selfDisposing)
	{
		CHECK_EQ(AUListenerDispose(g_selfDisposing), noErr);
		g_selfDisposing = NULL;
	}
}

static void param_proc(void* userData, void* object, const AudioUnitParameter* p, AudioUnitParameterValue value)
{
	(void) userData;
	AudioUnitEvent e = { kAudioUnitEvent_ParameterValueChange };
	e.mArgument.mParameter = *p;
	record(object, &e, 1, value);
}

static void reset(void)
{
	g_count = 0;
	memset(g_records, 0, sizeof(g_records));
}

static void pump(double seconds)
{
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
}

static int has_record(void* object, AudioUnitEventType type, UInt32 id, AudioUnitParameterValue value)
{
	for (int i = 0; i < g_count; i++)
		if (g_records[i].object == object && g_records[i].type == type && g_records[i].id == id && g_records[i].value == value)
			return 1;
	return 0;
}

static double seconds_since(UInt64 start)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);
	return (double) (mach_absolute_time() - start) * tb.numer / tb.denom / 1e9;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);
	g_main = pthread_self();

	AudioComponentDescription desc = { kAudioUnitType_Effect, 'aulr', 'Dtst', 0, 0 };
	AudioComponent comp = AudioComponentRegister(&desc, CFSTR("Darling test: listener unit"), 0x00010000, tu_factory);
	CHECK(comp != NULL);
	AudioUnit unit = NULL;
	CHECK_EQ(AudioComponentInstanceNew(comp, &unit), noErr);
	if (!unit)
		finish(1);
	TestUnit* state = g_unit;

	AudioUnitParameter pA = { unit, kParamA, kAudioUnitScope_Global, 0 };
	AudioUnitParameter pB = { unit, kParamB, kAudioUnitScope_Global, 0 };
	AudioUnitEvent valueA = { kAudioUnitEvent_ParameterValueChange };
	valueA.mArgument.mParameter = pA;
	AudioUnitEvent valueB = valueA;
	valueB.mArgument.mParameter = pB;
	AudioUnitEvent gestureA = { kAudioUnitEvent_BeginParameterChangeGesture };
	gestureA.mArgument.mParameter = pA;
	AudioUnitEvent propEvent = { kAudioUnitEvent_PropertyChange };
	propEvent.mArgument.mProperty.mAudioUnit = unit;
	propEvent.mArgument.mProperty.mPropertyID = kTestProperty;
	propEvent.mArgument.mProperty.mScope = kAudioUnitScope_Global;
	propEvent.mArgument.mProperty.mElement = 0;

	CFRunLoopRef rl = CFRunLoopGetCurrent();
	AUEventListenerRef L = NULL, M = NULL;

	// Argument checks
	CHECK_EQ(AUEventListenerCreate(NULL, NULL, rl, kCFRunLoopDefaultMode, 0, 0, &L), kAudio_ParamError);
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, NULL, kCFRunLoopDefaultMode, 0, 0, &L), kAudio_ParamError);
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, NULL, 0, 0, &L), kAudio_ParamError);
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0, 0, NULL), kAudio_ParamError);
	CHECK_EQ(AUListenerDispose(NULL), kAudio_ParamError);
	CHECK_EQ(AUParameterSet(NULL, NULL, NULL, 0, 0), kAudio_ParamError);

	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0, 0, &L), noErr);
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0, 0, &M), noErr);

	// Registration checks
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x100, &valueA), noErr);
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x100, &valueA), kAudio_ParamError);
	{
		AudioUnitEvent e = valueA;
		e.mArgument.mParameter.mParameterID = 7;
		CHECK_EQ(AUEventListenerAddEventType(L, NULL, &e), kAudioUnitErr_InvalidParameter);
		e = valueA;
		e.mArgument.mParameter.mElement = 2;
		CHECK_EQ(AUEventListenerAddEventType(L, NULL, &e), kAudioUnitErr_InvalidElement);
		e = valueA;
		e.mArgument.mParameter.mAudioUnit = NULL;
		CHECK_EQ(AUEventListenerAddEventType(L, NULL, &e), kAudio_ParamError);
		e = valueA;
		e.mEventType = (AudioUnitEventType) 7;
		CHECK_EQ(AUEventListenerAddEventType(L, NULL, &e), kAudio_ParamError);
		CHECK_EQ(AUEventListenerNotify(NULL, NULL, &e), noErr);
	}
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x200, &valueA), noErr);
	CHECK_EQ(AUEventListenerAddEventType(M, (void*) 0x300, &valueA), noErr);
	CHECK_EQ(AUEventListenerAddEventType(M, (void*) 0x400, &valueB), noErr);

	// A plain AudioUnitSetParameter tells nobody
	reset();
	CHECK_EQ(AudioUnitSetParameter(unit, kParamA, kAudioUnitScope_Global, 0, 0.5f, 0), noErr);
	pump(0.05);
	CHECK_EQ(g_count, 0);

	// AUParameterSet: an event listener hears about it on its run loop, not
	// synchronously, even with an interval of 0
	reset();
	CHECK_EQ(AUParameterSet(NULL, NULL, &pA, 1, 0), noErr);
	CHECK_EQ(g_count, 0);
	pump(0.05);
	CHECK_EQ(g_count, 3);
	CHECK(has_record((void*) 0x100, kAudioUnitEvent_ParameterValueChange, kParamA, 1));
	CHECK(has_record((void*) 0x200, kAudioUnitEvent_ParameterValueChange, kParamA, 1));
	CHECK(has_record((void*) 0x300, kAudioUnitEvent_ParameterValueChange, kParamA, 1));
	CHECK(g_records[0].onMain && g_records[0].hostTime != 0);
	{
		AudioUnitParameterValue v = 0;
		CHECK_EQ(AudioUnitGetParameter(unit, kParamA, kAudioUnitScope_Global, 0, &v), noErr);
		CHECK(v == 1);
	}

	// Errors of the unit are passed through
	{
		AudioUnitParameter bad = pA;
		bad.mParameterID = 9;
		CHECK_EQ(AUParameterSet(NULL, NULL, &bad, 1, 0), kAudioUnitErr_InvalidParameter);
		bad = pA;
		bad.mAudioUnit = NULL;
		CHECK_EQ(AUParameterSet(NULL, NULL, &bad, 1, 0), kAudio_ParamError);
	}

	// The sender is left out
	reset();
	AUParameterSet(L, NULL, &pA, 2, 0);
	pump(0.05);
	CHECK_EQ(g_count, 1);
	CHECK(has_record((void*) 0x300, kAudioUnitEvent_ParameterValueChange, kParamA, 2));

	reset();
	AUParameterSet(L, (void*) 0x100, &pA, 3, 0);
	pump(0.05);
	CHECK_EQ(g_count, 2);
	CHECK(has_record((void*) 0x200, kAudioUnitEvent_ParameterValueChange, kParamA, 3));
	CHECK(has_record((void*) 0x300, kAudioUnitEvent_ParameterValueChange, kParamA, 3));

	reset();
	AUParameterSet(NULL, (void*) 0x100, &pA, 4, 0);
	pump(0.05);
	CHECK_EQ(g_count, 3);

	// Other parameters are separate
	reset();
	AUParameterSet(NULL, NULL, &pB, 5, 0);
	pump(0.05);
	CHECK_EQ(g_count, 1);
	CHECK(has_record((void*) 0x400, kAudioUnitEvent_ParameterValueChange, kParamB, 5));

	// The wildcard reaches every registered parameter of the unit
	reset();
	{
		AudioUnitParameter any = { unit, kAUParameterListener_AnyParameter, kAudioUnitScope_Input, 5 };
		CHECK_EQ(AUParameterListenerNotify(NULL, NULL, &any), noErr);
		pump(0.05);
		CHECK_EQ(g_count, 4);
		CHECK(has_record((void*) 0x100, kAudioUnitEvent_ParameterValueChange, kParamA, 4));
		CHECK(has_record((void*) 0x400, kAudioUnitEvent_ParameterValueChange, kParamB, 5));
		AudioUnitParameter none = { NULL, kAUParameterListener_AnyParameter, kAudioUnitScope_Global, 0 };
		CHECK_EQ(AUParameterListenerNotify(NULL, NULL, &none), noErr);
	}

	// Gestures go to gesture registrations only, with a value of 0
	reset();
	CHECK_EQ(AUEventListenerNotify(NULL, NULL, &gestureA), noErr);
	pump(0.05);
	CHECK_EQ(g_count, 0);
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x700, &gestureA), noErr);
	reset();
	CHECK_EQ(AUEventListenerNotify(NULL, NULL, &gestureA), noErr);
	pump(0.05);
	CHECK_EQ(g_count, 1);
	CHECK(has_record((void*) 0x700, kAudioUnitEvent_BeginParameterChangeGesture, kParamA, 0));

	// A value change sent through AUEventListenerNotify
	reset();
	CHECK_EQ(AUEventListenerNotify(NULL, NULL, &valueB), noErr);
	pump(0.05);
	CHECK_EQ(g_count, 1);

	// Properties: announced by the unit, with its scope and element
	CHECK_EQ(AUEventListenerNotify(NULL, NULL, &propEvent), kAudio_ParamError);
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x800, &propEvent), noErr);
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x800, &propEvent), kAudio_ParamError);
	CHECK_EQ(test_property_listeners(state), 1);
	reset();
	announce(unit, kTestProperty, kAudioUnitScope_Output, 3);
	CHECK_EQ(g_count, 0);
	pump(0.05);
	CHECK_EQ(g_count, 1);
	CHECK(g_count == 1 && g_records[0].object == (void*) 0x800 && g_records[0].type == kAudioUnitEvent_PropertyChange
		&& g_records[0].id == kTestProperty && g_records[0].scope == kAudioUnitScope_Output
		&& g_records[0].element == 3 && g_records[0].value == 0);
	CHECK_EQ(AUEventListenerRemoveEventType(L, (void*) 0x800, &propEvent), noErr);
	CHECK_EQ(test_property_listeners(state), 0);
	reset();
	announce(unit, kTestProperty, kAudioUnitScope_Global, 0);
	pump(0.05);
	CHECK_EQ(g_count, 0);
	// Disposing of the listener takes its property listeners off the unit
	CHECK_EQ(AUEventListenerAddEventType(L, (void*) 0x800, &propEvent), noErr);
	CHECK_EQ(test_property_listeners(state), 1);

	CHECK_EQ(AUListenerDispose(L), noErr);
	CHECK_EQ(test_property_listeners(state), 0);
	CHECK_EQ(AUListenerDispose(L), kAudio_ParamError);
	CHECK_EQ(AUEventListenerAddEventType(L, NULL, &valueA), kAudio_ParamError);
	CHECK_EQ(AUEventListenerRemoveEventType(L, (void*) 0x100, &valueA), kAudio_ParamError);

	// Removing
	CHECK_EQ(AUEventListenerRemoveEventType(M, (void*) 0x300, &valueA), noErr);
	CHECK_EQ(AUEventListenerRemoveEventType(M, (void*) 0x300, &valueA), kAudio_ParamError);
	reset();
	AUParameterSet(NULL, NULL, &pA, 6, 0);
	pump(0.05);
	CHECK_EQ(g_count, 0);

	// Queued events are dropped when the listener goes away
	reset();
	AUParameterSet(NULL, NULL, &pB, 6, 0);
	CHECK_EQ(AUListenerDispose(M), noErr);
	pump(0.05);
	CHECK_EQ(g_count, 0);

	// ...also when it disposes of itself from its callback
	CHECK_EQ(AUEventListenerCreate(event_proc, (void*) 1, rl, kCFRunLoopDefaultMode, 0, 0, &g_selfDisposing), noErr);
	AUEventListenerAddEventType(g_selfDisposing, (void*) 0xa00, &valueA);
	AUEventListenerAddEventType(g_selfDisposing, (void*) 0xb00, &valueA);
	reset();
	AUParameterSet(NULL, NULL, &pA, 7, 0);
	pump(0.05);
	CHECK_EQ(g_count, 1);
	CHECK(g_selfDisposing == NULL);

	// Notification interval: the first change starts the interval, and all
	// changes queued until it ends are delivered together
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0.2f, 0, &L), noErr);
	AUEventListenerAddEventType(L, (void*) 0x100, &valueA);
	reset();
	UInt64 first = mach_absolute_time();
	for (int i = 1; i <= 5; i++)
		AUParameterSet(NULL, NULL, &pA, i, 0);
	pump(0.05);
	CHECK_EQ(g_count, 0);
	while (g_count == 0 && seconds_since(first) < 2)
		pump(0.01);
	double waited = seconds_since(first);
	CHECK(waited > 0.15 && waited < 1);
	pump(0.05);
	CHECK_EQ(g_count, 5);
	for (int i = 0; i < g_count && i < 5; i++)
		CHECK(g_records[i].value == i + 1);
	AUListenerDispose(L);

	// Value-change granularity: a change within it replaces the queued one
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0.3f, 0.05f, &L), noErr);
	AUEventListenerAddEventType(L, (void*) 0x100, &valueA);
	reset();
	AUParameterSet(NULL, NULL, &pA, 1, 0);
	AUParameterSet(NULL, NULL, &pA, 2, 0);
	usleep(100000);
	AUParameterSet(NULL, NULL, &pA, 3, 0);
	usleep(100000);
	AUParameterSet(NULL, NULL, &pA, 4, 0);
	AUParameterSet(NULL, NULL, &pA, 5, 0);
	pump(0.6);
	CHECK_EQ(g_count, 3);
	CHECK(g_records[0].value == 2 && g_records[1].value == 3 && g_records[2].value == 5);
	AUListenerDispose(L);

	// A run loop mode of the listener's own
	CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, CFSTR("DarlingTestMode"), 0, 0, &L), noErr);
	AUEventListenerAddEventType(L, (void*) 0xd00, &valueB);
	reset();
	AUParameterSet(NULL, NULL, &pB, 8, 0);
	pump(0.05);
	CHECK_EQ(g_count, 0);
	CFRunLoopRunInMode(CFSTR("DarlingTestMode"), 0.05, false);
	CHECK_EQ(g_count, 1);
	AUListenerDispose(L);

	// Parameter listeners: an interval of 0 means right away
	AUParameterListenerRef PL = NULL;
	CHECK_EQ(AUListenerCreate(NULL, NULL, NULL, NULL, 0, &PL), kAudio_ParamError);
	CHECK_EQ(AUListenerCreate(param_proc, NULL, rl, kCFRunLoopDefaultMode, 0, &PL), noErr);
	CHECK_EQ(AUListenerAddParameter(PL, (void*) 0xe00, &pB), noErr);
	CHECK_EQ(AUListenerAddParameter(PL, (void*) 0xe00, &pB), kAudio_ParamError);
	reset();
	AUParameterSet(NULL, NULL, &pB, 9, 0);
	CHECK_EQ(g_count, 1);
	CHECK(has_record((void*) 0xe00, kAudioUnitEvent_ParameterValueChange, kParamB, 9));
	CHECK_EQ(AUListenerRemoveParameter(PL, (void*) 0xe00, &pB), noErr);
	CHECK_EQ(AUListenerRemoveParameter(PL, (void*) 0xe00, &pB), kAudio_ParamError);
	AUListenerDispose(PL);

	// ...and no run loop means the creating thread's
	CHECK_EQ(AUListenerCreate(param_proc, NULL, NULL, NULL, 0.05f, &PL), noErr);
	AUListenerAddParameter(PL, (void*) 0xf00, &pB);
	reset();
	AUParameterSet(NULL, NULL, &pB, 10, 0);
	usleep(200000);
	CHECK_EQ(g_count, 0);
	pump(0.1);
	CHECK_EQ(g_count, 1);
	CHECK(g_count == 1 && g_records[0].onMain);
	AUListenerDispose(PL);

	// Dispatch queue: an event listener is called on its queue
	{
		dispatch_queue_t q = dispatch_queue_create("au-listener-test", NULL);
		dispatch_semaphore_t sem = dispatch_semaphore_create(0);
		__block int calls = 0;
		__block float seen = 0;
		__block int onMain = -1;
		CHECK_EQ(AUEventListenerCreateWithDispatchQueue(&L, 0, 0, NULL,
			^(void* o, const AudioUnitEvent* e, UInt64 t, AudioUnitParameterValue v) { (void) o; (void) e; (void) t; (void) v; }), kAudio_ParamError);
		CHECK_EQ(AUEventListenerCreateWithDispatchQueue(&L, 0, 0, q,
			^(void* o, const AudioUnitEvent* e, UInt64 t, AudioUnitParameterValue v) {
				(void) o; (void) e; (void) t;
				calls++;
				seen = v;
				onMain = pthread_equal(pthread_self(), g_main);
				dispatch_semaphore_signal(sem);
			}), noErr);
		AUEventListenerAddEventType(L, (void*) 0x100, &valueA);
		AUParameterSet(NULL, NULL, &pA, 11, 0);
		CHECK_EQ(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);
		dispatch_sync(q, ^{ });
		CHECK_EQ(calls, 1);
		CHECK(seen == 11);
		CHECK_EQ(onMain, 0);
		AUListenerDispose(L);

		__block int pcalls = 0;
		CHECK_EQ(AUListenerCreateWithDispatchQueue(&PL, 0.05f, q,
			^(void* o, const AudioUnitParameter* p, AudioUnitParameterValue v) {
				(void) o; (void) p; (void) v;
				pcalls++;
				dispatch_semaphore_signal(sem);
			}), noErr);
		AUListenerAddParameter(PL, NULL, &pB);
		AUParameterSet(NULL, NULL, &pB, 12, 0);
		CHECK_EQ(dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC)), 0);
		dispatch_sync(q, ^{ });
		CHECK_EQ(pcalls, 1);
		AUListenerDispose(PL);
	}

	// A unit disposed of before its listener
	{
		AudioUnit second = NULL;
		CHECK_EQ(AudioComponentInstanceNew(comp, &second), noErr);
		AudioUnitEvent e = propEvent;
		e.mArgument.mProperty.mAudioUnit = second;
		CHECK_EQ(AUEventListenerCreate(event_proc, NULL, rl, kCFRunLoopDefaultMode, 0, 0, &L), noErr);
		CHECK_EQ(AUEventListenerAddEventType(L, NULL, &e), noErr);
		CHECK_EQ(AudioComponentInstanceDispose(second), noErr);
		CHECK_EQ(AUListenerDispose(L), noErr);
	}
	AudioComponentInstanceDispose(unit);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}
