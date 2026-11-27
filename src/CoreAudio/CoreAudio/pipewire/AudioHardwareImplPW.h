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
#include <chrono>
#include <mutex>
#include <string>

struct darling_pw_volume;

// The HAL objects describe roles (default output, input, system output). Their
// volume and mute follow the server's default sink or source node (pipe_volume.h);
// while no server can be reached the values are kept locally.
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
	OSStatus addPropertyListener(const AudioObjectPropertyAddress* address,
		AudioObjectPropertyListenerProc listener, void* clientData) override;
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

	static std::atomic<UInt32>* systemSetting(AudioObjectPropertySelector selector);
	static bool uidFromCFString(CFStringRef string, std::string& out);
	static OSStatus reply(UInt32* ioDataSize, void* outData, const void* value, UInt32 size);
	static OSStatus replyArray(UInt32* ioDataSize, void* outData,
		const void* values, UInt32 count, UInt32 elementSize);
	bool hasActiveStreams();
	// Connects to the server's default node for volume and mute, if not yet done.
	// Returns the connection or nullptr when no server can be reached.
	darling_pw_volume* volumeClient();
	static void volumeChangedByServer(void* userData);
	void volumeChanged();
	bool ownsScope(UInt32 scope) const;

	bool m_system;
	bool m_input;
	std::string m_role;
	Float64 m_nominalRate = 0;
	std::atomic<uint32_t> m_latencyFrames{0};
	std::atomic<uint64_t> m_sampleClock{0};
	std::atomic<Float32> m_volume{1.0f};
	std::atomic<UInt32> m_mute{0};
	std::mutex m_volumeMutex;
	// Read by the PipeWire thread, which must never wait on m_volumeMutex: the
	// opener holds it while that thread delivers the first events.
	std::atomic<darling_pw_volume*> m_volumeClient{nullptr};
	std::chrono::steady_clock::time_point m_volumeAttempt{};
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
