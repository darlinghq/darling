// src/CoreAudio/CoreAudio/pulse/AudioHardwareImplPADevice.h
#ifndef AUDIOHARDWAREIMPLPADEVICE_H
#define AUDIOHARDWAREIMPLPADEVICE_H
#include "AudioHardwareImplPA.h"
#include "PADeviceInfo.h"
#include <mutex>
#include <string>
#include <sys/types.h>
#include <vector>

// What the server said about the sink or source of a device at one moment.
struct PADeviceSnapshot
{
	bool valid = false;
	uint32_t index = PA_INVALID_INDEX;
	uint32_t card = PA_INVALID_INDEX;
	std::string name, description, driver, bus, api, formFactor, vendor, product, serial;
	pa_sample_spec spec = {};
	pa_channel_map map = {};
	pa_cvolume volume = {};
	bool mute = false;
	bool running = false;               // the server is moving audio through the device
	uint64_t configuredLatencyUsec = 0;
	std::vector<PAPortDesc> ports;
	int activePort = -1;                // index into ports, -1 if there are none
};

// A sink or a source of the PulseAudio server, as an audio device of the HAL.
class AudioHardwareImplPADevice : public AudioHardwareImplPA
{
public:
	// isInput selects whether `paName` is a sink or a source.
	AudioHardwareImplPADevice(AudioObjectID myId, std::string paName, const char* description, bool isInput,
		const char* paRole = nullptr);
	~AudioHardwareImplPADevice();

	OSStatus getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData) override;

	OSStatus setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData) override;

	OSStatus isPropertySettable(const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable) override;

	OSStatus addPropertyListener(const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData) override;

	const char* paDeviceName() const override { return m_paName.c_str(); }
	bool isInputDevice() const override { return m_isInput; }

	void refreshFormat() override;
	std::vector<Float64> availableSampleRates() override;
	Float64 nominalSampleRate() override;

	// Compares the device with what its listeners were last told and tells them what changed.
	// Called when the server reports a change of a sink or source.
	void checkChanges();
	bool hasListeners() const { return !m_listeners.empty(); }

protected:
	AudioHardwareStream* createStream(AudioDeviceIOProc callback, void* clientData) override;
	void ioStateChanged() override;
	// The name that is looked up on the server; the server's own default for the objects of the default devices.
	std::string m_paName;
private:
	// The server's answer for the device, or one made up of the current format if it did not answer.
	PADeviceSnapshot snapshot();
	bool fetchSnapshot(PADeviceSnapshot& out);
	// Runs an operation of the server on the device and waits until the server has done it.
	bool paOperation(pa_operation* (^start)(pa_context*, pa_context_success_cb_t, void*));
	OSStatus setVolume(AudioObjectPropertySelector selector, UInt32 scope, UInt32 element, Float32 value);
	OSStatus setMute(bool mute);
	OSStatus setNominalRate(Float64 rate);
	OSStatus setDataSource(UInt32 code);
	OSStatus setHogMode(pid_t pid);
	bool isRunningHere();
	bool ownScope(UInt32 scope) const;

	bool m_isInput;
	std::mutex m_stateMutex;
	PADeviceSnapshot m_cached;       // the last answer of the server
	PADeviceSnapshot m_told;         // what the listeners were last told
	bool m_haveTold = false;
	bool m_toldRunningHere = false;
	bool m_formatSetByClient = false;
	Float64 m_nominalRate = 0;       // chosen by a client, 0 to follow the server
	pid_t m_hogPid = -1;
};

#endif /* AUDIOHARDWAREIMPLPADEVICE_H */
