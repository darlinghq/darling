/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <mach/mach.h>
#include <notify.h>
#include <notify_private.h>
#include <signal.h>
#include <dispatch/dispatch.h>

#define forever for(;;)
#define IndexNull ((uint32_t)-1)

#define PRINT_QUIET    0x00000000
#define PRINT_KEY      0x00000001
#define PRINT_STATE    0x00000002
#define PRINT_TIME     0x00000004
#define PRINT_TYPE     0x00000008
#define PRINT_VERBOSE  0xffffffff

#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000
#endif

#define TYPE_NULL 0
#define TYPE_PORT 1
#define TYPE_FILE 2
#define TYPE_DISPATCH 3
#define TYPE_SIGNAL 4
#define TYPE_CHECK 5
#define TYPE_PLAIN 6

static const char *typename[] = 
{
	"unknown",
	"port",
	"file",
	"dispatch",
	"signal",
	"check",
	"plain"
};

extern uint32_t notify_register_plain(const char *name, int *out_token);

typedef struct
{
	uint32_t token;
	uint32_t type;
	uint32_t signum;
	uint32_t count;
	char *name;
} reg_entry_t;

static reg_entry_t *reg;
static uint32_t reg_count = 0;

static int printopt;
static int port_flag;
static int file_flag;
static int watch_file;
static mach_port_t watch_port;
dispatch_source_t timer_src;
dispatch_source_t port_src;
dispatch_source_t file_src;
dispatch_source_t sig_src[__DARWIN_NSIG];
dispatch_queue_t watch_queue;

static void
usage(const char *name)
{
	fprintf(stderr, "usage: %s [-q] [-v] [-z msec] [-M] [-R] [command ...]\n", name);
	fprintf(stderr, "    -q             quiet mode\n");
	fprintf(stderr, "    -v             verbose - prints time, key, state value, and type\n");
	fprintf(stderr, "    -z msec        pause msec milliseconds after posting [default 100]\n");
	fprintf(stderr, "    -M             multiplex notifications from notifyd over a single mach port\n");
	fprintf(stderr, "    -R             regenerate registrations if notifyd restarts\n");
	fprintf(stderr, "commands:\n");
	fprintf(stderr, "    -port          switch to mach port for subsequent registrations [default]\n");
	fprintf(stderr, "    -file          switch to file descriptor for subsequent registrations\n");
	fprintf(stderr, "    -check         switch to shared memory for subsequent registrations\n");
	fprintf(stderr, "    -signal [#]    switch to signal [#] for subsequent registrations\n");
	fprintf(stderr, "                   initial default for signal is 1 (SIGHUP)\n");
	fprintf(stderr, "    -dispatch      switch to dispatch for subsequent registrations\n");
	fprintf(stderr, "    -p key         post a notifcation for key\n");
	fprintf(stderr, "    -w key         register for key and report notifications\n");
	fprintf(stderr, "    -# key         (# is an integer value, eg \"-1\") register for key and report # notifications\n");
	fprintf(stderr, "    -g key         get state value for key\n");
	fprintf(stderr, "    -s key val     set state value for key\n");
}

static const char *
notify_status_strerror(int status)
{
	switch (status)
	{
		case NOTIFY_STATUS_OK: return("OK");
		case NOTIFY_STATUS_INVALID_NAME: return "Invalid Name";
		case NOTIFY_STATUS_INVALID_TOKEN: return "Invalid Token";
		case NOTIFY_STATUS_INVALID_PORT: return "Invalid Port";
		case NOTIFY_STATUS_INVALID_FILE: return "Invalid File";
		case NOTIFY_STATUS_INVALID_SIGNAL: return "Invalid Signal";
		case NOTIFY_STATUS_INVALID_REQUEST: return "Invalid Request";
		case NOTIFY_STATUS_NOT_AUTHORIZED: return "Not Authorized";
		case NOTIFY_STATUS_FAILED:
		default: return "Failed";
	}
}

