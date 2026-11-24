// Copyright (C) 2026 Lubos Dolezel
//
// Test-only stand-in for the three CommonCrypto MD5 entry points that the ICC layer uses, so that tests/ICCProfileTest.c
// can be built natively on a host that has no CommonCrypto. It is only found when the test is compiled with
// "-I tests/host"; the framework build uses the real CommonCrypto of libSystem. It wraps the host libcrypto (OpenSSL).
#ifndef _COLORSYNC_TEST_COMMONDIGEST_H_
#define _COLORSYNC_TEST_COMMONDIGEST_H_

#include <openssl/md5.h>
#include <stdint.h>

typedef uint32_t CC_LONG;
#define CC_MD5_DIGEST_LENGTH 16
typedef MD5_CTX CC_MD5_CTX;

static inline int CC_MD5_Init(CC_MD5_CTX* c) { return MD5_Init(c); }
static inline int CC_MD5_Update(CC_MD5_CTX* c, const void* data, CC_LONG len) { return MD5_Update(c, data, len); }
static inline int CC_MD5_Final(unsigned char* md, CC_MD5_CTX* c) { return MD5_Final(md, c); }

#endif
