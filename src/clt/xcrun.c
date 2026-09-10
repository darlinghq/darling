#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

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

	if (argc < 2)
	{
		fprintf(stderr, "xcrun: error: no tool specified\n");
		return 1;
	}

	const char* toolName = argv[1];

	int prefixLength = 0;
	char* lastSlash = strrchr(argv[0], '/');
	if (lastSlash != NULL)
	{
		prefixLength = (int)(lastSlash - argv[0] + 1);
	}

	char *toolPath = calloc(prefixLength + strlen(toolName) + 1, sizeof(char));
	if (!toolPath)
	{
		fprintf(stderr, "xcrun: error: out of memory\n");
		return 1;
	}
	if (prefixLength > 0)
	{
		strncpy(toolPath, argv[0], prefixLength);
	}
	strcpy(toolPath + prefixLength, toolName);

#if DEBUG
	printf("Executing %s\n", toolPath);
#endif

	argv++;
	argv[0] = toolPath;

	execvp(argv[0], argv);

	// If toolPath execution failed, fallback to searching PATH for the tool.
	// Guard against infinite recursion when invoked from shims.
	if (getenv("__XCRUN_FALLBACK") != NULL)
	{
		fprintf(stderr, "xcrun: error: tool '%s' not found (recursion prevented)\n", toolName);
		return 1;
	}
	setenv("__XCRUN_FALLBACK", "1", 1);

	// If toolName contains a slash, try direct execution
	if (strchr(toolName, '/') != NULL && access(toolName, X_OK) == 0)
	{
		argv[0] = (char*) toolName;
		execv(toolName, argv);
	}

	const char* pathEnv = getenv("PATH");
	if (pathEnv != NULL)
	{
		char* pathCopy = strdup(pathEnv);
		if (pathCopy != NULL)
		{
			char* dir = strtok(pathCopy, ":");
			while (dir != NULL)
			{
				// Skip directories that contain xcrun-shims or recursion targets
				if (strcmp(dir, "/usr/bin") != 0 &&
				    strcmp(dir, "/bin") != 0 &&
				    strcmp(dir, "/usr/sbin") != 0 &&
				    strcmp(dir, "/sbin") != 0 &&
				    strncmp(dir, "/Library/Developer", 18) != 0)
				{
					char candidate[PATH_MAX];
					int written = snprintf(candidate, sizeof(candidate), "%s/%s", dir, toolName);
					if (written > 0 && (size_t)written < sizeof(candidate))
					{
						if (access(candidate, X_OK) == 0)
						{
							argv[0] = candidate;
							execv(candidate, argv);
						}
					}
				}
				dir = strtok(NULL, ":");
			}
			free(pathCopy);
		}
	}

	// Also check /usr/libexec/DeveloperTools if available
	char devToolsPath[PATH_MAX];
	snprintf(devToolsPath, sizeof(devToolsPath), "/usr/libexec/DeveloperTools/%s", toolName);
	if (access(devToolsPath, X_OK) == 0)
	{
		argv[0] = devToolsPath;
		execv(devToolsPath, argv);
	}

	fprintf(stderr, "xcrun: error: cannot find tool '%s'\n", toolName);
	return 1;
}

