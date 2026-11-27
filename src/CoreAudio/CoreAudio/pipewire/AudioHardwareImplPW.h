/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef AUDIOHARDWAREIMPLPW_H
#define AUDIOHARDWAREIMPLPW_H

#include "../AudioHardwareImpl.h"
#include <atomic>
#include <string>

// The HAL objects describe roles (default output, input, system output) and
// read volume and mute from the PipeWire server's selected default node. The
// atomics cache the last server value so property listeners can be notified
// without querying the server again.
class AudioHardwareImplPW : public AudioHardwareImpl
{
public:
	explicit AudioHardwareImplPW(AudioObjectID myId);
	~AudioHardwareImplPW() override;

	OSStatus getPropertyData(const AudioObjectPropertyAddress* address,
		UInt32 qualifierDataSize, const void* qualifierData,
		UInt32* ioDataSize, void* outData) override;
	OSStatus setPropertyData(const AudioObjectPropertyAddress* address,
		UInt32 qualifierDataSize, const void* qualifierData,
		UInt32 dataSize, const void* inData) override;
	OSStatus isPropertySettable(const AudioObjectPropertyAddress* address,
		Boolean* outIsSettable) override;
	OSStatus stop(AudioDeviceIOProcID procID) override;
	OSStatus getCurrentTime(AudioTimeStamp* outTime) override;

	void setLatencyFrames(uint32_t frames) { m_latencyFrames.store(frames); }
	void advanceSampleClock(uint64_t frames) { m_sampleClock.fetch_add(frames); }

protected:
	AudioHardwareImplPW(AudioObjectID myId, const char* name, const char* uid,
		bool input, const char* role);

	bool isInputDevice() const override { return m_input; }
	std::vector<Float64> availableSampleRates() override;
	Float64 nominalSampleRate() override;
	AudioHardwareStream* createStream(AudioDeviceIOProc callback, void* clientData) override;
	bool validateFormat(const AudioStreamBasicDescription* format) const override;

	bool systemObject() const { return m_system; }
	const std::string& role() const { return m_role; }

private:
	OSStatus systemPropertyData(const AudioObjectPropertyAddress* address,
		UInt32 qualifierDataSize, const void* qualifierData,
		UInt32* ioDataSize, void* outData);
	OSStatus devicePropertyData(const AudioObjectPropertyAddress* address,
		UInt32* ioDataSize, void* outData);
	OSStatus translateUID(const AudioObjectPropertyAddress* address,
		UInt32 qualifierDataSize, const void* qualifierData,
		UInt32* ioDataSize, void* outData);
	OSStatus setNominalRate(Float64 rate);

	static void audioStateChanged(void* userData, float volume, int mute);

	static std::atomic<UInt32>* systemSetting(AudioObjectPropertySelector selector);
	static bool uidFromCFString(CFStringRef string, std::string& out);
	static OSStatus reply(UInt32* ioDataSize, void* outData, const void* value, UInt32 size);
	static OSStatus replyArray(UInt32* ioDataSize, void* outData,
		const void* values, UInt32 count, UInt32 elementSize);
	bool hasActiveStreams();
	bool ownsScope(UInt32 scope) const;

	bool m_system;
	bool m_input;
	std::string m_role;
	Float64 m_nominalRate = 0;
	std::atomic<uint32_t> m_latencyFrames{0};
	std::atomic<uint64_t> m_sampleClock{0};
	std::atomic<Float32> m_volume{1.0f};
	std::atomic<UInt32> m_mute{0};
	struct darling_pw_audio_state* m_audioState = nullptr;
};

class AudioHardwareImplPWOutput : public AudioHardwareImplPW
{
public:
	explicit AudioHardwareImplPWOutput(AudioObjectID myId, const char* role = nullptr);
};

class AudioHardwareImplPWInput : public AudioHardwareImplPW
{
public:
	explicit AudioHardwareImplPWInput(AudioObjectID myId);
};

#endif /* AUDIOHARDWAREIMPLPW_H */
