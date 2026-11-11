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

#include <stddef.h>
#include <RemoteServiceDiscovery/RemoteServiceDiscovery.h>

/* Darling has no remote devices, so no device of any type is found and no device object is ever
   handed out. */

void *remote_device_copy_unique_of_type()
{
	return NULL;
}

void *remote_device_copy_property()
{
	return NULL;
}
