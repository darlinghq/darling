/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * adhoc-sign: gives linked Mach-O images an ad-hoc code signature, in place.
 *
 * Usage: adhoc-sign [--identifier=<id>] <file>...
 *
 * Each executable, dylib, bundle or dynamic linker gets an embedded signature of the same shape
 * as `codesign -s -` produces: a SuperBlob with one SHA-256 CodeDirectory (version 0x20400, 4 KiB
 * pages, 16 KiB on arm64), an empty requirement set and an empty CMS wrapper. Each slice of a
 * universal file is signed on its own and the file is laid out again. An existing signature is
 * replaced. Files that are not Mach-O images (scripts, archives, object files) are left alone.
 *
 * The signature format is the one described by the public <kern/cs_blobs.h>, <mach-o/loader.h>
 * and <mach-o/fat.h> headers. The digests are computed with OpenSSL's libcrypto.
 *
 * The identifier defaults to the file name. Exit status: 0 on success, 1 if a file could not be
 * signed, 2 on a usage error.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mach-o/fat.h>
#include <mach-o/loader.h>

#include <openssl/evp.h>

#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Mach-O headers are read in host byte order; a little-endian host is required"
#endif

/* values from the public <kern/cs_blobs.h> */
#define CSMAGIC_REQUIREMENTS          0xfade0c01u
#define CSMAGIC_CODEDIRECTORY         0xfade0c02u
#define CSMAGIC_EMBEDDED_SIGNATURE    0xfade0cc0u
#define CSMAGIC_BLOBWRAPPER           0xfade0b01u
#define CSSLOT_CODEDIRECTORY          0u
#define CSSLOT_INFOSLOT               1u
#define CSSLOT_REQUIREMENTS           2u
#define CSSLOT_SIGNATURESLOT          0x10000u
#define CS_ADHOC                      0x2u
#define CS_HASHTYPE_SHA256            2u
#define CS_SHA256_LEN                 32u
#define CS_SUPPORTSEXECSEG            0x20400u
#define CS_EXECSEG_MAIN_BINARY        0x1u

/* from the public <mach/machine.h>, which the Mach-O headers leave out on other hosts */
#define ADHOC_CPU_TYPE_ARM64          0x0100000c

/* the fixed part of a version 0x20400 CodeDirectory */
#define CD_HEADER_SIZE 88u
/* SuperBlob header with three index entries */
#define SUPERBLOB_HEADER_SIZE (12u + 3u * 8u)
#define REQUIREMENTS_SIZE 12u
#define BLOBWRAPPER_SIZE 8u
/* the number of special slots: Info.plist (unbound) and requirements */
#define SPECIAL_SLOTS 2u
/* the signature's offset and size are kept 16-byte aligned, as `codesign` does */
#define SIGNATURE_ALIGN 16u

static const char* progname = "adhoc-sign";

struct buffer {
	uint8_t* data;
	size_t length;
};

static void put_be32(uint8_t* p, uint32_t v) {
	p[0] = (uint8_t)(v >> 24);
	p[1] = (uint8_t)(v >> 16);
	p[2] = (uint8_t)(v >> 8);
	p[3] = (uint8_t)v;
}

static void put_be64(uint8_t* p, uint64_t v) {
	put_be32(p, (uint32_t)(v >> 32));
	put_be32(p + 4, (uint32_t)v);
}

