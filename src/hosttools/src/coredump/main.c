#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <libgen.h>

#include <mach-o/loader.h>
#include <elf.h>

#include <linux/time_types.h>

#if __x86_64__
#include <coredump/x86_64.h>
#else
#error Not implemented
#endif

#include <darling-config.h>

#ifndef DUMP_FLOAT_STATE
	#define DUMP_FLOAT_STATE 1
#endif

#ifndef ALIGN_REGIONS
	#define ALIGN_REGIONS 0
#endif

#ifndef INVALIDATE_DEVICES
	#define INVALIDATE_DEVICES 0
#endif

// interestingly enough, there are no existing tools that can perform this conversion (ELF coredump to Mach-O coredump).
// neither objcopy nor llvm-objcopy support Mach-O conversion like that, nor does objconv (it considers coredumps to be executables and refuses to operate on them).
// porting our existing code from the LKM is simple enough, so that's what we do here.

struct vm_area {
	const char* filename;
	size_t filename_length;
	uintptr_t memory_address;
	size_t memory_size;
	size_t file_offset;
	size_t file_size;
	uint8_t protection;
	bool valid;
	uint64_t expected_offset;
};

struct elf32_nt_file_header {
	uint32_t count;
	uint32_t page_size;
};

struct elf64_nt_file_header {
	uint64_t count;
	uint64_t page_size;
};

union nt_file_header {
	struct elf32_nt_file_header elf32;
	struct elf64_nt_file_header elf64;
};

struct elf32_nt_file_entry {
	uint32_t start;
	uint32_t end;
	uint32_t offset;
};

struct elf64_nt_file_entry {
	uint64_t start;
	uint64_t end;
	uint64_t offset;
};

union nt_file_entry {
	struct elf32_nt_file_entry elf32;
	struct elf64_nt_file_entry elf64;
};

union elf32_nt_prstatus_registers {
	struct nt_prstatus_registers_i386 i386;
};

union elf64_nt_prstatus_registers {
	struct nt_prstatus_registers_x86_64 x86_64;
};

struct elf32_kernel_old_timeval {
	int tv_sec;
	int tv_usec;
};

struct elf64_kernel_old_timeval {
	long tv_sec;
	long tv_usec;
};

struct elf32_nt_prstatus {
	int signal_number;
	int signal_code;
	int signal_errno;
	short current_signal;
	unsigned int pending_signals;
	unsigned int held_signals;
	pid_t pid;
	pid_t ppid;
	pid_t pgrp;
	pid_t sid;
	struct elf32_kernel_old_timeval user_time;
	struct elf32_kernel_old_timeval system_time;
	struct elf32_kernel_old_timeval cumulative_user_time;
	struct elf32_kernel_old_timeval cumulative_system_time;
	union elf32_nt_prstatus_registers general_registers;
};

struct elf64_nt_prstatus {
	int signal_number;
	int signal_code;
	int signal_errno;
	short current_signal;
	unsigned long pending_signals;
	unsigned long held_signals;
	pid_t pid;
	pid_t ppid;
	pid_t pgrp;
	pid_t sid;
	struct elf64_kernel_old_timeval user_time;
	struct elf64_kernel_old_timeval system_time;
	struct elf64_kernel_old_timeval cumulative_user_time;
	struct elf64_kernel_old_timeval cumulative_system_time;
	union elf64_nt_prstatus_registers general_registers;
};

union nt_prstatus {
	struct elf32_nt_prstatus elf32;
	struct elf64_nt_prstatus elf64;
};

struct thread_info {
	union nt_prstatus* prstatus;
};

struct elf_universal_header {
	unsigned char e_ident[EI_NIDENT];	/* Magic number and other info */
	/* Elf32_Half/Elf64_Half -> uint16_t */
	uint16_t e_type;			/* Object file type */
	/* Elf32_Half/Elf64_Half -> uint16_t */
	uint16_t e_machine;		/* Architecture */
	/* Elf32_Word/Elf64_Word ->  uint32_t */
	uint32_t e_version;		/* Object file version */
};

union Elf_Ehdr {
	Elf32_Ehdr elf32;
	Elf64_Ehdr elf64;
};

union Elf_Phdr {
	Elf32_Phdr elf32;
	Elf64_Phdr elf64;
};

union Elf_Nhdr {
	Elf32_Nhdr elf32;
	Elf64_Nhdr elf64;
};

#define cprm_elf(_is64, _expr, _member) ({ \
		__typeof__((_expr)) tmp = _expr; \
		/* cast to the bigger type (Elf64) */ \
		_is64 ? tmp->elf64._member : (__typeof__(tmp->elf64._member))tmp->elf32._member; \
	})

struct coredump_params {
	int input_corefile;
	int output_corefile;
	size_t input_corefile_size;
	const void* input_corefile_mapping;
	const struct elf_universal_header *universal_header;
	const union Elf_Ehdr* input_header;
	const union Elf_Phdr* input_program_headers;
	const union Elf_Phdr* input_program_headers_end;
	const union Elf_Nhdr* input_notes;
	size_t input_notes_size;
	struct vm_area* vm_areas;
	size_t vm_area_count;
	union nt_file_header* nt_file;
	const char** nt_file_filenames;
	struct thread_info* thread_infos;
	size_t thread_info_count;
	size_t written;
	const char* prefix;
	size_t prefix_length;
	const char* main_executable_path;
	size_t main_executable_path_length;
	bool is_64_bit;
};

