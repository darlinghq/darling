#include "dl_public.h"
#include "MachOObject.h"
#include <limits>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>
#include <util/debug.h>
#include <libgen.h>
#include <iostream>
#include "MachOMgr.h"
#include "eh/EHSection.h"
#include "DylibSearch.h"
#include "arch.h"
#include "TLS.h"

extern "C" int g_argc asm("NXArgc") = 0;
extern "C" char** g_argv asm("NXArgv") = nullptr;

namespace Darling {

struct ProgramVars
{
	const void* mh;
	int* NXArgcPtr;
	const char*** NXArgvPtr;
	const char*** environPtr;
	const char** __prognamePtr;
};

struct DyldSectData
{
	void* lazyBinderPtr;
	bool (*funcLookupPtr)(const char*, void**);
	ProgramVars vars;
};

MachOObject::MachOObject(const std::string& path)
{
	m_file = MachO::readFile(path, ARCH_NAME, true, MachOMgr::instance()->loadAnyArchitecture());
	
	if (!m_file)
	{
		std::stringstream ss;
		ss << "Cannot load " << path;
		throw std::runtime_error(ss.str());
	}
	
	postConstruct();
}

MachOObject::MachOObject(MachO* file)
: m_file(file)
{
	postConstruct();
}

void MachOObject::postConstruct()
{
	detectAbsolutePath();
	m_header = m_file->header();

	for (std::string rpath : m_file->rpaths())
	{
		if (rpath.compare(0, 16, "@executable_path") == 0)
		{
			if (isMainModule())
				rpath.replace(0, 16, directory());
			else
			{
				MachOObject* mainModule = MachOMgr::instance()->mainModule();
			
				if (!mainModule)
					throw std::runtime_error("Cannot resolve @executable_path without a main module");
			
				rpath.replace(0, 16, mainModule->directory());
			}
		}
		else if (rpath.compare(0, 12, "@loader_path") == 0)
		{
			rpath = expandLoaderPath(rpath, this);
		}

		m_rpaths.push_back(rpath);
	}

	m_bindAllAtLoad = MachOMgr::instance()->bindAtLaunch();
}

MachOObject::~MachOObject()
{
	if (isLoaded())
		unload();
	delete m_file;
}

std::string MachOObject::expandLoaderPath(std::string path, MachOObject* loader)
{
	char* p;
	path.replace(0, 12, loader->directory());

	p = realpath(path.c_str(), nullptr);
	
	if (p != nullptr)
	{
		path = p;
		free(p);
	}

	return path;
}

void MachOObject::setRequesterRunpaths(MachOObject* requester)
{
	const auto& rpaths = requester->m_rpaths;
	m_rpaths.insert(m_rpaths.begin(), rpaths.begin(), rpaths.end());
}

void* MachOObject::maxAddress() const
{
	if (m_mappings.empty())
		return nullptr;
	
	auto& last = m_mappings.back();
	
	return static_cast<char*>(last.start) + last.size;
}

bool MachOObject::isMainModule() const
{
	return m_file->header().filetype == MH_EXECUTE;
}

void MachOObject::load()
{
	if (isLoaded())
		throw std::logic_error("Module already loaded");
	if (strcmp(m_file->platform(), ARCH_NAME) != 0)
	{
		std::stringstream ss;
		ss << "This version of Darling dyld cannot load binaries for " << m_file->platform() << ".";
		throw std::runtime_error(ss.str());
	}
	
	loadSegments();
	transitionState(dyld_image_state_mapped);
	
	if (m_slide > 0)
		rebase();
	transitionState(dyld_image_state_rebased);
	
	readSymbols();
	readExports();

	MachOMgr::instance()->add(this, isMainModule());

	loadDependencies();
	
	performRelocations();
	performBinds();
	transitionState(dyld_image_state_bound);
	transitionState(dyld_image_state_dependents_initialized);
	
	// registerEHSection();
	
	if (isMainModule())
		fillInProgramVars();
	fillInDyldData();
	
	setInitialSegmentProtection();
	setupTLS();
	
	MachOMgr::instance()->notifyAdd(this);
	runInitializers();

	transitionState(dyld_image_state_initialized);
	
	m_file->closeFd();
	
	if (MachOMgr::instance()->printLibraries())
		std::cerr << "dyld: Loaded " << this->path() << std::endl;
}

void MachOObject::unload()
{
	if (!isLoaded())
		throw std::logic_error("Module not loaded");
	
	MachOMgr::instance()->remove(this);
	
	teardownTLS();
	runFinalizers();
	
	if (!MachOMgr::instance()->isDestroying())
	{
		unloadSegments();
		// unregisterEHSection();
	}
	
	for (LoadableObject* dep : m_dependencies)
		dep->delRef();
	
	m_dependencies.clear();
	transitionState(dyld_image_state_terminated);
}

bool MachOObject::isLoaded() const
{
	return !m_mappings.empty();
}

#if (__WORDSIZE == 64)

typedef segment_command_64 Segment;
static const std::vector<Segment*>& getSegments(const MachO& mach) { return mach.segments64(); }

#elif (__WORDSIZE == 32)

typedef segment_command Segment;
static const std::vector<Segment*>& getSegments(const MachO& mach) { return mach.segments(); }

#else
#	error Unknown __WORDSIZE value!
#endif

void MachOObject::loadSegments()
{
	for (Segment* seg : getSegments(*m_file))
	{
		uintptr_t mappingSize;
		int initprot, maxprot;
		void* mappingAddr;
		void* rv;
		int flags = MAP_PRIVATE;
		
		if (strcmp(seg->segname, SEG_PAGEZERO) == 0 || seg->vmsize == 0)
			continue;
		
		assert(seg->vmsize >= seg->filesize);
		
		if (!m_base)
		{
			mappingAddr = (void*) seg->vmaddr;
			
			if (mappingAddr < MachOMgr::instance()->maxAddress())
				mappingAddr = MachOMgr::instance()->maxAddress();
		}
		else
			mappingAddr = (void*) (seg->vmaddr + m_slide);
		
		mappingSize = pageAlign(seg->filesize);
		maxprot = machoProtectionFlagsToMmap(seg->maxprot);
		initprot = machoProtectionFlagsToMmap(seg->initprot);
	
		if (MachOMgr::instance()->printSegments())
			std::cerr << "dyld: Mapping segment " << seg->segname << " from " << m_file->filename() << " to " << mappingAddr << ", slide is 0x" << std::hex << m_slide << std::dec << std::endl;
		
#ifndef __arm__ // All executables on iOS are PIE
		// The first segment can be moved by mmap, but not the following ones
		auto filetype = m_file->header().filetype;
		if (m_base || (!(m_file->header().flags & MH_PIE) && filetype != MH_DYLIB && filetype != MH_BUNDLE))
			flags |= MAP_FIXED;
		else
#endif
		{
			// When letting the system decide where to place the mapping, we need to make sure that the spot chosen
			// is big enough for all segments
			uintptr_t size = getTotalMappingSize();
			rv = ::mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
			if (rv == MAP_FAILED)
			{
				std::stringstream ss;
				ss << "Failed to mmap temporary anonymous range: "  << strerror(errno);
				throw std::runtime_error(ss.str());
			}
			
			flags |= MAP_FIXED;
			mappingAddr = rv;
		}

		rv = ::mmap(mappingAddr, mappingSize, maxprot, flags, m_file->fd(), m_file->offset() + seg->fileoff);
		if (rv == MAP_FAILED)
		{
			std::stringstream ss;
			
			if (errno == EPERM && uintptr_t(mappingAddr) < getMinMappingAddr())
			{
				ss << "This executable is not position independent and your vm.mmap_min_addr is too low to load it. ";
				ss << "As low as " << uintptr_t(mappingAddr) << " is needed.";
			}
			else
				ss << "Failed to mmap '" << m_file->filename() << "': " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
		if (!m_base)
		{
			m_slide = (intptr_t(rv) - intptr_t(seg->vmaddr));
			m_base = rv;
			mappingAddr = rv;
		}
		
		m_mappings.push_back(Mapping { mappingAddr, pageAlign(seg->vmsize), initprot, maxprot });

		if (seg->vmsize > mappingSize)
		{
			// Map empty pages to cover the vmsize range
			mappingAddr = (void*) (seg->vmaddr + m_slide + mappingSize);
			mappingSize = seg->vmsize - mappingSize;
			
			rv = ::mmap(mappingAddr, mappingSize, maxprot, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
			//int err = ::mprotect(mappingAddr, mappingSize, maxprot);
			
			if (rv == MAP_FAILED)
			{
				std::stringstream ss;
				ss << "Failed to mmap anonymous pages for '" << m_file->filename() << "': " << strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}
	}
}

uintptr_t MachOObject::getTotalMappingSize()
{
	uintptr_t minAddr = std::numeric_limits<uintptr_t>::max();
	uintptr_t maxAddr = 0;

	for (Segment* seg : getSegments(*m_file))
	{
		if (strcmp(seg->segname, SEG_PAGEZERO) == 0)
			continue;
		if (seg->vmaddr < minAddr)
			minAddr = seg->vmaddr;
		if (seg->vmaddr+seg->vmsize > maxAddr)
			maxAddr = seg->vmaddr+seg->vmsize;
	}

	if (!maxAddr)
		return 0;
	else
		return maxAddr-minAddr;
}

void MachOObject::setInitialSegmentProtection()
{
	for (const Mapping& map : m_mappings)
	{
		if (::mprotect(map.start, map.size, map.initprot) == -1)
		{
			std::stringstream ss;
			ss << "Failed to mprotect() memory: " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
	}
}

void MachOObject::unloadSegments()
{
	::munmap(baseAddress(), uintptr_t(maxAddress()) - uintptr_t(baseAddress()));
	m_mappings.clear();
}

uintptr_t MachOObject::pageAlign(uintptr_t what)
{
	static const uintptr_t howMuch = getpagesize() - 1;
	return (what + howMuch) & ~howMuch;
}

int MachOObject::machoProtectionFlagsToMmap(int machoFlags)
{
	int prot = 0;
	
	if (machoFlags & VM_PROT_READ)
		prot |= PROT_READ;

	if (machoFlags & VM_PROT_WRITE)
		prot |= PROT_WRITE;
	
	if (machoFlags & VM_PROT_EXECUTE)
		prot |= PROT_EXEC;
	
	return prot;
}

uintptr_t MachOObject::getMinMappingAddr()
{
	static uintptr_t minimum = -1;
	if (minimum == -1)
	{
		std::ifstream f("/proc/sys/vm/mmap_min_addr");

		if (!f.is_open())
			minimum = 0x1000;
		else
			f >> minimum;
	}

	return minimum;
}

void MachOObject::rebase()
{
	for (const MachO::Rebase* rebase : m_file->rebases())
	{
		void* addr = reinterpret_cast<void*>(rebase->vmaddr + m_slide);
		switch (rebase->type)
		{
			case REBASE_TYPE_POINTER:
			{
				uintptr_t* ptr = reinterpret_cast<uintptr_t*>(addr);

				*ptr += m_slide;
				break;
			}
			case REBASE_TYPE_TEXT_ABSOLUTE32:
			{
				uint32_t* ptr = reinterpret_cast<uint32_t*>(addr);

				*ptr = m_file->fixEndian(*ptr);
				*ptr += static_cast<uint32_t>(m_slide);
				break;
			}
			case REBASE_TYPE_TEXT_PCREL32: // TODO: test it
			{
				uint32_t* ptr = reinterpret_cast<uint32_t*>(addr);
				*ptr = m_file->fixEndian(*ptr);
				*ptr = uintptr_t(addr) - 4 - (*ptr);
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

void MachOObject::writeBind(int type, void** ptr, void* newAddr, const std::string& name)
{
	//assert(newAddr != nullptr);
	if (MachOMgr::instance()->printBindings())
		std::cerr << "dyld: Binding " << name << " at " << ptr << ": " << (void*)(*ptr) << " -> " << (void*)newAddr << std::endl;

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
		instr->addr = uint32_t(newAddr) - uint32_t(ptr) - sizeof(jmp_instr);
	}
	else if (type == BIND_TYPE_PCREL)
	{
		*ptr = (void*) (uint32_t(newAddr) - uintptr_t(ptr) - 4);
	}
#endif
}

void MachOObject::loadDependencies()
{
	std::vector<const char*> dylibs = m_file->dylibs();
	std::string path;
	const char* libsystem_dylib = "/usr/lib/libSystem.B.dylib";

	// libSystem MUST always be loaded first
	// This is essentially a hack, but it should be safe.
	LoadableObject* libSystem;

	path = DylibSearch::instance()->resolve(libsystem_dylib, this);
	libSystem = LoadableObject::instantiateForPath(path, this);
	libSystem->load();

	for (std::string dylib : dylibs)
	{
		LoadableObject* dep;
		
		if (dylib.empty())
		{
			ERROR() << "Empty dylib dependency in " << this->path();
			continue;
		}
		
		path = DylibSearch::instance()->resolve(dylib, this);
		if (path.empty())
		{
			std::stringstream ss;
			ss << "dyld: Library not loaded: " << dylib;
			throw std::runtime_error(ss.str());
		}
		
		if (path == libSystem->path())
			dep = libSystem;
		else
			dep = MachOMgr::instance()->lookup(path);

		if (dep != nullptr)
		{
			dep->addRef();
			m_dependencies.push_back(dep);
		}
		else
		{
			dep = LoadableObject::instantiateForPath(path, this);
			dep->load();
			
			m_dependencies.push_back(dep);
		}
	}

	if (libSystem->refCount() > 1)
		libSystem->delRef();
	else
	{
		std::cerr << "Beware! This executable does not link against libSystem. "
			"It is very likely a copy-protected executable which makes direct system calls.\n"
			"Darling currently cannot support that.\n";
	}
}

void MachOObject::readSymbols()
{
	for (const MachO::Symbol& sym : m_file->symbols())
	{
		void* addr = (void*)(sym.addr + m_slide);
		
		m_symbols[addr] = sym.name.c_str();
	}
}

void MachOObject::readExports()
{
	for (MachO::Export* exp : m_file->exports())
	{
		MachO::Export e = *exp;
		e.addr += m_slide;
		
		if (e.resolver && (e.flag & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER)) // EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER support
			e.resolver += m_slide;
		
		m_exports[e.name] = e;
	}
}

void* MachOObject::getExportedSymbol(const std::string& symbolName, bool nonWeakOnly) const
{
	auto it = m_exports.find("_"+symbolName);
	
	if (it != m_exports.end())
	{
		if (nonWeakOnly && (it->second.flag & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION))
			return nullptr;
		
		if (it->second.resolver && (it->second.flag & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER))
		{
			typedef void* (*Resolver)();
			
			Resolver res = Resolver(it->second.resolver);
			it->second.addr = (uint64_t) res();
			it->second.resolver = 0;
		}
		
		return (void*)it->second.addr;
	}
	else
		return nullptr;
}

bool MachOObject::findSymbolInfo(const void* addr, Dl_info* p) const
{
	p->dli_fname = m_file->filename().c_str();
	p->dli_fbase = m_base;
	
	auto it = m_symbols.upper_bound(const_cast<void*>(addr));
	if (it != m_symbols.begin())
	{
		it--;
		p->dli_sname = it->second;
		p->dli_saddr = it->first;
		return true;
	}
	else
	{
		p->dli_saddr = nullptr;
		p->dli_sname = nullptr;
		return false;
	}
}

void MachOObject::fillInProgramVars()
{
	ProgramVars* vars = getProgramVars();
	
	if (vars != nullptr)
	{
		vars->mh = &m_header;
		vars->NXArgcPtr = &g_argc;
		vars->NXArgvPtr = (const char***) &g_argv;
		vars->environPtr = (const char***) &environ;
		vars->__prognamePtr = (const char**) &g_argv[0];
	}
	else
	{
		auto it = m_exports.find("_NXArgc");
		if (it != m_exports.end())
			*((int*)it->second.addr) = g_argc;
		
		it = m_exports.find("_NXArgv");
		if (it != m_exports.end())
			*((char***)it->second.addr) = g_argv;
		
		it = m_exports.find("_environ");
		if (it != m_exports.end())
			*((char***)it->second.addr) = environ;
		
		it = m_exports.find("___progname");
		if (it != m_exports.end())
			*((const char**)it->second.addr) = g_argv[0];
	}
}

void MachOObject::fillInDyldData()
{
	DyldSectData* dyld = (DyldSectData*) getSection("__DATA", "__dyld");
	if (dyld)
		dyld->funcLookupPtr = lookupDyldFunction;
}

bool MachOObject::lookupDyldFunction(const char* name, void** addr)
{
	LOG << "lookupDyldFunction: " << name << std::endl;
	
	*addr = dlsym(RTLD_DEFAULT, name);

	if (!*addr)
		*addr = (void*) (void (*)()) []() { LOG << "Fake dyld function called\n"; };

	return (*addr) != 0;
}


ProgramVars* MachOObject::getProgramVars()
{
	ProgramVars* vars = (ProgramVars*) getSection("__DATA", "__program_vars");
	
	if (!vars)
	{
		uintptr_t size;
		DyldSectData* dyld = (DyldSectData*) getSection("__DATA", "__dyld", &size);
		
		if (dyld && size > sizeof(void*)*2)
			vars = &dyld->vars;
	}
	
	return vars;
}

// These are libgcc functions
extern "C" void __register_frame(void*) __attribute__((weak));
extern "C" void __deregister_frame(void*) __attribute__((weak));

void MachOObject::registerEHSection()
{
	if (!__register_frame)
		return;

	auto eh_frame = m_file->get_eh_frame();
	if (!eh_frame.first)
		return;
	
	try
	{
		EHSection ehSection;
		void* original_eh_data;
		
		// On Darwin/i386, esp and ebp register numbers are swapped
#ifdef __i386__
		static const std::map<int, int> regSwap = {
			std::make_pair<int, int>(4, 5),
			std::make_pair<int, int>(5, 4)
		};
#endif
		
		original_eh_data = (void*) (eh_frame.first + m_slide);
		LOG << "Reworking __eh_frame at " << original_eh_data << std::endl;
		
		ehSection.load(original_eh_data, eh_frame.second);
		
#ifdef __i386__
		ehSection.swapRegisterNumbers(regSwap);
#endif
		
		ehSection.store(&m_reworkedEHData, nullptr);
		
		LOG << "Registering reworked __eh_frame at " << m_reworkedEHData << std::endl;
#ifndef __arm__
		__register_frame(m_reworkedEHData);
#endif
	}
	catch (const std::exception& e)
	{
		ERROR() << "Failed to rework the __eh_frame: " << e.what() << ", exception handling WILL NOT WORK!";
	}
}

void MachOObject::unregisterEHSection()
{
	if (m_reworkedEHData)
	{
#ifndef __arm__
		__deregister_frame(m_reworkedEHData);
#endif
		EHSection::free(m_reworkedEHData);
		m_reworkedEHData = nullptr;
	}
}

void MachOObject::performRelocations()
{
	for (const MachO::Relocation* rel : m_file->relocations())
	{
		uintptr_t* ptr = (uintptr_t*) (uintptr_t(rel->addr) + m_slide);
		uintptr_t symbol;
		uintptr_t value = *ptr;
		int libraryOrdinal;

		libraryOrdinal = usesTwoLevelNamespace() ? rel->ordinal : MachO::Bind::OrdinalSpecialFlatLookup;

		symbol = (uintptr_t) resolveSymbol(rel->name, libraryOrdinal);

		value += symbol;

#ifdef __i386__
		if (rel->pcrel)
		{
			//LOG << "reloc(pcrel): @" << ptr << " " << std::hex << *ptr << " -> " << (value - uintptr_t(ptr) - 4) << std::dec << std::endl;
			writeBind(BIND_TYPE_PCREL, (void**) ptr, (void*) value, rel->name);
			//*ptr = value - uintptr_t(ptr) - 4;
		}
		else
#endif
		{
			//LOG << "reloc: @" << ptr << " " << std::hex << *ptr << " -> " << value << std::dec << std::endl;
			*ptr = value;
			writeBind(BIND_TYPE_POINTER, (void**) ptr, (void*) value, rel->name);
		}
	}
}

void MachOObject::performBinds()
{
	for (MachO::Bind* bind : m_file->binds())
	{
		if (bind->name.empty())
			continue; // WTF...
		
		if (!bind->is_lazy || m_bindAllAtLoad)
			performBind(bind);
	}
}

void* MachOObject::performBind(MachO::Bind* bind)
{
	if (bind->type == BIND_TYPE_POINTER || bind->type == BIND_TYPE_STUB)
	{
		void* addr = nullptr;

		if (bind->is_classic && bind->is_local)
			addr = (void*) bind->value;
		else
		{
			int ordinal = usesTwoLevelNamespace() ? bind->ordinal : MachO::Bind::OrdinalSpecialFlatLookup;
			addr = resolveSymbol(bind->name, ordinal);
		}
		
		if (!addr && !bind->is_weak)
		{
#ifdef HAS_DEBUG_HELPERS
			if (MachOMgr::instance()->ignoreMissingSymbols() && bind->name[0] == '_')
			{
				ERROR().write("Creating a fake implementation for " + bind->name.substr(1));
				addr = MachOMgr::instance()->undefMgr()->generateNew(bind->name.c_str()+1);
				bind->addend = 0;
			}
			else
#endif
			{
				std::stringstream ss;
				ss << "Symbol not found: " << bind->name;
				throw std::runtime_error(ss.str());
			}
		}
		
		if (!bind->is_classic)
			addr = (void*) (uintptr_t(addr) + bind->addend);

#ifdef HAS_DEBUG_HELPERS
		if (addr && MachOMgr::instance()->useTrampolines() && bind->name[0] == '_')
		{
			TrampolineMgr* mgr = MachOMgr::instance()->trampolineMgr();

			addr = mgr->generate(addr, bind->name.c_str()+1);
		}
#endif
		
		if (addr != nullptr)
			writeBind(bind->type, (void**)(bind->vmaddr + m_slide), addr, bind->name);
		
		return addr;
	}
	else
	{
		std::stringstream ss;
		ss << "Unknown bind type: 0x" << std::hex << bind->type;
		throw std::runtime_error(ss.str());
	}
}

void* MachOObject::resolveSymbol(const std::string& name, int libraryOrdinal)
{
	void* addr;
	void* module;
	
	if (name.empty())
		return nullptr;
	if (name == "__mh_dylib_header")
		return m_base;
	
	if (name[0] != '_')
	{
		void* addr = nullptr;
		lookupDyldFunction(name.c_str(), &addr);
		return addr;
	}

	if (MachOMgr::instance()->forceFlatNamespace())
		libraryOrdinal = MachO::Bind::OrdinalSpecialFlatLookup;
	
	switch (libraryOrdinal)
	{
		case MachO::Bind::OrdinalSpecialSelf:
			module = this;
			break;
		case MachO::Bind::OrdinalSpecialExecutable:
			module = DARWIN_RTLD_MAIN_ONLY;
			break;
		case MachO::Bind::OrdinalSpecialFlatLookup:
			module = DARWIN_RTLD_DEFAULT;
			break;
		default:
		{
			if (libraryOrdinal > 0 && libraryOrdinal <= m_dependencies.size())
				module = m_dependencies[libraryOrdinal-1];
			else
				return nullptr;
		}
	}

	return __darwin_dlsym(module, name.c_str() + 1);
}

void MachOObject::runInitializers()
{
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	ProgramVars* pvars = nullptr;
	
	if (MachOMgr::instance()->mainModule() != nullptr)
		pvars = MachOMgr::instance()->mainModule()->getProgramVars();
	else
		; // TODO: provide an alternative ProgramVars instance
	
	typedef void (*Initializer)(int argc, char** argv, char** envp, char** apple, ProgramVars* progvars);
	
	for (uint64_t offset : m_file->init_funcs())
	{
		Initializer* init = (Initializer*)(offset + m_slide);
		
		if (MachOMgr::instance()->printInitializers())
			std::cerr << "dyld: Running initializer at " << (void*)*init << std::endl;
		
		(*init)(g_argc, g_argv, environ, apple, pvars);
	}
}

void MachOObject::runFinalizers()
{
	typedef void (*Finalizer)();
	
	for (uint64_t offset : m_file->exit_funcs())
	{
		Finalizer* fini = (Finalizer*)(offset + m_slide);
		(*fini)();
	}
}

void MachOObject::detectAbsolutePath()
{
	char path[PATH_MAX];
	size_t pos;
	
	realpath(m_file->filename().c_str(), path);
	
	m_absolutePath = path;
	m_absolutePathDir = dirname(path);

	pos = m_absolutePath.rfind('/');

	if (pos == std::string::npos)
		m_name = m_absolutePath;
	else
		m_name = m_absolutePath.substr(pos+1);
}

void MachOObject::setCommandLine(int argc, char** argv, char** envp)
{
	if (argv[argc] != nullptr)
		throw std::invalid_argument("argv is not nullptr terminated");
	if (!isMainModule())
		throw std::logic_error("setCommandLine() used on a non-main module");
	
	m_argc = argc;
	m_argv = argv;
	m_envp = envp;
	
	// FIXME: this needs to go away
	g_argc = argc;
	g_argv = argv;
}

void MachOObject::commandLine(int* argc, char*** argv, char*** envp)
{
	if (argc)
		*argc = m_argc;
	if (argv)
		*argv = m_argv;
	if (envp)
		*envp = m_envp;
}

void MachOObject::commandLine(int** argc, char**** argv, char**** envp)
{
	if (argc)
		*argc = &m_argc;
	if (argv)
		*argv = &m_argv;
	if (envp)
		*envp = &m_envp;
}

void MachOObject::run()
{
	if (!isMainModule())
		throw std::runtime_error("Tried to run a file that is not the main module");
	
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	void (*pAtExit)(void(*)());

	pAtExit = reinterpret_cast<void(*)(void(*)())>(MachOMgr::instance()->getExportedSymbol("atexit"));

	if (pAtExit != nullptr)
	{
		pAtExit([]() {
				MachOMgr::instance()->atexit();
		});
	}

#ifdef HAS_DEBUG_HELPERS
	if (MachOMgr::instance()->useTrampolines())
		MachOMgr::instance()->trampolineMgr()->printPath();
#endif
	
	if (m_file->entry())
	{
		LOG << "Jumping to " << (void*)m_file->entry() << "..." << std::endl;
		
		jumpToStart();
	}
	else if (m_file->main())
	{
		LOG << "Running main at " << (void*) m_file->main() << "...\n";
		
		int (*pMain)(int, char**, char**, char**) = reinterpret_cast<int (*)(int, char**, char**, char**)>(m_file->main() + m_slide);
		void (*pExit)(int);

		pExit = reinterpret_cast<void(*)(int)>(MachOMgr::instance()->getExportedSymbol("exit"));
		
		int rv = pMain(m_argc, m_argv, m_envp, apple);

		if (pExit != nullptr)
			pExit(rv);
		else
			abort();

		__builtin_unreachable();
	}
	else
		throw std::runtime_error("No entry point found");
}

void MachOObject::jumpToStart()
{
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	uintptr_t entry = m_file->entry() + m_slide;
	void** sp;
	int pushCount = 0;
	
#ifdef __x86_64__
#	define GETSP(ptr) __asm__ volatile("movq %%rsp, %0" : "=r"(ptr) ::)
#	define JUMPX(pushCount, addr) __asm__ volatile("sub %1, %%rsp; jmpq *%0" :: "m"(addr), "r"(pushCount * sizeof(void*)) :)
#elif defined(__i386__)
#	define GETSP(ptr) __asm__ volatile("movl %%esp, %0" : "=m"(ptr) ::)
#	define JUMPX(pushCount, addr) __asm__ volatile("sub %1, %%esp; jmp *%0" :: "m"(addr), "r"(pushCount * sizeof(void*)) :)
#elif defined(__arm__)
#	define GETSP(ptr) __asm__ volatile("mov %0, sp" : "=r"(ptr) ::)
#	define JUMPX(pushCount, addr) __asm__ volatile("sub sp, %1; bx %0" :: "r"(addr), "r"(pushCount * sizeof(void*)) :)
#else 
#       error Unsupported platform!
#endif
	
	GETSP(sp);
	sp--;

	for (int i = std::char_traits<char*>::length(apple)-1; i >= 0; i--)
		*(sp-(pushCount++)) = apple[i];

	for (int i = std::char_traits<char*>::length(m_envp)-1; i >= 0; i--)
		*(sp-(pushCount++)) = m_envp[i];

	for (int i = m_argc; i >= 0; i--)
		*(sp-(pushCount++)) = m_argv[i];

	*(sp-(pushCount++)) = (void*) uintptr_t(m_argc);
	JUMPX(pushCount, entry);

	__builtin_unreachable();
}


void* MachOObject::getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize)
{
	LOG << "Looking for section " << sectionName << " in segment " << segmentName << std::endl;

	for (const MachO::Section& sect : m_file->sections())
	{
		if ((segmentName.empty() || sect.segment == segmentName) && sect.section == sectionName)
		{
			void* addr;

			if (sectionSize)
				*sectionSize = sect.size;

			addr = (void*) (sect.addr + m_slide);

			LOG << "...found at " << addr << std::endl;

			return addr;
		}
	}

	LOG << "...not found\n";
	return nullptr;
}

void MachOObject::setupTLS()
{
#ifdef HAS_TLS_SUPPORT
	std::vector<tlv_descriptor*> descriptors;
	const std::vector<MachO::TLVSection>& tlvSections = m_file->tlv_sections();
	std::vector<void*> initializers;
	
	if (tlvSections.empty())
		return;
	
	for (MachO::TLVSection sect : tlvSections)
	{
		tlv_descriptor* des = reinterpret_cast<tlv_descriptor*>(sect.firstDescriptor+m_slide);
		for (size_t i = 0; i < sect.count; i++)
			descriptors.push_back(&des[i]);
	}
	
	for (uint64_t offset : m_file->tlv_init_funcs())
		initializers.push_back((void*) (offset+m_slide));
	
	Darling::TLSSetup(this, descriptors, initializers, (void*) (m_file->tlv_initial_values().first+m_slide), m_file->tlv_initial_values().second);
#endif
}

void MachOObject::teardownTLS()
{
#ifdef HAS_TLS_SUPPORT
	Darling::TLSTeardown(this);
#endif
}

void* MachOObject::doLazyBind(uintptr_t lazyOffset)
{
	for (MachO::Bind* bind : m_file->binds())
	{
		if (bind->is_lazy && bind->offset == lazyOffset)
		{
			try
			{
				return performBind(bind);
			}
			catch (const std::exception& e)
			{
				std::cerr << "dyld_stub_binder_fixup(): Failed to resolve the symbol: " << e.what() << std::endl;
				abort();
			}
		}
	}

	std::cerr << "dyld_stub_binder_fixup(): Lazy bind not found for offset " << std::hex << lazyOffset << std::dec << std::endl;
	abort();
}

const char* MachOObject::name() const
{
	return m_name.c_str();
}

void* MachOObject::getExportedSymbolRecursive(const std::string& symbolName) const
{
	std::set<const LoadableObject*> seen;
	void* addr;

	seen.insert(this);

	if ((addr = getExportedSymbol(symbolName, false)))
		return addr;

	return getExportedSymbolRecursive(symbolName, seen);
}

void* MachOObject::getExportedSymbolRecursive(const std::string& symbolName, std::set<const LoadableObject*>& visited) const
{
	std::set<const MachOObject*> recurse;

	for (const LoadableObject* dep : m_dependencies)
	{
		void* addr;

		if (visited.find(dep) != visited.end())
			continue;

		visited.insert(dep);

		if ((addr = dep->getExportedSymbol(symbolName, false)))
			return addr;

		if (const MachOObject* obj = dynamic_cast<const MachOObject*>(dep))
			recurse.insert(obj);
	}

	for (const MachOObject* obj : recurse)
	{
		void* addr;

		if ((addr = obj->getExportedSymbolRecursive(symbolName, visited)))
			return addr;
	}

	return nullptr;
}

void* MachOObject::dyld_stub_binder_fixup(MachOObject** obj, uintptr_t lazyOffset)
{
	if (!*obj)
	{
		LOG << "Finding MachOObject for address " << obj << std::endl;
		*obj = MachOMgr::instance()->objectForAddress((void*) obj);

		if (!*obj)
		{
			std::cerr << "dyld_stub_binder_fixup(): Mach-O object not found for " << obj << std::endl;
			abort();
		}
	}

	return (*obj)->doLazyBind(lazyOffset);
}

void MachOObject::atExit()
{
	runFinalizers();
}

} // namespace Darling
