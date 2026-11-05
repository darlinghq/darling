/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#include "AudioHardwareImplPA.h"
#include "../stub.h"
#include "AudioHardwareStreamPA.h"
#include "AudioHardwareImplPADevice.h"
#include "PADispatchMainLoop.h"
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <condition_variable>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>

AudioHardwareImplPA::AudioHardwareImplPA(AudioObjectID myId, const char* paRole)
: AudioHardwareImpl(myId), m_paRole(paRole)
{
	m_name = CFSTR("PulseAudio");
	m_manufacturer = CFSTR("PulseAudio");
	m_uid = CFSTR("PulseAudio:SystemObject");
}

AudioHardwareImplPA::~AudioHardwareImplPA()
{
	for (auto& stream : m_streams)
		stream.second->stop();
	m_streams.clear();

	if (m_context && m_loop)
	{
		m_loop->sync(^{
			pa_context_disconnect(m_context);
			pa_context_unref(m_context);
		});
		m_context = nullptr;
	}
}

void AudioHardwareImplPA::syncPA(void (^block)(void))
{
	if (m_loop)
		m_loop->sync(block);
	else
		block();
}

OSStatus AudioHardwareImplPA::start(AudioDeviceIOProcID inProcID, AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags)
{
	// Connect first, so that the queue exists: every change to m_streams happens on it.
	// A mutex wouldn't do, as an IOProc calling stop() would wait for a start() in progress,
	// which in turn needs the queue that the IOProc is blocking.
	__block bool connected = false;
	getPAContext(^(pa_context* context) {
		connected = context != nullptr;
	});
	if (!connected)
		return kAudioHardwareNotRunningError;

	__block OSStatus result = noErr;
	syncPA(^{
		result = AudioHardwareImpl::start(inProcID, ioRequestedStartTime, inFlags);
	});
	return result;
}

OSStatus AudioHardwareImplPA::stop(AudioDeviceIOProcID inProcID)
{
	__block OSStatus result = noErr;

	syncPA(^{
		auto it = m_streams.find(inProcID);
		if (it == m_streams.end())
		{
			result = kAudioHardwareNotRunningError;
			return;
		}

		AudioHardwareStream* stream = it->second.release();
		m_streams.erase(it);
		stream->stop();

		// An IOProc may stop itself. Keep its stream alive until the current
		// PulseAudio callback has unwound on this serial queue.
		if (m_loop)
			m_loop->async(^{ delete stream; });
		else
			delete stream;
	});
	return result;
}

