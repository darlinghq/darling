#ifndef LIBC_NEXTSTEP_H
#define LIBC_NEXTSTEP_H

extern "C"
{

char*** _NSGetEnviron();
int* _NSGetArgc();
char*** _NSGetArgv();

}

#endif
