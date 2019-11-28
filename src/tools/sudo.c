/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <unistd.h>
#include <stdio.h>
#include <string.h>

unsigned int firstarg = 1, uid = 0, gid = 0;

int argparse(int, char**);
int usage(int, int);

int main(int argc, char **argv)
{
	if ( ( argparse(argc, argv) || usage(argc, 0) ) == 1 )
		return 1;

	setuid(uid);
	setgid(gid);

	execvp(argv[firstarg], &argv[firstarg]);
	perror("Error");
	return 1;
}

int argparse(int argc, char **argv)
{
	while( (argv[firstarg] != NULL) && (!strncmp(argv[firstarg], "-", 1)) )
	{
		if(!strcmp(argv[firstarg], "-g"))
		{
			sscanf(argv[++firstarg], "%u", &gid);
			firstarg++;
		}
		else if(!strcmp(argv[firstarg], "-u"))
		{
			sscanf(argv[++firstarg], "%u", &uid);
			firstarg++;
		}
		else if (!strcmp(argv[firstarg], "-A"))
		{
			firstarg++;
		}
		else if (!strcmp(argv[firstarg], "-n"))
		{
			firstarg++;
		}
		else if (!strcmp(argv[firstarg], "-v"))
		{
			firstarg++;
		}
		else if(!strcmp(argv[firstarg], "--"))
		{
			firstarg++;
			break;
		}
		else if(!strcmp(argv[firstarg], "-k"))
			firstarg++;
		else if(!strcmp(argv[firstarg], "--help"))
			return usage(argc, 1);
		else
		{
			fprintf(stderr, "Unknown option: %s\n", argv[firstarg]);
			return usage(argc, 1);
		}
	}
	return 0;
}

int usage(int argc, int arg_help)
{
	if (argc <= (int)firstarg || arg_help == 1)
	{
		fprintf(stderr, "This is a fake 'sudo' implementation, intended for use in Darling.\n"
				"Processes will think they are run as UID/GID 0, but they are still run as your original UID/GID.\n"
				"One purpose of this program is to convince some tools that they can write into '/'.\n"
				"Another is to enable you to talk to certain system daemons.\n"
				"\nUsage:\n"
				"  sudo [-g GID] [-u UID] [-k] [--] COMMAND\n");
		return 1;
	}
	return 0;
}