static uint32_t get_be32(const uint8_t* p) {
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static uint64_t get_be64(const uint8_t* p) {
	return ((uint64_t)get_be32(p) << 32) | get_be32(p + 4);
}

static uint64_t align_up(uint64_t value, uint64_t alignment) {
	return (value + alignment - 1) & ~(alignment - 1);
}

static bool sha256(const void* data, size_t length, uint8_t out[CS_SHA256_LEN]) {
	unsigned int out_length = 0;
	return EVP_Digest(data, length, out, &out_length, EVP_sha256(), NULL) == 1 && out_length == CS_SHA256_LEN;
}

/* what kind of file a buffer holds */
enum kind {
	KIND_OTHER,  /* not a Mach-O image: left alone */
	KIND_THIN,   /* a little-endian Mach-O image that can be signed */
	KIND_FAT,    /* a universal file */
};

static bool signable_filetype(uint32_t filetype) {
	switch (filetype) {
		case MH_EXECUTE:
		case MH_DYLIB:
		case MH_DYLINKER:
		case MH_BUNDLE:
			return true;
		default:
			return false;
	}
}

static enum kind classify(const uint8_t* data, size_t length) {
	uint32_t magic;

	if (length < 8) {
		return KIND_OTHER;
	}
	memcpy(&magic, data, sizeof(magic));

	if (magic == MH_MAGIC || magic == MH_MAGIC_64) {
		if (length < sizeof(struct mach_header)) {
			return KIND_OTHER;
		}
		const struct mach_header* header = (const struct mach_header*)data;
		return signable_filetype(header->filetype) ? KIND_THIN : KIND_OTHER;
	}

	uint32_t be_magic = get_be32(data);
	if (be_magic == FAT_MAGIC || be_magic == FAT_MAGIC_64) {
		/*
		 * Java class files share FAT_MAGIC; their next field (the version) reads as a
		 * slice count far larger than any universal file has.
		 */
		uint32_t count = get_be32(data + 4);
		return (count > 0 && count <= 64) ? KIND_FAT : KIND_OTHER;
	}

	return KIND_OTHER;
}

struct segment {
	uint8_t* command;
	uint64_t vmaddr;
	uint64_t vmsize;
	uint64_t fileoff;
	uint64_t filesize;
};

static void read_segment(uint8_t* command, bool is_64, struct segment* segment) {
	segment->command = command;
	if (is_64) {
		const struct segment_command_64* seg = (const struct segment_command_64*)command;
		segment->vmaddr = seg->vmaddr;
		segment->vmsize = seg->vmsize;
		segment->fileoff = seg->fileoff;
		segment->filesize = seg->filesize;
	} else {
		const struct segment_command* seg = (const struct segment_command*)command;
		segment->vmaddr = seg->vmaddr;
		segment->vmsize = seg->vmsize;
		segment->fileoff = seg->fileoff;
		segment->filesize = seg->filesize;
	}
}

static bool is_zerofill(uint32_t flags) {
	uint32_t type = flags & SECTION_TYPE;
	return type == S_ZEROFILL || type == S_GB_ZEROFILL || type == S_THREAD_LOCAL_ZEROFILL;
}

/*
 * Signs one thin image. @p in holds the image; the signed image is returned in @p out.
 */
static int sign_image(const char* name, const uint8_t* in, size_t in_length, const char* identifier, struct buffer* out) {
	struct mach_header header;
	bool is_64;
	size_t header_size;

	memcpy(&header, in, sizeof(header));
	is_64 = (header.magic == MH_MAGIC_64);
	header_size = is_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header);

	if (in_length < header_size || header.sizeofcmds > in_length - header_size) {
		fprintf(stderr, "%s: %s: the load commands are truncated\n", progname, name);
		return -1;
	}

	/* the new header and load commands are built in a copy that has room for one more command */
	size_t commands_end = header_size + header.sizeofcmds;
	uint8_t* commands = calloc(1, commands_end + sizeof(struct linkedit_data_command));
	if (!commands) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, name);
		return -1;
	}
	memcpy(commands, in, commands_end);

	struct segment text = { 0 }, linkedit = { 0 };
	struct linkedit_data_command* signature_command = NULL;
	uint64_t first_content_offset = UINT64_MAX;
	uint8_t* p = commands + header_size;

	for (uint32_t i = 0; i < header.ncmds; ++i) {
		struct load_command lc;
		size_t remaining = commands_end - (size_t)(p - commands);

		if (remaining < sizeof(lc)) {
			goto truncated;
		}
		memcpy(&lc, p, sizeof(lc));
		if (lc.cmdsize < sizeof(lc) || lc.cmdsize > remaining) {
			goto truncated;
		}

		if (lc.cmd == (is_64 ? LC_SEGMENT_64 : LC_SEGMENT)) {
			size_t segment_size = is_64 ? sizeof(struct segment_command_64) : sizeof(struct segment_command);
			size_t section_size = is_64 ? sizeof(struct section_64) : sizeof(struct section);
			struct segment segment;
			char segname[17] = { 0 };
			uint32_t nsects;

			if (lc.cmdsize < segment_size) {
				goto truncated;
			}
			memcpy(segname, p + offsetof(struct segment_command, segname), 16);
			nsects = is_64 ? ((struct segment_command_64*)p)->nsects : ((struct segment_command*)p)->nsects;
			if (nsects > (lc.cmdsize - segment_size) / section_size) {
				goto truncated;
			}
			read_segment(p, is_64, &segment);

			if (strcmp(segname, SEG_TEXT) == 0) {
				text = segment;
			} else if (strcmp(segname, SEG_LINKEDIT) == 0) {
				linkedit = segment;
			}

			/*
			 * The load commands may grow up to the first section placed in the file (even an
			 * empty one) or the first segment after the header, whichever comes first.
			 */
			if (segment.fileoff != 0 && segment.filesize != 0 && segment.fileoff < first_content_offset) {
				first_content_offset = segment.fileoff;
			}
			for (uint32_t s = 0; s < nsects; ++s) {
				const uint8_t* section = p + segment_size + s * section_size;
				uint32_t offset, flags;

				if (is_64) {
					const struct section_64* sect = (const struct section_64*)section;
					offset = sect->offset;
					flags = sect->flags;
				} else {
					const struct section* sect = (const struct section*)section;
					offset = sect->offset;
					flags = sect->flags;
				}
				if (offset != 0 && !is_zerofill(flags) && offset < first_content_offset) {
					first_content_offset = offset;
				}
			}
		} else if (lc.cmd == LC_CODE_SIGNATURE) {
			if (lc.cmdsize < sizeof(struct linkedit_data_command) || signature_command) {
				fprintf(stderr, "%s: %s: malformed LC_CODE_SIGNATURE\n", progname, name);
				goto fail;
			}
			signature_command = (struct linkedit_data_command*)p;
		}

		p += lc.cmdsize;
	}

	if (!linkedit.command || !text.command) {
		fprintf(stderr, "%s: %s: the image has no __TEXT or __LINKEDIT segment\n", progname, name);
		goto fail;
	}

	/*
	 * The signature goes at the end of __LINKEDIT, which must be the end of the image. An
	 * existing signature is dropped first.
	 */
	uint64_t code_end = in_length;
	if (signature_command) {
		uint64_t end = (uint64_t)signature_command->dataoff + signature_command->datasize;
		if (end != in_length || signature_command->dataoff < linkedit.fileoff) {
			fprintf(stderr, "%s: %s: the existing signature is not at the end of __LINKEDIT\n", progname, name);
			goto fail;
		}
		code_end = signature_command->dataoff;
	}
	if (linkedit.fileoff + linkedit.filesize != in_length) {
		fprintf(stderr, "%s: %s: __LINKEDIT does not end the image\n", progname, name);
		goto fail;
	}

	if (!signature_command) {
		size_t new_end = commands_end + sizeof(struct linkedit_data_command);
		if (first_content_offset == UINT64_MAX || new_end > first_content_offset) {
			fprintf(stderr, "%s: %s: no room for LC_CODE_SIGNATURE after the load commands\n", progname, name);
			goto fail;
		}
		signature_command = (struct linkedit_data_command*)(commands + commands_end);
		signature_command->cmd = LC_CODE_SIGNATURE;
		signature_command->cmdsize = sizeof(struct linkedit_data_command);
		((struct mach_header*)commands)->ncmds = header.ncmds + 1;
		((struct mach_header*)commands)->sizeofcmds = header.sizeofcmds + sizeof(struct linkedit_data_command);
		commands_end = new_end;
	}

	uint32_t page_size_log2 = (header.cputype == ADHOC_CPU_TYPE_ARM64) ? 14 : 12;
	uint64_t page_size = 1ull << page_size_log2;
	uint64_t vm_page_size = page_size;

	uint64_t code_limit = align_up(code_end, SIGNATURE_ALIGN);
	uint64_t code_slots = (code_limit + page_size - 1) / page_size;
	size_t identifier_length = strlen(identifier) + 1;
	uint64_t cd_length = CD_HEADER_SIZE + identifier_length + (SPECIAL_SLOTS + code_slots) * CS_SHA256_LEN;
	uint64_t superblob_length = SUPERBLOB_HEADER_SIZE + cd_length + REQUIREMENTS_SIZE + BLOBWRAPPER_SIZE;
	uint64_t signature_size = align_up(superblob_length, SIGNATURE_ALIGN);

	if (code_limit + signature_size > UINT32_MAX) {
		fprintf(stderr, "%s: %s: the image is too large to sign\n", progname, name);
		goto fail;
	}

	signature_command->dataoff = (uint32_t)code_limit;
	signature_command->datasize = (uint32_t)signature_size;

	/* __LINKEDIT now ends with the signature */
	uint64_t linkedit_filesize = code_limit + signature_size - linkedit.fileoff;
	uint64_t linkedit_vmsize = align_up(linkedit_filesize, vm_page_size);
	if (linkedit_vmsize < linkedit.vmsize) {
		linkedit_vmsize = linkedit.vmsize;
	}
	if (linkedit_vmsize != linkedit.vmsize) {
		/* the larger segment must not run into another one */
		p = commands + header_size;
		for (uint32_t i = 0; i < header.ncmds; ++i) {
			struct load_command lc;
			memcpy(&lc, p, sizeof(lc));
			if (lc.cmd == (is_64 ? LC_SEGMENT_64 : LC_SEGMENT) && p != linkedit.command) {
				struct segment other;
				read_segment(p, is_64, &other);
				if (other.vmsize != 0 && other.vmaddr >= linkedit.vmaddr && other.vmaddr < linkedit.vmaddr + linkedit_vmsize) {
					fprintf(stderr, "%s: %s: no address space to grow __LINKEDIT\n", progname, name);
					goto fail;
				}
			}
			p += lc.cmdsize;
		}
	}
	if (is_64) {
		struct segment_command_64* seg = (struct segment_command_64*)linkedit.command;
		seg->filesize = linkedit_filesize;
		seg->vmsize = linkedit_vmsize;
	} else {
		struct segment_command* seg = (struct segment_command*)linkedit.command;
		if (linkedit_vmsize > UINT32_MAX || linkedit.vmaddr + linkedit_vmsize > (uint64_t)UINT32_MAX + 1) {
			fprintf(stderr, "%s: %s: no address space to grow __LINKEDIT\n", progname, name);
			goto fail;
		}
		seg->filesize = (uint32_t)linkedit_filesize;
		seg->vmsize = (uint32_t)linkedit_vmsize;
	}

	/* the signed image: header, the rest of the code, padding, signature */
	out->length = (size_t)(code_limit + signature_size);
	out->data = calloc(1, out->length);
	if (!out->data) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, name);
		goto fail;
	}
	memcpy(out->data, commands, commands_end);
	if (code_end > commands_end) {
		memcpy(out->data + commands_end, in + commands_end, (size_t)(code_end - commands_end));
	}

	uint8_t* superblob = out->data + code_limit;
	uint32_t cd_offset = SUPERBLOB_HEADER_SIZE;
	uint32_t requirements_offset = cd_offset + (uint32_t)cd_length;
	uint32_t wrapper_offset = requirements_offset + REQUIREMENTS_SIZE;

	put_be32(superblob + 0, CSMAGIC_EMBEDDED_SIGNATURE);
	put_be32(superblob + 4, (uint32_t)superblob_length);
	put_be32(superblob + 8, 3);
	put_be32(superblob + 12, CSSLOT_CODEDIRECTORY);
	put_be32(superblob + 16, cd_offset);
	put_be32(superblob + 20, CSSLOT_REQUIREMENTS);
	put_be32(superblob + 24, requirements_offset);
	put_be32(superblob + 28, CSSLOT_SIGNATURESLOT);
	put_be32(superblob + 32, wrapper_offset);

	/* an empty requirement set and an empty CMS wrapper (no CMS signature: ad-hoc) */
	uint8_t* requirements = superblob + requirements_offset;
	put_be32(requirements + 0, CSMAGIC_REQUIREMENTS);
	put_be32(requirements + 4, REQUIREMENTS_SIZE);
	put_be32(requirements + 8, 0);

	uint8_t* wrapper = superblob + wrapper_offset;
	put_be32(wrapper + 0, CSMAGIC_BLOBWRAPPER);
	put_be32(wrapper + 4, BLOBWRAPPER_SIZE);

	uint8_t* cd = superblob + cd_offset;
	uint32_t hash_offset = CD_HEADER_SIZE + (uint32_t)identifier_length + SPECIAL_SLOTS * CS_SHA256_LEN;
	put_be32(cd + 0, CSMAGIC_CODEDIRECTORY);
	put_be32(cd + 4, (uint32_t)cd_length);
	put_be32(cd + 8, CS_SUPPORTSEXECSEG);         /* version */
	put_be32(cd + 12, CS_ADHOC);                  /* flags */
	put_be32(cd + 16, hash_offset);
	put_be32(cd + 20, CD_HEADER_SIZE);            /* identOffset */
	put_be32(cd + 24, SPECIAL_SLOTS);
	put_be32(cd + 28, (uint32_t)code_slots);
	put_be32(cd + 32, (uint32_t)code_limit);
	cd[36] = CS_SHA256_LEN;                       /* hashSize */
	cd[37] = CS_HASHTYPE_SHA256;
	cd[38] = 0;                                   /* platform */
	cd[39] = (uint8_t)page_size_log2;
	/* spare2, scatterOffset, teamOffset, spare3 and codeLimit64 stay zero */
	put_be64(cd + 64, text.fileoff);              /* execSegBase */
	put_be64(cd + 72, text.filesize);             /* execSegLimit */
	put_be64(cd + 80, header.filetype == MH_EXECUTE ? CS_EXECSEG_MAIN_BINARY : 0);
	memcpy(cd + CD_HEADER_SIZE, identifier, identifier_length);

	/* special slot n is n hashes before the code slots; the Info.plist slot stays zero */
	if (!sha256(requirements, REQUIREMENTS_SIZE, cd + hash_offset - CSSLOT_REQUIREMENTS * CS_SHA256_LEN)) {
		goto digest_failed;
	}
	for (uint64_t i = 0; i < code_slots; ++i) {
		uint64_t start = i * page_size;
		uint64_t length = (code_limit - start < page_size) ? code_limit - start : page_size;
		if (!sha256(out->data + start, (size_t)length, cd + hash_offset + i * CS_SHA256_LEN)) {
			goto digest_failed;
		}
	}

	free(commands);
	return 0;