static char default_output_name[4096];
static char default_prefix[4096];

static uint64_t round_up_pow2(uint64_t number, uint64_t multiple) {
	return (number + (multiple - 1)) & -multiple;
};

static uint64_t round_down_pow2(uint64_t number, uint64_t multiple) {
	return (number) & -multiple;
};

static const char* note_name(const struct coredump_params* cprm, const union Elf_Nhdr* note) {
	return (const char*)note + (cprm->is_64_bit ? sizeof(note->elf64) : sizeof(note->elf64));
};

static const void* note_data(const struct coredump_params* cprm, const union Elf_Nhdr* note) {
	return note_name(cprm, note) + round_up_pow2(cprm_elf(cprm->is_64_bit, note, n_namesz), 4);
};

static const union Elf_Nhdr* find_next_note(const struct coredump_params* cprm, const union Elf_Nhdr* note) {
	uint64_t length = sizeof(*note) + round_up_pow2(cprm_elf(cprm->is_64_bit, note, n_namesz), 4) + round_up_pow2(cprm_elf(cprm->is_64_bit, note, n_descsz), 4);
	return (const union Elf_Nhdr*)((char*)note + length);
};

static size_t nt_file_entry_size(const struct coredump_params* cprm) {
	return cprm->is_64_bit ? sizeof(struct elf64_nt_file_entry) : sizeof(struct elf32_nt_file_entry);
};

static size_t nt_file_header_size(const struct coredump_params* cprm) {
	return cprm->is_64_bit ? sizeof(struct elf64_nt_file_header) : sizeof(struct elf32_nt_file_header);
};

static const union nt_file_entry* nt_file_get_entry(const struct coredump_params* cprm, const union nt_file_header* header, size_t index) {
	return (const union nt_file_entry*)((const char*)header + nt_file_header_size(cprm) + (nt_file_entry_size(cprm) * index));
};

// first tries to open the file directly, then tries to open the file in the lower layer of the overlay
// (because if we're outside the container, the overlay won't be mounted, but the core dump paths would refer to it)
static int open_file(struct coredump_params* cprm, const char* filename, size_t filename_length) {
	int fd = -1;

	// try to open it directly first
	fd = open(filename, O_RDONLY);

	// if that fails, try to see if it refers to the mounted prefix; if it does, try the lower layer
	if (fd < 0 && filename_length >= cprm->prefix_length && strncmp(filename, cprm->prefix, cprm->prefix_length) == 0) {
		char* temp_filename = malloc(sizeof(LIBEXEC_PATH "/") + (filename_length - cprm->prefix_length));
		sprintf(temp_filename, "%s/%s", LIBEXEC_PATH, &filename[cprm->prefix_length]);
		fd = open(temp_filename, O_RDONLY);
		free(temp_filename);
	}

	return fd;
};

