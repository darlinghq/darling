#include "config.h"
#include "exec.h"
#include "common/path.h"
#include "libc/darwin_errno_codes.h"
#include "libc/errno.h"
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <stdarg.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <mach/vm_types.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>

extern char g_loader_path[PATH_MAX];

static const char* findInPath(const char* file)
{
	static __thread char buffer[DARWIN_MAXPATHLEN];
	
	if (strchr(file, '/') != 0)
	{
		// No PATH resolution, only fix the case
		strcpy(buffer, file);
		translatePathCI(buffer);
		return buffer;
	}
	
	const char* path = getenv("PATH");
	if (!path)
	{
		// Get the default path.
		size_t len = confstr(_CS_PATH, 0, 0);
		char* buf = reinterpret_cast<char*>( alloca(len + 1) );
		buf[0] = ':';
		
		confstr(_CS_PATH, buf + 1, len);
		
		path = buf;
	}

	const char* p = path;
	do
	{
		const char* end = strchrnul(p, ':');
		size_t len = end-p;
		memcpy(buffer, p, len);
		if (buffer[len-1] != '/')
			buffer[len++] = '/';
		
		strcpy(buffer+len, file);
		
		translatePathCI(buffer);
		
		if (::access(buffer, X_OK) == 0)
			return buffer;
	}
	while (*p++);

	return 0;
}

static char* const* prependLoaderPath(char *const argv[], const char* fullMachoPath)
{
	int count = 0;
	const char** rv;
	
	while (argv[count++]);
	
	rv = new const char*[count+1];
	rv[0] = g_loader_path;
	memcpy(rv+1, argv, count * sizeof(char*));
	
	rv[1] = fullMachoPath;
	
	return const_cast<char* const*>(rv);
}

static bool isMachO(const char* path)
{
	bool is_macho = false;
	int fd = ::open(path, O_RDONLY);
	
	if (fd != -1)
	{
		uint32_t magic;
		if (::read(fd, &magic, 4) == 4)
		{
			is_macho = ( magic == MH_MAGIC_64 || magic == MH_MAGIC || magic == FAT_CIGAM || FAT_MAGIC );
		}
		::close(fd);
	}
	return is_macho;
}

int __darwin_execl(const char *path, const char *arg, ...)
{
	va_list vl;
	va_start(vl, arg);
	std::vector<char*> argv;
	char* an_arg;
	
	while ((an_arg = va_arg(vl, char*)) != 0)
		argv.push_back(an_arg);
	
	argv.push_back(0);
	va_end(vl);
	
	return __darwin_execv(path, &argv[0]);
}

int __darwin_execlp(const char *file, const char *arg, ...)
{
	va_list vl;
	va_start(vl, arg);
	std::vector<char*> argv;
	char* an_arg;
	
	while ((an_arg = va_arg(vl, char*)) != 0)
		argv.push_back(an_arg);
	
	argv.push_back(0);
	va_end(vl);
	
	return __darwin_execvp(file, &argv[0]);
}

int __darwin_execle(const char *path, const char *arg, ... /*, char * const envp[]*/)
{
	va_list vl;
	va_start(vl, arg);
	std::vector<char*> argv;
	char* an_arg;
	char** envp;

	while ((an_arg = va_arg(vl, char*)) != 0)
		argv.push_back(an_arg);
	
	argv.push_back(0);
	envp = va_arg(vl, char**);
	va_end(vl);
	
	// using an execvpe version, but given the behavior of FindInPath(), it should be OK
	return __darwin_execvpe(path, &argv[0], envp);
}

int __darwin_execv(const char *path, char *const argv[])
{
	path = translatePathCI(path);
	
	if (!isMachO(path))
	{
		int rv = execv(path, argv);
		errnoOut();
		return rv;
	}
	else
	{
		argv = prependLoaderPath(argv, path);
		int rv = execv(g_loader_path, argv);
		errnoOut();
		
		delete [] argv;
		return rv;
	}
}

int __darwin_execvp(const char *file, char *const argv[])
{
	const char* path = findInPath(file);
	if (!path)
	{
		errno = DARWIN_ENOENT;
		return -1;
	}
	
	return __darwin_execv(path, argv);
}

int __darwin_execvpe(const char *file, char *const argv[], char *const envp[])
{
	const char* path = findInPath(file);
	if (!path)
	{
		errno = DARWIN_ENOENT;
		return -1;
	}
	
	if (!isMachO(path))
	{
		int rv = execv(path, argv);
		errnoOut();
		return rv;
	}
	else
	{
		argv = prependLoaderPath(argv, path);
		int rv = execvpe(g_loader_path, argv, envp);
		errnoOut();
		
		delete [] argv;
		return rv;
	}
}

