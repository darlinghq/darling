#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	int firstarg = 1;

	if (argc <= 1)
	{
		fprintf(stderr, "This is Darling fake sudo.\n"
				"Processes will think they run as UID/GID 0, but Linux kernel will still see your original UID.\n"
				"The purpose is to convince some tools that they can write into / or enable you to talk to certain system daemons\n.");
		return 1;
	}

	setuid(0);
	setgid(0);

	if (strcmp(argv[1], "-k") == 0)
		firstarg++;

	execvp(argv[firstarg], &argv[firstarg]);
	perror("Cannot execute:");
	return 1;
}

