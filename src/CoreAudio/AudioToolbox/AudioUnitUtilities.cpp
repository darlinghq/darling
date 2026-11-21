/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Parameter and event listeners (AudioUnitUtilities.h).
//
// Listeners register interest in parameters, change gestures and properties of
// audio units. Parameter values and gestures are announced by whoever changes
// them (AUParameterSet, AUParameterListenerNotify, AUEventListenerNotify), so
// every registration lives in one process-wide registry. Property changes come
// from the audio unit itself: each property registration installs an ordinary
// property listener on the unit.
//
// Events are queued per listener and delivered on the listener's run loop or
// dispatch queue when the notification interval that the first of them
// started has passed. A parameter value
// change that follows a queued change of the same parameter more closely than
// the listener's value-change granularity replaces it.

#include <AudioToolbox/AudioUnitUtilities.h>
#include <AudioToolbox/AUComponent.h>
#include <CarbonCore/MacErrors.h>
#include <Block.h>
#include <mach/mach_time.h>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <set>
#include <vector>
#include "AudioUnitUtilitiesInternal.h"

namespace
{

struct PendingEvent
{
	void* object;
	AudioUnitEvent event;
	UInt64 hostTime;
	AudioUnitParameterValue value;
};

struct PropertyHook;

}

struct AUListenerBase
{
	// How the callback is invoked
	AUEventListenerProc eventProc = nullptr;
	AUParameterListenerProc parameterProc = nullptr;
	void* userData = nullptr;
	AUEventListenerBlock eventBlock = nullptr;
	AUParameterListenerBlock parameterBlock = nullptr;

	// Where it is invoked: a run loop, a dispatch queue, or (synchronous)
	// the thread that announced the change
	CFRunLoopRef runLoop = nullptr;
	CFStringRef runLoopMode = nullptr;
	dispatch_queue_t queue = nullptr;
	bool synchronous = false;

	double interval = 0;      // seconds between deliveries
	double granularity = 0;   // seconds within which value changes coalesce

	std::mutex lock;
	std::vector<PendingEvent> pending;
	bool flushScheduled = false;

	std::atomic<bool> disposed { false };
	std::atomic<int> refs { 1 };

	void retain() { refs++; }
	void release();
	~AUListenerBase();

	void deliver(const PendingEvent& e);
	void enqueue(const PendingEvent& e);
	void scheduleFlush(UInt64 delayNanos);
	void flush();
};

