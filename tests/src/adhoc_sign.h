// Copyright (C) 2026 Lubos Dolezel
//
// Ad-hoc code signing for guest tests that need signed programs: writes a copy
// of a thin Mach-O file with an embedded signature (SHA-256 CodeDirectory,
// 4 KiB pages, empty requirements, and optionally an entitlements blob with
// the given keys set to true), laid out as <kern/cs_blobs.h> describes. The
// copy must not be signed already. Include it in one source file; it needs
// CommonCrypto.
#ifndef _TESTS_ADHOC_SIGN_H_
#define _TESTS_ADHOC_SIGN_H_

#include <CommonCrypto/CommonDigest.h>
#include <fcntl.h>
#include <kern/cs_blobs.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/loader.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SIGN_PAGE_SHIFT 12
#define SIGN_PAGE_SIZE (1u << SIGN_PAGE_SHIFT)


struct buffer {
	uint8_t* data;
	size_t length;
};

static int append(struct buffer* b, const void* data, size_t length)
{
	uint8_t* grown = realloc(b->data, b->length + length);
	if (!grown)
		return -1;
	b->data = grown;
	if (data)
		memcpy(b->data + b->length, data, length);
	else
		memset(b->data + b->length, 0, length);
	b->length += length;
	return 0;
}

static void put32(uint8_t* p, uint32_t v)
{
	OSWriteBigInt32(p, 0, v);
}

static void put64(uint8_t* p, uint64_t v)
{
	OSWriteBigInt64(p, 0, v);
}

static size_t round_up(size_t value, size_t to)
{
	return (value + to - 1) / to * to;
}

// The entitlements blob for the given keys (all set to true), or an empty
// buffer when there are none.
static struct buffer entitlements_blob(const char* const* keys)
{
	struct buffer xml = { NULL, 0 }, blob = { NULL, 0 };
	const char* head = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n";
	const char* tail = "</dict>\n</plist>\n";

	if (!keys || !keys[0])
		return blob;

	append(&xml, head, strlen(head));
	for (size_t i = 0; keys[i]; i++) {
		char line[256];
		snprintf(line, sizeof(line), "\t<key>%s</key>\n\t<true/>\n", keys[i]);
		append(&xml, line, strlen(line));
	}
	append(&xml, tail, strlen(tail));

	uint8_t header[8];
	put32(header, CSMAGIC_EMBEDDED_ENTITLEMENTS);
	put32(header + 4, (uint32_t)(sizeof(header) + xml.length));
	append(&blob, header, sizeof(header));
	append(&blob, xml.data, xml.length);
	free(xml.data);
	return blob;
}

