
// Definitions:
// FUNCTION_NAME (load32/load64)
// SEGMENT_STRUCT (segment_command/SEGMENT_STRUCT)
// SEGMENT_COMMAND (LC_SEGMENT/SEGMENT_COMMAND)
// MACH_HEADER_STRUCT (mach_header/MACH_HEADER_STRUCT)
// SECTION_STRUCT (section/SECTION_STRUCT)

#if defined(GEN_64BIT)
#   define FUNCTION_NAME load64
#   define SEGMENT_STRUCT segment_command_64
#   define SEGMENT_COMMAND LC_SEGMENT_64
#   define MACH_HEADER_STRUCT mach_header_64
#   define SECTION_STRUCT section_64
#	define MAP_EXTRA 0
#elif defined(GEN_32BIT)
#   define FUNCTION_NAME load32
#   define SEGMENT_STRUCT segment_command
#   define SEGMENT_COMMAND LC_SEGMENT
#   define MACH_HEADER_STRUCT mach_header
#   define SECTION_STRUCT section
#	define MAP_EXTRA MAP_32BIT
#else
#   error See above
#endif

void FUNCTION_NAME(int fd, uintptr_t* entryPoint_out, uintptr_t* mh_out)
{
	struct MACH_HEADER_STRUCT header;
	uint8_t* cmds;
	uintptr_t entryPoint = 0, entryPointDylinker = 0;
	struct MACH_HEADER_STRUCT* mappedHeader = NULL;
	uintptr_t slide = 0;
	uintptr_t mmapSize = 0;
	bool pie = false;
	uint32_t fat_offset;

	fat_offset = lseek(fd, 0, SEEK_CUR);

	if (read(fd, &header, sizeof(header)) != sizeof(header))
	{
		fprintf(stderr, "Cannot read the mach header.\n");
		exit(1);
	}

	cmds = (uint8_t*) malloc(header.sizeofcmds);
	if (!cmds)
	{
		fprintf(stderr, "Cannot allocate %u bytes for loader commands.\n", header.sizeofcmds);
		exit(1);
	}

	if (read(fd, cmds, header.sizeofcmds) != header.sizeofcmds)
	{
		fprintf(stderr, "Cannot read loader commands.\n");
		exit(1);
	}

	if ((header.filetype == MH_EXECUTE && header.flags & MH_PIE) || header.filetype == MH_DYLINKER)
	{
		uintptr_t base = -1;

		// Go through all SEGMENT_COMMAND commands to get the total continuous range required.
		for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
		{
			struct SEGMENT_STRUCT* seg = (struct SEGMENT_STRUCT*) &cmds[p];

			// Load commands are always sorted, so this will get us the maximum address.
			if (seg->cmd == SEGMENT_COMMAND)
			{
				if (base == -1)
				{
					base = seg->vmaddr;
					if (base != 0 && header.filetype == MH_DYLINKER)
						goto no_slide;
				}
				mmapSize = seg->vmaddr + seg->vmsize - base;
			}

			p += seg->cmdsize;
		}

		slide = (uintptr_t) mmap((void*) base, mmapSize, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_EXTRA, -1, 0);
		if (slide == (uintptr_t)MAP_FAILED)
		{
			fprintf(stderr, "Cannot mmap anonymous memory range: %s", strerror(errno));
			exit(1);
		}

		pie = true;
	}
no_slide:

	for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
	{
		struct load_command* lc;

		lc = (struct load_command*) &cmds[p];

		switch (lc->cmd)
		{
			case SEGMENT_COMMAND:
			{
				struct SEGMENT_STRUCT* seg = (struct SEGMENT_STRUCT*) lc;
				void* rv;

				if (memcmp(SEG_PAGEZERO, seg->segname, sizeof(SEG_PAGEZERO)) == 0)
				{
					// Let's skip pagezero and map a single page at address 0 elsewhere.
					p += lc->cmdsize;
					continue;
				}

				if (seg->filesize < seg->vmsize)
				{
					if (slide != 0)
					{
						// Some segments' filesize != vmsize, thus this mprotect().
						if (mprotect((void*) (seg->vmaddr + slide), seg->vmsize, native_prot(seg->maxprot)) != 0)
						{
							fprintf(stderr, "Cannot mprotect for segment %s: %s\n", seg->segname, strerror(errno));
							exit(1);
						}
					}
					else
					{
						size_t size = seg->vmsize - seg->filesize;
						rv = mmap((void*) PAGE_ALIGN(seg->vmaddr + seg->vmsize - size), PAGE_ROUNDUP(size), native_prot(seg->maxprot),
								MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
					}
				}

				if (seg->filesize > 0)
				{
					rv = mmap((void*) (seg->vmaddr + slide), seg->filesize, native_prot(seg->maxprot),
							MAP_FIXED | MAP_PRIVATE, fd, seg->fileoff + fat_offset);
					if (rv == (void*)MAP_FAILED)
					{
						fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)(uintptr_t)seg->vmaddr, strerror(errno));
						exit(1);
					}
				
					if (seg->fileoff == 0)
						mappedHeader = (struct MACH_HEADER_STRUCT*) (seg->vmaddr + slide);
				}

				if (strcmp(SEG_DATA, seg->segname) == 0)
				{
					// Look for section named __all_image_info for GDB integration
					struct SECTION_STRUCT* sect = (struct SECTION_STRUCT*) (seg+1);
					struct SECTION_STRUCT* end = (struct SECTION_STRUCT*) (&cmds[p + lc->cmdsize]);

					while (sect < end)
					{
						if (strncmp(sect->sectname, "__all_image_info", 16) == 0)
						{
#ifdef GEN_64BIT
							setup_gdb_notifications(slide, sect->addr);
#endif
							break;
						}
						sect++;
					}
				}
				break;
			}
			case LC_UNIXTHREAD:
			{
#ifdef GEN_64BIT
				entryPoint = ((uint64_t*) lc)[18];
#endif
#ifdef GEN_32BIT
				entryPoint = ((uint32_t*) lc)[14];
#endif
				entryPoint += slide;
				break;
			}
			case LC_LOAD_DYLINKER:
			{
				struct dylinker_command* dy = (struct dylinker_command*) lc;
				char path[4096];
				size_t length = dy->cmdsize - dy->name.offset;

				if (length > sizeof(path)-1)
				{
					fprintf(stderr, "Dynamic linker name too long: %zu\n", length);
					exit(1);
				}

				memcpy(path, ((char*) dy) + dy->name.offset, length);
				path[length] = '\0';

				apply_root_path(path);

#ifdef GEN_64BIT
				load(path, &entryPointDylinker, NULL, CPU_TYPE_X86_64, NULL);
#endif
#ifdef GEN_32BIT
				load(path, &entryPointDylinker, NULL, CPU_TYPE_X86, NULL);
#endif

				break;
			}
			case LC_MAIN:
			{
				struct entry_point_command* ee = (struct entry_point_command*) lc;
				if (ee->stacksize > stack_size)
					stack_size = ee->stacksize;
				break;
			}
			case LC_UUID:
			{
				if (header.filetype == MH_EXECUTE)
				{
					struct uuid_command* ue = (struct uuid_command*) lc;
					memcpy(exe_uuid, ue->uuid, sizeof(exe_uuid));
				}
				break;
			}
		}

		p += lc->cmdsize;
	}

	free(cmds);

	if (entryPoint_out != NULL)
		*entryPoint_out = entryPointDylinker ? entryPointDylinker : entryPoint;
	if (mh_out != NULL)
		*mh_out = (uintptr_t) mappedHeader;
}


#undef FUNCTION_NAME
#undef SEGMENT_STRUCT
#undef SEGMENT_COMMAND
#undef MACH_HEADER_STRUCT
#undef SECTION_STRUCT
#undef MAP_EXTRA

