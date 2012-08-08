#ifndef DARWIN_LD_H
#define DARWIN_LD_H
#include <dlfcn.h>

#define DARWIN_RTLD_LAZY		0x1
#define DARWIN_RTLD_NOW			0x2
#define DARWIN_RTLD_LOCAL		0x4
#define DARWIN_RTLD_GLOBAL		0x8
#define DARWIN_RTLD_NOLOAD		0x10
#define DARWIN_RTLD_NODELETE	0x80
#define DARWIN_RTLD_FIRST		0x100

#define DARWIN_RTLD_NEXT		((void*)-1)
#define DARWIN_RTLD_DEFAULT		((void*)-2)
#define DARWIN_RTLD_SELF		((void*)-3)
#define DARWIN_RTLD_MAIN_ONLY	((void*)-5)

extern "C"
{

void* __darwin_dlopen(const char* filename, int flag);
int __darwin_dlclose(void* handle);
const char* __darwin_dlerror(void);
void* __darwin_dlsym(void* handle, const char* symbol);
int __darwin_dladdr(void *addr, Dl_info *info);

}

#ifdef DARWIN_LD_INTERNAL
enum LoadedLibraryType { LoadedLibraryDylib, LoadedLibraryNative };

struct LoadedLibrary
{
	std::string name;
	int refCount;
	LoadedLibraryType type;
	union
	{
		void* nativeRef;
		MachO* machoRef;
	};
};
#endif

#endif
