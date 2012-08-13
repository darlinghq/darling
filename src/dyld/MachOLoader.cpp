#include "MachOLoader.h"
#include "MachO.h"
#include "ld.h"
#include "log.h"
#include "trace.h"
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <sys/mman.h>

#define FLAGS_TRACE_FUNCTIONS 0

FileMap g_file_map;
static std::map<std::string, std::string> g_rename;
static std::vector<std::string> g_bound_names;
static std::set<std::string> g_no_trampoline;

extern char g_darwin_executable_path[PATH_MAX];

static void initNoTrampoline() __attribute__((constructor))
{
#define NO_TRAMPOLINE(name) g_no_trampoline.insert(#name);
#include "no_trampoline.tab"
#undef NO_TRAMPOLINE
}

static void* undefinedFunction()
{
	fprintf(stderr, "Undefined function called, returning NULL\n");
	//abort();
	return 0;
}

static void doNothing()
{
}

static bool lookupDyldFunction(const char* name, uint64_t* addr)
{
	LOG << "lookupDyldFunction: " << name <<std::endl;
	*addr = (int64_t)&doNothing;
	return true;
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
: m_last_addr(0)
{
	if (FLAGS_TRACE_FUNCTIONS)
	{
		// Push all arguments into stack.

		// push %rax
		pushTrampolineCode(0x50);
		// push %rdi
		pushTrampolineCode(0x57);
		// push %rsi
		pushTrampolineCode(0x56);
		// push %rdx
		pushTrampolineCode(0x52);
		// push %rcx
		pushTrampolineCode(0x51);
		// push %r8
		pushTrampolineCode(0x5041);
		// push %r9
		pushTrampolineCode(0x5141);

		// push %xmm0..%xmm7
		for (int i = 0; i < 8; i++)
		{
			// sub $8, %rsp
			pushTrampolineCode(0x08ec8348);

			// movq %xmmN, (%rsp)
			pushTrampolineCode(0xd60f66);
			pushTrampolineCode(4 + i * 8);
			pushTrampolineCode(0x24);
		}

		// mov %r10, %rdi
		pushTrampolineCode(0xd7894c);

		// mov $func, %rdx
		pushTrampolineCode(0xba48);
		pushTrampolineCode64((unsigned long long)(void*)&dumpInt);

		// call *%rdx
		pushTrampolineCode(0xd2ff);

		// pop %xmm7..%xmm0
		for (int i = 7; i >= 0; i--)
		{
			// movq (%rsp), %xmmN
			pushTrampolineCode(0x7e0ff3);
			pushTrampolineCode(4 + i * 8);
			pushTrampolineCode(0x24);

			// add $8, %rsp
			pushTrampolineCode(0x08c48348);
		}

		// pop %r9
		pushTrampolineCode(0x5941);
		// pop %r8
		pushTrampolineCode(0x5841);
		// pop %rcx
		pushTrampolineCode(0x59);
		// pop %rdx
		pushTrampolineCode(0x5a);
		// pop %rsi
		pushTrampolineCode(0x5e);
		// pop %rdi
		pushTrampolineCode(0x5f);
		// pop %rax
		pushTrampolineCode(0x58);

		// ret
		pushTrampolineCode(0xc3);
	}
}

void MachOLoader::loadDylibs(const MachO& mach)
{
	for (size_t i = 0; i < mach.dylibs().size(); i++)
	{
		std::string dylib = mach.dylibs()[i];

		// __darwin_dlopen checks if already loaded
		// automatically adds a reference if so
		if (!__darwin_dlopen(dylib.c_str(), DARWIN_RTLD_GLOBAL))
			throw std::runtime_error("Cannot load " + dylib + "!");
	}
}


void MachOLoader::loadSegments(const MachO& mach, intptr* slide, intptr* base)
{
	*base = 0;
	--*base;

	const std::vector<Segment*>& segments = getSegments(mach);
	for (size_t i = 0; i < segments.size(); i++)
	{
		Segment* seg = segments[i];
		const char* name = seg->segname;
		if (!strcmp(name, SEG_PAGEZERO))
			continue;

		LOG << seg->segname << ": "
			<< "fileoff=" << seg->fileoff
			<< ", vmaddr=" << seg->vmaddr <<std::endl;

		int prot = 0;
		if (seg->initprot & VM_PROT_READ)
			prot |= PROT_READ;
		
		if (seg->initprot & VM_PROT_WRITE)
			prot |= PROT_WRITE;
		
		if (seg->initprot & VM_PROT_EXECUTE)
			prot |= PROT_EXEC;
		

		intptr filesize = alignMem(seg->filesize, 0x1000);
		intptr vmaddr = seg->vmaddr + *slide;
		
		if (vmaddr < m_last_addr)
		{
			LOG << "will rebase: filename=" << mach.filename()
				<< ", vmaddr=" << (void*)vmaddr
				<< ", last_addr=" << (void*)last_addr_ <<std::endl;
			assert(i == 0);
			vmaddr = last_addr_;
			*slide = vmaddr - seg->vmaddr;
		}
		*base = std::min(*base, vmaddr);

		intptr vmsize = alignMem(seg->vmsize, 0x1000);
		LOG << "mmap(file) " << mach.filename() << ' ' << name
			<< ": " << (void*)vmaddr << "-" << (void*)(vmaddr + filesize)
			<< " offset=" << mach.offset() + seg->fileoff <<std::endl;
		
		if (filesize == 0)
			continue;
		
		void* mapped = ::mmap((void*)vmaddr, filesize, prot,
							MAP_PRIVATE | MAP_FIXED,
							mach.fd(), mach.offset() + seg->fileoff);
		
		if (mapped == MAP_FAILED)
			err(1, "%s mmap(file) failed", mach.filename().c_str());

		if (vmsize != filesize)
		{
			LOG << "mmap(anon) " << mach.filename() << ' ' << name
				<< ": " << (void*)(vmaddr + filesize) << "-"
				<< (void*)(vmaddr + vmsize)
				<<std::endl;
				
			assert(vmsize > filesize);
			
			void* mapped = ::mmap((void*)(vmaddr + filesize),
								vmsize - filesize, prot,
								MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
								0, 0);
								
			if (mapped == MAP_FAILED)
			{
				// TODO: insert a suggestion for a fix on 32bit systems
				err(1, "%s mmap(anon) failed", mach.filename().c_str());
			}
		}

		m_last_addr = std::max(m_last_addr, (intptr)vmaddr + vmsize);
	}
}

void MachOLoader::doRebase(const MachO& mach, intptr slide)
{
	for (size_t i = 0; i < mach.rebases().size(); i++)
	{
		const MachO::Rebase& rebase = *mach.rebases()[i];
		switch (rebase.type)
		{
			case REBASE_TYPE_POINTER:
			{
				char** ptr = (char**)(rebase.vmaddr + slide);
				LOG << "rebase: " << i << ": " << (void*)rebase.vmaddr << ' '
					<< (void*)*ptr << " => "
					<< (void*)(*ptr + slide) << " @" << ptr <<std::endl;
				*ptr += slide;
				break;
			}

			default:
			{
				std::stringstream ss;
				ss << "Unknown rebase type: " << rebase.type;
				
				throw std::runtime_error(ss.str());
			}
		}
	}
}

void MachOLoader::loadInitFuncs(const MachO& mach, intptr slide)
{
	for (size_t i = 0; i < mach.init_funcs().size(); i++)
	{
		intptr addr = mach.init_funcs()[i] + slide;
		LOG << "Registering init func " << (void*)addr
			<< " from " << mach.filename() << std::endl;
		m_init_funcs.push_back(addr);
	}
}

void MachOLoader::doBind(const MachO& mach, intptr slide)
{
	std::string last_weak_name;
	char* last_weak_sym = 0;
	size_t seen_weak_bind_index = 0;
	size_t seen_weak_binds_orig_size = m_seen_weak_binds.size();

	unsigned int common_code_size = (unsigned int)m_trampoline.size();
	// Ensure that we won't change the address.
	m_trampoline.reserve(common_code_size +
						(1 + 6 + 5 + 10 + 3 + 2 + 1) * mach.binds().size());
	g_bound_names.resize(mach.binds().size());

	for (size_t i = 0; i < mach.binds().size(); i++)
	{
		MachO::Bind* bind = mach.binds()[i];
		if (bind->name[0] != '_')
		{
			LOG << bind->name << ": skipping" << endl;
			continue;
		}

		if (bind->type == BIND_TYPE_POINTER)
		{
			string name = bind->name.substr(1);
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
					if (seen_weak_bind_index != seen_weak_binds_orig_size && !strcmp(seen_weak_binds_[seen_weak_bind_index].first.c_str(), name.c_str()))
					{
						last_weak_sym = sym =
						seen_weak_binds_[seen_weak_bind_index].second;
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
							const Exports::const_iterator export_found = exports_.find(bind->name);
							if (export_found != exports_.end())
								*ptr = last_weak_sym = (char*)export_found->second.addr;
							else
								last_weak_sym = (char*)*ptr;
						}
					}
					
					seen_weak_binds_.push_back(make_pair(name, last_weak_sym));
					while (seen_weak_bind_index != seen_weak_binds_orig_size && strcmp(seen_weak_binds_[seen_weak_bind_index].first.c_str(), name.c_str()) <= 0)
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
		
				sym = reinterpret_cast<char*>(__darwin_dlsym(DARWIN_RTLD_GLOBAL, name.c_str()));
				if (!sym)
				{
					std::stringstream ss;
					ss << "Undefined symbol: " << name;
					throw std::runtime_error(ss);
				}
				
				sym += bind->addend;
			}

			LOG << "bind " << name << ": "
				<< *ptr << " => " << (void*)sym << " @" << ptr << endl;

			if (FLAGS_TRACE_FUNCTIONS && !g_no_trampoline.count(name))
			{
				LOG << "Generating trampoline for " << name << "..." << endl;

				*ptr = &m_trampoline[0] + m_trampoline.size();
				g_bound_names[i] = name;

				// push %rax  ; to adjust alignment for sse
				pushTrampolineCode(0x50);

				// mov $i, %r10d
				pushTrampolineCode(0xba41);
				pushTrampolineCode32((unsigned int)i);

				// call &m_trampoline[0]
				pushTrampolineCode(0xe8);
				pushTrampolineCode32((unsigned int)(-4-m_trampoline.size()));

				// mov $sym, %r10
				pushTrampolineCode(0xba49);
				pushTrampolineCode64((unsigned long long)(void*)sym);
				// call *%r10
				pushTrampolineCode(0xd2ff41);

				// pop %r10
				pushTrampolineCode(0x5a41);

				// ret
				pushTrampolineCode(0xc3);
			}
			else
			{
				*ptr = sym;
			}
		}
		else
		{
			std::stringstream ss;
			ss << "Unknown bind type: " << bind->type;
			throw std::runtime_exception(ss.str());
		}
	}

	std::inplace_merge(m_seen_weak_binds.begin(), m_seen_weak_binds.begin() + seen_weak_binds_orig_size, m_seen_weak_binds.end());
}



