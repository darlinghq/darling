// src/CoreAudio/CoreAudio/pulse/AudioHardwareImplPADevice.h
#ifndef AUDIOHARDWAREIMPLPADEVICE_H
#define AUDIOHARDWAREIMPLPADEVICE_H
#include "AudioHardwareImplPA.h"
#include <string>

// Represents one specific, currently-known PulseAudio sink or source,
// discovered by AudioHardwareImplPA. Unlike AudioHardwareImplPA (which owns
// the PulseAudio connection and the whole-registry properties on the system
// object), this class answers *per-device* properties for one fixed
// PulseAudio name. A later task wires real discovery (enumerating PulseAudio
// sinks/sources and constructing one of these per device) on top of this
// class; this task only introduces the class itself.
//
// Streams created on it connect to this sink or source rather than to the server default.
class AudioHardwareImplPADevice : public AudioHardwareImplPA
{
public:
	// isInput selects whether `paName` is a sink or a source.
	AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, const char* description, bool isInput);
	~AudioHardwareImplPADevice();

	OSStatus getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData) override;

	OSStatus setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData) override;

	const char* paDeviceName() const override { return m_paName.c_str(); }
	bool isInputDevice() const override { return m_isInput; }

protected:
	AudioHardwareStream* createStream(AudioDeviceIOProc callback, void* clientData) override;
private:
	std::string m_paName;
	bool m_isInput;
	Float32 m_lastVolume = 1.0f;
};

#endif /* AUDIOHARDWAREIMPLPADEVICE_H */
