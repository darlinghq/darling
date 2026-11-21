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

// libssl.48/libcrypto.46 (LibreSSL 3.3.6): the library reports its version,
// offers TLS 1.3, and completes a TLS 1.3 handshake and data exchange over a
// local socketpair between two threads, with a generated self-signed P-256
// certificate (no network).  A server limited to TLS 1.2 must still negotiate
// TLS 1.2 with the same client, so 1.3 is offered rather than forced.
//
// Build against the LibreSSL 3.3.6 headers and link the installed dylibs:
//   libressl_tls13_loopback.c -I<libressl-3.3.6>/include
//       /usr/lib/libcrypto.46.dylib /usr/lib/libssl.48.dylib

// LibreSSL's public headers expose the TLS 1.3 constants only on request
#define LIBRESSL_HAS_TLS1_3

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/opensslv.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Throw-away key pair generated only for this test.
static const char test_cert_pem[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBfzCCASWgAwIBAgIUNh1DlOWt3xpaczu8FHFWJztL2KcwCgYIKoZIzj0EAwIw\n"
	"FDESMBAGA1UEAwwJbG9jYWxob3N0MCAXDTI2MTExNzE3MDcxOVoYDzIxMjYxMDI0\n"
	"MTcwNzE5WjAUMRIwEAYDVQQDDAlsb2NhbGhvc3QwWTATBgcqhkjOPQIBBggqhkjO\n"
	"PQMBBwNCAAQhVxZlWuqQdAIMUs2vBOYJYaEmh4rbA2g5KAAh3m1anvvsFhIGO/Un\n"
	"sLiD08J3dNdvzb3d16t1O54dJkM2qhsHo1MwUTAdBgNVHQ4EFgQUZmNYWtUGDQYX\n"
	"L0z7E8vsi9IFb/0wHwYDVR0jBBgwFoAUZmNYWtUGDQYXL0z7E8vsi9IFb/0wDwYD\n"
	"VR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAgNIADBFAiBZlYWixugCkvTkBc+s6fPL\n"
	"kFd3418HWkhwcKPhe19ThAIhAPi/dqmSkeTOMEeBnR/jMCmUgQJz93GGaiPysvWs\n"
	"FJro\n"
	"-----END CERTIFICATE-----\n";

static const char test_key_pem[] =
	"-----BEGIN PRIVATE KEY-----\n"
	"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgwElsQ9PCLhifujOs\n"
	"6lpc3KI8/Cm0CJ1C7fMErwMJB4ahRANCAAQhVxZlWuqQdAIMUs2vBOYJYaEmh4rb\n"
	"A2g5KAAh3m1anvvsFhIGO/UnsLiD08J3dNdvzb3d16t1O54dJkM2qhsH\n"
	"-----END PRIVATE KEY-----\n";

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		printf("FAIL line %d: ", __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		failures++; \
	} \
} while (0)

struct peer {
	int fd;
	int max_version;	// 0: library default
	int ok;
	int version;
	char version_name[16];
};

static SSL_CTX *
make_ctx(int server, int max_version)
{
	SSL_CTX *ctx = SSL_CTX_new(server ? TLS_server_method() : TLS_client_method());

	if (ctx == NULL)
		return NULL;
	if (max_version != 0)
		SSL_CTX_set_max_proto_version(ctx, max_version);
	if (server) {
		BIO *cb = BIO_new_mem_buf((void *)test_cert_pem, -1);
		BIO *kb = BIO_new_mem_buf((void *)test_key_pem, -1);
		X509 *cert = cb ? PEM_read_bio_X509(cb, NULL, NULL, NULL) : NULL;
		EVP_PKEY *key = kb ? PEM_read_bio_PrivateKey(kb, NULL, NULL, NULL) : NULL;
		int ok = cert && key && SSL_CTX_use_certificate(ctx, cert) == 1 &&
		    SSL_CTX_use_PrivateKey(ctx, key) == 1 && SSL_CTX_check_private_key(ctx) == 1;

		X509_free(cert);
		EVP_PKEY_free(key);
		BIO_free(cb);
		BIO_free(kb);
		if (!ok) {
			SSL_CTX_free(ctx);
			return NULL;
		}
	} else {
		// the certificate is self-signed and not in any trust store; its
		// public key is still decoded and used to verify the handshake
		SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
	}
	return ctx;
}

