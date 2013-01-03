/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel
Copyright (C) 2011 Shinichiro Hamaji

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
#include <libgen.h>

FileMap g_file_map;
static std::vector<std::string> g_bound_names;

char g_darwin_loader_path[PATH_MAX] = "";

extern char g_darwin_executable_path[PATH_MAX];
extern bool g_trampoline;
extern bool g_noWeak;
extern std::set<LoaderHookFunc*> g_machoLoaderHooks;
extern MachOLoader* g_loader;

// These are GCC internals
extern "C" void __register_frame(void*);
extern "C" void __unregister_frame(void*);

static bool lookupDyldFunction(const char* name, void** addr)
{
	LOG << "lookupDyldFunction: " << name <<std::endl;
	*addr = dlsym(dlopen(0, 0), name);

	if (!*addr)
		*addr = (void*) (void (*)()) []() { LOG << "Fake dyld function called\n"; };

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
#ifdef DEBUG
	m_pUndefMgr = new UndefMgr;

	if (g_trampoline)
	{
		m_pTrampolineMgr = new TrampolineMgr;
		
		const char* info = getenv("TRAMPOLINE_INFO");
		if (info)
			TrampolineMgr::loadFunctionInfo(info);
	}
#endif
}

MachOLoader::~MachOLoader()
{
}

