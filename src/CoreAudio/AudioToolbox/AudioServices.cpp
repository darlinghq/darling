/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <AudioToolbox/AudioServices.h>

OSStatus AudioServicesAddSystemSoundCompletion(SystemSoundID inSystemSoundID, CFRunLoopRef inRunLoop, CFStringRef inRunLoopMode, AudioServicesSystemSoundCompletionProc inCompletionRoutine, void *inClientData)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

OSStatus AudioServicesCreateSystemSoundID(CFURLRef inFileURL, SystemSoundID *outSystemSoundID)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

OSStatus AudioServicesDisposeSystemSoundID(SystemSoundID inSystemSoundID)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);

    return 0;
}

void AudioServicesPlaySystemSound(SystemSoundID inSystemSoundID)
{
    printf("STUB %s\n", __PRETTY_FUNCTION__);
}
