#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <limits.h>
#include <getopt.h>
#include <regex.h>
#include <errno.h>
#include <openssl/evp.h>
#include <xar/xar.h>

off_t HeapOff = 0;

static char* xar_format_md5(const unsigned char* m) {
        char* result = NULL;
        asprintf(&result,
                "%02x%02x%02x%02x"
                "%02x%02x%02x%02x"
                "%02x%02x%02x%02x"
                "%02x%02x%02x%02x",
                m[0], m[1], m[2], m[3],
                m[4], m[5], m[6], m[7],
                m[8], m[9], m[10], m[11],
                m[12], m[13], m[14], m[15]);
        return result;
}

void heap_check(int fd, const char *name, const char *prop, off_t offset, off_t length, const char *csum) {
	char *buf;
	EVP_MD_CTX ctx;
	const EVP_MD *md;
	unsigned char md5str[EVP_MAX_MD_SIZE];
	unsigned int len;
	ssize_t r;
	char *formattedmd5;

	fprintf(stderr, "Heap checking %s %s at offset: %" PRIu64 "\n", name, prop, HeapOff+offset);
	OpenSSL_add_all_digests();
	md = EVP_get_digestbyname("md5");
	if( md == NULL ) {
		fprintf(stderr, "No md5 digest in openssl\n");
		exit(1);
	}
	EVP_DigestInit(&ctx, md);

	buf = malloc(length);
	if( !buf ) {
		fprintf(stderr, "Error allocating buffer\n");
		exit(1);
	}

	if( lseek(fd, HeapOff+offset, SEEK_SET) < 0 ) {
		perror("lseek");
		fprintf(stderr, "Error seeking in the heap to offet: %" PRIu64 "\n", HeapOff+offset);
		exit(1);
	}

	r = read(fd, buf, length);
	if( r != length ) {
		fprintf(stderr, "Error reading from the heap\n");
		exit(1);
	}
	EVP_DigestUpdate(&ctx, buf, length);
	EVP_DigestFinal(&ctx, md5str, &len);

	formattedmd5 = xar_format_md5(md5str);
	if( strcmp(formattedmd5, csum) != 0 ) {
		fprintf(stderr, "%s %s checksum does not match\n", name, prop);
	}
	free(formattedmd5);
	free(buf);
}

void prop_check(int fd, xar_t x, xar_file_t f) {
	xar_iter_t i;
	const char *key, *value;
	const char *name = NULL;
	char *tmp;
	off_t offset = 0, length = 0;
	const char *origcsum = NULL;
	
	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating new prop iter\n");
		exit(1);
	}

	xar_prop_get(f, "name", &name);

	for( key = xar_prop_first(f, i); key; key = xar_prop_next(i) ) {
		asprintf(&tmp, "%s/offset", key);
		if( xar_prop_get(f, tmp, &value) == 0 ) {
			offset = strtoll(value, NULL, 10);
		}
		free(tmp);
		asprintf(&tmp, "%s/length", key);
		if( xar_prop_get(f, tmp, &value) == 0 ) {
			length = strtoll(value, NULL, 10);
		}
		free(tmp);
		asprintf(&tmp, "%s/archived-checksum", key);
		if( xar_prop_get(f, tmp, &value) == 0 ) {
			origcsum = value;
		}
		free(tmp);

		if( offset && length && origcsum ) {
			heap_check(fd, name, key, offset, length, origcsum);
			offset = 0;
			length = 0;
			origcsum = NULL;
		}
	}

}

int main(int argc, char *argv[]) {
	char *file = argv[1];
	xar_t x;
	xar_iter_t ifile, iprop, iattr;
	xar_file_t f;
	const char *key;
	int fd;
	off_t off;
	xar_header_t hdr;
	
	x = xar_open(file, READ);
	if( !x ) {
		fprintf(stderr, "Error opening archive\n");
		exit(1);
	}

	fd = open(file, O_RDONLY);
	if( fd < 0 ) {
		fprintf(stderr, "Error opening archive\n");
		exit(1);
	}

	read(fd, &hdr, sizeof(hdr));
	hdr.size = htons(hdr.size);
	hdr.toc_length_compressed = xar_ntoh64(hdr.toc_length_compressed);
	HeapOff = hdr.size + hdr.toc_length_compressed;

	ifile = xar_iter_new();
	if( !ifile ) {
		fprintf(stderr, "Error creating file iterator");
		exit(1);
	}

	for(f = xar_file_first(x, ifile); f; f = xar_file_next(ifile)) {
		prop_check(fd, x, f);
	}
	
}