void macho_coredump(struct coredump_params* cprm);

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input-coredump> [output-coredump]\n", (argc > 0 ? argv[0] : "darling-coredump"));
		return 1;
	}

	char *tmp_output_dirname = strdup(argv[1]);
	char *tmp_output_basename = strdup(argv[1]);
	if (snprintf(default_output_name, sizeof(default_output_name), "%s/darlingcore-%s", dirname(tmp_output_dirname), basename(tmp_output_basename)) < 0) {
		perror("snprintf");
		return 1;
	}
	free(tmp_output_dirname); tmp_output_dirname = NULL;
	free(tmp_output_basename); tmp_output_basename = NULL;

	const char* homedir = NULL;

	if (getenv("HOME")) {
		homedir = getenv("HOME");
	} else {
		struct passwd* pwd = getpwuid(getuid());
		homedir = pwd->pw_dir;
	}

	if (snprintf(default_prefix, sizeof(default_prefix), "%s/.darling", homedir) < 0) {
		perror("snprintf");
		return 1;
	}

	struct coredump_params cprm = {0};

	cprm.prefix = getenv("DPREFIX");

	if (!cprm.prefix) {
		cprm.prefix = default_prefix;
	}

	cprm.prefix_length = strlen(cprm.prefix);

	cprm.input_corefile = open(argv[1], O_RDONLY);
	if (cprm.input_corefile < 0) {
		perror("open input");
		return 1;
	}

	struct stat input_corefile_stats;
	if (fstat(cprm.input_corefile, &input_corefile_stats) < 0) {
		perror("fstat");
		return 1;
	}
	cprm.input_corefile_size = input_corefile_stats.st_size;
	cprm.input_corefile_mapping = mmap(NULL, cprm.input_corefile_size, PROT_READ, MAP_PRIVATE, cprm.input_corefile, 0);

	cprm.output_corefile = open((argc > 2 ? argv[2] : default_output_name), O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (cprm.output_corefile < 0) {
		perror("open output");
		return 1;
	}

	cprm.universal_header = cprm.input_corefile_mapping;

	if (
		cprm.universal_header->e_ident[EI_MAG0] != ELFMAG0 ||
		cprm.universal_header->e_ident[EI_MAG1] != ELFMAG1 ||
		cprm.universal_header->e_ident[EI_MAG2] != ELFMAG2 ||
		cprm.universal_header->e_ident[EI_MAG3] != ELFMAG3 ||
		cprm.universal_header->e_ident[EI_VERSION] != EV_CURRENT ||
		cprm.universal_header->e_version != EV_CURRENT ||
		cprm.universal_header->e_type != ET_CORE
	) {
		fprintf(stderr, "Input file is not a valid corefile\n");
		return 1;
	}

	if (cprm.universal_header->e_ident[EI_CLASS] == ELFCLASS64) {
		cprm.is_64_bit = true;
	} else if (cprm.universal_header->e_ident[EI_CLASS] == ELFCLASS32) {
		cprm.is_64_bit = false;
	} else {
		fprintf(stderr, "Input file is not a valid corefile (invalid ELF class)\n");
		return 1;
	}

	switch (cprm.universal_header->e_machine) {
		case EM_X86_64:
		case EM_386:
			cprm.input_header = cprm.input_corefile_mapping;
			break;
		default:
			fprintf(stderr, "Unexpected e_machine (%d) detected, aborting.\n", cprm.universal_header->e_machine);
			return 1;
	}

	cprm.input_program_headers = (const void*)((const char*)cprm.input_corefile_mapping + cprm_elf(cprm.is_64_bit, cprm.input_header, e_phoff));
	cprm.input_program_headers_end = (const void*)((const char*)cprm.input_program_headers + (cprm_elf(cprm.is_64_bit, cprm.input_header, e_phentsize) * cprm_elf(cprm.is_64_bit, cprm.input_header, e_phnum)));

	// first, count how many VM areas we have
	for (const union Elf_Phdr* program_header = cprm.input_program_headers; program_header < cprm.input_program_headers_end; program_header = (const void*)((const char*)program_header + cprm_elf(cprm.is_64_bit, cprm.input_header, e_phentsize))) {
		if (cprm_elf(cprm.is_64_bit, program_header, p_type) == PT_LOAD) {
			++cprm.vm_area_count;
		} else if (cprm_elf(cprm.is_64_bit, program_header, p_type) == PT_NOTE) {
			// while we're at it, also load the NOTE segment

			// XXX: ignoring it is probably not the best choice
			if (cprm.input_notes) {
				printf("warning: ignoring extra PT_NOTE segment\n");
				continue;
			}

			cprm.input_notes = (const void*)((const char*)cprm.input_corefile_mapping + cprm_elf(cprm.is_64_bit, program_header, p_offset)); 
			cprm.input_notes_size = cprm_elf(cprm.is_64_bit, program_header, p_filesz);
		}
	}

	if (!cprm.input_notes) {
		fprintf(stderr, "Input corefile does not contain PT_NOTE segment\n");
		return 1;
	}

	for (const union Elf_Nhdr* note_header = cprm.input_notes; note_header < (const union Elf_Nhdr*)((const char*)cprm.input_notes + cprm.input_notes_size); note_header = find_next_note(&cprm, note_header)) {
		if (cprm_elf(cprm.is_64_bit, note_header, n_type) == NT_FILE) {
			// allocate a copy for alignment purposes
			cprm.nt_file = malloc(cprm_elf(cprm.is_64_bit, note_header, n_descsz));
			if (!cprm.nt_file) {
				perror("malloc");
				return 1;
			}
			memcpy(cprm.nt_file, note_data(&cprm, note_header), cprm_elf(cprm.is_64_bit, note_header, n_descsz));

			cprm.nt_file_filenames = malloc(cprm_elf(cprm.is_64_bit, cprm.nt_file, count) * sizeof(const char*));
			if (!cprm.nt_file_filenames) {
				perror("malloc");
				return 1;
			}

			const char* filenames = (const char*)nt_file_get_entry(&cprm, cprm.nt_file, cprm_elf(cprm.is_64_bit, cprm.nt_file, count));
			for (size_t i = 0, offset = 0; i < cprm_elf(cprm.is_64_bit, cprm.nt_file, count); ++i) {
				cprm.nt_file_filenames[i] = &filenames[offset];
				offset += strlen(&filenames[offset]) + 1;
			}
		} else if (cprm_elf(cprm.is_64_bit, note_header, n_type) == NT_PRSTATUS) {
			++cprm.thread_info_count;
		} else {
			continue;
		}
	}

	if (!cprm.nt_file) {
		fprintf(stderr, "Input corefile does not contain NT_FILE note\n");
		return 1;
	}

	cprm.thread_infos = malloc(sizeof(struct thread_info) * cprm.thread_info_count);
	if (!cprm.thread_infos) {
		perror("malloc");
		return 1;
	}

	size_t thread_info_index = 0;
	for (const union Elf_Nhdr* note_header = cprm.input_notes; note_header < (const union Elf_Nhdr*)((const char*)cprm.input_notes + cprm.input_notes_size); note_header = find_next_note(&cprm, note_header)) {
		if (cprm_elf(cprm.is_64_bit, note_header, n_type) == NT_PRSTATUS) {
			// allocate a copy for alignment purposes
			union nt_prstatus* prstatus = malloc(cprm_elf(cprm.is_64_bit, note_header, n_descsz));
			if (!prstatus) {
				perror("malloc");
				return 1;
			}
			cprm.thread_infos[thread_info_index++].prstatus = prstatus;
			memcpy(prstatus, note_data(&cprm, note_header), cprm_elf(cprm.is_64_bit, note_header, n_descsz));
		} else {
			continue;
		}
	}

	// determine if we have extra mappings in NT_FILE that aren't present in the program headers (gcore tends to do this)
	// also try to determine which file is our main executable
	for (size_t i = 0; i < cprm_elf(cprm.is_64_bit, cprm.nt_file, count); ++i) {
		const union nt_file_entry* entry = nt_file_get_entry(&cprm, cprm.nt_file, i);
		const char* filename = cprm.nt_file_filenames[i];
		bool found = false;

		// try to determine if this is the main executable file
		if (strncmp(filename, "/dev/", sizeof("/dev/") - 1) != 0) {
			int tmpfd = open_file(&cprm, filename, strlen(filename));
			if (tmpfd >= 0) {
				struct mach_header_64 mh;
				if (read(tmpfd, &mh, sizeof(mh)) == sizeof(mh) && mh.filetype == MH_EXECUTE) {
					if (cprm.main_executable_path) {
						if (strcmp(cprm.main_executable_path, filename) != 0) {
							printf("Already had main executable (\"%s\") but found another? (\"%s\"). Continuing anyways...\n", cprm.main_executable_path, filename);
							//exit(EXIT_FAILURE);
						}
					} else {
						cprm.main_executable_path = filename;
						cprm.main_executable_path_length = strlen(cprm.main_executable_path);
						printf("Found main executable: %s\n", cprm.main_executable_path);
					}
				}
				close(tmpfd);
			}
		}

		for (const union Elf_Phdr* program_header = cprm.input_program_headers; program_header < cprm.input_program_headers_end; program_header = (const union Elf_Phdr*)((const char*)program_header + cprm_elf(cprm.is_64_bit, cprm.input_header, e_phentsize))) {
			if (cprm_elf(cprm.is_64_bit, program_header, p_type) != PT_LOAD) {
				continue;
			}
		
			if (cprm_elf(cprm.is_64_bit, entry, start) == cprm_elf(cprm.is_64_bit, program_header, p_vaddr)) {
				found = true;
				break;
			}
		}

		if (found) {
			continue;
		}

		++cprm.vm_area_count;

		break;
	}

	// now allocate the VM area array
	cprm.vm_areas = malloc(sizeof(*cprm.vm_areas) * cprm.vm_area_count);
	if (!cprm.vm_areas) {
		perror("malloc");
		return 1;
	}

	memset(cprm.vm_areas, 0, sizeof(*cprm.vm_areas) * cprm.vm_area_count);

	// now load up the VM area array
	size_t vm_area_index = 0;
	for (const union Elf_Phdr* program_header = cprm.input_program_headers; program_header < cprm.input_program_headers_end; program_header = (const union Elf_Phdr*)((const char*)program_header + cprm_elf(cprm.is_64_bit, cprm.input_header, e_phentsize))) {
		if (cprm_elf(cprm.is_64_bit, program_header, p_type) == PT_LOAD) {
			struct vm_area* vm_area = &cprm.vm_areas[vm_area_index++];

			vm_area->valid = true;
			vm_area->memory_address = cprm_elf(cprm.is_64_bit, program_header, p_vaddr);
			vm_area->memory_size = cprm_elf(cprm.is_64_bit, program_header, p_memsz);
			vm_area->file_offset = cprm_elf(cprm.is_64_bit, program_header, p_offset);

			if (!cprm.main_executable_path && vm_area->memory_address == 0) {
				// if this is PAGEZERO and we don't yet have a main executable registered,
				// the file containing this segment is most likely the main executable
				cprm.main_executable_path = vm_area->filename;
				cprm.main_executable_path_length = vm_area->filename_length;
				printf("Found main executable (maybe): %s\n", cprm.main_executable_path);
			}

			vm_area->protection = 0;
			if (cprm_elf(cprm.is_64_bit, program_header, p_flags) & PF_R) {
				vm_area->protection |= PROT_READ;
			}
			if (cprm_elf(cprm.is_64_bit, program_header, p_flags) & PF_W) {
				vm_area->protection |= PROT_WRITE;
			}
			if (cprm_elf(cprm.is_64_bit, program_header, p_flags) & PF_X) {
				vm_area->protection |= PROT_EXEC;
			}

			if (vm_area->protection == 0) {
				// if the area has no protection flags, it most likely wasn't included anywhere.
				// no need to include it in the core dump. this also handles the case of __PAGEZERO.
				vm_area->valid = false;
			}

			if (cprm_elf(cprm.is_64_bit, program_header, p_filesz) == 0) {
				// contents contained within original file

				// in this case, the file size *should* be the same as the size in memory
				vm_area->file_size = vm_area->memory_size;

				// let's look for the NT_FILE entry
				for (size_t i = 0; i < cprm_elf(cprm.is_64_bit, cprm.nt_file, count); ++i) {
					const union nt_file_entry* entry = nt_file_get_entry(&cprm, cprm.nt_file, i);
					const char* filename = cprm.nt_file_filenames[i];

					if (cprm_elf(cprm.is_64_bit, entry, start) > vm_area->memory_address || cprm_elf(cprm.is_64_bit, entry, end) < vm_area->memory_address + vm_area->memory_size) {
						continue;
					}

					vm_area->filename = filename;
					vm_area->filename_length = strlen(vm_area->filename);
					vm_area->file_offset = (cprm_elf(cprm.is_64_bit, entry, offset) * cprm_elf(cprm.is_64_bit, cprm.nt_file, page_size)) + (vm_area->memory_address - cprm_elf(cprm.is_64_bit, entry, start));
					vm_area->file_size = cprm_elf(cprm.is_64_bit, entry, end) - cprm_elf(cprm.is_64_bit, entry, start);

					break;
				}

				if (!vm_area->filename) {
					// this can occur for pages that are allocated but never accessed, so there would be no reason to include them in the core dump.
					// in this case, the program header literally indicates that the region's file size is 0, since it's unnecessary to include anywhere,
					// not necessarily that it resides in a file.

					vm_area->file_size = 0;
				}

#if INVALIDATE_DEVICES
				if (vm_area->filename_length >= 4 && strncmp(vm_area->filename, "/dev", 4) == 0) {
					vm_area->valid = false;
				}
#endif
			} else {
				// contents contained within this corefile
				vm_area->filename = NULL;
				vm_area->filename_length = 0;
				vm_area->file_size = cprm_elf(cprm.is_64_bit, program_header, p_filesz);
			}
		}
	}

	// load in extra mappings from NT_FILE
	for (size_t i = 0; i < cprm_elf(cprm.is_64_bit, cprm.nt_file, count); ++i) {
		const union nt_file_entry* entry = nt_file_get_entry(&cprm, cprm.nt_file, i);
		const char* filename = cprm.nt_file_filenames[i];
		bool found = false;

		for (const union Elf_Phdr* program_header = cprm.input_program_headers; program_header < cprm.input_program_headers_end; program_header = (const union Elf_Phdr*)((const char*)program_header + cprm_elf(cprm.is_64_bit, cprm.input_header, e_phentsize))) {
			if (cprm_elf(cprm.is_64_bit, program_header, p_type) != PT_LOAD) {
				continue;
			}
		
			if (cprm_elf(cprm.is_64_bit, entry, start) == cprm_elf(cprm.is_64_bit, program_header, p_vaddr)) {
				found = true;
				break;
			}
		}

		if (found) {
			continue;
		}

		struct vm_area* vm_area = &cprm.vm_areas[vm_area_index++];

		vm_area->filename = filename;
		vm_area->filename_length = strlen(vm_area->filename);
		vm_area->memory_address = cprm_elf(cprm.is_64_bit, entry, start);
		vm_area->memory_size = cprm_elf(cprm.is_64_bit, entry, end) - cprm_elf(cprm.is_64_bit, entry, start);
		vm_area->file_size = vm_area->memory_size;
		vm_area->file_offset = cprm_elf(cprm.is_64_bit, entry, offset) * cprm_elf(cprm.is_64_bit, cprm.nt_file, page_size);
		vm_area->protection = VM_PROT_READ;
		vm_area->valid = true;

		break;
	}

	macho_coredump(&cprm);

	// now let's clean-up

	for (size_t i = 0; i < cprm.thread_info_count; ++i) {
		free(cprm.thread_infos[i].prstatus);
	}

	free(cprm.thread_infos);
	free(cprm.vm_areas);
	free(cprm.nt_file_filenames);
	free(cprm.nt_file);

	return 0;
};

