// src/CoreAudio/CoreAudio/pulse/AudioHardwareImplPADevice.cpp
#include "AudioHardwareImplPADevice.h"
#include "AudioHardwareStreamPAInput.h"
#include "AudioHardwareStreamPAOutput.h"
#include <algorithm>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>

AudioHardwareImplPADevice::AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, const char* description, bool isInput)
: AudioHardwareImplPA(myId, nullptr)
, m_paName(std::move(paName))
, m_isInput(isInput)
{
	m_name = CFStringCreateWithCString(nullptr, description ? description : m_paName.c_str(), kCFStringEncodingUTF8);
}

AudioHardwareImplPADevice::~AudioHardwareImplPADevice()
{
	CFRelease(m_name);
}

AudioHardwareStream* AudioHardwareImplPADevice::createStream(AudioDeviceIOProc callback, void* clientData)
{
	if (m_isInput)
		return new AudioHardwareStreamPAInput(this, callback, clientData);
	return new AudioHardwareStreamPAOutput(this, callback, clientData);
}

struct PAVolumeQuery
{
	Float32 volume;
	dispatch_semaphore_t done;
};

static void paSinkVolumeCB(pa_context*, const pa_sink_info* info, int eol, void* userdata)
{
	PAVolumeQuery* query = static_cast<PAVolumeQuery*>(userdata);
	if (info)
		query->volume = pa_cvolume_avg(&info->volume) / (float) PA_VOLUME_NORM;
	if (eol)
		dispatch_semaphore_signal(query->done);
}

static void paSourceVolumeCB(pa_context*, const pa_source_info* info, int eol, void* userdata)
{
	PAVolumeQuery* query = static_cast<PAVolumeQuery*>(userdata);
	if (info)
		query->volume = pa_cvolume_avg(&info->volume) / (float) PA_VOLUME_NORM;
	if (eol)
		dispatch_semaphore_signal(query->done);
}

OSStatus AudioHardwareImplPADevice::getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32* ioDataSize, void* outData)
{
	switch (inAddress->mSelector)
	{
		case kAudioDevicePropertyDeviceUID:
		{
			if (CFStringRef* ref = static_cast<CFStringRef*>(outData); ref && *ioDataSize >= sizeof(CFStringRef))
			{
				CFStringRef uid = CFStringCreateWithCString(nullptr, m_paName.c_str(), kCFStringEncodingUTF8);
				*ref = uid;
			}
			*ioDataSize = sizeof(CFStringRef);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyTransportType:
		{
			if (UInt32* v = static_cast<UInt32*>(outData); v && *ioDataSize >= sizeof(UInt32))
				*v = kAudioDeviceTransportTypeVirtual;
			*ioDataSize = sizeof(UInt32);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyStreamConfiguration:
		{
			const bool hasChannels = inAddress->mScope == (m_isInput ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput);

			if (AudioBufferList* abl = static_cast<AudioBufferList*>(outData); abl && *ioDataSize >= sizeof(AudioBufferList))
			{
				abl->mNumberBuffers = hasChannels ? 1 : 0;
				if (hasChannels)
				{
					abl->mBuffers[0].mNumberChannels = asbd().mChannelsPerFrame;
					abl->mBuffers[0].mData = nullptr;
					abl->mBuffers[0].mDataByteSize = 0;
				}
			}
			*ioDataSize = sizeof(AudioBufferList);
			return kAudioHardwareNoError;
		}
		case kAudioDevicePropertyVolumeScalar:
		{
			if (Float32* v = static_cast<Float32*>(outData); v && *ioDataSize >= sizeof(Float32))
			{
				// The answer arrives on the PulseAudio queue, which an IOProc of this device would be blocking.
				if (onPAQueue())
				{
					*v = m_lastVolume;
					*ioDataSize = sizeof(Float32);
					return kAudioHardwareNoError;
				}

				PAVolumeQuery query = { m_lastVolume, dispatch_semaphore_create(0) };
				PAVolumeQuery* queryPtr = &query;
				__block pa_operation* op = nullptr;
				const char* name = m_paName.c_str();
				const bool isInput = m_isInput;

				getPAContext(^(pa_context* ctx) {
					if (ctx)
					{
						op = isInput
							? pa_context_get_source_info_by_name(ctx, name, paSourceVolumeCB, queryPtr)
							: pa_context_get_sink_info_by_name(ctx, name, paSinkVolumeCB, queryPtr);
					}
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
				m_lastVolume = query.volume;
				*v = query.volume;
			}
			*ioDataSize = sizeof(Float32);
			return kAudioHardwareNoError;
		}
	}

	// Fall through to the base class for everything already handled there
	// (stream format, manufacturer, name, volume stubs, etc.)
	return AudioHardwareImplPA::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}

OSStatus AudioHardwareImplPADevice::setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
	const void* inQualifierData, UInt32 inDataSize, const void* inData)
{
	if (inAddress->mSelector == kAudioDevicePropertyVolumeScalar)
	{
		if (inDataSize != sizeof(Float32))
			return kAudioHardwareBadPropertySizeError;

		Float32 volume = *static_cast<const Float32*>(inData);
		pa_volume_t paVolume = (pa_volume_t)(std::clamp(volume, 0.0f, 1.0f) * PA_VOLUME_NORM);
		const char* name = m_paName.c_str();
		bool isInput = m_isInput;
		m_lastVolume = volume;

		getPAContext(^(pa_context* ctx) {
			if (!ctx)
				return;

			// A single channel applies to all of the device's channels, however many it has.
			pa_cvolume cv;
			pa_cvolume_set(&cv, 1, paVolume);
			pa_operation* op = isInput
				? pa_context_set_source_volume_by_name(ctx, name, &cv, nullptr, nullptr)
				: pa_context_set_sink_volume_by_name(ctx, name, &cv, nullptr, nullptr);
			if (op)
				pa_operation_unref(op);
		});

		notifyPropertyChanged(kAudioDevicePropertyVolumeScalar);
		return kAudioHardwareNoError;
	}

	return AudioHardwareImplPA::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}
