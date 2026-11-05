/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef AUDIOHARDWAREIMPLPA_H
#define AUDIOHARDWAREIMPLPA_H
#include "../AudioHardwareImpl.h"
#include "PADispatchMainLoop.h"
#include "PADeviceRegistry.h"
#include <memory>
#include <unordered_map>
#include <vector>

class PADispatchMainLoop;

// A device's stream object has the ID kPAStreamObjectBase | <device ID>.
static const AudioObjectID kPAStreamObjectBase = 0x44410000u;

// Defined in AudioHardware.cpp. Shared so AudioHardwareImplPA can add/remove
// dynamically-discovered PulseAudio devices from the same object map used
// to dispatch AudioObjectID-based CoreAudio calls.
std::unordered_map<AudioObjectID, std::unique_ptr<AudioHardwareImpl>>& PAGetObjectMap();
// Guards PAGetObjectMap(): it changes on PulseAudio events while clients look objects up.
std::mutex& PAGetObjectMapMutex();

class AudioHardwareImplPA : public AudioHardwareImpl
{
public:
	AudioHardwareImplPA(AudioObjectID myId, const char* paRole = nullptr);
	~AudioHardwareImplPA();

	OSStatus getPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData) override;

	OSStatus setPropertyData(const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData) override;

	void getPAContext(void (^cb)(pa_context*));
	// Runs the block on the PulseAudio queue and waits for it (directly, if already on that queue).
	void syncPA(void (^block)(void));
	bool onPAQueue() const { return m_loop && m_loop->onQueue(); }
	// The PulseAudio sink/source streams connect to; nullptr selects the server default.
	virtual const char* paDeviceName() const { return nullptr; }
	OSStatus stop(AudioDeviceIOProcID inProcID) override;
	static pa_sample_spec paSampleSpecForASBD(const AudioStreamBasicDescription& asbd, bool* convertSignedUnsigned = nullptr);

	// Synchronously (re)enumerates PulseAudio sinks/sources, creating/removing
	// AudioHardwareImplPADevice entries in PAGetObjectMap() as needed. Safe to
	// call repeatedly (e.g. on every kAudioHardwarePropertyDevices query, or
	// whenever PulseAudio reports a sink/source change).
	void refreshDeviceList();

	// All currently-known dynamically-discovered device AudioObjectIDs
	// (does not include the 3 fixed role IDs +1/+2/+3).
	std::vector<AudioObjectID> allDeviceIds() const;
protected:
	AudioHardwareStream* createStream(AudioDeviceIOProc callback, void* clientData) override;
	bool validateFormat(const AudioStreamBasicDescription* asbd) const override;
private:
	// Seeded so dynamically discovered device IDs never collide with the 4
	// fixed IDs (0..3: system object, default-output, default-input, event-output).
	static PADeviceRegistry& registry();
private:
	pa_context* m_context = nullptr;
	std::unique_ptr<PADispatchMainLoop> m_loop;
	const char* m_paRole;
	std::mutex m_contextMutex;
};

#endif