OSStatus AudioHardwareImplPA::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyStreams:
		{
			if (id() == kAudioObjectSystemObject)
				return kAudioHardwareUnknownPropertyError;

			const bool matches = inAddress->mScope == kAudioObjectPropertyScopeGlobal
				|| inAddress->mScope == (isInputDevice() ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput);
			UInt32 size = matches ? sizeof(AudioStreamID) : 0;

			if (outData && *ioDataSize < size)
				return kAudioHardwareBadPropertySizeError;
			if (outData && size)
				*static_cast<AudioStreamID*>(outData) = kPAStreamObjectBase | id();

			*ioDataSize = size;
			return kAudioHardwareNoError;
		}
		case kAudioHardwarePropertyDeviceForUID:
		{
			if (id() != kAudioObjectSystemObject)
				break;
			if (!outData)
			{
				*ioDataSize = sizeof(AudioValueTranslation);
				return kAudioHardwareNoError;
			}
			if (*ioDataSize < sizeof(AudioValueTranslation))
				return kAudioHardwareBadPropertySizeError;

			AudioValueTranslation* translation = static_cast<AudioValueTranslation*>(outData);
			if (!translation->mInputData || translation->mInputDataSize != sizeof(CFStringRef)
				|| !translation->mOutputData || translation->mOutputDataSize < sizeof(AudioDeviceID))
				return kAudioHardwareBadPropertySizeError;

			refreshDeviceList();

			std::vector<AudioHardwareImpl*> devices;
			{
				std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
				for (auto& object : PAGetObjectMap())
				{
					if (object.first != kAudioObjectSystemObject)
						devices.push_back(object.second.get());
				}
			}

			CFStringRef wanted = *static_cast<CFStringRef*>(translation->mInputData);
			AudioDeviceID found = kAudioObjectUnknown;
			const AudioObjectPropertyAddress uidAddress = { kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };

			for (AudioHardwareImpl* device : devices)
			{
				CFStringRef uid = nullptr;
				UInt32 size = sizeof(uid);
				if (device->getPropertyData(&uidAddress, 0, nullptr, &size, &uid) == kAudioHardwareNoError && uid)
				{
					const bool match = CFEqual(uid, wanted);
					CFRelease(uid);
					if (match)
					{
						found = device->id();
						break;
					}
				}
			}

			*static_cast<AudioDeviceID*>(translation->mOutputData) = found;
			translation->mOutputDataSize = sizeof(found);
			*ioDataSize = sizeof(AudioValueTranslation);
			return kAudioHardwareNoError;
		}
		// BEGIN kAudioObjectSystemObject properties
		case kAudioHardwarePropertyDefaultInputDevice: // returns AudioDeviceID
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				*devId = kAudioObjectSystemObject + 2;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyDefaultOutputDevice: // returns AudioDeviceID
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				*devId = kAudioObjectSystemObject + 1;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyDefaultSystemOutputDevice:
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				*devId = kAudioObjectSystemObject + 3;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyDevices:
		{
			// Only the system object exposes the full device list.
			if (id() != kAudioObjectSystemObject)
				break;

			refreshDeviceList();

			std::vector<AudioObjectID> ids = allDeviceIds();
			ids.push_back(kAudioObjectSystemObject + 1); // default output
			ids.push_back(kAudioObjectSystemObject + 2); // default input
			ids.push_back(kAudioObjectSystemObject + 3); // event output

			UInt32 neededSize = UInt32(ids.size() * sizeof(AudioDeviceID));

			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= neededSize)
			{
				for (size_t i = 0; i < ids.size(); i++)
					devId[i] = ids[i];
			}
			*ioDataSize = neededSize;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeDecibelsToScalar:
		{
			if (Float32* vol = static_cast<Float32*>(outData); vol && *ioDataSize == sizeof(Float32))
			{
				*vol = pa_sw_volume_to_linear(pa_sw_volume_from_dB(*vol));
			}
			*ioDataSize = sizeof(Float32);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeScalarToDecibels:
		{
			if (Float32* vol = static_cast<Float32*>(outData); vol && *ioDataSize == sizeof(Float32))
			{
				*vol = pa_sw_volume_to_dB(pa_sw_volume_from_linear(*vol));
			}
			*ioDataSize = sizeof(Float32);
			return kAudioHardwareNoError;
		}
	}

	return AudioHardwareImpl::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}
	
OSStatus AudioHardwareImplPA::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	return AudioHardwareImpl::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

AudioHardwareStream* AudioHardwareImplPA::createStream(AudioDeviceIOProc callback, void* clientData)
{
	// Must be called on a subclass instance
	return nullptr;
}

extern "C" char ***_NSGetArgv(void);

struct PAContextReady
{
	std::mutex mutex;
	std::condition_variable condition;
	bool finished = false;
};

static void paContextStateCB(pa_context* c, void* priv)
{
	pa_context_state_t state = pa_context_get_state(c);
	if (state != PA_CONTEXT_READY && state != PA_CONTEXT_FAILED && state != PA_CONTEXT_TERMINATED)
		return;

	PAContextReady* ready = static_cast<PAContextReady*>(priv);
	std::lock_guard<std::mutex> lock(ready->mutex);
	ready->finished = true;
	ready->condition.notify_one();
}

static std::string cfStringToUTF8(CFStringRef str)
{
	char buffer[256];
	if (str && CFStringGetCString(str, buffer, sizeof(buffer), kCFStringEncodingUTF8))
		return buffer;
	return std::string();
}

static std::string appNameFromExecutable()
{
	const char* name = (*_NSGetArgv())[0];
	const char* p = std::strrchr(name, '/');

	if (p && *(p+1))
		return p+1;
	return name;
}

