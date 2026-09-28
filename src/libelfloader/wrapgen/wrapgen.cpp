
#include <string>
#include <iostream>
#include <set>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <link.h>

#ifndef PATH_MAX
#	define PATH_MAX	4096
#endif

// This is a generator of assembly code for Mach-O
// libraries wrapping ELF libraries.

// TODO: use wrapgen32 to generate 32-bit wrappers.

void parse_elf(const char* elf, std::string& soname_out, std::set<std::string>& functions_out, std::set<std::string>& vars_out);
void generate_wrapper(std::ofstream& output, const char* soname, const std::set<std::string>& symbols);
void generate_var_wrappers(std::ofstream& output, std::ofstream& outputHeader, const std::set<std::string>& vars);
Elf64_Off vaddr_to_offset(const Elf64_Ehdr* ehdr, Elf64_Addr vaddr);

int main(int argc, const char** argv)
{
	std::string elfLibrary;
	std::set<std::string> functions, vars;
	std::string soname;
	std::ofstream output;

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <library-name> <output-file> <var-access-header>\n";
		return 1;
	}

	elfLibrary = argv[1];
	output.open(argv[2]);

	try
	{
		if (!output.is_open())
			throw std::runtime_error("Cannot open output file");

		if (access(elfLibrary.c_str(), R_OK) == -1)
		{
			// Try loading the library and then ask the loader where it found the library.
			// It is simpler than parsing /etc/ld.so.conf.

			void* handle = dlopen(elfLibrary.c_str(), RTLD_LAZY | RTLD_LOCAL);
			struct link_map* lm = NULL;

			if (!handle)
			{
				std::stringstream ss;
				ss << "Cannot load " << elfLibrary << ": " << dlerror();
				throw std::runtime_error(ss.str());
			}

			if (dlinfo(handle, RTLD_DI_LINKMAP, &lm) == 0)
			{
				elfLibrary = lm->l_name;
			}
			else
			{
				std::stringstream ss;
				std::cerr << "Cannot locate " << elfLibrary << ": " << dlerror();
				throw std::runtime_error(ss.str());
			}

			dlclose(handle);
		}

		parse_elf(elfLibrary.c_str(), soname, functions, vars);
		generate_wrapper(output, soname.c_str(), functions);

		if (!vars.empty())
		{
			std::ofstream outputHeader(argv[3]);
			if (!outputHeader.is_open())
				throw std::runtime_error("Cannot open output macro header file");

			generate_var_wrappers(output, outputHeader, vars);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}

void parse_elf(const char* elf, std::string& soname, std::set<std::string>& symbols, std::set<std::string>& vars)
{
	int fd;
	const Elf64_Ehdr* ehdr;
	const char* strings = NULL;
	uint64_t length;

	fd = open(elf, O_RDONLY);
	if (fd < 0)
	{
		std::stringstream ss;
		ss << "Error opening " << elf << ": " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	length = lseek(fd, 0, SEEK_END);

	ehdr = (Elf64_Ehdr*) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
	if (ehdr == MAP_FAILED)
	{
		std::stringstream ss;
		ss << "Cannot mmap ELF file: " << strerror(errno);
		throw std::runtime_error(ss.str());
	}

	close(fd);

	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0 || ehdr->e_ident[EI_CLASS] != ELFCLASS64)
	{
		std::stringstream ss;
		ss << elf << " is not a 64-bit ELF";
		throw std::runtime_error(ss.str());
	}

	if (ehdr->e_type != ET_DYN)
	{
		std::stringstream ss;
		ss << elf << " is not a dynamic library ELF";
		throw std::runtime_error(ss.str());
	}

	if (ehdr->e_machine != EM_X86_64 && ehdr->e_machine != EM_AARCH64)
	{
		std::stringstream ss;
		ss << elf << " is not a supported 64-bit ELF (expected x86-64 or aarch64)";
		throw std::runtime_error(ss.str());
	}

	// Now read program headers, find a PT_DYNAMIC segment type
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		const Elf64_Phdr* phdr;

		phdr = (const Elf64_Phdr*) (((char*) ehdr) + ehdr->e_phoff + (i * ehdr->e_phentsize));

		if (phdr->p_type == PT_DYNAMIC)
		{
			Elf64_Xword off_strtab, off_soname;

			off_strtab = off_soname = 0;

			for (int j = 0; j < phdr->p_filesz; j += sizeof(Elf64_Dyn))
			{
				const Elf64_Dyn* dyn;

				dyn = (const Elf64_Dyn*) (((char*) ehdr) + phdr->p_offset + j);
				
				switch (dyn->d_tag)
				{
					case DT_STRTAB:
						off_strtab = vaddr_to_offset(ehdr, dyn->d_un.d_val);
						break;
					case DT_SONAME:
						off_soname = vaddr_to_offset(ehdr, dyn->d_un.d_val);
						break;
					case DT_NULL:
						goto end_dyn;
				}
			}
end_dyn:

			if (off_strtab != 0)
			{
				strings = (const char*) ((char*) ehdr) + off_strtab;

				if (off_soname != 0)
				{
					// Read DT_SONAME value from the string table
					soname = strings + off_soname;
				}
			}

			break;
		}
	}

	if (strings != NULL)
	{
		// Load symbol list
		const Elf64_Shdr* shdr;

		for (int i = 0; i < ehdr->e_shnum; i++)
		{
			shdr = (const Elf64_Shdr*) (((char*) ehdr) + ehdr->e_shoff + (i * ehdr->e_shentsize));
			if (shdr->sh_type == SHT_DYNSYM)
			{
				for (int j = 0; j < shdr->sh_size; j += sizeof(Elf64_Sym))
				{
					const Elf64_Sym* sym;

					sym = (const Elf64_Sym*) (((char*) ehdr) + shdr->sh_offset + j);

					if (ELF64_ST_TYPE(sym->st_info) != STT_OBJECT && ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
						continue;
					if (ELF64_ST_BIND(sym->st_info) != STB_GLOBAL)
						continue;
					if (sym->st_shndx == SHN_UNDEF || sym->st_value == 0)
						continue;
					if (ELF64_ST_VISIBILITY(sym->st_other) != STV_DEFAULT)
						continue;

					if (ELF64_ST_TYPE(sym->st_info) == STT_FUNC)
						symbols.insert(strings + sym->st_name);
					else
						vars.insert(strings + sym->st_name);
				}

				break;
			}
		}
	}

	if (soname.empty())
	{
		std::cerr << "WARNING: No DT_SONAME in " << elf << ".\n";

		const char* slash = strrchr(elf, '/');
		if (slash != NULL)
			soname = slash + 1;
		else
			soname = elf;
	}
	if (symbols.empty())
	{
		std::stringstream ss;
		ss << "No symbols found in " << elf;
		throw std::runtime_error(ss.str());
	}

	munmap((void*) ehdr, length);
}

void generate_wrapper(std::ofstream& output, const char* soname, const std::set<std::string>& symbols)
{
	output << "#include <elfcalls.h>\n"
		"extern struct elf_calls* _elfcalls;\n\n"
		"extern const char __elfname[];\n\n";

	// The functions below are symbol resolvers: the first call to each one goes
	// through ld64's resolver helper, which calls the resolver and then jumps to
	// the function it returned with the caller's arguments still in place.
	// On x86-64 the helper only saves the integer argument registers, while
	// looking the symbol up in the ELF library can clobber the XMM registers
	// carrying floating-point and vector arguments. preserve_all makes the
	// resolver itself save them.
	output << "#ifdef __x86_64__\n"
		"#define WRAPGEN_RESOLVER __attribute__((preserve_all))\n"
		"#else\n"
		"#define WRAPGEN_RESOLVER\n"
		"#endif\n\n";

	output << "static void* lib_handle;\n"
		"static void open_library(void) {\n"
		"\tif (!lib_handle)\n"
		"\t\tlib_handle = _elfcalls->dlopen_fatal(__elfname);\n"
		"}\n\n"
		"__attribute__((constructor)) static void initializer() {\n"
		"\topen_library();\n"
		"}\n\n";

	output << "__attribute__((destructor)) static void destructor() {\n"
		"\tif (lib_handle)\n"
		"\t\t_elfcalls->dlclose_fatal(lib_handle);\n"
		"}\n\n";

	// dyld runs symbol resolvers while binding, and for non-lazy binds (chained fixups, bind_at_load)
	// that happens before libSystem's initializer sets _elfcalls. Such resolvers return a trampoline
	// that looks the symbol up on its first call.
	output << "#if defined(__arm64__)\n"
		"struct lazy_symbol { void* address; const char* name; };\n\n"
		"__attribute__((visibility(\"hidden\"), used))\n"
		"void* __elf_lazy_resolve(struct lazy_symbol* symbol) {\n"
		"\tif (!symbol->address) {\n"
		"\t\topen_library();\n"
		"\t\tsymbol->address = _elfcalls->dlsym_fatal(lib_handle, symbol->name);\n"
		"\t}\n"
		"\treturn symbol->address;\n"
		"}\n\n"
		"__asm__(\".text\\n.p2align 2\\n.private_extern ___elf_lazy_common\\n"
		"___elf_lazy_common:\\n"
		"\\tldr x17, [x16]\\n"
		"\\tcbz x17, 1f\\n"
		"\\tbr x17\\n"
		"1:\\tstp x29, x30, [sp, #-16]!\\n"
		"\\tmov x29, sp\\n"
		"\\tsub sp, sp, #208\\n"
		"\\tstp x0, x1, [sp]\\n\\tstp x2, x3, [sp, #16]\\n\\tstp x4, x5, [sp, #32]\\n\\tstp x6, x7, [sp, #48]\\n"
		"\\tstp x8, xzr, [sp, #64]\\n"
		"\\tstp q0, q1, [sp, #80]\\n\\tstp q2, q3, [sp, #112]\\n\\tstp q4, q5, [sp, #144]\\n\\tstp q6, q7, [sp, #176]\\n"
		"\\tmov x0, x16\\n"
		"\\tbl ___elf_lazy_resolve\\n"
		"\\tmov x17, x0\\n"
		"\\tldp x0, x1, [sp]\\n\\tldp x2, x3, [sp, #16]\\n\\tldp x4, x5, [sp, #32]\\n\\tldp x6, x7, [sp, #48]\\n"
		"\\tldr x8, [sp, #64]\\n"
		"\\tldp q0, q1, [sp, #80]\\n\\tldp q2, q3, [sp, #112]\\n\\tldp q4, q5, [sp, #144]\\n\\tldp q6, q7, [sp, #176]\\n"
		"\\tmov sp, x29\\n"
		"\\tldp x29, x30, [sp], #16\\n"
		"\\tbr x17\\n\");\n"
		"#endif\n\n";

	for (const std::string& sym : symbols)
	{
		output << "#if defined(__arm64__)\n"
			"__attribute__((visibility(\"hidden\"), used))\n"
			"struct lazy_symbol __elf_lazy_symbol_" << sym << " = { 0, \"" << sym << "\" };\n"
			"extern void __elf_lazy_" << sym << "(void);\n"
			"__asm__(\".text\\n.p2align 2\\n.private_extern ___elf_lazy_" << sym << "\\n"
			"___elf_lazy_" << sym << ":\\n"
			"\\tadrp x16, ___elf_lazy_symbol_" << sym << "@PAGE\\n"
			"\\tadd x16, x16, ___elf_lazy_symbol_" << sym << "@PAGEOFF\\n"
			"\\tb ___elf_lazy_common\\n\");\n"
			"#endif\n"
			"WRAPGEN_RESOLVER void* " << sym << "() {\n"
			"\t__asm__(\".symbol_resolver _" << sym << "\");\n"
			"#if defined(__arm64__)\n"
			"\tif (!_elfcalls)\n"
			"\t\treturn __elf_lazy_" << sym << ";\n"
			"#endif\n"
			"\topen_library();\n"
			"\treturn _elfcalls->dlsym_fatal(lib_handle, \"" << sym << "\");\n"
			"}\n\n";
	}
	output << "asm(\".section __TEXT,__elfname\\n"
		".private_extern ___elfname\\n"
		"___elfname: .asciz \\\"" << soname << "\\\"\");\n";
}

void generate_var_wrappers(std::ofstream& output, std::ofstream& outputHeader, const std::set<std::string>& vars)
{
	outputHeader << "#pragma once\n\n";
	outputHeader << "#ifdef __cplusplus\n"
		"extern \"C\" {\n"
		"#endif\n\n";

	for (const std::string& sym : vars)
	{
		output << "void* __elf_get_" << sym << "(void) {\n"
			"\treturn _elfcalls->dlsym_fatal(lib_handle, \"" << sym << "\");\n"
			"}\n\n";
		
		outputHeader << "extern __typeof(" << sym << ")* __elf_get_" << sym << "(void);\n"
			"#define " << sym << " (*__elf_get_" << sym << "())\n\n";
	}

	outputHeader << "\n\n#ifdef __cplusplus\n"
		"}\n"
		"#endif\n\n";
}

Elf64_Off vaddr_to_offset(const Elf64_Ehdr* ehdr, Elf64_Xword vaddr)
{
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		const Elf64_Phdr* phdr;
		Elf64_Addr vstart, vend;

		phdr = (const Elf64_Phdr*) (((char*) ehdr) + ehdr->e_phoff + (i * ehdr->e_phentsize));
		vstart = phdr->p_vaddr;
		vend = phdr->p_vaddr + phdr->p_memsz;
		if (vstart <= vaddr && vaddr < vend) {
			return phdr->p_offset + vaddr - vstart;
		}
	}

	return 0;
}