namespace
{

struct Registration
{
	AUListenerBase* listener;
	void* object;
	AudioUnitEvent event;
	PropertyHook* hook;
};

struct PropertyHook
{
	AUListenerBase* listener;
	void* object;
	AudioUnit unit;
	AudioUnitPropertyID property;
};

std::mutex& registryLock()
{
	static std::mutex m;
	return m;
}
// guarded by registryLock()
std::set<AUListenerBase*>& liveListeners()
{
	static std::set<AUListenerBase*> s;
	return s;
}
std::vector<Registration>& registrations()
{
	static std::vector<Registration> r;
	return r;
}
std::set<PropertyHook*>& liveHooks()
{
	static std::set<PropertyHook*> s;
	return s;
}

double hostTicksToSeconds(UInt64 ticks)
{
	static mach_timebase_info_data_t tb;
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	return double(ticks) * tb.numer / tb.denom / 1e9;
}

bool sameParameter(const AudioUnitParameter& a, const AudioUnitParameter& b)
{
	return a.mAudioUnit == b.mAudioUnit && a.mParameterID == b.mParameterID
		&& a.mScope == b.mScope && a.mElement == b.mElement;
}

bool sameProperty(const AudioUnitProperty& a, const AudioUnitProperty& b)
{
	return a.mAudioUnit == b.mAudioUnit && a.mPropertyID == b.mPropertyID
		&& a.mScope == b.mScope && a.mElement == b.mElement;
}

bool sameEvent(const AudioUnitEvent& a, const AudioUnitEvent& b)
{
	if (a.mEventType != b.mEventType)
		return false;
	if (a.mEventType == kAudioUnitEvent_PropertyChange)
		return sameProperty(a.mArgument.mProperty, b.mArgument.mProperty);
	return sameParameter(a.mArgument.mParameter, b.mArgument.mParameter);
}

bool isParameterEvent(AudioUnitEventType type)
{
	return type == kAudioUnitEvent_ParameterValueChange
		|| type == kAudioUnitEvent_BeginParameterChangeGesture
		|| type == kAudioUnitEvent_EndParameterChangeGesture;
}

// Called by the audio unit when one of the registered properties changes.
void propertyChanged(void* refCon, AudioUnit unit, AudioUnitPropertyID property,
	AudioUnitScope scope, AudioUnitElement element)
{
	PropertyHook* hook = static_cast<PropertyHook*>(refCon);
	AUListenerBase* listener;
	void* object;
	{
		std::lock_guard<std::mutex> guard(registryLock());
		if (liveHooks().count(hook) == 0)
			return;
		listener = hook->listener;
		object = hook->object;
		listener->retain();
	}

	PendingEvent e;
	e.object = object;
	e.event.mEventType = kAudioUnitEvent_PropertyChange;
	e.event.mArgument.mProperty.mAudioUnit = unit;
	e.event.mArgument.mProperty.mPropertyID = property;
	e.event.mArgument.mProperty.mScope = scope;
	e.event.mArgument.mProperty.mElement = element;
	e.hostTime = mach_absolute_time();
	e.value = 0;
	listener->enqueue(e);
	listener->release();
}

// Detaches a registration's property hook; called with registryLock() held.
// The unit's own listener is removed by the caller after unlocking.
PropertyHook* detachHook(Registration& r)
{
	PropertyHook* hook = r.hook;
	if (hook)
		liveHooks().erase(hook);
	r.hook = nullptr;
	return hook;
}

void removeHookFromUnit(PropertyHook* hook)
{
	if (!hook)
		return;
	if (hook->unit)
		AudioUnitRemovePropertyListenerWithUserData(hook->unit, hook->property, propertyChanged, hook);
	delete hook;
}

OSStatus notifyListeners(AUListenerBase* sendingListener, void* sendingObject, const AudioUnitEvent& event)
{
	const AudioUnitParameter& param = event.mArgument.mParameter;
	if (param.mAudioUnit == nullptr)
		return noErr;

	const bool anyParameter = param.mParameterID == kAUParameterListener_AnyParameter;
	std::vector<std::pair<AUListenerBase*, PendingEvent>> targets;
	const UInt64 now = mach_absolute_time();

	{
		std::lock_guard<std::mutex> guard(registryLock());
		for (const Registration& r : registrations())
		{
			if (r.event.mEventType != event.mEventType)
				continue;
			const AudioUnitParameter& rp = r.event.mArgument.mParameter;
			if (rp.mAudioUnit != param.mAudioUnit)
				continue;
			// The wildcard reaches every registered parameter of the unit,
			// whatever its scope and element
			if (!anyParameter && !sameParameter(rp, param))
				continue;
			// A sender does not hear its own change: all of its objects when
			// it names none, otherwise just the object it names
			if (sendingListener && r.listener == sendingListener
					&& (sendingObject == nullptr || sendingObject == r.object))
				continue;

			PendingEvent e;
			e.object = r.object;
			e.event = r.event;
			e.hostTime = now;
			e.value = 0;
			r.listener->retain();
			targets.emplace_back(r.listener, e);
		}
	}

	for (auto& t : targets)
	{
		PendingEvent& e = t.second;
		if (e.event.mEventType == kAudioUnitEvent_ParameterValueChange)
		{
			const AudioUnitParameter& p = e.event.mArgument.mParameter;
			AudioUnitGetParameter(p.mAudioUnit, p.mParameterID, p.mScope, p.mElement, &e.value);
		}
		t.first->enqueue(e);
		t.first->release();
	}
	return noErr;
}

// Looks up a listener that the caller passed in; returns it retained.
AUListenerBase* lookupListener(AUListenerBase* listener)
{
	std::lock_guard<std::mutex> guard(registryLock());
	if (!listener || liveListeners().count(listener) == 0)
		return nullptr;
	listener->retain();
	return listener;
}

OSStatus publishListener(AUListenerBase* listener, AUListenerBase** out)
{
	{
		std::lock_guard<std::mutex> guard(registryLock());
		liveListeners().insert(listener);
	}
	*out = listener;
	return noErr;
}

bool validInterval(Float32 seconds)
{
	return std::isfinite(seconds) && seconds >= 0;
}

}

void AUListenerBase::release()
{
	if (--refs == 0)
		delete this;
}