void AudioHardwareImplPA::getPAContext(void (^cb)(pa_context*))
{
	// Connects once, waiting for readiness, so that the callback always runs on the
	// PulseAudio queue with a context that is ready for use (or nullptr).
	std::unique_lock<std::mutex> creationLock(m_contextMutex);

	if (m_context == nullptr)
	{
		std::string appname = appNameFromExecutable();
		std::string appid = "org.darlinghq.some-app";

		pa_proplist* proplist = pa_proplist_new();

		// Try to find a better application name & id from the bundle
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		if (mainBundle)
		{
			std::string ident = cfStringToUTF8(CFBundleGetIdentifier(mainBundle));
			if (!ident.empty())
				appid = ident;

			CFDictionaryRef infoDict = CFBundleGetLocalInfoDictionary(mainBundle);
			if (!infoDict)
				infoDict = CFBundleGetInfoDictionary(mainBundle);

			if (infoDict)
			{
				CFStringRef name = (CFStringRef) CFDictionaryGetValue(infoDict, CFSTR("CFBundleDisplayName"));
				if (!name)
					name = (CFStringRef) CFDictionaryGetValue(infoDict, CFSTR("CFBundleName"));

				std::string bundleName = cfStringToUTF8(name);
				if (!bundleName.empty())
					appname = bundleName;
			}
		}

		pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, appname.c_str());
		pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, appid.c_str());
		if (m_paRole)
			pa_proplist_sets(proplist, PA_PROP_MEDIA_ROLE, m_paRole);

		m_loop.reset(new PADispatchMainLoop);

		m_context = pa_context_new_with_proplist(m_loop->getAPI(), appname.c_str(), proplist);
		pa_proplist_free(proplist);

		if (!m_context)
		{
			m_loop.reset();
			cb(nullptr);
			return;
		}

		PAContextReady ready;
		pa_context_set_state_callback(m_context, paContextStateCB, &ready);

		int connected = pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
		if (connected < 0)
			std::cerr << "pa_context_connect() returned an error\n";

		m_loop->resume();

		if (connected >= 0)
		{
			std::unique_lock<std::mutex> lock(ready.mutex);
			ready.condition.wait_for(lock, std::chrono::seconds(10), [&]{ return ready.finished; });
		}

		// `ready` is about to go out of scope.
		__block bool isReady = false;
		m_loop->sync(^{
			pa_context_set_state_callback(m_context, nullptr, nullptr);
			isReady = pa_context_get_state(m_context) == PA_CONTEXT_READY;
			if (!isReady)
			{
				pa_context_disconnect(m_context);
				pa_context_unref(m_context);
			}
		});

		if (!isReady)
		{
			// Leave nothing behind so that the next call tries to connect again.
			std::cerr << "PulseAudio error: could not connect to the server\n";
			m_context = nullptr;
			m_loop.reset();
			cb(nullptr);
			return;
		}
	}

	creationLock.unlock();

	m_loop->sync(^{
		cb(pa_context_get_state(m_context) == PA_CONTEXT_READY ? m_context : nullptr);
	});
}

