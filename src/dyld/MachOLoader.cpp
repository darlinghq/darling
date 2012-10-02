#include "config.h"
#include "MachOLoader.h"
#include "MachO.h"
#include "ld.h"
#include "log.h"
#include "trace.h"
#include "FileMap.h"
#include "stlutils.h"
#include "public.h"
#include <limits.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <set>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>

FileMap g_file_map;
static std::vector<std::string> g_bound_names;

extern char g_darwin_executable_path[PATH_MAX];
extern int g_argc;
extern char** g_argv;
extern bool g_trampoline;
extern std::set<LoaderHookFunc*> g_machoLoaderHooks;

// These are GCC internals
extern "C" void __register_frame(void*);
extern "C" void __unregister_frame(void*);

static bool lookupDyldFunction(const char* name, void** addr)
{
	LOG << "lookupDyldFunction: " << name <<std::endl;
	*addr = dlsym(dlopen(0, 0), name);
	return (*addr) != 0;
}

static uint64_t alignMem(uint64_t p, uint64_t a)
{
	a--;
	return (p + a) & ~a;
}

static void dumpInt(int bound_name_id)
{
	if (bound_name_id < 0)
	{
		fprintf(stderr, "%d: negative bound function id\n", bound_name_id);
		return;
	}
	if (bound_name_id >= (int)g_bound_names.size())
	{
		fprintf(stderr, "%d: bound function id overflow\n", bound_name_id);
		return;
	}
	if (g_bound_names[bound_name_id].empty())
	{
		fprintf(stderr, "%d: unbound function id\n", bound_name_id);
		return;
	}
	printf("calling %s(%d)\n",
			g_bound_names[bound_name_id].c_str(), bound_name_id);
	fflush(stdout);
}

MachOLoader::MachOLoader()
: m_last_addr(0), m_pTrampolineMgr(0)
{
	m_pUndefMgr = new UndefMgr;

	if (g_trampoline)
	{
		m_pTrampolineMgr = new TrampolineMgr;
		
		const char* info = getenv("TRAMPOLINE_INFO");
		if (info)
			TrampolineMgr::loadFunctionInfo(info);
	}
}

MachOLoader::~MachOLoader()
{
}

void MachOLoader::loadDylibs(const MachO& mach)
{
	for (std::string dylib : mach.dylibs())
	{
		// __darwin_dlopen checks if already loaded
		// automatically adds a reference if so
		
		if (!__darwin_dlopen(dylib.c_str(), DARWIN_RTLD_GLOBAL|DARWIN_RTLD_NOW))
		{
			LOG << "Failed to dlopen " << dylib << ", throwing an exception\n";
			std::stringstream ss;
			ss << "Cannot load " << dylib << ": " << __darwin_dlerror();
			throw std::runtime_error(ss.str());
		}
	}
}

void MachOLoader::doMProtect()
{
	for (MProtect p : m_mprotects)
		::mprotect(p.addr, p.len, p.prot);
	m_mprotects.clear();
}

