// CommonCrypto Diffie-Hellman (CCDH*), CCBigNumModExp and PKCS #7 padding
// (kCCOptionPKCS7Padding) against values computed outside Darling: see
// tests/commoncrypto-dh-modexp-pkcs7-vectors.py, which generates the vectors header.
// Builds unchanged for macOS; the CCDH and CCBigNum calls are declared here
// because their headers are not in the public SDK.
//
// Prints FAIL lines and exits with the number of failed checks. It also prints
// one "DH ..." line per group: the guest's public key and the secret it computed
// with a peer key 2^b mod p; tests/commoncrypto-dh-check.py recomputes y^b mod p
// with Python's pow() and compares.
//
// Run it for x86_64 and i386 and keep the output, then check the DH lines:
//   tests/commoncrypto-dh-check.py out-x86_64.txt out-i386.txt
// On macOS 10.14.4 the test passes except the two modexp edge cases noted at
// test_modexp; macOS 26.7.3 no longer exports the DH group constants (the DH
// part is skipped) and refuses even moduli in CCBigNumModExp.
#include <CommonCrypto/CommonCryptor.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "commoncrypto_dh_modexp_pkcs7_vectors.h"

// CommonDH.h
typedef struct CCDHRef_s *CCDHRef;
typedef struct CCDHParameters_s *CCDHParameters;
// weak: newer macOS releases no longer export the group constants
extern const CCDHParameters kCCDHRFC2409Group2 __attribute__((weak_import));
extern const CCDHParameters kCCDHRFC3526Group5 __attribute__((weak_import));
CCDHRef CCDHCreate(CCDHParameters dhParameter);
void CCDHRelease(CCDHRef ref);
int CCDHGenerateKey(CCDHRef ref, void *output, size_t *outputLength);
int CCDHComputeKey(unsigned char *sharedKey, size_t *sharedKeyLen, const void *peerPubKey, size_t peerPubKeyLen, CCDHRef ref);

// CommonBigNum.h
typedef int32_t CCStatus;
typedef struct _CCBigNumRef *CCBigNumRef;
CCBigNumRef CCCreateBigNum(CCStatus *status);
void CCBigNumFree(CCBigNumRef bn);
CCBigNumRef CCBigNumFromData(CCStatus *status, const void *s, size_t len);
size_t CCBigNumToData(CCStatus *status, const CCBigNumRef bn, void *to);
uint32_t CCBigNumByteCount(const CCBigNumRef bn);
CCStatus CCBigNumModExp(CCBigNumRef result, const CCBigNumRef a, const CCBigNumRef power, const CCBigNumRef modulus);

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL line %d: %s: ", __LINE__, #cond); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static size_t from_hex(const char *hex, uint8_t *out)
{
	size_t n = strlen(hex) / 2;
	for (size_t i = 0; i < n; i++) {
		unsigned v;
		sscanf(hex + 2 * i, "%2x", &v);
		out[i] = (uint8_t)v;
	}
	return n;
}

static void print_hex(const uint8_t *data, size_t n)
{
	for (size_t i = 0; i < n; i++)
		printf("%02x", data[i]);
}

// big-endian byte strings, compared as integers (leading zeros ignored)
static int same_integer(const uint8_t *a, size_t an, const uint8_t *b, size_t bn)
{
	while (an && !*a) { a++; an--; }
	while (bn && !*b) { b++; bn--; }
	return an == bn && memcmp(a, b, an) == 0;
}

static CCBigNumRef bignum_from_hex(const char *hex)
{
	uint8_t buf[1024];
	size_t n = from_hex(hex, buf);
	CCStatus status = -1;
	CCBigNumRef bn = CCBigNumFromData(&status, buf, n);
	CHECK(bn && status == 0, "CCBigNumFromData(%s...) status %d", n ? "" : "empty", (int)status);
	return bn;
}