void MachOLoader::loadExports(const MachO& mach, intptr base, Exports* exports)
{
	exports->rehash(exports->size() + mach.exports().size());
	
	for (size_t i = 0; i < mach.exports().size(); i++)
	{
		MachO::Export exp = *mach.exports()[i];
		exp.addr += base;
		// TODO(hamaji): Not 100% sure, but we may need to consider weak symbols.
		if (!exports->insert(make_pair(exp.name, exp)).second)
			fprintf(stderr, "duplicated exported symbol: %s\n", exp.name.c_str());
	}
}


void MachOLoader::loadSymbols(const MachO& mach, intptr slide, intptr base)
{
	g_file_map.add(mach, slide, base);
}


void MachOLoader::load(const MachO& mach, Exports* exports)
{
	if (!exports)
		exports = m_exports;

	intptr slide = 0;
	intptr base = 0;

	loadSegments(mach, &slide, &base);

	doRebase(mach, slide);

	runPendingInitFuncs(mach, slide);

	loadDylibs(mach);

	loadExports(mach, base, exports);

	doBind(mach, slide);

	loadSymbols(mach, slide, base);
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
		
		if (argc >= 0)
			((void(*)(int, char**, char**, char**))*init_func)(argc, argv, envp, apple);
		else
			((void(*)())*init_func)();
	}
	m_init_funcs.clear();
}


