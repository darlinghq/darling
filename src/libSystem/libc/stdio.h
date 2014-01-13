#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H
#include <cstdio>
#include <stdarg.h>
#include <wchar.h>
#include <bits/basic_file.h>
#include <ext/stdio_filebuf.h>

/* stdio buffers */
struct __darwin_sbuf
{
        unsigned char   *_base;
        int             _size;
};

// Unfortunately, putc_nolock depends on FILE's layout,
// so we need to wrap linux's FILE with darwin's layout.
typedef struct __darwin_sFILE
{
	union
	{
#if defined(__x86_64__) || defined(__ppc64__)
		char __identical_size[152];
#else
		char __identical_size[88];
#endif

		struct
		{
			unsigned char *_p;      /* current position in (some) buffer */
			int     _r;             /* read space left for getc() */
			int     _w;             /* write space left for putc() */
			// TODO(hamaji): we need to modify this value with ferror and feof...
			short   _flags;         /* flags, below; this FILE is free if 0 */
			short   _file;          /* fileno, if Unix descriptor, else -1 */
			struct  __darwin_sbuf _bf;     /* the buffer (at least 1 byte, if !NULL) */
			int     _lbfsize;       /* 0 or -_bf._size, for inline putc */

#if 0
  /* operations */
  void    *_cookie;       /* cookie passed to io functions */
  int     (*_close)(void *);
  int     (*_read) (void *, char *, int);
  fpos_t  (*_seek) (void *, fpos_t, int);
  int     (*_write)(void *, const char *, int);

  /* separate buffer for long sequences of ungetc() */
  struct  __sbuf _ub;     /* ungetc buffer */
  struct __sFILEX *_extra; /* additions to FILE to not break ABI */
  int     _ur;            /* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3]; /* guarantee an ungetc() buffer */
  unsigned char _nbuf[1]; /* guarantee a getc() buffer */

  /* separate buffer for fgetln() when line crosses buffer boundary */
  struct  __sbuf _lb;     /* buffer for fgetln() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int     _blksize;       /* stat.st_blksize (may be != _bf._size) */
  fpos_t  _offset;        /* current lseek offset (see WARNING) */

#endif

			FILE* linux_fp;
		};
	};
} __darwin_FILE;

extern "C"
{

__darwin_FILE* __darwin_fopen(const char* path, const char* mode);
__darwin_FILE* __darwin_fdopen(int fd, const char* mode);
__darwin_FILE* __darwin_freopen(const char* path, const char* mode, __darwin_FILE* fp);
int __darwin_fclose(__darwin_FILE* fp);
size_t __darwin_fread(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp);
size_t __darwin_fwrite(void* ptr, size_t size, size_t nmemb, __darwin_FILE* fp);
int __darwin_fseek(__darwin_FILE* fp, long offset, int whence);
int __darwin_fseeko(__darwin_FILE* fp, off_t offset, int whence);
long __darwin_ftell(__darwin_FILE* fp);
off_t __darwin_ftello(__darwin_FILE* fp);
void __darwin_rewind(__darwin_FILE* fp);
int __darwin_getc(__darwin_FILE* fp);
int __darwin_fgetc(__darwin_FILE* fp);
int __darwin_ungetc(int c, __darwin_FILE* fp);
char* __darwin_fgets(char* s, int size, __darwin_FILE* fp);
int __darwin_putc(int c, __darwin_FILE* fp);
int __darwin_fputc(int c, __darwin_FILE* fp);
int __darwin_fputs(const char* s, __darwin_FILE* fp);
int __darwin_fprintf(__darwin_FILE* fp, const char* fmt, ...);
int __darwin_vfscanf(__darwin_FILE* fp, const char* fmt, va_list ap);
int __darwin_fscanf(__darwin_FILE* fp, const char* fmt, ...);
int __darwin_vfprintf(__darwin_FILE* fp, const char* fmt, va_list ap);
int __darwin_fflush(__darwin_FILE* fp);
int __darwin_setvbuf(__darwin_FILE* stream, char* buffer, int mode, size_t size);
void __darwin_setbuf(__darwin_FILE* stream, char* buffer);
void __darwin_setbuffer(__darwin_FILE* fp, char* buf, size_t size);
int __darwin_ferror(__darwin_FILE* fp);
int __darwin_fileno(__darwin_FILE* fp);
__darwin_FILE* __darwin_tmpfile();

int __darwin_feof(__darwin_FILE *stream);
void __darwin_clearerr(__darwin_FILE *stream);

// pipes
__darwin_FILE* __darwin_popen(const char* command, const char* type);
int __darwin_pclose(__darwin_FILE* stream);

int __darwin_fgetpos(__darwin_FILE *stream, fpos_t *pos);
int __darwin_fsetpos(__darwin_FILE *stream, fpos_t *pos);
int __darwin_fpurge(__darwin_FILE *stream);
int __darwin_getw(__darwin_FILE *stream);
int __darwin_putw(int w, __darwin_FILE *stream);
wint_t __darwin_fgetwc(__darwin_FILE *stream);
wint_t __darwin_getwc(__darwin_FILE *stream);
wint_t __darwin_fputwc(wchar_t wc, __darwin_FILE *stream);
wint_t __darwin_putwc(wchar_t wc, __darwin_FILE *stream);
int __darwin_fputws(const wchar_t *ws, __darwin_FILE *stream);
wchar_t *__darwin_fgetws(wchar_t *ws, int n, __darwin_FILE *stream);
wint_t __darwin_ungetwc(wint_t wc, __darwin_FILE *stream);

// Internal Darwin Libc functions
int __darwin___srget(__darwin_FILE* f);
int __darwin___swbuf(int c, __darwin_FILE* f);
int __darwin___svfscanf(__darwin_FILE* f, const char* fmt, va_list va);

int __darwin_remove(const char* path);

// bsd/stdio.h
char* __darwin_fgetln(__darwin_FILE* f, size_t* lenp);

// bsd/libutil.h
char* __darwin_fparseln(__darwin_FILE* f, size_t* a2, size_t* a3, const char* a4, int a5);

// bsd/wchar.h
wchar_t* __darwin_fgetwln(__darwin_FILE* f, size_t* len);

char* ctermid_r(char* p);

std::__basic_file<char>* _ZNSt12__basic_fileIcE8sys_openEP7__sFILESt13_Ios_Openmode(std::__basic_file<char>* pThis, __darwin_FILE* f, std::ios_base::openmode mode);
void _ZN9__gnu_cxx13stdio_filebufIcSt11char_traitsIcEEC1EP7__sFILESt13_Ios_Openmodem(__gnu_cxx::stdio_filebuf<char, std::char_traits<char> >* pThis, __darwin_FILE* f, std::_Ios_Openmode m, unsigned long l);
void _ZN9__gnu_cxx13stdio_filebufIcSt11char_traitsIcEEC2EP7__sFILESt13_Ios_Openmodem(__gnu_cxx::stdio_filebuf<char, std::char_traits<char> >* pThis, __darwin_FILE* f, std::_Ios_Openmode m, unsigned long l);

}

#endif
