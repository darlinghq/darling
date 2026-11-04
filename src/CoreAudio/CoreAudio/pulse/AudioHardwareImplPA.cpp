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

AudioHardwareImplPA::AudioHardwareImplPA(AudioObjectID myId, const char* paRole)
: AudioHardwareImpl(myId), m_paRole(paRole)
{
	m_name = CFSTR("PulseAudio");
	m_manufacturer = CFSTR("PulseAudio");
	m_uid = CFSTR("PulseAudio:SystemObject");
}

AudioHardwareImplPA::~AudioHardwareImplPA()
{
	if (m_context)
		pa_context_unref(m_context);
}

OSStatus AudioHardwareImplPA::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	switch (inAddress->mSelector)
	{
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

static void paContextStateCB(pa_context* c, void* priv)
{
	void (^cb)(pa_context*) = (void (^)(pa_context*)) priv;

	pa_context_state_t state = pa_context_get_state(c);
	if (state == PA_CONTEXT_READY)
	{
		// std::cout << "PA_CONTEXT_READY\n";

		cb(c);
		Block_release(cb);

		// FIXME: We won't be notified about later disconnects...
		pa_context_set_state_callback(c, nullptr, nullptr);
	}
	else if (state == PA_CONTEXT_FAILED)
	{
		std::cerr << "PulseAudio error: PA_CONTEXT_FAILED\n";

		cb(nullptr);
		Block_release(cb);
		pa_context_set_state_callback(c, nullptr, nullptr);
	}
}

static const char* appNameFromExecutable()
{
	const char* name = (*_NSGetArgv())[0];
	const char* p = std::strrchr(name, '/');

	if (p && *(p+1))
		return p+1;
	return name;
}

void AudioHardwareImplPA::getPAContext(void (^cb)(pa_context*))
{
	// TODO: destruction
	static std::mutex mutex;

	if (m_context == nullptr)
	{
		std::unique_lock<std::mutex> l(mutex);

		if (m_context == nullptr)
		{
			const char* appname = appNameFromExecutable();
			const char* appid = "org.darlinghq.some-app";

			pa_proplist* proplist = pa_proplist_new();

			// Try to find a better application name & id from the bundle
			CFBundleRef mainBundle = CFBundleGetMainBundle();
			if (mainBundle)
			{
				CFStringRef ident = CFBundleGetIdentifier(mainBundle);
				if (ident)
					appid = CFStringGetCStringPtr(ident, kCFStringEncodingUTF8);

				CFDictionaryRef infoDict = CFBundleGetLocalInfoDictionary(mainBundle);
				if (!infoDict)
					infoDict = CFBundleGetInfoDictionary(mainBundle);
				
				if (infoDict)
				{
					CFStringRef name = (CFStringRef) CFDictionaryGetValue(infoDict, CFSTR("CFBundleDisplayName"));
					if (!name)
						name = (CFStringRef) CFDictionaryGetValue(infoDict, CFSTR("CFBundleName"));
					if (name)
						appname = CFStringGetCStringPtr(name, kCFStringEncodingUTF8);
				}
			}

			pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, appname);
			pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, appid);
			if (m_paRole)
				pa_proplist_sets(proplist, PA_PROP_MEDIA_ROLE, m_paRole);
			// pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "icon-name");
			// pa_proplist_sets(proplist, PA_PROP_MEDIA_ROLE, "game");

			m_loop.reset(new PADispatchMainLoop);

			m_context = pa_context_new_with_proplist(m_loop->getAPI(), appname, proplist);
			pa_proplist_free(proplist);

			if (!m_context)
			{
				cb(nullptr);
				return;
			}

			pa_context_set_state_callback(m_context, paContextStateCB, Block_copy(cb));

			if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0)
			{
				std::cerr << "pa_context_connect() returned an error\n";
				pa_context_set_state_callback(m_context, nullptr, nullptr);
				cb(nullptr);
				return;
			}

			m_loop->resume();
		}
	}
	else
		cb(m_context);
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
	static PADeviceRegistry reg(kAudioObjectSystemObject + 100);
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
				auto& objects = PAGetObjectMap();
				if (objects.find(devId) == objects.end())
					objects.insert(std::make_pair(devId, std::make_unique<AudioHardwareImplPADevice>(devId, info->name, false)));
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
				auto& objects = PAGetObjectMap();
				if (objects.find(devId) == objects.end())
					objects.insert(std::make_pair(devId, std::make_unique<AudioHardwareImplPADevice>(devId, info->name, true)));
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

	dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
	dispatch_release(sem);

	registry().endRefresh();

	// Drop any dynamically-assigned object (id >= kAudioObjectSystemObject + 100)
	// whose PulseAudio name was released by the refresh above.
	std::vector<uint32_t> live = registry().allIds();
	std::unordered_set<AudioObjectID> liveSet(live.begin(), live.end());

	auto& objects = PAGetObjectMap();
	for (auto it = objects.begin(); it != objects.end(); )
	{
		if (it->first >= kAudioObjectSystemObject + 100 && !liveSet.count(it->first))
			it = objects.erase(it);
		else
			++it;
	}
}
