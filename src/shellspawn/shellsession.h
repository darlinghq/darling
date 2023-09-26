/*
 * This file is part of Darling.
 * 
 * Copyright (C) 2023 Darling Developers
 * 
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SHELLSESSION_H_
#define _SHELLSESSION_H_

#ifdef TESTING
#define SHELLSESSION_SOCKPATH "/tmp/shellsession.sock"
#define SHELLSESSION_STARTED_PATH "/tmp/shellsession.running"
#else
#define SHELLSESSION_SOCKPATH "/var/run/shellsession.sock"
#define SHELLSESSION_STARTED_PATH "/var/run/shellsession.running"
#endif

typedef struct shellsession_cmd {
	unsigned int uid;
	unsigned int gid;
} shellsession_cmd_t;

#endif // _SHELLSESSION_H_