static bool dump_emit(struct coredump_params* cprm, const void* buffer, size_t buffer_size) {
	while (buffer_size > 0) {
		ssize_t written = write(cprm->output_corefile, buffer, buffer_size);
		if (written < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			} else {
				return false;
			}
		} else {
			buffer_size -= written;
			buffer = (const char*)buffer + written;
		}
	}
	return true;
};

static bool dump_skip(struct coredump_params* cprm, size_t size) {
	// leaving holes seems to confuse LLDB
#if 0
	if (lseek(cprm->output_corefile, size, SEEK_CUR) < 0) {
		perror("lseek");
		return false;
	}
#else
	static char zeros[4096];
	while (sizeof(zeros) < size) {
		if (!dump_emit(cprm, zeros, sizeof(zeros)))
			return false;
		size -= sizeof(zeros);
	}
	if (!dump_emit(cprm, zeros, size))
		return false;
#endif
	return true;
};

static uint64_t dump_offset_get(struct coredump_params* cprm) {
	return lseek(cprm->output_corefile, 0, SEEK_CUR);
};

static bool dump_align(struct coredump_params* cprm, size_t alignment) {
	uint64_t offset = dump_offset_get(cprm);
	uint64_t aligned = round_up_pow2(offset, alignment);
	return dump_skip(cprm, aligned - offset);
};

