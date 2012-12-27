/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DARWIN_LD_H
#define DARWIN_LD_H
#include <dlfcn.h>
#include <unordered_map>
//#include "MachOLoader.h"
#include "ELFBlock.h"

#define DARWIN_RTLD_LAZY		0x1
#define DARWIN_RTLD_NOW			0x2
#define DARWIN_RTLD_LOCAL		0x4
#define DARWIN_RTLD_GLOBAL		0x8
#define DARWIN_RTLD_NOLOAD		0x10
#define DARWIN_RTLD_NODELETE	0x80
#define DARWIN_RTLD_FIRST		0x100
#define __DARLING_RTLD_NOBIND	0x20000000

#define DARWIN_RTLD_NEXT		((void*)-1)
#define DARWIN_RTLD_DEFAULT		((void*)-2)
#define DARWIN_RTLD_SELF		((void*)-3)
#define DARWIN_RTLD_MAIN_ONLY	((void*)-5)

// Internal, only for weak symbol resolution
#define __DARLING_RTLD_STRONG ((void*)-20)

typedef void* NSSymbol;
typedef void* NSModule;

extern "C"
{

void* __darwin_dlopen(const char* filename, int flag);
int __darwin_dlclose(void* handle);
const char* __darwin_dlerror(void);
void* __darwin_dlsym(void* handle, const char* symbol, void* extra = nullptr);
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

enum LoadedLibraryType { LoadedLibraryDylib, LoadedLibraryNative, LoadedLibraryDummy };

typedef std::unordered_map<std::string, MachO::Export> Exports;

// TODO: this should be united with the list in FileMap
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
	//intptr slide;
    //intptr base;
	Exports* exports;
	ELFBlock elf;

	LoadedLibrary(void) : elf(name) {}
};

namespace Darling
{
	typedef bool (*DlsymHookFunc)(char* symName);
	void registerDlsymHook(DlsymHookFunc func);
	void deregisterDlsymHook(DlsymHookFunc func);
	void* DlopenWithContext(const char* filename, int flag, const std::vector<std::string>& rpaths, bool* notFoundError = nullptr);
};

#endif
