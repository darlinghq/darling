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
#include "AudioHardwareStreamPAOutput.h"
#include "AudioHardwareImplPADevice.h"
#include "PADispatchMainLoop.h"
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <condition_variable>
#include <cstring>
#include <chrono>
#include <unistd.h>
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
	refreshFormat();

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
	if (result == noErr)
		ioStateChanged();
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
	if (result == noErr)
		ioStateChanged();
	return result;
}

// The settings of the process that the system object keeps. Nothing but this process reads them:
// the server has no use for them, so they are what the process last set.
static std::atomic<UInt32> g_processIsMain{0}, g_sleepingIsAllowed{1}, g_unloadingIsAllowed{1}, g_hogModeIsAllowed{1},
	g_processIsAudible{1}, g_powerHint{0}, g_mixStereoToMono{0};

bool PAHogModeIsAllowed()
{
	return g_hogModeIsAllowed.load() != 0;
}

static std::atomic<UInt32>* systemSetting(AudioObjectPropertySelector selector)
{
	switch (selector)
	{
		case kAudioHardwarePropertyProcessIsMaster:
		case kAudioHardwarePropertyProcessIsMain: return &g_processIsMain;
		case kAudioHardwarePropertySleepingIsAllowed: return &g_sleepingIsAllowed;
		case kAudioHardwarePropertyUnloadingIsAllowed: return &g_unloadingIsAllowed;
		case kAudioHardwarePropertyHogModeIsAllowed: return &g_hogModeIsAllowed;
		case kAudioHardwarePropertyProcessIsAudible: return &g_processIsAudible;
		case kAudioHardwarePropertyPowerHint: return &g_powerHint;
		case kAudioHardwarePropertyMixStereoToMono: return &g_mixStereoToMono;
		default: return nullptr;
	}
}

OSStatus AudioHardwareImplPA::isPropertySettable(const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable)
{
	if (id() == kAudioObjectSystemObject && systemSetting(inAddress->mSelector))
	{
		*outIsSettable = true;
		return kAudioHardwareNoError;
	}
	return AudioHardwareImpl::isPropertySettable(inAddress, outIsSettable);
}

