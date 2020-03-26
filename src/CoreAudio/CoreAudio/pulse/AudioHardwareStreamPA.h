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

#ifndef AUDIOHARDWARESTREAMPA_H
#define AUDIOHARDWARESTREAMPA_H
#include "../AudioHardwareStream.h"
#include <pulse/pulseaudio.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/AudioHardware.h>
#include "AudioHardwareImplPA.h"

class AudioHardwareStreamPA : public AudioHardwareStream
{
public:
	AudioHardwareStreamPA(AudioHardwareImplPA* hw, AudioDeviceIOProc callback, void* clientData);
	~AudioHardwareStreamPA();

	void stop(void(^cbDone)()) override;
protected:
	virtual void start() = 0;
	void transformSignedUnsigned(AudioBufferList* abl) const;
protected:
	AudioDeviceIOProc m_callback;
	void* m_clientData;
	pa_stream* m_stream;
	void(^m_cbDone)();
	bool m_convertSignedUnsigned = false;
};

#endif /* AUDIOHARDWARESTREAMPA_H */

