#include "MachOObject.h"
#include <climits>
#include <cstring>
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
#include "ld.h"

// FIXME: this needs to go away
extern "C" int g_argc asm("NXArgc");
extern "C" char** g_argv asm("NXArgv");

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

MachOObject::MachOObject(const std::string& path, const char* arch)
{
	m_file = MachO::readFile(path, arch);
	
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
	m_bindAllAtLoad = MachOMgr::instance()->bindAtLaunch();
}

MachOObject::~MachOObject()
{
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
	m_base = MachOMgr::instance()->maxAddress();
	
	loadSegments();
	
	if (m_slide > 0)
		rebase();
	
	readSymbols();
	loadDependencies();
	
	performRelocations();
	performBinds();
	
	registerEHSection();
	
	if (isMainModule())
		fillInProgramVars();
	fillInDyldData();
	
	runInitializers();
	
	MachOMgr::instance()->add(this, isMainModule());
	m_file->close();
	
	if (MachOMgr::instance()->printLibraries())
		std::cerr << "dyld: Loaded " << this->path() << std::endl;
}

void MachOObject::unload()
{
	runFinalizers();
	unloadSegments();
	unregisterEHSection();
	
	for (LoadableObject* dep : m_dependencies)
		dep->delRef();
	
	m_dependencies.clear();
	
	MachOMgr::instance()->remove(this);
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
			m_base = (void*) seg->vmaddr;
		
		if (!m_base)
			throw std::runtime_error("Cannot map to 0x0");
		
		if ((void*)seg->vmaddr < m_base)
			m_slide = uintptr_t(m_base) - seg->vmaddr;
		
		mappingSize = pageAlign(seg->filesize);
		mappingAddr = (void*) (seg->vmaddr + m_slide);
		maxprot = machoProtectionFlagsToMmap(seg->maxprot);
		initprot = machoProtectionFlagsToMmap(seg->initprot);
		
		checkMappingAddr(mappingAddr);
		
		LOG << "Mapping segment " << seg->segname << " from " << m_file->filename() << " to " << mappingAddr << ", slide is " << m_slide << std::endl;
		
		rv = ::mmap(mappingAddr, mappingSize, maxprot, MAP_FIXED | MAP_PRIVATE, m_file->fd(), m_file->offset() + seg->fileoff);
		if (rv == MAP_FAILED)
		{
			std::stringstream ss;
			ss << "Failed to mmap '" << m_file->filename() << "': " << strerror(errno);
			throw std::runtime_error(ss.str());
		}
		
		m_mappings.push_back(Mapping { mappingAddr, seg->vmsize, initprot, maxprot });
		
		if (seg->vmsize < mappingSize)
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

void MachOObject::checkMappingAddr(void* addr)
{
	static intptr_t minimum = -1;
	if (minimum == -1)
	{
		std::ifstream f("/proc/sys/vm/mmap_min_addr");

		if (!f.is_open())
			minimum = 0;
		else
			f >> minimum;
	}

	if (addr < (void*)minimum)
	{
		std::stringstream ss;
		ss << "Your vm.mmap_min_addr is too low for this application to be loaded. ";
		ss << "Try running `sysctl -w vm.mmap_min_addr=\"" << addr << "\"'";
		throw std::runtime_error(ss.str());
	}
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

void MachOObject::writeBind(int type, uintptr_t* ptr, uintptr_t newAddr)
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

void MachOObject::loadDependencies()
{
	// TODO
}

void MachOObject::readSymbols()
{
	for (const MachO::Symbol& sym : m_file->symbols())
	{
		void* addr = (void*)(sym.addr + m_slide);
		
		m_symbols[addr] = sym.name.c_str();
	}
}


void* MachOObject::getExportedSymbol(const std::string& symbolName) const
{
	auto it = m_exports.find(symbolName);
	
	if (it != m_exports.end())
		return it->second;
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
			*((int*)it->second) = g_argc;
		
		it = m_exports.find("_NXArgv");
		if (it != m_exports.end())
			*((char***)it->second) = g_argv;
		
		it = m_exports.find("_environ");
		if (it != m_exports.end())
			*((char***)it->second) = environ;
		
		it = m_exports.find("___progname");
		if (it != m_exports.end())
			*((const char**)it->second) = g_argv[0];
	}
}

void MachOObject::fillInDyldData()
{
	DyldSectData* dyld = (DyldSectData*) getSection("__DATA", "__dyld");
	dyld->funcLookupPtr = lookupDyldFunction;
}

bool MachOObject::lookupDyldFunction(const char* name, void** addr)
{
	LOG << "lookupDyldFunction: " << name << std::endl;
	
	static void* self = dlopen(0, 0);
	
	*addr = dlsym(self, name);

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
		
		if (size > sizeof(void*)*2)
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
		__register_frame(m_reworkedEHData); // TODO: free when unloading the image
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
	// TODO
	return nullptr;
}

void* MachOObject::resolveSymbol(const std::string& name)
{
	void* addr;
	
	if (!name.empty())
		return nullptr;
	
	if (name[0] != '_')
	{
		void* addr = nullptr;
		lookupDyldFunction(name.c_str(), &addr);
		return addr;
	}
	
#ifndef __x86_64__
	if (name.size() > 10 && name.compare(name.size()-10, 10, "$UNIX_2003") == 0)
	{
		name.resize(name.size() - 10);
	}
#endif

	addr = __darwin_dlsym(DARWIN_RTLD_DEFAULT, name.c_str() + 1);
	
	if (!addr && MachOMgr::instance()->ignoreMissingSymbols())
	{
		// TODO:
	}
	
	if (addr && MachOMgr::instance()->useTrampolines())
	{
		// TODO:
	}
	
	return addr;
}

void MachOObject::runInitializers()
{
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	ProgramVars* pvars = MachOMgr::instance()->mainModule()->getProgramVars();
	
	typedef void (*Initializer)(int argc, char** argv, char** envp, char** apple, ProgramVars* progvars);
	
	for (uint64_t offset : m_file->init_funcs())
	{
		Initializer init = Initializer(offset + m_slide);
		
		if (MachOMgr::instance()->printInitializers())
			std::cerr << "dyld: Running initializer at " << (void*)init << std::endl;
		
		init(g_argc, g_argv, environ, apple, pvars);
	}
}

void MachOObject::runFinalizers()
{
	typedef void (*Finalizer)();
	
	for (uint64_t offset : m_file->exit_funcs())
	{
		Finalizer fini = Finalizer(offset + m_slide);
		fini();
	}
}

void MachOObject::detectAbsolutePath()
{
	char path[PATH_MAX];
	
	realpath(m_file->filename().c_str(), path);
	
	m_absolutePath = path;
	m_absolutePathDir = dirname(path);
}

void MachOObject::setCommandLine(int argc, char** argv, char** envp)
{
	assert(m_argv[m_argc] == nullptr);
	
	m_argc = argc;
	m_argv = argv;
	m_envp = envp;
}

void MachOObject::commandLine(int* argc, char*** argv, char*** envp)
{
	*argc = m_argc;
	*argv = m_argv;
	*envp = m_envp;
}

void MachOObject::run()
{
	if (!isMainModule())
		throw std::runtime_error("Tried to run a file that is not the main module");
	
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	
	if (m_file->entry())
	{
		LOG << "Jumping to " << (void*)m_file->entry() << "..." << std::endl;
		
		jumpToStart();
	}
	else if (m_file->main())
	{
		LOG << "Running main at " << (void*) m_file->main() << "...\n";
		
		int (*pMain)(int, char**, char**, char**) = reinterpret_cast<int (*)(int, char**, char**, char**)>(m_file->main());
		
		int rv = pMain(m_argc, m_argv, m_envp, apple);
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

void MachOObject::jumpToStart()
{
	char* apple[2] = { const_cast<char*>(m_absolutePath.c_str()), nullptr };
	uintptr_t entry = m_file->entry();
	
#ifdef __x86_64__
#	define PUSH(val) __asm__ volatile("pushq %0" :: "r"(uint64_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmpq *%0" :: "m"(addr) :)
#elif defined(__i386__)
#	define PUSH(val) __asm__ volatile("pushl %0" :: "r"(uint32_t(val)) :)
#	define JUMP(addr) __asm__ volatile("jmp *%0" :: "r"(addr) :)
#endif

	for (int i = findLast(apple); i >= 0; i--)
		PUSH(apple[i]);

	for (int i = findLast(m_envp); i >= 0; i--)
		PUSH(m_envp[i]);

	for (int i = m_argc; i >= 0; i--)
		PUSH(m_argv[i]);

	PUSH(m_argc);
	JUMP(entry);
}


void* MachOObject::getSection(const std::string& segmentName, const std::string& sectionName, uintptr_t* sectionSize)
{
	for (const MachO::Section& sect : m_file->sections())
	{
		if (sect.segment == segmentName && sect.section == sectionName)
		{
			if (sectionSize)
				*sectionSize = sect.size;
			return (void*) (sect.addr + m_slide);
		}
	}
	return nullptr;
}

