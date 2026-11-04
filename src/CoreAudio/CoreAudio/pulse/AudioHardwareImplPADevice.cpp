// src/CoreAudio/CoreAudio/pulse/AudioHardwareImplPADevice.cpp
#include "AudioHardwareImplPADevice.h"
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>

AudioHardwareImplPADevice::AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, bool isInput)
: AudioHardwareImplPA(myId, nullptr)
, m_paName(std::move(paName))
, m_isInput(isInput)
{
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
		case kAudioDevicePropertyVolumeScalar:
		{
			if (Float32* v = static_cast<Float32*>(outData); v && *ioDataSize >= sizeof(Float32))
			{
				Float32 result = 1.0f;
				const std::string& name = m_paName;
				bool isInput = m_isInput;

				getPAContext(^(pa_context* ctx) {
					if (!ctx)
						return;

					pa_operation* op = isInput
						? pa_context_get_source_info_by_name(ctx, name.c_str(), [](pa_context*, const pa_source_info* info, int, void* ud)
							{ if (info) *static_cast<Float32*>(ud) = pa_cvolume_avg(&info->volume) / (float)PA_VOLUME_NORM; }, &result)
						: pa_context_get_sink_info_by_name(ctx, name.c_str(), [](pa_context*, const pa_sink_info* info, int, void* ud)
							{ if (info) *static_cast<Float32*>(ud) = pa_cvolume_avg(&info->volume) / (float)PA_VOLUME_NORM; }, &result);

					if (op)
					{
						while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
							; // getPAContext's callback already runs on PADispatchMainLoop's queue, which is what drives this operation to completion
						pa_operation_unref(op);
					}
				});

				*v = result;
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
		pa_volume_t paVolume = (pa_volume_t)(volume * PA_VOLUME_NORM);
		const std::string& name = m_paName;
		bool isInput = m_isInput;

		getPAContext(^(pa_context* ctx) {
			if (!ctx)
				return;

			pa_cvolume cv;
			pa_cvolume_set(&cv, 2, paVolume);
			pa_operation* op = isInput
				? pa_context_set_source_volume_by_name(ctx, name.c_str(), &cv, nullptr, nullptr)
				: pa_context_set_sink_volume_by_name(ctx, name.c_str(), &cv, nullptr, nullptr);
			if (op)
				pa_operation_unref(op);
		});

		notifyPropertyChanged(kAudioDevicePropertyVolumeScalar);
		return kAudioHardwareNoError;
	}

	return AudioHardwareImplPA::setPropertyData(inAddress, inQualifierDataSize, inQualifierData, inDataSize, inData);
}
