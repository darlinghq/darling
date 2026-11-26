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

#ifndef _OSSERVICES_KEYCHAINCORE_H
#define _OSSERVICES_KEYCHAINCORE_H

// The Keychain Manager part of OSServices. Its functions are long deprecated
// and not provided; what remains is that, as on macOS, including CoreServices
// gives the Security framework's API too.
// Darling's own build does not get them through here, so that its components
// keep naming the Security dependency they have themselves.
#ifndef DARLING
#include <Security/Security.h>
#endif

#endif