// the following coredump code has been imported from the LKM and adapted for use in userspace

struct thread_flavor
{
	// preceded by struct thread_command and other flavors
	uint32_t flavor;
	uint32_t count;
	char state[0];
	// followed by x86_thread_state32_t, for example
};

static
void fill_thread_state32(x86_thread_state32_t* state, const struct thread_info* info)
{
	state->eax    = info->prstatus->elf32.general_registers.i386.eax;
	state->ebx    = info->prstatus->elf32.general_registers.i386.ebx;
	state->ecx    = info->prstatus->elf32.general_registers.i386.ecx;
	state->edx    = info->prstatus->elf32.general_registers.i386.edx;
	state->edi    = info->prstatus->elf32.general_registers.i386.edi;
	state->esi    = info->prstatus->elf32.general_registers.i386.esi;
	state->ebp    = info->prstatus->elf32.general_registers.i386.ebp;
	state->esp    = info->prstatus->elf32.general_registers.i386.esp;
	state->ss     = info->prstatus->elf32.general_registers.i386.ss;
	state->eflags = info->prstatus->elf32.general_registers.i386.eflags;
	state->eip    = info->prstatus->elf32.general_registers.i386.eip;
	state->cs     = info->prstatus->elf32.general_registers.i386.cs;
	state->ds     = info->prstatus->elf32.general_registers.i386.ds;
	state->es     = info->prstatus->elf32.general_registers.i386.es;
	state->fs     = info->prstatus->elf32.general_registers.i386.fs;
	state->gs     = info->prstatus->elf32.general_registers.i386.gs;
}

