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
#include "AudioFileMP3.h"
#include "AVFormatFileObject.h"
#include "AudioFileFormatGeneric.h"

class AudioFileFormatMP3 : public AudioFileFormatGeneric
{
public:
	AudioFileFormatMP3() : AudioFileFormatGeneric('.mp3', "mp3") {}
protected:
	const Description& description() const override
	{
		static const Description d = {
			.name = "MPEG Layer 3",
			.extensions = { "mp3" },
			.utis = { "public.mp3", "public.audio", "public.data" },
			.mimeTypes = { "audio/mpeg" },
			.formats = { '.mp3' },
		};
		return d;
	}
};

class MP3Component : public AVFormatFileObject<AudioFileFormatMP3, AudioFileMP3, '.mp3'>
{
public:
	MP3Component(AudioComponentInstance inInstance) : AVFormatFileObject(inInstance) {}
};

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AudioFileComponentFactory, MP3Component);
#pragma GCC visibility pop

AudioFileMP3::AudioFileMP3()
: AudioFileObject('.mp3')
{

}

Boolean AudioFileMP3::IsDataFormatSupported(const AudioStreamBasicDescription	*inFormat)
{
	return true;
}
