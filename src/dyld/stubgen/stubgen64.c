#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <elf.h>
#include <errno.h>
#include <dlfcn.h>

#ifndef PATH_MAX
#	define PATH_MAX	4096
#endif

// This is a generator of assembly code for Mach-O
// libraries wrapping ELF libraries.

// TODO: use stubgen32 to generate 32-bit wrappers.

const char* get_soname(const char* elf);
void generate_wrapper(const char* soname, const char* symbols);

int main(int argc, const char** argv)
{
	const char* elfLibrary;
	const char* symbols;
	const char* soname;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <library-name> <symbols>\n", argv[0]);
		fprintf(stderr, "...where symbols should be semicolon-separated (CMake string list)\n");
		exit(1);
	}

	elfLibrary = argv[1];
	symbols = argv[2];

	if (access(elfLibrary, R_OK) == -1)
	{
		// Try loading the library and then ask the loader where it found the library.
		// It is simpler than parsing /etc/ld.so.conf.

		void* handle = dlopen(elfLibrary, RTLD_LAZY | RTLD_LOCAL);
		char path[PATH_MAX];

		if (!handle)
		{
			fprintf(stderr, "Cannot load %s: %s\n", elfLibrary, dlerror());
			exit(1);
		}

		if (dlinfo(handle, RTLD_DI_ORIGIN, path) == 0)
		{
			strcat(path, "/");
			strcat(path, elfLibrary);
			elfLibrary = strdup(path);
		}
		else
		{
			fprintf(stderr, "Cannot locate %s: %s.\n", elfLibrary, dlerror());
			exit(1);
		}

		dlclose(handle);
	}
library_found:

	soname = get_soname(elfLibrary);

	generate_wrapper(soname, symbols);

	return 0;
}

const char* get_soname(const char* elf)
{
	FILE* file;
	Elf64_Ehdr ehdr;

	file = fopen(elf, "rb");
	if (!file)
	{
		fprintf(stderr, "Error opening %s: %s\n", elf, strerror(errno));
		exit(1);
	}

	if (fread(&ehdr, 1, sizeof(ehdr), file) != sizeof(ehdr))
	{
		fprintf(stderr, "Error reading %s\n", elf);
		exit(1);
	}

	if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0 || ehdr.e_ident[EI_CLASS] != ELFCLASS64)
	{
		fprintf(stderr, "%s is not a 64-bit ELF\n", elf);
		exit(1);
	}

	if (ehdr.e_type != ET_DYN)
	{
		fprintf(stderr, "%s is not a dynamic library ELF\n", elf);
		exit(1);
	}

	if (ehdr.e_machine != EM_X86_64)
	{
		fprintf(stderr, "%s is not an ELF for x86-64\n", elf);
		exit(1);
	}

	// Now read program headers, find a PT_DYNAMIC segment type
	for (int i = 0; i < ehdr.e_phnum; i++)
	{
		Elf64_Phdr phdr;

		fseek(file, ehdr.e_phoff + (i * ehdr.e_phentsize), SEEK_SET);
		if (fread(&phdr, 1, sizeof(phdr), file) != sizeof(phdr))
		{
			fprintf(stderr, "Cannot read ELF phdr in %s\n", elf);
			exit(1);
		}

		if (phdr.p_type == PT_DYNAMIC)
		{
			Elf64_Xword off_strtab, off_soname;

			off_strtab = off_soname = 0;

			// Inside the PT_DYNAMIC segment, look for a DT_SONAME tag in Elf64_Dyn.d_tag
			fseek(file, phdr.p_offset, SEEK_SET);

			for (int j = 0; j < phdr.p_filesz; j += sizeof(Elf64_Dyn))
			{
				Elf64_Dyn dyn;
				if (fread(&dyn, 1, sizeof(dyn), file) != sizeof(dyn))
				{
					fprintf(stderr, "Cannot read PT_DYNAMIC entry in %s\n", elf);
					exit(1);
				}

				if (dyn.d_tag == DT_STRTAB)
					off_strtab = dyn.d_un.d_val;
				else if (dyn.d_tag == DT_SONAME)
					off_soname = dyn.d_un.d_val;
			}

			if (off_strtab != 0 && off_soname != 0)
			{
				char soname[256];

				// Read DT_SONAME value from the string table
				fseek(file, off_strtab + off_soname, SEEK_SET);
				fread(soname, 1, sizeof(soname)-1, file);
				soname[255] = '\0';

				fclose(file);

				return strdup(soname);
			}

			break;
		}
	}

	fprintf(stderr, "WARNING: No DT_SONAME in %s.\n", elf);
	fclose(file);

	const char* slash = strrchr(elf, '/');
	if (slash != NULL)
		return slash + 1;
	else
		return elf;
}

