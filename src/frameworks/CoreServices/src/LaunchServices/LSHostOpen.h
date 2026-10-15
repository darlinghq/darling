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

/* Hands the URL to the host desktop. */
OSStatus _LSOpenURLOnHost(CFURLRef url);

/* Reports the host opener as the default application for a host-opened URL. */
OSStatus _LSGetHostHandlerForURL(CFURLRef url, FSRef* outAppRef, CFURLRef* outAppURL);

#ifdef __cplusplus
}
#endif

#endif
