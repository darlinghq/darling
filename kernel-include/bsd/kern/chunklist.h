#ifndef _CHUNKLIST_H
#define _CHUNKLIST_H

#include <libkern/crypto/sha2.h>

/*
 * Boot argument for disabling trust in rev2 development key(s)
 * Set by boot.efi
 */
#define CHUNKLIST_NO_REV2_DEV           "-chunklist-no-rev2-dev"

/*
 * Boot argument for disabling trust in rev1 chunklists
 * Set by boot.efi
 */
#define CHUNKLIST_NO_REV1               "-chunklist-no-rev1"

/*
 * Boot argument for obtaining current security epoch
 * Set by boot.efi
 */
#define CHUNKLIST_SECURITY_EPOCH        "chunklist-security-epoch"
#define CHUNKLIST_MIN_SECURITY_EPOCH    0

/*
 * Chunklist file format
 */
#define CHUNKLIST_MAGIC                 0x4C4B4E43
#define CHUNKLIST_FILE_VERSION_10       1
#define CHUNKLIST_CHUNK_METHOD_10       1
#define CHUNKLIST_SIGNATURE_METHOD_REV1 1
#define CHUNKLIST_SIGNATURE_METHOD_REV2 3
#define CHUNKLIST_REV1_SIG_LEN          256
#define CHUNKLIST_REV2_SIG_LEN          808
#define CHUNKLIST_PUBKEY_LEN            (2048/8)
#define CHUNKLIST_SIGNATURE_LEN         (2048/8)

struct efi_guid_t {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t  data4[8];
} __attribute__((packed));

// 45E7BC51-913C-42AC-96A2-10712FFBEBA7
#define CHUNKLIST_REV2_SIG_HASH_GUID \
{ \
    0x45E7BC51, 0x913C, 0x42AC, { 0x96, 0xA2, 0x10, 0x71, 0x2F, 0xFB, 0xEB, 0xA7 } \
};

// A7717414-C616-4977-9420-844712A735BF
#define EFI_CERT_TYPE_RSA2048_SHA256 \
{ \
    0xa7717414, 0xc616, 0x4977, { 0x94, 0x20, 0x84, 0x47, 0x12, 0xa7, 0x35, 0xbf } \
}

#define WIN_CERT_TYPE_EFI_GUID 0x0EF1

struct chunklist_hdr {
	uint32_t cl_magic;
	uint32_t cl_header_size;
	uint8_t  cl_file_ver;
	uint8_t  cl_chunk_method;
	uint8_t  cl_sig_method;
	uint8_t  __unused1;
	uint64_t cl_chunk_count;
	uint64_t cl_chunk_offset;
	uint64_t cl_sig_offset;
} __attribute__((packed));

struct chunklist_chunk {
	uint32_t chunk_size;
	uint8_t  chunk_sha256[SHA256_DIGEST_LENGTH];
} __attribute__((packed));

struct rev2_chunklist_certificate {
	uint32_t length;
	uint8_t  revision;
	uint8_t  security_epoch;
	uint16_t certificate_type;
	guid_t   certificate_guid;
	guid_t   hash_type_guid;
	uint8_t  rsa_public_key[CHUNKLIST_PUBKEY_LEN];
	uint8_t  rsa_signature[CHUNKLIST_SIGNATURE_LEN];
} __attribute__((packed));

struct chunklist_pubkey {
	const boolean_t is_production;
	const uint8_t key[CHUNKLIST_PUBKEY_LEN];
};

int authenticate_root_with_chunklist(const char *root_path);
int authenticate_root_version_check(void);
#endif /* _CHUNKLIST_H */
