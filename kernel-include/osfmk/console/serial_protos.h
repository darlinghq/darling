/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * @APPLE_FREE_COPYRIGHT@
 */

#ifndef _CONSOLE_SERIAL_PROTOS_H_
#define _CONSOLE_SERIAL_PROTOS_H_

#ifdef __cplusplus
extern "C" {
#endif


void serial_keyboard_init(void);
void serial_keyboard_start(void) __dead2;
void serial_keyboard_poll(void) __dead2;

extern uint32_t serialmode;

#define SERIALMODE_OUTPUT    0x1
#define SERIALMODE_INPUT     0x2
#define SERIALMODE_SYNCDRAIN 0x4

extern uint32_t cons_ops_index;
extern const uint32_t nconsops;
extern unsigned int disable_serial_output;
#if defined(__arm__) || defined(__arm64__)
/* ARM64_TODO */
extern void *console_cpu_alloc(boolean_t);
extern void console_cpu_free(void *);
void console_init(void);
#endif

int _serial_getc(int unit, int line, boolean_t wait, boolean_t raw);

struct console_ops {
	void    (*putc)(int, int, int);
	int     (*getc)(int, int, boolean_t, boolean_t);
};

boolean_t console_is_serial(void);
int switch_to_serial_console(void);
int switch_to_video_console(void);
void switch_to_old_console(int old_console);

#define SERIAL_CONS_OPS 0
#define VC_CONS_OPS 1

#ifdef XNU_KERNEL_PRIVATE

#define SERIAL_CONS_BUF_SIZE  256
struct console_printbuf_state {
	int pos;
	int total;
	int flags;
#define CONS_PB_WRITE_NEWLINE  0x1
#define CONS_PB_CANBLOCK       0x2
	char str[SERIAL_CONS_BUF_SIZE];
};

extern int console_printbuf_drain_initialized;
void console_printbuf_state_init(struct console_printbuf_state * data, int write_on_newline, int can_block);
void console_printbuf_putc(int ch, void *arg);
void console_printbuf_clear(struct console_printbuf_state * info);
int console_write_try(char * str, int size);


#endif /* XNU_KERNEL_PRIVATE */

#ifdef __cplusplus
}
#endif


#endif /* _CONSOLE_SERIAL_PROTOS_H_ */
