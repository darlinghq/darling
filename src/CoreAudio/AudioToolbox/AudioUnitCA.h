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

#ifndef AUDIOUNITCA_H
#define	AUDIOUNITCA_H
#include "AudioOutputUnitComponent.h"
#include <CoreAudio/AudioHardware.h>

#ifndef kOutputBus
#	define kOutputBus		0 // playback
#	define kInputBus		1 // recording
#endif

class AudioUnitCA : public AudioOutputUnitComponent
{
public:
	AudioUnitCA();
	virtual ~AudioUnitCA();
	
	OSStatus init() override;
	OSStatus deinit() override;

	OSStatus start() override;
	OSStatus stop() override;
	
	OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) override;
	
	OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) override;
	
	int cardIndex() const override;
private:
	OSStatus initOutput();
	OSStatus initInput();
private:
	void requestDataForPlayback(size_t length);
	
	inline bool isOutputPlanar() const { return m_config[kOutputBus].second.mFormatFlags & kAudioFormatFlagIsNonInterleaved; }
	inline bool isInputPlanar() const { return m_config[kInputBus].first.mFormatFlags & kAudioFormatFlagIsNonInterleaved; }
	
	OSStatus renderOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderInterleavedOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderPlanarOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	
	OSStatus renderInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderInterleavedInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderPlanarInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);	
private:
	
	bool m_started = false;
	AudioStreamBasicDescription m_deviceASBD;
	AudioDeviceIOProcID m_outputProcID, m_inputProcID;
};

#endif	/* AUDIOUNITPA_H */