pa_sample_spec AudioHardwareImplPA::paSampleSpecForASBD(const AudioStreamBasicDescription& asbd, bool* convertSignedUnsigned)
{
	const bool isFloat = asbd.mFormatFlags & kAudioFormatFlagIsFloat;
	const bool isSigned = asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger;
	const bool isBE = asbd.mFormatFlags & kAudioFormatFlagIsBigEndian;
	
	pa_sample_spec spec;
	
	spec.rate = asbd.mSampleRate;
	spec.channels = asbd.mChannelsPerFrame;
	spec.format = PA_SAMPLE_INVALID;

	if (convertSignedUnsigned)
		*convertSignedUnsigned = false;
	
	if (asbd.mFormatID == kAudioFormatLinearPCM)
	{
		if (isFloat)
			spec.format = isBE ? PA_SAMPLE_FLOAT32BE : PA_SAMPLE_FLOAT32LE;
		else
		{
			switch (asbd.mBitsPerChannel)
			{
				case 8:
					if (convertSignedUnsigned)
						*convertSignedUnsigned = isSigned;
					spec.format = PA_SAMPLE_U8;
					break;
				case 16:
					if (convertSignedUnsigned)
						*convertSignedUnsigned = !isSigned;
					
					spec.format = isBE ? PA_SAMPLE_S16BE : PA_SAMPLE_S16LE;
					break;
				case 24:
					if (convertSignedUnsigned)
						*convertSignedUnsigned = !isSigned;
					
					if (asbd.mFormatFlags & kAudioFormatFlagIsPacked)
						spec.format = isBE ? PA_SAMPLE_S24BE : PA_SAMPLE_S24LE;
					else
						spec.format = isBE ? PA_SAMPLE_S24_32BE : PA_SAMPLE_S24_32LE;
					break;
				case 32:
					if (convertSignedUnsigned)
						*convertSignedUnsigned = !isSigned;
					
					spec.format = isBE ? PA_SAMPLE_S32BE : PA_SAMPLE_S32LE;
					break;
			}
		}
	}
	
	return spec;
}

bool AudioHardwareImplPA::validateFormat(const AudioStreamBasicDescription* asbd) const
{
	pa_sample_spec spec = paSampleSpecForASBD(*asbd);
	return pa_sample_spec_valid(&spec);
}

PADeviceRegistry& AudioHardwareImplPA::registry()
{
	// +100 so dynamically discovered devices never collide with the 4 fixed
	// IDs (0..3: system object, default-output, default-input, event-output).
	static PADeviceRegistry& reg = *new PADeviceRegistry(kAudioObjectSystemObject + 100);
	return reg;
}

std::vector<AudioObjectID> AudioHardwareImplPA::allDeviceIds() const
{
	// PADeviceRegistry stays independent of CoreAudio headers and uses plain
	// uint32_t; AudioObjectID is not necessarily the same type on this target
	// (e.g. it can be `unsigned long`), so this can't be a bare return.
	std::vector<uint32_t> ids = registry().allIds();
	return std::vector<AudioObjectID>(ids.begin(), ids.end());
}

// Trampolines: PulseAudio's info-list/subscribe callbacks are plain C function
// pointers, not blocks, so (as with paContextStateCB above) we Block_copy() the
// block into the opaque userdata pointer and have a small static function
// forward the call.
static void paSinkInfoListCB(pa_context* c, const pa_sink_info* i, int eol, void* userdata)
{
	void (^cb)(const pa_sink_info*, int) = (void (^)(const pa_sink_info*, int)) userdata;
	cb(i, eol);
	if (eol)
		Block_release(cb);
}

static void paSourceInfoListCB(pa_context* c, const pa_source_info* i, int eol, void* userdata)
{
	void (^cb)(const pa_source_info*, int) = (void (^)(const pa_source_info*, int)) userdata;
	cb(i, eol);
	if (eol)
		Block_release(cb);
}

static void paSubscribeCB(pa_context* c, pa_subscription_event_type_t t, uint32_t idx, void* userdata)
{
	AudioHardwareImplPA* self = static_cast<AudioHardwareImplPA*>(userdata);
	unsigned facility = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

	if (facility == PA_SUBSCRIPTION_EVENT_SINK || facility == PA_SUBSCRIPTION_EVENT_SOURCE)
	{
		// We're being called from within PADispatchMainLoop's GCD queue here
		// (the same queue refreshDeviceList() needs to wait on), so calling
		// refreshDeviceList() directly would deadlock. Hand it off to a
		// different queue instead.
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			self->refreshDeviceList();
			self->notifyPropertyChanged(kAudioHardwarePropertyDevices);
		});
	}
	else if (facility == PA_SUBSCRIPTION_EVENT_SERVER)
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			self->notifyPropertyChanged(kAudioHardwarePropertyDefaultOutputDevice);
			self->notifyPropertyChanged(kAudioHardwarePropertyDefaultInputDevice);
		});
	}
}

