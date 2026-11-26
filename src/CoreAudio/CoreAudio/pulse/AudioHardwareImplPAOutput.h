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

#ifndef AUDIOHARDWAREIMPLPAOUTPUT_H
#define AUDIOHARDWAREIMPLPAOUTPUT_H
#include "AudioHardwareImplPADevice.h"

// The output device that stands for whatever the server's default sink is.
class AudioHardwareImplPAOutput : public AudioHardwareImplPADevice
{
public:
	AudioHardwareImplPAOutput(AudioObjectID myId, const char* paRole = nullptr);

	// The stream connects to the server's default.
	const char* paDeviceName() const override { return nullptr; }
};

#endif
