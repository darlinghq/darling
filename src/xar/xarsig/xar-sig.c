/*
 * Copyright (c) 2005 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of his contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 03-Apr-2005
 * DRI: Rob Braun <bbraun@opendarwin.org>
 */

#include <spawn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlstring.h>
#include <limits.h>
#include <getopt.h>
#include <regex.h>
#include <errno.h>
#ifdef XARSIG_BUILDING_WITH_XAR
#include "xar.h"
#else
#include <xar/xar.h>
#endif // XARSIG_BUILDING_WITH_XAR
#include "filetree.h"

#define SYMBOLIC 1
#define NUMERIC  2

// error codes for B&I
#define E_NOSIG		60
#define E_SIGEXISTS 61

extern char **environ;

static int Perms = 0;
static int Local = 0;
static char *Subdoc = NULL;
static char *SubdocName = NULL;
static char *Toccksum = NULL;
static char *Filecksum = NULL;
static char *Compression = NULL;
static char *Rsize = NULL;
static char *DataToSignDumpPath = NULL;
static char *SigOffsetDumpPath = NULL;
static char *LeafCertPath = NULL;
static char *IntermediateCertPath = NULL;
static char *RootCertPath = NULL;

static int Err = 0;
static int Verbose = 0;
static int Coalesce = 0;
static int LinkSame = 0;
static int DoSign = 0;

static long SigSize = 0;

struct lnode {
	char *str;
	regex_t reg;
	struct lnode *next;
};
struct __stack_element {
	struct __stack_element *prev;
	struct __stack_element *next;
	void *data;
};
struct __stack {
	struct __stack_element *bottom;
	struct __stack_element *top;
};

typedef struct __stack_element *stack_element;
typedef struct __stack *stack;

struct lnode *Exclude = NULL;
struct lnode *Exclude_Tail = NULL;
struct lnode *NoCompress = NULL;
struct lnode *NoCompress_Tail = NULL;

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx);
static int32_t signingCallback(xar_signature_t sig, void *context, uint8_t *data, uint32_t length, uint8_t **signed_data, uint32_t *signed_len);
static void insert_cert(xar_signature_t sig, const char *cert_path);
static void add_subdoc(xar_t x);

static void _set_xarsig_opts_from_args(xar_t x) {
	// Update checksum algorithms before we start messing with the heap length in xar_signature_new()
	if( Toccksum )
		xar_opt_set(x, XAR_OPT_TOCCKSUM, Toccksum);
	
	if( Filecksum )
		xar_opt_set(x, XAR_OPT_FILECKSUM, Filecksum);
	
	if ( SigSize ) {
		xar_signature_t sig = xar_signature_new(x, "RSA", SigSize, &signingCallback, NULL);
		if ( LeafCertPath )
			insert_cert(sig, LeafCertPath);
		if ( IntermediateCertPath )
			insert_cert(sig, IntermediateCertPath);
		if ( RootCertPath )
			insert_cert(sig, RootCertPath);
	}
	
	if( Compression )
		xar_opt_set(x, XAR_OPT_COMPRESSION, Compression);
	
	if( Coalesce )
		xar_opt_set(x, XAR_OPT_COALESCE, "true");
	
	if( LinkSame )
		xar_opt_set(x, XAR_OPT_LINKSAME, "true");
	
	if ( Rsize != NULL )
		xar_opt_set(x, XAR_OPT_RSIZE, Rsize);
	
	xar_register_errhandler(x, err_callback, NULL);
	
	if( Subdoc )
		add_subdoc(x);
	
	if( Perms == SYMBOLIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_SYMBOLIC);
	}
	if( Perms == NUMERIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_NUMERIC);
	}
}

static void print_file(xar_file_t f) {
	if( Verbose ) {
		char *path = xar_get_path(f);
		printf("%s\n", path);
		free(path);
	}
}

static void add_subdoc(xar_t x) {
	xar_subdoc_t s;
	int fd;
	unsigned char *buf;
	unsigned int len;
	struct stat sb;

	if( SubdocName == NULL ) SubdocName = "subdoc";
	s = xar_subdoc_new(x, (const char *)SubdocName);

	fd = open(Subdoc, O_RDONLY);
	if( fd < 0 )
		return;
	fstat(fd, &sb);
	len = sb.st_size;
	buf = malloc(len+1);
	if( buf == NULL ) {
		close(fd);
		return;
	}
	memset(buf, 0, len+1);
	read(fd, buf, len);
	close(fd);

	xar_subdoc_copyin(s, buf, len);


	return;
}

