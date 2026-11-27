/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "AudioHardwareImplPW.h"
#include "AudioHardwareStreamPWInput.h"
#include "AudioHardwareStreamPWOutput.h"
#include "PWConfig.h"
#include <CoreAudio/AudioHardwareBase.h>
#include <pipe_volume.h>
#include <CoreAudio/HostTime.h>
#include <CoreFoundation/CFString.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <thread>

namespace {
// Process-wide settings have no server-side representation in PipeWire; the
// value is what the process last set, as with the PulseAudio backend.
std::atomic<UInt32> g_processIsMain{0}, g_sleepingIsAllowed{1}, g_unloadingIsAllowed{1},
	g_hogModeIsAllowed{1}, g_processIsAudible{1}, g_powerHint{0}, g_mixStereoToMono{0};

constexpr AudioObjectPropertyAddress kDevicesAddress = {
	kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
constexpr AudioObjectPropertyAddress kDefaultOutputAddress = {
	kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
constexpr AudioObjectPropertyAddress kDefaultInputAddress = {
	kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
}

bool AudioHardwareImplPW::uidFromCFString(CFStringRef string, std::string& out)
{
	if (!string)
		return false;
	const CFIndex length = CFStringGetLength(string);
	const CFIndex capacity = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	out.resize(static_cast<size_t>(capacity));
	if (!CFStringGetCString(string, &out[0], static_cast<CFIndex>(out.size()), kCFStringEncodingUTF8))
	{
		out.clear();
		return false;
	}
	out.resize(std::strlen(out.c_str()));
	return true;
}

OSStatus AudioHardwareImplPW::reply(UInt32* ioDataSize, void* outData,
	const void* value, UInt32 size)
{
	if (outData)
	{
		if (*ioDataSize < size)
			return kAudioHardwareBadPropertySizeError;
		std::memcpy(outData, value, size);
	}
	*ioDataSize = size;
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPW::replyArray(UInt32* ioDataSize, void* outData,
	const void* values, UInt32 count, UInt32 elementSize)
{
	const UInt32 size = count * elementSize;
	if (outData)
	{
		if (*ioDataSize < size)
			return kAudioHardwareBadPropertySizeError;
		std::memcpy(outData, values, size);
	}
	*ioDataSize = size;
	return kAudioHardwareNoError;
}

std::atomic<UInt32>* AudioHardwareImplPW::systemSetting(AudioObjectPropertySelector selector)
{
	switch (selector)
	{
		case kAudioHardwarePropertyProcessIsMaster: return &g_processIsMain;
		case kAudioHardwarePropertySleepingIsAllowed: return &g_sleepingIsAllowed;
		case kAudioHardwarePropertyUnloadingIsAllowed: return &g_unloadingIsAllowed;
		case kAudioHardwarePropertyHogModeIsAllowed: return &g_hogModeIsAllowed;
		case kAudioHardwarePropertyProcessIsAudible: return &g_processIsAudible;
		case kAudioHardwarePropertyPowerHint: return &g_powerHint;
		case kAudioHardwarePropertyMixStereoToMono: return &g_mixStereoToMono;
		default: return nullptr;
	}
}

AudioHardwareImplPW::AudioHardwareImplPW(AudioObjectID myId)
: AudioHardwareImpl(myId), m_system(true), m_input(false), m_role("playback")
{
	m_name = CFSTR("PipeWire");
	m_manufacturer = CFSTR("PipeWire");
	m_uid = CFSTR("PipeWire:SystemObject");
}

AudioHardwareImplPW::AudioHardwareImplPW(AudioObjectID myId, const char* name,
	const char* uid, bool input, const char* role)
: AudioHardwareImpl(myId), m_system(false), m_input(input), m_role(role ? role : "")
{
	m_name = CFStringCreateWithCString(nullptr, name ? name : "PipeWire", kCFStringEncodingUTF8);
	m_uid = CFStringCreateWithCString(nullptr, uid ? uid : "PipeWire", kCFStringEncodingUTF8);
}

AudioHardwareImplPW::~AudioHardwareImplPW()
{
	if (auto* client = m_volumeClient.load())
		darling_pw_volume_close(client);
	if (!m_system)
	{
		CFRelease(m_name);
		CFRelease(m_uid);
	}
}

OSStatus AudioHardwareImplPW::translateUID(const AudioObjectPropertyAddress* address,
	UInt32 qualifierDataSize, const void* qualifierData,
	UInt32* ioDataSize, void* outData)
{
	if (address->mSelector == kAudioHardwarePropertyDeviceForUID)
	{
		if (!outData)
			return reply(ioDataSize, nullptr, nullptr, sizeof(AudioValueTranslation));
		if (*ioDataSize < sizeof(AudioValueTranslation))
			return kAudioHardwareBadPropertySizeError;
		auto* translation = static_cast<AudioValueTranslation*>(outData);
		if (!translation->mInputData || translation->mInputDataSize != sizeof(CFStringRef)
			|| !translation->mOutputData || translation->mOutputDataSize < sizeof(AudioDeviceID))
			return kAudioHardwareBadPropertySizeError;

		std::string uid;
		AudioDeviceID found = kAudioObjectUnknown;
		if (uidFromCFString(*static_cast<CFStringRef*>(translation->mInputData), uid))
			found = PWDeviceForUID(uid.c_str());
		*static_cast<AudioDeviceID*>(translation->mOutputData) = found;
		translation->mOutputDataSize = sizeof(found);
		return reply(ioDataSize, nullptr, nullptr, sizeof(AudioValueTranslation));
	}

	if (qualifierDataSize != sizeof(CFStringRef) || !qualifierData)
		return kAudioHardwareBadPropertySizeError;
	std::string uid;
	const AudioObjectID unknown = kAudioObjectUnknown;
	if (!uidFromCFString(*static_cast<const CFStringRef*>(qualifierData), uid))
		return reply(ioDataSize, outData, &unknown, sizeof(unknown));

	const AudioObjectID found = PWDeviceForUID(uid.c_str());
	return reply(ioDataSize, outData, &found, sizeof(found));
}

OSStatus AudioHardwareImplPW::systemPropertyData(const AudioObjectPropertyAddress* address,
	UInt32 qualifierDataSize, const void* qualifierData,
	UInt32* ioDataSize, void* outData)
{
	switch (address->mSelector)
	{
		case kAudioObjectPropertyClass:
		{
			const UInt32 classId = kAudioSystemObjectClassID;
			return reply(ioDataSize, outData, &classId, sizeof(classId));
		}
		case kAudioObjectPropertyBaseClass:
		{
			const UInt32 classId = kAudioObjectClassID;
			return reply(ioDataSize, outData, &classId, sizeof(classId));
		}
		case kAudioHardwarePropertyDevices:
		{
			const AudioObjectID devices[] = {
				kPWDefaultOutputObject, kPWDefaultInputObject, kPWSystemOutputObject };
			return replyArray(ioDataSize, outData, devices, 3, sizeof(devices[0]));
		}
		case kAudioHardwarePropertyDefaultOutputDevice:
		case kAudioHardwarePropertyDefaultInputDevice:
		{
			const AudioObjectID device = address->mSelector == kAudioHardwarePropertyDefaultInputDevice
				? kPWDefaultInputObject : kPWDefaultOutputObject;
			return reply(ioDataSize, outData, &device, sizeof(device));
		}
		case kAudioHardwarePropertyDefaultSystemOutputDevice:
			return reply(ioDataSize, outData, &kPWSystemOutputObject, sizeof(kPWSystemOutputObject));
		case kAudioHardwarePropertyPlugInList:
		case kAudioHardwarePropertyBoxList:
		case kAudioHardwarePropertyClockDeviceList:
		case kAudioHardwarePropertyTransportManagerList:
		case kAudioHardwarePropertyTapList:
			*ioDataSize = 0;
			return kAudioHardwareNoError;
		case kAudioHardwarePropertyTranslateUIDToTap:
		{
			if (qualifierDataSize != sizeof(CFStringRef) || !qualifierData)
				return kAudioHardwareBadPropertySizeError;
			const AudioObjectID unknown = kAudioObjectUnknown;
			return reply(ioDataSize, outData, &unknown, sizeof(unknown));
		}
		case kAudioHardwarePropertyDeviceForUID:
		case kAudioHardwarePropertyTranslateUIDToDevice:
			return translateUID(address, qualifierDataSize, qualifierData, ioDataSize, outData);
		case kAudioHardwarePropertyUserSessionIsActiveOrHeadless:
		{
			const UInt32 active = 1;
			return reply(ioDataSize, outData, &active, sizeof(active));
		}
		default:
			if (auto* setting = systemSetting(address->mSelector))
			{
				const UInt32 value = setting->load(std::memory_order_relaxed);
				return reply(ioDataSize, outData, &value, sizeof(value));
			}
			return kAudioHardwareUnknownPropertyError;
	}
}

OSStatus AudioHardwareImplPW::devicePropertyData(const AudioObjectPropertyAddress* address,
	UInt32* ioDataSize, void* outData)
{
	switch (address->mSelector)
	{
		case kAudioObjectPropertyClass:
		{
			const UInt32 classId = kAudioDeviceClassID;
			return reply(ioDataSize, outData, &classId, sizeof(classId));
		}
		case kAudioObjectPropertyBaseClass:
		{
			const UInt32 classId = kAudioObjectClassID;
			return reply(ioDataSize, outData, &classId, sizeof(classId));
		}
		case kAudioDevicePropertyStreams:
		{
			const bool matches = address->mScope == kAudioObjectPropertyScopeGlobal
				|| address->mScope == (m_input ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput);
			if (!matches)
			{
				*ioDataSize = 0;
				return kAudioHardwareNoError;
			}
			const AudioStreamID stream = kPWStreamObjectBase | id();
			return reply(ioDataSize, outData, &stream, sizeof(stream));
		}
		case kAudioDevicePropertyLatency:
		{
			const UInt32 frames = m_latencyFrames.load(std::memory_order_relaxed);
			return reply(ioDataSize, outData, &frames, sizeof(frames));
		}
		case kAudioDevicePropertyBufferFrameSizeRange:
		{
			AudioValueRange range = { 64, 4096 };
			return reply(ioDataSize, outData, &range, sizeof(range));
		}
		case kAudioDevicePropertyVolumeScalar:
		{
			// m_volume is what the listeners were last told (or the local value without a
			// server), so a read must not change it.
			float linear;
			int muted;
			Float32 volume = m_volume.load(std::memory_order_relaxed);
			if (auto* client = volumeClient(); client && darling_pw_volume_get(client, &linear, &muted) == 0)
				volume = std::cbrt(std::min(std::max(linear, 0.f), 1.f));
			return reply(ioDataSize, outData, &volume, sizeof(volume));
		}
		case kAudioDevicePropertyMute:
		{
			float linear;
			int muted;
			UInt32 muted32 = m_mute.load(std::memory_order_relaxed);
			if (auto* client = volumeClient(); client && darling_pw_volume_get(client, &linear, &muted) == 0)
				muted32 = muted ? 1 : 0;
			return reply(ioDataSize, outData, &muted32, sizeof(muted32));
		}
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		{
			const UInt32 running = hasActiveStreams() ? 1 : 0;
			return reply(ioDataSize, outData, &running, sizeof(running));
		}
		case kAudioDevicePropertyDeviceIsRunning:
		{
			const UInt32 running = hasActiveStreams() ? 1 : 0;
			return reply(ioDataSize, outData, &running, sizeof(running));
		}
		case kAudioDevicePropertyDeviceIsAlive:
		{
			const UInt32 alive = 1;
			return reply(ioDataSize, outData, &alive, sizeof(alive));
		}
		case kAudioDevicePropertyVolumeDecibelsToScalar:
		{
			if (!outData)
				return reply(ioDataSize, nullptr, nullptr, sizeof(Float32));
			if (*ioDataSize < sizeof(Float32))
				return kAudioHardwareBadPropertySizeError;
			const Float32 db = *static_cast<Float32*>(outData);
			// 20 * log10(v) inverted. Zero dB is unity gain; -inf dB is silence.
			const Float32 scalar = db <= -80.f ? 0.f : std::pow(10.f, db / 20.f);
			*static_cast<Float32*>(outData) = scalar;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeScalarToDecibels:
		{
			if (!outData)
				return reply(ioDataSize, nullptr, nullptr, sizeof(Float32));
			if (*ioDataSize < sizeof(Float32))
				return kAudioHardwareBadPropertySizeError;
			const Float32 scalar = std::max(*static_cast<Float32*>(outData), 0.f);
			*static_cast<Float32*>(outData) = scalar > 0.f ? 20.f * std::log10(scalar) : -80.f;
			return kAudioHardwareNoError;
		}
		default:
			return kAudioHardwareUnknownPropertyError;
	}
}

OSStatus AudioHardwareImplPW::getPropertyData(const AudioObjectPropertyAddress* address,
	UInt32 qualifierDataSize, const void* qualifierData,
	UInt32* ioDataSize, void* outData)
{
	if (m_system)
	{
		const OSStatus status = systemPropertyData(address, qualifierDataSize,
			qualifierData, ioDataSize, outData);
		if (status != kAudioHardwareUnknownPropertyError)
			return status;
	}
	else
	{
		const OSStatus status = devicePropertyData(address, ioDataSize, outData);
		if (status != kAudioHardwareUnknownPropertyError)
			return status;
	}
	return AudioHardwareImpl::getPropertyData(address, qualifierDataSize,
		qualifierData, ioDataSize, outData);
}

OSStatus AudioHardwareImplPW::setNominalRate(Float64 rate)
{
	if (rate <= 0 || !PWAvailableSampleRates(rate).size())
		return kAudioDeviceUnsupportedFormatError;
	const AudioValueRange allowed = [&] {
		AudioValueRange range = { 0, 0 };
		for (Float64 candidate : PWAvailableSampleRates(0))
		{
			if (std::abs(candidate - rate) < 1.0)
				range.mMinimum = range.mMaximum = candidate;
		}
		return range;
	}();
	if (allowed.mMinimum == 0 && allowed.mMaximum == 0)
		// A rate the server cannot negotiate is still recorded: the stream
		// asks for the device's rate and reports failure if negotiation fails.
		m_nominalRate = rate;
	else
		m_nominalRate = allowed.mMinimum;
	m_asbd.mSampleRate = m_nominalRate;
	notifyPropertyChanged(kAudioDevicePropertyNominalSampleRate);
	notifyPropertyChanged(kAudioDevicePropertyStreamFormat);
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPW::setPropertyData(const AudioObjectPropertyAddress* address,
	UInt32 qualifierDataSize, const void* qualifierData,
	UInt32 dataSize, const void* inData)
{
	(void) qualifierDataSize;
	(void) qualifierData;

	if (m_system)
	{
		if (auto* setting = systemSetting(address->mSelector))
		{
			if (!inData || dataSize != sizeof(UInt32))
				return kAudioHardwareBadPropertySizeError;
			setting->store(*static_cast<const UInt32*>(inData), std::memory_order_relaxed);
			return kAudioHardwareNoError;
		}
		if (address->mSelector == kAudioHardwarePropertyDefaultOutputDevice
			|| address->mSelector == kAudioHardwarePropertyDefaultInputDevice)
			return kAudioHardwareNoError;
		return AudioHardwareImpl::setPropertyData(address, qualifierDataSize,
			qualifierData, dataSize, inData);
	}

	switch (address->mSelector)
	{
		case kAudioDevicePropertyVolumeScalar:
			if (!inData || dataSize != sizeof(Float32))
				return kAudioHardwareBadPropertySizeError;
		{
			const Float32 scalar = std::min(std::max(*static_cast<const Float32*>(inData), 0.f), 1.f);
			m_volume.store(scalar, std::memory_order_relaxed);
			// The server stores linear gain; the scalar follows the cubic curve
			// that desktop mixers use, so both show the same percentage.
			if (auto* client = volumeClient())
				darling_pw_volume_set_linear(client, scalar * scalar * scalar);
			notifyPropertyChanged(kAudioDevicePropertyVolumeScalar, kAudioObjectPropertyScopeWildcard, kAudioObjectPropertyElementWildcard);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyMute:
			if (!inData || dataSize != sizeof(UInt32))
				return kAudioHardwareBadPropertySizeError;
			m_mute.store(*static_cast<const UInt32*>(inData) ? 1 : 0, std::memory_order_relaxed);
			if (auto* client = volumeClient())
				darling_pw_volume_set_mute(client, m_mute.load(std::memory_order_relaxed));
			notifyPropertyChanged(kAudioDevicePropertyMute, kAudioObjectPropertyScopeWildcard, kAudioObjectPropertyElementWildcard);
			return kAudioHardwareNoError;
		case kAudioDevicePropertyNominalSampleRate:
			if (!inData || dataSize != sizeof(Float64))
				return kAudioHardwareBadPropertySizeError;
			return setNominalRate(*static_cast<const Float64*>(inData));
		case kAudioDevicePropertyBufferFrameSize:
		{
			const OSStatus status = AudioHardwareImpl::setPropertyData(address,
				qualifierDataSize, qualifierData, dataSize, inData);
			if (status == kAudioHardwareNoError)
				notifyPropertyChanged(kAudioDevicePropertyBufferFrameSize);
			return status;
		}
		default:
			return AudioHardwareImpl::setPropertyData(address, qualifierDataSize,
				qualifierData, dataSize, inData);
	}
}

darling_pw_volume* AudioHardwareImplPW::volumeClient()
{
	if (m_system)
		return nullptr;
	std::lock_guard<std::mutex> lock(m_volumeMutex);
	if (auto* existing = m_volumeClient.load())
		return existing;
	// Without a server the connection attempt fails at once; it is repeated
	// at most once a second so that a server started later is picked up.
	const auto now = std::chrono::steady_clock::now();
	if (m_volumeAttempt.time_since_epoch().count() != 0
		&& now - m_volumeAttempt < std::chrono::seconds(1))
		return nullptr;
	m_volumeAttempt = now;
	const darling_pw_volume_events events = { &AudioHardwareImplPW::volumeChangedByServer };
	darling_pw_volume* client = nullptr;
	if (darling_pw_volume_open(m_input ? 0 : 1, &events, this, &client) != 0)
		return nullptr;
	// What the listeners are measured against: the values the server has now.
	float linear;
	int muted;
	if (darling_pw_volume_get(client, &linear, &muted) == 0)
	{
		m_volume.store(std::cbrt(std::min(std::max(linear, 0.f), 1.f)), std::memory_order_relaxed);
		m_mute.store(muted ? 1 : 0, std::memory_order_relaxed);
	}
	m_volumeClient.store(client);
	return client;
}

void AudioHardwareImplPW::volumeChangedByServer(void* userData)
{
	static_cast<AudioHardwareImplPW*>(userData)->volumeChanged();
}

// Runs on the PipeWire thread when the server's values differ from the last ones seen.
void AudioHardwareImplPW::volumeChanged()
{
	float linear;
	int muted;
	darling_pw_volume* client = m_volumeClient.load();
	if (!client || darling_pw_volume_get(client, &linear, &muted) != 0)
		return;
	const Float32 scalar = std::cbrt(std::min(std::max(linear, 0.f), 1.f));
	if (std::fabs(m_volume.exchange(scalar, std::memory_order_relaxed) - scalar) > 1e-4f)
		notifyPropertyChanged(kAudioDevicePropertyVolumeScalar, kAudioObjectPropertyScopeWildcard, kAudioObjectPropertyElementWildcard);
	if (m_mute.exchange(muted ? 1 : 0, std::memory_order_relaxed) != (muted ? 1u : 0u))
		notifyPropertyChanged(kAudioDevicePropertyMute, kAudioObjectPropertyScopeWildcard, kAudioObjectPropertyElementWildcard);
}

OSStatus AudioHardwareImplPW::addPropertyListener(const AudioObjectPropertyAddress* address,
	AudioObjectPropertyListenerProc listener, void* clientData)
{
	// Server-side changes can only be reported while the connection is open.
	if (address->mSelector == kAudioDevicePropertyVolumeScalar
		|| address->mSelector == kAudioDevicePropertyMute)
		volumeClient();
	return AudioHardwareImpl::addPropertyListener(address, listener, clientData);
}

OSStatus AudioHardwareImplPW::isPropertySettable(const AudioObjectPropertyAddress* address,
	Boolean* outIsSettable)
{
	if (m_system)
	{
		if (systemSetting(address->mSelector)
			|| address->mSelector == kAudioHardwarePropertyDefaultOutputDevice
			|| address->mSelector == kAudioHardwarePropertyDefaultInputDevice)
		{
			*outIsSettable = true;
			return kAudioHardwareNoError;
		}
	}
	else if (address->mSelector == kAudioDevicePropertyVolumeScalar
		|| address->mSelector == kAudioDevicePropertyMute
		|| address->mSelector == kAudioDevicePropertyNominalSampleRate)
	{
		*outIsSettable = true;
		return kAudioHardwareNoError;
	}
	return AudioHardwareImpl::isPropertySettable(address, outIsSettable);
}

std::vector<Float64> AudioHardwareImplPW::availableSampleRates()
{
	return PWAvailableSampleRates(m_nominalRate > 0 ? m_nominalRate : m_asbd.mSampleRate);
}

Float64 AudioHardwareImplPW::nominalSampleRate()
{
	if (m_nominalRate > 0)
		return m_nominalRate;
	if (m_asbd.mSampleRate > 0)
		return m_asbd.mSampleRate;
	return 48000.0;
}

AudioHardwareStream* AudioHardwareImplPW::createStream(AudioDeviceIOProc callback, void* clientData)
{
	if (m_system)
		return nullptr;

	AudioHardwareStreamPW* stream = m_input
		? static_cast<AudioHardwareStreamPW*>(
			new AudioHardwareStreamPWInput(this, callback, clientData, true, "capture"))
		: static_cast<AudioHardwareStreamPW*>(
			new AudioHardwareStreamPWOutput(this, callback, clientData, false, m_role.c_str()));
	// A failed PipeWire connection or negotiation leaves an inert stream. Do
	// not hand it to AudioHardwareImpl::start(), which would report success.
	if (!stream->valid())
	{
		delete stream;
		return nullptr;
	}
	return stream;
}

bool AudioHardwareImplPW::validateFormat(const AudioStreamBasicDescription* format) const
{
	// The shared client negotiates interleaved native-endian float32. Other
	// PCM layouts are converted by AudioConverter above the HAL.
	return format && format->mFormatID == kAudioFormatLinearPCM
		&& (format->mFormatFlags & kLinearPCMFormatFlagIsFloat)
		&& !(format->mFormatFlags & kAudioFormatFlagIsNonInterleaved);
}

bool AudioHardwareImplPW::hasActiveStreams()
{
	std::lock_guard<std::mutex> lock(m_procMutex);
	return !m_streams.empty();
}

bool AudioHardwareImplPW::ownsScope(UInt32 scope) const
{
	return scope == kAudioObjectPropertyScopeGlobal
		|| scope == (m_input ? kAudioObjectPropertyScopeInput : kAudioObjectPropertyScopeOutput);
}

OSStatus AudioHardwareImplPW::stop(AudioDeviceIOProcID procID)
{
	std::lock_guard<std::mutex> lock(m_procMutex);
	auto it = m_streams.find(procID);
	if (it == m_streams.end())
		return kAudioHardwareNotRunningError;
	AudioHardwareStream* stream = it->second.release();
	m_streams.erase(it);
	stream->stop();

	// An IOProc may stop its own stream from the pump thread. AudioHardwareStreamPW::stop()
	// avoids joining the current thread, so the pump can still be unwinding here.
	// Reap on a separate thread: it can join the pump and delete the object only
	// after the callback has returned, and stop() is idempotent for the normal
	// case where the pump was already joined above.
	std::thread([stream]() {
		stream->stop();
		delete stream;
	}).detach();

	notifyPropertyChanged(kAudioDevicePropertyDeviceIsRunningSomewhere);
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPW::getCurrentTime(AudioTimeStamp* outTime)
{
	if (!outTime)
		return kAudioHardwareBadPropertySizeError;
	std::memset(outTime, 0, sizeof(*outTime));
	const uint64_t frames = m_sampleClock.load(std::memory_order_relaxed);
	const Float64 rate = nominalSampleRate();
	outTime->mSampleTime = rate > 0 ? double(frames) / rate : 0;
	outTime->mHostTime = AudioGetCurrentHostTime();
	outTime->mFlags = AudioTimeStampFlags(kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid);
	return kAudioHardwareNoError;
}

AudioHardwareImplPWOutput::AudioHardwareImplPWOutput(AudioObjectID myId, const char* role)
: AudioHardwareImplPW(myId, "PipeWire Output",
	PWUIDForDevice(myId) ? PWUIDForDevice(myId) : "PipeWire:Output", false, role)
{
}

AudioHardwareImplPWInput::AudioHardwareImplPWInput(AudioObjectID myId)
: AudioHardwareImplPW(myId, "PipeWire Input", "PipeWire:Input", true, "capture")
{
}