void MachOLoader::loadDylibs(const MachO& mach, bool nobind, bool bindLazy)
{
	for (std::string dylib : mach.dylibs())
	{
		// __darwin_dlopen checks if already loaded
		// automatically adds a reference if so
		
		int flags = DARWIN_RTLD_GLOBAL;
		if (nobind)
			flags |= __DARLING_RTLD_NOBIND; 
		if (bindLazy)
			flags |= DARWIN_RTLD_LAZY;
		else
			flags |= DARWIN_RTLD_NOW;

		if (!Darling::DlopenWithContext(dylib.c_str(), flags, m_rpathContext))
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

#ifdef DEBUG
	if (m_pTrampolineMgr)
		m_pTrampolineMgr->invalidateMemoryMap();
#endif

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
					<< std::hex << *ptr << std::dec << " => " << (void*)(mach.fixEndian(*ptr) + slide) << std::endl;
				*ptr = mach.fixEndian(*ptr);
				*ptr += static_cast<uint32_t>(slide);
				break;
			}
			case REBASE_TYPE_TEXT_PCREL32: // TODO: test it
			{
				uint32_t* ptr = reinterpret_cast<uint32_t*>(addr);
				LOG << "rebase(pcrel32): " << addr << ' '
					<< std::hex << *ptr << std::dec << " => " << (void*)(uintptr_t(addr) + 4 - mach.fixEndian(*ptr)) << std::endl;
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

void MachOLoader::doRelocations(const std::vector<MachO::Relocation*>& rels, intptr base, intptr slide)
{
	m_lastResolvedSymbol.clear();
	m_lastResolvedAddress = 0;

	for (const MachO::Relocation* rel : rels)
	{
		uintptr_t* ptr = (uintptr_t*) (uintptr_t(rel->addr) /*+ base*/ + slide);
		uintptr_t symbol;
		uintptr_t value = *ptr;

		symbol = getSymbolAddress(rel->name);

		value += symbol;

#ifdef __i386__
		if (rel->pcrel)
		{
			LOG << "reloc(pcrel): @" << ptr << " " << std::hex << *ptr << " -> " << (value - uintptr_t(ptr) - 4) << std::dec << std::endl;
			*ptr = value - uintptr_t(ptr) - 4;
		}
		else
#endif
		{
			LOG << "reloc: @" << ptr << " " << std::hex << *ptr << " -> " << value << std::dec << std::endl;
			*ptr = value;
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

void* MachOLoader::doBind(const std::vector<MachO::Bind*>& binds, intptr slide, bool resolveLazy)
{
	std::string last_weak_name;
	uintptr_t last_weak_sym = 0;
	size_t seen_weak_bind_index = 0;
	size_t seen_weak_binds_orig_size = m_seen_weak_binds.size();
	uintptr_t sym;

	g_bound_names.resize(binds.size());

	m_lastResolvedSymbol.clear();
	m_lastResolvedAddress = 0;

	for (const MachO::Bind* bind : binds)
	{
		if (bind->is_lazy)
		{
			if (!resolveLazy)
			{
				LOG << "Delaying lazy bind resolution, offset=" << std::hex << bind->offset << std::dec << std::endl;
				continue;
			}
			else
				LOG << "Lazy bind resolution forced\n";
		}
		
		if (bind->type == BIND_TYPE_POINTER || bind->type == BIND_TYPE_STUB)
		{
			std::string name = bind->name.substr(1);
			uintptr_t* ptr = (uintptr_t*)(bind->vmaddr + slide);

			sym = 0;
		
			// FIXME: This unholy mess needs to be fixed, weak bind handling shouldn't be implemented separately like this
			if (bind->is_weak)
			{
				if (g_noWeak)
					continue;
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
						//if (false)
						{
							uintptr_t addr = bind->value;
							if (!addr)
							{
								LOG << "Trying to resolve classic,weak,null\n";
								addr = (uintptr_t) __darwin_dlsym(__DARLING_RTLD_STRONG, name.c_str());
							}
							LOG << "Bind (classic, weak) @" << ptr << " -> " << std::hex << uintptr_t(addr) << std::dec << std::endl;
							
							writeBind(bind->type, ptr, uintptr_t(addr));
							last_weak_sym = uintptr_t(addr);
						}
						else
						{
							void* expAddr = __darwin_dlsym(__DARLING_RTLD_STRONG, name.c_str());
							if (expAddr != nullptr)
							{
								LOG << "Weak reference overridden for " << name << std::endl;
								LOG << (void*)*ptr << " -> " << (void*)expAddr << " @" << ptr << std::endl;

								writeBind(bind->type, ptr, uintptr_t(expAddr));

								*ptr = last_weak_sym = uintptr_t(expAddr);
							}
							else
							{
								LOG << "Weak reference not overriden for " << name << std::endl;
								last_weak_sym = *ptr;
							}
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
				sym = getSymbolAddress(bind->name, bind, slide);

				if (!bind->is_classic)
					sym += bind->addend;
			}

			LOG << "bind " << name << ": "
				<< std::hex << *ptr << std::dec << " => " << (void*)sym << " @" << ptr << std::endl;
#ifdef DEBUG
			if (g_trampoline)
				sym = (uintptr_t) m_pTrampolineMgr->generate((void*)sym, name.c_str());
#endif

			writeBind(bind->type, ptr, sym);
		}
		else
		{
			std::stringstream ss;
			ss << "Unknown bind type: " << bind->type;
			throw std::runtime_error(ss.str());
		}
	}

	std::inplace_merge(m_seen_weak_binds.begin(), m_seen_weak_binds.begin() + seen_weak_binds_orig_size, m_seen_weak_binds.end());

	// This return value is used by dyld_stub_binder
	return reinterpret_cast<void*>(sym);
}

uintptr_t MachOLoader::getSymbolAddress(const std::string& oname, const MachO::Bind* bind, intptr slide)
{
	std::string name;
	uintptr_t sym;

	if (oname == m_lastResolvedSymbol)
		return m_lastResolvedAddress;

	if (oname[0] != '_')
	{
		// assume local (e.g. dyld_stub_binder)
		name = oname;
		sym = reinterpret_cast<uintptr_t>(dlsym(dlopen(0, 0), name.c_str()));
	}
	else
	{
		name = oname.substr(1);

		// TODO: remove, replace with aliases
#ifndef __x86_64__
		static const char* SUF_UNIX03 = "$UNIX2003";
		static const size_t SUF_UNIX03_LEN = strlen(SUF_UNIX03);
		if (name.size() > SUF_UNIX03_LEN && !strcmp(name.c_str() + name.size() - SUF_UNIX03_LEN, SUF_UNIX03))
			name = name.substr(0, name.size() - SUF_UNIX03_LEN);
#endif
		sym = reinterpret_cast<uintptr_t>(__darwin_dlsym(DARWIN_RTLD_DEFAULT, name.c_str()));
	}
	
	if (!sym)
	{
#ifdef DEBUG
		static const char* ign_sym = getenv("DYLD_IGN_MISSING_SYMS");
		if (!bind || !bind->is_classic || !bind->value)
		{
			if (ign_sym && atoi(ign_sym))
			{
				std::cerr << "!!! Undefined symbol: " << name << std::endl;
							
				char* dname = new char[name.size()+1];
				strcpy(dname, name.c_str());
				
				sym = reinterpret_cast<uintptr_t>(m_pUndefMgr->generateNew(dname));
			}
			else
			{
				std::stringstream ss;
				ss << "Undefined symbol: " << name;
				throw std::runtime_error(ss.str());
			}
		}
#endif
		if (bind && bind->is_classic)
			sym = uintptr_t(bind->value) + slide;
	}

	m_lastResolvedSymbol = oname;
	m_lastResolvedAddress = sym;

	return sym;
}

void MachOLoader::writeBind(int type, uintptr_t* ptr, uintptr_t newAddr)
{
	if (type == BIND_TYPE_POINTER)
		*ptr = newAddr;

#ifdef __i386__
	else if (type == BIND_TYPE_STUB)
	{
		struct jmp_instr
		{
			uint8_t relJmp;
			uint32_t addr;
		} __attribute__((packed));
		static_assert(sizeof(jmp_instr) == 5, "Incorrect jmp instruction size");

		jmp_instr* instr = reinterpret_cast<jmp_instr*>(ptr);

		instr->relJmp = 0xE9; // x86 jmp rel32
		instr->addr = newAddr - uint32_t(ptr) - sizeof(jmp_instr);
	}
#endif
}

void MachOLoader::loadExports(const MachO& mach, intptr base, Exports* exports)
{
	exports->rehash(exports->size() + mach.exports().size());
	
	for (MachO::Export* exp : mach.exports())
	{
		exp->addr += base;
		// TODO(hamaji): Not 100% sure, but we may need to consider weak symbols.
		if (!exports->insert(make_pair(exp->name, *exp)).second)
		{
			// Until we support two-level namespaces, duplicate symbols may happen where they would not happen on Darwin.
			// In this case we simply use the first known symbol.
			LOG << "Warning: duplicate exported symbol name: " << exp->name << std::endl;
		}
	}
}

const std::string& MachOLoader::getCurrentLoader() const
{
	assert(!m_loaderPath.empty());
	return m_loaderPath.top();
}

void MachOLoader::pushCurrentLoader(const char* currentLoader)
{
	char path[4096];
	strcpy(path, currentLoader);
	// @loader_path contains the directory where the Mach-O file currently loading other libraries resides
	m_loaderPath.push(dirname(path));
}

void MachOLoader::popCurrentLoader()
{
	m_loaderPath.pop();
}

void MachOLoader::load(const MachO& mach, std::string sourcePath, Exports* exports, bool bindLater, bool bindLazy)
{
	intptr slide = 0;
	intptr base = 0;
	const FileMap::ImageMap* img;
	size_t origRpathCount;

	m_exports.push_back(exports);
	pushCurrentLoader(sourcePath.c_str());

	loadSegments(mach, &slide, &base);

	doRebase(mach, slide);
	doMProtect(); // decrease the segment protection value
	
	
	origRpathCount = m_rpathContext.size();
	
	for (const char* rpath : mach.rpaths())
		m_rpathContext.push_back(rpath);
	loadDylibs(mach, bindLater, bindLazy);
	m_rpathContext.resize(origRpathCount);
	
	loadInitFuncs(mach, slide);

	loadExports(mach, base, exports);
	
	
	img = g_file_map.add(mach, slide, base, bindLazy);
	
	if (!bindLater)
		doBind(mach.binds(), slide, !bindLazy);
	doRelocations(mach.relocations(), base, slide);

	if (!bindLater)
	{
		for (LoaderHookFunc* func : g_machoLoaderHooks)
			func(img->header, slide);
	}
	else
	{
		LOG << mach.binds().size() << " binds pending\n";
		m_pendingBinds.push_back(PendingBind{ &mach, img->header, slide, bindLazy });
	}
	
	popCurrentLoader();
}

void MachOLoader::doPendingBinds()
{
	for (const PendingBind& b : m_pendingBinds)
	{
		LOG << "Perform " << b.macho->binds().size() << " binds\n";
		doBind(b.macho->binds(), b.slide, b.bindLazy);
		if (b.macho->get_eh_frame().first)
		{
			void* frame = reinterpret_cast<void*>(b.macho->get_eh_frame().first + b.slide);
			LOG << "Registering EH frame at " << frame << std::endl;
			__register_frame(frame);
		}
		for (LoaderHookFunc* func : g_machoLoaderHooks)
			func(b.header, b.slide);
	}
	m_pendingBinds.clear();
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


void MachOLoader::run(MachO& mach, int argc, char** argv, char** envp, bool bindLazy)
{
	char* apple[2] = { g_darwin_executable_path, 0 };
	std::vector<char*> envCopy;
	
	//for (int i = 0; envp[i]; i++)
	//	envCopy.push_back(envp[i]);
	envCopy.push_back(0);

	m_mainExports = new Exports;
	load(mach, g_darwin_executable_path, m_mainExports, true, bindLazy);
	setupDyldData(mach);

	g_file_map.addWatchDog(m_last_addr + 1);

	//g_timer.print(mach.filename().c_str());

	doPendingBinds();
	runPendingInitFuncs(argc, argv, &envCopy[0], apple);
	
	mach.close();
	
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

extern "C" void* dyld_stub_binder_fixup(const FileMap::ImageMap** imageMap, uintptr_t lazyOffset)
{
	if (!*imageMap)
	{
		LOG << "Finding image for address " << imageMap << std::endl;
		*imageMap = g_file_map.imageMapForAddr((void*) imageMap);

		if (!*imageMap)
		{
			std::cerr << "dyld_stub_binder_fixup(): Image map not found for " << imageMap << std::endl;
			abort();
		}
	}

	std::vector<MachO::Bind*> toBind; // will hold only the single item we need to bind

	(*imageMap)->mutex_lazy_binds.lock();

	for (auto it = (*imageMap)->lazy_binds.begin(); it != (*imageMap)->lazy_binds.end(); it++)
	{
		if (it->offset == lazyOffset)
		{
			toBind.push_back(const_cast<MachO::Bind*>(&*it));
			break;
		}
	}

	if (toBind.empty())
	{
		std::cerr << "dyld_stub_binder_fixup(): Lazy bind not found for offset " << std::hex << lazyOffset << std::dec << std::endl;
		abort();
	}

	void* addr;
	try
	{
		addr = g_loader->doBind(toBind, (*imageMap)->slide, true);
	}
	catch (const std::exception& e)
	{
		std::cerr << "dyld_stub_binder_fixup(): Failed to resolve the symbol: " << e.what() << std::endl;
		abort();
	}

	(*imageMap)->mutex_lazy_binds.unlock();
	return addr;
}

#ifdef DEBUG
// GDB helper
extern "C" const char* dumpSymbol(void* p)
{
	return g_file_map.gdbInfoForAddr(p);
}
#endif



