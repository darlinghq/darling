/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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

#ifndef _SHELLSPAWN_H
#define _SHELLSPAWN_H

#ifdef TESTING
#define SHELLSPAWN_SOCKPATH "/tmp/shellspawn.sock"
#else
#define SHELLSPAWN_SOCKPATH "/var/run/shellspawn.sock"
#endif

typedef unsigned short shellspawn_cmd_type_t;

enum {
	SHELLSPAWN_ADDARG = 1, // add shell argument
	SHELLSPAWN_SETENV, // add env variable string
	SHELLSPAWN_CHDIR,
	SHELLSPAWN_GO, // execute the shell now, must also contain file descriptors
	SHELLSPAWN_SIGNAL, // pass a signal from client
	SHELLSPAWN_SETUIDGID, // set virtual uid and gid
};

struct __attribute__((packed)) shellspawn_cmd
{
	shellspawn_cmd_type_t cmd;
	unsigned short data_length;
	char data[];
};

#endif

