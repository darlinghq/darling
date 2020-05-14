/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Arc4 random number generator for OpenBSD.
 *
 * This code is derived from section 17.1 of Applied Cryptography,
 * second edition, which describes a stream cipher allegedly
 * compatible with RSA Labs "RC4" cipher (the actual description of
 * which is a trade secret).  The same algorithm is used as a stream
 * cipher called "arcfour" in Tatu Ylonen's ssh package.
 *
 * Here the stream cipher has been modified always to include the time
 * when initializing the state.  That makes it impossible to
 * regenerate the same random sequence twice, so this can't be used
 * for encryption, but will generate good random numbers.
 *
 * RC4 is a registered trademark of RSA Laboratories.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/arc4random.c,v 1.25 2008/09/09 09:46:36 ache Exp $");

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/random.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <TargetConditionals.h>
#if !TARGET_OS_DRIVERKIT
#define OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE 1
#endif
#include <os/assumes.h>
#include <os/lock.h>

#include "string.h"
#include "libc_private.h"

#if defined(__APPLE__) && !defined(VARIANT_STATIC)
#include <corecrypto/ccrng.h>

static struct ccrng_state *rng;

static void
arc4_init(void)
{
	int err;

	if (rng != NULL) return;

	rng = ccrng(&err);
	if (rng == NULL) {
#if OS_CRASH_ENABLE_EXPERIMENTAL_LIBTRACE
		os_crash("arc4random: unable to get ccrng() handle (%d)", err);
#else
		os_crash("arc4random: unable to get ccrng() handle");
#endif
	}
}

void
arc4random_addrandom(__unused u_char *dat, __unused int datlen)
{
	/* NOP - deprecated */
}

uint32_t
arc4random(void)
{
	uint32_t rand;

	arc4random_buf(&rand, sizeof(rand));

	return rand;
}

void
arc4random_buf(void *buf, size_t buf_size)
{
	arc4_init();
	ccrng_generate(rng, buf_size, buf);
}

void
arc4random_stir(void)
{
	/* NOP */
}

uint32_t
arc4random_uniform(uint32_t upper_bound)
{
	uint64_t rand;

	arc4_init();
	ccrng_uniform(rng, upper_bound, &rand);

	return (uint32_t)rand;
}

__private_extern__ void
_arc4_fork_child(void)
{
	/* NOP */
}

#else /* __APPLE__ && !VARIANT_STATIC */

#define	RANDOMDEV	"/dev/random"

static void
_my_getentropy(uint8_t *buf, size_t size){
    int fd;
	ssize_t ret;
	if (getentropy(buf, size) == 0) {
		return;
	}

	// The above should never fail, but we'll try /dev/random anyway
	fd = open(RANDOMDEV, O_RDONLY, 0);
	if (fd == -1) {
#if !defined(VARIANT_STATIC)
		os_crash("arc4random: unable to open /dev/random");
#else
		abort();
#endif
	}
shortread:
	ret = read(fd, buf, size);
	if (ret == -1) {
#if !defined(VARIANT_STATIC)
		os_crash("arc4random: unable to read from /dev/random");
#else
		abort();
#endif
	} else if ((size_t)ret < size) {
		size -= (size_t)ret;
		buf += ret;
		goto shortread;
	}
	(void)close(fd);
}

struct arc4_stream {
	u_int8_t i;
	u_int8_t j;
	u_int8_t s[256];
};

#define KEYSIZE		128

static struct arc4_stream rs = {
	.i = 0,
	.j = 0,
	.s = {
		  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
		 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
		 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
		 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
		 64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
		 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
		 96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
		128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
		176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
		192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
		208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
		224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
		240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
	}
};
static int rs_stired;

static inline u_int8_t arc4_getbyte(void);
static void arc4_stir(void);

static struct {
	struct timeval	tv;
	pid_t		pid;
	u_int8_t	rnd[KEYSIZE];
} rdat;
static volatile int rs_data_available = 0;

static inline void
arc4_addrandom(u_char *dat, int datlen)
{
	int     n;
	u_int8_t si;

	rs.i--;
	for (n = 0; n < 256; n++) {
		rs.i = (rs.i + 1);
		si = rs.s[rs.i];
		rs.j = (rs.j + si + dat[n % datlen]);
		rs.s[rs.i] = rs.s[rs.j];
		rs.s[rs.j] = si;
	}
	rs.j = rs.i;
}

static void
arc4_fetch(void)
{
	_my_getentropy((uint8_t*)&rdat, KEYSIZE);
}

