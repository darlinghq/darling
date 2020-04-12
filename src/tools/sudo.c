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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

unsigned int firstarg = 1, uid = 0, gid = 0;

static int list_mode = 0;

int argparse(int, char**);
int usage(int, int);

int main(int argc, char **argv)
{
	char *path = getenv("PATH"), *path_orig, *fullpath = NULL;
	int i, len;
	if (argparse(argc, argv))
			return 1;

	setuid(uid);
	setgid(gid);

	if (firstarg == argc && list_mode)
	{
		printf("List mode without command not supported yet.\n");
		return EXIT_SUCCESS;
	}
	else if (list_mode)
	{
		/* Print fill path to command, then args if present */
		if (!path)
			printf("can't search for command if no path\n");
		path = strdup(path);
		path_orig = path;
		len = strlen(path_orig);
		for (i = 0; i < len; i++)
		{
			if (path_orig[i] == ':' || path_orig[i] == '\0')
			{
				path_orig[i] = '\0';
				fullpath = malloc(strlen(argv[firstarg])+strlen(path)+2);
				strcpy(fullpath, path);
				fullpath[strlen(path)] = '/';
				strcpy(fullpath+strlen(path)+1, argv[firstarg]);
				if (access(fullpath, X_OK) == 0)
					break;
				free(fullpath);
				fullpath = NULL;
				path = path_orig+i+1;
			}
		}
		free(path_orig);
		if (!fullpath)
		{
			fprintf(stderr, "%s: %s: command not found\n", argv[0], argv[firstarg]);
			return EXIT_FAILURE;
		}
		printf("%s", fullpath);
		free(fullpath);
		for (i = firstarg+1; i < argc; i++)
		{
			printf(" %s", argv[i]);
		}
		putchar('\n');
		return EXIT_SUCCESS;
	}
	else
	{
		if (firstarg < argc)
		{
			execvp(argv[firstarg], &argv[firstarg]);
			perror("Running sudo failed");
		}
		else
			return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int argparse(int argc, char **argv)
{
	for (firstarg = 1; firstarg < argc; firstarg++)
	{
		if(!strcmp(argv[firstarg], "-g"))
		{
			sscanf(argv[++firstarg], "%u", &gid);
		}
		else if(!strcmp(argv[firstarg], "-u"))
		{
			sscanf(argv[++firstarg], "%u", &uid);
		}
		else if (!strcmp(argv[firstarg], "-A"))
		{
		}
		else if (!strcmp(argv[firstarg], "-n"))
		{
		}
		else if (!strcmp(argv[firstarg], "-v"))
		{
		}
		else if(!strcmp(argv[firstarg], "--"))
		{
			break;
		}
		else if(!strcmp(argv[firstarg], "-k"))
		{
		}
		else if(!strcmp(argv[firstarg], "--help"))
		{
			return usage(argc, 1);
		}
		else if (!strcmp(argv[firstarg], "-l"))
		{
			list_mode = 1;
		}
		else
		{
			break;
		}
	}
	return 0;
}

int usage(int argc, int arg_help)
{
	if (argc <= firstarg || arg_help == 1)
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
