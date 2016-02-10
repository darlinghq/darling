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
 * DRI: Rob Braun <bbraun@synack.net>
 */

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
#include <time.h>
#include "xar.h"
#include "config.h"

#define SYMBOLIC 1
#define NUMERIC  2
static int Perms = 0;
static int Local = 0;
static char *Subdoc = NULL;
static char *SubdocName = NULL;
static char *Toccksum = NULL;
static char *Filecksum = NULL;
static char *Compression = NULL;
static char *Rsize = NULL;
static char *CompressionArg = NULL;
static char *Chdir = NULL;

static int Err = 0;
static int List = 0;
static int Verbose = 0;
static int Coalesce = 0;
static int LinkSame = 0;
static int NoOverwrite = 0;
static int SaveSuid = 0;

struct lnode {
	char *str;
	regex_t reg;
	struct lnode *next;
};

struct lnode *Exclude = NULL;
struct lnode *Exclude_Tail = NULL;
struct lnode *NoCompress = NULL;
struct lnode *NoCompress_Tail = NULL;
struct lnode *PropInclude = NULL;
struct lnode *PropInclude_Tail = NULL;
struct lnode *PropExclude = NULL;
struct lnode *PropExclude_Tail = NULL;

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx);

static void print_file(xar_t x, xar_file_t f) {
	if( List && Verbose ) {
		char *size = xar_get_size(x, f);
		char *path = xar_get_path(f);
		char *type = xar_get_type(x, f);
		char *mode = xar_get_mode(x, f);
		char *user = xar_get_owner(x, f);
		char *group = xar_get_group(x, f);
		char *mtime = xar_get_mtime(x, f);
		printf("%s %8s/%-8s %10s %s %s\n", mode, user, group, size, mtime, path);
		free(size);
		free(type);
		free(path);
		free(mode);
		free(user);
		free(group);
		free(mtime);
	} else if( List || Verbose ) {
		char *path = xar_get_path(f);
		
		if (xar_path_issane(path) == 0) 
			printf("Warning, archive contains invalid path: %s\n", path);
		else 
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

	fd = open(Subdoc, O_RDONLY);
	if( fd < 0 ) {
		fprintf(stderr, "ERROR: subdoc file %s doesn't exist.  Ignoring.\n", Subdoc);
		return;
	}
	s = xar_subdoc_new(x, (const char *)SubdocName);
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

	if( Toccksum )
		xar_opt_set(x, XAR_OPT_TOCCKSUM, Toccksum);
	
	if( Filecksum )
		xar_opt_set(x, XAR_OPT_FILECKSUM, Filecksum);

	if( CompressionArg )
		xar_opt_set(x, XAR_OPT_COMPRESSIONARG, CompressionArg);
	
	if( Compression )
		xar_opt_set(x, XAR_OPT_COMPRESSION, Compression);

	if( Coalesce )
		xar_opt_set(x, XAR_OPT_COALESCE, "true");

	if( LinkSame )
		xar_opt_set(x, XAR_OPT_LINKSAME, "true");

	if ( Rsize != NULL )
		xar_opt_set(x, XAR_OPT_RSIZE, Rsize);

	xar_register_errhandler(x, err_callback, NULL);

	for( i = PropInclude; i; i=i->next ) {
		xar_opt_set(x, XAR_OPT_PROPINCLUDE, i->str);
	}
	for( i = PropExclude; i; i=i->next ) {
		xar_opt_set(x, XAR_OPT_PROPEXCLUDE, i->str);
	}

	if( Subdoc )
		add_subdoc(x);

	if( Perms == SYMBOLIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_SYMBOLIC);
	}
	if( Perms == NUMERIC ) {
		xar_opt_set(x, XAR_OPT_OWNERSHIP, XAR_OPT_VAL_NUMERIC);
	}

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
			print_file(x, f);
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

	return Err;
}

