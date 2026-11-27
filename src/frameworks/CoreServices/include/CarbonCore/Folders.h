/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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


#ifndef _CS_FOLDERS_H
#define _CS_FOLDERS_H
#include <CoreServices/MacTypes.h>
#include <CarbonCore/Files.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The folder types of FindFolder and FSFindFolder (declared here, not in Files.h). */
enum
{
	kSystemFolderType = 'macs',
	kDesktopFolderType = 'desk',
	kSystemDesktopFolderType = 'sdsk',
	kTrashFolderType = 'trsh',
	kSystemTrashFolderType = 'strs',
	kWhereToEmptyTrashFolderType = 'empt',
	kPrintMonitorDocsFolderType = 'prnt',
	kStartupFolderType = 'strt',
	kShutdownFolderType = 'shdf',
	kAppleMenuFolderType = 'amnu',
	kControlPanelFolderType = 'ctrl',
	kSystemControlPanelFolderType = 'sctl',
	kExtensionFolderType = 'extn',
	kFontsFolderType = 'font',
	kPreferencesFolderType = 'pref',
	kSystemPreferencesFolderType = 'sprf',
	kTemporaryFolderType = 'temp'
};

/* The volume (domain) and creation arguments of the two functions. */
enum
{
	kOnSystemDisk = -32768,
	kOnAppropriateDisk = -32767,
	kSystemDomain = -32766,
	kLocalDomain = -32765,
	kNetworkDomain = -32764,
	kUserDomain = -32763,
	kClassicDomain = -32762,
	kLastDomainConstant = -32762
};

enum
{
	kCreateFolder = true,
	kDontCreateFolder = false
};

OSStatus FSFindFolder(long vRefNum, OSType folderType, Boolean createFolder, struct FSRef* location);

#ifndef __LP64__
OSErr FindFolder(short volume, OSType folderType, Boolean createFolder, short* foundVolume, long* foundDirectory);
#endif

#ifdef __cplusplus
}
#endif

#endif
