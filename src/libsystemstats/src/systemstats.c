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

/* These report history recorded by systemstatsd. Darling runs no systemstatsd, so there is no
   history and both report none. The argument lists are not known; they are ignored. */

void *systemstats_get_top_coalitions()
{
	return NULL;
}

void *systemstats_get_battery_charge_graph()
{
	return NULL;
}
