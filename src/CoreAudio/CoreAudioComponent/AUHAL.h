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
#include "AURingBuffer.h"
#include <CoreAudio/AudioHardware.h>
#include <memory>
#include <stdint.h>
#include <vector>

class AUHAL : public AUBase
{
public:
	AUHAL(AudioComponentInstance inInstance, bool supportRecording = true);
	~AUHAL();

	UInt32 GetChannelLayoutTags(AudioUnitScope scope, AudioUnitElement element, AudioChannelLayoutTag* tags) override;
	UInt32 GetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element, AudioChannelLayout* layout, Boolean& writable) override;
	OSStatus SetAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element, const AudioChannelLayout* layout) override;
	bool CanScheduleParameters() const override;
	bool StreamFormatWritable(AudioUnitScope scope, AudioUnitElement element) override;
	OSStatus Version() override;

	OSStatus Start() override;
	OSStatus Stop() override;

	OSStatus SetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, const void* inData, UInt32 inDataSize) override;
	OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, UInt32& outDataSize, Boolean& outWritable) override;
	OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope, AudioUnitElement inElement, void* outData) override;

	OSStatus RenderBus(AudioUnitRenderActionFlags& ioActionFlags, const AudioTimeStamp& inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames) override;

	// Only reachable while stopped (StreamFormatWritable() requires !m_running);
	// invalidates m_recordRingBuffer so a changed channel count can't be read
	// through a ring buffer still sized/channeled for the old format.
	OSStatus ChangeStreamFormat(AudioUnitScope inScope, AudioUnitElement inElement,
		const CAStreamBasicDescription& inPrevFormat, const CAStreamBasicDescription& inNewFormat) override;

	// The client sides accept any format PulseAudio converts and our copies handle,
	// in either layout; the device sides keep AUBase's canonical rule.
	bool ValidFormat(AudioUnitScope inScope, AudioUnitElement inElement,
		const CAStreamBasicDescription& inNewFormat) override;

	static OSStatus ComponentEntryDispatch(ComponentParameters *params, AUHAL *This);
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

	// Holds interleaved data in the bus 1 output format. Constructed in Start() (while !m_running, before the recording IOProc
	// is installed) so doRecord() and Render() -- which run on separate
	// threads once started -- never race over construction. Decouples the
	// HAL's IOProc callback rate from Render()'s AU-render-pull rate; an
	// underflowing Render() is silence-filled rather than blocking the
	// render thread.
	std::unique_ptr<AURingBuffer> m_recordRingBuffer;
	std::vector<unsigned char> m_recordScratch;

	// Conversion storage for planar client formats. Playback callbacks run serially.
	std::vector<unsigned char> m_playbackSamples;
	std::vector<unsigned char> m_playbackBufferList;
};

#endif