// Writes an ad-hoc signed copy of the thin Mach-O file `source` to `target`.
static int adhoc_sign(const char* source, const char* target, const char* identifier, int runtime, const char* const* entitlement_keys)
{
	struct buffer file = { NULL, 0 };
	int fd = open(source, O_RDONLY);
	if (fd < 0)
		return -1;
	struct stat st;
	if (fstat(fd, &st) != 0 || append(&file, NULL, (size_t)st.st_size) != 0 ||
			pread(fd, file.data, file.length, 0) != (ssize_t)file.length) {
		close(fd);
		return -1;
	}
	close(fd);

	const struct mach_header* mh = (const struct mach_header*)file.data;
	int is64 = (mh->magic == MH_MAGIC_64);
	if (!is64 && mh->magic != MH_MAGIC)
		return -1;
	size_t header_size = is64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header);

	// find __TEXT, __LINKEDIT, the first section and any old signature
	uint8_t* linkedit = NULL;
	uint64_t text_offset = 0, text_size = 0, first_section = file.length;
	uint8_t* cmd = file.data + header_size;
	for (uint32_t i = 0; i < mh->ncmds; i++) {
		const struct load_command* lc = (const struct load_command*)cmd;
		if (lc->cmd == LC_SEGMENT_64) {
			const struct segment_command_64* seg = (const void*)cmd;
			const struct section_64* sect = (const void*)(seg + 1);
			if (strcmp(seg->segname, SEG_TEXT) == 0) {
				text_offset = seg->fileoff;
				text_size = seg->filesize;
			} else if (strcmp(seg->segname, SEG_LINKEDIT) == 0) {
				linkedit = cmd;
			}
			for (uint32_t s = 0; s < seg->nsects; s++)
				if (sect[s].offset != 0 && sect[s].offset < first_section)
					first_section = sect[s].offset;
		} else if (lc->cmd == LC_SEGMENT) {
			const struct segment_command* seg = (const void*)cmd;
			const struct section* sect = (const void*)(seg + 1);
			if (strcmp(seg->segname, SEG_TEXT) == 0) {
				text_offset = seg->fileoff;
				text_size = seg->filesize;
			} else if (strcmp(seg->segname, SEG_LINKEDIT) == 0) {
				linkedit = cmd;
			}
			for (uint32_t s = 0; s < seg->nsects; s++)
				if (sect[s].offset != 0 && sect[s].offset < first_section)
					first_section = sect[s].offset;
		} else if (lc->cmd == LC_CODE_SIGNATURE) {
			// re-signing is not needed for this test
			return -1;
		}
		cmd += lc->cmdsize;
	}
	if (!linkedit || header_size + mh->sizeofcmds + sizeof(struct linkedit_data_command) > first_section)
		return -1;

	// the signature goes at the end of the file, 16-byte aligned
	uint32_t code_limit = (uint32_t)round_up(file.length, 16);
	append(&file, NULL, code_limit - file.length);

	// blobs
	struct buffer requirements = { NULL, 0 };
	uint8_t req[12];
	put32(req, CSMAGIC_REQUIREMENTS);
	put32(req + 4, sizeof(req));
	put32(req + 8, 0);
	append(&requirements, req, sizeof(req));
	struct buffer entitlements = entitlements_blob(entitlement_keys);

	uint32_t special_slots = entitlements.length ? CSSLOT_ENTITLEMENTS : CSSLOT_REQUIREMENTS;
	uint32_t code_slots = (code_limit + SIGN_PAGE_SIZE - 1) / SIGN_PAGE_SIZE;
	size_t cd_header = offsetof(CS_CodeDirectory, end_withPreEncryptOffset);
	size_t ident_offset = cd_header;
	size_t hash_offset = round_up(ident_offset + strlen(identifier) + 1, 4) + special_slots * CS_SHA256_LEN;
	size_t cd_length = hash_offset + code_slots * CS_SHA256_LEN;

	uint32_t blob_count = entitlements.length ? 3 : 2;
	size_t super_header = sizeof(CS_SuperBlob) + blob_count * sizeof(CS_BlobIndex);
	size_t signature_length = super_header + cd_length + requirements.length + entitlements.length;
	uint32_t data_size = (uint32_t)round_up(signature_length, 16);

	// load command and __LINKEDIT first: the code pages include the header
	struct mach_header* wmh = (struct mach_header*)file.data;
	struct linkedit_data_command* sig = (struct linkedit_data_command*)(file.data + header_size + wmh->sizeofcmds);
	sig->cmd = LC_CODE_SIGNATURE;
	sig->cmdsize = sizeof(*sig);
	sig->dataoff = code_limit;
	sig->datasize = data_size;
	wmh->ncmds++;
	wmh->sizeofcmds += sizeof(*sig);

	cmd = file.data + header_size;
	for (uint32_t i = 0; i < wmh->ncmds; i++) {
		struct load_command* lc = (struct load_command*)cmd;
		if (lc->cmd == LC_SEGMENT_64 && strcmp(((struct segment_command_64*)cmd)->segname, SEG_LINKEDIT) == 0) {
			struct segment_command_64* seg = (void*)cmd;
			seg->filesize = code_limit + data_size - seg->fileoff;
			if (seg->vmsize < round_up(seg->filesize, SIGN_PAGE_SIZE))
				seg->vmsize = round_up(seg->filesize, SIGN_PAGE_SIZE);
		} else if (lc->cmd == LC_SEGMENT && strcmp(((struct segment_command*)cmd)->segname, SEG_LINKEDIT) == 0) {
			struct segment_command* seg = (void*)cmd;
			seg->filesize = code_limit + data_size - seg->fileoff;
			if (seg->vmsize < round_up(seg->filesize, SIGN_PAGE_SIZE))
				seg->vmsize = (uint32_t)round_up(seg->filesize, SIGN_PAGE_SIZE);
		}
		cmd += lc->cmdsize;
	}

	// CodeDirectory
	uint8_t* cd = calloc(1, cd_length);
	if (!cd)
		return -1;
	put32(cd + offsetof(CS_CodeDirectory, magic), CSMAGIC_CODEDIRECTORY);
	put32(cd + offsetof(CS_CodeDirectory, length), (uint32_t)cd_length);
	put32(cd + offsetof(CS_CodeDirectory, version), CS_SUPPORTSRUNTIME);
	put32(cd + offsetof(CS_CodeDirectory, flags), CS_ADHOC | (runtime ? CS_RUNTIME : 0));
	put32(cd + offsetof(CS_CodeDirectory, hashOffset), (uint32_t)hash_offset);
	put32(cd + offsetof(CS_CodeDirectory, identOffset), (uint32_t)ident_offset);
	put32(cd + offsetof(CS_CodeDirectory, nSpecialSlots), special_slots);
	put32(cd + offsetof(CS_CodeDirectory, nCodeSlots), code_slots);
	put32(cd + offsetof(CS_CodeDirectory, codeLimit), code_limit);
	cd[offsetof(CS_CodeDirectory, hashSize)] = CS_SHA256_LEN;
	cd[offsetof(CS_CodeDirectory, hashType)] = CS_HASHTYPE_SHA256;
	cd[offsetof(CS_CodeDirectory, pageSize)] = SIGN_PAGE_SHIFT;
	put64(cd + offsetof(CS_CodeDirectory, execSegBase), text_offset);
	put64(cd + offsetof(CS_CodeDirectory, execSegLimit), text_size);
	put64(cd + offsetof(CS_CodeDirectory, execSegFlags), CS_EXECSEG_MAIN_BINARY);
	if (runtime)
		put32(cd + offsetof(CS_CodeDirectory, runtime), 0x000b0000); // hardened runtime version: macOS 11.0
	memcpy(cd + ident_offset, identifier, strlen(identifier) + 1);
	CC_SHA256(requirements.data, (CC_LONG)requirements.length, cd + hash_offset - CSSLOT_REQUIREMENTS * CS_SHA256_LEN);
	if (entitlements.length)
		CC_SHA256(entitlements.data, (CC_LONG)entitlements.length, cd + hash_offset - CSSLOT_ENTITLEMENTS * CS_SHA256_LEN);
	for (uint32_t i = 0; i < code_slots; i++) {
		size_t start = (size_t)i * SIGN_PAGE_SIZE;
		size_t length = (start + SIGN_PAGE_SIZE <= code_limit) ? SIGN_PAGE_SIZE : code_limit - start;
		CC_SHA256(file.data + start, (CC_LONG)length, cd + hash_offset + i * CS_SHA256_LEN);
	}

	// SuperBlob
	struct buffer signature = { NULL, 0 };
	uint8_t head[sizeof(CS_SuperBlob) + 3 * sizeof(CS_BlobIndex)];
	size_t offset = super_header;
	put32(head, CSMAGIC_EMBEDDED_SIGNATURE);
	put32(head + 4, (uint32_t)signature_length);
	put32(head + 8, blob_count);
	put32(head + 12, CSSLOT_CODEDIRECTORY);
	put32(head + 16, (uint32_t)offset);
	offset += cd_length;
	put32(head + 20, CSSLOT_REQUIREMENTS);
	put32(head + 24, (uint32_t)offset);
	offset += requirements.length;
	if (entitlements.length) {
		put32(head + 28, CSSLOT_ENTITLEMENTS);
		put32(head + 32, (uint32_t)offset);
	}
	append(&signature, head, super_header);
	append(&signature, cd, cd_length);
	append(&signature, requirements.data, requirements.length);
	append(&signature, entitlements.data, entitlements.length);
	append(&signature, NULL, data_size - signature.length);
	free(cd);
	free(requirements.data);
	free(entitlements.data);

	fd = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	int ok = fd >= 0 && write(fd, file.data, file.length) == (ssize_t)file.length &&
		write(fd, signature.data, signature.length) == (ssize_t)signature.length;
	if (fd >= 0)
		close(fd);
	free(file.data);
	free(signature.data);
	return ok ? 0 : -1;
}

#endif // _TESTS_ADHOC_SIGN_H_
