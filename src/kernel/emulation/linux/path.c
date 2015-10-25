#include "path.h"

extern char *strcpy(char *, const char *);
extern char *strchr(const char *s, int c);
extern unsigned long strlen(const char *s);

bool path_icase(const char* path_in, char* path_out)
{
	// Go part by part, open() every one of them,
	// call getdirentries and find better match
	int pos = 0;
	const int len = strlen(path_in);
	bool made_changes = false;
	
	strcpy(path_out, path_in);
	while (pos < len)
	{
		// TODO
	}

	return made_changes;
}

