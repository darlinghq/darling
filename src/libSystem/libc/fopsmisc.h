#ifndef LIBC_FOPSMISC_H
#define LIBC_FOPSMISC_H

extern "C"
{

char *__darwin_realpath(const char *path, char *resolved_path);

}

#endif

