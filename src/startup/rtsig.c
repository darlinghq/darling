#include <signal.h>
#include <stdio.h>

// Determine RT signal ranges without polluting the build environment with Linux system headers
int main(int argc, char** argv)
{
	FILE* output = stdout;

	if (argc > 1)
	{
		output = fopen(argv[1], "w");
		if (!output)
		{
			perror("fopen");
			return 1;
		}
	}

	fprintf(output, "#define LINUX_SIGRTMIN %d\n", SIGRTMIN);
	fprintf(output, "#define LINUX_SIGRTMAX %d\n", SIGRTMAX);

	fclose(output);
	return 0;
}

