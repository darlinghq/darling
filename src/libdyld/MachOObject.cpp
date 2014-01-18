#include "dl_public.h"
#include "MachOObject.h"
#include <climits>
#include <cstring>
#include <cassert>
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

// FIXME: this needs to go away
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
	m_rpaths.insert(m_rpaths.begin(), m_file->rpaths().begin(), m_file->rpaths().end());
	m_bindAllAtLoad = MachOMgr::instance()->bindAtLaunch();
}

MachOObject::~MachOObject()
{
	LOG << "deleting " << name() << " at " << this << std::endl;
	delete m_file;
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
	
	if (m_slide > 0)
		rebase();
	
	readSymbols();
	readExports();

	MachOMgr::instance()->add(this, isMainModule());

	loadDependencies();
	
	performRelocations();
	performBinds();
	
	registerEHSection();
	
	if (isMainModule())
		fillInProgramVars();
	fillInDyldData();
	
	setInitialSegmentProtection();
	setupTLS();
	
	MachOMgr::instance()->notifyAdd(this);
	runInitializers();
	
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
	unloadSegments();
	unregisterEHSection();
	
	for (LoadableObject* dep : m_dependencies)
		dep->delRef();
	
	m_dependencies.clear();
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
		
		if (strcmp(seg->segname, SEG_PAGEZERO) == 0)
			continue;
		
		assert(seg->vmsize >= seg->filesize);
		
		if (!m_base)
		{
			m_base = (void*) seg->vmaddr;
			
			if (!m_base)
				m_base = MachOMgr::instance()->maxAddress();
			
			if (m_base && m_base < (void*)getMinMappingAddr())
				m_base = (void*) getMinMappingAddr();
			
			if (!m_base)
			{
				// This is normally used only for:
				// a) DYLD_PRELOAD
				// b) Standalone use of libdyld
			
	#if (__WORDSIZE == 64)
				m_base = (void*) 0x200000000L;
	#else
				m_base = (void*) 0x1000000;
	#endif
			}
			
			if ((void*)seg->vmaddr != m_base && !m_slide)
				m_slide = uintptr_t(m_base) - seg->vmaddr;
		}
		
		mappingSize = pageAlign(seg->filesize);
		mappingAddr = (void*) (seg->vmaddr + m_slide);
		maxprot = machoProtectionFlagsToMmap(seg->maxprot);
		initprot = machoProtectionFlagsToMmap(seg->initprot);
	
		if (MachOMgr::instance()->printSegments())
			std::cerr << "dyld: Mapping segment " << seg->segname << " from " << m_file->filename() << " to " << mappingAddr << ", slide is 0x" << std::hex << m_slide << std::dec << std::endl;
		
		rv = ::mmap(mappingAddr, mappingSize, maxprot, MAP_FIXED | MAP_PRIVATE, m_file->fd(), m_file->offset() + seg->fileoff);
		if (rv == MAP_FAILED)
		{
			std::stringstream ss;
			ss << "Failed to mmap '" << m_file->filename() << "': " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
		
		m_mappings.push_back(Mapping { mappingAddr, pageAlign(seg->vmsize), initprot, maxprot });
		
		if (seg->vmsize > mappingSize)
		{
			// Map empty pages to cover the vmsize range
			mappingAddr = (void*) (seg->vmaddr + m_slide + mappingSize);
			mappingSize = seg->vmsize - mappingSize;
			
			rv = ::mmap(mappingAddr, mappingSize, maxprot, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
			
			if (rv == MAP_FAILED)
			{
				std::stringstream ss;
				ss << "Failed to mmap anonymous pages for '" << m_file->filename() << "': " << strerror(errno);
				throw std::runtime_error(ss.str());
			}
		}
	}
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
	for (std::string dylib : m_file->dylibs())
	{
		LoadableObject* dep;
		
		if (dylib.empty())
		{
			ERROR() << "Empty dylib dependency in " << path();
			continue;
		}
		
		std::string path = DylibSearch::instance()->resolve(dylib, this);
		if (path.empty())
		{
			std::stringstream ss;
			ss << "dyld: Library not loaded: " << dylib;
			throw std::runtime_error(ss.str());
		}
		
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
		*vars->NXArgcPtr = g_argc;
		*vars->NXArgvPtr = (const char**) g_argv;
		*vars->environPtr = (const char**) environ;
		*vars->__prognamePtr = g_argv[0];
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
extern "C" void __register_frame(void*);
extern "C" void __deregister_frame(void*);

void MachOObject::registerEHSection()
{
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
		__register_frame(m_reworkedEHData);
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
		__deregister_frame(m_reworkedEHData);
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

		symbol = (uintptr_t) resolveSymbol(rel->name);

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
			addr = resolveSymbol(bind->name);
		
		if (!addr && !bind->is_weak)
		{
			if (MachOMgr::instance()->ignoreMissingSymbols() && bind->name[0] == '_')
			{
				ERROR().write("Creating a fake implementation for " + bind->name.substr(1));
				addr = MachOMgr::instance()->undefMgr()->generateNew(bind->name.c_str()+1);
				bind->addend = 0;
			}
			else
			{
				std::stringstream ss;
				ss << "Symbol not found: " << bind->name;
				throw std::runtime_error(ss.str());
			}
		}
		
		if (!bind->is_classic)
			addr = (void*) (uintptr_t(addr) + bind->addend);
		
		if (addr && MachOMgr::instance()->useTrampolines() && bind->name[0] == '_')
		{
			TrampolineMgr* mgr = MachOMgr::instance()->trampolineMgr();

			addr = mgr->generate(addr, bind->name.c_str()+1);
		}
		
		//if (addr != nullptr)
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

void* MachOObject::resolveSymbol(const std::string& name)
{
	void* addr;
	
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
	
	return __darwin_dlsym(DARWIN_RTLD_DEFAULT, name.c_str() + 1);
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

	if (MachOMgr::instance()->useTrampolines())
		MachOMgr::instance()->trampolineMgr()->printPath();
	
	if (m_file->entry())
	{
		LOG << "Jumping to " << (void*)m_file->entry() << "..." << std::endl;
		
		jumpToStart();
	}
	else if (m_file->main())
	{
		LOG << "Running main at " << (void*) m_file->main() << "...\n";
		
		int (*pMain)(int, char**, char**, char**) = reinterpret_cast<int (*)(int, char**, char**, char**)>(m_file->main() + m_slide);
		
		int rv = pMain(m_argc, m_argv, m_envp, apple);
		exit(rv);
	}
	else
		throw std::runtime_error("No entry point found");
}

void MachOObject::jumpToStart()
{
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	uintptr_t entry = m_file->entry() + m_slide;
	
#ifdef __x86_64__
#	define PUSH(val) __asm__ volatile("pushq %0" :: "r"(uint64_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmpq *%0" :: "m"(addr) :)
#elif defined(__i386__)
#	define PUSH(val) __asm__ volatile("pushl %0" :: "r"(uint32_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmp *%0" :: "r"(addr) :)
#else
#	error Unsupported platform!
#endif

	for (int i = std::char_traits<char*>::length(apple)-1; i >= 0; i--)
		PUSH(apple[i]);

	for (int i = std::char_traits<char*>::length(m_envp)-1; i >= 0; i--)
		PUSH(m_envp[i]);

	for (int i = m_argc; i >= 0; i--)
		PUSH(m_argv[i]);

	PUSH(m_argc);
	JUMP(entry);

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
}

void MachOObject::teardownTLS()
{
	Darling::TLSTeardown(this);
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


} // namespace Darling
