
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

	if (ehdr->e_machine != EM_X86_64)
	{
		std::stringstream ss;
		ss << elf << " is not an ELF for x86-64";
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
						off_strtab = dyn->d_un.d_val;
						break;
					case DT_SONAME:
						off_soname = dyn->d_un.d_val;
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

	output << "static void* lib_handle;\n"
		"__attribute__((constructor)) static void initializer() {\n"
		"\tlib_handle = _elfcalls->dlopen_fatal(__elfname);\n"
		"}\n\n";

	output << "__attribute__((destructor)) static void destructor() {\n"
		"\t_elfcalls->dlclose_fatal(lib_handle);\n"
		"}\n\n";
	
	for (const std::string& sym : symbols)
	{
		output << "void* " << sym << "() {\n"
			"\t__asm__(\".symbol_resolver _" << sym << "\");\n"
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
