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
