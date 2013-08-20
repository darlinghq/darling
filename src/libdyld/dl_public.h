#ifndef DL_PUBLIC_H
#define DL_PUBLIC_H
#include <dlfcn.h>
#include <string>

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

namespace Darling
{
	class LoadableObject;
	
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
	
	void dl_setLastError(const std::string& str);
}

typedef void* NSSymbol;
typedef Darling::LoadableObject* NSModule;

extern "C"
{

void* __darwin_dlopen(const char* filename, int flag);
int __darwin_dlclose(void* handle);
const char* __darwin_dlerror(void);
void* __darwin_dlsym(void* handle, const char* symbol);
int __darwin_dladdr(void *addr, Dl_info *info);

// Obsolete (or "not recommended") APIs
NSSymbol NSLookupAndBindSymbol(const char* symbolName);
void* NSAddressOfSymbol(NSSymbol nssymbol);
NSSymbol NSLookupSymbolInModule(NSModule module, const char* symbolName);
const char* NSNameOfSymbol(NSSymbol nssymbol);
int NSIsSymbolNameDefined(const char* name);
NSModule NSModuleForSymbol(NSSymbol symbol);
int NSIsSymbolNameDefinedInImage(const struct mach_header *image, const char *symbolName);
const char* NSNameOfModule(NSModule m); 
const char* NSLibraryNameForModule(NSModule m);
// TODO: rest of these NS* calls if used anywhere

}

#endif