// The expected values are the mathematical ones (Python's pow). macOS differs on a
// few edge cases, and Darling keeps the mathematical result there: macOS 26.7.3
// refuses even moduli and modulus 1 with kCCParamError (macOS 10.14.4 computes
// them), and macOS 10.14.4 returns 1 for x^0 mod 1 and an unreduced value
// (above the modulus) for the modulus 2^64 case.
static void test_modexp(void)
{
	for (size_t i = 0; i < sizeof(modexp_vectors) / sizeof(modexp_vectors[0]); i++) {
		const struct modexp_vector *v = &modexp_vectors[i];
		CCBigNumRef b = bignum_from_hex(v->base), e = bignum_from_hex(v->exponent), m = bignum_from_hex(v->modulus);
		CCStatus status = -1;
		CCBigNumRef r = CCCreateBigNum(&status);
		if (!b || !e || !m || !r) {
			CHECK(0, "%s: allocation", v->name);
			continue;
		}
		status = CCBigNumModExp(r, b, e, m);
		CHECK(status == 0, "%s: CCBigNumModExp status %d", v->name, (int)status);

		uint8_t expected[1024], actual[1024];
		size_t en = from_hex(v->result, expected);
		size_t count = CCBigNumByteCount(r);
		size_t an = CCBigNumToData(&status, r, actual);
		CHECK(an == count, "%s: CCBigNumToData %zu bytes, CCBigNumByteCount %zu", v->name, an, count);
		if (!same_integer(expected, en, actual, an)) {
			CHECK(0, "%s: result differs", v->name);
			printf("  expected %s\n  actual   ", v->result);
			print_hex(actual, an);
			printf("\n");
		}

		// the result may also be one of the operands
		CCBigNumModExp(b, b, e, m);
		an = CCBigNumToData(&status, b, actual);
		CHECK(same_integer(expected, en, actual, an), "%s: result into the base differs", v->name);

		CCBigNumFree(b); CCBigNumFree(e); CCBigNumFree(m); CCBigNumFree(r);
	}
}

static void test_dh_group(const char *name, CCDHParameters params, const char *prime_hex,
                          const char *peer_private_hex, const char *peer_public_hex)
{
	uint8_t p[1024], peer[1024];
	const size_t plen = from_hex(prime_hex, p);
	const size_t peer_len = from_hex(peer_public_hex, peer);

	CCDHRef a = CCDHCreate(params), b = CCDHCreate(params);
	CHECK(a && b, "%s: CCDHCreate", name);
	if (!a || !b)
		return;

	// public keys are the size of p
	uint8_t pub_a[1024], pub_b[1024];
	size_t len_a = 1, len_b = sizeof(pub_b);
	CHECK(CCDHGenerateKey(a, pub_a, &len_a) == -1 && len_a == plen, "%s: short buffer gives -1 and the size (got %zu)", name, len_a);
	len_a = sizeof(pub_a);
	CHECK(CCDHGenerateKey(a, pub_a, &len_a) == 0 && len_a == plen, "%s: public key length %zu", name, len_a);
	CHECK(CCDHGenerateKey(b, pub_b, &len_b) == 0 && len_b == plen, "%s: public key length %zu", name, len_b);
	CHECK(memcmp(pub_a, pub_b, plen) != 0, "%s: two keys are equal", name);
	// 2 <= y <= p - 2
	CHECK(memcmp(pub_a, p, plen) < 0, "%s: public key not below p", name);

	// both sides agree
	uint8_t z_ab[1024], z_ba[1024];
	size_t zlen_ab = sizeof(z_ab), zlen_ba = sizeof(z_ba);
	CHECK(CCDHComputeKey(z_ab, &zlen_ab, pub_b, len_b, a) == 0, "%s: compute a", name);
	CHECK(CCDHComputeKey(z_ba, &zlen_ba, pub_a, len_a, b) == 0, "%s: compute b", name);
	CHECK(zlen_ab == zlen_ba && memcmp(z_ab, z_ba, zlen_ab) == 0, "%s: shared secrets differ", name);
	CHECK(zlen_ab <= plen && zlen_ab > plen - 8 && z_ab[0] != 0, "%s: secret length %zu, first byte %02x", name, zlen_ab, z_ab[0]);

	// a buffer shorter than p is refused
	size_t small = plen - 1;
	CHECK(CCDHComputeKey(z_ab, &small, pub_b, len_b, a) != 0 && small == plen, "%s: short secret buffer (length %zu)", name, small);

	// secret with a known peer key: checked on the host (tests/commoncrypto-dh-check.py)
	uint8_t z[1024];
	size_t zlen = sizeof(z);
	CHECK(CCDHComputeKey(z, &zlen, peer, peer_len, a) == 0, "%s: compute with the known peer key", name);
	printf("DH %s p=%s b=%s y=", name, prime_hex, peer_private_hex);
	print_hex(pub_a, len_a);
	printf(" z=");
	print_hex(z, zlen);
	printf("\n");

	// invalid peer keys: 0, 1, p - 1, p, p + 1 and an empty key (SP 800-56A 5.6.2.3)
	uint8_t bad[1024];
	const char *bad_names[] = { "0", "1", "p - 1", "p", "p + 1", "2^(8 len) - 1", "empty" };
	for (int k = 0; k < 7; k++) {
		size_t bad_len = plen;
		memset(bad, 0, plen);
		switch (k) {
		case 0: break;
		case 1: bad[plen - 1] = 1; break;
		case 2: memcpy(bad, p, plen); bad[plen - 1] -= 1; break;
		case 3: memcpy(bad, p, plen); break;
		case 4: memcpy(bad, p, plen); bad[plen - 1] += 1; break; // p ends in ff..ff: + 1 carries
		case 5: memset(bad, 0xff, plen); break;
		case 6: bad_len = 0; break;
		}
		if (k == 4) {
			// p ends in 0xff, so p + 1 ends in 0x00 with a carry
			memcpy(bad, p, plen);
			for (size_t i = plen; i-- > 0;) { if (++bad[i]) break; }
		}
		size_t out_len = sizeof(z);
		int rc = CCDHComputeKey(z, &out_len, bad, bad_len, a);
		CHECK(rc != 0, "%s: peer key %s accepted", name, bad_names[k]);
	}
	// the smallest and largest valid peer keys, 2 and p - 2, are accepted
	memset(bad, 0, plen);
	bad[plen - 1] = 2;
	size_t out_len = sizeof(z);
	CHECK(CCDHComputeKey(z, &out_len, bad, plen, a) == 0, "%s: peer key 2 refused", name);
	memcpy(bad, p, plen);
	bad[plen - 1] -= 2;
	out_len = sizeof(z);
	CHECK(CCDHComputeKey(z, &out_len, bad, plen, a) == 0, "%s: peer key p - 2 refused", name);

	CCDHRelease(a);
	CCDHRelease(b);
}

