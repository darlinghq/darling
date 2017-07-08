#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define CPU_ARCH_ABI64  0x01000000

static void load(const char* path);

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Prints the name of the ELF library referenced by given Mach-O library.\n\n"
				"Usage: print_wrapped_elf <mach-o file>\n";
		return 1;
	}

	try
	{
		load(argv[1]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "When processing " << argv[1] << ": " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

static void throwErrno(const char* msg)
{
	std::stringstream ss;
	ss << msg << ": " << strerror(errno);
	throw std::runtime_error(ss.str());
}

template<typename mhdr_type, typename section_type, int seg_cmd, typename segment_type>
void parseMachO(int fd)
{
	mhdr_type header;
	uint8_t* cmds;
	unsigned long fat_offset;

	fat_offset = lseek(fd, 0, SEEK_CUR);

	if (read(fd, &header, sizeof(header)) != sizeof(header))
		throwErrno("Cannot read the mach header");

	cmds = new uint8_t[header.sizeofcmds];

	if (read(fd, cmds, header.sizeofcmds) != header.sizeofcmds)
		throwErrno("Cannot read loader commands");

	for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
	{
		load_command* lc;

		lc = (load_command*) &cmds[p];

		if (lc->cmd == seg_cmd)
		{
			segment_type* seg = (segment_type*) lc;

			if (strcmp(seg->segname, "__TEXT") == 0)
			{
				section_type* sect = (section_type*) (seg+1);
				section_type* end = (section_type*) (&cmds[p + lc->cmdsize]);

				while (sect < end)
				{
					if (strcmp(sect->sectname, "__elfname") == 0)
					{
						char* text;

						if (sect->size > 100)
							throw std::runtime_error("__TEXT,__elfname section has unusual length");

						text = new char[sect->size];

						if (pread(fd, text, sect->size, sect->offset + fat_offset) != sect->size)
							throwErrno("Cannot read __elfname section");

						if (text[sect->size-1] != 0)
							throw std::runtime_error("__TEXT,__elfname section contents don't look like a string");

						std::cout << text << std::endl;
						delete [] text;
					}
					sect++;
				}
			}
		}

		p += lc->cmdsize;
	}

	delete [] cmds;
}

void parseMachO32(int fd)
{
	parseMachO<mach_header, section, LC_SEGMENT, segment_command>(fd);
}

void parseMachO64(int fd)
{
	parseMachO<mach_header_64, section_64, LC_SEGMENT_64, segment_command_64>(fd);
}

static void load(const char* path)
{
	int fd;
	uint32_t magic;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		throwErrno("Cannot open the file");

	if (read(fd, &magic, sizeof(magic)) != sizeof(magic))
		throwErrno("Cannot read file header");

	if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64)
	{
		lseek(fd, 0, SEEK_SET);
		parseMachO64(fd);
	}
	else if (magic == MH_MAGIC || magic == MH_CIGAM)
	{
		lseek(fd, 0, SEEK_SET);
		parseMachO32(fd);
	}
	else if (magic == FAT_MAGIC || magic == FAT_CIGAM)
	{
		struct fat_header fhdr;
		const bool swap = magic == FAT_CIGAM;

#define SWAP32(x) x = __bswap_32(x)

		if (read(fd, &fhdr.nfat_arch, sizeof(fhdr.nfat_arch)) != sizeof(fhdr.nfat_arch))
			throwErrno("Cannot read file header");

		if (swap)
			SWAP32(fhdr.nfat_arch);

		for (uint32_t i = 0; i < fhdr.nfat_arch; i++)
		{
			struct fat_arch arch;

			if (read(fd, &arch, sizeof(arch)) != sizeof(arch))
				throwErrno("Cannot read fat_arch header");

			if (swap)
			{
				SWAP32(arch.cputype);
				SWAP32(arch.cpusubtype);
				SWAP32(arch.offset);
				SWAP32(arch.size);
				SWAP32(arch.align);
			}

			if (lseek(fd, arch.offset, SEEK_SET) == -1)
				throwErrno("Cannot seek to selected arch in fat");

			if (arch.cputype & CPU_ARCH_ABI64)
				parseMachO64(fd);
			else
				parseMachO32(fd);

			break;
		}
	}

	::close(fd);
}