void MachOLoader::loadSegments(const MachO& mach, intptr* slide, intptr* base)
{
	*base = 0;
	--*base;
	
	if (m_pTrampolineMgr)
		m_pTrampolineMgr->invalidateMemoryMap();

	const std::vector<Segment*>& segments = getSegments(mach);
	for (Segment* seg : segments)
	{
		const char* name = seg->segname;
		if (!strcmp(name, SEG_PAGEZERO))
			continue;

		LOG << seg->segname << ": "
			<< "fileoff=" << seg->fileoff
			<< ", vmaddr=" << seg->vmaddr <<std::endl;

		int prot = 0, maxprot = 0;

		if (seg->initprot & VM_PROT_READ)
			prot |= PROT_READ;

		if (seg->initprot & VM_PROT_WRITE)
			prot |= PROT_WRITE;
		
		if (seg->initprot & VM_PROT_EXECUTE)
			prot |= PROT_EXEC;

		if (seg->maxprot & VM_PROT_READ)
			maxprot |= PROT_READ;

		if (seg->maxprot & VM_PROT_WRITE)
			maxprot |= PROT_WRITE;

		if (seg->maxprot & VM_PROT_EXECUTE)
			maxprot |= PROT_EXEC;
		

		intptr filesize = alignMem(seg->filesize, 0x1000);
		intptr vmaddr = seg->vmaddr + *slide;
		
		if (vmaddr < m_last_addr)
		{
			LOG << "will rebase: filename=" << mach.filename()
				<< ", vmaddr=" << (void*)vmaddr
				<< ", last_addr=" << (void*)m_last_addr <<std::endl;
			assert(seg == segments[0]);
			vmaddr = m_last_addr;
			*slide = vmaddr - seg->vmaddr;
		}
		*base = std::min(*base, vmaddr);

		intptr vmsize = alignMem(seg->vmsize, 0x1000);
		LOG << "mmap(file) " << mach.filename() << ' ' << name
			<< ": " << (void*)vmaddr << "-" << (void*)(vmaddr + filesize)
			<< " offset=" << mach.offset() + seg->fileoff <<std::endl;
		
		if (filesize == 0)
			continue;
		
		checkMmapMinAddr(vmaddr);
		
		void* mapped = ::mmap((void*)vmaddr, filesize, maxprot, MAP_PRIVATE | MAP_FIXED, mach.fd(), mach.offset() + seg->fileoff);

		m_mprotects.push_back(MProtect{(void*) vmaddr, filesize, prot});
		
		if (mapped == MAP_FAILED)
		{
			std::stringstream ss;
			ss << "Failed to mmap '" << mach.filename() << "': " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
		
		assert(vmsize >= filesize);

		if (vmsize > filesize)
		{
			LOG << "mmap(anon) " << mach.filename() << ' ' << name
				<< ": " << (void*)(vmaddr + filesize) << "-"
				<< (void*)(vmaddr + vmsize)
				<<std::endl;
			
			void* mapped = ::mmap(reinterpret_cast<char*>(vmaddr) + filesize, vmsize - filesize, prot, MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, 0, 0);
								
			if (mapped == MAP_FAILED)
			{
				std::stringstream ss;
				ss << "mmap(anon) failed on '" << mach.filename() << "': " << strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}

		m_last_addr = std::max(m_last_addr, (intptr)vmaddr + vmsize);
	}

	if (mach.get_eh_frame().first)
		__register_frame(reinterpret_cast<void*>(mach.get_eh_frame().first));
}

void MachOLoader::checkMmapMinAddr(intptr addr)
{
	static intptr minimum = -1;
	if (minimum == intptr(-1))
	{
		std::ifstream f("/proc/sys/vm/mmap_min_addr");

		if (!f.is_open())
			minimum = 0;
		else
			f >> minimum;
	}

	if (addr < minimum)
	{
		std::stringstream ss;
		ss << "Your vm.mmap_min_addr is too low for this application to be loaded. ";
		ss << "Try running `sysctl -w vm.mmap_min_addr=\"" << addr << "\"'";
		throw std::runtime_error(ss.str());
	}
}

void MachOLoader::doRebase(const MachO& mach, intptr slide)
{
	for (const MachO::Rebase* rebase : mach.rebases())
	{
		void* addr = reinterpret_cast<void*>(rebase->vmaddr + slide);
		switch (rebase->type)
		{
			case REBASE_TYPE_POINTER:
			{
				uintptr_t* ptr = reinterpret_cast<uintptr_t*>(addr);
				LOG << "rebase(ptr): " << addr << ' '
					<< (void*)*ptr << " => "
					<< (void*)(*ptr + slide) << " @" << ptr <<std::endl;
				*ptr += slide;
				break;
			}
			case REBASE_TYPE_TEXT_ABSOLUTE32:
			{
				uint32_t* ptr = reinterpret_cast<uint32_t*>(addr);
				LOG << "rebase(abs32): " << addr << ' '
					<< (void*)*ptr << " => " << (void*)(mach.fixEndian(*ptr) + slide) << std::endl;
				*ptr = mach.fixEndian(*ptr);
				*ptr += static_cast<uint32_t>(slide);
				break;
			}
			case REBASE_TYPE_TEXT_PCREL32: // TODO: test it
			{
				uint32_t* ptr = reinterpret_cast<uint32_t*>(addr);
				LOG << "rebase(pcrel32): " << addr << ' '
					<< (void*)*ptr << " => " << (void*)(uintptr_t(addr) + 4 - mach.fixEndian(*ptr)) << std::endl;
				*ptr = mach.fixEndian(*ptr);
				*ptr = uintptr_t(addr) + 4 - (*ptr);
				break;
			}

			default:
			{
				std::stringstream ss;
				ss << "Unknown rebase type: " << int(rebase->type);
				
				throw std::runtime_error(ss.str());
			}
		}
	}
}

void MachOLoader::loadInitFuncs(const MachO& mach, intptr slide)
{
	for (intptr addr : mach.init_funcs())
	{
		addr += slide;
		LOG << "Registering init func " << (void*)addr << " from " << mach.filename() << std::endl;
		m_init_funcs.push_back(addr);
	}
}

void MachOLoader::doBind(const MachO& mach, intptr slide)
{
	std::string last_weak_name;
	char* last_weak_sym = 0;
	size_t seen_weak_bind_index = 0;
	size_t seen_weak_binds_orig_size = m_seen_weak_binds.size();

	g_bound_names.resize(mach.binds().size());

	for (MachO::Bind* bind : mach.binds())
	{
		if (bind->type == BIND_TYPE_POINTER)
		{
			std::string name = bind->name.substr(1);
			void** ptr = (void**)(bind->vmaddr + slide);
			char* sym = 0;
			
			if (bind->is_weak)
			{
				if (last_weak_name == name)
				{
					sym = last_weak_sym;
				}
				else
				{
					last_weak_name = name;
					if (seen_weak_bind_index != seen_weak_binds_orig_size && !strcmp(m_seen_weak_binds[seen_weak_bind_index].first.c_str(), name.c_str()))
					{
						last_weak_sym = sym =
						m_seen_weak_binds[seen_weak_bind_index].second;
						seen_weak_bind_index++;
					}
					else
					{
						if (bind->is_classic)
						{
							*ptr = last_weak_sym = (char*)bind->value;
						}
						else
						{
							const Exports::const_iterator export_found = m_exports.find(bind->name);
							if (export_found != m_exports.end())
								*ptr = last_weak_sym = (char*)export_found->second.addr;
							else
								last_weak_sym = (char*)*ptr;
						}
					}
					
					m_seen_weak_binds.push_back(make_pair(name, last_weak_sym));
					while (seen_weak_bind_index != seen_weak_binds_orig_size && strcmp(m_seen_weak_binds[seen_weak_bind_index].first.c_str(), name.c_str()) <= 0)
						seen_weak_bind_index++;
					
					continue;
				}
			}
			else // not weak
			{
				// TODO: remove, replace with aliases
#ifndef __x86_64__
				static const char* SUF_UNIX03 = "$UNIX2003";
				static const size_t SUF_UNIX03_LEN = strlen(SUF_UNIX03);
				if (name.size() > SUF_UNIX03_LEN &&
					!strcmp(name.c_str() + name.size() - SUF_UNIX03_LEN,
							SUF_UNIX03)) {
				name = name.substr(0, name.size() - SUF_UNIX03_LEN);
				}
#endif

				if (bind->name[0] != '_')
				{
					// assume local (e.g. dyld_stub_binder)
					name = bind->name; // for correct error reporting
					sym = reinterpret_cast<char*>(dlsym(dlopen(0, 0), bind->name.c_str()));
				}
				else
					sym = reinterpret_cast<char*>(__darwin_dlsym(DARWIN_RTLD_DEFAULT, name.c_str()));
				
				if (!sym)
				{
					const char* ign_sym = getenv("DYLD_IGN_MISSING_SYMS");
					//if (!bind->addend)
					{
#ifdef __x86_64__
						if (ign_sym && atoi(ign_sym))
						{
							std::cerr << "!!! Undefined symbol: " << name << std::endl;
							
							char* dname = new char[name.size()+1];
							strcpy(dname, name.c_str());
							
							sym = reinterpret_cast<char*>(m_pUndefMgr->generateNew(dname));
						}
						else
#endif
						{
							std::stringstream ss;
							ss << "Undefined symbol: " << name;
							throw std::runtime_error(ss.str());
						}
					}
					//else
					//	sym = reinterpret_cast<char*>(bind->addend + slide);
				}
				else
					sym += bind->addend;
			}

			LOG << "bind " << name << ": "
				<< *ptr << " => " << (void*)sym << " @" << ptr << std::endl;

			if (g_trampoline)
				*ptr = m_pTrampolineMgr->generate(sym, name.c_str());
			else
				*ptr = sym;
		}
		else
		{
			std::stringstream ss;
			ss << "Unknown bind type: " << bind->type;
			throw std::runtime_error(ss.str());
		}
	}

	std::inplace_merge(m_seen_weak_binds.begin(), m_seen_weak_binds.begin() + seen_weak_binds_orig_size, m_seen_weak_binds.end());
}



void MachOLoader::loadExports(const MachO& mach, intptr base, Exports* exports)
{
	exports->rehash(exports->size() + mach.exports().size());
	
	for (MachO::Export* exp : mach.exports())
	{
		exp->addr += base;
		// TODO(hamaji): Not 100% sure, but we may need to consider weak symbols.
		if (!exports->insert(make_pair(exp->name, *exp)).second)
			fprintf(stderr, "duplicated exported symbol: %s\n", exp->name.c_str());
	}
}



void MachOLoader::load(const MachO& mach, std::string sourcePath, Exports* exports)
{
	if (!exports)
		exports = &m_exports;

	intptr slide = 0;
	intptr base = 0;

	loadSegments(mach, &slide, &base);

	doRebase(mach, slide);
	doMProtect(); // decrease the segment protection value

	loadDylibs(mach);
	
	loadInitFuncs(mach, slide);

	loadExports(mach, base, exports);

	doBind(mach, slide);

	const FileMap::ImageMap* img = g_file_map.add(mach, slide, base);

	for (LoaderHookFunc* func : g_machoLoaderHooks)
		func(&img->header, slide);
}


void MachOLoader::setupDyldData(const MachO& mach)
{
	if (!mach.dyld_data())
		return;

	void** dyld_data = (void**)mach.dyld_data();
	dyld_data[1] = reinterpret_cast<void*>(&lookupDyldFunction);
}

void MachOLoader::runPendingInitFuncs(int argc, char** argv, char** envp, char** apple)
{
	for (size_t i = 0; i < m_init_funcs.size(); i++)
	{
		void** init_func = (void**) m_init_funcs[i];
		LOG << "calling initializer function " << *init_func << std::endl;
		
		// TODO: missing ProgramVars! http://blogs.embarcadero.com/eboling/2010/01/29/5639/
		/*
		 *	struct ProgramVars {
			const void*	mh;
			int*		NXArgcPtr;
			const char***	NXArgvPtr;
			const char***	environPtr;
			const char**	__prognamePtr;
			};
		*/
		
		if (argc >= 0)
			((void(*)(int, char**, char**, char**))*init_func)(argc, argv, envp, apple);
		else
			((void(*)())*init_func)(); // ?!
	}
	m_init_funcs.clear();
}


void MachOLoader::run(MachO& mach, int argc, char** argv, char** envp)
{
	char* apple[2] = { g_darwin_executable_path, 0 };
	std::vector<char*> envCopy;
	
	//for (int i = 0; envp[i]; i++)
	//	envCopy.push_back(envp[i]);
	envCopy.push_back(0);

	load(mach, g_darwin_executable_path);
	setupDyldData(mach);

	g_file_map.addWatchDog(m_last_addr + 1);

	//g_timer.print(mach.filename().c_str());

	mach.close();

	runPendingInitFuncs(argc, argv, &envCopy[0], apple);
	
	fflush(stdout);
	assert(argc > 0);

	if (mach.entry())
	{
		LOG << "booting from " << (void*)mach.entry() << "..." << std::endl;
		LOG << "==========\n";
		boot(mach.entry(), argc, argv, envp, apple);
	}
	else if (mach.main())
	{
		LOG << "running main at " << (void*) mach.main() << "...\n";
		LOG << "==========\n";
		
		int (*pMain)(int, char**, char**, char**) = reinterpret_cast<int (*)(int, char**, char**, char**)>(mach.main());
		
		int rv = pMain(argc, argv, envp, apple);
		exit(rv);
	}
	else
		throw std::runtime_error("No entry point found");
}

static inline int findLast(char** array)
{
	for (int i = 0; ; i++)
	{
		if (!array[i])
			return i;
	}
}

void MachOLoader::boot( uint64_t entry, int argc, char** argv, char** envp, char** apple)
{
#ifdef __x86_64__
#	define PUSH(val) __asm__ volatile("pushq %0" :: "r"(uint64_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmpq *%0" :: "m"(addr) :)
#else
#	define PUSH(val) __asm__ volatile("pushl %0" :: "r"(uint32_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmp *%0" :: "r"(addr) :)
#endif

	for (int i = findLast(apple); i >= 0; i--)
		PUSH(apple[i]);

	for (int i = findLast(envp); i >= 0; i--)
		PUSH(envp[i]);

	assert(!argv[argc]);

	for (int i = argc; i >= 0; i--)
		PUSH(argv[i]);

	PUSH(argc);
	JUMP(entry);
}

#ifdef DEBUG
// GDB helper
extern "C" const char* dumpSymbol(void* p)
{
	return g_file_map.gdbInfoForAddr(p);
}
#endif