void define_string(const char* name, const char* value)
{
	printf(".section __TEXT,__cstring,cstring_literals\n");
	printf("%s: .asciz \"%s\"\n", name, value);
}

void print_init_exit(void)
{
	puts(".zerofill __DATA,__bss,_lib_handle,8,3\n");
	puts(".section __TEXT,__text,regular,pure_instructions\n");

	puts("_initializer:\n"
				"\tpushq   %rdi\n"
				"\tmovq    __elfcalls@GOTPCREL(%rip), %rax\n"
				"\tmovq    _library_name(%rip), %rdi\n"
				"\tcallq   *56(%rax)\n" // dlopen_fatal
				"\tmovq    %rax, _lib_handle(%rip)\n"
				"\tpopq    %rdi\n"
				"\tret\n");

	puts("_destructor:\n"
				"\tpushq   %rdi\n"
				"\tmovq    __elfcalls@GOTPCREL(%rip), %rcx\n"
				"\tmovq    _lib_handle(%rip), %rdi\n"
				"\tcallq   *64(%rcx)\n" // dlclose_fatal
				"\tpopq    %rdi\n"
				"\tret\n");

	puts(".section __DATA,__mod_init_func,mod_init_funcs\n"
				".align  3\n"
				".long   _initializer\n"
				".section __DATA,__mod_term_func,mod_term_funcs\n"
				".align  3\n"
				".long   _destructor\n");

	puts(".macro loadfunc\n"
			"\tpushq %rdi\n"
			"\tpushq %rsi\n"
			"\tmovq _lib_handle(%rip), %rdi\n"
			"\tmovq $0(%rip), %rsi\n"
			"\tmovq __elfcalls@GOTPCREL(%rip), %rax\n"
			"\tcallq *72(%rax)\n" // dlsym_fatal
			"\tmovq %rax, $1(%rip)\n"
			"\tpopq %rsi\n"
			"\tpopq %rdi\n"
			".endmacro\n");
}

void generate_wrapper(const char* soname, const char* symbols)
{
	char* sym;
	char* syms = strdup(symbols);

	printf("#ifdef __x86_64__\n");

	define_string("_library_name", soname);

	print_init_exit();

	sym = strtok(syms, ";");
	while (sym != NULL)
	{
		char name[128];

		printf(".zerofill __DATA,__bss,_%s_impl,8,3\n", sym);
		printf(".section __TEXT,__text,regular,pure_instructions\n\n");
		printf(".globl _%s\n"
				"_%s:\n"
				"\tmovq _%s_impl(%%rip), %%rax\n"
				"\ttestq %%rax, %%rax\n"
				"\tjz 1f\n"
				"\tjmpq *%%rax\n"
				"1:\n"
				"\tloadfunc _%s_name, _%s_impl\n"
				"\tjmpq *%%rax\n\n",
				sym, sym, sym, sym, sym);

		sprintf(name, "_%s_name", sym);
		define_string(name, sym);

		sym = strtok(NULL, ";");
	}

	printf("#endif // __x86_64__\n");
	free(syms);
}

