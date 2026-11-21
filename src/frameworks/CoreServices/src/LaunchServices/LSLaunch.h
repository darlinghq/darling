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

#ifndef LS_LAUNCH_H_
#define LS_LAUNCH_H_

#include <CoreFoundation/CoreFoundation.h>
#include <LaunchServices/LaunchServices.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LSOpenFromURLSpec with what LSApplicationParameters adds for a process that is started: argv
 * (CFStrings passed to main() after the executable's path) and environment (CFString keys and
 * values set in the new process). Either can be NULL.
 */
__attribute__((visibility("hidden")))
OSStatus _LSOpenFromURLSpecWithParameters(const LSLaunchURLSpec *inLaunchSpec, CFArrayRef argv,
	CFDictionaryRef environment, CFURLRef *outLaunchedURL);

#ifdef __cplusplus
}
#endif

#endif