static void extract_subdoc(xar_t x, const char *name) {
	xar_subdoc_t i;

	for( i = xar_subdoc_first(x); i; i = xar_subdoc_next(i) ) {
		const char *sname = xar_subdoc_name(i);
		unsigned char *sdoc;
		int fd, size;
		if( name && strcmp(name, sname) != 0 )
			continue;
		xar_subdoc_copyout(i, &sdoc, (unsigned int *)&size);
		fd = open(Subdoc, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		if( fd < 0 ) return;
		write(fd, sdoc, size);
		close(fd);
		free(sdoc);
	}

	return;
}

static void extract_data_to_sign(const char *filename) {
	xar_signature_t sig;
	off_t signatureOffset;
	FILE *file;
	xar_t x;
	int i;
	uint32_t dataToSignOffset = 0;
	uint32_t dataToSignSize = 0;
	char *buffer = NULL;
	const char *value;
	
	// find signature stub
	x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open %s to extract data to sign!\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( !sig ) {
		fprintf(stderr, "No signatures found to extract data from.\n");
		exit(E_NOSIG);
	}

	// locate data to sign
	if( 0 != xar_prop_get((xar_file_t)x, "checksum/offset" ,&value) ) {
		fprintf(stderr, "Could not locate checksum/offset in archive.\n");
		exit(1);
	}
	dataToSignOffset = xar_get_heap_offset(x);
	dataToSignOffset += strtoull(value, (char **)NULL, 10);
	if( 0 != xar_prop_get((xar_file_t)x, "checksum/size" ,&value) ) {
		fprintf(stderr, "Could not locate checksum/size in archive.\n");
		exit(1);
	}
	dataToSignSize = strtoull(value, (char **)NULL, 10);

	// get signature offset (inject signature here) 
	xar_signature_copy_signed_data(sig, NULL, NULL, NULL, NULL, &signatureOffset);
	signatureOffset += xar_get_heap_offset(x);
	xar_close(x);
	
	// now get data to be signed, using offset and size
	file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "Could not open %s for reading data to sign!\n", filename);
		exit(1);
	}
	fseek(file, dataToSignOffset, SEEK_SET);
	buffer = malloc(dataToSignSize);
	i = fread(buffer, dataToSignSize, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to read data to sign from %s!\n", filename);
		exit(1);
	}
	fclose(file);
	
	// save data to sign
	file = fopen(DataToSignDumpPath, "w");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving data to sign!\n", DataToSignDumpPath);
		exit(1);
	}
	i = fwrite(buffer, dataToSignSize, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to write data to sign to %s (fwrite() returned %i)!\n", DataToSignDumpPath, i);
		exit(1);
	}
	fclose(file);

	// save signature offset
	file = fopen(SigOffsetDumpPath, "w");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving signature offset!\n", SigOffsetDumpPath);
		exit(1);
	}
	i = fprintf(file, "%lli\n", signatureOffset);
	if (i < 0) {
		fprintf(stderr, "Failed to write signature offset to %s (fprintf() returned %i)!\n", SigOffsetDumpPath, i);
		exit(1);
	}
	fclose(file);
	
	free(buffer);
}

static void extract_certs(char *filename, char *cert_base_path) {
	xar_signature_t sig;
	xar_t x;
	int32_t count;
	int i;
	const uint8_t *cert_data;
	uint32_t cert_len;
	FILE *file;
	char *cert_path;
	
	// open xar, get signature
	x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open %s to extract certificates!\n", filename);
		exit(1);
	}
	sig = xar_signature_first(x);
	if ( !sig ) {
		fprintf(stderr, "No signatures found to extract data from.\n");
		exit(E_NOSIG);
	}
	
	// iterate through all certificates associated with that signature, write them to disk
	count = xar_signature_get_x509certificate_count(sig);
	if (!count) {
		fprintf(stderr, "Signature bears no certificates. Odd.\n");
		exit(1);
	}
	for (i=0; i<count; i++) {
		xar_signature_get_x509certificate_data(sig, i, &cert_data, &cert_len);
		asprintf(&cert_path, "%s/cert%02i", cert_base_path, i);
		file = fopen(cert_path, "w");
		if (!file) {
			fprintf(stderr, "Could not save certificate %i to %s.\n", i, cert_path);
			exit(1);
		}
		int n = fwrite(cert_data, cert_len, 1, file);
		if (n < 0) {
			fprintf(stderr, "Failed to write certificate to %s (fwrite() returned %i)!\n", cert_path, n);
			exit(1);
		}
		fclose(file);
		free(cert_path);
	}

	// clean up
	xar_close(x);
}

static off_t get_sig_offset(xar_t x) {
	off_t signedDataOffset = 0;

	xar_signature_t sig = xar_signature_first(x);
	xar_signature_copy_signed_data(sig,NULL,NULL,NULL,NULL,&signedDataOffset);
	signedDataOffset += xar_get_heap_offset(x);
	return signedDataOffset;
}

static int extract_sig_offset(xar_t x, const char *filename) {
	off_t signedDataOffset;
	FILE *file;
	int i;
	
	// get offset
	signedDataOffset = get_sig_offset(x);
	
	// and save it to file
	file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "Could not open %s for saving signature offset!\n", filename);
		return 1;
	}
	i = fprintf(file, "%lli\n", signedDataOffset);
	if (i < 0) {
		fprintf(stderr, "Failed to write signature offset to %s (fprintf() returned %i)!\n", filename, i);
		return 1;
	}
	fclose(file);
	
	return 0;
}

stack stack_new() {
	stack s = (stack)malloc(sizeof(struct __stack));
	s->bottom = s->top = NULL;
	return s;
}
void stack_free(stack s) {
	free(s);
}
void stack_push(stack s, void *data) {
	stack_element e = malloc(sizeof(struct __stack_element));
	e->data = data;
	if (s->top) {
		s->top->next = e;
		e->prev = s->top;
	} else {
		s->top = s->bottom = e;
		e->prev = NULL;
	}
	e->next = NULL;
	s->top = e;
}
void *stack_pop(stack s) {
	if (!s->top)
		return NULL;
	void *ret = s->top->data;
	stack_element temp = s->top;
	s->top = s->top->prev;
	free(temp);
	if (s->top)
		s->top->next = NULL;
	else 
		s->bottom = NULL;
	return ret;
}

