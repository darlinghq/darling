/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#ifndef GESTALT_H
#define GESTALT_H

#define gestaltSystemVersion 'sysv'
#define gestaltSystemVersionMajor 'sys1'
#define gestaltSystemVersionMinor 'sys2'
#define gestaltSystemVersionPatch 'sys3'

#define gestaltSysArchitecture 'sysa'
#define gestaltPowerPC 2
#define gestaltIntel 10

#ifdef __cplusplus
extern "C" {
#endif

OSStatus Gestalt(uint32_t type, int* value);

#ifdef __cplusplus
}
#endif

#endif

