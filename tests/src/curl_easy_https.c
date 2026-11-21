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

// libcurl.4 (curl 8.7.1): curl_version_info reports 8.7.1 with LibreSSL 3.3.6
// as the TLS backend, zlib, HTTP/2 and HTTPS support; the easy API downloads
// an https URL into memory with certificate verification, and a request for
// a host whose certificate has expired fails with CURLE_PEER_FAILED_VERIFICATION.
// Needs network access.
//
//   curl_easy_https.c -I<darling>/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
//       /usr/lib/libcurl.4.dylib

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(cond, ...) do { \
	if (!(cond)) { \
		printf("FAIL line %d: ", __LINE__); \
		printf(__VA_ARGS__); \
		printf("\n"); \
		failures++; \
	} \
} while (0)

struct buffer {
	char* data;
	size_t size;
};

static size_t collect(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	struct buffer* buf = userdata;
	size_t n = size * nmemb;
	char* grown = realloc(buf->data, buf->size + n + 1);
	if (!grown)
		return 0;
	buf->data = grown;
	memcpy(buf->data + buf->size, ptr, n);
	buf->size += n;
	buf->data[buf->size] = '\0';
	return n;
}

static CURLcode fetch(const char* url, struct buffer* buf, long* status, long http_version)
{
	CURL* easy = curl_easy_init();
	CURLcode rc;

	buf->data = NULL;
	buf->size = 0;
	curl_easy_setopt(easy, CURLOPT_URL, url);
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, collect);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, buf);
	curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(easy, CURLOPT_TIMEOUT, 60L);
	curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, "darling-curl-test");
	if (http_version)
		curl_easy_setopt(easy, CURLOPT_HTTP_VERSION, http_version);
	rc = curl_easy_perform(easy);
	*status = 0;
	if (rc == CURLE_OK)
		curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, status);
	curl_easy_cleanup(easy);
	return rc;
}

int main(void)
{
	curl_version_info_data* info;
	struct buffer buf;
	long status;
	CURLcode rc;

	CHECK(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK, "curl_global_init");

	info = curl_version_info(CURLVERSION_NOW);
	CHECK(info != NULL, "curl_version_info returned NULL");
	if (info) {
		CHECK(strcmp(info->version, "8.7.1") == 0, "version is %s", info->version);
		CHECK(info->version_num == 0x080701, "version_num is 0x%x", info->version_num);
		CHECK(info->ssl_version && strstr(info->ssl_version, "LibreSSL/3.3.6"),
			"ssl_version is %s", info->ssl_version ? info->ssl_version : "(null)");
		CHECK(info->libz_version != NULL, "no zlib");
		CHECK(info->features & CURL_VERSION_SSL, "SSL feature missing");
		CHECK(info->features & CURL_VERSION_LIBZ, "libz feature missing");
		CHECK(info->features & CURL_VERSION_HTTP2, "HTTP2 feature missing");
		CHECK(info->features & CURL_VERSION_ASYNCHDNS, "AsynchDNS feature missing");
		CHECK(info->features & CURL_VERSION_LARGEFILE, "Largefile feature missing");
		CHECK(info->features & CURL_VERSION_UNIX_SOCKETS, "UnixSockets feature missing");
		CHECK(info->features & CURL_VERSION_HTTPS_PROXY, "HTTPS-proxy feature missing");
		int https = 0;
		for (const char* const* p = info->protocols; p && *p; p++)
			if (strcmp(*p, "https") == 0)
				https = 1;
		CHECK(https, "https is not a supported protocol");
	}

	rc = fetch("https://github.com/", &buf, &status, 0);
	CHECK(rc == CURLE_OK, "github.com: %s", curl_easy_strerror(rc));
	CHECK(status == 200, "github.com status %ld", status);
	CHECK(buf.size > 1000 && buf.data && strstr(buf.data, "<html"), "github.com body of %zu bytes", buf.size);
	free(buf.data);

	rc = fetch("https://www.cloudflare.com/", &buf, &status, CURL_HTTP_VERSION_1_1);
	CHECK(rc == CURLE_OK, "cloudflare.com over HTTP/1.1: %s", curl_easy_strerror(rc));
	CHECK(status == 200, "cloudflare.com status %ld", status);
	free(buf.data);

	rc = fetch("https://expired.badssl.com/", &buf, &status, 0);
	CHECK(rc == CURLE_PEER_FAILED_VERIFICATION, "expired certificate gave %d (%s)", rc, curl_easy_strerror(rc));
	free(buf.data);

	curl_global_cleanup();
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