/* replace_sign: rip out all current signatures and certs and insert a new pair
		Since libxar is currently not capable of doing this directly, we have to create a new xar archive,
		copy all the files and options from the current archive, and sign the new archive
*/
static void replace_sign(const char *filename) {

	xar_t old_xar, new_xar;
	xar_signature_t sig;
	char *new_xar_path = (char *)malloc(15);
	strncpy(new_xar_path, "/tmp/xar.XXXXX", 15);
	new_xar_path = mktemp(new_xar_path);
	int err;
	pid_t pid;
	char **argv;
	int status;
	
	// open the first archive
	old_xar = xar_open(filename, READ);
	if ( old_xar == NULL ) {
		fprintf(stderr, "Could not open archive %s!\n", filename);
		exit(1);
	}
	
	// use command-line arguments to set xar opts; the copy operation below won't work unless they're set in the first place!
	_set_xarsig_opts_from_args(old_xar);
	
	// open the second archive
	new_xar = xar_open(new_xar_path, WRITE);
	if( !new_xar ) {
		fprintf(stderr, "Error creating new archive %s\n", new_xar_path);
		exit(1);
	}
	
	// copy options
	char *opts[7] = {XAR_OPT_TOCCKSUM, XAR_OPT_FILECKSUM, XAR_OPT_COMPRESSION, XAR_OPT_COALESCE, XAR_OPT_LINKSAME, XAR_OPT_RSIZE, XAR_OPT_OWNERSHIP};
	int i;
	const char *opt;
	for (i=0; i<7; i++) {
		opt = xar_opt_get(old_xar, opts[i]);
		if (opt)
			xar_opt_set(new_xar, opts[i], opt);
	}
	
	// install new signature and new certs in new_xar
	sig = xar_signature_new(new_xar, "RSA", SigSize, &signingCallback, NULL);
	if ( LeafCertPath )
		insert_cert(sig, LeafCertPath);
	if ( IntermediateCertPath )
		insert_cert(sig, IntermediateCertPath);
	if ( RootCertPath )
		insert_cert(sig, RootCertPath);

	// skip copy subdocs for now since we don't use them yet
	
	// copy files
	xar_iter_t iter = xar_iter_new();
	xar_file_t f = xar_file_first(old_xar, iter);
		// xar_file_next iterates the archive depth-first, i.e. all children are enumerated before the siblings.
	const char *name;
	stack s_new = stack_new();
	stack s_old = stack_new();
	xar_file_t last_copied = NULL, last_added;
	
	xar_iter_t loopIter = xar_iter_new();
	xar_file_t current_xar_file;
	for (current_xar_file = xar_file_first(old_xar, loopIter); current_xar_file; current_xar_file = xar_file_next(loopIter))
	{
		printf("old_xar -> %s (parent: %s)\n",xar_get_path(current_xar_file),XAR_FILE(current_xar_file)->parent?xar_get_path(XAR_FILE(current_xar_file)->parent):"(nil)");
	}
	
	do {
		// parent is the parent in the new archive!
		// 3 cases:
		//  1. the file has no parent. Happens for every file at the top level of the archive.
		//  2. the file's parent is the last file we added. Happens while descending down a path
		//  3. the file's parent is one of the ancestors of the last file (and not NULL, that would be case 1)
		//		that means we either go back up the tree and add a sibling of one of the ancestors, or we add a
		//		sibling on the same level
		xar_prop_get(f, "name", &name);	// filename, without any path info
		if (!XAR_FILE(f)->parent) {	// case 1
			printf("root: %s\n",xar_get_path(f));
			last_added = xar_add_from_archive(new_xar, NULL, name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		} else if (f->parent == last_copied) {	// case 2			
			printf("child: %s -> %s\n",xar_get_path(f->parent),xar_get_path(f));
			last_added = xar_add_from_archive(new_xar, last_added, name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		} else {	// case 3
			printf("searching for parent: %s ?\n",xar_get_path(f));
			while (XAR_FILE(f)->parent != XAR_FILE(s_old->top->data)->parent) {
				printf("popping: %s\n",xar_get_path(XAR_FILE(s_old->top->data)));
				stack_pop(s_new);
				stack_pop(s_old);
			}
			printf("found: %s -> %s\n",xar_get_path(XAR_FILE(s_new->top->data)),xar_get_path(f));
			stack_pop(s_new);
			stack_pop(s_old);
			last_added = xar_add_from_archive(new_xar, (xar_file_t)(s_new->top->data), name, old_xar, f);
			last_copied = f;
			stack_push(s_new, (void *)last_added);
			stack_push(s_old, (void *)last_copied);
		}
	} while ((f = xar_file_next(iter)));
	
	loopIter = xar_iter_new();
	for (current_xar_file = xar_file_first(new_xar, loopIter); current_xar_file; current_xar_file = xar_file_next(loopIter))
	{
		char * current_path = xar_get_path(current_xar_file);
		printf("new_xar -> %s\n",current_path);
	}
	
	xar_iter_free(iter);
	stack_free(s_new);
	stack_free(s_old);
	if( xar_close(new_xar) != 0 ) {
		fprintf(stderr, "Error creating the archive\n");
		if( !Err )
			Err = 42;
	}
	xar_close(old_xar);
	
	// write signature offset to file (have to re-open so xar_close can figure out the correct offset)
	new_xar = xar_open(new_xar_path, READ);
	if( !new_xar ) {
		fprintf(stderr, "Error re-opening new archive %s\n", new_xar_path);
		unlink(new_xar_path);
		exit(1);
	}
	
	if (extract_sig_offset(new_xar, SigOffsetDumpPath)) {
		xar_close(new_xar);
		unlink(new_xar_path);
		exit(1);
	}
		
	xar_close(new_xar);
	
	// delete old archive, move new in its place
	unlink(filename);
	argv = calloc(sizeof(char *), 4);
	argv[0] = "cp";
	argv[1] = new_xar_path;
	argv[2] = (char *)filename;
	err = posix_spawn(&pid, "/bin/cp", NULL, NULL, argv, environ);
	free(argv);
	
	if (err) {
		fprintf(stderr, "Could not copy new archive to final location (posix_spawn(3) failed with %d)\n", errno);
		unlink(new_xar_path);
		exit(1);
	}
	
	do {
		err = waitpid(pid, &status, 0);
		if (err == pid) {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) != 0) {
					fprintf(stderr, "Could not copy new archive to final location (cp(1) exited %d)\n", WEXITSTATUS(status));
					unlink(new_xar_path);
					exit(1);
				}
				break;
			}
		} else {
			fprintf(stderr, "Could not copy new archive to final location (waidpid(2) failed with %d)\n", errno);
			unlink(new_xar_path);
			exit(1);
		}
	} while (1);
	
	// Delete the tmp archive
	unlink(new_xar_path);
}

