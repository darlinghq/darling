/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Based on work by lucannez64 (public repository lucannez64/darling-lol,
commit 8f62017), GPL-3.0.

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

/*
 * Which URLs may be handed to the host desktop: http, https and mailto only,
 * written in RFC 3986 characters, so nothing in them can look like an option or
 * a shell word to the host opener. No Darling or CoreFoundation dependencies.
 */

#ifndef LS_HOST_OPEN_URL_H_
#define LS_HOST_OPEN_URL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LS_HOST_OPEN_MAX_URL_LENGTH 4096

/* Linux errno value; the guest's errno numbering differs. */
#define LS_HOST_LINUX_EINVAL 22

/* 1 if the URL has a scheme that is handed to the host (http, https, mailto). */
int _LSHostOpenSchemeSupported(const char* url);

/* 0 if the URL may be handed to the host, otherwise -EINVAL. */
int _LSHostOpenValidateURL(const char* url);

#ifdef __cplusplus
}
#endif

#endif
