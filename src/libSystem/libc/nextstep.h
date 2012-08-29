#ifndef LIBC_NEXTSTEP_H
#define LIBC_NEXTSTEP_H

extern "C" char* __darwin_executable_path;
extern "C" char* __loader_path;

extern "C"
{

int _NSGetExecutablePath(char* buf, unsigned int* size);
char*** _NSGetEnviron();

}

#endif