/*	belated_sign
	Prepare a previously unsigned archive for signing by creating a signature placeholder and inserting the certificates.
	Since libxar is currently not capable of doing this directly, we have to create a new xar archive,
	copy all the files and options from the current archive, and sign the new archive
*/
static void belated_sign(const char *filename) {
	xar_t x = xar_open(filename, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open archive %s!\n", filename);
		exit(1);
	}
	xar_signature_t sig = xar_signature_first(x);
	if ( sig ) {
		fprintf(stderr, "Archive has already been signed. Use --replace-sign instead.\n");
		exit(E_SIGEXISTS);
	}
	xar_close(x);
	replace_sign(filename);
}

static int32_t signingCallback(xar_signature_t sig, void *context, uint8_t *data, uint32_t length, uint8_t **signed_data, uint32_t *signed_len) {
		
	// save data to file for later signature
	if (DataToSignDumpPath) {
		FILE *file = fopen(DataToSignDumpPath, "w");
		if (!file) {
			fprintf(stderr, "Could not open %s for saving data to sign!\n", DataToSignDumpPath);
			exit(1);
		}
		int i = fwrite(data, length, 1, file);
		if (i != 1) {
			fprintf(stderr, "Failed to write data to sign to %s (fwrite() returned %i)!\n", DataToSignDumpPath, i);
			exit(1);
		}
		fclose(file);
	}
	
	// now return blank placeholder data
	*signed_data = (uint8_t *)malloc(SigSize);
	memset(*signed_data, 0, SigSize);
	strncpy((char *)*signed_data, "helloworld", 10);	// debug
	*signed_len = SigSize;
	return 0;	// no error
}

static void insert_cert(xar_signature_t sig, const char *cert_path) {
	struct stat *s = malloc(sizeof(struct stat));
	if (stat(cert_path, s) == -1) {
		fprintf(stderr, "Could not stat() certificate file %s (errno == %i)\n", cert_path, errno);
		exit(1);
	}
	void *cert = malloc(s->st_size);
	FILE *file = fopen(cert_path, "r");
	if (!file) {
		fprintf(stderr, "Could not open %s for reading certificate!\n", cert_path);
		exit(1);
	}
	int i = fread(cert, s->st_size, 1, file);
	if (i != 1) {
		fprintf(stderr, "Failed to read certificate from %s!\n", cert_path);
		exit(1);
	}
	fclose(file);
	xar_signature_add_x509certificate(sig, cert, s->st_size);
	free(s);
	free(cert);
}

static void inject_signature(const char *xar_path, const char *sig_path) {
	// since there is no API to insert a signature other than during signingCallback time, we have to
	// inject it by editing the raw file
	int buffer_size = 1024;
	void *buffer = malloc(buffer_size);
	FILE *sig, *xar;
	off_t signedDataOffset;
	xar_t x;
	int i;
	
	printf("inject_signature(%s, %s)",xar_path,sig_path);
	
	// open xar via the API first to determine the signature offset
	x = xar_open(xar_path, READ);
	if ( x == NULL ) {
		fprintf(stderr, "Could not open xar archive %s to get signature offset!\n", xar_path);
		exit(1);
	}
	signedDataOffset = get_sig_offset(x);
	xar_close(x);
	
	// then re-open xar and signature files raw...
	sig = fopen(sig_path, "r");
	if (!sig) {
		fprintf(stderr, "Could not open %s for reading signature!\n", sig_path);
		exit(1);
	}
	xar = fopen(xar_path, "r+");
	if (!xar) {
		fprintf(stderr, "Could not open xar archive %s for injecting signature!\n", xar_path);
		exit(1);
	}
	
	// ...and inject the signature
	fseek(xar, signedDataOffset, SEEK_SET);
	do {
		i = fread(buffer, 1, buffer_size, sig);
		if (ferror(sig)) {
			fprintf(stderr, "Failed to read signature from %s!\n", sig_path);
			exit(1);
		}
		fwrite(buffer, 1, i, xar);
	} while (!feof(sig));
	fclose(sig);
	fclose(xar);
	
	free(buffer);
}

static int archive(const char *filename, int arglen, char *args[]) {
	xar_t x;
	FTS *fts;
	FTSENT *ent;
	int flags;
	struct lnode *i;
	const char *default_compression;

	x = xar_open(filename, WRITE);
	if( !x ) {
		fprintf(stderr, "Error creating archive %s\n", filename);
		exit(1);
	}
	
	_set_xarsig_opts_from_args(x);
	
	default_compression = strdup(xar_opt_get(x, XAR_OPT_COMPRESSION));
	if( !default_compression )
		default_compression = strdup(XAR_OPT_VAL_GZIP);
	
	flags = FTS_PHYSICAL|FTS_NOSTAT|FTS_NOCHDIR;
	if( Local )
		flags |= FTS_XDEV;
	fts = fts_open(args, flags, NULL);
	if( !fts ) {
		fprintf(stderr, "Error traversing file tree\n");
		exit(1);
	}

	while( (ent = fts_read(fts)) ) {
		xar_file_t f;
		int exclude_match = 1;
		int nocompress_match = 1;
		if( ent->fts_info == FTS_DP )
			continue;

		if( strcmp(ent->fts_path, "/") == 0 )
			continue;
		if( strcmp(ent->fts_path, ".") == 0 )
			continue;
		
		for( i = Exclude; i; i=i->next ) {
			exclude_match = regexec(&i->reg, ent->fts_path, 0, NULL, 0);
			if( !exclude_match )
				break;
		}
		if( !exclude_match ) {
			if( Verbose )
				printf("Excluding %s\n", ent->fts_path);
			continue;
		}

		for( i = NoCompress; i; i=i->next ) {
			nocompress_match = regexec(&i->reg, ent->fts_path, 0, NULL, 0);
			if( !nocompress_match ) {
				xar_opt_set(x, XAR_OPT_COMPRESSION, XAR_OPT_VAL_NONE);
				break;
			}
		}
		f = xar_add(x, ent->fts_path);
		if( !f ) {
			fprintf(stderr, "Error adding file %s\n", ent->fts_path);
		} else {
			print_file(f);
		}
		if( !nocompress_match )
			xar_opt_set(x, XAR_OPT_COMPRESSION, default_compression);
	}
	fts_close(fts);
	if( xar_close(x) != 0 ) {
		fprintf(stderr, "Error creating the archive\n");
		if( !Err )
			Err = 42;
	}

	free((char *)default_compression);
	for( i = Exclude; i; ) {
		struct lnode *tmp;
		regfree(&i->reg);
		tmp = i;
		i = i->next;
		free(tmp);
	}
	for( i = NoCompress; i; ) {
		struct lnode *tmp;
		regfree(&i->reg);
		tmp = i;
		i = i->next;
		free(tmp);
	}

	if ( SigOffsetDumpPath ) {
		x = xar_open(filename, READ);
		if( !x ) {
			fprintf(stderr, "Error re-opening archive %s\n", filename);
			exit(1);
		}
		if (extract_sig_offset(x, SigOffsetDumpPath)) 
			exit(1);
	}
	
	return Err;
}