OSStatus AudioHardwareImplPA::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	if (id() == kAudioObjectSystemObject)
	{
		auto replyValue = [&](UInt32 value) {
			if (outData)
			{
				if (*ioDataSize < sizeof(value))
					return OSStatus(kAudioHardwareBadPropertySizeError);
				*static_cast<UInt32*>(outData) = value;
			}
			*ioDataSize = sizeof(value);
			return OSStatus(kAudioHardwareNoError);
		};

		switch (inAddress->mSelector)
		{
			case kAudioObjectPropertyClass:
				return replyValue(kAudioSystemObjectClassID);
			case kAudioObjectPropertyBaseClass:
				return replyValue(kAudioObjectClassID);
			case kAudioHardwarePropertyPlugInList:
			case kAudioHardwarePropertyBoxList:
			case kAudioHardwarePropertyClockDeviceList:
			case kAudioHardwarePropertyTransportManagerList:
				// There are no plug-ins, boxes, clock devices or transport managers behind the PulseAudio backend.
				*ioDataSize = 0;
				return kAudioHardwareNoError;
			case kAudioHardwarePropertyUserSessionIsActiveOrHeadless:
				return replyValue(1);
			default:
				if (std::atomic<UInt32>* setting = systemSetting(inAddress->mSelector))
					return replyValue(setting->load());
		}
	}

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
		case kAudioDevicePropertyLatency:
		{
			// An IOProc block is heard after what PulseAudio already holds. Until a stream has measured
			// that, report what the stream asks for.
			if (id() == kAudioObjectSystemObject || isInputDevice() || inAddress->mScope == kAudioObjectPropertyScopeInput
				|| !asbd().mBytesPerFrame)
				break;
			if (outData && *ioDataSize < sizeof(UInt32))
				return kAudioHardwareBadPropertySizeError;
			if (outData)
			{
				// The frames are those of the stream's rate; the property counts frames of the device's rate.
				const UInt32 measured = m_measuredOutputLatency.load(std::memory_order_relaxed);
				const UInt32 frames = measured ? measured
					: AudioHardwareStreamPAOutput::queuedFrames(bufferSize() / asbd().mBytesPerFrame);
				const Float64 streamRate = asbd().mSampleRate, deviceRate = nominalSampleRate();
				*static_cast<UInt32*>(outData) = (streamRate > 0 && deviceRate > 0 && streamRate != deviceRate)
					? UInt32(frames * deviceRate / streamRate + 0.5) : frames;
			}
			*ioDataSize = sizeof(UInt32);
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

			AudioDeviceID found = deviceForUID(*static_cast<CFStringRef*>(translation->mInputData));

			*static_cast<AudioDeviceID*>(translation->mOutputData) = found;
			translation->mOutputDataSize = sizeof(found);
			*ioDataSize = sizeof(AudioValueTranslation);
			return kAudioHardwareNoError;
		}
		case kAudioHardwarePropertyTranslateUIDToDevice:
		{
			// The qualifier is the UID string and the answer the device's object ID, which is unknown
			// for a UID that no device has.
			if (id() != kAudioObjectSystemObject)
				break;
			if (inQualifierDataSize != sizeof(CFStringRef) || !inQualifierData || !*static_cast<const CFStringRef*>(inQualifierData))
				return kAudioHardwareBadPropertySizeError;
			if (outData && *ioDataSize < sizeof(AudioObjectID))
				return kAudioHardwareBadPropertySizeError;
			if (outData)
				*static_cast<AudioObjectID*>(outData) = deviceForUID(*static_cast<const CFStringRef*>(inQualifierData));
			*ioDataSize = sizeof(AudioObjectID);
			return kAudioHardwareNoError;
		}
		// BEGIN kAudioObjectSystemObject properties
		case kAudioHardwarePropertyDefaultInputDevice: // returns AudioDeviceID
		case kAudioHardwarePropertyDefaultOutputDevice: // returns AudioDeviceID
		{
			if (id() != kAudioObjectSystemObject)
				break;
			const bool input = inAddress->mSelector == kAudioHardwarePropertyDefaultInputDevice;
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				// The device PulseAudio's default is; the objects that stand for the default itself are
				// the answer only while the server's default is not one of the listed devices.
				AudioObjectID known = defaultDeviceId(input);
				*devId = known != kAudioObjectUnknown ? known : kAudioObjectSystemObject + (input ? 2 : 1);
			}
			*ioDataSize = sizeof(AudioDeviceID);
			return kAudioHardwareNoError;
		}
		case kAudioHardwarePropertyDefaultSystemOutputDevice:
			if (AudioDeviceID* devId = static_cast<AudioDeviceID*>(outData); devId && *ioDataSize >= sizeof(AudioDeviceID))
			{
				*devId = kAudioObjectSystemObject + 3;
			}
			*ioDataSize = sizeof(AudioDeviceID);
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyTapList:
			// No process tap can exist (see AudioHardwareTapping.m): the system object lists none.
			if (id() != kAudioObjectSystemObject)
				break;
			*ioDataSize = 0;
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyTranslateUIDToTap:
			// The qualifier is the tap's UID string and the answer is the tap's object ID, which is
			// unknown for every UID as no tap exists.
			if (id() != kAudioObjectSystemObject)
				break;
			if (inQualifierDataSize != sizeof(CFStringRef) || !inQualifierData)
				return kAudioHardwareBadPropertySizeError;
			if (AudioObjectID* tapId = static_cast<AudioObjectID*>(outData); tapId && *ioDataSize >= sizeof(AudioObjectID))
				*tapId = kAudioObjectUnknown;
			*ioDataSize = sizeof(AudioObjectID);
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
	
struct PAServerQuery
{
	std::string defaultName;
	bool input;
	dispatch_semaphore_t done;
};

static void paServerInfoCB(pa_context*, const pa_server_info* info, void* userdata)
{
	PAServerQuery* query = static_cast<PAServerQuery*>(userdata);
	if (info)
	{
		const char* name = query->input ? info->default_source_name : info->default_sink_name;
		if (name)
			query->defaultName = name;
	}
	dispatch_semaphore_signal(query->done);
}

AudioObjectID AudioHardwareImplPA::deviceForUID(CFStringRef wanted)
{
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
				return device->id();
		}
	}
	return kAudioObjectUnknown;
}

