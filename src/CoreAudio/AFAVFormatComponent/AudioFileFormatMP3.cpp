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

#include "AudioFileFormatMP3.h"

AudioFileFormatMP3::AudioFileFormatMP3()
: AudioFileFormatGeneric('.mp3', "mp3")
{
}

AudioFileFormatMP3* AudioFileFormatMP3::instance()
{
	static AudioFileFormatMP3 inst;
	return &inst;
}
 
const AudioFileFormatMP3::Description& AudioFileFormatMP3::description() const
{
	static const Description d = {
		.name = "MPEG Layer 3",
		.extensions = { "mp3" },
		.utis = { "public.mp3", "public.audio", "public.data" },
		.mimeTypes = { "audio/mpeg" },
	};
	return d;
}
