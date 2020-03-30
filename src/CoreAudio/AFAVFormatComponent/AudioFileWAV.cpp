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
#include "AudioFileWAV.h"
#include "AVFormatFileObject.h"
#include "AudioFileFormatGeneric.h"

class AudioFileFormatWAV : public AudioFileFormatGeneric
{
public:
	AudioFileFormatWAV() : AudioFileFormatGeneric('.mp3', "wav") {}
protected:
	const Description& description() const override
	{
		static const Description d = {
			.name = "WAVE",
			.extensions = { "wav" },
			.utis = { "com.microsoft.waveform-audio", "public.audio", "public.data" },
			.mimeTypes = { "audio/wav" },
			.formats = {
				Format(kAudioFormatLinearPCM, 0, 8),
				Format(kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger, 16),
				Format(kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger, 24),
				Format(kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger, 32),
				Format(kAudioFormatLinearPCM, kAudioFormatFlagIsFloat, 32),
				Format(kAudioFormatLinearPCM, kAudioFormatFlagIsFloat, 64),
				'ulaw', 'alaw'
			},
		};
		return d;
	}
};

class WAVComponent : public AVFormatFileObject<AudioFileFormatWAV, AudioFileWAV, '.wav'>
{
public:
	WAVComponent(AudioComponentInstance inInstance) : AVFormatFileObject(inInstance) {}
};

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AudioFileComponentFactory, WAVComponent);
#pragma GCC visibility pop

AudioFileWAV::AudioFileWAV()
: AudioFileObject('.wav')
{

}

Boolean AudioFileWAV::IsDataFormatSupported(const AudioStreamBasicDescription	*inFormat)
{
	return true;
}
