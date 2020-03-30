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

#include "AudioFileFormatManager.h"

AudioFileFormatManager::AudioFileFormatManager()
{

}

void AudioFileFormatManager::buildDatabase()
{
	AudioComponentDescription acd = { 0 };
	acd.componentType = 'afil';

	AudioComponent component = nullptr;

	while ((component = ::AudioComponentFindNext(component, &acd)) != nullptr)
	{
		AudioComponentInstance inst;
		OSStatus status = ::AudioComponentInstanceNew(component, &inst);

		if (status != noErr)
			continue;

		analyzeAudioFileComponent(inst);
		::AudioComponentInstanceDispose(inst);
	}
}

void AudioFileFormatManager::analyzeAudioFileComponent(AudioFileComponent component)
{
	/*
	 kAudioFileComponent_FileTypeName = 'ftnm',
 kAudioFileComponent_UTIsForType = 'futi',
 kAudioFileComponent_MIMETypesForType = 'fmim',
 kAudioFileComponent_ExtensionsForType = 'fext',
 kAudioFileComponent_AvailableFormatIDs = 'fmid',
 */
}

AudioFileFormatManager* AudioFileFormatManager::instance()
{
	static AudioFileFormatManager inst;
	return &inst;
}