static int extract(const char *filename, int arglen, char *args[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	int files_extracted = 0;

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	xar_register_errhandler(x, err_callback, NULL);

	if( Perms == SYMBOLIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_SYMBOLIC);
	}
	if( Perms == NUMERIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_NUMERIC);
	}
	if ( Rsize != NULL ) {
		xar_opt_set(x, XAR_OPT_RSIZE, Rsize);
	}
	
	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating xar iterator\n");
		exit(1);
	}

	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		int matched = 0;
		int exclude_match = 1;
		struct lnode *i;

		char *path = xar_get_path(f);

		if( args[0] ) {
			int i;
			for(i = 0; args[i]; i++) {
				if( strcmp(path, args[i]) == 0 ) {
					matched = 1;
					break;
				}
			}
		} else {
			matched = 1;
		}

		for( i = Exclude; i; i=i->next ) {
			exclude_match = regexec(&i->reg, path, 0, NULL, 0);
			if( !exclude_match )
				break;
		}
		if( !exclude_match ) {
			if( Verbose )
				printf("Excluding %s\n", path);
			free(path);
			continue;
		}
		
		if( matched ) {
			files_extracted++;
			print_file(f);
			xar_extract(x, f);
		}
		free(path);
	}
	if( args[0] && (files_extracted == 0) ) {
		fprintf(stderr, "No files matched extraction criteria.\n");
		Err = 3;
	}

	if( Subdoc )
		extract_subdoc(x, NULL);

	xar_iter_free(i);
	if( xar_close(x) != 0 ) {
		fprintf(stderr, "Error extracting the archive\n");
		if( !Err )
			Err = 42;
	}
	return Err;
}

static int list_subdocs(const char *filename) {
	xar_t x;
	xar_subdoc_t s;

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	for(s = xar_subdoc_first(x); s; s = xar_subdoc_next(s)) {
		printf("%s\n", xar_subdoc_name(s));
	}
	xar_close(x);

	return Err;
}

static int list(const char *filename, int arglen, char *args[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	i = xar_iter_new();
	if( !i ) {
		fprintf(stderr, "Error creating xar iterator\n");
		exit(1);
	}

	for(f = xar_file_first(x, i); f; f = xar_file_next(i)) {
		print_file(f);
	}

	xar_iter_free(i);
	xar_close(x);

	return Err;
}

static int dumptoc(const char *filename, const char* tocfile) {
	xar_t x;
	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	xar_serialize(x, tocfile);
	xar_close(x);
	return Err;
}

static int dump_header(const char *filename) {
	int fd;
	xar_header_t xh;

	if(filename == NULL)
		fd = 0;
	else {
		fd = open(filename, O_RDONLY);
		if( fd < 0 ) {
			perror("open");
			exit(1);
		}
	}

	if( read(fd, &xh, sizeof(xh)) < sizeof(xh) ) {
		fprintf(stderr, "error reading header\n");
		exit(1);
	}

	printf("magic:                  0x%x ", ntohl(xh.magic));
	if( ntohl(xh.magic) != XAR_HEADER_MAGIC )
		printf("(BAD)\n");
	else
		printf("(OK)\n");
	printf("size:                   %d\n", ntohs(xh.size));
	printf("version:                %d\n", ntohs(xh.version));
	printf("Compressed TOC length:  %" PRId64 "\n", xar_ntoh64(xh.toc_length_compressed));
	printf("Uncompressed TOC length: %" PRId64 "\n", xar_ntoh64(xh.toc_length_uncompressed));
	printf("Checksum algorithm:     %d ", ntohl(xh.cksum_alg));
	switch( ntohl(xh.cksum_alg) ) {
	case XAR_CKSUM_NONE: printf("(unsupported (none))\n");
	                     break;
	case XAR_CKSUM_SHA1: printf("(SHA1)\n");
	                     break;
	case XAR_CKSUM_SHA256: printf("(SHA256)\n");
	                     break;
	case XAR_CKSUM_SHA512: printf("(SHA512)\n");
	                     break;
#ifdef XAR_SUPOPRT_MD5
	case XAR_CKSUM_MD5: printf("(MD5)\n");
	                     break;
#else
	case XAR_CKSUM_MD5: printf("(unsupported (MD5))\n");
	                     break;
#endif // XAR_SUPPORT_MD5
	default: printf("(unknown)\n");
	         break;
	};

	return 0;
}

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	xar_file_t f;
	const char *str;
	int e;

	f = xar_err_get_file(ctx);
	str = xar_err_get_string(ctx);
	e = xar_err_get_errno(ctx);

	switch(sev) {
	case XAR_SEVERITY_DEBUG:
	case XAR_SEVERITY_INFO:
		break;
	case XAR_SEVERITY_WARNING:
		printf("%s\n", str);
		break;
	case XAR_SEVERITY_NORMAL:
		if( (err = XAR_ERR_ARCHIVE_CREATION) && f )
    			print_file(f);
		break;
	case XAR_SEVERITY_NONFATAL:
	case XAR_SEVERITY_FATAL:
		Err = 2;
		printf("Error while ");
		if( err == XAR_ERR_ARCHIVE_CREATION ) printf("creating");
		if( err == XAR_ERR_ARCHIVE_EXTRACTION ) printf("extracting");
		printf(" archive");
		if( f ) {
			const char *file = xar_get_path(f);
			if( file ) printf(":(%s)", file);
			free((char *)file);
		}
		if( str ) printf(": %s", str);
		if( err ) printf(" (%s)", strerror(e));
		if( sev == XAR_SEVERITY_NONFATAL ) {
			printf(" - ignored");
			printf("\n");
		} else {
			printf("\n");
			exit(1);
		}
		break;
	}
	return 0;
}