static int extract(const char *filename, int arglen, char *args[]) {
	xar_t x;
	xar_iter_t i;
	xar_file_t f;
	int files_extracted = 0;
	int argi;
	struct lnode *extract_files = NULL;
	struct lnode *extract_tail = NULL;
	struct lnode *lnodei = NULL;
	struct lnode *dirs = NULL;

	for(argi = 0; args[argi]; argi++) {
		struct lnode *tmp;
		int err;
		tmp = malloc(sizeof(struct lnode));
		tmp->str = strdup(args[argi]);
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			printf("Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( extract_files == NULL ) {
			extract_files = tmp;
			extract_tail = tmp;
		} else {
			extract_tail->next = tmp;
			extract_tail = tmp;
		}
		
		/* Add a clause for recursive extraction */
		tmp = malloc(sizeof(struct lnode));
		asprintf(&tmp->str, "%s/.*", args[argi]);
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			printf("Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( extract_files == NULL ) {
			extract_files = tmp;
			extract_tail = tmp;
		} else {
			extract_tail->next = tmp;
			extract_tail = tmp;
		}
	}

	x = xar_open(filename, READ);
	if( !x ) {
		fprintf(stderr, "Error opening xar archive: %s\n", filename);
		exit(1);
	}

	if(Chdir) {
		if( chdir(Chdir) != 0 ) {
			fprintf(stderr, "Unable to chdir to %s\n", Chdir);
			exit(1);
		}
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
	if( SaveSuid ) {
		xar_opt_set(x, XAR_OPT_SAVESUID, XAR_OPT_VAL_TRUE);
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
			for(i = extract_files; i != NULL; i = i->next) {
				int extract_match = 1;

				extract_match = regexec(&i->reg, path, 0, NULL, 0);
				if( !extract_match ) {
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
		
		if (!xar_path_issane(path)) {
			if (Verbose)
				printf("Warning, not extracting file \"%s\" because it's path is invalid.\n", path);
			free(path);
			continue;
		}
		
		if( matched ) {
			struct stat sb;
			if( NoOverwrite && (lstat(path, &sb) == 0) ) {
				printf("%s already exists, not overwriting\n", path);
			} else {
				const char *prop = NULL;
				int deferred = 0;
				if( xar_prop_get(f, "type", &prop) == 0 ) {
					if( strcmp(prop, "directory") == 0 ) {
						struct lnode *tmpl = calloc(sizeof(struct lnode),1);
						tmpl->str = (char *)f;
						tmpl->next = dirs;
						dirs = tmpl;
						deferred = 1;
					}
				}
				if( ! deferred ) {
					files_extracted++;
					print_file(x, f);
					xar_extract(x, f);
				}
			}
		}
		free(path);
	}
	for(lnodei = dirs; lnodei; lnodei = lnodei->next) {
		files_extracted++;
		print_file(x,(xar_file_t)lnodei->str);
		xar_extract(x, (xar_file_t)lnodei->str);
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

	for(lnodei = extract_files; lnodei != NULL; ) {
		struct lnode *tmp;
		free(lnodei->str);
		regfree(&lnodei->reg);
		tmp = lnodei;
		lnodei = lnodei->next;
		free(tmp);
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
	int argi = 0;
	struct lnode *list_files = NULL;
	struct lnode *list_tail = NULL;
	struct lnode *lnodei = NULL;

	for(argi = 0; args[argi]; argi++) {
		struct lnode *tmp;
		int err;
		tmp = malloc(sizeof(struct lnode));
		tmp->str = strdup(args[argi]);
		tmp->next = NULL;
		err = regcomp(&tmp->reg, tmp->str, REG_NOSUB);
		if( err ) {
			char errstr[1024];
			regerror(err, &tmp->reg, errstr, sizeof(errstr));
			printf("Error with regular expression %s: %s\n", tmp->str, errstr);
			exit(1);
		}
		if( list_files == NULL ) {
			list_files = tmp;
			list_tail = tmp;
		} else {
			list_tail->next = tmp;
			list_tail = tmp;
		}
	}

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
		int matched = 0;

		if( args[0] ) {
			char *path = xar_get_path(f);
			
			if (xar_path_issane(path) == 0) {
				fprintf(stderr, "Warning, archive contains invalid path: %s\n", path);
				free(path);
				continue;
			}

			for(lnodei = list_files; lnodei != NULL; lnodei = lnodei->next) {
				int list_match = 1;

				list_match = regexec(&lnodei->reg, path, 0, NULL, 0);
				if( !list_match ) {
					matched = 1;
					break;
				}
			}
			free(path);
		} else {
			matched = 1;
		}

		if( matched )
			print_file(x, f);
	}

	xar_iter_free(i);
	xar_close(x);

	for(lnodei = list_files; lnodei != NULL; ) {
		struct lnode *tmp;
		free(lnodei->str);
		regfree(&lnodei->reg);
		tmp = lnodei;
		lnodei = lnodei->next;
		free(tmp);
	}

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
	case XAR_CKSUM_NONE: printf("(none)\n");
	                     break;
	case XAR_CKSUM_SHA1: printf("(SHA1)\n");
	                     break;
	case XAR_CKSUM_SHA256: printf("(SHA256)\n");
		break;
	case XAR_CKSUM_SHA512: printf("(SHA512)\n");
		break;
	case XAR_CKSUM_MD5: printf("(MD5)\n");
	                    break;
	default: printf("(unknown)\n");
	         break;
	};

	return 0;
}

static int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx) {
	xar_file_t f;
	xar_t x;
	const char *str;
	int e;

	x = xar_err_get_archive(ctx);
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
			print_file(x, f);
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
		if( err && e ) printf(" (%s)", strerror(e));
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
	fprintf(stderr, "\t-f <filename>    Specifies an archive to operate on [REQUIRED!]\n");
	fprintf(stderr, "\t-v               Print filenames as they are archived\n");
	fprintf(stderr, "\t-C <path>        On extract, chdir to this location\n");
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
	fprintf(stderr, "\t                      Valid values: none, md5, sha1, sha256, and sha512\n");
	fprintf(stderr, "\t                      Default: sha1\n");
	fprintf(stderr, "\t--file-cksum     Specifies the hashing algorithm to use for\n");
	fprintf(stderr, "\t                      file content verification.\n");
	fprintf(stderr, "\t                      Valid values: none, md5, sha1, sha256, and sha512\n");
	fprintf(stderr, "\t                      Default: sha1\n");
	fprintf(stderr, "\t--dump-toc=<filename> Has xar dump the xml header into the\n");
	fprintf(stderr, "\t                      specified file.\n");
	fprintf(stderr, "\t--dump-header    Prints out the xar binary header information\n");
	fprintf(stderr, "\t--compression    Specifies the compression type to use.\n");
	fprintf(stderr, "\t                      Valid values: none, gzip, bzip2, lzma\n");
	fprintf(stderr, "\t                      Default: gzip\n");
	fprintf(stderr, "\t-a               Synonym for \"--compression=lzma\"\n");
	fprintf(stderr, "\t-j               Synonym for \"--compression=bzip2\"\n");
	fprintf(stderr, "\t-z               Synonym for \"--compression=gzip\"\n");
	fprintf(stderr, "\t--compression-args=arg Specifies arguments to be passed\n");
	fprintf(stderr, "\t                       to the compression engine.\n");
	fprintf(stderr, "\t--compress-heap  Compress entire heap instead of individual files.\n");
	fprintf(stderr, "\t                      Currently limited to gzip compression.\n");
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
	fprintf(stderr, "\t                      to archive, but not compress.\n");
	fprintf(stderr, "\t--prop-include   File properties to include in archive\n");
	fprintf(stderr, "\t--prop-exclude   File properties to exclude in archive\n");
	fprintf(stderr, "\t--distribution   Only includes a subset of file properties\n");
	fprintf(stderr, "\t                      appropriate for archive distribution\n");
	fprintf(stderr, "\t--keep-existing  Do not overwrite existing files while extracting\n");
	fprintf(stderr, "\t-k               Synonym for --keep-existing\n");
	fprintf(stderr, "\t--keep-setuid    Preserve the suid/sgid bits when extracting\n");
	fprintf(stderr, "\t--version        Print xar's version number\n");

	return;
}

static void print_version() {
	printf("xar %s\n", XAR_VERSION);
}

int main(int argc, char *argv[]) {
	int ret;
	char *filename = NULL;
	char command = 0, c;
	char **args;
	const char *tocfile = NULL;
	int arglen, i, err;
	xar_t x;
	int loptind = 0;
	int required_dash_f = 0;  /* This release requires us to use -f */
	struct lnode *tmp;
	long int longtmp;
	struct option o[] = { 
		{"toc-cksum",        required_argument, 0, 1},
		{"file-cksum",       required_argument, 0, 19}, // out of order to avoid regressions
		{"dump-toc",         required_argument, 0, 'd'},
		{"compression",      required_argument, 0, 2},
		{"list-subdocs",     no_argument,       0, 3},
		{"help",             no_argument,       0, 'h'},
		{"version",          no_argument,       0, 4},
		{"dump-header",      no_argument,       0, 5},
		{"extract-subdoc",   required_argument, 0, 6},
		{"exclude",          required_argument, 0, 7},
		{"rsize",            required_argument, 0, 8},
		{"coalesce-heap",    no_argument,       0, 9},
		{"link-same",        no_argument,       0, 10},
		{"no-compress",      required_argument, 0, 11},
		{"prop-include",     required_argument, 0, 12},
		{"prop-exclude",     required_argument, 0, 13},
		{"distribution",     no_argument,       0, 14},
		{"keep-existing",    no_argument,       0, 15},
		{"keep-setuid",      no_argument,       0, 16},
		{"compression-args", required_argument, 0, 17},
		{ 0, 0, 0, 0}
	};

	if( argc < 2 ) {
		usage(argv[0]);
		exit(1);
	}

	while( (c = getopt_long(argc, argv, "axcC:vtjzf:hpPln:s:d:vk", o, &loptind)) != -1 ) {
		switch(c) {
		case  1 : // toc-cksum
			if( !optarg ) {
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
		case 19 : // file-cksum
			if( !optarg ) {
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
		case  2 : // compression
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			if( (strcmp(optarg, XAR_OPT_VAL_NONE) != 0) &&
				(strcmp(optarg, XAR_OPT_VAL_GZIP) != 0) &&
				(strcmp(optarg, XAR_OPT_VAL_BZIP) != 0) &&
				(strcmp(optarg, XAR_OPT_VAL_LZMA) != 0) ) {
				usage(argv[0]);
				exit(1);
			}
			Compression = optarg;
			break;
		case  3 : // list-subdocs
			if( command && (command != 3) ) {
				fprintf(stderr, "Conflicting commands specified\n");
				exit(1);
			}
			command = 3;
			break;
		case  4 : // version
			print_version();
			exit(0);
		case 'd': // dump-toc
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			tocfile = optarg;
			command = 'd';
			break;
		case  5 : // dump-header
			command = 5;
			break;
		case  6 : // extract-subdoc
			SubdocName = optarg;
			asprintf(&Subdoc, "%s.xml", SubdocName);
			if( !command )
				command = 6;
			break;
		case  7 : // exclude
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
		case  8 : // rsize
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
		case  9 : Coalesce = 1; break; // coalesce-heap
		case 10 : LinkSame = 1; break; // link-same
		case 11 : // no-compress
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
		case 12 : // prop-include
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			if( PropInclude == NULL ) {
				PropInclude = tmp;
				PropInclude_Tail = tmp;
			} else {
				PropInclude_Tail->next = tmp;
				PropInclude_Tail = tmp;
			}
			break;
		case 13 : // prop-exclude
			tmp = malloc(sizeof(struct lnode));
			tmp->str = optarg;
			tmp->next = NULL;
			if( PropExclude == NULL ) {
				PropExclude = tmp;
				PropExclude_Tail = tmp;
			} else {
				PropExclude_Tail->next = tmp;
				PropExclude_Tail = tmp;
			}
			break;
		case 14 : // distribution
		{
			char *props[] = { "type", "data", "mode", "name" };
			int i;
			for( i = 0; i < 4; i++ ) {
				tmp = malloc(sizeof(struct lnode));
				tmp->str = strdup(props[i]);
				tmp->next = NULL;
				if( PropInclude == NULL ) {
					PropInclude = tmp;
					PropInclude_Tail = tmp;
				} else {
					PropInclude_Tail->next = tmp;
					PropInclude_Tail = tmp;
				}
			}
		}
			break;
		case 'k': // keep-existing
		case 15 :
			NoOverwrite++;
			break;
		case 16 : // keep-setuid
			SaveSuid++;
			break;
		case 17 : // compression-args
			CompressionArg = optarg;
			break;
		case 'C': // chdir to
			if( !optarg ) {
				usage(argv[0]);
				exit(1);
			}
			Chdir = optarg;
			break;
		case 'c': // create
		case 'x': // extract
		case 't': // list
			if( command && (command != 's') ) {
				usage(argv[0]);
				fprintf(stderr, "Conflicting command flags: %c and %c specified\n", c, command);
				exit(1);
			}
			if( c == 't' )
				List = 1;
			command = c;
			break;
		case 'a':
			Compression = "lzma";
			break;
		case 'j':
			Compression = "bzip2";
			break;
		case 'z':
			Compression = "gzip";
			break;
		case 'f': // filename
			required_dash_f = 1;
			filename = optarg;
			break;
		case 'p': // set ownership based on symbolic names (if possible)
			Perms = SYMBOLIC;
			break;
		case 'P': // set ownership based on uid/gid
			Perms = NUMERIC;
			break;
		case 'l': // stay on local device
			Local = 1;
			break;
		case 'n': // provide subdocument name
			SubdocName = optarg;
			break;
		case 's': // extract subdocuments to/add subdocuments from
			Subdoc = optarg;
			if( !command )
				command = 's';
			break;
		case 'v': // print filenames
			Verbose++;
			break;
		case 'h': // help
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
			args = calloc(sizeof(char*) * (arglen+1),1);
			for( i = 0; i < arglen; i++ )
				args[i] = strdup(argv[optind + i]);
			ret = list(filename, arglen, args);
			for( i = 0; i < arglen; i++ )
				free(args[i]);
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
		default:
			usage(argv[0]);
			fprintf(stderr, "Unrecognized command.\n");
			exit(1);
	}

	/* unreached */
	exit(0);
}
