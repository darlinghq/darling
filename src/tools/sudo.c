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
#include <errno.h>
#include <limits.h>
#include <paths.h>

unsigned int firstarg = 1, uid = 0, gid = 0;

static int list_mode = 0;

int argparse(int, char**, char**);
int usage(int, int);

static int parse_id(const char *value, unsigned int *id)
{
	char *end;
	unsigned long number;

	if (*value < '0' || *value > '9')
		return 1;
	errno = 0;
	number = strtoul(value, &end, 10);
	if (errno == ERANGE || *end != '\0' || number > UINT_MAX)
		return 1;
	*id = (unsigned int)number;
	return 0;
}

static int apply_environment(char **assignments)
{
	for (; *assignments; assignments++)
	{
		char *equals = strchr(*assignments, '=');
		int result;

		*equals = '\0';
		result = setenv(*assignments, equals + 1, 1);
		*equals = '=';
		if (result != 0)
		{
			perror("Setting sudo environment failed");
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	const char *inherited_path = getenv("PATH");
	char *path_orig = strdup(inherited_path ? inherited_path : _PATH_DEFPATH);
	char *path = path_orig, *fullpath = NULL;
	char **assignments = calloc(argc, sizeof(*assignments));
	int status = EXIT_FAILURE;
	int i, len;

	if (!path_orig || !assignments)
	{
		perror("Preparing sudo environment failed");
		goto out;
	}
	if (argparse(argc, argv, assignments))
		goto out;
	if (!list_mode && apply_environment(assignments))
		goto out;

	setuid(uid);
	setgid(gid);

	if (firstarg == argc && list_mode)
	{
		printf("List mode without command not supported yet.\n");
		status = EXIT_SUCCESS;
	}
	else if (list_mode)
	{
		/* Print fill path to command, then args if present */
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
		if (!fullpath)
		{
			fprintf(stderr, "%s: %s: command not found\n", argv[0], argv[firstarg]);
			goto out;
		}
		printf("%s", fullpath);
		for (i = firstarg+1; i < argc; i++)
		{
			printf(" %s", argv[i]);
		}
		putchar('\n');
		status = EXIT_SUCCESS;
	}
	else
	{
		if (firstarg < argc)
		{
			/* Command assignments change the child environment, not lookup. */
			execvP(argv[firstarg], path_orig, &argv[firstarg]);
			perror("Running sudo failed");
		}
		else
			status = EXIT_SUCCESS;
	}
out:
	free(assignments);
	free(path_orig);
	free(fullpath);
	return status;
}

int argparse(int argc, char **argv, char **assignments)
{
	unsigned int env_vars = 0;

	for (firstarg = 1; firstarg < argc; firstarg++)
	{
		if (!strcmp(argv[firstarg], "-g") || !strcmp(argv[firstarg], "-u"))
		{
			const char *option = argv[firstarg];
			unsigned int *id = option[1] == 'u' ? &uid : &gid;

			if (++firstarg == (unsigned int)argc || parse_id(argv[firstarg], id))
			{
				fprintf(stderr, "%s requires a numeric ID\n", option);
				return 1;
			}
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
			firstarg++;
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
			char *equals = strchr(argv[firstarg], '=');
			char *slash = strchr(argv[firstarg], '/');

			/* An '=' in a command path is part of the command. */
			if (argv[firstarg][0] == '-' || !equals || (slash && slash < equals))
				break;

			if (equals == argv[firstarg])
			{
				fprintf(stderr, "Environment assignment requires a name\n");
				return 1;
			}
			assignments[env_vars++] = argv[firstarg];
		}
	}
	if (env_vars && firstarg == (unsigned int)argc)
		return usage(argc, 0);
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
				"  sudo [-g GID] [-u UID] [-k] [NAME=value ...] [--] COMMAND [ARG ...]\n");
		return 1;
	}
	return 0;
}