static void usage(const char *prog) {
	fprintf(stderr, "Usage: %s -[ctx][v] -f <archive> ...\n", prog);
	fprintf(stderr, "\t-c               Creates an archive\n");
	fprintf(stderr, "\t-x               Extracts an archive\n");
	fprintf(stderr, "\t-t               Lists an archive\n");
	fprintf(stderr, "\t--sign           Creates a placeholder signature and saves\n");
	fprintf(stderr, "\t                 the data to sign to disk. Works with -c or -f, requires\n");
	fprintf(stderr, "\t                 --sig-size, --leaf-cert-loc and \n");
	fprintf(stderr, "\t                 --intermediate-cert-loc to be set. Setting\n");
	fprintf(stderr, "\t                 --data-to-sign and --sig-offset is optional.\n");
	fprintf(stderr, "\t                 Fails with error code %i if the archive has already\n", E_SIGEXISTS);
	fprintf(stderr, "\t                 been signed.\n");
	fprintf(stderr, "\t--replace-sign   Rips out existing signature(s) and makes a new one.\n");
	fprintf(stderr, "\t                 Same required parameter set as --sign, \n");
	fprintf(stderr, "\t                 but -f instead of -c.\n");
	fprintf(stderr, "\t--extract-data-to-sign Extracts data to be signed from an\n");
	fprintf(stderr, "\t                 existing archive. Requires --data-to-sign\n");
	fprintf(stderr, "\t                 and --sig-offset (and -f) to be set.\n");
	fprintf(stderr, "\t--extract-certs <dir> Extracts all certificates into the\n");
	fprintf(stderr, "\t                 specified directory, naming them 'cert1', 'cert2' etc.\n");
	fprintf(stderr, "\t                 Requires -f.\n");
	fprintf(stderr, "\t--inject-sig <filename>     After extracting the data to be signed and\n");
	fprintf(stderr, "\t                 doing the signing externally, injects.\n");
	fprintf(stderr, "\t                 the signature. Requires -f\n");
	fprintf(stderr, "\t-f <filename>    Specifies an archive to operate on [REQUIRED!]\n");
	fprintf(stderr, "\t-v               Print filenames as they are archived\n");
	fprintf(stderr, "\t-n name          Provides a name for a subdocument\n");
	fprintf(stderr, "\t-s <filename>    On extract, specifies the file to extract\n");
	fprintf(stderr, "\t                      subdocuments to.\n");
	fprintf(stderr, "\t                 On archival, specifies an xml file to add\n");
	fprintf(stderr, "\t                      as a subdocument.\n");
	fprintf(stderr, "\t-l               On archival, stay on the local device.\n");
	fprintf(stderr, "\t-p               On extract, set ownership based on symbolic\n");
	fprintf(stderr, "\t                      names, if possible.\n");
	fprintf(stderr, "\t-P               On extract, set ownership based on uid/gid.\n");
	fprintf(stderr, "\t--toc-cksum      Specifies the hashing algorithm to use for\n");
	fprintf(stderr, "\t                      xml header verification.\n");
#ifdef XAR_SUPPORT_MD5
	fprintf(stderr, "\t                      Valid values: none, md5, sha1, sha256, and sha512\n");
#else
	fprintf(stderr, "\t                      Valid values: none, sha1, sha256, and sha512\n");
#endif // XAR_SUPPORT_MD5
	fprintf(stderr, "\t                      Default: sha1\n");
	fprintf(stderr, "\t--file-cksum     Specifies the hashing algorithm to use for\n");
	fprintf(stderr, "\t                      xml header verification.\n");
#ifdef XAR_SUPPORT_MD5
	fprintf(stderr, "\t                      Valid values: none, md5, sha1, sha256, and sha512\n");
#else
	fprintf(stderr, "\t                      Valid values: none, sha1, sha256, and sha512\n");
#endif // XAR_SUPPORT_MD5
	fprintf(stderr, "\t                      Default: sha1\n");
	fprintf(stderr, "\t--dump-toc=<filename> Has xar dump the xml header into the\n");
	fprintf(stderr, "\t                      specified file.\n");
	fprintf(stderr, "\t--dump-header    Prints out the xar binary header information\n");
	fprintf(stderr, "\t--compression    Specifies the compression type to use.\n");
	fprintf(stderr, "\t                      Valid values: none, gzip, bzip2\n");
	fprintf(stderr, "\t                      Default: gzip\n");
	fprintf(stderr, "\t--list-subdocs   List the subdocuments in the xml header\n");
	fprintf(stderr, "\t--extract-subdoc=name Extracts the specified subdocument\n");
	fprintf(stderr, "\t                      to a document in cwd named <name>.xml\n");
	fprintf(stderr, "\t--exclude        POSIX regular expression of files to \n");
	fprintf(stderr, "\t                      ignore while archiving.\n");
	fprintf(stderr, "\t--rsize          Specifies the size of the buffer used\n");
	fprintf(stderr, "\t                      for read IO operations in bytes.\n");
	fprintf(stderr, "\t--coalesce-heap  When archived files are identical, only store one copy\n");
	fprintf(stderr, "\t                      This option creates an archive which\n");
	fprintf(stderr, "\t                      is not streamable\n");
	fprintf(stderr, "\t--link-same      Hardlink identical files\n");
	fprintf(stderr, "\t--no-compress    POSIX regular expression of files\n");
	fprintf(stderr, "\t                      not to archive, but not compress.\n");
	fprintf(stderr, "\t--sig-size n     Size (in bytes) of the signature placeholder\n");
	fprintf(stderr, "\t                      to generate.\n");
	fprintf(stderr, "\t--data-to-sign=file   Path where to dump the data to be signed.\n");
	fprintf(stderr, "\t--sig-offset=file     Path where to dump the signature's offset\n");
	fprintf(stderr, "\t                      within the xar.\n");
	fprintf(stderr, "\t--leaf-cert-loc=file         Location of the leaf certificate\n");
	fprintf(stderr, "\t--intermediate-cert-loc=file Location of the intermediate certificate\n");
	fprintf(stderr, "\t--root-cert-loc=file         Location of the root certificate\n");
	fprintf(stderr, "\t--version        Print xar's version number\n");

	return;
}

