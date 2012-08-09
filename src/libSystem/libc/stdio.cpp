#include "stdio.h"
#include "common/path.h"
#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include "errno.h"
#include "darwin_errno_codes.h"
#include "log.h"

extern "C" __darwin_FILE* __stdinp;
extern "C" __darwin_FILE* __stdoutp;
extern "C" __darwin_FILE* __stderrp;

static __darwin_FILE* InitDarwinFILE(FILE* linux_fp)
{
	if (!linux_fp)
		return 0;

	__darwin_FILE* fp = new __darwin_FILE;
	fp->_p = NULL;
	fp->_r = -1;
	fp->_w = -1;
	fp->_flags = 0;
	fp->_file = fileno(linux_fp);
	fp->_lbfsize = 0;
	fp->linux_fp = linux_fp;
	return fp;
}

__darwin_FILE* __darwin_fopen(const char* path, const char* mode)
{
	return InitDarwinFILE(fopen(translatePathCI(path), mode));
}

__darwin_FILE* __darwin_fdopen(int fd, const char* mode)
{
	return InitDarwinFILE(fdopen(fd, mode));
}

__darwin_FILE* __darwin_freopen(const char* path, const char* mode, __darwin_FILE* fp)
{
	return InitDarwinFILE(freopen(translatePathCI(path), mode, fp->linux_fp));
}

int __darwin_fclose(__darwin_FILE* fp)
{
	int r = fclose(fp->linux_fp);
	delete fp;
	return r;
}

size_t __darwin_fread(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp)
{
	return fread(ptr, size, nmemb, fp->linux_fp);
}

size_t __darwin_fwrite(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp)
{
	return fwrite(ptr, size, nmemb, fp->linux_fp);
}

int __darwin_fseek(__darwin_FILE* fp, long offset, int whence)
{
	return fseek(fp->linux_fp, offset, whence);
}

long __darwin_ftell(__darwin_FILE* fp)
{
	return ftell(fp->linux_fp);
}

void __darwin_rewind(__darwin_FILE* fp)
{
	return rewind(fp->linux_fp);
}

int __darwin_getc(__darwin_FILE* fp)
{
	return getc(fp->linux_fp);
}

int __darwin_fgetc(__darwin_FILE* fp)
{
	return fgetc(fp->linux_fp);
}

int __darwin_ungetc(int c, __darwin_FILE* fp)
{
	return ungetc(c, fp->linux_fp);
}

char* __darwin_fgets(char* s, int size, __darwin_FILE* fp)
{
	return fgets(s, size, fp->linux_fp);
}

int __darwin_putc(int c, __darwin_FILE* fp)
{
	return putc(c, fp->linux_fp);
}

int __darwin_fputc(int c, __darwin_FILE* fp)
{
	return fputc(c, fp->linux_fp);
}

int __darwin_fputs(const char* s, __darwin_FILE* fp)
{
	return fputs(s, fp->linux_fp);
}

int __darwin_fprintf(__darwin_FILE* fp, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfprintf(fp->linux_fp, fmt, ap);
	va_end(ap);
	return r;
}

int __darwin_vfscanf(__darwin_FILE* fp, const char* fmt, va_list ap)
{
	return vfscanf(fp->linux_fp, fmt, ap);
}

int __darwin_fscanf(__darwin_FILE* fp, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = vfscanf(fp->linux_fp, fmt, ap);
	va_end(ap);
	return r;
}

int __darwin_vfprintf(__darwin_FILE* fp, const char* fmt, va_list ap)
{
	return vfprintf(fp->linux_fp, fmt, ap);
}

int __darwin_fflush(__darwin_FILE* fp)
{
	if (!fp)
		return fflush(NULL);
	return fflush(fp->linux_fp);
}

void __darwin_setbuf(__darwin_FILE* fp, char* buf)
{
	setbuf(fp->linux_fp, buf);
}

void __darwin_setbuffer(__darwin_FILE* fp, char* buf, size_t size)
{
	setbuffer(fp->linux_fp, buf, size);
}

int __darwin_ferror(__darwin_FILE* fp)
{
	LOGF("ferror: %d\n", ferror(fp->linux_fp));
	return ferror(fp->linux_fp);
}

int __darwin_fileno(__darwin_FILE* fp)
{
	return fileno(fp->linux_fp);
}

__darwin_FILE* __darwin_tmpfile()
{
	return InitDarwinFILE(tmpfile());
}

__attribute__((constructor)) static void initStdio()
{
	__stderrp = InitDarwinFILE(stdin);
	__stdoutp = InitDarwinFILE(stdout);
	__stdinp = InitDarwinFILE(stderr);
}

char *__darwin_realpath(const char *path, char *resolved_path)
{
	if (!path)
	{
		errno = DARWIN_EINVAL;
		return 0;
	}
	
	path = translatePathCI(path);
	
	char* rv = realpath(path, resolved_path);
	if (!rv)
		errnoOut();
	
	return rv;
}