void MachOLoader::run(MachO& mach, int argc, char** argv, char** envp)
{
	char* apple[2] = { g_darwin_executable_path, 0 };

	load(mach);
	setupDyldData(mach);

	g_file_map.addWatchDog(m_last_addr + 1);

	char* trampoline_start_addr = (char*)(((uintptr_t)&m_trampoline[0]) & ~0xfff);
    uint64_t trampoline_size = alignMem(&trampoline_[0] + m_trampoline.size() - trampoline_start_addr, 0x1000);
	
    ::mprotect(trampoline_start_addr, trampoline_size, PROT_READ | PROT_WRITE | PROT_EXEC);

	//g_timer.print(mach.filename().c_str());

	mach.close();

	runInitFuncs(argc, argv, envp, apple);

	LOG << "booting from " << (void*)mach.entry() << "..." << std::endl;
	
	fflush(stdout);
	assert(argc > 0);
	
	boot(mach.entry(), argc, argv, envp);
	
	/*
		int (*fp)(int, char**, char**) =
		(int(*)(int, char**, char**))mach.entry();
		int ret = fp(argc, argv, envp);
		exit(ret);
	*/
}

void MachOLoader::pushTrampolineCode(unsigned int c)
{
	while (c)
	{
		m_trampoline.push_back(c & 255);
		c >>= 8;
	}
}