static void print_version() {
	printf("xar %s\n", XAR_VERSION);
}

int main(int argc, char *argv[]) {
	char *filename = NULL;
	char *sig_path = NULL;
	char *cert_path = NULL;
	char command = 0, c;
	char **args;
	const char *tocfile = NULL;
	int arglen, i, err;
	xar_t x;
	int loptind = 0;
	int required_dash_f = 0;  /* This release requires us to use -f */
	struct lnode *tmp;
	long int longtmp;
	struct stat stat_struct;
	struct option o[] = { 
		{"toc-cksum", 1, 0, 1},
		{"file-cksum", 1, 0, 22}, // note out-of-order argument number
		{"dump-toc", 1, 0, 'd'},
		{"compression", 1, 0, 2},
		{"list-subdocs", 0, 0, 3},
		{"help", 0, 0, 'h'},
		{"version", 0, 0, 4},
		{"dump-header", 0, 0, 5},
		{"extract-subdoc", 1, 0, 6},
		{"exclude", 1, 0, 7},
		{"rsize", 1, 0, 8},
		{"coalesce-heap", 0, 0, 9},
		{"link-same", 0, 0, 10},
		{"no-compress", 1, 0, 11},
		{"sig-size", 1, 0, 12},
		{"data-to-sign", 1, 0, 13},
		{"sig-offset", 1, 0, 14},
		{"leaf-cert-loc", 1, 0, 15},
		{"intermediate-cert-loc", 1, 0, 16},
		{"extract-data-to-sign", 0, 0, 17},
		{"sign", 0, 0, 18},
		{"replace-sign", 0, 0, 19},
		{"inject-sig", 1, 0, 20},
		{"extract-certs", 1, 0, 21},
		{"root-cert-loc", 1, 0, 23},
		{ 0, 0, 0, 0}
	};

	if( argc < 2 ) {
		usage(argv[0]);
		exit(1);
	}

	while( (c = getopt_long(argc, argv, "xcvtf:hpPln:s:d:v", o, &loptind)) != -1 ) {
		switch(c) {
		case  1 : if( !optarg ) {
		          	usage(argv[0]);
		          	exit(1);
		          }
		          if( (strcmp(optarg, XAR_OPT_VAL_NONE) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_SHA1) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_SHA256) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_SHA512) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_MD5)  != 0) ) {
		          	usage(argv[0]);
		          	exit(1);
		          }
		          Toccksum = optarg;
		          break;
		case  22: if( !optarg ) {
		            usage(argv[0]);
		            exit(1);
				  }
		          if( (strcmp(optarg, XAR_OPT_VAL_NONE) != 0) &&
		            (strcmp(optarg, XAR_OPT_VAL_SHA1) != 0) &&
		            (strcmp(optarg, XAR_OPT_VAL_SHA256) != 0) &&
		            (strcmp(optarg, XAR_OPT_VAL_SHA512) != 0) &&
		            (strcmp(optarg, XAR_OPT_VAL_MD5)  != 0) ) {
				  usage(argv[0]);
				  exit(1);
				  }
				  Filecksum = optarg;
				  break;
		case  2 : if( !optarg ) {
		          	usage(argv[0]);
		          	exit(1);
		          }
		          if( (strcmp(optarg, XAR_OPT_VAL_NONE) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_GZIP) != 0) &&
		              (strcmp(optarg, XAR_OPT_VAL_BZIP) != 0) ) {
		          	usage(argv[0]);
		          	exit(1);
		          }
		          Compression = optarg;
		          break;
		case  3 : if( command && (command != 3) ) {
		          	fprintf(stderr, "Conflicting commands specified\n");
				exit(1);
		          }
			  command = 3;
			  break;
		case  4 : print_version();
		          exit(0);
		case 'd':
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			tocfile = optarg;
			command = 'd';
			break;
		case  5 : command = 5;
		          break;
		case  6 :
			SubdocName = optarg;
			asprintf(&Subdoc, "%s.xml", SubdocName);
			if( !command )
				command = 6;
			break;
		case  7 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
			if( err ) {
				char errstr[1024];
				regerror(err, &tmp->reg, errstr, sizeof(errstr));
				printf("Error with regular expression %s: %s\n", tmp->str, errstr);
				exit(1);
			}
			if( Exclude == NULL ) {
				Exclude = tmp;
				Exclude_Tail = tmp;
			} else {
				Exclude_Tail->next = tmp;
				Exclude_Tail = tmp;
			}
			break;
		case  8 :
			if ( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			longtmp = strtol(optarg, NULL, 10);
			if( (((longtmp == LONG_MIN) || (longtmp == LONG_MAX)) && (errno == ERANGE)) || (longtmp < 1) ) {
				fprintf(stderr, "Invalid rsize value: %s\n", optarg);
				exit(5);
			}
			Rsize = optarg;
			break;
		case  9 : Coalesce = 1; break;
		case 10 : LinkSame = 1; break;
		case 11 :
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
			if( err ) {
				char errstr[1024];
				regerror(err, &tmp->reg, errstr, sizeof(errstr));
				printf("Error with regular expression %s: %s\n", tmp->str, errstr);
				exit(1);
			}
			if( NoCompress == NULL ) {
				NoCompress = tmp;
				NoCompress_Tail = tmp;
			} else {
				NoCompress_Tail->next = tmp;
				NoCompress_Tail = tmp;
			}
			break;
		case  12 : 
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			SigSize = strtol(optarg, (char **)NULL, 10);
			break;
		case  13 : 
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			DataToSignDumpPath = optarg;
			break;
		case  14 : 
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			SigOffsetDumpPath = optarg;
			break;
		case  15 : 
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			LeafCertPath = optarg;
			break;
		case  16 : 
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			IntermediateCertPath = optarg;
			break;
		case 17 :	// extract-data-to-sign
			command = 'e';
			break;
		case 18 :	// sign
			DoSign = 1;
			break;
		case 19 :	// replace-sign
			command = 'r';
			break;
		case 20 :	// inject signature
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			sig_path = optarg;
			command = 'i';
			break;
		case 21 :	// extract-certs
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			cert_path = optarg;
			stat(cert_path, &stat_struct);
			if (!(stat_struct.st_mode & S_IFDIR)) {
				usage(argv[0]);
				fprintf(stderr, "%s is not a directory.\n", cert_path);
				exit(1);
			}
			command = 'j';
			break;
		case 23 :
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			RootCertPath = optarg;
			break;
		case 'c':
		case 'x':
		case 't':
			if( command && (command != 's') ) {
				usage(argv[0]);
				fprintf(stderr, "Conflicting command flags: %c and %c specified\n", c, command);
				exit(1);
			}
			if( c == 't' )
				Verbose++;
			command = c;
			break;
		case 'f':
			required_dash_f = 1;
			filename = optarg;
			break;
		case 'p':
			Perms = SYMBOLIC;
			break;
		case 'P':
			Perms = NUMERIC;
			break;
		case 'l':
			Local = 1;
			break;
		case 'n':
			SubdocName = optarg;
			break;
		case 's':
			Subdoc = optarg;
			if( !command )
				command = 's';
			break;
		case 'v':
			Verbose++;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(1);
		}
	}

	if (! required_dash_f)	{
		usage(argv[0]);
		fprintf(stderr, "\n -f option is REQUIRED\n");
		exit(1);
	}

	// extract-data-to-sign
	if ( (command == 'e') && ((!filename) || (!DataToSignDumpPath) || (!SigOffsetDumpPath)) ) {
		usage(argv[0]);
		exit(1);
	}
	
	if ( DoSign ) { // XXX: should we require RootCertPath as well?
		if (  ( !SigSize || !LeafCertPath || !IntermediateCertPath ) 
			  || ((command != 'c') && (!filename)) ) {
			usage(argv[0]);
			exit(1);
		}
		if (!command)
			command = 'n';
	}

	if (command == 'r') {
		/*if ( !SigSize || !LeafCertPath || !IntermediateCertPath || !filename) {
			usage(argv[0]);
			exit(1);
		}
		xar_t x = xar_open(filename, READ);
		if ( x == NULL ) {
			fprintf(stderr, "Could not open archive %s!\n", filename);
			exit(1);
		}
		xar_signature_t sig = xar_signature_first(x);
		if ( !sig ) {
			fprintf(stderr, "No signature found to replace.\n");
			exit(E_NOSIG);
		}
		xar_close(x);*/
	}
	
	if ((command == 'i') && ((!filename) || (!sig_path))) {
		usage(argv[0]);
		exit(1);
	}

	if ((command == 'j') && (!filename)) {
		usage(argv[0]);
		exit(1);
	}
	
	switch(command) {
		case  5 : 
		        return dump_header(filename);
		case  3 : 
			return list_subdocs(filename);
		case 'c':
			if( optind == argc ) {
				usage(argv[0]);
				fprintf(stderr, "No files to operate on.\n");
				exit(1);
			}
			arglen = argc - optind;
			args = malloc(sizeof(char*) * (arglen+1));
			memset(args, 0, sizeof(char*) * (arglen+1));
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);

			return archive(filename, arglen, args);
		case 'd':
			if( !tocfile ) {
				usage(argv[0]);
				exit(1);
			}
			return dumptoc(filename, tocfile);
		case 'x':
			arglen = argc - optind;
			args = malloc(sizeof(char*) * (arglen+1));
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);
			args[i] = NULL;
			return extract(filename, arglen, args);
		case 't':
			arglen = argc - optind;
			args = malloc(sizeof(char*) * (arglen+1));
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);
			return list(filename, arglen, args);
		case  6 :
		case 's':
			x = xar_open(filename, READ);
			if( !x ) {
				fprintf(stderr, "Error opening xar archive: %s\n", filename);
				exit(1);
			}
			xar_register_errhandler(x, err_callback, NULL);
			extract_subdoc(x, SubdocName);
			xar_close(x);
			exit(Err);
			break;
		case 'e':
			extract_data_to_sign(filename);
			exit(Err);
		case 'r':
			replace_sign(filename);
			exit(Err);
		case 'i':
			inject_signature(filename, sig_path);
			exit(Err);
		case 'n':
			belated_sign(filename);
			exit(Err);
		case 'j':
			extract_certs(filename, cert_path);
			exit(Err);
		default:
			usage(argv[0]);
			fprintf(stderr, "Unrecognized command.\n");
			exit(1);
	}

	/* unreached */
	exit(0);
}
