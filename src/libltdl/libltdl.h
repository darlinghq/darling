#ifndef LIBLTDL_H
#define LIBLTDL_H
#include <ltdl.h>

extern "C" {

int __darwin_lt_dlinit();
int __darwin_lt_dlexit();

lt_dlhandle __darwin_lt_dlopen(const char *filename);
lt_dlhandle __darwin_lt_dlopenext(const char *filename);
int __darwin_lt_dlclose (lt_dlhandle handle);
lt_ptr __darwin_lt_dlsym (lt_dlhandle handle, const char *name);
const char* __darwin_lt_dlerror();
int __darwin_lt_dlpreload (const lt_dlsymlist *preloaded);
int __darwin_lt_dlpreload_default (const lt_dlsymlist *preloaded);
int __darwin_lt_dladdsearchdir (const char *search_dir);
int __darwin_lt_dlsetsearchpath (const char *search_path);
const char* __darwin_lt_dlgetsearchpath();
int __darwin_lt_dlmakeresident (lt_dlhandle handle);
int __darwin_lt_dlisresident (lt_dlhandle handle);

}

#endif
