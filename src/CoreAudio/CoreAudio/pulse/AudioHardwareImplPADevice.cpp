/*
This file is part of Darling.

Copyright (C) 2020-2026 Lubos Dolezel

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

#include "AudioHardwareImplPADevice.h"
#include "AudioHardwareStreamPAInput.h"
#include "AudioHardwareStreamPAOutput.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unistd.h>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>

// The selectors of the main volume and balance, which AudioToolbox/AudioHardwareService.h names
// kAudioHardwareServiceDeviceProperty_VirtualMainVolume and ..._VirtualMainBalance.
static const AudioObjectPropertySelector kVirtualMainVolume = 'vmvc', kVirtualMainBalance = 'vmbc';

// kAudioHardwareUnknownPropertyError is the answer for a property the device does not have,
// for a scope or an element it does not have it on.

AudioHardwareImplPADevice::AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, const char* description, bool isInput,
	const char* paRole)
: AudioHardwareImplPA(myId, paRole)
, m_paName(std::move(paName))
, m_isInput(isInput)
{
	m_name = CFStringCreateWithCString(nullptr, description ? description : m_paName.c_str(), kCFStringEncodingUTF8);
	m_uid = CFStringCreateWithCString(nullptr, m_paName.c_str(), kCFStringEncodingUTF8);
}

AudioHardwareImplPADevice::~AudioHardwareImplPADevice()
{
	CFRelease(m_name);
	CFRelease(m_uid);
}

AudioHardwareStream* AudioHardwareImplPADevice::createStream(AudioDeviceIOProc callback, void* clientData)
{
	if (m_isInput)
		return new AudioHardwareStreamPAInput(this, callback, clientData);
	return new AudioHardwareStreamPAOutput(this, callback, clientData);
}

// ---- What the server says about the device

static PAPortKind portKindOf(uint32_t type)
{
	switch (type)
	{
		case PA_DEVICE_PORT_TYPE_SPEAKER: return PAPortKind::Speaker;
		case PA_DEVICE_PORT_TYPE_HEADPHONES: return PAPortKind::Headphones;
		case PA_DEVICE_PORT_TYPE_HEADSET: return PAPortKind::Headset;
		case PA_DEVICE_PORT_TYPE_LINE: return PAPortKind::Line;
		case PA_DEVICE_PORT_TYPE_MIC: return PAPortKind::Microphone;
		case PA_DEVICE_PORT_TYPE_HDMI: return PAPortKind::HDMI;
		case PA_DEVICE_PORT_TYPE_SPDIF: return PAPortKind::SPDIF;
		case PA_DEVICE_PORT_TYPE_BLUETOOTH: return PAPortKind::Bluetooth;
		case PA_DEVICE_PORT_TYPE_USB: return PAPortKind::USB;
		case PA_DEVICE_PORT_TYPE_ANALOG: return PAPortKind::Analog;
		default: return PAPortKind::Other;
	}
}

// pa_sink_info and pa_source_info have the same members for what is used here.
template <class Info, class PortInfo>
static void fillSnapshot(PADeviceSnapshot& s, const Info* info, bool running)
{
	auto property = [&](const char* key) {
		const char* value = pa_proplist_gets(info->proplist, key);
		return std::string(value ? value : "");
	};

	s.valid = true;
	s.index = info->index;
	s.card = info->card;
	s.name = info->name ? info->name : "";
	s.description = info->description ? info->description : "";
	s.driver = info->driver ? info->driver : "";
	s.bus = property(PA_PROP_DEVICE_BUS);
	s.api = property(PA_PROP_DEVICE_API);
	s.formFactor = property(PA_PROP_DEVICE_FORM_FACTOR);
	s.vendor = property(PA_PROP_DEVICE_VENDOR_NAME);
	s.product = property(PA_PROP_DEVICE_PRODUCT_NAME);
	s.serial = property(PA_PROP_DEVICE_SERIAL);
	s.spec = info->sample_spec;
	s.map = info->channel_map;
	s.volume = info->volume;
	s.mute = info->mute != 0;
	s.running = running;
	s.configuredLatencyUsec = info->configured_latency;
	s.ports.clear();
	s.activePort = -1;
	for (uint32_t i = 0; i < info->n_ports; i++)
	{
		const PortInfo* port = info->ports[i];
		PAPortDesc desc;
		desc.name = port->name ? port->name : "";
		desc.description = port->description ? port->description : "";
		desc.kind = portKindOf(port->type);
		desc.available = port->available == PA_PORT_AVAILABLE_YES ? PAPortAvailability::Yes
			: port->available == PA_PORT_AVAILABLE_NO ? PAPortAvailability::No : PAPortAvailability::Unknown;
		if (info->active_port == port)
			s.activePort = int(s.ports.size());
		s.ports.push_back(desc);
	}
}

struct PASnapshotQuery
{
	PADeviceSnapshot snapshot;
	dispatch_semaphore_t done;
};

static void paSinkSnapshotCB(pa_context*, const pa_sink_info* info, int eol, void* userdata)
{
	PASnapshotQuery* query = static_cast<PASnapshotQuery*>(userdata);
	if (info)
		fillSnapshot<pa_sink_info, pa_sink_port_info>(query->snapshot, info, info->state == PA_SINK_RUNNING);
	if (eol)
		dispatch_semaphore_signal(query->done);
}

static void paSourceSnapshotCB(pa_context*, const pa_source_info* info, int eol, void* userdata)
{
	PASnapshotQuery* query = static_cast<PASnapshotQuery*>(userdata);
	if (info)
		fillSnapshot<pa_source_info, pa_source_port_info>(query->snapshot, info, info->state == PA_SOURCE_RUNNING);
	if (eol)
		dispatch_semaphore_signal(query->done);
}

// All the devices ask the server through the connection of the system object, instead of each opening one.
static AudioHardwareImplPA* serverConnection(AudioHardwareImplPA* fallback)
{
	std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
	auto& objects = PAGetObjectMap();
	auto it = objects.find(kAudioObjectSystemObject);
	if (it != objects.end())
		if (AudioHardwareImplPA* system = dynamic_cast<AudioHardwareImplPA*>(it->second.get()))
			return system;
	return fallback;
}

bool AudioHardwareImplPADevice::fetchSnapshot(PADeviceSnapshot& out)
{
	AudioHardwareImplPA* server = serverConnection(this);

	// The answer arrives on the PulseAudio queue, which an IOProc of the device would be blocking.
	if (onPAQueue() || server->onPAQueue())
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		out = m_cached;
		return out.valid;
	}

	PASnapshotQuery query = { PADeviceSnapshot(), dispatch_semaphore_create(0) };
	PASnapshotQuery* queryPtr = &query;
	__block pa_operation* op = nullptr;
	const char* name = m_paName.c_str();
	const bool isInput = m_isInput;

	server->getPAContext(^(pa_context* ctx) {
		if (ctx)
			op = isInput ? pa_context_get_source_info_by_name(ctx, name, paSourceSnapshotCB, queryPtr)
			             : pa_context_get_sink_info_by_name(ctx, name, paSinkSnapshotCB, queryPtr);
		if (!op)
			dispatch_semaphore_signal(queryPtr->done);
	});

	// PulseAudio drops pending operations without calling back when the connection is lost.
	dispatch_semaphore_wait(query.done, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
	// Nothing can write to query once this has run.
	server->syncPA(^{
		if (op)
		{
			pa_operation_cancel(op);
			pa_operation_unref(op);
		}
	});
	dispatch_release(query.done);

	if (query.snapshot.valid)
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		m_cached = query.snapshot;
	}
	out = query.snapshot;
	return out.valid;
}

PADeviceSnapshot AudioHardwareImplPADevice::snapshot()
{
	PADeviceSnapshot s;
	if (fetchSnapshot(s))
		return s;

	// The server did not answer: describe the device by the format it has.
	s = PADeviceSnapshot();
	s.spec.format = PA_SAMPLE_FLOAT32LE;
	s.spec.rate = m_asbd.mSampleRate > 0 ? uint32_t(m_asbd.mSampleRate) : 44100;
	s.spec.channels = uint8_t(m_asbd.mChannelsPerFrame ? m_asbd.mChannelsPerFrame : 2);
	pa_channel_map_init_auto(&s.map, s.spec.channels, PA_CHANNEL_MAP_DEFAULT);
	pa_cvolume_reset(&s.volume, s.spec.channels);
	return s;
}

// Runs an operation on the server and waits until it has been done. When called from the
// PulseAudio queue, which can't wait for the server, the operation is only started.
bool AudioHardwareImplPADevice::paOperation(pa_operation* (^start)(pa_context*, pa_context_success_cb_t, void*))
{
	AudioHardwareImplPA* server = serverConnection(this);

	if (onPAQueue() || server->onPAQueue())
	{
		__block bool started = false;
		server->getPAContext(^(pa_context* ctx) {
			if (!ctx)
				return;
			if (pa_operation* op = start(ctx, nullptr, nullptr))
			{
				pa_operation_unref(op);
				started = true;
			}
		});
		return started;
	}

	struct Result { bool ok; dispatch_semaphore_t done; };
	Result result = { false, dispatch_semaphore_create(0) };
	Result* resultPtr = &result;
	__block pa_operation* op = nullptr;

	server->getPAContext(^(pa_context* ctx) {
		if (ctx)
			op = start(ctx, [](pa_context*, int success, void* userdata) {
				Result* r = static_cast<Result*>(userdata);
				r->ok = success != 0;
				dispatch_semaphore_signal(r->done);
			}, resultPtr);
		if (!op)
			dispatch_semaphore_signal(resultPtr->done);
	});

	dispatch_semaphore_wait(result.done, dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC));
	server->syncPA(^{
		if (op)
		{
			pa_operation_cancel(op);
			pa_operation_unref(op);
		}
	});
	dispatch_release(result.done);
	return result.ok;
}

// ---- Format and rate

void AudioHardwareImplPADevice::refreshFormat()
{
	if (m_formatSetByClient || onPAQueue() || isRunningHere())
		return;

	PADeviceSnapshot s;
	if (!fetchSnapshot(s) || !s.spec.rate || !s.spec.channels)
		return;

	const Float64 rate = m_nominalRate > 0 ? m_nominalRate : s.spec.rate;
	if (m_asbd.mSampleRate == rate && m_asbd.mChannelsPerFrame == s.spec.channels)
		return;

	m_asbd.mSampleRate = rate;
	m_asbd.mChannelsPerFrame = s.spec.channels;
	m_asbd.mBytesPerFrame = m_asbd.mBytesPerPacket = m_asbd.mChannelsPerFrame * m_asbd.mBitsPerChannel / 8;
}

Float64 AudioHardwareImplPADevice::nominalSampleRate()
{
	if (m_nominalRate > 0)
		return m_nominalRate;
	PADeviceSnapshot s;
	return fetchSnapshot(s) && s.spec.rate ? Float64(s.spec.rate) : m_asbd.mSampleRate;
}

std::vector<Float64> AudioHardwareImplPADevice::availableSampleRates()
{
	PADeviceSnapshot s;
	const double native = fetchSnapshot(s) ? s.spec.rate : m_asbd.mSampleRate;
	return PAAvailableSampleRates(native, m_nominalRate);
}

OSStatus AudioHardwareImplPADevice::setNominalRate(Float64 rate)
{
	const std::vector<Float64> rates = availableSampleRates();
	if (std::find(rates.begin(), rates.end(), rate) == rates.end())
		return kAudioHardwareIllegalOperationError;
	// A stream that runs has the rate it was made with.
	if (isRunningHere())
		return kAudioHardwareIllegalOperationError;

	m_nominalRate = rate;
	m_asbd.mSampleRate = rate;
	notifyPropertyChanged(kAudioDevicePropertyNominalSampleRate);
	notifyPropertyChanged(kAudioDevicePropertyActualSampleRate);
	return kAudioHardwareNoError;
}

bool AudioHardwareImplPADevice::isRunningHere()
{
	__block bool running = false;
	syncPA(^{ running = !m_streams.empty(); });
	return running;
}

bool AudioHardwareImplPADevice::ownScope(UInt32 scope) const
{
	return scope == (m_isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput);
}

// ---- Replies

static OSStatus reply(UInt32* ioDataSize, void* outData, const void* source, UInt32 size)
{
	if (outData)
	{
		if (*ioDataSize < size)
			return kAudioHardwareBadPropertySizeError;
		memcpy(outData, source, size);
	}
	*ioDataSize = size;
	return kAudioHardwareNoError;
}

static OSStatus replyUInt32(UInt32* ioDataSize, void* outData, UInt32 value)
{
	return reply(ioDataSize, outData, &value, sizeof(value));
}

// An array: a buffer that is too small gets the elements that fit.
static OSStatus replyList(UInt32* ioDataSize, void* outData, const void* source, size_t count, size_t elementSize)
{
	if (outData)
	{
		const size_t fit = std::min<size_t>(count, *ioDataSize / elementSize);
		if (fit)
			memcpy(outData, source, fit * elementSize);
		*ioDataSize = UInt32(fit * elementSize);
		return kAudioHardwareNoError;
	}
	*ioDataSize = UInt32(count * elementSize);
	return kAudioHardwareNoError;
}

static OSStatus replyString(UInt32* ioDataSize, void* outData, const std::string& value)
{
	if (outData)
	{
		if (*ioDataSize < sizeof(CFStringRef))
			return kAudioHardwareBadPropertySizeError;
		*static_cast<CFStringRef*>(outData) = CFStringCreateWithCString(nullptr, value.c_str(), kCFStringEncodingUTF8);
	}
	*ioDataSize = sizeof(CFStringRef);
	return kAudioHardwareNoError;
}

static Float32 scalarOf(pa_volume_t volume)
{
	return std::min(1.0f, volume / float(PA_VOLUME_NORM));
}

static Float32 decibelsOf(pa_volume_t volume)
{
	return Float32(pa_sw_volume_to_dB(volume));
}

// The volume of one element: a channel, or 0 for the whole device, which is the loudest channel.
static pa_volume_t volumeOfElement(const PADeviceSnapshot& s, UInt32 element)
{
	return element == 0 ? pa_cvolume_max(&s.volume) : s.volume.values[element - 1];
}

static bool canBalance(const PADeviceSnapshot& s)
{
	return pa_channel_map_valid(&s.map) && pa_channel_map_can_balance(&s.map);
}

static AudioChannelLabel channelLabelOf(pa_channel_position_t position, unsigned channels)
{
	switch (position)
	{
		case PA_CHANNEL_POSITION_MONO: return kAudioChannelLabel_Mono;
		case PA_CHANNEL_POSITION_FRONT_LEFT: return kAudioChannelLabel_Left;
		case PA_CHANNEL_POSITION_FRONT_RIGHT: return kAudioChannelLabel_Right;
		case PA_CHANNEL_POSITION_FRONT_CENTER: return kAudioChannelLabel_Center;
		case PA_CHANNEL_POSITION_LFE: return kAudioChannelLabel_LFEScreen;
		case PA_CHANNEL_POSITION_REAR_LEFT: return kAudioChannelLabel_LeftSurround;
		case PA_CHANNEL_POSITION_REAR_RIGHT: return kAudioChannelLabel_RightSurround;
		case PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER: return kAudioChannelLabel_LeftCenter;
		case PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER: return kAudioChannelLabel_RightCenter;
		case PA_CHANNEL_POSITION_REAR_CENTER: return kAudioChannelLabel_CenterSurround;
		case PA_CHANNEL_POSITION_SIDE_LEFT: return kAudioChannelLabel_LeftSurroundDirect;
		case PA_CHANNEL_POSITION_SIDE_RIGHT: return kAudioChannelLabel_RightSurroundDirect;
		case PA_CHANNEL_POSITION_TOP_CENTER: return kAudioChannelLabel_TopCenterSurround;
		default: return kAudioChannelLabel_Unknown;
	}
}

// The code of the data source of the device that the port stands for.
static void dataSourcesOf(const PADeviceSnapshot& s, bool input, std::vector<uint32_t>& codes, std::vector<bool>& listed)
{
	codes = PADataSourceCodes(s.ports, input);
	listed.assign(s.ports.size(), false);
	for (size_t i = 0; i < s.ports.size(); i++)
		listed[i] = s.ports[i].available != PAPortAvailability::No || int(i) == s.activePort;
}

OSStatus AudioHardwareImplPADevice::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	const UInt32 scope = inAddress->mScope, element = inAddress->mElement;
	const bool own = ownScope(scope);
	// Leniency: volume and mute of the global scope are those of the side that the device has, which is what
	// the first version of this HAL answered and what callers of it are written for.
	const bool control = own || scope == kAudioObjectPropertyScopeGlobal;

	switch (inAddress->mSelector)
	{
		case kAudioObjectPropertyClass:
			return replyUInt32(ioDataSize, outData, kAudioDeviceClassID);
		case kAudioObjectPropertyBaseClass:
			return replyUInt32(ioDataSize, outData, kAudioObjectClassID);
		case kAudioDevicePropertyDeviceUID:
		{
			if (outData)
			{
				if (*ioDataSize < sizeof(CFStringRef))
					return kAudioHardwareBadPropertySizeError;
				*static_cast<CFStringRef*>(outData) = static_cast<CFStringRef>(CFRetain(m_uid));
			}
			*ioDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyModelUID:
		case kAudioObjectPropertyModelName:
		{
			const PADeviceSnapshot s = snapshot();
			return replyString(ioDataSize, outData, !s.product.empty() ? s.product : !s.description.empty() ? s.description : m_paName);
		}
		case kAudioObjectPropertySerialNumber:
		{
			const PADeviceSnapshot s = snapshot();
			if (s.serial.empty())
				return kAudioHardwareUnknownPropertyError;
			return replyString(ioDataSize, outData, s.serial);
		}
		case kAudioDevicePropertyDeviceManufacturerCFString:
		case kAudioDevicePropertyDeviceManufacturer:
		{
			const PADeviceSnapshot s = snapshot();
			if (s.vendor.empty())
				break;
			if (inAddress->mSelector == kAudioDevicePropertyDeviceManufacturerCFString)
				return replyString(ioDataSize, outData, s.vendor);

			// The older property is a C string.
			const UInt32 size = UInt32(s.vendor.size() + 1);
			if (outData)
			{
				if (*ioDataSize < size)
					return kAudioHardwareBadPropertySizeError;
				memcpy(outData, s.vendor.c_str(), size);
			}
			*ioDataSize = size;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyTransportType:
		{
			const PADeviceSnapshot s = snapshot();
			PATransportInput input;
			input.name = s.name;
			input.driver = s.driver;
			input.bus = s.bus;
			input.api = s.api;
			input.formFactor = s.formFactor;
			input.hasCard = s.card != PA_INVALID_INDEX;
			if (s.activePort >= 0)
				input.activePort = &s.ports[s.activePort];
			return replyUInt32(ioDataSize, outData, PATransportType(input));
		}
		case kAudioDevicePropertyRelatedDevices:
		{
			const PADeviceSnapshot s = snapshot();
			std::vector<AudioObjectID> candidates;
			{
				std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
				for (auto& object : PAGetObjectMap())
				{
					if (object.first > kAudioObjectSystemObject + 3 && dynamic_cast<AudioHardwareImplPADevice*>(object.second.get()))
						candidates.push_back(object.first);
				}
			}

			std::vector<AudioObjectID> related = { id() };
			if (s.card != PA_INVALID_INDEX)
			{
				for (AudioObjectID candidate : candidates)
				{
					if (candidate == id())
						continue;
					AudioHardwareImplPADevice* other = nullptr;
					{
						std::lock_guard<std::mutex> lock(PAGetObjectMapMutex());
						auto it = PAGetObjectMap().find(candidate);
						if (it != PAGetObjectMap().end())
							other = dynamic_cast<AudioHardwareImplPADevice*>(it->second.get());
					}
					if (other && other->snapshot().card == s.card)
						related.push_back(candidate);
				}
			}
			std::sort(related.begin(), related.end());
			return replyList(ioDataSize, outData, related.data(), related.size(), sizeof(AudioObjectID));
		}
		case kAudioDevicePropertyClockDomain:
		{
			// The sinks and sources of one sound card run on the clock of the card.
			const PADeviceSnapshot s = snapshot();
			return replyUInt32(ioDataSize, outData, s.card == PA_INVALID_INDEX ? 0 : s.card + 1);
		}
		case kAudioDevicePropertyDeviceIsAlive:
			return replyUInt32(ioDataSize, outData, 1);
		case kAudioDevicePropertyDeviceIsRunning:
			return replyUInt32(ioDataSize, outData, outData && isRunningHere() ? 1 : 0);
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		{
			if (!outData)
				return replyUInt32(ioDataSize, outData, 0);
			const PADeviceSnapshot s = snapshot();
			return replyUInt32(ioDataSize, outData, s.running || isRunningHere() ? 1 : 0);
		}
		case kAudioDevicePropertyDeviceCanBeDefaultDevice:
			if (scope != kAudioDevicePropertyScopeInput && scope != kAudioDevicePropertyScopeOutput)
				return kAudioHardwareUnknownPropertyError;
			return replyUInt32(ioDataSize, outData, own ? 1 : 0);
		case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice:
			if (scope != kAudioDevicePropertyScopeInput && scope != kAudioDevicePropertyScopeOutput)
				return kAudioHardwareUnknownPropertyError;
			return replyUInt32(ioDataSize, outData, own && !m_isInput ? 1 : 0);
		case kAudioDevicePropertyLatency:
		{
			// The delay of a recording is what the source says its buffer holds. An output's is handled
			// by the class below, which knows what the stream has queued.
			if (!own || !m_isInput)
				break;
			const PADeviceSnapshot s = snapshot();
			return replyUInt32(ioDataSize, outData, UInt32(double(s.configuredLatencyUsec) * s.spec.rate / 1e6));
		}
		case kAudioDevicePropertyStreamConfiguration:
		{
			const PADeviceSnapshot s = snapshot();
			const UInt32 buffers = own ? 1 : 0;
			const UInt32 size = UInt32(offsetof(AudioBufferList, mBuffers) + buffers * sizeof(AudioBuffer));
			if (AudioBufferList* list = static_cast<AudioBufferList*>(outData))
			{
				if (*ioDataSize < size)
					return kAudioHardwareBadPropertySizeError;
				list->mNumberBuffers = buffers;
				if (buffers)
				{
					list->mBuffers[0].mNumberChannels = s.spec.channels;
					list->mBuffers[0].mDataByteSize = 0;
					list->mBuffers[0].mData = nullptr;
				}
			}
			*ioDataSize = size;
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyPreferredChannelsForStereo:
		{
			if (!own)
				break;
			const PADeviceSnapshot s = snapshot();
			UInt32 channels[2] = { 1, 2 };
			for (unsigned i = 0; i < s.map.channels; i++)
			{
				if (s.map.map[i] == PA_CHANNEL_POSITION_FRONT_LEFT)
					channels[0] = i + 1;
				else if (s.map.map[i] == PA_CHANNEL_POSITION_FRONT_RIGHT)
					channels[1] = i + 1;
			}
			return reply(ioDataSize, outData, channels, sizeof(channels));
		}
		case kAudioDevicePropertyPreferredChannelLayout:
		{
			if (!own)
				break;
			const PADeviceSnapshot s = snapshot();
			if (s.spec.channels <= 2)
				break;
			const UInt32 size = UInt32(offsetof(AudioChannelLayout, mChannelDescriptions) + s.spec.channels * sizeof(AudioChannelDescription));
			if (AudioChannelLayout* layout = static_cast<AudioChannelLayout*>(outData))
			{
				if (*ioDataSize < size)
					return kAudioHardwareBadPropertySizeError;
				memset(layout, 0, size);
				layout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
				layout->mNumberChannelDescriptions = s.spec.channels;
				for (unsigned i = 0; i < s.spec.channels; i++)
					layout->mChannelDescriptions[i].mChannelLabel = channelLabelOf(s.map.map[i], s.spec.channels);
			}
			*ioDataSize = size;
			return kAudioHardwareNoError;
		}
		case kAudioObjectPropertyElementName:
		{
			if (!own)
				return kAudioHardwareUnknownPropertyError;
			const PADeviceSnapshot s = snapshot();
			if (element < 1 || element > s.spec.channels)
				return kAudioHardwareUnknownPropertyError;
			const char* name = pa_channel_position_to_pretty_string(s.map.map[element - 1]);
			return replyString(ioDataSize, outData, name ? name : "");
		}
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeDecibels:
		case kVirtualMainVolume:
		{
			if (!control)
				return kAudioHardwareUnknownPropertyError;
			const bool main = inAddress->mSelector == kVirtualMainVolume;
			const PADeviceSnapshot s = snapshot();
			if (element > s.spec.channels || (main && element != 0))
				return kAudioHardwareUnknownPropertyError;
			const pa_volume_t volume = volumeOfElement(s, element);
			Float32 value = inAddress->mSelector == kAudioDevicePropertyVolumeDecibels ? decibelsOf(volume) : scalarOf(volume);
			if (!outData)
				return replyUInt32(ioDataSize, nullptr, sizeof(Float32));
			return reply(ioDataSize, outData, &value, sizeof(value));
		}
		case kAudioDevicePropertyVolumeRangeDecibels:
		{
			if (!control)
				return kAudioHardwareUnknownPropertyError;
			const PADeviceSnapshot s = snapshot();
			if (element > s.spec.channels)
				return kAudioHardwareUnknownPropertyError;
			// From the quietest the server can tell apart to the full volume of the device.
			AudioValueRange range = { Float64(pa_sw_volume_to_dB(1)), 0 };
			return reply(ioDataSize, outData, &range, sizeof(range));
		}
		case kVirtualMainBalance:
		{
			if (!control || element != 0)
				return kAudioHardwareUnknownPropertyError;
			const PADeviceSnapshot s = snapshot();
			if (!canBalance(s))
				return kAudioHardwareUnknownPropertyError;
			// 0 is all to the left, 1 all to the right, 0.5 even.
			Float32 balance = (pa_cvolume_get_balance(&s.volume, &s.map) + 1.0f) / 2.0f;
			return reply(ioDataSize, outData, &balance, sizeof(balance));
		}
		case kAudioDevicePropertyMute:
		{
			// The server mutes a device as a whole, there is no mute of a single channel.
			if (!control || element != 0)
				return kAudioHardwareUnknownPropertyError;
			const PADeviceSnapshot s = snapshot();
			return replyUInt32(ioDataSize, outData, s.mute ? 1 : 0);
		}
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyDataSources:
		case kAudioDevicePropertyDataSourceNameForID:
		case kAudioDevicePropertyJackIsConnected:
		{
			if (!own)
				return kAudioHardwareUnknownPropertyError;
			const PADeviceSnapshot s = snapshot();
			std::vector<uint32_t> codes;
			std::vector<bool> listed;
			dataSourcesOf(s, m_isInput, codes, listed);

			if (inAddress->mSelector == kAudioDevicePropertyJackIsConnected)
			{
				// A jack is a port for headphones or a headset: connected when the server has detected a plug.
				for (size_t i = 0; i < s.ports.size(); i++)
				{
					if (s.ports[i].kind != PAPortKind::Headphones && s.ports[i].kind != PAPortKind::Headset)
						continue;
					if (s.ports[i].available == PAPortAvailability::Unknown)
						continue;
					return replyUInt32(ioDataSize, outData, s.ports[i].available == PAPortAvailability::Yes ? 1 : 0);
				}
				return kAudioHardwareUnknownPropertyError;
			}

			if (s.ports.empty())
				return kAudioHardwareUnknownPropertyError;

			if (inAddress->mSelector == kAudioDevicePropertyDataSource)
			{
				if (s.activePort < 0)
					return kAudioHardwareUnknownPropertyError;
				return replyUInt32(ioDataSize, outData, codes[s.activePort]);
			}
			if (inAddress->mSelector == kAudioDevicePropertyDataSources)
			{
				std::vector<uint32_t> available;
				for (size_t i = 0; i < codes.size(); i++)
				{
					if (listed[i])
						available.push_back(codes[i]);
				}
				return replyList(ioDataSize, outData, available.data(), available.size(), sizeof(uint32_t));
			}

			// The name of a data source: the translation takes the code and gives the name.
			if (!outData)
			{
				*ioDataSize = sizeof(AudioValueTranslation);
				return kAudioHardwareNoError;
			}
			if (*ioDataSize < sizeof(AudioValueTranslation))
				return kAudioHardwareBadPropertySizeError;
			AudioValueTranslation* translation = static_cast<AudioValueTranslation*>(outData);
			if (!translation->mInputData || translation->mInputDataSize != sizeof(UInt32)
				|| !translation->mOutputData || translation->mOutputDataSize < sizeof(CFStringRef))
				return kAudioHardwareBadPropertySizeError;
			const UInt32 wanted = *static_cast<const UInt32*>(translation->mInputData);
			for (size_t i = 0; i < codes.size(); i++)
			{
				if (codes[i] != wanted)
					continue;
				*static_cast<CFStringRef*>(translation->mOutputData) = CFStringCreateWithCString(nullptr,
					s.ports[i].description.empty() ? s.ports[i].name.c_str() : s.ports[i].description.c_str(), kCFStringEncodingUTF8);
				translation->mOutputDataSize = sizeof(CFStringRef);
				*ioDataSize = sizeof(AudioValueTranslation);
				return kAudioHardwareNoError;
			}
			return kAudioHardwareIllegalOperationError;
		}
		case kAudioDevicePropertyHogMode:
		{
			const pid_t pid = m_hogPid;
			return reply(ioDataSize, outData, &pid, sizeof(pid));
		}
		case kAudioDevicePropertyStreamFormat:
		case kAudioDevicePropertyBufferFrameSize:
		case kAudioDevicePropertyBufferSize:
		case kAudioDevicePropertyBufferSizeRange:
			if (outData)
				refreshFormat();
			break;
	}

	// Fall through to the base class for everything already handled there
	// (stream format, buffer sizes, latency of an output, names, etc.)
	return AudioHardwareImplPA::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus AudioHardwareImplPADevice::isPropertySettable(const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeDecibels:
		case kVirtualMainVolume:
		case kVirtualMainBalance:
		case kAudioDevicePropertyMute:
		case kAudioDevicePropertyNominalSampleRate:
		case kAudioDevicePropertyDataSource:
		case kAudioDevicePropertyHogMode:
		case kAudioDevicePropertyStreamFormat:
		{
			UInt32 size = 0;
			OSStatus status = getPropertyData(inAddress, 0, nullptr, &size, nullptr);
			if (status != kAudioHardwareNoError)
			{
				*outIsSettable = false;
				return status;
			}
			*outIsSettable = true;
			if (inAddress->mSelector == kAudioDevicePropertyDataSource)
			{
				// There is something to choose from when the device has more than one data source.
				UInt32 listSize = 0;
				AudioObjectPropertyAddress sources = { kAudioDevicePropertyDataSources, inAddress->mScope, kAudioObjectPropertyElementMaster };
				*outIsSettable = getPropertyData(&sources, 0, nullptr, &listSize, nullptr) == kAudioHardwareNoError && listSize > sizeof(UInt32);
			}
			return kAudioHardwareNoError;
		}
	}
	return AudioHardwareImplPA::isPropertySettable(inAddress, outIsSettable);
}

OSStatus AudioHardwareImplPADevice::setVolume(AudioObjectPropertySelector selector, UInt32 scope, UInt32 element, Float32 value)
{
	if (!ownScope(scope) && scope != kAudioObjectPropertyScopeGlobal)
		return kAudioHardwareUnknownPropertyError;

	PADeviceSnapshot s;
	if (!fetchSnapshot(s))
		return kAudioHardwareUnspecifiedError;
	if (element > s.spec.channels)
		return kAudioHardwareUnknownPropertyError;

	pa_cvolume volume = s.volume;
	if (selector == kVirtualMainBalance)
	{
		if (element != 0 || !canBalance(s))
			return kAudioHardwareUnknownPropertyError;
		pa_cvolume_set_balance(&volume, &s.map, std::clamp(value, 0.0f, 1.0f) * 2.0f - 1.0f);
	}
	else
	{
		if (selector == kVirtualMainVolume && element != 0)
			return kAudioHardwareUnknownPropertyError;

		pa_volume_t target;
		if (selector == kAudioDevicePropertyVolumeDecibels)
			target = std::isnan(value) ? PA_VOLUME_MUTED : std::min<pa_volume_t>(pa_sw_volume_from_dB(value), PA_VOLUME_NORM);
		else
			target = pa_volume_t(std::clamp(value, 0.0f, 1.0f) * PA_VOLUME_NORM + 0.5f);

		if (element == 0)
		{
			// The whole device: every channel moves, the balance between them stays.
			if (pa_cvolume_max(&volume) == PA_VOLUME_MUTED)
				pa_cvolume_set(&volume, volume.channels, target);
			else
				pa_cvolume_scale(&volume, target);
		}
		else
			volume.values[element - 1] = target;
	}

	const std::string name = m_paName;
	const bool isInput = m_isInput;
	if (!paOperation(^pa_operation*(pa_context* ctx, pa_context_success_cb_t cb, void* userdata) {
			return isInput ? pa_context_set_source_volume_by_name(ctx, name.c_str(), &volume, cb, userdata)
			               : pa_context_set_sink_volume_by_name(ctx, name.c_str(), &volume, cb, userdata);
		}))
		return kAudioHardwareUnspecifiedError;

	checkChanges();
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPADevice::setMute(bool mute)
{
	const std::string name = m_paName;
	const bool isInput = m_isInput;
	if (!paOperation(^pa_operation*(pa_context* ctx, pa_context_success_cb_t cb, void* userdata) {
			return isInput ? pa_context_set_source_mute_by_name(ctx, name.c_str(), mute, cb, userdata)
			               : pa_context_set_sink_mute_by_name(ctx, name.c_str(), mute, cb, userdata);
		}))
		return kAudioHardwareUnspecifiedError;

	checkChanges();
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPADevice::setDataSource(UInt32 code)
{
	PADeviceSnapshot s;
	if (!fetchSnapshot(s) || s.ports.empty())
		return kAudioHardwareUnknownPropertyError;

	std::vector<uint32_t> codes;
	std::vector<bool> listed;
	dataSourcesOf(s, m_isInput, codes, listed);
	for (size_t i = 0; i < codes.size(); i++)
	{
		if (codes[i] != code || !listed[i])
			continue;

		const std::string name = m_paName, port = s.ports[i].name;
		const bool isInput = m_isInput;
		if (!paOperation(^pa_operation*(pa_context* ctx, pa_context_success_cb_t cb, void* userdata) {
				return isInput ? pa_context_set_source_port_by_name(ctx, name.c_str(), port.c_str(), cb, userdata)
				               : pa_context_set_sink_port_by_name(ctx, name.c_str(), port.c_str(), cb, userdata);
			}))
			return kAudioHardwareUnspecifiedError;
		checkChanges();
		return kAudioHardwareNoError;
	}
	return kAudioHardwareIllegalOperationError;
}

OSStatus AudioHardwareImplPADevice::setHogMode(pid_t pid)
{
	// The server shares its devices between all clients and has no exclusive access, so hogging
	// is only known to this process: it makes the device one that other users of the HAL in the
	// process find taken.
	if (!PAHogModeIsAllowed())
		return kAudioHardwareIllegalOperationError;
	if (pid == -1)
	{
		if (m_hogPid != -1 && m_hogPid != getpid())
			return kAudioHardwareIllegalOperationError;
		if (m_hogPid == -1)
			return kAudioHardwareNoError;
	}
	else if (pid != getpid() || (m_hogPid != -1 && m_hogPid != pid))
		return kAudioHardwareIllegalOperationError;

	m_hogPid = pid;
	notifyPropertyChanged(kAudioDevicePropertyHogMode);
	return kAudioHardwareNoError;
}

OSStatus AudioHardwareImplPADevice::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyVolumeScalar:
		case kAudioDevicePropertyVolumeDecibels:
		case kVirtualMainVolume:
		case kVirtualMainBalance:
			if (!ownScope(inAddress->mScope) && inAddress->mScope != kAudioObjectPropertyScopeGlobal)
				return kAudioHardwareUnknownPropertyError;
			if (inDataSize != sizeof(Float32) || !inData)
				return kAudioHardwareBadPropertySizeError;
			return setVolume(inAddress->mSelector, inAddress->mScope, inAddress->mElement, *static_cast<const Float32*>(inData));
		case kAudioDevicePropertyMute:
			// Only the side of the device that exists can be muted: an output of an output device.
			if ((!ownScope(inAddress->mScope) && inAddress->mScope != kAudioObjectPropertyScopeGlobal) || inAddress->mElement != 0)
				return kAudioHardwareUnknownPropertyError;
			if (inDataSize != sizeof(UInt32) || !inData)
				return kAudioHardwareBadPropertySizeError;
			return setMute(*static_cast<const UInt32*>(inData) != 0);
		case kAudioDevicePropertyNominalSampleRate:
			if (inDataSize != sizeof(Float64) || !inData)
				return kAudioHardwareBadPropertySizeError;
			return setNominalRate(*static_cast<const Float64*>(inData));
		case kAudioDevicePropertyDataSource:
			if (!ownScope(inAddress->mScope))
				return kAudioHardwareUnknownPropertyError;
			if (inDataSize != sizeof(UInt32) || !inData)
				return kAudioHardwareBadPropertySizeError;
			return setDataSource(*static_cast<const UInt32*>(inData));
		case kAudioDevicePropertyHogMode:
			if (inDataSize != sizeof(pid_t) || !inData)
				return kAudioHardwareBadPropertySizeError;
			return setHogMode(*static_cast<const pid_t*>(inData));
		case kAudioDevicePropertyStreamFormat:
		{
			OSStatus status = AudioHardwareImplPA::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
			if (status == kAudioHardwareNoError)
				m_formatSetByClient = true;
			return status;
		}
		case kAudioDevicePropertyDeviceUID:
		case kAudioDevicePropertyModelUID:
		case kAudioDevicePropertyTransportType:
		case kAudioDevicePropertyRelatedDevices:
		case kAudioDevicePropertyClockDomain:
		case kAudioDevicePropertyDeviceIsAlive:
		case kAudioDevicePropertyDeviceIsRunning:
		case kAudioDevicePropertyDeviceIsRunningSomewhere:
		case kAudioDevicePropertyStreamConfiguration:
		case kAudioDevicePropertyAvailableNominalSampleRates:
		case kAudioDevicePropertyActualSampleRate:
		case kAudioDevicePropertyDataSources:
		case kAudioDevicePropertyJackIsConnected:
		{
			// The property exists but is only to be read.
			UInt32 size = 0;
			AudioObjectPropertyAddress address = *inAddress;
			if (getPropertyData(&address, 0, nullptr, &size, nullptr) == kAudioHardwareUnknownPropertyError)
				return kAudioHardwareUnknownPropertyError;
			return kAudioHardwareUnsupportedOperationError;
		}
	}

	return AudioHardwareImplPA::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}

OSStatus AudioHardwareImplPADevice::addPropertyListener(const AudioObjectPropertyAddress* inAddress,
	AudioObjectPropertyListenerProc inListener, void* inClientData)
{
	// What the listener is told about later is the difference to what the device is now.
	bool needBaseline;
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		needBaseline = !m_haveTold;
	}
	if (needBaseline)
		checkChanges();
	return AudioHardwareImplPA::addPropertyListener(inAddress, inListener, inClientData);
}

void AudioHardwareImplPADevice::ioStateChanged()
{
	// Not from the thread that started or stopped the IO, which may be an IOProc.
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		notifyPropertyChanged(kAudioDevicePropertyDeviceIsRunning);
		checkChanges();
	});
}

void AudioHardwareImplPADevice::checkChanges()
{
	PADeviceSnapshot now;
	if (!fetchSnapshot(now))
		return;

	struct Change { AudioObjectPropertySelector selector; UInt32 scope, element; };
	std::vector<Change> changes;
	const UInt32 ownScopeValue = m_isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
	const bool running = isRunningHere();

	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		if (m_haveTold)
		{
			const PADeviceSnapshot& before = m_told;
			auto volumeChange = [&](UInt32 element, pa_volume_t was, pa_volume_t is) {
				if (scalarOf(was) == scalarOf(is) && was == is)
					return;
				changes.push_back({ kAudioDevicePropertyVolumeScalar, ownScopeValue, element });
				changes.push_back({ kAudioDevicePropertyVolumeDecibels, ownScopeValue, element });
				if (element == 0)
					changes.push_back({ kVirtualMainVolume, ownScopeValue, 0 });
			};
			if (before.volume.channels == now.volume.channels)
			{
				volumeChange(0, pa_cvolume_max(&before.volume), pa_cvolume_max(&now.volume));
				for (unsigned i = 0; i < now.volume.channels; i++)
					volumeChange(i + 1, before.volume.values[i], now.volume.values[i]);
				if (canBalance(now) && pa_cvolume_get_balance(&before.volume, &before.map) != pa_cvolume_get_balance(&now.volume, &now.map))
					changes.push_back({ kVirtualMainBalance, ownScopeValue, 0 });
			}
			if (before.mute != now.mute)
				changes.push_back({ kAudioDevicePropertyMute, ownScopeValue, 0 });
			if (before.activePort != now.activePort || before.ports.size() != now.ports.size())
			{
				changes.push_back({ kAudioDevicePropertyDataSource, ownScopeValue, 0 });
				changes.push_back({ kAudioDevicePropertyDataSources, ownScopeValue, 0 });
				changes.push_back({ kAudioDevicePropertyJackIsConnected, ownScopeValue, 0 });
			}
			else
			{
				for (size_t i = 0; i < now.ports.size(); i++)
				{
					if (before.ports[i].available != now.ports[i].available)
					{
						changes.push_back({ kAudioDevicePropertyDataSources, ownScopeValue, 0 });
						changes.push_back({ kAudioDevicePropertyJackIsConnected, ownScopeValue, 0 });
						break;
					}
				}
			}
			if (before.spec.rate != now.spec.rate && m_nominalRate == 0)
			{
				changes.push_back({ kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, 0 });
				changes.push_back({ kAudioDevicePropertyActualSampleRate, kAudioObjectPropertyScopeGlobal, 0 });
			}
			if ((before.running || m_toldRunningHere) != (now.running || running))
				changes.push_back({ kAudioDevicePropertyDeviceIsRunningSomewhere, kAudioObjectPropertyScopeGlobal, 0 });
		}
		m_told = now;
		m_toldRunningHere = running;
		m_haveTold = true;
	}

	for (const Change& change : changes)
		notifyPropertyChanged(change.selector, change.scope, change.element);
}
