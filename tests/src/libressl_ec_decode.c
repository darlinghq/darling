/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// A valid NIST P-256/P-384/P-521 SubjectPublicKeyInfo must decode with
// d2i_PUBKEY() and pass EC_KEY_check_key() in every libcrypto the system
// provides, and a point that is not on the curve must be rejected.  The
// vectors are public keys generated with a different EC implementation (no
// network, no certificate chain).  The library is opened with dlopen() so one
// binary can be run against libcrypto.35/.41/.42/.44/.46:
//
//   libressl_ec_decode [/usr/lib/libcrypto.44.dylib]

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* P-256: 24 generated SubjectPublicKeyInfo encodings */
static const char *const spki_P256[] = {
	"3059301306072a8648ce3d020106082a8648ce3d030107034200046e1d6d351beca12d26b50073ad09dba35fae0a9dea4cbf4fc566cc26638f27590ddfa82db122c6d74bd80139fcf24451d0b680f37ab9b43f58e0525c79737fbf",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004ee32dfd1f3f8309f46636528e61065006692042c7b269e0406441bb6e628f7095f83a03dcb9bf0829d0425bf00c21b92f37e69f4ec7607807876047babcac33e",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004352a4e4c6fe106a57af9e760f77ad2eed73cd849ac7eb5333c5cc77a290f14081b7be1364d710876312f59194f75f0f232c66281afadb0e097621b5bbd79c4c7",
	"3059301306072a8648ce3d020106082a8648ce3d0301070342000434b2ee48bb98d2668e644cdef0dd7467564a67eaddf1a055f6378d56055903f12cca730ae609ed723252ff1ec5a681949aab6a9d422fc714f32231aee734454c",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004fffffe7f5a7d38cbf0258f828c08f9e35a252d6e90af02b57480b722c60da20027207360ddd227d96495ab1a7cef43b541439561b5904fad5b14ada3462ebe5c",
	"3059301306072a8648ce3d020106082a8648ce3d0301070342000480d4b1e289385447e7089874b03c4bee235a01af79a87e6ad56aa18d19c93f8d2bbdbfc426be500e5465c2f7965489455c8fb3ffc789d01de69837e3a5d7c4e7",
	"3059301306072a8648ce3d020106082a8648ce3d0301070342000406490dd90fc109abae50a3409a8122f6e3e198b7841ee15de1367008664fd9eb36c14cdeefb66d8feffd63b08486e8ddfb49e4b7559e132f80d25d0e383b9a04",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004684ed67b51836f1474f92701a5efbe9fe96be7df510376c7473ba16697b41918d37e9c3d4f04aba9cf891e4a2dedf698c0a4a42d9253462f68d33b59d7e6f035",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004ef8923c3fd553246c7e28392a6890b620065a8d016171f54298a602890368822ffc4d2b0dcb3bdeb2d92cb717ae98d9715fd468cdb1c11e0253c63776823d156",
	"3059301306072a8648ce3d020106082a8648ce3d0301070342000421c39c60f83bcb8b472102e12caacc95722ae9d49bbdb025d958f5a200ce211d792c0160ef592144732140826935e8311e88732ea9e29703d7c35c5e48d64c56",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004e2529340acec02489f81f33f9a8b3f5aac46b80dfa7d5bf4be1a580934d6644f9bfae14b5d6702c496598f993cf99ed40b32ea221e95c120b4c84d95c5b22d08",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200041a31d7ce86a9f09814961f01b0f8e8a3af1b4c871636d9409c31d005bfce28783213ba1310d32a0a4aa94015e10767f10f3bf2cb18cd93f8bd3f7229ab5e238a",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004a5a086652286c4266d5c5595996635e7cd907d21d9e1b0e117d4a81212ddaa4b73e486240e2cacf20778342127b2ad5ad8d2c33baa96ea6dec0c1e40eb77425f",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004a352480bbbb414a1dddcb93ef6740c533f84fb2e5fae41d14f9f828b7dc07b807d5b93d5217c205dc7d943ab1dc4829a55e132490ccfe2bb24f1c2d677faf389",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200045853eed24a512931fc874d87056db3766689e08e49bc11f8e61775cc14cb940a306fb7dd2921a6d1dbdbe212d5296a6506cf030ea5afaa7268ae9632dfa606db",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004c298d32a1f2dc3d258e7ad932dd2021657bf9db8abba4cb81ec8196fde8a02239502c3cc821a1db85db99be6ddbc1b320d85e116e4618b59fda368f35411ce48",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200049e0d200f4b8f38a04d102e1da1bfa759a82eaed3de0d6b870dcca60dad592c1061cd0d57132d5666e4242839cfd7ba6e8739b35a4601a1e26d4e10398a190ecb",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200048cd292b1be86e3f4e6f0d27da87784fe1cf2b510df03aae5cfd5f7531d8c993d14def101eab4a4b457bb684cac28ecb09142c0ef18e88da792b967a00307aaa3",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004378f02b9d8d2f9d431ad883b97dab0c093666fe0a8f040c3711a600ee9fb514703e066eb5914e0fb15aae43c346df1eba7e376af9e50f4bf36b94a5e6514d0d6",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200044c9ffe22495dec2af1a1d22e7aa31d3361351313caea1b436b32724e06f1479d06c6f45a6ce76665cdf9975c46500852e432bf7176986ca87c24b7364216d059",
	"3059301306072a8648ce3d020106082a8648ce3d030107034200040b609fd60f9ad382c52801075da126a80f5e1597fdddc2c448487f603c1796269e7eb32869e3a2ab954eeb24b09c35b0233978c307e7ffe67b709118e95bccd5",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004cb750cc9e43b350beaf193e43cf8028ffa07103ef5176ecc751bc2602636600a351774859d8a3f59ebde9b22455787e4bfbc8413e1b06efcddff7d7797978ed1",
	"3059301306072a8648ce3d020106082a8648ce3d03010703420004fb5d02619022e26c9e8d4812e9d674c4a0426756133772886927dc95deb4ba306ccdbae1778d26eb782c1c5769a43edea39cba54a678ec4edea1238e0388ca89",
	"3059301306072a8648ce3d020106082a8648ce3d0301070342000430f3e4408ef86409a5ff324ce0c5e713545a74783c58845f47fe1e94fd978c3621995e6cc46d4c2314b5aaf65e800e275c1b38a046e1f2c5a00ad6259893d6ea",
};