static
void fill_float_state32(x86_float_state32_t* state, const struct thread_info* info)
{
	// TODO
	memset(state, 0, sizeof(*state));
}

static
void fill_thread_state64(x86_thread_state64_t* state, const struct thread_info* info)
{
	state->rax    = info->prstatus->elf64.general_registers.x86_64.ax;
	state->rbx    = info->prstatus->elf64.general_registers.x86_64.bx;
	state->rcx    = info->prstatus->elf64.general_registers.x86_64.cx;
	state->rdx    = info->prstatus->elf64.general_registers.x86_64.dx;
	state->rdi    = info->prstatus->elf64.general_registers.x86_64.di;
	state->rsi    = info->prstatus->elf64.general_registers.x86_64.si;
	state->rbp    = info->prstatus->elf64.general_registers.x86_64.bp;
	state->rsp    = info->prstatus->elf64.general_registers.x86_64.sp;
	state->r8     = info->prstatus->elf64.general_registers.x86_64.r8;
	state->r9     = info->prstatus->elf64.general_registers.x86_64.r9;
	state->r10    = info->prstatus->elf64.general_registers.x86_64.r10;
	state->r11    = info->prstatus->elf64.general_registers.x86_64.r11;
	state->r12    = info->prstatus->elf64.general_registers.x86_64.r12;
	state->r13    = info->prstatus->elf64.general_registers.x86_64.r13;
	state->r14    = info->prstatus->elf64.general_registers.x86_64.r14;
	state->r15    = info->prstatus->elf64.general_registers.x86_64.r15;
	state->rip    = info->prstatus->elf64.general_registers.x86_64.ip;
	state->rflags = info->prstatus->elf64.general_registers.x86_64.flags;
	state->cs     = info->prstatus->elf64.general_registers.x86_64.cs;
	state->fs     = info->prstatus->elf64.general_registers.x86_64.fs;
	state->gs     = info->prstatus->elf64.general_registers.x86_64.gs;
}

static
void fill_float_state64(x86_float_state64_t* state, const struct thread_info* info)
{
	// TODO
	memset(state, 0, sizeof(*state));
}

static
bool macho_dump_headers32(struct coredump_params* cprm)
{
	// Count memory segments and threads
	unsigned int segs = cprm->vm_area_count;
	unsigned int threads = cprm->thread_info_count;
	struct mach_header mh;
	const uint32_t align_page_size = ALIGN_REGIONS ? sysconf(_SC_PAGE_SIZE) : 1;

	for (size_t i = 0; i < cprm->vm_area_count; ++i) {
		struct vm_area* vma = &cprm->vm_areas[i];

		if (!vma->valid) {
			// we don't dump inaccessible/invalid regions, so remove them from the count
			--segs;
		}
	}

	mh.magic = MH_MAGIC;
#ifdef __x86_64__
	mh.cputype = CPU_TYPE_X86;
	mh.cpusubtype = CPU_SUBTYPE_X86_ALL;
#else
#warning Missing code for this arch
#endif
	mh.filetype = MH_CORE;
	mh.ncmds = segs + threads;

	const int statesize = sizeof(x86_thread_state32_t) + (DUMP_FLOAT_STATE ? sizeof(x86_float_state32_t) : 0) + sizeof(struct thread_flavor) * (DUMP_FLOAT_STATE ? 2 : 1);
	mh.sizeofcmds = segs * sizeof(struct segment_command) + threads * (sizeof(struct thread_command) + statesize);
	mh.flags = 0;

	if (cprm->main_executable_path) {
		int tmpfd = open_file(cprm, cprm->main_executable_path, cprm->main_executable_path_length);
		if (tmpfd >= 0) {
			struct mach_header that_mh;
			if (read(tmpfd, &that_mh, sizeof(that_mh)) == sizeof(that_mh)) {
				mh.flags = that_mh.flags;
			}
			close(tmpfd);
		}
	} else {
		fprintf(stderr, "No main executable detected?\n");
	}

	if (!dump_emit(cprm, &mh, sizeof(mh)))
		goto fail;

	uint32_t file_offset = round_up_pow2(mh.sizeofcmds + sizeof(mh), align_page_size);

	// TODO: maybe align the initial offset to 0x1000

	for (size_t i = 0; i < cprm->vm_area_count; ++i) {
		struct vm_area* vma = &cprm->vm_areas[i];
		struct segment_command sc;

		if (!vma->valid) {
			// ignore inaccessible/invalid regions (like the __PAGEZERO segment, which may be really large)
			continue;
		}

		sc.cmd = LC_SEGMENT;
		sc.cmdsize = sizeof(sc);
		sc.segname[0] = 0;
		sc.nsects = 0;
		sc.flags = 0;
		sc.vmaddr = vma->memory_address;
		sc.vmsize = vma->memory_size;
		sc.fileoff = file_offset;

		vma->expected_offset = sc.fileoff;

		// see explanation in `macho_coredump` for why we always use the memory size
		sc.filesize = /*(vma->file_size == 0) ?*/ vma->memory_size /*: vma->file_size*/;
		sc.initprot = 0;

		if (vma->protection & PROT_READ)
			sc.initprot |= VM_PROT_READ;
		if (vma->protection & PROT_WRITE)
			sc.initprot |= VM_PROT_WRITE;
		if (vma->protection & PROT_EXEC)
			sc.initprot |= VM_PROT_EXECUTE;
		sc.maxprot = sc.initprot;

		if (!dump_emit(cprm, &sc, sizeof(sc)))
			goto fail;

		file_offset += round_up_pow2(sc.filesize, align_page_size);
	}

	const int memsize = sizeof(struct thread_command) + statesize;
	uint8_t* buffer = malloc(memsize);

	for (size_t i = 0; i < cprm->thread_info_count; ++i) {
		const struct thread_info* thread_info = &cprm->thread_infos[i];
		struct thread_command* tc = (struct thread_command*) buffer;
		struct thread_flavor* tf = (struct thread_flavor*)(tc+1);

		tc->cmd = LC_THREAD;
		tc->cmdsize = memsize;

		// General registers
		tf->flavor = x86_THREAD_STATE32;
		tf->count = x86_THREAD_STATE32_COUNT;

		fill_thread_state32((x86_thread_state32_t*)tf->state, thread_info);

#if DUMP_FLOAT_STATE
		// Float registers
		tf = (struct thread_flavor*) (tf->state + sizeof(x86_thread_state32_t));
		tf->flavor = x86_FLOAT_STATE32;
		tf->count = x86_FLOAT_STATE32_COUNT;

		fill_float_state32((x86_float_state32_t*)tf->state, thread_info);
#endif

		if (!dump_emit(cprm, buffer, memsize))
		{
			free(buffer);
			goto fail;
		}
	}
	free(buffer);

	return true;
fail:
	return false;
}

