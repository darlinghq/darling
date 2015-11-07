#include "termios.h"
#include "../../../../../platform-include/sys/errno.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Speeds are stored in cflags
// http://osxr.org/glibc/source/sysdeps/unix/sysv/linux/speed.c?v=glibc-2.13

extern void *memset(void *s, int c, size_t n);
static void cc_linux_to_bsd(const unsigned char* l, unsigned char* b);
static void cc_bsd_to_linux(const unsigned char* b, unsigned char* l);
static void iflag_linux_to_bsd(unsigned int l, unsigned long* b);
static void iflag_bsd_to_linux(unsigned long b, unsigned int* l);

int handle_termios(int fd, int cmd, void* arg, int* retval)
{
	switch (cmd)
	{
		case BSD_TIOCGETA:
		{
			struct bsd_termios* out = (struct bsd_termios*) arg;
			struct linux_termios in;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in);

			// TODO: translate termios
			memset(out, 0, sizeof(*out));
			cc_linux_to_bsd(in.c_cc, out->c_cc);
			iflag_linux_to_bsd(in.c_iflag, &out->c_iflag);

			return IOCTL_HANDLED;
		}
		case BSD_TIOCSETA:
		case BSD_TIOCSETAW:
		case BSD_TIOCSETAF:
		{
			const struct bsd_termios* in = (struct bsd_termios*) arg;
			struct linux_termios out;
			int op;

			if (cmd == BSD_TIOCSETA)
				op = LINUX_TCSETS;
			else if (cmd == BSD_TIOCSETAW)
				op = LINUX_TCSETSW;
			else
				op = LINUX_TCSETSF;

			// TODO: translate termios
			memset(&out, 0, sizeof(out));
			cc_bsd_to_linux(in->c_cc, out.c_cc);
			iflag_bsd_to_linux(in->c_iflag, &out.c_iflag);

			*retval = __real_ioctl(fd, op, &out);
			
			return IOCTL_HANDLED;
		}
		case BSD_TIOCGWINSZ:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCGWINSZ, arg);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCSWINSZ:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCSWINSZ, arg);
			return IOCTL_HANDLED;
		}
		default:
			return IOCTL_PASS;
	}
}

static void cc_linux_to_bsd(const unsigned char* l, unsigned char* b)
{
	b[BSD_VEOF] = l[LINUX_VEOF];
	b[BSD_VEOL] = l[LINUX_VEOL];
	b[BSD_VEOL2] = l[LINUX_VEOL2];
	b[BSD_VERASE] = l[LINUX_VERASE];
	b[BSD_VWERASE] = l[LINUX_VWERASE];
	b[BSD_VKILL] = l[LINUX_VKILL];
	b[BSD_VREPRINT] = l[LINUX_VREPRINT];
	b[BSD_VINTR] = l[LINUX_VINTR];
	b[BSD_VQUIT] = l[LINUX_VQUIT];
	b[BSD_VSUSP] = l[LINUX_VSUSP];
	b[BSD_VSTART] = l[LINUX_VSTART];
	b[BSD_VSTOP] = l[LINUX_VSTOP];
	b[BSD_VLNEXT] = l[LINUX_VLNEXT];
	b[BSD_VDISCARD] = l[LINUX_VDISCARD];
	b[BSD_VMIN] = l[LINUX_VMIN];
	b[BSD_VTIME] = l[LINUX_VTIME];
}

static void cc_bsd_to_linux(const unsigned char* b, unsigned char* l)
{
	l[LINUX_VEOF] = b[BSD_VEOF];
	l[LINUX_VEOL] = b[BSD_VEOL];
	l[LINUX_VEOL2] = b[BSD_VEOL2];
	l[LINUX_VERASE] = b[BSD_VERASE];
	l[LINUX_VWERASE] = b[BSD_VWERASE];
	l[LINUX_VKILL] = b[BSD_VKILL];
	l[LINUX_VREPRINT] = b[BSD_VREPRINT];
	l[LINUX_VINTR] = b[BSD_VINTR];
	l[LINUX_VQUIT] = b[BSD_VQUIT];
	l[LINUX_VSUSP] = b[BSD_VSUSP];
	l[LINUX_VSTART] = b[BSD_VSTART];
	l[LINUX_VSTOP] = b[BSD_VSTOP];
	l[LINUX_VLNEXT] = b[BSD_VLNEXT];
	l[LINUX_VDISCARD] = b[BSD_VDISCARD];
	l[LINUX_VMIN] = b[BSD_VMIN];
	l[LINUX_VTIME] = b[BSD_VTIME];
}

static void iflag_linux_to_bsd(unsigned int l, unsigned long* b)
{
	if (l & LINUX_IGNBRK)
		*b |= BSD_IGNBRK;
	if (l & LINUX_BRKINT)
		*b |= BSD_BRKINT;
	if (l & LINUX_IGNPAR)
		*b |= BSD_IGNPAR;
	if (l & LINUX_PARMRK)
		*b |= BSD_PARMRK;
	if (l & LINUX_INPCK)
		*b |= BSD_INPCK;
	if (l & LINUX_ISTRIP)
		*b |= BSD_ISTRIP;
	if (l & LINUX_INLCR)
		*b |= BSD_INLCR;
	if (l & LINUX_IGNCR)
		*b |= BSD_IGNCR;
	if (l & LINUX_ICRNL)
		*b |= BSD_ICRNL;
	if (l & LINUX_IXON)
		*b |= BSD_IXON;
	if (l & LINUX_IXANY)
		*b |= BSD_IXANY;
	if (l & LINUX_IXOFF)
		*b |= BSD_IXOFF;
	if (l & LINUX_IMAXBEL)
		*b |= BSD_IMAXBEL;
	if (l & LINUX_IUTF8)
		*b |= BSD_IUTF8;
}

static void iflag_bsd_to_linux(unsigned long b, unsigned int* l)
{
	if (b & BSD_IGNBRK)
		*l |= LINUX_IGNBRK;
	if (b & BSD_BRKINT)
		*l |= LINUX_BRKINT;
	if (b & BSD_IGNPAR)
		*l |= LINUX_IGNPAR;
	if (b & BSD_PARMRK)
		*l |= LINUX_PARMRK;
	if (b & BSD_INPCK)
		*l |= LINUX_INPCK;
	if (b & BSD_ISTRIP)
		*l |= LINUX_ISTRIP;
	if (b & BSD_INLCR)
		*l |= LINUX_INLCR;
	if (b & BSD_IGNCR)
		*l |= LINUX_IGNCR;
	if (b & BSD_ICRNL)
		*l |= LINUX_ICRNL;
	if (b & BSD_IXON)
		*l |= LINUX_IXON;
	if (b & BSD_IXOFF)
		*l |= LINUX_IXOFF;
	if (b & BSD_IXANY)
		*l |= LINUX_IXANY;
	if (b & BSD_IMAXBEL)
		*l |= LINUX_IMAXBEL;
	if (b & BSD_IUTF8)
		*l |= LINUX_IUTF8;
}

