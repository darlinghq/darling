/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#import <PackageKit/PackageKit.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread/private.h>

// This class is (will) be implemented to allow applications such as
// https://github.com/danzimm/xip_extract/blob/master/xip_extract.m
// to work.

static int open_cb(struct archive *a, void *_client_data)
{
    return ARCHIVE_OK;
}

static int close_cb(struct archive *a, void *_client_data)
{
    return ARCHIVE_OK;
}

struct ReadContext
{
    xar_stream* xs;
    uint64_t blockLength;

	char inbuf[4096];
	char outbuf[4096 * 1024];
};

static ssize_t read_cb(struct archive *a, void *_client_data, const void **_buffer)
{
    struct ReadContext* rc = (struct ReadContext*) _client_data;
    
    if (!rc->blockLength)
    {
        // TODO
    }
}

static NSError* makeError(NSURL* url, NSString* errorString)
{
    NSDictionary* userInfo = @{
        NSLocalizedDescriptionKey: errorString,
        NSFilePathErrorKey: [url absoluteString]
    };
    return [[NSError alloc] initWithDomain:@"PackageKit"
                            code: 0
                            userInfo: userInfo];
}

static inline BOOL xar_read(char *buffer, uint32_t size, xar_stream *stream) {
    stream->next_out = buffer;
    stream->avail_out = size;
    while (stream->avail_out)
    {
        if (xar_extract_tostream(stream) != XAR_STREAM_OK)
            return NO;
    }
    return YES;
}

static inline uint64_t xar_read_64(xar_stream *stream) {
    char t[8];
    xar_read(t, 8, stream);
    return __builtin_bswap64(*(uint64_t *)t);
}

static int copy_data(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	off_t offset;

	for (;;)
    {
		r = archive_read_data_block(ar, &buff, &size, &offset);
		if (r == ARCHIVE_EOF)
			return (ARCHIVE_OK);
		if (r != ARCHIVE_OK)
			return (r);
		r = archive_write_data_block(aw, buff, size, offset);
		if (r != ARCHIVE_OK)
        {
            NSLog(@"archive_write_data_block error\n");
			return (r);
		}
	}
}

@implementation PKSignedContainer

- (instancetype)initForReadingFromContainerAtURL:(NSURL *)url
                                                    error:(NSError **)error
{
    const char* path = [url fileSystemRepresentation];

    _xar = xar_open(path, READ);

    if (!_xar)
    {
        *error = makeError(url, @"Cannot open input XIP (XAR error)");
        return nil;
    }

    xar_iter_t i = xar_iter_new();
    _xar_file = xar_file_first(_xar, i);
    char *xarPath;

    while (strncmp((xarPath = xar_get_path(_xar_file)), "Content", 7) && (_xar_file = xar_file_next(i)))
        free(xarPath);

    free(xarPath);
    xar_iter_free(i);

    if (!_xar_file)
    {
        *error = makeError(url, @"Not a XIP, 'Content' not found");
        return nil;
    }

    if (xar_verify(_xar, _xar_file) != XAR_STREAM_OK)
    {
        *error = makeError(url, @"xar_verify failed");
        return nil;
    }

    if (xar_extract_tostream_init(_xar, _xar_file, &_xar_stream) != XAR_STREAM_OK)
    {
        *error = makeError(url, @"XAR init failed");
        return nil;
    }

    char header[4];
    xar_read(header, sizeof(header), &_xar_stream);

    if (strncmp(header, "pbzx", 4) != 0)
    {
        *error = makeError(url, @"Not a pbzx stream");
        return nil;
    }

    xar_extract_tostream_end(&_xar_stream);
    
    return self;
}

- (void)dealloc
{
    lzma_end(&_zs);

    if (_xar)
        xar_close(_xar);
    if (_archive)
        archive_read_finish(_archive);
}

- (void)cancelOperation:(id)arg1
{
    _abort = YES;
}

- (void)startUnarchivingAtPath:(NSString *)path
                 notifyOnQueue:(dispatch_queue_t)queue
                      progress:(void (^)(double, NSString *))progressBlock
                        finish:(void (^)(BOOL))finishBlock
{
    if (_archive)
    {
        NSLog(@"PackageKit: Unarchiving already in progress\n");
        dispatch_async(queue, ^{
            finishBlock(NO);
        });
        return;
    }

    int dfd = open([path UTF8String], O_RDONLY | O_DIRECTORY);
    if (dfd == -1)
    {
        NSLog(@"PackageKit: target directory cannot be opened\n");
        dispatch_async(queue, ^{
            finishBlock(NO);
        });
        return;
    }

    if (lzma_stream_decoder(&_zs, UINT64_MAX, LZMA_CONCATENATED) != LZMA_OK)
    {
        close(dfd);
        NSLog(@"PackageKit: LZMA init failed\n");
        dispatch_async(queue, ^{
            finishBlock(NO);
        });
        return;
    }

    char header[4];
    xar_read(header, sizeof(header), &_xar_stream);
    _flags = xar_read_64(&_xar_stream);

    _archive = archive_read_new();

    archive_read_support_format_cpio(_archive);

    dispatch_queue_t q = dispatch_queue_create("org.darlinghq.PackageKit", NULL);

    _abort = NO;
    dispatch_async(q, ^{
        BOOL result = YES;
        struct ReadContext* rc = (struct ReadContext*) malloc(sizeof(struct ReadContext));

        rc->xs = &_xar_stream;
        rc->blockLength = 0;

        pthread_fchdir_np(dfd);
        close(dfd);

        int err = archive_read_open(_archive, (__bridge void*) self, open_cb, read_cb, close_cb);
        if (err != ARCHIVE_OK)
        {
            NSLog(@"archive_read_open error\n");
            result = NO;
            goto out;
        }

        struct archive* ext = archive_write_disk_new();

        while (!_abort)
        {
            struct archive_entry *entry;
            int r = archive_read_next_header(_archive, &entry);

            if (r == ARCHIVE_EOF)
                break;

            if (r != ARCHIVE_OK)
            {
                NSLog(@"archive_read_next_header failed\n");
                result = NO;
                break;
            }

            r = archive_write_header(ext, entry);
			if (r != ARCHIVE_OK)
            {
				NSLog(@"archive_write_header failed\n");
                result = NO;
                break;
            }
            r = copy_data(_archive, ext);
            if (r != ARCHIVE_OK)
            {
                NSLog(@"failed to write data\n");
                result = NO;
                break;
            }

            r = archive_write_finish_entry(ext);
            if (r != ARCHIVE_OK)
            {
				NSLog(@"archive_write_finish_entry failed\n");
                result = NO;
                break;
            }

            // TODO: report progress
        }

out:
        archive_write_finish(ext);
        archive_read_close(_archive);
        archive_read_finish(_archive);
        _archive = NULL;
        free(rc);
        pthread_fchdir_np(-1);

        dispatch_async(queue, ^{
            finishBlock(result);
        });
    });

    dispatch_release(q);
}

- (xar_stream*)_xarStream
{
    return &_xar_stream;
}

@end