AUListenerBase::~AUListenerBase()
{
	if (eventBlock)
		Block_release(eventBlock);
	if (parameterBlock)
		Block_release(parameterBlock);
	if (runLoop)
		CFRelease(runLoop);
	if (runLoopMode)
		CFRelease(runLoopMode);
	if (queue)
		dispatch_release(queue);
}

void AUListenerBase::deliver(const PendingEvent& e)
{
	if (disposed)
		return;

	if (eventProc)
		eventProc(userData, e.object, &e.event, e.hostTime, e.value);
	else if (eventBlock)
		eventBlock(e.object, &e.event, e.hostTime, e.value);
	else if (parameterProc)
		parameterProc(userData, e.object, &e.event.mArgument.mParameter, e.value);
	else if (parameterBlock)
		parameterBlock(e.object, &e.event.mArgument.mParameter, e.value);
}

void AUListenerBase::enqueue(const PendingEvent& e)
{
	if (disposed)
		return;

	if (synchronous)
	{
		deliver(e);
		return;
	}

	std::lock_guard<std::mutex> guard(lock);

	if (granularity > 0 && e.event.mEventType == kAudioUnitEvent_ParameterValueChange)
	{
		for (auto it = pending.rbegin(); it != pending.rend(); ++it)
		{
			if (it->object != e.object || it->event.mEventType != kAudioUnitEvent_ParameterValueChange
					|| !sameParameter(it->event.mArgument.mParameter, e.event.mArgument.mParameter))
				continue;
			if (hostTicksToSeconds(e.hostTime - it->hostTime) < granularity)
			{
				it->value = e.value;
				it->hostTime = e.hostTime;
				return;
			}
			break;
		}
	}

	pending.push_back(e);

	// The first queued event starts the notification interval; everything
	// queued until it ends is delivered together
	if (!flushScheduled)
	{
		flushScheduled = true;
		scheduleFlush(UInt64(interval * 1e9));
	}
}

void AUListenerBase::scheduleFlush(UInt64 delayNanos)
{
	retain();

	if (queue)
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delayNanos), queue, ^{
			flush();
			release();
		});
		return;
	}

	CFRunLoopRef rl = runLoop;
	CFStringRef mode = runLoopMode;
	void (^post)(void) = ^{
		CFRunLoopPerformBlock(rl, mode, ^{
			flush();
			release();
		});
		CFRunLoopWakeUp(rl);
	};

	if (delayNanos == 0)
		post();
	else
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, delayNanos),
			dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), post);
}

void AUListenerBase::flush()
{
	std::vector<PendingEvent> events;
	{
		std::lock_guard<std::mutex> guard(lock);
		events.swap(pending);
		flushScheduled = false;
	}

	for (const PendingEvent& e : events)
	{
		if (disposed)
			break;
		deliver(e);
	}
}

void AUListenersForgetUnit(AudioUnit unit)
{
	std::vector<PropertyHook*> hooks;
	{
		std::lock_guard<std::mutex> guard(registryLock());
		auto& regs = registrations();
		for (auto it = regs.begin(); it != regs.end(); )
		{
			if (it->event.mArgument.mParameter.mAudioUnit == unit)
			{
				if (PropertyHook* hook = detachHook(*it))
					hooks.push_back(hook);
				it = regs.erase(it);
			}
			else
				++it;
		}
	}
	// The unit is going away, so its listener list is never consulted again
	for (PropertyHook* hook : hooks)
		delete hook;
}