digest_failed:
	fprintf(stderr, "%s: %s: SHA-256 failed\n", progname, name);
	free(out->data);
	out->data = NULL;
	goto fail;
truncated:
	fprintf(stderr, "%s: %s: malformed load commands\n", progname, name);
fail:
	free(commands);
	return -1;
}

/*
 * Signs every slice of a universal file and lays the file out again, keeping each slice's
 * alignment. Returns 1, without output, if no slice is an image.
 */
static int sign_fat(const char* name, const uint8_t* in, size_t in_length, const char* identifier, struct buffer* out) {
	bool is_64 = get_be32(in) == FAT_MAGIC_64;
	uint32_t count = get_be32(in + 4);
	size_t entry_size = is_64 ? 32 : 20;
	size_t table_end = 8 + count * entry_size;
	struct buffer* slices;
	uint64_t* offsets;
	int result = -1;

	if (table_end > in_length) {
		fprintf(stderr, "%s: %s: truncated universal header\n", progname, name);
		return -1;
	}

	slices = calloc(count, sizeof(*slices));
	offsets = calloc(count, sizeof(*offsets));
	if (!slices || !offsets) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, name);
		goto out;
	}

	/* a universal file without images (e.g. of object files) is left alone */
	bool any_image = false;
	for (uint32_t i = 0; i < count; ++i) {
		const uint8_t* entry = in + 8 + i * entry_size;
		uint64_t offset = is_64 ? get_be64(entry + 8) : get_be32(entry + 8);
		uint64_t size = is_64 ? get_be64(entry + 16) : get_be32(entry + 12);
		if (offset <= in_length && size <= in_length - offset && classify(in + offset, (size_t)size) == KIND_THIN) {
			any_image = true;
		}
	}
	if (!any_image) {
		result = 1;
		goto out;
	}

	uint64_t end = table_end;
	for (uint32_t i = 0; i < count; ++i) {
		const uint8_t* entry = in + 8 + i * entry_size;
		uint64_t offset = is_64 ? get_be64(entry + 8) : get_be32(entry + 8);
		uint64_t size = is_64 ? get_be64(entry + 16) : get_be32(entry + 12);
		uint32_t align = get_be32(entry + (is_64 ? 24 : 16));
		const uint8_t* slice = in + offset;

		if (offset < table_end || offset > in_length || size > in_length - offset || align > 15) {
			fprintf(stderr, "%s: %s: slice %u lies outside the file\n", progname, name, i);
			goto out;
		}

		if (classify(slice, (size_t)size) == KIND_THIN) {
			if (sign_image(name, slice, (size_t)size, identifier, &slices[i]) != 0) {
				goto out;
			}
		} else {
			/* not a signable image (e.g. an object file or an archive): copied as it is */
			slices[i].data = malloc(size ? (size_t)size : 1);
			if (!slices[i].data) {
				fprintf(stderr, "%s: %s: out of memory\n", progname, name);
				goto out;
			}
			memcpy(slices[i].data, slice, (size_t)size);
			slices[i].length = (size_t)size;
		}

		offsets[i] = align_up(end, 1ull << align);
		end = offsets[i] + slices[i].length;
		if (!is_64 && end > UINT32_MAX) {
			fprintf(stderr, "%s: %s: the universal file is too large\n", progname, name);
			goto out;
		}
	}

	out->length = (size_t)end;
	out->data = calloc(1, out->length);
	if (!out->data) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, name);
		goto out;
	}
	memcpy(out->data, in, table_end);
	for (uint32_t i = 0; i < count; ++i) {
		uint8_t* entry = out->data + 8 + i * entry_size;
		if (is_64) {
			put_be64(entry + 8, offsets[i]);
			put_be64(entry + 16, slices[i].length);
		} else {
			put_be32(entry + 8, (uint32_t)offsets[i]);
			put_be32(entry + 12, (uint32_t)slices[i].length);
		}
		memcpy(out->data + offsets[i], slices[i].data, slices[i].length);
	}
	result = 0;

