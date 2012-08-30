
#ifndef FILEMAP_H
#define FILEMAP_H
#include <stdint.h>
#include <string>
#include <map>
#include "MachO.h"
#include <dlfcn.h>

class FileMap
{
public:
	~FileMap();
	
	void add(const MachO& mach, uintptr_t slide, uintptr_t base);

	void addWatchDog(uintptr_t addr);

	const char* gdbInfoForAddr(void* p);
	bool findSymbolInfo(void* addr, Dl_info* p); // used by __darwin_dladdr

private:
	struct SymbolMap
	{
		std::string filename;
		std::map<uintptr_t, std::string> symbols;
		uintptr_t base;
	};

	std::map<uintptr_t, SymbolMap*> m_maps;
	char m_dumped_stack_frame_buf[4096];
};


#endif