static void
reg_add(uint32_t tid, uint32_t type, uint32_t signum, uint32_t count, const char *name)
{
	if (name == NULL) return;

	reg = (reg_entry_t *)reallocf(reg, (reg_count + 1) * sizeof(reg_entry_t));
	if (reg == NULL)
	{
		fprintf(stderr, "Can't allocate memory!\n");
		reg_count = 0;
		return;
	}

	reg[reg_count].token = tid;
	reg[reg_count].type = type;
	reg[reg_count].signum = signum;
	reg[reg_count].count = count;
	reg[reg_count].name = strdup(name);
	if (reg[reg_count].name == NULL)
	{
		fprintf(stderr, "Can't allocate memory!\n");
		reg = NULL;
		reg_count = 0;
		return;
	}

	reg_count++;
}

static void
reg_delete(uint32_t index)
{
	uint32_t i;

	if (index == IndexNull) return;
	if (index >= reg_count) return;

	free(reg[index].name);

	for (i = index + 1; i < reg_count; i++) reg[i - 1] = reg[i];
	reg_count--;

	if (reg_count == 0)
	{
		free(reg);
		reg = NULL;
	}
	else
	{
		reg = (reg_entry_t *)reallocf(reg, reg_count * sizeof(reg_entry_t));
		if (reg == NULL)
		{
			fprintf(stderr, "Can't allocate memory!\n");
			reg_count = 0;
		}
	}
}

static uint32_t
reg_find_name(const char *name)
{
	uint32_t i;

	for (i = 0; i < reg_count; i++) if (!strcmp(reg[i].name, name)) return i;
	return IndexNull;
}

static uint32_t
reg_find_token(uint32_t tid)
{
	uint32_t i;

	for (i = 0; i < reg_count; i++) if (tid == reg[i].token) return i;
	return IndexNull;
}

static void
process_event(int tid)
{
	struct timeval now;
	char tstr[32];
	int status, index, needspace;
	uint64_t state;

	gettimeofday(&now, NULL);

	index = reg_find_token(tid);
	if (index == IndexNull) return;

	needspace = 0;

	if (printopt & PRINT_TIME)
	{
		snprintf(tstr, sizeof(tstr), "%llu", now.tv_usec + USEC_PER_SEC + 500);
		tstr[4] = '\0';
		printf("%d.%s", (int)now.tv_sec, tstr+1);
		needspace = 1;
	}

	if (printopt & PRINT_KEY)
	{
		if (needspace) printf(" ");
		printf("%s", reg[index].name);
		needspace = 1;
	}

	if (printopt & PRINT_STATE)
	{
		if (needspace) printf(" ");
		state = 0;
		status = notify_get_state(tid, &state);
		if (status == NOTIFY_STATUS_OK) printf("%llu",(unsigned long long)state);
		else printf(": %s", notify_status_strerror(status));
		needspace = 1;
	}

	if (printopt & PRINT_TYPE)
	{
		if (needspace) printf(" ");
		printf("%s", typename[reg[index].type]);
		needspace = 1;
	}
	
	if (printopt != PRINT_QUIET) printf("\n");

	if ((reg[index].count != IndexNull) && (reg[index].count != 0)) reg[index].count--;
	if (reg[index].count == 0)
	{
		status = notify_cancel(tid);
		reg_delete(index);
	}
}

static void 
file_handler(int fd)
{
	ssize_t i;
	int tid;

	if (fd < 0) return;

	i = read(fd, &tid, sizeof(tid));
	if (i < 0) return;

	tid = ntohl(tid);
	process_event(tid);

	if (reg_count == 0) exit(0);
}

