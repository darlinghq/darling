/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * uhid <name> <vendor hex> <product hex> <bus> <descriptor hex>: a virtual HID
 * device made through /dev/uhid, whose report descriptor is the one given, so
 * that the kernel makes the evdev (and hidraw) nodes for it as for a real device.
 * Commands on stdin: report HEX (an input report, with its report ID byte
 * first if the descriptor has report IDs) | sleep MS | quit.
 * Prints "created" once the device is made, and "output HEX" for every output
 * report (LEDs, rumble) the host sends to it. The device is removed on quit.
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/uhid.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int fd;

static int hex(const char *s, unsigned char *out, int max)
{
	int n = 0;
	while (s[0] && s[1] && n < max)
	{
		unsigned v;
		if (sscanf(s, "%2x", &v) != 1)
			break;
		out[n++] = (unsigned char) v;
		s += 2;
		while (*s == ' ')
			s++;
	}
	return n;
}

static void send_event(struct uhid_event *ev)
{
	if (write(fd, ev, sizeof(*ev)) != (ssize_t) sizeof(*ev))
		perror("uhid write");
}

/* Answers what the kernel asks of a device; a device that has no feature reports fails them */
static void drain(void)
{
	struct uhid_event ev;

	while (read(fd, &ev, sizeof(ev)) == (ssize_t) sizeof(ev))
	{
		if (ev.type == UHID_GET_REPORT)
		{
			struct uhid_event reply;
			memset(&reply, 0, sizeof(reply));
			reply.type = UHID_GET_REPORT_REPLY;
			reply.u.get_report_reply.id = ev.u.get_report.id;
			reply.u.get_report_reply.err = EIO;
			send_event(&reply);
		}
		else if (ev.type == UHID_OUTPUT)
		{
			printf("output");
			for (int i = 0; i < ev.u.output.size; i++)
				printf(" %02x", ev.u.output.data[i]);
			printf("\n");
			fflush(stdout);
		}
		else if (ev.type == UHID_SET_REPORT)
		{
			struct uhid_event reply;
			memset(&reply, 0, sizeof(reply));
			reply.type = UHID_SET_REPORT_REPLY;
			reply.u.set_report_reply.id = ev.u.set_report.id;
			reply.u.set_report_reply.err = EIO;
			send_event(&reply);
		}
	}
}

int main(int argc, char **argv)
{
	struct uhid_event ev;
	unsigned char desc[4096];
	int n;
	char line[8192];

	if (argc < 6)
	{
		fprintf(stderr, "usage: uhid name vendor product bus descriptor-hex\n");
		return 2;
	}
	fd = open("/dev/uhid", O_RDWR | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
	{
		perror("/dev/uhid");
		return 1;
	}
	n = hex(argv[5], desc, sizeof(desc));
	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_CREATE2;
	snprintf((char *) ev.u.create2.name, sizeof(ev.u.create2.name), "%s", argv[1]);
	ev.u.create2.rd_size = n;
	ev.u.create2.bus = atoi(argv[4]);
	ev.u.create2.vendor = strtoul(argv[2], 0, 16);
	ev.u.create2.product = strtoul(argv[3], 0, 16);
	ev.u.create2.version = 0x0414;
	memcpy(ev.u.create2.rd_data, desc, n);
	send_event(&ev);
	printf("created\n");
	fflush(stdout);

	for (;;)
	{
		struct pollfd p[2] = { { fd, POLLIN, 0 }, { 0, POLLIN, 0 } };
		int a, b;

		poll(p, 2, 100);
		drain();
		if (!(p[1].revents & POLLIN))
			continue;
		if (!fgets(line, sizeof(line), stdin))
			break;
		if (!strncmp(line, "report ", 7))
		{
			memset(&ev, 0, sizeof(ev));
			ev.type = UHID_INPUT2;
			ev.u.input2.size = hex(line + 7, ev.u.input2.data, sizeof(ev.u.input2.data));
			send_event(&ev);
		}
		else if (sscanf(line, "sleep %i", &a) == 1)
			usleep(a * 1000);
		else if (!strncmp(line, "quit", 4))
			break;
		(void) b;
	}
	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_DESTROY;
	send_event(&ev);
	close(fd);
	return 0;
}