/* P-384: 8 generated SubjectPublicKeyInfo encodings */
static const char *const spki_P384[] = {
	"3076301006072a8648ce3d020106052b81040022036200047320dd62cf020d14a7f45842909ef64fc5f99e4a2167e9790ba73de0a99479629844d170f13366b9f7570121547a72fb7c35227d842d3b5cf71277e014c5f700f56bef4094fab8019f3e965b6a5dc669d31b19cd3bfe17c3ed02a0b0985710cc",
	"3076301006072a8648ce3d020106052b81040022036200042902aef5d48d969632caebd050683634d0f96d37890bb03166a88e83d27d3558ec0c3fea21e34edbb088525ca5fabbdae4551030379d6e511e6afde71aa82980da3bebeb8b0d928abef8823fb60a47f486e7155d86e839c557e59c09fc313258",
	"3076301006072a8648ce3d020106052b8104002203620004e19d48432fa76b20a2ea47d67cb64be6e7734c72bde05a422f69ecb15995961941d014119dd5a383f15cb308b8ca480322c99e26cf1b32166c1c55bf81d95768fdc42e5f728c4ac5f618eaf692286c414557bdf1a433b4fba732bc67d44c6e9e",
	"3076301006072a8648ce3d020106052b81040022036200044c8948f113746d43c4f2133b783c68bbe1575301c5784f8f8b82604751fbc8dcf59332c2a5b4bb23b4ae1940fb671159deef9460c1f260c122327646de74b84ef124b4336fef0cdaf64506901bf167c81497e0a97b709b00962ddc33138ba745",
	"3076301006072a8648ce3d020106052b8104002203620004c4f4e9faf7a7ca519f09a926db4a1915dddf614c8b97476441a6e0ca200703e2332bc852dca6909ae0c8ab64df5a25a37715a9e821673eba06f1bedf1e8704bb7c2c44d349b27c99a4b71c077fce90ab5a681952bb6c615f01dda224d3e5e72d",
	"3076301006072a8648ce3d020106052b8104002203620004ec34ed4f2e6a9c76eb71222fd2a771bde365591015a673d3307d943eb571b526c272a3bde529960ef91f76aaa680d56cf0149bc73e233fce13d014bf726b33d1a3144ab11d383a6ab3bd7ee49ac73714f68722a0ed1464638335386557d84feb",
	"3076301006072a8648ce3d020106052b8104002203620004f9aacdc6fd011a31ff138c0abd9c762c1f503c32fd6b88b77dae5b608c08c40e9f4cd713668abbf4317742996eaaec73f06f41b35e692db3b6e64230c4f58cb9867aa398923b69f380644827ee79fb9529e31a7e07b1c79624fdf297577fe06a",
	"3076301006072a8648ce3d020106052b8104002203620004d824796938133dfe3d010b9c4e9105e7e75b3dd2e4a3635472b3ee996d5b90eef5efcf5fc051da91a854f861833e5b63e914d9a3c53946f05f6dda3669122994c55d3208fdd606ee6b95a69e8981fe6854a2deedffa047668cd0f793cb1ef0c4",
};

