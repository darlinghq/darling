#include "path.h"
#include <sys/types.h>
#include <dirent.h>
#include <cstring>
#ifdef TEST_PATH
#	include <iostream>
#endif

void translatePathCI(char* path)
{
	char* p = path;
	char* const end = path + strlen(path);
	char buffer[PATH_MAX];
	
	if (*p == '/')
		p++;
	
	while (p < end)
	{
		// list the contents of [path,p]
		DIR* dir;
		struct dirent* ent;
		char* nextp = strchr(p, '/');
		// if we're in the middle of a path, prefer dirs or symlinks to dirs
		bool preferDirOrSymlink = nextp != 0;
		char good[PATH_MAX] = "";
		
		memcpy(buffer, path, p-path);
		buffer[p-path] = 0;
		
		dir = ::opendir(buffer);
		if (!dir)
			return;
		
		if (!nextp)
			nextp = p + strlen(p);
		
		memcpy(buffer, p, nextp-p);
		buffer[nextp-p] = 0;
		
		while ((ent = ::readdir(dir)) != 0)
		{
			if (strcasecmp(ent->d_name, buffer))
				continue;
			strcpy(good, ent->d_name);
			if ((ent->d_type == DT_DIR || ent->d_type == DT_LNK) && preferDirOrSymlink)
				break;
		}
		
		::closedir(dir);
		
		if (!good[0]) // not found
			break;
		
		memcpy(p, good, nextp-p);
		p = nextp + 1; // skip the slash
	}
}

char* translatePathCI(const char* path)
{
	static __thread char buf[DARWIN_MAXPATHLEN];
	strcpy(buf, path);
	translatePathCI(buf);
	return buf;
}

#ifdef TEST_PATH
int main(int argc, char** argv)
{
	if (argc != 2)
		return 1;
	
	char buf[PATH_MAX];
	strcpy(buf, argv[1]);
	translatePathCI(buf);
	
	std::cout << buf << std::endl;
}
#endif
