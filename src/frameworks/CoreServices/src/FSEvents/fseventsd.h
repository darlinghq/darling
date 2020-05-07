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

#ifndef FSEVENTSD_H_
#define FSEVENTSD_H_

#define FSEVENTSD_SOCKET_PATH "/var/run/fseventsd.sock"

typedef struct fseventsd_monitor {
	int id;
	char path[4096];
	int flags;
} fseventsd_monitor_t;

typedef struct fseventsd_event {
	int id;
	char path[4096];
	int flags;
} fseventsd_event_t;

#endif

