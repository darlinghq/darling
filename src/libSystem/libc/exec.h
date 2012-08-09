#ifndef LIBC_EXEC_H
#define LIBC_EXEC_H

extern "C"
{

int __darwin_execl(const char *path, const char *arg, ...);
int __darwin_execlp(const char *file, const char *arg, ...);
int __darwin_execle(const char *path, const char *arg, ... /*, char * const envp[]*/);
int __darwin_execv(const char *path, char *const argv[]);
int __darwin_execvp(const char *file, char *const argv[]);
int __darwin_execvpe(const char *file, char *const argv[], char *const envp[]);

}

#endif