/* P-521: 4 generated SubjectPublicKeyInfo encodings */
static const char *const spki_P521[] = {
	"30819b301006072a8648ce3d020106052b8104002303818600040013509611cc7f6974d5878e037d733a5158d76fdf55487af9a25d3b19cae2760482021f9fb281377a8fdd4d6b02c2140149d9efb9aa4b07946ae5edd76cd98ab231012d08dc522e378661f415a3713cc457efe8cd388888cf1cecb76b878e1280eb03f8b9b68c5200b32f1025942775e19f7e249d2540b3ca4a2697014c070284899f84",
	"30819b301006072a8648ce3d020106052b81040023038186000401689056d5c94c948cd1322043ecdbe25fa6b0ea0d8e3f18247e4fc9042eaeac86a9a86fb32251a75909debf030a846630c20caad0adba2a0ccc07e30b814b6a61d901303006d08b9cb065151fe7718c4eceab39e121c4f7487c0ee8ef6514f01d6d6087aaeb419969c22a75e69872b64301003316787f784dbc19465013ac3dedc06e3c",
	"30819b301006072a8648ce3d020106052b8104002303818600040122e5e26724cd8d9ae1e937bdfc1d715ba75bfdfab6758f9e71c823621fe0e6e6f15dde2091546429b52cc0d80313737571fdc6d8af2748229809ada03f51bc6a2300a5323e0e3e4604d4acdd303afc494e8c87793ff24b5efa82b708db2ea2ea7aebe3f058af982a18b13f8fafef20221b2c146084540909eaa1237f05c837ccf8270b",
	"30819b301006072a8648ce3d020106052b810400230381860004002e01f544f8d9aea68a80bc6628f952a4427b02bf1aedcb3a987da5fd4f73b9cb49df69c72ed5933515736ebd6a99512620abdafc6286dc16321e329c6c212fb3b501ee346750d5f851c2bcd2d927ccf022934db9bb476180569c7ed129b25fd764737c50f39adad6c11409197bf31c0ef9412abf8935b54eba24a960b196a3b5187f1a",
};

typedef void EVP_PKEY;
typedef void EC_KEY;

static void *(*p_d2i_PUBKEY)(void **, const unsigned char **, long);
static EC_KEY *(*p_EVP_PKEY_get1_EC_KEY)(EVP_PKEY *);
static int (*p_EC_KEY_check_key)(const EC_KEY *);
static void (*p_EC_KEY_free)(EC_KEY *);
static void (*p_EVP_PKEY_free)(EVP_PKEY *);

static int failures;

static int
unhex(const char *hex, unsigned char *out)
{
	size_t n = strlen(hex) / 2;

	for (size_t i = 0; i < n; i++) {
		unsigned v;
		if (sscanf(hex + 2 * i, "%2x", &v) != 1)
			return -1;
		out[i] = (unsigned char)v;
	}
	return (int)n;
}

// returns 1 when the encoding decodes to a key that passes EC_KEY_check_key()
static int
decode_ok(const unsigned char *der, int len)
{
	const unsigned char *p = der;
	EVP_PKEY *pkey = p_d2i_PUBKEY(NULL, &p, len);
	EC_KEY *ec;
	int ok;

	if (pkey == NULL)
		return 0;
	ec = p_EVP_PKEY_get1_EC_KEY(pkey);
	ok = ec != NULL && p_EC_KEY_check_key(ec) == 1;
	if (ec != NULL)
		p_EC_KEY_free(ec);
	p_EVP_PKEY_free(pkey);
	return ok;
}

static void
run_curve(const char *name, const char *const *vectors, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		unsigned char der[256];
		int len = unhex(vectors[i], der);

		if (len <= 0 || !decode_ok(der, len)) {
			printf("FAIL %s key %zu: valid public key was rejected\n", name, i);
			failures++;
			continue;
		}
		// flip a bit of the last coordinate byte: with overwhelming
		// probability this is no longer a point on the curve
		der[len - 1] ^= 1;
		if (decode_ok(der, len)) {
			printf("FAIL %s key %zu: off-curve point was accepted\n", name, i);
			failures++;
		}
	}
}

int
main(int argc, char **argv)
{
	const char *path = argc > 1 ? argv[1] : "/usr/lib/libcrypto.44.dylib";
	void *h = dlopen(path, RTLD_NOW);

	if (h == NULL) {
		printf("FAIL dlopen %s: %s\n", path, dlerror());
		return 2;
	}
	p_d2i_PUBKEY = dlsym(h, "d2i_PUBKEY");
	p_EVP_PKEY_get1_EC_KEY = dlsym(h, "EVP_PKEY_get1_EC_KEY");
	p_EC_KEY_check_key = dlsym(h, "EC_KEY_check_key");
	p_EC_KEY_free = dlsym(h, "EC_KEY_free");
	p_EVP_PKEY_free = dlsym(h, "EVP_PKEY_free");
	if (!p_d2i_PUBKEY || !p_EVP_PKEY_get1_EC_KEY || !p_EC_KEY_check_key ||
	    !p_EC_KEY_free || !p_EVP_PKEY_free) {
		printf("FAIL missing symbols in %s\n", path);
		return 2;
	}

	run_curve("P-256", spki_P256, sizeof(spki_P256) / sizeof(spki_P256[0]));
	run_curve("P-384", spki_P384, sizeof(spki_P384) / sizeof(spki_P384[0]));
	run_curve("P-521", spki_P521, sizeof(spki_P521) / sizeof(spki_P521[0]));

	if (failures) {
		printf("%d failure(s) with %s\n", failures, path);
		return 1;
	}
	printf("PASS %s\n", path);
	return 0;
}
