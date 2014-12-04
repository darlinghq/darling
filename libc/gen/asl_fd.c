/*
 * Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/event.h>
#include <asl.h>
#include <asl_private.h>
#include <errno.h>
#include <fcntl.h>
#include <dispatch/dispatch.h>

/* asl.c */
__private_extern__ void asl_client_release(asl_client_t *asl);
__private_extern__ asl_client_t *asl_client_retain(asl_client_t *asl);

#define BUF_SIZE 512

static dispatch_queue_t redirect_serial_q;
static dispatch_group_t read_source_group;

typedef struct {
    int level;
    asl_client_t *asl;
    asl_msg_t *msg;

    /* Buffered reading */
    char *buf;
    char *w;

    dispatch_source_t read_source;
} asl_redirect_t;

static asl_redirect_t *redirect_descriptors = NULL;
static int n_redirect_descriptors = 0;

/* Read from the FD until there is no more to read and redirect to ASL.
 * Preconditions:
 *      1: called from the appropriate serial queue for operating on
 *         redirect_descriptors
 *      2: descriptor corresponds to a valid entry in redirect_descriptors
 *
 * Return values:
 *      If the pipe is closed, EOF is returned regardless of how many bytes
 *      were processed.  If the pipe is still open, the number of read bytes
 *      is returned.
 */
static inline int _read_redirect(int descriptor, int flush) {
    int total_read = 0;
    int nbytes;
    asl_redirect_t *aslr = &redirect_descriptors[descriptor];

    while ((nbytes = read(descriptor, aslr->w, BUF_SIZE - (aslr->w - aslr->buf) - 1)) > 0) {
        char *s, *p;

        /* Increment our returned number read */
        total_read += nbytes;

        nbytes += (aslr->w - aslr->buf);
        aslr->buf[nbytes] = '\0';

        /* One line at a time */
        for (p=aslr->buf; *p && (p - aslr->buf) < nbytes; p = s + 1) {
            // Find null or \n
            for (s=p; *s && *s != '\n'; s++);
            if (*s == '\n') {
                *s='\0';
                asl_log((aslclient)aslr->asl, (aslmsg)aslr->msg, aslr->level, "%s", p);
            } else if (aslr->buf != p) {
                memmove(aslr->buf, p, BUF_SIZE - (p - aslr->buf));
                aslr->w = aslr->buf + (s - p);
                break;
            } else if (nbytes == BUF_SIZE - 1) {
                asl_log((aslclient)aslr->asl, (aslmsg)aslr->msg, aslr->level, "%s", p);
                aslr->w = aslr->buf;
                break;
            }
        }
    }

    /* Flush if requested or we're at EOF */
    if (flush || nbytes == 0) {
        if (aslr->w > aslr->buf) {
            *aslr->w = '\0';
            asl_log((aslclient)aslr->asl, (aslmsg)aslr->msg, aslr->level, "%s", aslr->buf);
        }
    }

    if (nbytes == 0)
        return EOF;
    return total_read;
}

static void read_from_source(void *_source) {
    dispatch_source_t source = (dispatch_source_t)_source;
    int descriptor = dispatch_source_get_handle(source);
    if (_read_redirect(descriptor, 0) == EOF) {
        dispatch_source_cancel(source);
    }
}

static void cancel_source(void *_source) {
    dispatch_source_t source = (dispatch_source_t)_source;
    int descriptor = dispatch_source_get_handle(source);
    asl_redirect_t *aslr = &redirect_descriptors[descriptor];

    /* Flush the buffer */
    _read_redirect(descriptor, 1);

    close(descriptor);

    asl_client_release(aslr->asl);
    asl_msg_release(aslr->msg);
    free(aslr->buf);

    memset(aslr, 0, sizeof(*aslr));
    dispatch_release(source);
    dispatch_group_leave(read_source_group);
}


static void redirect_atexit(void) {
    int i;

    /* stdout is linebuffered, so flush the buffer */
    if (redirect_descriptors[STDOUT_FILENO].buf)
        fflush(stdout);

    /* Cancel all of our dispatch sources, so they flush to ASL */
    for (i=0; i < n_redirect_descriptors; i++)
        if (redirect_descriptors[i].read_source)
            dispatch_source_cancel(redirect_descriptors[i].read_source);

    /* Wait at least three seconds for our sources to flush to ASL */
    dispatch_group_wait(read_source_group, dispatch_time(DISPATCH_TIME_NOW, 3LL * NSEC_PER_SEC));
}

