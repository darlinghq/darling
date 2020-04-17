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
#include "AVFormatFileObject.h"
#include <sstream>
#include "AudioFileFormatMP3.h"

extern "C" {
#include <libavformat/avformat.h>
}

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AudioFileComponentFactory, AVFormatFileObject);
#pragma GCC visibility pop

AVFormatFileObject::AVFormatFileObject(AudioComponentInstance inInstance)
: AudioFileObjectComponentBase(inInstance)
{

}

// Analyze mAudioFileObject and return an AudioFileFormat based on that
AudioFileFormat* AVFormatFileObject::GetAudioFormat() const
{
	uint8_t buffer[1024 + AVPROBE_PADDING_SIZE];
	AVProbeData probeData;

	probeData.filename = "";
	probeData.buf = buffer;
	probeData.buf_size = 1024;

	if (mAudioFileObject->ReadBytes(false, 0, (UInt32*)&probeData.buf_size, probeData.buf) != noErr)
		return nullptr;

	AVInputFormat* fmt = av_probe_input_format(&probeData, true);
	if (!fmt)
		return nullptr;

	std::istringstream ss(fmt->name);
	std::string formatName;

	while (std::getline(ss, formatName, ','))
	{
		if (formatName == "aac") // Raw AAC
		{

		}
		else if (formatName == "ac3")
		{

		}
		else if (formatName == "aiff")
		{

		}
		else if (formatName == "caff")
		{

		}
		else if (formatName == "mp3")
		{
			return AudioFileFormatMP3::instance();
		}
		else if (formatName == "mov" || formatName == "mp4")
		{

		}
		else if (formatName == "au")
		{

		}
		else if (formatName == "wav")
		{

		}
	}

	return nullptr;
}