void AudioHardwareImplPA::refreshDeviceList()
{
	// Clients and PulseAudio events can both trigger a refresh, and the registry
	// tracks a single refresh at a time.
	static std::mutex* refreshMutex = new std::mutex();
	std::lock_guard<std::mutex> refreshLock(*refreshMutex);

	dispatch_semaphore_t sem = dispatch_semaphore_create(0);

	registry().beginRefresh();

	getPAContext(^(pa_context* context) {
		if (!context)
		{
			dispatch_semaphore_signal(sem);
			return;
		}

		// Subscribe to sink/source changes once so the device list stays
		// current without requiring callers to re-query. Only ever meaningful
		// (and only ever reached) on the long-lived system-object instance,
		// since that's the only instance whose getPropertyData() calls
		// refreshDeviceList().
		static bool subscribed = false;
		if (!subscribed)
		{
			subscribed = true;

			pa_context_set_subscribe_callback(context, paSubscribeCB, this);

			pa_operation* opSubscribe = pa_context_subscribe(context,
				(pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE | PA_SUBSCRIPTION_MASK_SERVER), nullptr, nullptr);
			if (opSubscribe)
				pa_operation_unref(opSubscribe);
		}

		__block int pendingLists = 2;

		void (^sinkCb)(const pa_sink_info*, int) = Block_copy(^(const pa_sink_info* info, int eol) {
			if (info)
			{
				AudioObjectID devId = registry().idForName(info->name);
				std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
				auto& objects = PAGetObjectMap();
				if (objects.find(devId) == objects.end())
					objects.insert(std::make_pair(devId, std::make_unique<AudioHardwareImplPADevice>(devId, info->name, info->description, false)));
			}

			if (eol && --pendingLists == 0)
				dispatch_semaphore_signal(sem);
		});

		pa_operation* opSink = pa_context_get_sink_info_list(context, paSinkInfoListCB, sinkCb);
		if (opSink)
			pa_operation_unref(opSink);
		else
		{
			Block_release(sinkCb);
			if (--pendingLists == 0)
				dispatch_semaphore_signal(sem);
		}

		void (^sourceCb)(const pa_source_info*, int) = Block_copy(^(const pa_source_info* info, int eol) {
			// Real CoreAudio never exposes a monitor-of-sink as an input device.
			if (info && info->monitor_of_sink == PA_INVALID_INDEX)
			{
				AudioObjectID devId = registry().idForName(info->name);
				std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
				auto& objects = PAGetObjectMap();
				if (objects.find(devId) == objects.end())
					objects.insert(std::make_pair(devId, std::make_unique<AudioHardwareImplPADevice>(devId, info->name, info->description, true)));
			}

			if (eol && --pendingLists == 0)
				dispatch_semaphore_signal(sem);
		});

		pa_operation* opSource = pa_context_get_source_info_list(context, paSourceInfoListCB, sourceCb);
		if (opSource)
			pa_operation_unref(opSource);
		else
		{
			Block_release(sourceCb);
			if (--pendingLists == 0)
				dispatch_semaphore_signal(sem);
		}
	});

	// PulseAudio drops pending operations without calling back when the connection is lost.
	// The callbacks may still signal after a timeout, so the semaphore is only released on success.
	if (dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)) == 0)
		dispatch_release(sem);

	registry().endRefresh();

	// Drop any dynamically-assigned object (id >= kAudioObjectSystemObject + 100)
	// whose PulseAudio name was released by the refresh above. Clients can be in the
	// middle of a call on one, so removed objects stay allocated.
	static auto& removedObjects = *new std::vector<std::unique_ptr<AudioHardwareImpl>>();

	std::vector<uint32_t> live = registry().allIds();
	std::unordered_set<AudioObjectID> liveSet(live.begin(), live.end());

	std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
	auto& objects = PAGetObjectMap();
	for (auto it = objects.begin(); it != objects.end(); )
	{
		if (it->first >= kAudioObjectSystemObject + 100 && !liveSet.count(it->first))
		{
			removedObjects.push_back(std::move(it->second));
			it = objects.erase(it);
		}
		else
			++it;
	}
}
