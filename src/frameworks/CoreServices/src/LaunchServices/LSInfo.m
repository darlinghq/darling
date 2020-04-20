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


#include <LaunchServices/LaunchServices.h>
#include <CoreServices/MacErrors.h>
#include <stdio.h>

OSStatus
LSFindApplicationForInfo(
  OSType inCreator,
  CFStringRef inBundleID,
  CFStringRef inName,
  FSRef * outAppRef,
  CFURLRef * outAppURL)
{
	puts("LSFindApplicationForInfo STUB");
	return unimpErr;
}

OSStatus
LSGetApplicationForInfo(
  OSType inType,
  OSType inCreator,
  CFStringRef inExtension,
  LSRolesMask inRoleMask,
  FSRef * outAppRef,
  CFURLRef * outAppURL)
{
	puts("LSGetApplicationForInfo STUB");
	return unimpErr;
}
