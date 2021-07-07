/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#ifndef _ServerInformation_H_
#define _ServerInformation_H_

#if __OBJC__
#import <ServerInformation/ServerInformationComputerModelInfo.h>
#endif

void* SICopyLocalizedServerProductName(void);
void* SICopyMinimumServerVersion(void);
void* SICopyServerBuildVersion(void);
void* SICopyServerVersion(void);
void* SIGetServerInstallPathPrefix(void);
void* SIGetServerPerformanceModeEnabled(void);
void* SIIsOSXServerVolume(void);
void* SIIsOSXServerVolumeConfigured(void);
void* SIIsServerHardware(void);
void* SIIsXsanCapable(void);
void* SISetServerPerformanceModeEnabled(void);
void* _SIIsOSXVolumeVersionEqualToOrGreater(void);

#endif
