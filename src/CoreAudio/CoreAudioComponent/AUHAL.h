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

#ifndef CAC_AUHAL_H
#define CAC_AUHAL_H
#include "AUBase.h"
#include <CoreAudio/AudioHardware.h>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <stdint.h>

class AUHAL : public AUBase
{
public:
	AUHAL(AudioComponentInstance inInstance, bool supportRecording = true);
	~AUHAL();

	bool CanScheduleParameters() const override;
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override;
	OSStatus Version() override;

	OSStatus Start() override;
	OSStatus Stop() override;

	OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize) override;
	OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32& outDataSize, Boolean& outWritable) override;
	OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData) override;

	OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inNumberFrames) override;
private:
	static OSStatus playbackCallback(AudioObjectID inObjectID,
		const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
		const AudioTimeStamp* inInputTime,
		AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime,
		void* inClientData);
	static OSStatus recordCallback(AudioObjectID inObjectID,
		const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
		const AudioTimeStamp* inInputTime,
		AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime,
		void* inClientData);

	OSStatus doPlayback(const AudioTimeStamp* inNow, AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime);
	OSStatus doRecord(const AudioTimeStamp* inNow, const AudioBufferList* inInputData, const AudioTimeStamp* inInputTime);
	
protected:
	AURenderCallbackStruct m_outputAvailableCb = { 0 };
	bool m_enableOutput = true, m_enableInput = false;
	bool m_running = false;

	AudioDeviceID m_outputDevice, m_inputDevice;
	AudioDeviceIOProcID m_outputProcID = 0, m_inputProcID = 0;

	bool m_dataAvailable = false;
	std::condition_variable m_dataAvailableCV;
	std::mutex m_dataAvailableMutex;

	// TODO: Prepare for non-interleaved audio
	std::unique_ptr<uint8_t[]> m_buffer;
	size_t m_bufferSize = 0, m_bufferUsed = 0;
};

#endif