out:
	if (slices) {
		for (uint32_t i = 0; i < count; ++i) {
			free(slices[i].data);
		}
	}
	free(slices);
	free(offsets);
	return result;
}

static int read_file(const char* path, struct buffer* buffer, struct stat* st) {
	int fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot open %s: %s\n", progname, path, strerror(errno));
		return -1;
	}
	if (fstat(fd, st) != 0 || !S_ISREG(st->st_mode)) {
		fprintf(stderr, "%s: %s is not a regular file\n", progname, path);
		close(fd);
		return -1;
	}

	buffer->length = (size_t)st->st_size;
	buffer->data = malloc(buffer->length ? buffer->length : 1);
	if (!buffer->data) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, path);
		close(fd);
		return -1;
	}

	size_t done = 0;
	while (done < buffer->length) {
		ssize_t n = read(fd, buffer->data + done, buffer->length - done);
		if (n < 0 && errno == EINTR) {
			continue;
		}
		if (n <= 0) {
			fprintf(stderr, "%s: cannot read %s\n", progname, path);
			free(buffer->data);
			close(fd);
			return -1;
		}
		done += (size_t)n;
	}

	close(fd);
	return 0;
}

/* replaces @p path atomically, keeping its permissions */
static int write_file(const char* path, const struct buffer* buffer, const struct stat* st) {
	size_t path_length = strlen(path);
	char* temp = malloc(path_length + sizeof(".adhoc-sign.XXXXXX"));
	int fd;

	if (!temp) {
		fprintf(stderr, "%s: %s: out of memory\n", progname, path);
		return -1;
	}
	memcpy(temp, path, path_length);
	memcpy(temp + path_length, ".adhoc-sign.XXXXXX", sizeof(".adhoc-sign.XXXXXX"));

	fd = mkstemp(temp);
	if (fd < 0) {
		fprintf(stderr, "%s: cannot create a file next to %s: %s\n", progname, path, strerror(errno));
		free(temp);
		return -1;
	}

	size_t done = 0;
	while (done < buffer->length) {
		ssize_t n = write(fd, buffer->data + done, buffer->length - done);
		if (n < 0 && errno == EINTR) {
			continue;
		}
		if (n <= 0) {
			fprintf(stderr, "%s: cannot write %s: %s\n", progname, temp, strerror(errno));
			goto fail;
		}
		done += (size_t)n;
	}

	if (fchmod(fd, st->st_mode & 07777) != 0 || close(fd) != 0) {
		fd = -1;
		fprintf(stderr, "%s: cannot finish %s: %s\n", progname, temp, strerror(errno));
		goto fail;
	}
	fd = -1;

	if (rename(temp, path) != 0) {
		fprintf(stderr, "%s: cannot replace %s: %s\n", progname, path, strerror(errno));
		goto fail;
	}

	free(temp);
	return 0;

fail:
	if (fd >= 0) {
		close(fd);
	}
	unlink(temp);
	free(temp);
	return -1;
}