AudioObjectID AudioHardwareImplPA::defaultDeviceId(bool input)
{
	refreshDeviceList();

	PAServerQuery query = { std::string(), input, dispatch_semaphore_create(0) };
	PAServerQuery* queryPtr = &query;
	__block pa_operation* op = nullptr;

	getPAContext(^(pa_context* ctx) {
		if (ctx)
			op = pa_context_get_server_info(ctx, paServerInfoCB, queryPtr);
		if (!op)
			dispatch_semaphore_signal(queryPtr->done);
	});

	// PulseAudio drops pending operations without calling back when the connection is lost.
	dispatch_semaphore_wait(query.done, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
	// Nothing can write to query once this has run.
	syncPA(^{
		if (op)
		{
			pa_operation_cancel(op);
			pa_operation_unref(op);
		}
	});
	dispatch_release(query.done);

	if (query.defaultName.empty())
		return kAudioObjectUnknown;

	for (AudioObjectID candidate : allDeviceIds())
	{
		if (registry().nameForId(candidate) != query.defaultName)
			continue;
		std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
		auto& objects = PAGetObjectMap();
		auto it = objects.find(candidate);
		// a sink name is never also a source name, but a source can be the monitor of a sink, which is not listed
		if (it != objects.end() && it->second->isInputDevice() == input)
			return candidate;
	}
	return kAudioObjectUnknown;
}

static void paSuccessCB(pa_context*, int success, void* userdata)
{
	struct Result { bool ok; dispatch_semaphore_t done; };
	Result* result = static_cast<Result*>(userdata);
	result->ok = success != 0;
	dispatch_semaphore_signal(result->done);
}

OSStatus AudioHardwareImplPA::setDefaultDevice(AudioObjectID device, bool input)
{
	// The objects that stand for the default follow it already.
	if (device == kAudioObjectSystemObject + 1 || device == kAudioObjectSystemObject + 2)
		return kAudioHardwareNoError;

	std::string name;
	{
		std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
		auto& objects = PAGetObjectMap();
		auto it = objects.find(device);
		if (device <= kAudioObjectSystemObject + 3 || it == objects.end())
			return kAudioHardwareBadObjectError;
		if (it->second->isInputDevice() != input)
			return kAudioHardwareIllegalOperationError;
		const AudioHardwareImplPA* paDevice = dynamic_cast<const AudioHardwareImplPA*>(it->second.get());
		const char* paName = paDevice ? paDevice->paDeviceName() : nullptr;
		if (!paName)
			return kAudioHardwareBadObjectError;
		name = paName;
	}

	struct Result { bool ok; dispatch_semaphore_t done; };
	Result result = { false, dispatch_semaphore_create(0) };
	Result* resultPtr = &result;
	__block pa_operation* op = nullptr;

	getPAContext(^(pa_context* ctx) {
		if (ctx)
			op = input ? pa_context_set_default_source(ctx, name.c_str(), paSuccessCB, resultPtr)
			           : pa_context_set_default_sink(ctx, name.c_str(), paSuccessCB, resultPtr);
		if (!op)
			dispatch_semaphore_signal(resultPtr->done);
	});

	dispatch_semaphore_wait(result.done, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
	syncPA(^{
		if (op)
		{
			pa_operation_cancel(op);
			pa_operation_unref(op);
		}
	});
	dispatch_release(result.done);
	if (!result.ok)
		return kAudioHardwareUnspecifiedError;

	// The server may apply the change a moment after it has accepted it; the new default is
	// in effect when this returns, as the callers of the HAL expect.
	for (int i = 0; i < 40 && defaultDeviceId(input) != device; i++)
		usleep(25000);
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPA::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	if (id() == kAudioObjectSystemObject)
	{
		if (std::atomic<UInt32>* setting = systemSetting(inAddress->mSelector))
		{
			if (inDataSize != sizeof(UInt32) || !inData)
				return kAudioHardwareBadPropertySizeError;
			const UInt32 value = *static_cast<const UInt32*>(inData);
			if (setting->exchange(value) != value)
				notifyPropertyChanged(inAddress->mSelector);
			return kAudioHardwareNoError;
		}
	}

	if (id() == kAudioObjectSystemObject
		&& (inAddress->mSelector == kAudioHardwarePropertyDefaultOutputDevice
			|| inAddress->mSelector == kAudioHardwarePropertyDefaultInputDevice))
	{
		if (inDataSize != sizeof(AudioDeviceID) || !inData)
			return kAudioHardwareBadPropertySizeError;
		const bool input = inAddress->mSelector == kAudioHardwarePropertyDefaultInputDevice;
		refreshDeviceList();
		OSStatus status = setDefaultDevice(*static_cast<const AudioDeviceID*>(inData), input);
		if (status == kAudioHardwareNoError)
			notifyPropertyChanged(inAddress->mSelector);
		return status;
	}
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
	unsigned type = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;

	// The list of devices changes when a device appears or disappears. PulseAudio also reports
	// "change" for a sink or source whose state, volume or mute changed, which happens whenever a
	// stream starts or stops; that leaves the list as it was, and announcing it makes applications
	// that rebuild their output on a device list change restart it again and again.
	if ((facility == PA_SUBSCRIPTION_EVENT_SINK || facility == PA_SUBSCRIPTION_EVENT_SOURCE) &&
		(type == PA_SUBSCRIPTION_EVENT_NEW || type == PA_SUBSCRIPTION_EVENT_REMOVE))
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
	else if ((facility == PA_SUBSCRIPTION_EVENT_SINK || facility == PA_SUBSCRIPTION_EVENT_SOURCE) && type == PA_SUBSCRIPTION_EVENT_CHANGE)
	{
		// The volume, mute, port, rate or state of a device changed, possibly by another application.
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			std::vector<AudioHardwareImplPADevice*> devices;
			{
				std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
				for (auto& object : PAGetObjectMap())
				{
					AudioHardwareImplPADevice* device = dynamic_cast<AudioHardwareImplPADevice*>(object.second.get());
					if (device && device->hasListeners())
						devices.push_back(device);
				}
			}
			// The objects of the map live on for as long as the process, so the pointers stay valid.
			for (AudioHardwareImplPADevice* device : devices)
				device->checkChanges();
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
