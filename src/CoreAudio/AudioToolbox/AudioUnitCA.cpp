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
#include "AudioUnitCA.h"

#include <CoreServices/MacErrors.h>

AudioUnitCA::AudioUnitCA()
{
}

AudioUnitCA::~AudioUnitCA()
{
	deinit();
}

OSStatus AudioUnitCA::init()
{
	OSStatus ret = noErr;
	
	// kAudioUnitErr_FailedInitialization
	if (m_enableOutput)
	{
		ret = initOutput();

		if (ret != noErr)
			return ret;
	}

	if (m_enableInput)
		ret = initInput();
	
	return ret;
}

OSStatus AudioUnitCA::initOutput()
{
	AudioDeviceID adid;
	UInt32 propSize = sizeof(adid);

	AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propSize, &adid);

	propSize = sizeof(m_deviceASBD);
	AudioDeviceGetProperty(adid, 0, false, kAudioDevicePropertyStreamFormat, &propSize, &m_deviceASBD);

	AudioDeviceCreateIOProcID(adid, playbackCallback, this, &m_outputProcID);
	
	return noErr;
}

void AudioUnitCA::requestDataForPlayback(size_t length)
{
	AudioUnitRenderActionFlags flags;
	AudioTimeStamp ts;
	AudioBufferList* bufs;
	OSStatus err;
	std::unique_ptr<uint8_t[]> data;
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	const UInt32 cc = config.mChannelsPerFrame;

	
	//TRACE() << "m_started=" << m_started;
	
	if (!m_started)
	{
		std::unique_ptr<char[]> empty;
		size_t len;
		int ret;
		
		len = config.mBytesPerFrame * (config.mSampleRate / 10);
		empty.reset(new char[len]);
		
		memset(empty.get(), 0, len);
		
		//ret = pa_stream_write(m_stream, empty.get(), len,
		//		nullptr, 0, PA_SEEK_RELATIVE);
		
		return;
	}
	
	memset(&ts, 0, sizeof(ts));
	
	if (isOutputPlanar())
	{
		bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList) + (cc-1)*sizeof(AudioBuffer));
		bufs->mNumberBuffers = cc;
	}
	else
	{
		bufs = (AudioBufferList*) operator new(sizeof(AudioBufferList));
		bufs->mNumberBuffers = 1;
	}
	
	if (m_shouldAllocateBuffer)
	{
		if (isOutputPlanar())
		{
			UInt32 bytesPerChannel = length / cc;
			
			data.reset(new uint8_t[cc*bytesPerChannel]);
			
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mNumberChannels = 1;
				bufs->mBuffers[i].mDataByteSize = bytesPerChannel;
				bufs->mBuffers[i].mData = data.get() + i*bytesPerChannel;
			}
		}
		else
		{
			bufs->mBuffers[0].mNumberChannels = cc;
			bufs->mBuffers[0].mDataByteSize = length;
		
			data.reset(new uint8_t[bufs->mBuffers[0].mDataByteSize]);
			bufs->mBuffers[0].mData = data.get();
		}
	}
	else
	{
		if (isOutputPlanar())
		{
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mDataByteSize = 0;
				bufs->mBuffers[i].mData = nullptr;
			}
		}
		else
		{
			bufs->mBuffers[0].mDataByteSize = 0;
			bufs->mBuffers[0].mData = nullptr;
		}
	}
	
	err = AudioUnitRender(m_inputUnit.sourceAudioUnit, &flags, &ts, kOutputBus, length / config.mBytesPerFrame, bufs);
	
	if (err != noErr || bufs->mBuffers[0].mDataByteSize == 0)
	{
		// ERROR() << "Render callback failed with error " << err;
		
		// Fill with silence, the error may be temporary
		UInt32 bytes = length;
		
		if (!m_shouldAllocateBuffer)
		{
			if (isOutputPlanar())
			{
				data.reset(new uint8_t[bytes*cc]);
				memset(data.get(), 0, bytes*cc);
			}
			else
			{
				data.reset(new uint8_t[bytes]);
				memset(data.get(), 0, bytes);
			}
		}
		
		if (isOutputPlanar())
		{
			for (UInt32 i = 0; i < cc; i++)
			{
				bufs->mBuffers[i].mData = data.get() + bytes*i;
				bufs->mBuffers[i].mDataByteSize = bytes;
			}
		}
		else
		{
			bufs->mBuffers[0].mData = data.get();
			bufs->mBuffers[0].mDataByteSize = bytes;
		}
	}
	
	//LOG << "Rendering...\n";
	m_lastRenderError = AudioUnitRender(this, &flags, &ts, kOutputBus, length / config.mBytesPerFrame, bufs);
	
	operator delete(bufs);
}

OSStatus AudioUnitCA::render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (inBusNumber == kOutputBus)
		return renderOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else if (inBusNumber == kInputBus)
		return renderInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return paramErr;
}

OSStatus AudioUnitCA::renderOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!isOutputPlanar())
		return renderInterleavedOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarOutput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitCA::renderInterleavedOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	const AudioStreamBasicDescription& config = m_config[kOutputBus].first;
	UInt32 framesSoFar = 0;
	
	for (UInt32 i = 0; i < ioData->mNumberBuffers; i++)
	{
		size_t bytes = std::min<size_t>((inNumberFrames - framesSoFar) * config.mBytesPerFrame, ioData->mBuffers[i].mDataByteSize);
		
		if (!bytes)
			break;
		
		//LOG << "AudioUnitPA::renderInterleavedOutput(): data=" << ioData->mBuffers[i].mData << ", bytes=" << bytes << std::endl;
		//pa_stream_write(m_stream, ((char*) ioData->mBuffers[i].mData) + framesSoFar * config.mBytesPerFrame, bytes,
		//		nullptr, 0, PA_SEEK_RELATIVE);
		
		framesSoFar += ioData->mBuffers[i].mDataByteSize / config.mBytesPerFrame;
	}
	
	return noErr;
}

OSStatus AudioUnitCA::renderPlanarOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	// PulseAudio doesn't support planar audio, we have to perform conversion
	return unimpErr;
}

OSStatus AudioUnitCA::renderInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	if (!m_outputCallback.inputProc)
		return noErr; // We don't push, we should be polled
	
	if (!isInputPlanar())
		return renderInterleavedInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
	else
		return renderPlanarInput(ioActionFlags, inTimeStamp, inNumberFrames, ioData);
}

OSStatus AudioUnitCA::renderInterleavedInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	return unimpErr;
}

OSStatus AudioUnitCA::renderPlanarInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	return unimpErr;
}


OSStatus AudioUnitCA::reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	return unimpErr;
}

OSStatus AudioUnitCA::deinit()
{
	return noErr;
}

OSStatus AudioUnitCA::start()
{
	
	return noErr;
}

OSStatus AudioUnitCA::stop()
{
	
	m_started = false;
	return noErr;
}
