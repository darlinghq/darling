
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

	struct ImageMap
	{
		std::string filename;
		std::map<uintptr_t, std::string> symbols;
		uintptr_t base, slide;
		mach_header header;
		std::pair<uint64_t,uint64_t> eh_frame;
		std::pair<uint64_t,uint64_t> unwind_info;
	};
	
	const ImageMap* imageMapForAddr(const void* p);
	const char* fileNameForAddr(const void* p);
	const char* gdbInfoForAddr(const void* p);
	bool findSymbolInfo(const void* addr, Dl_info* p); // used by __darwin_dladdr

	const std::vector<ImageMap*>& images() const { return m_maps_vec; }

private:

	std::map<uintptr_t, ImageMap*> m_maps;
	std::vector<ImageMap*> m_maps_vec; // for easier access
	char m_dumped_stack_frame_buf[4096];
};


#endif
