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
	}

	// Fall through to the base class for everything already handled there
	// (stream format, manufacturer, name, volume stubs, etc.)
	return AudioHardwareImplPA::getPropertyData(inAddress, inQualifierDataSize, inQualifierData, ioDataSize, outData);
}
