/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

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

#ifndef LS_HOST_OPEN_H_
#define LS_HOST_OPEN_H_

#include <CoreFoundation/CoreFoundation.h>
#include <LaunchServices/LaunchServices.h>

#ifdef __cplusplus
extern "C" {
#endif

/* URLs with these schemes (http, https, mailto) are opened by the host desktop. */
Boolean _LSIsHostOpenURL(CFURLRef url);

/*
 * Hands the URL to the host desktop. No application in the prefix handles
 * these URLs, so LSGetApplicationForURL and LSCopyDefaultApplicationURLForURL
 * report none for them, while opening them succeeds.
 */
OSStatus _LSOpenURLOnHost(CFURLRef url);

/*
 * Shows a file or folder in the host's file manager through the desktop portal
 * (see LSHostOpenBus.h). With reveal the folder containing the item is shown
 * with the item selected, otherwise the item is opened (a folder is shown as
 * a folder). Returns kLSApplicationNotFoundErr when the host has no way to do
 * it, as there is no file manager in the prefix.
 */
OSStatus _LSOpenFileOnHost(CFURLRef url, Boolean reveal);

#ifdef __cplusplus
}
#endif

#endif