static os_unfair_lock arc4_lock = OS_UNFAIR_LOCK_INIT;
static int arc4_count;

static void
arc4_stir(void)
{
	int n;
	/*
	 * If we don't have data, we need some now before we can integrate
	 * it into the static buffers
	 */
	if (!rs_data_available)
	{
		arc4_fetch();
	}
	rs_data_available = 0;
	__sync_synchronize();

	arc4_addrandom((u_char *)&rdat, KEYSIZE);

	/*
	 * Throw away the first N bytes of output, as suggested in the
	 * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
	 * by Fluher, Mantin, and Shamir.  N=1024 is based on
	 * suggestions in the paper "(Not So) Random Shuffles of RC4"
	 * by Ilya Mironov.
	 */
	for (n = 0; n < 1024; n++)
		(void) arc4_getbyte();
	arc4_count = 1600000;
	rs_stired = 1;
}

static inline u_int8_t
arc4_getbyte(void)
{
	u_int8_t si, sj;

	rs.i = (rs.i + 1);
	si = rs.s[rs.i];
	rs.j = (rs.j + si);
	sj = rs.s[rs.j];
	rs.s[rs.i] = sj;
	rs.s[rs.j] = si;

	return (rs.s[(si + sj) & 0xff]);
}

static inline u_int32_t
arc4_getword(void)
{
	u_int32_t val;

	val = arc4_getbyte() << 24;
	val |= arc4_getbyte() << 16;
	val |= arc4_getbyte() << 8;
	val |= arc4_getbyte();

	return (val);
}

/* 7944700: force restir in child */
__private_extern__ void
_arc4_fork_child(void)
{
	arc4_lock = OS_UNFAIR_LOCK_INIT;
	rs_stired = 0;
	rs_data_available = 0;
}

static inline int
arc4_check_stir(void)
{
	if (!rs_stired || arc4_count <= 0) {
		arc4_stir();
		return 1;
	}
	return 0;
}

void
arc4random_addrandom(u_char *dat, int datlen)
{
	os_unfair_lock_lock(&arc4_lock);
	arc4_check_stir();
	arc4_addrandom(dat, datlen);
	os_unfair_lock_unlock(&arc4_lock);
}

u_int32_t
arc4random(void)
{
	u_int32_t rnd;

	os_unfair_lock_lock(&arc4_lock);

	int did_stir = arc4_check_stir();
	rnd = arc4_getword();
	arc4_count -= 4;

	os_unfair_lock_unlock(&arc4_lock);
	if (did_stir)
	{
		/* stirring used up our data pool, we need to read in new data outside of the lock */
		arc4_fetch();
		rs_data_available = 1;
		__sync_synchronize();
	}

	return (rnd);
}

void
arc4random_buf(void *_buf, size_t n)
{
	u_char *buf = (u_char *)_buf;
	int did_stir = 0;

	os_unfair_lock_lock(&arc4_lock);

	while (n--) {
		if (arc4_check_stir())
		{
			did_stir = 1;
		}
		buf[n] = arc4_getbyte();
		arc4_count--;
	}

	os_unfair_lock_unlock(&arc4_lock);
	if (did_stir)
	{
		/* stirring used up our data pool, we need to read in new data outside of the lock */
		arc4_fetch();
		rs_data_available = 1;
		__sync_synchronize();
	}
}

void
arc4random_stir(void)
{
	os_unfair_lock_lock(&arc4_lock);
	arc4_stir();
	os_unfair_lock_unlock(&arc4_lock);
}

/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by trying successive ranges of bits from the random
 * value, each large enough to hold the desired upper bound, until a range
 * holding a value less than the bound is found.
 */
uint32_t
arc4random_uniform(uint32_t upper_bound)
{
	if (upper_bound < 2)
		return 0;

	// find smallest 2**n -1 >= upper_bound
	int zeros = __builtin_clz(upper_bound);
	int bits = CHAR_BIT * sizeof(uint32_t) - zeros;
	uint32_t mask = 0xFFFFFFFFU >> zeros;

	do {
		uint32_t value = arc4random();

		// If low 2**n-1 bits satisfy the requested condition, return result
		uint32_t result = value & mask;
		if (result < upper_bound) {
			return result;
		}

		// otherwise consume remaining bits of randomness looking for a satisfactory result.
		int bits_left = zeros;
		while (bits_left >= bits) {
			value >>= bits;
			result = value & mask;
			if (result < upper_bound) {
				return result;
			}
			bits_left -= bits;
		}
	} while (1);
}

#endif /* __APPLE__ */