static void asl_descriptor_init(void *ctx __unused)
{
    assert((redirect_descriptors = calloc(16, sizeof(*redirect_descriptors))) != NULL);
    n_redirect_descriptors = 16;

    redirect_serial_q = dispatch_queue_create("com.apple.asl-redirect", NULL);
    assert(redirect_serial_q != NULL);

    read_source_group = dispatch_group_create();
    assert(read_source_group != NULL);

    atexit(redirect_atexit);
}

static int asl_log_from_descriptor(aslclient ac, aslmsg am, int level, int descriptor) {
    int err __block = 0;
    static dispatch_once_t once_control;
    dispatch_once_f(&once_control, NULL, asl_descriptor_init);
    asl_client_t *asl = (asl_client_t *)ac;
    asl_msg_t *msg = (asl_msg_t *)am;

    if (descriptor < 0)
        return EBADF;

    if (msg != NULL) {
        msg = asl_msg_copy(msg);
        if (msg == NULL)
            return ENOMEM;
    }

    dispatch_sync(redirect_serial_q, ^{
        dispatch_source_t read_source;

        /* Reallocate if we need more space */
        if (descriptor >= n_redirect_descriptors) {
            size_t new_n = 1 << (ffs(descriptor) + 1);
            asl_redirect_t *new_array = realloc(redirect_descriptors, new_n * sizeof(*redirect_descriptors));
            if (!new_array) {
                err = errno;
                return;
            }
            redirect_descriptors = new_array;
            memset(redirect_descriptors + n_redirect_descriptors, 0, new_n - n_redirect_descriptors);
            n_redirect_descriptors = new_n;
        }
        
        /* If we're already listening on it, return error. */
        if (redirect_descriptors[descriptor].buf != NULL) {
            err = EBADF;
            return;
        }
        
        /* Initialize our buffer */
        redirect_descriptors[descriptor].buf = (char *)malloc(BUF_SIZE);
        if (redirect_descriptors[descriptor].buf == NULL) {
            err = errno;
            return;
        }
        redirect_descriptors[descriptor].w = redirect_descriptors[descriptor].buf;
        
        /* Store our ASL settings */
        redirect_descriptors[descriptor].level = level;
        redirect_descriptors[descriptor].asl = asl_client_retain(asl);
        redirect_descriptors[descriptor].msg = msg;
        
        /* Don't block on reads from this descriptor */
        (void)fcntl(descriptor, F_SETFL, O_NONBLOCK);
        
        /* Start listening */
        read_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, descriptor, 0, redirect_serial_q);
        redirect_descriptors[descriptor].read_source = read_source;
        dispatch_set_context(read_source, read_source);
        dispatch_source_set_event_handler_f(read_source, read_from_source);
        dispatch_source_set_cancel_handler_f(read_source, cancel_source);
        dispatch_group_enter(read_source_group);
        dispatch_resume(read_source);
    });

    if (err) {
        asl_msg_release(msg);
    }
    
    return err;
}

int asl_log_descriptor(aslclient ac, aslmsg am, int level, int descriptor, uint32_t fd_type) {
    int pipepair[2];
    int retval;
    int oerrno = errno;

    if (fd_type == ASL_LOG_DESCRIPTOR_READ)
        return asl_log_from_descriptor(ac, am, level, descriptor);

    assert(fd_type == ASL_LOG_DESCRIPTOR_WRITE);

    /* Create pipe */
    if (pipe(pipepair) == -1) {
        retval = errno;
        errno = oerrno;
        return retval;
    }
    
    /* Close the read descriptor but not the write descriptor on exec */
    if (fcntl(pipepair[0], F_SETFD, FD_CLOEXEC) == -1) {
        retval = errno;
        errno = oerrno;
        return retval;
    }

    /* Replace the existing descriptor */
    if (dup2(pipepair[1], descriptor) == -1) {
        close(pipepair[0]);
        close(pipepair[1]);
        retval = errno;
        errno = oerrno;
        return retval;
    }

    /* If we capture STDOUT_FILENO, make sure we linebuffer stdout */
    if (descriptor == STDOUT_FILENO)
        setlinebuf(stdout);
    
    /* Close the duplicate descriptors since they've been reassigned */
    close(pipepair[1]);

    /* Hand off the read end of our pipe to asl_log_descriptor */
    return asl_log_from_descriptor(ac, am, level, pipepair[0]);
}