static int sign_file(const char* path, const char* identifier) {
	struct buffer in = { 0 }, out = { 0 };
	struct stat st;
	int result = -1;

	if (read_file(path, &in, &st) != 0) {
		return -1;
	}

	if (!identifier || !*identifier) {
		const char* slash = strrchr(path, '/');
		identifier = slash ? slash + 1 : path;
	}

	switch (classify(in.data, in.length)) {
		case KIND_OTHER:
			result = 0;
			goto done;
		case KIND_THIN:
			result = sign_image(path, in.data, in.length, identifier, &out);
			break;
		case KIND_FAT:
			result = sign_fat(path, in.data, in.length, identifier, &out);
			break;
	}

	if (result == 0) {
		result = write_file(path, &out, &st);
	} else if (result > 0) {
		result = 0;
	}

done:
	free(in.data);
	free(out.data);
	return result;
}

int main(int argc, char** argv) {
	const char* identifier = NULL;
	int first = 1;
	int status = 0;

	for (; first < argc; ++first) {
		if (strncmp(argv[first], "--identifier=", 13) == 0) {
			identifier = argv[first] + 13;
		} else if (strcmp(argv[first], "--") == 0) {
			++first;
			break;
		} else if (argv[first][0] == '-') {
			fprintf(stderr, "usage: %s [--identifier=<id>] <file>...\n", progname);
			return 2;
		} else {
			break;
		}
	}

	if (first >= argc) {
		fprintf(stderr, "usage: %s [--identifier=<id>] <file>...\n", progname);
		return 2;
	}

	for (int i = first; i < argc; ++i) {
		if (sign_file(argv[i], identifier) != 0) {
			status = 1;
		}
	}

	return status;
}