static void 
port_handler(mach_port_t port)
{
	int tid;
	mach_msg_empty_rcv_t msg;
	kern_return_t status;

	if (port == MACH_PORT_NULL) return;

	memset(&msg, 0, sizeof(msg));
	status = mach_msg(&msg.header, MACH_RCV_MSG, 0, sizeof(msg), port, 0, MACH_PORT_NULL);
	if (status != KERN_SUCCESS) return;

	tid = msg.header.msgh_id;
	process_event(tid);

	if (reg_count == 0) exit(0);
}

static void
signal_handler(uint32_t sig)
{
	uint32_t i, status;
	int check;

	if (printopt != PRINT_QUIET) printf("SIGNAL %u\n", sig);
	for (i = 0; i < reg_count; i++)
	{
		if ((reg[i].type == TYPE_SIGNAL) && (reg[i].signum == sig))
		{
			check = 0;
			status = notify_check(reg[i].token, &check);
			if ((status == NOTIFY_STATUS_OK) && (check != 0)) process_event(reg[i].token);
		}
	}

	if (reg_count == 0) exit(0);
}

static void
dispatch_handler(const char *name)
{
	uint32_t index = reg_find_name(name);
	if (index == IndexNull) return;

	process_event(reg[index].token);
}

static void
timer_handler(void)
{
	uint32_t i, status;
	int check;

	for (i = 0; i < reg_count; i++)
	{
		if ((reg[i].type == TYPE_CHECK) || (reg[i].type == TYPE_PLAIN))
		{
			check = 0;
			status = notify_check(reg[i].token, &check);
			if ((status == NOTIFY_STATUS_OK) && (check != 0)) process_event(reg[i].token);
		}
	}

	if (reg_count == 0) exit(0);
}

static uint32_t
do_register(const char *name, uint32_t type, uint32_t signum, uint32_t count)
{
	int tid, check;
	uint32_t status;

	switch (type)
	{
		case TYPE_PORT:
		{
			status = notify_register_mach_port(name, &watch_port, port_flag, &tid);
			if (status != NOTIFY_STATUS_OK) return status;

			port_flag = NOTIFY_REUSE;
			if (port_src == NULL)
			{
				port_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, watch_port, 0, watch_queue);
				dispatch_source_set_event_handler(port_src, ^{
					port_handler(watch_port);
				});
				dispatch_resume(port_src);
			}

			break;
		}

		case TYPE_FILE:
		{
			status = notify_register_file_descriptor(name, &watch_file, file_flag, &tid);
			if (status != NOTIFY_STATUS_OK) return status;

			file_flag = NOTIFY_REUSE;
			if (file_src == NULL)
			{
				file_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, (uintptr_t)watch_file, 0, watch_queue);
				dispatch_source_set_event_handler(file_src, ^{
					file_handler(watch_file);
				});
				dispatch_resume(file_src);
			}

			break;
		}

		case TYPE_SIGNAL:
		{
			signal(signum, SIG_IGN);

			status = notify_register_signal(name, signum, &tid);
			if (status != NOTIFY_STATUS_OK) return status;

			status = notify_check(tid, &check);

			if (sig_src[signum] == NULL)
			{
				sig_src[signum] = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t)signum, 0, watch_queue);
				dispatch_source_set_event_handler(sig_src[signum], ^{
					signal_handler(signum);
				});
				dispatch_resume(sig_src[signum]);
			}

			break;
		}

		case TYPE_DISPATCH:
		{
			status = notify_register_dispatch(name, &tid, watch_queue, ^(int x){ dispatch_handler(name); });
			if (status != NOTIFY_STATUS_OK) return status;
			break;
		}

		case TYPE_CHECK:
		{
			status = notify_register_check(name, &tid);
			if (status != NOTIFY_STATUS_OK) return status;

			status = notify_check(tid, &check);

			if (timer_src == NULL)
			{
				timer_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, watch_queue);
				dispatch_source_set_event_handler(timer_src, ^{
					timer_handler();
				});
				dispatch_source_set_timer(timer_src, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 10), NSEC_PER_SEC / 10, 0);
				dispatch_resume(timer_src);
			}

			break;
		}

		case TYPE_PLAIN:
		{
			status = notify_register_plain(name, &tid);
			if (status != NOTIFY_STATUS_OK) return status;

			status = notify_check(tid, &check);

			if (timer_src == NULL)
			{
				timer_src = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, watch_queue);
				dispatch_source_set_event_handler(timer_src, ^{
					timer_handler();
				});
				dispatch_source_set_timer(timer_src, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 10), NSEC_PER_SEC / 10, 0);
				dispatch_resume(timer_src);
			}

			break;
		}

		default: return NOTIFY_STATUS_FAILED;
	}

	reg_add(tid, type, signum, count, name);
	return NOTIFY_STATUS_OK;
}