static
bool macho_dump_headers64(struct coredump_params* cprm)
{
	// Count memory segments and threads
	unsigned int segs = cprm->vm_area_count;
	unsigned int threads = cprm->thread_info_count;
	struct mach_header_64 mh;
	const uint64_t align_page_size = ALIGN_REGIONS ? sysconf(_SC_PAGE_SIZE) : 1;

	for (size_t i = 0; i < cprm->vm_area_count; ++i) {
		struct vm_area* vma = &cprm->vm_areas[i];

		if (!vma->valid) {
			// we don't dump inaccessible/invalid regions, so remove them from the count
			--segs;
		}
	}

	mh.magic = MH_MAGIC_64;
#ifdef __x86_64__
	mh.cputype = CPU_TYPE_X86_64;
	mh.cpusubtype = CPU_SUBTYPE_X86_64_ALL;
#else
#warning Missing code for this arch
#endif
	mh.filetype = MH_CORE;
	mh.ncmds = segs + threads;

	const int statesize = sizeof(x86_thread_state64_t) + (DUMP_FLOAT_STATE ? sizeof(x86_float_state64_t) : 0) + sizeof(struct thread_flavor) * (DUMP_FLOAT_STATE ? 2 : 1);
	mh.sizeofcmds = segs * sizeof(struct segment_command_64) + threads * (sizeof(struct thread_command) + statesize);
	mh.flags = 0;
	mh.reserved = 0;

	if (cprm->main_executable_path) {
		int tmpfd = open_file(cprm, cprm->main_executable_path, cprm->main_executable_path_length);
		if (tmpfd >= 0) {
			struct mach_header_64 that_mh;
			if (read(tmpfd, &that_mh, sizeof(that_mh)) == sizeof(that_mh)) {
				mh.flags = that_mh.flags;
			}
			close(tmpfd);
		}
	} else {
		fprintf(stderr, "No main executable detected?\n");
	}

	if (!dump_emit(cprm, &mh, sizeof(mh)))
		goto fail;

	uint64_t file_offset = round_up_pow2(mh.sizeofcmds + sizeof(mh), align_page_size);

	for (size_t i = 0; i < cprm->vm_area_count; ++i) {
		struct vm_area* vma = &cprm->vm_areas[i];
		struct segment_command_64 sc;

		if (!vma->valid) {
			// ignore inaccessible/invalid regions (like the __PAGEZERO segment, which may be really large)
			continue;
		}

		sc.cmd = LC_SEGMENT_64;
		sc.cmdsize = sizeof(sc);
		sc.segname[0] = 0;
		sc.nsects = 0;
		sc.flags = 0;
		sc.vmaddr = vma->memory_address;
		sc.vmsize = vma->memory_size;
		sc.fileoff = file_offset;

		vma->expected_offset = sc.fileoff;

		// see explanation in `macho_coredump` for why we always use the memory size
		sc.filesize = /*(vma->file_size == 0) ?*/ vma->memory_size /*: vma->file_size*/;
		sc.initprot = 0;

		if (vma->protection & PROT_READ)
			sc.initprot |= VM_PROT_READ;
		if (vma->protection & PROT_WRITE)
			sc.initprot |= VM_PROT_WRITE;
		if (vma->protection & PROT_EXEC)
			sc.initprot |= VM_PROT_EXECUTE;
		sc.maxprot = sc.initprot;

		if (!dump_emit(cprm, &sc, sizeof(sc)))
			goto fail;

		file_offset += round_up_pow2(sc.filesize, align_page_size);
	}

	const int memsize = sizeof(struct thread_command) + statesize;
	uint8_t* buffer = malloc(memsize);

	for (size_t i = 0; i < cprm->thread_info_count; ++i) {
		const struct thread_info* thread_info = &cprm->thread_infos[i];
		struct thread_command* tc = (struct thread_command*) buffer;
		struct thread_flavor* tf = (struct thread_flavor*)(tc+1);

		tc->cmd = LC_THREAD;
		tc->cmdsize = memsize;

		// General registers
		tf->flavor = x86_THREAD_STATE64;
		tf->count = x86_THREAD_STATE64_COUNT;

		fill_thread_state64((x86_thread_state64_t*)tf->state, thread_info);

#if DUMP_FLOAT_STATE
		// Float registers
		tf = (struct thread_flavor*) (tf->state + sizeof(x86_thread_state64_t));
		tf->flavor = x86_FLOAT_STATE64;
		tf->count = x86_FLOAT_STATE64_COUNT;

		fill_float_state64((x86_float_state64_t*)tf->state, thread_info);
#endif

		if (!dump_emit(cprm, buffer, memsize))
		{
			free(buffer);
			goto fail;
		}
	}
	free(buffer);

	return true;
fail:
	return false;
}