static void test_pkcs7_vectors(const char *name, CCAlgorithm alg, CCOptions mode, const uint8_t *key, size_t key_len,
                               const uint8_t *iv, size_t block, const struct pkcs7_vector *vectors, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		const struct pkcs7_vector *v = &vectors[i];
		uint8_t out[128], back[128];
		size_t moved = 0;

		// RFC 5652 6.3: always 1 to block bytes of padding
		CHECK(v->cipher_length == (v->length / block + 1) * block, "%s %zu: vector length", name, v->length);

		CCCryptorStatus st = CCCrypt(kCCEncrypt, alg, kCCOptionPKCS7Padding | mode, key, key_len, iv,
		                             pkcs7_plain, v->length, out, sizeof(out), &moved);
		CHECK(st == kCCSuccess && moved == v->cipher_length && memcmp(out, v->cipher, moved) == 0,
		      "%s %zu: CCCrypt encrypt status %d, %zu bytes", name, v->length, (int)st, moved);

		st = CCCrypt(kCCDecrypt, alg, kCCOptionPKCS7Padding | mode, key, key_len, iv,
		             v->cipher, v->cipher_length, back, sizeof(back), &moved);
		CHECK(st == kCCSuccess && moved == v->length && memcmp(back, pkcs7_plain, moved) == 0,
		      "%s %zu: CCCrypt decrypt status %d, %zu bytes", name, v->length, (int)st, moved);

		// streaming, one byte per update
		CCCryptorRef ref = NULL;
		st = CCCryptorCreate(kCCEncrypt, alg, kCCOptionPKCS7Padding | mode, key, key_len, iv, &ref);
		CHECK(st == kCCSuccess, "%s %zu: CCCryptorCreate", name, v->length);
		if (st != kCCSuccess)
			continue;
		size_t total = 0;
		for (size_t j = 0; j < v->length; j++) {
			st = CCCryptorUpdate(ref, pkcs7_plain + j, 1, out + total, sizeof(out) - total, &moved);
			CHECK(st == kCCSuccess, "%s %zu: update", name, v->length);
			total += moved;
		}
		CHECK(CCCryptorGetOutputLength(ref, 0, true) == v->cipher_length - total, "%s %zu: final output length %zu", name, v->length, CCCryptorGetOutputLength(ref, 0, true));
		st = CCCryptorFinal(ref, out + total, sizeof(out) - total, &moved);
		total += moved;
		CHECK(st == kCCSuccess && total == v->cipher_length && memcmp(out, v->cipher, total) == 0,
		      "%s %zu: streaming encrypt status %d, %zu bytes", name, v->length, (int)st, total);
		CCCryptorRelease(ref);

		st = CCCryptorCreate(kCCDecrypt, alg, kCCOptionPKCS7Padding | mode, key, key_len, iv, &ref);
		total = 0;
		for (size_t j = 0; j < v->cipher_length; j++) {
			st = CCCryptorUpdate(ref, v->cipher + j, 1, back + total, sizeof(back) - total, &moved);
			total += moved;
		}
		st = CCCryptorFinal(ref, back + total, sizeof(back) - total, &moved);
		total += moved;
		CHECK(st == kCCSuccess && total == v->length && memcmp(back, pkcs7_plain, total) == 0,
		      "%s %zu: streaming decrypt status %d, %zu bytes", name, v->length, (int)st, total);
		CCCryptorRelease(ref);
	}
}