int
main(int argc, const char *argv[])
{
	const char *name;
	uint32_t i, n, index, signum, ntype, status, opts, nap;
	int tid;
	uint64_t state;

	for (i = 0; i < __DARWIN_NSIG; i++) sig_src[i] = NULL;

	ntype = TYPE_PORT;
	signum = 1;
	watch_file = -1;
	watch_port = MACH_PORT_NULL;
	printopt = PRINT_KEY;
	opts = 0;
	nap = 100000;

	watch_queue = dispatch_queue_create("Watch Q", NULL);

	name = strrchr(argv[0], '/');
	if (name == NULL) name = argv[0];
	else name++;

	for (i = 1; i < argc; i++)
	{
		if ((!strcmp(argv[i], "-help")) || (!strcmp(argv[i], "-h")))
		{
			usage(name);
			exit(0);
		}
		else if (!strcmp(argv[i], "-q"))
		{
			printopt = PRINT_QUIET;
		}
		else if (!strcmp(argv[i], "-v"))
		{
			printopt |= PRINT_VERBOSE;
		}
		else if (!strcmp(argv[i], "-M"))
		{
			opts |= NOTIFY_OPT_DEMUX;
		}
		else if (!strcmp(argv[i], "-R"))
		{
			opts |= NOTIFY_OPT_REGEN;
		}
		else if (!strcmp(argv[i], "-z"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "timer value must be supplied following -z\n");
				usage(name);
				exit(1);
			}
			
			i++;

			if ((argv[i][0] < '0') || (argv[i][1] > '9'))
			{
				fprintf(stderr, "-z %s is invalid\n", argv[i]);
				fprintf(stderr, "timer value must be an integer\n");
				usage(name);
				exit(1);
			}
			nap = 1000 * atoi(argv[i]);
		}
		else if (!strcmp(argv[i], "-port"))
		{}
		else if (!strcmp(argv[i], "-file"))
		{}
		else if ((!strcmp(argv[i], "-sig")) || (!strcmp(argv[i], "-signal")))
		{
			if ((i + 1) >= argc) continue;
			if (argv[i + 1][0] == '-') continue;

			i++;

			if ((argv[i][0] < '0') || (argv[i][1] > '9'))
			{
				fprintf(stderr, "-signal %s is invalid\n", argv[i]);
				fprintf(stderr, "signals must be specified as integer values\n");
				usage(name);
				exit(1);
			}

		}
		else if (!strcmp(argv[i], "-dispatch"))
		{}
		else if (!strcmp(argv[i], "-check"))
		{}
		else if (!strcmp(argv[i], "-plain"))
		{}
		else if (!strcmp(argv[i], "-p"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "name required following -p\n");
				usage(name);
				exit(1);
			}
			
			i++;
		}
		else if ((argv[i][0] == '-') && ((argv[i][1] == 'w') || ((argv[i][1] >= '0') && (argv[i][1] <= '9'))))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "name required following %s\n", argv[i]);
				usage(name);
				exit(1);
			}
			
			i++;
		}
		else if (!strcmp(argv[i], "-g"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "name required following -g\n");
				usage(name);
				exit(1);
			}

			i++;
		}
		else if (!strcmp(argv[i], "-s"))
		{
			if ((i + 1) >= argc)
			{
				fprintf(stderr, "name required following -s\n");
				usage(name);
				exit(1);
			}
			
			i++;

			if ((i + 1) >= argc)
			{
				fprintf(stderr, "value required following -s name\n");
				usage(name);
				exit(1);
			}
			
			i++;
			state = atoll(argv[i]);
			if ((state == 0) && (strcmp(argv[i], "0")))
			{
				fprintf(stderr, "value following -s name must be a 64-bit integer\n");
			}
		}
		else
		{
			fprintf(stderr, "unrecognized option: %s\n", argv[i]);
			usage(name);
			exit(1);
		}
	}

    if (opts != 0) notify_set_options(opts);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-port"))
		{
			ntype = TYPE_PORT;
		}
		else if (!strcmp(argv[i], "-file"))
		{
			ntype = TYPE_FILE;
		}
		else if ((!strcmp(argv[i], "-sig")) || (!strcmp(argv[i], "-signal")))
		{

			ntype = TYPE_SIGNAL;
			if (((i + 1) < argc) && (argv[i + 1][0] != '-'))
			{
				i++;
				signum = atoi(argv[i]);
			}
		}
		else if (!strcmp(argv[i], "-dispatch"))
		{
			ntype = TYPE_DISPATCH;
		}
		else if (!strcmp(argv[i], "-check"))
		{
			ntype = TYPE_CHECK;
		}
		else if (!strcmp(argv[i], "-plain"))
		{
			ntype = TYPE_PLAIN;
		}
		else if (!strcmp(argv[i], "-p"))
		{
			if ((i + 1) >= argc)
			{
				usage(name);
				exit(1);
			}

			i++;

			status = notify_post(argv[i]);
			if (status != NOTIFY_STATUS_OK) printf("%s: %s\n", argv[i], notify_status_strerror(status));
			else if (nap > 0) usleep(nap);
		}
		else if ((argv[i][0] == '-') && ((argv[i][1] == 'w') || ((argv[i][1] >= '0') && (argv[i][1] <= '9'))))
		{
			if ((i + 1) >= argc)
			{
				usage(name);
				exit(1);
			}

			n = IndexNull;
			if (argv[i][1] != 'w') n = atoi(argv[i] + 1);

			i++;
			tid = IndexNull;

			index = reg_find_name(argv[i]);
			if (index != IndexNull)
			{
				fprintf(stderr, "Already watching for %s\n", argv[i]);
				continue;
			}

			status = do_register(argv[i], ntype, signum, n);
			if (status != NOTIFY_STATUS_OK) printf("%s: %s\n", argv[i], notify_status_strerror(status));
		}
		else if (!strcmp(argv[i], "-g"))
		{
			if ((i + 1) >= argc)
			{
				usage(name);
				exit(1);
			}

			i++;
			state = 0;
			tid = IndexNull;

			status = notify_register_plain(argv[i], &tid);
			if (status == NOTIFY_STATUS_OK)
			{
				status = notify_get_state(tid, &state);
				notify_cancel(tid);
			}

			if (status == NOTIFY_STATUS_OK) printf("%s %llu\n", argv[i], (unsigned long long)state);
			else printf("%s: %s\n", argv[i], notify_status_strerror(status));
		}
		else if (!strcmp(argv[i], "-s"))
		{
			if ((i + 2) >= argc)
			{
				usage(name);
				exit(1);
			}

			i++;
			tid = IndexNull;
			status = notify_register_plain(argv[i], &tid);
			if (status == NOTIFY_STATUS_OK)
			{
				state = atoll(argv[i + 1]);
				status = notify_set_state(tid, state);
				notify_cancel(tid);
			}

			if (status != NOTIFY_STATUS_OK)  printf("%s: %s\n", argv[i], notify_status_strerror(status));
			i++;
		}
	}

	if (reg_count == 0) exit(0);

	dispatch_main();
}