extern "C" {

OSStatus AUEventListenerCreate(AUEventListenerProc inProc, void* inUserData,
	CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode, Float32 inNotificationInterval,
	Float32 inValueChangeGranularity, AUEventListenerRef* outListener)
{
	if (!inProc || !inRunLoop || !inRunLoopMode || !outListener
			|| !validInterval(inNotificationInterval) || !validInterval(inValueChangeGranularity))
		return paramErr;

	AUListenerBase* l = new AUListenerBase;
	l->eventProc = inProc;
	l->userData = inUserData;
	l->runLoop = (CFRunLoopRef) CFRetain(inRunLoop);
	l->runLoopMode = CFStringCreateCopy(nullptr, inRunLoopMode);
	l->interval = inNotificationInterval;
	l->granularity = inValueChangeGranularity;
	return publishListener(l, outListener);
}

OSStatus AUEventListenerCreateWithDispatchQueue(AUEventListenerRef* outListener,
	Float32 inNotificationInterval, Float32 inValueChangeGranularity,
	dispatch_queue_t inDispatchQueue, AUEventListenerBlock inBlock)
{
	if (!outListener || !inDispatchQueue || !inBlock
			|| !validInterval(inNotificationInterval) || !validInterval(inValueChangeGranularity))
		return paramErr;

	AUListenerBase* l = new AUListenerBase;
	l->eventBlock = Block_copy(inBlock);
	dispatch_retain(inDispatchQueue);
	l->queue = inDispatchQueue;
	l->interval = inNotificationInterval;
	l->granularity = inValueChangeGranularity;
	return publishListener(l, outListener);
}

OSStatus AUListenerCreate(AUParameterListenerProc inProc, void* inUserData,
	CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode, Float32 inNotificationInterval,
	AUParameterListenerRef* outListener)
{
	if (!inProc || !outListener || !validInterval(inNotificationInterval))
		return paramErr;

	AUListenerBase* l = new AUListenerBase;
	l->parameterProc = inProc;
	l->userData = inUserData;
	// With no interval, a parameter listener is called right away, on the
	// thread that announced the change. Otherwise a missing run loop means
	// the creating thread's.
	l->synchronous = inNotificationInterval == 0;
	l->runLoop = (CFRunLoopRef) CFRetain(inRunLoop ? inRunLoop : CFRunLoopGetCurrent());
	l->runLoopMode = inRunLoopMode ? CFStringCreateCopy(nullptr, inRunLoopMode)
		: (CFStringRef) CFRetain(kCFRunLoopDefaultMode);
	l->interval = inNotificationInterval;
	return publishListener(l, outListener);
}

OSStatus AUListenerCreateWithDispatchQueue(AUParameterListenerRef* outListener,
	Float32 inNotificationInterval, dispatch_queue_t inDispatchQueue,
	AUParameterListenerBlock inBlock)
{
	if (!outListener || !inDispatchQueue || !inBlock || !validInterval(inNotificationInterval))
		return paramErr;

	AUListenerBase* l = new AUListenerBase;
	l->parameterBlock = Block_copy(inBlock);
	dispatch_retain(inDispatchQueue);
	l->queue = inDispatchQueue;
	l->synchronous = inNotificationInterval == 0;
	l->interval = inNotificationInterval;
	return publishListener(l, outListener);
}

OSStatus AUListenerDispose(AUParameterListenerRef inListener)
{
	std::vector<PropertyHook*> hooks;
	{
		std::lock_guard<std::mutex> guard(registryLock());
		if (!inListener || liveListeners().erase(inListener) == 0)
			return paramErr;

		auto& regs = registrations();
		for (auto it = regs.begin(); it != regs.end(); )
		{
			if (it->listener == inListener)
			{
				if (PropertyHook* hook = detachHook(*it))
					hooks.push_back(hook);
				it = regs.erase(it);
			}
			else
				++it;
		}
	}

	for (PropertyHook* hook : hooks)
		removeHookFromUnit(hook);

	// Events that are already queued are dropped, also when the listener
	// disposes of itself from its own callback
	inListener->disposed = true;
	inListener->release();
	return noErr;
}

OSStatus AUEventListenerAddEventType(AUEventListenerRef inListener, void* inObject,
	const AudioUnitEvent* inEvent)
{
	if (!inEvent)
		return paramErr;
	if (!isParameterEvent(inEvent->mEventType) && inEvent->mEventType != kAudioUnitEvent_PropertyChange)
		return paramErr;

	AUListenerBase* listener = lookupListener(inListener);
	if (!listener)
		return paramErr;

	OSStatus status = noErr;
	PropertyHook* hook = nullptr;

	if (isParameterEvent(inEvent->mEventType))
	{
		// The parameter has to exist on the unit
		const AudioUnitParameter& p = inEvent->mArgument.mParameter;
		AudioUnitParameterValue value;
		if (!p.mAudioUnit)
			status = paramErr;
		else
			status = AudioUnitGetParameter(p.mAudioUnit, p.mParameterID, p.mScope, p.mElement, &value);
	}
	else if (inEvent->mArgument.mProperty.mAudioUnit)
	{
		hook = new PropertyHook { listener, inObject, inEvent->mArgument.mProperty.mAudioUnit,
			inEvent->mArgument.mProperty.mPropertyID };
	}

	if (status == noErr)
	{
		std::lock_guard<std::mutex> guard(registryLock());
		bool duplicate = false;
		for (const Registration& r : registrations())
		{
			if (r.listener == listener && r.object == inObject && sameEvent(r.event, *inEvent))
			{
				duplicate = true;
				break;
			}
		}

		if (duplicate || liveListeners().count(listener) == 0)
			status = paramErr;
		else
		{
			registrations().push_back(Registration { listener, inObject, *inEvent, hook });
			if (hook)
				liveHooks().insert(hook);
		}
	}

	if (status == noErr && hook)
	{
		status = AudioUnitAddPropertyListener(hook->unit, hook->property, propertyChanged, hook);
		if (status != noErr)
		{
			std::lock_guard<std::mutex> guard(registryLock());
			liveHooks().erase(hook);
			auto& regs = registrations();
			regs.erase(std::remove_if(regs.begin(), regs.end(),
				[hook](const Registration& r) { return r.hook == hook; }), regs.end());
		}
		else
			hook = nullptr;
	}

	delete hook;
	listener->release();
	return status;
}

OSStatus AUEventListenerRemoveEventType(AUEventListenerRef inListener, void* inObject,
	const AudioUnitEvent* inEvent)
{
	if (!inListener || !inEvent)
		return paramErr;

	PropertyHook* hook = nullptr;
	{
		std::lock_guard<std::mutex> guard(registryLock());
		if (liveListeners().count(inListener) == 0)
			return paramErr;

		auto& regs = registrations();
		auto it = std::find_if(regs.begin(), regs.end(), [&](const Registration& r) {
			return r.listener == inListener && r.object == inObject && sameEvent(r.event, *inEvent);
		});
		if (it == regs.end())
			return paramErr;

		hook = detachHook(*it);
		regs.erase(it);
	}

	removeHookFromUnit(hook);
	return noErr;
}

OSStatus AUEventListenerNotify(AUEventListenerRef inSendingListener, void* inSendingObject,
	const AudioUnitEvent* inEvent)
{
	if (!inEvent)
		return paramErr;
	// Property changes are announced by the audio unit itself
	if (inEvent->mEventType == kAudioUnitEvent_PropertyChange)
		return paramErr;
	if (!isParameterEvent(inEvent->mEventType))
		return noErr;

	return notifyListeners(inSendingListener, inSendingObject, *inEvent);
}

OSStatus AUListenerAddParameter(AUParameterListenerRef inListener, void* inObject,
	const AudioUnitParameter* inParameter)
{
	if (!inParameter)
		return paramErr;

	AudioUnitEvent e = {};
	e.mEventType = kAudioUnitEvent_ParameterValueChange;
	e.mArgument.mParameter = *inParameter;
	return AUEventListenerAddEventType(inListener, inObject, &e);
}

OSStatus AUListenerRemoveParameter(AUParameterListenerRef inListener, void* inObject,
	const AudioUnitParameter* inParameter)
{
	if (!inParameter)
		return paramErr;

	AudioUnitEvent e = {};
	e.mEventType = kAudioUnitEvent_ParameterValueChange;
	e.mArgument.mParameter = *inParameter;
	return AUEventListenerRemoveEventType(inListener, inObject, &e);
}

OSStatus AUParameterListenerNotify(AUParameterListenerRef inSendingListener, void* inSendingObject,
	const AudioUnitParameter* inParameter)
{
	if (!inParameter)
		return paramErr;

	AudioUnitEvent e = {};
	e.mEventType = kAudioUnitEvent_ParameterValueChange;
	e.mArgument.mParameter = *inParameter;
	return notifyListeners(inSendingListener, inSendingObject, e);
}

OSStatus AUParameterSet(AUParameterListenerRef inSendingListener, void* inSendingObject,
	const AudioUnitParameter* inParameter, AudioUnitParameterValue inValue,
	UInt32 inBufferOffsetInFrames)
{
	if (!inParameter || !inParameter->mAudioUnit)
		return paramErr;

	OSStatus status = AudioUnitSetParameter(inParameter->mAudioUnit, inParameter->mParameterID,
		inParameter->mScope, inParameter->mElement, inValue, inBufferOffsetInFrames);
	if (status != noErr)
		return status;

	return AUParameterListenerNotify(inSendingListener, inSendingObject, inParameter);
}

// Value <-> slider position mapping follows the parameter's display flags.

static bool getParameterInfo(const AudioUnitParameter* p, AudioUnitParameterInfo& info)
{
	if (!p || !p->mAudioUnit)
		return false;
	UInt32 size = sizeof(info);
	return AudioUnitGetProperty(p->mAudioUnit, kAudioUnitProperty_ParameterInfo,
		p->mScope, p->mParameterID, &info, &size) == noErr;
}

AudioUnitParameterValue AUParameterValueFromLinear(Float32 inLinearValue, const AudioUnitParameter* inParameter)
{
	AudioUnitParameterInfo info;
	if (!getParameterInfo(inParameter, info))
		return 0;

	const double x = inLinearValue;
	const double lo = info.minValue, hi = info.maxValue, range = hi - lo;

	if (info.flags & kAudioUnitParameterFlag_DisplayLogarithmic)
	{
		if (lo > 0 && hi > 0)
			return lo * pow(hi / lo, x);
		return lo + range * x;
	}

	switch (GetAudioUnitParameterDisplayType(info.flags))
	{
		case kAudioUnitParameterFlag_DisplaySquareRoot:
			return lo + range * x * x;
		case kAudioUnitParameterFlag_DisplaySquared:
			return lo + range * sqrt(x);
		case kAudioUnitParameterFlag_DisplayCubed:
			return lo + range * cbrt(x);
		case kAudioUnitParameterFlag_DisplayCubeRoot:
			return lo + range * x * x * x;
		case kAudioUnitParameterFlag_DisplayExponential:
			return log(exp(lo) + x * (exp(hi) - exp(lo)));
		default:
			return lo + range * x;
	}
}

Float32 AUParameterValueToLinear(AudioUnitParameterValue inParameterValue, const AudioUnitParameter* inParameter)
{
	AudioUnitParameterInfo info;
	if (!getParameterInfo(inParameter, info))
		return 0;

	const double v = inParameterValue;
	const double lo = info.minValue, hi = info.maxValue, range = hi - lo;
	if (range == 0)
		return 0;
	const double n = (v - lo) / range;

	if (info.flags & kAudioUnitParameterFlag_DisplayLogarithmic)
	{
		if (lo > 0 && hi > 0 && v > 0)
			return log(v / lo) / log(hi / lo);
		return n;
	}

	switch (GetAudioUnitParameterDisplayType(info.flags))
	{
		case kAudioUnitParameterFlag_DisplaySquareRoot:
			return n >= 0 ? sqrt(n) : 0;
		case kAudioUnitParameterFlag_DisplaySquared:
			return n * n;
		case kAudioUnitParameterFlag_DisplayCubed:
			return n * n * n;
		case kAudioUnitParameterFlag_DisplayCubeRoot:
			return cbrt(n);
		case kAudioUnitParameterFlag_DisplayExponential:
			return (exp(v) - exp(lo)) / (exp(hi) - exp(lo));
		default:
			return n;
	}
}

// Digits in front of the decimal point, counting at least one.
static int integerDigits(double v)
{
	v = fabs(v);
	if (v < 10)
		return 1;
	return int(floor(log10(v))) + 1;
}

char* AUParameterFormatValue(Float64 inParameterValue, const AudioUnitParameter* inParameter,
	char* inTextBuffer, UInt32 inDigits)
{
	// The buffer has room for 32 characters
	const size_t bufferSize = 32;
	if (!inTextBuffer)
		return inTextBuffer;

	AudioUnitParameterInfo info;
	if (!getParameterInfo(inParameter, info) || !std::isfinite(inParameterValue))
	{
		inTextBuffer[0] = '\0';
		return inTextBuffer;
	}

	// inDigits is the number of significant digits. For a logarithmic
	// parameter they count from the value itself; for any other parameter
	// from the largest magnitude in its range, so that every value of the
	// parameter shows the same number of decimals.
	int decimals;
	if (info.flags & kAudioUnitParameterFlag_DisplayLogarithmic)
	{
		if (inParameterValue == 0)
			decimals = int(inDigits) + 5;
		else
			decimals = int(inDigits) - integerDigits(inParameterValue);
	}
	else
	{
		double magnitude = std::max(fabs(info.minValue), fabs(info.maxValue));
		decimals = int(inDigits) - integerDigits(magnitude);
	}
	decimals = std::max(0, std::min(decimals, 15));

	if (snprintf(inTextBuffer, bufferSize, "%.*f", decimals, inParameterValue) >= int(bufferSize))
		snprintf(inTextBuffer, bufferSize, "%g", inParameterValue);
	return inTextBuffer;
}

}
