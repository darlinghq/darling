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

#include <AudioToolbox/MusicDevice.h>
#include <CarbonCore/MacErrors.h>
#include <iostream>

OSStatus
MusicDeviceMIDIEvent( MusicDeviceComponent inUnit,
      UInt32 inStatus,
      UInt32 inData1,
      UInt32 inData2,
      UInt32 inOffsetSampleFrame)
{
	std::cerr << "NOT IMPLEMENTED: " << __FUNCTION__ << std::endl;
	return unimpErr;
}

OSStatus
MusicDeviceSysEx( MusicDeviceComponent inUnit,
      const UInt8 * inData,
      UInt32 inLength)
{
	std::cerr << "NOT IMPLEMENTED: " << __FUNCTION__ << std::endl;
	return unimpErr;
}

OSStatus
MusicDeviceStartNote( MusicDeviceComponent inUnit,
      MusicDeviceInstrumentID inInstrument,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID * outNoteInstanceID,
      UInt32 inOffsetSampleFrame,
      const MusicDeviceNoteParams * inParams)
{
	std::cerr << "NOT IMPLEMENTED: " << __FUNCTION__ << std::endl;
	return unimpErr;
}

OSStatus
MusicDeviceStopNote( MusicDeviceComponent inUnit,
      MusicDeviceGroupID inGroupID,
      NoteInstanceID inNoteInstanceID,
      UInt32 inOffsetSampleFrame)
{
	std::cerr << "NOT IMPLEMENTED: " << __FUNCTION__ << std::endl;
	return unimpErr;
}