static void test_pkcs7(void)
{
	// SP 800-38A F.2.1: the 64-byte case starts with the published CBC ciphertext
	uint8_t out[128];
	size_t moved = 0;
	CCCryptorStatus st = CCCrypt(kCCEncrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, aes_key, 16, aes_iv,
	                             pkcs7_plain, 64, out, sizeof(out), &moved);
	CHECK(st == kCCSuccess && moved == 80 && memcmp(out, sp800_38a_cbc, 64) == 0, "SP 800-38A F.2.1 CBC-AES128 with padding");

	test_pkcs7_vectors("AES-128-CBC", kCCAlgorithmAES, 0, aes_key, 16, aes_iv, 16,
	                   aes_cbc_vectors, sizeof(aes_cbc_vectors) / sizeof(aes_cbc_vectors[0]));
	test_pkcs7_vectors("AES-128-ECB", kCCAlgorithmAES, kCCOptionECBMode, aes_key, 16, NULL, 16,
	                   aes_ecb_vectors, sizeof(aes_ecb_vectors) / sizeof(aes_ecb_vectors[0]));
	test_pkcs7_vectors("3DES-CBC", kCCAlgorithm3DES, 0, des3_key, 24, des3_iv, 8,
	                   des3_cbc_vectors, sizeof(des3_cbc_vectors) / sizeof(des3_cbc_vectors[0]));

	// a too small output buffer
	st = CCCrypt(kCCEncrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, aes_key, 16, aes_iv,
	             pkcs7_plain, 16, out, 31, &moved);
	CHECK(st == kCCBufferTooSmall && moved == 32, "16 bytes into 31: status %d, needed %zu", (int)st, moved);

}

// QUIRK: decryption looks only at the last byte k of the last block: 1 <= k <= block
// size drops k bytes whatever the other padding bytes hold, any other k drops nothing,
// and CCCrypt still returns kCCSuccess. Observed on macOS 10.14.4 and 26.7.3 (x86_64),
// both through CCCrypt and through corecrypto's ccpad_pkcs7_decode; see the QUIRK
// comment in darling-corecrypto src/ccpad.c. RFC-valid padding is removed as usual.
static void test_quirk_pkcs7_padding_bytes_unchecked(void)
{
	for (size_t i = 0; i < sizeof(raw_padding_vectors) / sizeof(raw_padding_vectors[0]); i++) {
		const struct raw_padding_vector *v = &raw_padding_vectors[i];
		uint8_t out[64];
		size_t moved = 12345;
		CCCryptorStatus st = CCCrypt(kCCDecrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding, aes_key, 16, aes_iv,
		                             v->cipher, 32, out, sizeof(out), &moved);
		CHECK(st == kCCSuccess && moved == 32 - v->dropped && memcmp(out, pkcs7_plain, 16) == 0,
		      "padding %s (%s): status %d, %zu bytes, expected %zu", v->name, v->valid ? "valid" : "invalid",
		      (int)st, moved, 32 - v->dropped);
	}
}

int main(void)
{
	test_modexp();
	if (&kCCDHRFC2409Group2 && &kCCDHRFC3526Group5) {
		test_dh_group("group2", kCCDHRFC2409Group2, dh_group2_prime, dh_group2_peer_private, dh_group2_peer_public);
		test_dh_group("group5", kCCDHRFC3526Group5, dh_group5_prime, dh_group5_peer_private, dh_group5_peer_public);
	} else {
		printf("SKIP DH: this system does not export kCCDHRFC2409Group2 and kCCDHRFC3526Group5\n");
	}
	test_pkcs7();
	test_quirk_pkcs7_padding_bytes_unchecked();
	printf("%s (%d failures)\n", failures ? "FAILED" : "ALL PASSED", failures);
	return failures;
}
