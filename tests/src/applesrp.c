// SRP-6a exchange through AppleSRP using the RFC 5054 appendix B group, salt and verifier.
#include <AppleSRP/srp.h>
#include <stdio.h>
#include <string.h>

static int failures;

static void check(int ok, const char *what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void free_all(cstr **strs, int n)
{
	for (int i = 0; i < n; i++)
		if (strs[i])
			cstr_free(strs[i]);
}

static int unhex(const char *hex, unsigned char *out)
{
	int n = 0;
	for (; hex[0] && hex[1]; hex += 2)
		sscanf(hex, "%2hhx", &out[n++]);
	return n;
}

static const char *N_hex =
	"EEAF0AB9ADB38DD69C33F80AFA8FC5E86072618775FF3C0B9EA2314C9C256576D674DF74"
	"96EA81D3383B4813D692C6E0E0D5D8E250B98BE48E495C1D6089DAD15DC7D7B46154D6B6"
	"CE8EF4AD69B15D4982559B297BCF1885C529F566660E57EC68EDBC3C05726CC02FD4CBF4"
	"976EAA9AFD5138FE8376435B9FC61D2FC0EB06E3";
static const char *salt_hex = "BEB25379D1A8581EB5A727673A2441EE";
static const char *x_hex = "94B7555AABE9127CC58CCF4993DB6CF84D16C124";
static const char *v_hex =
	"7E273DE8696FFC4F4E337D05B4B375BEB0DDE1569E8FA00A9886D8129BADA1F1822223CA"
	"1A605B530E379BA4729FDC59F105B4787E5186F5C671085A1447B52A48CF1970B4FB6F84"
	"00BBF4CEBFBB168152E08AB5EA53D15C1AFF87B2B9DA6E04E058AD51CC72BFC9033B564E"
	"26480D78E955A5E29E7AB245DB2BE315E2099AFB";

static unsigned char N[128], salt[16], x[20], v[128];
static int Nlen, saltlen, xlen, vlen;
static const unsigned char g[] = { 2 };

static SRP *client(const char *password)
{
	SRP *srp = SRP_new(SRP6a_client_method());
	if (!srp || !SRP_OK(SRP_set_username(srp, "alice"))
		|| !SRP_OK(SRP_set_params(srp, N, Nlen, g, sizeof(g), salt, saltlen))
		|| !SRP_OK(SRP_set_auth_password(srp, password)))
		return NULL;
	return srp;
}

static SRP *server(void)
{
	SRP *srp = SRP_new(SRP6a_server_method());
	if (!srp || !SRP_OK(SRP_set_username(srp, "alice"))
		|| !SRP_OK(SRP_set_params(srp, N, Nlen, g, sizeof(g), salt, saltlen))
		|| !SRP_OK(SRP_set_authenticator(srp, v, vlen)))
		return NULL;
	return srp;
}

// Runs one exchange; returns 1 when both sides accepted each other's proof.
static int exchange(const char *password, int *keys_match)
{
	SRP *c = client(password), *s = server();
	cstr *A = NULL, *B = NULL, *Kc = NULL, *Ks = NULL, *M1 = NULL, *M2 = NULL;
	int ok = 0;

	*keys_match = 0;
	if (c && s && SRP_OK(SRP_gen_pub(c, &A)) && SRP_OK(SRP_gen_pub(s, &B))
		&& A->length == Nlen && B->length == Nlen
		&& SRP_OK(SRP_compute_key(c, &Kc, (unsigned char *)B->data, B->length))
		&& SRP_OK(SRP_compute_key(s, &Ks, (unsigned char *)A->data, A->length))) {
		*keys_match = Kc->length == Ks->length && memcmp(Kc->data, Ks->data, Kc->length) == 0;
		if (SRP_OK(SRP_respond(c, &M1))
			&& SRP_OK(SRP_verify(s, (unsigned char *)M1->data, M1->length))
			&& SRP_OK(SRP_respond(s, &M2))
			&& SRP_OK(SRP_verify(c, (unsigned char *)M2->data, M2->length)))
			ok = 1;
	}
	free_all((cstr *[]){ A, B, Kc, Ks, M1, M2 }, 6);
	if (c) SRP_free(c);
	if (s) SRP_free(s);
	return ok;
}

int main(void)
{
	Nlen = unhex(N_hex, N);
	saltlen = unhex(salt_hex, salt);
	xlen = unhex(x_hex, x);
	vlen = unhex(v_hex, v);

	BigInteger bx = BigIntegerFromBytes(x, xlen), bN = BigIntegerFromBytes(N, Nlen);
	BigInteger bg = BigIntegerFromInt(2), bv = BigIntegerFromInt(0);
	unsigned char computed[128];
	BigIntegerModExp(bv, bg, bx, bN, NULL, NULL);
	check(BigIntegerToBytes(bv, computed, sizeof(computed)) == vlen && memcmp(computed, v, vlen) == 0,
		"BigIntegerModExp gives the RFC 5054 verifier v = g^x mod N");
	BigIntegerFree(bx); BigIntegerFree(bN); BigIntegerFree(bg); BigIntegerFree(bv);

	int keys_match;
	check(exchange("password123", &keys_match), "client and server accept each other's proofs");
	check(keys_match, "client and server derive the same session key");
	check(!exchange("password124", &keys_match), "wrong password is rejected");
	check(!keys_match, "wrong password derives a different session key");

	SRP *c = SRP_new(SRP6a_client_method());
	unsigned char bad_N[128];
	memcpy(bad_N, N, Nlen);
	bad_N[Nlen - 1] ^= 2;
	SRP_set_username(c, "alice");
	SRP_set_client_param_verify_cb(c, SRP_CLIENT_builtin_param_verify_cb);
	check(!SRP_OK(SRP_set_params(c, bad_N, Nlen, g, sizeof(g), salt, saltlen)),
		"built-in group check refuses an unknown modulus");
	check(SRP_OK(SRP_set_params(c, N, Nlen, g, sizeof(g), salt, saltlen)),
		"built-in group check accepts the RFC 5054 1024-bit group");
	SRP_free(c);

	c = client("password123");
	cstr *A = NULL, *K = NULL;
	check(SRP_OK(SRP_gen_pub(c, &A)) && !SRP_OK(SRP_compute_key(c, &K, N, Nlen)),
		"client refuses a server value B >= N");
	free_all((cstr *[]){ A, K }, 2);
	SRP_free(c);

	printf("failures=%d\n", failures);
	return failures != 0;
}
