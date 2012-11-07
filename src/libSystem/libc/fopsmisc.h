#ifndef LIBC_FOPSMISC_H
#define LIBC_FOPSMISC_H

#ifdef __cplusplus
extern "C"
{
#endif

char *__darwin_realpath(const char *path, char *resolved_path);
int __darwin_mkstemp(char *tpl);
int __darwin_mkostemp (char *tpl, int flags);
char *__darwin_mktemp(char *tpl);
char *__darwin_tempnam(const char *dir, const char *pfx);

#ifdef __cplusplus
}
#endif

#endif

