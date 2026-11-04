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
class AudioHardwareImplPADevice : public AudioHardwareImplPA
{
public:
	// isInput selects whether `paName` is looked up via get_sink_info or
	// get_source_info when answering stream-format/sample-rate properties
	// (wired up in a later task; for now this constructor just records the
	// name and role).
	AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, bool isInput);

	OSStatus getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData) override;

	OSStatus setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData) override;

	const std::string& paName() const { return m_paName; }
	bool isInput() const { return m_isInput; }

private:
	std::string m_paName;
	bool m_isInput;
};

#endif /* AUDIOHARDWAREIMPLPADEVICE_H */