void macho_coredump(struct coredump_params* cprm)
{
	// Write the Mach-O header and loader commands
	if (!cprm->is_64_bit)
	{
		// 32-bit executables
		if (!macho_dump_headers32(cprm))
			exit(EXIT_FAILURE);
	}
	else
	{
		// 64-bit executables
		if (!macho_dump_headers64(cprm))
			exit(EXIT_FAILURE);
	}

	// Dump memory contents

	uint64_t align_page_size = ALIGN_REGIONS ? sysconf(_SC_PAGESIZE) : 1;

	if (!dump_align(cprm, align_page_size))
		exit(EXIT_FAILURE);

	// Inspired by elf_core_dump()
	for (size_t i = 0; i < cprm->vm_area_count; ++i) {
		const struct vm_area* vma = &cprm->vm_areas[i];
		unsigned long addr;

		if (!vma->valid) {
			continue;
		}

		uint64_t curr = dump_offset_get(cprm);
		if (curr != vma->expected_offset) {
			fprintf(stderr, "Expected offset %lx, got offset %lx\n", vma->expected_offset, curr);
			exit(EXIT_FAILURE);
		}

		if (vma->file_size == 0) {
			// this region needs to be zeroed out
			//
			// it's likely a region that was allocated with mmap but never accessed,
			// so it remained zero-filled and there was no need to include it in the ELF core file.
			if (!dump_skip(cprm, vma->memory_size))
				exit(EXIT_FAILURE);
			continue;
		}

		if (vma->filename) {
			if (vma->filename_length >= sizeof("/dev/") - 1 && strncmp(vma->filename, "/dev/", sizeof("/dev/") - 1) == 0) {
				printf("Warning: skipping device \"%s\"\n", vma->filename);
				if (!dump_skip(cprm, vma->file_size)) {
					exit(EXIT_FAILURE);
				}
				continue;
			}

			int fd = open_file(cprm, vma->filename, vma->filename_length);
			if (fd < 0) {
				fprintf(stderr, "Warning: failed to open %s: %d (%s)\n", vma->filename, errno, strerror(errno));
				//exit(EXIT_FAILURE);
				// just zero it out
				if (!dump_skip(cprm, vma->file_size)) {
					exit(EXIT_FAILURE);
				}
				continue;
			}

			uintptr_t aligned_offset = round_down_pow2(vma->file_offset, sysconf(_SC_PAGESIZE));
			size_t aligned_size = round_up_pow2(vma->file_size + (vma->file_offset - aligned_offset), sysconf(_SC_PAGESIZE));
			void* mapping = mmap(NULL, aligned_size, PROT_READ, MAP_PRIVATE, fd, aligned_offset);
			if (mapping == MAP_FAILED) {
				perror("mmap");
				exit(EXIT_FAILURE);
			}
			const void* start = (const char*)mapping + (vma->file_offset - aligned_offset);

			if (!dump_emit(cprm, start, vma->file_size))
				exit(EXIT_FAILURE);

			if (munmap(mapping, aligned_size) < 0) {
				perror("munmap");
				exit(EXIT_FAILURE);
			}

			close(fd);
		} else {
			if (!dump_emit(cprm, (const char*)cprm->input_corefile_mapping + vma->file_offset, vma->file_size))
				exit(EXIT_FAILURE);
		}

		// LLDB assumes that contiguous memory regions are written to the file as contiguous memory regions, even if they're written as separate segments,
		// and (notably) even if their file sizes differ from their memory sizes (which would mean they're not actually fully contiguous in the file).
		// this is a fairly reasonable assumption, but this means that, in cases where the memory size is greater than the file size, we have to manually
		// zero-out the extra region space so LLDB will read them properly.
		if (!dump_skip(cprm, vma->memory_size - vma->file_size))
			exit(EXIT_FAILURE);

		if (!dump_align(cprm, align_page_size))
			exit(EXIT_FAILURE);
	}
}
