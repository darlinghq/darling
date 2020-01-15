#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
#if DEBUG
	printf("Invoked with:");
	for (int i = 0; i < argc; i++)
	{
		printf(" %s", argv[i]);
	}
	printf("\n");
#endif
	const int prefixLength = strlen(argv[0])-strlen("xcrun"); 
	char *toolPath = calloc(prefixLength+strlen(argv[1]), sizeof(char));
	strncpy(toolPath, argv[0], prefixLength);
	strcpy(toolPath+prefixLength, argv[1]);
	
#if DEBUG
	printf("Executing %s\n", toolPath);
#endif

	argv++;
	argv[0] = toolPath;

	execvp(argv[0], argv);
	
	printf("Failed to execute program %s\n", argv[0]);

	return 1;
}