void MachOLoader::pushTrampolineCode64(uint64_t c)
{
	for (int i = 0; i < 8; i++)
	{
		m_trampoline.push_back(c & 255);
		c >>= 8;
	}
}

void MachOLoader::pushTrampolineCode32(uint32_t c)
{
	for (int i = 0; i < 4; i++)
	{
		m_trampoline.push_back(c & 255);
		c >>= 8;
	}
}

void MachOLoader::boot( uint64_t entry, int argc, char** argv, char** envp)
{
#ifdef __x86_64__
  // 0x08: argc
  // 0x10: argv[0]
  // 0x18: argv[1]
  //  ...: argv[n]
  //       0
  //       envp[0]
  //       envp[1]
  //       envp[n]
  
  // TODO: add 'apple'!
  
  __asm__ volatile(" mov %1, %%rax;\n"
                   " mov %2, %%rdx;\n"
                   " push $0;\n"
                   // TODO(hamaji): envp
                   " push $0;\n"
                   ".loop64:\n"
                   " sub $8, %%rdx;\n"
                   " push (%%rdx);\n"
                   " dec %%eax;\n"
                   " jnz .loop64;\n"
                   " mov %1, %%eax;\n"
                   " push %%rax;\n"
                   " jmp *%0;\n"
                   ::"r"(entry), "r"(argc), "r"(argv + argc), "r"(envp)
                   :"%rax", "%rdx");
  //fprintf(stderr, "done!\n");
#else
  __asm__ volatile(" mov %1, %%eax;\n"
                   " mov %2, %%edx;\n"
                   " push $0;\n"
                   ".loop32:\n"
                   " sub $4, %%edx;\n"
                   " push (%%edx);\n"
                   " dec %%eax;\n"
                   " jnz .loop32;\n"
                   " mov %1, %%eax;\n"
                   " push %%eax;\n"
                   " jmp *%0;\n"
                   // TODO(hamaji): Fix parameters
                   ::"r"(entry), "r"(argc), "r"(argv + argc), "g"(envp)
                   :"%eax", "%edx");
#endif
}