static void
record_version(struct peer *p, SSL *ssl)
{
	const char *v = SSL_get_version(ssl);

	p->version = SSL_version(ssl);
	snprintf(p->version_name, sizeof(p->version_name), "%s", v ? v : "?");
}

static void *
server_thread(void *arg)
{
	struct peer *p = arg;
	SSL_CTX *ctx = make_ctx(1, p->max_version);
	SSL *ssl = ctx ? SSL_new(ctx) : NULL;
	char buf[16];

	if (ssl == NULL || SSL_set_fd(ssl, p->fd) != 1 || SSL_accept(ssl) != 1)
		goto out;
	record_version(p, ssl);
	if (SSL_read(ssl, buf, sizeof(buf)) != 4 || memcmp(buf, "ping", 4) != 0)
		goto out;
	if (SSL_write(ssl, "pong", 4) != 4)
		goto out;
	p->ok = 1;
out:
	if (!p->ok)
		ERR_print_errors_fp(stdout);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	return NULL;
}

static void
run_handshake(const char *label, int server_max, int expect_version, const char *expect_name)
{
	struct peer server = { .max_version = server_max };
	struct peer client = { 0 };
	SSL_CTX *ctx;
	SSL *ssl;
	pthread_t thread;
	char buf[16];
	int sv[2];

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) {
		CHECK(0, "%s: socketpair", label);
		return;
	}
	server.fd = sv[0];
	client.fd = sv[1];
	pthread_create(&thread, NULL, server_thread, &server);

	ctx = make_ctx(0, 0);
	ssl = ctx ? SSL_new(ctx) : NULL;
	if (ssl != NULL && SSL_set_fd(ssl, client.fd) == 1 && SSL_connect(ssl) == 1) {
		record_version(&client, ssl);
		if (SSL_write(ssl, "ping", 4) == 4 &&
		    SSL_read(ssl, buf, sizeof(buf)) == 4 && memcmp(buf, "pong", 4) == 0)
			client.ok = 1;
	}
	if (!client.ok)
		ERR_print_errors_fp(stdout);
	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(client.fd);
	pthread_join(thread, NULL);
	close(server.fd);

	CHECK(client.ok, "%s: client handshake or exchange failed", label);
	CHECK(server.ok, "%s: server handshake or exchange failed", label);
	CHECK(client.version == expect_version, "%s: client version 0x%x, expected 0x%x",
	    label, client.version, expect_version);
	CHECK(server.version == expect_version, "%s: server version 0x%x, expected 0x%x",
	    label, server.version, expect_version);
	CHECK(strcmp(client.version_name, expect_name) == 0, "%s: version name %s, expected %s",
	    label, client.version_name, expect_name);
}

int
main(void)
{
	const char *runtime = OpenSSL_version(OPENSSL_VERSION);

	CHECK(runtime != NULL && strcmp(runtime, "LibreSSL 3.3.6") == 0,
	    "OpenSSL_version() is \"%s\"", runtime ? runtime : "(null)");
	CHECK(strcmp(SSLeay_version(SSLEAY_VERSION), "LibreSSL 3.3.6") == 0,
	    "SSLeay_version() is \"%s\"", SSLeay_version(SSLEAY_VERSION));
	CHECK(strcmp(LIBRESSL_VERSION_TEXT, "LibreSSL 3.3.6") == 0, "headers are not 3.3.6");

	run_handshake("default", 0, TLS1_3_VERSION, "TLSv1.3");
	run_handshake("server max TLS 1.2", TLS1_2_VERSION, TLS1_2_VERSION, "TLSv1.2");

	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS %s\n", runtime);
	return 0;
}
