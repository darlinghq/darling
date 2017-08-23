#include "termios.h"
#include <sys/errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl_compat.h>
#include "../simple.h"

#define USE_OLD_TTY
#include <sys/ioctl_compat.h>

// Speeds are stored in cflags
// http://osxr.org/glibc/source/sysdeps/unix/sysv/linux/speed.c?v=glibc-2.13

extern void *memset(void *s, int c, size_t n);
extern long sys_write(int fd, const void* buf, size_t n);

static void cc_linux_to_bsd(const unsigned char* l, unsigned char* b);
static void cc_bsd_to_linux(const unsigned char* b, unsigned char* l);
static void iflag_linux_to_bsd(unsigned int l, unsigned long* b);
static void iflag_bsd_to_linux(unsigned long b, unsigned int* l);
static void oflag_linux_to_bsd(unsigned int l, unsigned long* b);
static void oflag_bsd_to_linux(unsigned long b, unsigned int* l);
static void cflag_linux_to_bsd(unsigned int l, unsigned long* b);
static void cflag_bsd_to_linux(unsigned long b, unsigned int* l);
static void lflag_linux_to_bsd(unsigned int l, unsigned long* b);
static void lflag_bsd_to_linux(unsigned long b, unsigned int* l);
static void speed_linux_to_bsd(unsigned int l, unsigned long* b);
static void speed_bsd_to_linux(unsigned long b,	unsigned int* l);
static void sgflag_linux_to_bsd(const struct linux_termios* tos, short* sgflags);
static void sgflag_bsd_to_linux(const short sgflags, struct linux_termios* tos);

int handle_termios(int fd, unsigned int cmd, void* arg, int* retval)
{
	switch (cmd)
	{
		case BSD_TIOCGETA:
		{
			struct bsd_termios* out = (struct bsd_termios*) arg;
			struct linux_termios in;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in);

			// translate termios fields
			memset(out, 0, sizeof(*out));
			cc_linux_to_bsd(in.c_cc, out->c_cc);
			iflag_linux_to_bsd(in.c_iflag, &out->c_iflag);
			oflag_linux_to_bsd(in.c_oflag, &out->c_oflag);
			cflag_linux_to_bsd(in.c_cflag, &out->c_cflag);
			lflag_linux_to_bsd(in.c_lflag, &out->c_lflag);

			if (!(in.c_cflag & (LINUX_CBAUD|LINUX_CBAUDEX)))
			{
				speed_linux_to_bsd(in.c_ispeed, &out->c_ispeed);
				speed_linux_to_bsd(in.c_ospeed, &out->c_ospeed);
			}
			else
			{
				speed_linux_to_bsd(in.c_cflag & (LINUX_CBAUD|LINUX_CBAUDEX),
						&out->c_ispeed);
				out->c_ospeed = out->c_ispeed;
			}

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

			// translate termios fields
			memset(&out, 0, sizeof(out));
			cc_bsd_to_linux(in->c_cc, out.c_cc);
			iflag_bsd_to_linux(in->c_iflag, &out.c_iflag);
			oflag_bsd_to_linux(in->c_oflag, &out.c_oflag);
			cflag_bsd_to_linux(in->c_cflag, &out.c_cflag);
			lflag_bsd_to_linux(in->c_lflag, &out.c_lflag);
			speed_bsd_to_linux(in->c_ispeed, &out.c_ispeed);
			speed_bsd_to_linux(in->c_ospeed, &out.c_ospeed);

			*retval = __real_ioctl(fd, op, &out);
			
			return IOCTL_HANDLED;
		}
		case BSD_TIOCGETP:
		{
			struct linux_termios in;
			struct sgttyb* out = (struct sgttyb*) arg;
			unsigned long ispeed, ospeed;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in);
			
			speed_linux_to_bsd(in.c_ispeed, &ispeed);
			speed_linux_to_bsd(in.c_ospeed, &ospeed);
			out->sg_ispeed = ispeed;
			out->sg_ospeed = ospeed;

			out->sg_erase = in.c_cc[LINUX_VERASE];
			out->sg_kill = in.c_cc[LINUX_VKILL];
			sgflag_linux_to_bsd(&in, &out->sg_flags);
			
			return IOCTL_HANDLED;
		}
		case BSD_TIOCSETP:
		{
			struct linux_termios out;
			const struct sgttyb* in = (struct sgttyb*) arg;

			// Get existing values so that we don't overwrite many
			// of the parameters not specified in struct sgttyb.
			__real_ioctl(fd, LINUX_TCGETS, &out);

			speed_bsd_to_linux(in->sg_ispeed, &out.c_ispeed);
			speed_bsd_to_linux(in->sg_ospeed, &out.c_ospeed);
			out.c_cc[LINUX_VERASE] = in->sg_erase;
			out.c_cc[LINUX_VKILL] = in->sg_kill;
			sgflag_bsd_to_linux(in->sg_flags, &out);

			*retval = __real_ioctl(fd, LINUX_TCSETS, &out);

			return IOCTL_HANDLED;
		}
		case BSD_TIOCGETC:
		{
			struct linux_termios in;
			struct tchars* out = (struct tchars*) arg;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in);

			out->t_intrc = in.c_cc[LINUX_VINTR];
			out->t_quitc = in.c_cc[LINUX_VQUIT];
			out->t_startc = in.c_cc[LINUX_VSTART];
			out->t_stopc = in.c_cc[LINUX_VSTOP];
			out->t_eofc = in.c_cc[LINUX_VEOF];
			out->t_brkc = in.c_cc[LINUX_VEOL2];

			return IOCTL_HANDLED;
		}
		case BSD_TIOCSETC:
		{
			struct linux_termios out;
			const struct tchars* in = (const struct tchars*) arg;

			// Get existing values so that we don't overwrite many
			// of the parameters not specified in struct tchars.
			__real_ioctl(fd, LINUX_TCGETS, &out);

			out.c_cc[LINUX_VINTR] = in->t_intrc;
			out.c_cc[LINUX_VQUIT] = in->t_quitc;
			out.c_cc[LINUX_VSTART] = in->t_startc;
			out.c_cc[LINUX_VSTOP] = in->t_stopc;
			out.c_cc[LINUX_VEOF] = in->t_eofc;
			out.c_cc[LINUX_VEOL2] = in->t_brkc;

			*retval = __real_ioctl(fd, LINUX_TCSETS, &out);

			return IOCTL_HANDLED;
		}
		case BSD_TIOCGLTC:
		{
			struct linux_termios in;
			struct ltchars* out = (struct ltchars*) arg;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in);

			out->t_suspc = in.c_cc[LINUX_VSUSP];
			out->t_dsuspc = in.c_cc[LINUX_VSUSP];
			out->t_rprntc = in.c_cc[LINUX_VREPRINT];
			out->t_flushc = in.c_cc[LINUX_VEOL2];
			out->t_werasc = in.c_cc[LINUX_VWERASE];
			out->t_lnextc = in.c_cc[LINUX_VLNEXT];

			return IOCTL_HANDLED;
		}
		case BSD_TIOCSLTC:
		{
			struct linux_termios out;
			const struct ltchars* in = (const struct ltchars*) arg;

			// Get existing values so that we don't overwrite many
			// of the parameters not specified in struct ltchars.
			__real_ioctl(fd, LINUX_TCGETS, &out);

			out.c_cc[LINUX_VSUSP] = in->t_suspc;
			out.c_cc[LINUX_VREPRINT] = in->t_rprntc;
			out.c_cc[LINUX_VEOL2] = in->t_flushc;
			out.c_cc[LINUX_VWERASE] = in->t_werasc;
			out.c_cc[LINUX_VLNEXT] = in->t_lnextc;

			*retval = __real_ioctl(fd, LINUX_TCSETS, &out);

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
		case BSD_TIOCSTOP:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCSTOP, NULL);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCSTART:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCSTART, NULL);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCIXON:
		case BSD_TIOCIXOFF:
		{
			struct linux_termios tos;
			unsigned char c;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &tos);
			if (*retval < 0)
				return IOCTL_HANDLED;

			c = tos.c_cc[cmd == BSD_TIOCIXON ? LINUX_VSTART : LINUX_VSTOP];
			if (c != LINUX_POSIX_VDISABLE)
				*retval = sys_write(fd, &c, 1);
			else
				*retval = 0;

			return IOCTL_HANDLED;
		}
		case BSD_TIOCSBRK:
		case BSD_TIOCCBRK:
		{
			*retval = __real_ioctl(fd,
					cmd == BSD_TIOCSBRK ? LINUX_TIOCSBRK : LINUX_TIOCCBRK,
					NULL);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCGPGRP:
		case BSD_TIOCSPGRP:
		{
			*retval = __real_ioctl(fd,
					cmd == BSD_TIOCGPGRP ? LINUX_TIOCGPGRP : LINUX_TIOCSPGRP,
					arg);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCGETD:
		case BSD_TIOCSETD:
		{
			*retval = __real_ioctl(fd,
					cmd == BSD_TIOCGETD ? LINUX_TIOCGETD : LINUX_TIOCSETD,
					arg);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCOUTQ:
		{
			*retval = __real_ioctl(fd, 0x00005411, arg);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCPTYUNLK:
		{
			int unlock = 0;
			*retval = __real_ioctl(fd, LINUX_TIOCSPTLCK, &unlock);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCPTYGNAME:
		{
			char* buf = (char*) arg; // 128-byte long buffer
			int ptyno;

			*retval = __real_ioctl(fd, LINUX_TIOCGPTN, &ptyno);
			if (*retval == 0)
			{
				__simple_sprintf(buf, "/dev/pts/%d", ptyno);
			}

			return IOCTL_HANDLED;
		}
		case BSD_TIOCPTYGRANT:
		{
			*retval = 0;
			return IOCTL_HANDLED;
		}
		case BSD_TIOCSCTTY:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCSCTTY, arg);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCCONS:
		{
			*retval = __real_ioctl(fd, LINUX_TIOCCONS, 0);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCFLUSH:
		{
			int queue;
			switch (*((int*) arg))
			{
				case BSD_FREAD:
					queue = LINUX_TCIFLUSH; break;
				case BSD_FWRITE:
					queue = LINUX_TCOFLUSH; break;
				case (BSD_FREAD | BSD_FWRITE):
					queue = LINUX_TCIOFLUSH; break;
				default:
					*retval = -EINVAL;
					return IOCTL_HANDLED;
			}

			*retval = __real_ioctl(fd, LINUX_TCFLSH, queue);
			return IOCTL_HANDLED;
		}
		case BSD_TIOCISATTY:
		{
			struct linux_termios in;

			*retval = __real_ioctl(fd, LINUX_TCGETS, &in) == 0;
			return IOCTL_HANDLED;
		}
		default:
		{
			__simple_printf("Passing thru unhandled ioctl 0x%x on fd %d\n", cmd, fd);
			return IOCTL_PASS;
		}
	}
}

inline static unsigned char c_l2b(unsigned char c)
{
	if (c == LINUX_POSIX_VDISABLE)
		return BSD_POSIX_VDISABLE;
	return c;
}

inline static unsigned char c_b2l(unsigned char c)
{
	if (c == BSD_POSIX_VDISABLE)
		return LINUX_POSIX_VDISABLE;
	return c;
}

static void cc_linux_to_bsd(const unsigned char* l, unsigned char* b)
{
	b[BSD_VEOF] = c_l2b(l[LINUX_VEOF]);
	b[BSD_VEOL] = c_l2b(l[LINUX_VEOL]);
	b[BSD_VEOL2] = c_l2b(l[LINUX_VEOL2]);
	b[BSD_VERASE] = c_l2b(l[LINUX_VERASE]);
	b[BSD_VWERASE] = c_l2b(l[LINUX_VWERASE]);
	b[BSD_VKILL] = c_l2b(l[LINUX_VKILL]);
	b[BSD_VREPRINT] = c_l2b(l[LINUX_VREPRINT]);
	b[BSD_VINTR] = c_l2b(l[LINUX_VINTR]);
	b[BSD_VQUIT] = c_l2b(l[LINUX_VQUIT]);
	b[BSD_VSUSP] = c_l2b(l[LINUX_VSUSP]);
	b[BSD_VSTART] = c_l2b(l[LINUX_VSTART]);
	b[BSD_VSTOP] = c_l2b(l[LINUX_VSTOP]);
	b[BSD_VLNEXT] = c_l2b(l[LINUX_VLNEXT]);
	b[BSD_VDISCARD] = c_l2b(l[LINUX_VDISCARD]);
	b[BSD_VMIN] = c_l2b(l[LINUX_VMIN]);
	b[BSD_VTIME] = c_l2b(l[LINUX_VTIME]);
}

static void cc_bsd_to_linux(const unsigned char* b, unsigned char* l)
{
	l[LINUX_VEOF] = c_b2l(b[BSD_VEOF]);
	l[LINUX_VEOL] = c_b2l(b[BSD_VEOL]);
	l[LINUX_VEOL2] = c_b2l(b[BSD_VEOL2]);
	l[LINUX_VERASE] = c_b2l(b[BSD_VERASE]);
	l[LINUX_VWERASE] = c_b2l(b[BSD_VWERASE]);
	l[LINUX_VKILL] = c_b2l(b[BSD_VKILL]);
	l[LINUX_VREPRINT] = c_b2l(b[BSD_VREPRINT]);
	l[LINUX_VINTR] = c_b2l(b[BSD_VINTR]);
	l[LINUX_VQUIT] = c_b2l(b[BSD_VQUIT]);
	l[LINUX_VSUSP] = c_b2l(b[BSD_VSUSP]);
	l[LINUX_VSTART] = c_b2l(b[BSD_VSTART]);
	l[LINUX_VSTOP] = c_b2l(b[BSD_VSTOP]);
	l[LINUX_VLNEXT] = c_b2l(b[BSD_VLNEXT]);
	l[LINUX_VDISCARD] = c_b2l(b[BSD_VDISCARD]);
	l[LINUX_VMIN] = c_b2l(b[BSD_VMIN]);
	l[LINUX_VTIME] = c_b2l(b[BSD_VTIME]);
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

static void oflag_linux_to_bsd(unsigned int l, unsigned long* b)
{
	if (l & LINUX_OPOST)
		*b |= BSD_OPOST;
	if (l & LINUX_ONLCR)
		*b |= BSD_ONLCR;
	if (l & LINUX_XTABS)
		*b |= BSD_OXTABS;
	//if (l & LINUX_ONOEOT)
	//	*b |= BSD_ONOEOT;
}

static void oflag_bsd_to_linux(unsigned long b, unsigned int* l)
{
	if (b & BSD_OPOST)
		*l |= LINUX_OPOST;
	if (b & BSD_ONLCR)
		*l |= LINUX_ONLCR;
	if (b & BSD_OXTABS)
		*l |= LINUX_XTABS;
	//if (b & BSD_ONOEOT)
	//	*l |= LINUX_ONOEOT;
}

static void cflag_linux_to_bsd(unsigned int l, unsigned long* b)
{
	if (l & LINUX_CSIZE)
		*b |= BSD_CSIZE;
	if (l & LINUX_CS6)
		*b |= BSD_CS6;
	if (l & LINUX_CS7)
		*b |= BSD_CS7;
	if (l & LINUX_CS8)
		*b |= BSD_CS8;
	if (l & LINUX_CSTOPB)
		*b |= BSD_CSTOPB;
	if (l & LINUX_CREAD)
		*b |= BSD_CREAD;
	if (l & LINUX_PARENB)
		*b |= BSD_PARENB;
	if (l & LINUX_PARODD)
		*b |= BSD_PARODD;
	if (l & LINUX_HUPCL)
		*b |= BSD_HUPCL;
	if (l & LINUX_CLOCAL)
		*b |= BSD_CLOCAL;
}

static void cflag_bsd_to_linux(unsigned long b, unsigned int* l)
{
	if (b & BSD_CSIZE)
		*l |= LINUX_CSIZE;
	if (b & BSD_CS6)
		*l |= LINUX_CS6;
	if (b & BSD_CS7)
		*l |= LINUX_CS7;
	if (b & BSD_CS8)
		*l |= LINUX_CS8;
	if (b & BSD_CSTOPB)
		*l |= LINUX_CSTOPB;
	if (b & BSD_CREAD)
		*l |= LINUX_CREAD;
	if (b & BSD_PARENB)
		*l |= LINUX_PARENB;
	if (b & BSD_PARODD)
		*l |= LINUX_PARODD;
	if (b & BSD_HUPCL)
		*l |= LINUX_HUPCL;
	if (b & BSD_CLOCAL)
		*l |= LINUX_CLOCAL;
}

static void lflag_linux_to_bsd(unsigned int l, unsigned long* b)
{
	if (l & LINUX_ISIG)
		*b |= BSD_ISIG;
	if (l & LINUX_ICANON)
		*b |= BSD_ICANON;
	if (l & LINUX_ECHO)
		*b |= BSD_ECHO;
	if (l & LINUX_ECHOE)
		*b |= BSD_ECHOE;
	if (l & LINUX_ECHOKE)
		*b |= BSD_ECHOKE;
	if (l & LINUX_ECHOK)
		*b |= BSD_ECHOK;
	if (l & LINUX_ECHONL)
		*b |= BSD_ECHONL;
	if (l & LINUX_NOFLSH)
		*b |= BSD_NOFLSH;
	if (l & LINUX_TOSTOP)
		*b |= BSD_TOSTOP;
	if (l & LINUX_ECHOCTL)
		*b |= BSD_ECHOCTL;
	if (l & LINUX_ECHOPRT)
		*b |= BSD_ECHOPRT;
	if (l & LINUX_FLUSHO)
		*b |= BSD_FLUSHO;
	if (l & LINUX_PENDIN)
		*b |= BSD_PENDIN;
	if (l & LINUX_IEXTEN)
		*b |= BSD_IEXTEN;
	if (l & LINUX_EXTPROC)
		*b |= BSD_EXTPROC;
}

static void lflag_bsd_to_linux(unsigned long b, unsigned int* l)
{
	if (b & BSD_ISIG)
		*l |= LINUX_ISIG;
	if (b & BSD_ICANON)
		*l |= LINUX_ICANON;
	if (b & BSD_ECHO)
		*l |= LINUX_ECHO;
	if (b & BSD_ECHOE)
		*l |= LINUX_ECHOE;
	if (b & BSD_ECHOKE)
		*l |= LINUX_ECHOKE;
	if (b & BSD_ECHOK)
		*l |= LINUX_ECHOK;
	if (b & BSD_ECHONL)
		*l |= LINUX_ECHONL;
	if (b & BSD_NOFLSH)
		*l |= LINUX_NOFLSH;
	if (b & BSD_TOSTOP)
		*l |= LINUX_TOSTOP;
	if (b & BSD_ECHOCTL)
		*l |= LINUX_ECHOCTL;
	if (b & BSD_ECHOPRT)
		*l |= LINUX_ECHOPRT;
	if (b & BSD_FLUSHO)
		*l |= LINUX_FLUSHO;
	if (b & BSD_PENDIN)
		*l |= LINUX_PENDIN;
	if (b & BSD_IEXTEN)
		*l |= LINUX_IEXTEN;
	if (b & BSD_EXTPROC)
		*l |= LINUX_EXTPROC;
}

static void speed_linux_to_bsd(unsigned int l, unsigned long* b)
{
	unsigned long bspeed;

	switch (l)
	{
		case LINUX_B0:
			bspeed = BSD_B0; break;
		case LINUX_B50:
			bspeed = BSD_B50; break;
		case LINUX_B75:
			bspeed = BSD_B75; break;
		case LINUX_B110:
			bspeed = BSD_B110; break;
		case LINUX_B134:
			bspeed = BSD_B134; break;
		case LINUX_B150:
			bspeed = BSD_B150; break;
		case LINUX_B200:
			bspeed = BSD_B200; break;
		case LINUX_B300:
			bspeed = BSD_B300; break;
		case LINUX_B600:
			bspeed = BSD_B600; break;
		case LINUX_B1200:
			bspeed = BSD_B1200; break;
		case LINUX_B1800:
			bspeed = BSD_B1800; break;
		case LINUX_B2400:
			bspeed = BSD_B2400; break;
		case LINUX_B4800:
			bspeed = BSD_B4800; break;
		case LINUX_B9600:
			bspeed = BSD_B9600; break;
		case LINUX_B19200:
			bspeed = BSD_B19200; break;
		case LINUX_B38400:
			bspeed = BSD_B38400; break;
		case LINUX_B57600:
			bspeed = BSD_B57600; break;
		case LINUX_B115200:
			bspeed = BSD_B115200; break;
		case LINUX_B230400:
			bspeed = BSD_B230400; break;
		default:
			bspeed = 0;
	}

	*b = bspeed;
}

static void speed_bsd_to_linux(unsigned long b,	unsigned int* l)
{
	unsigned int lspeed;

	switch (b)
	{
		case BSD_B0:
			lspeed = LINUX_B0; break;
		case BSD_B50:
			lspeed = LINUX_B50; break;
		case BSD_B75:
			lspeed = LINUX_B75; break;
		case BSD_B110:
			lspeed = LINUX_B110; break;
		case BSD_B134:
			lspeed = LINUX_B134; break;
		case BSD_B150:
			lspeed = LINUX_B150; break;
		case BSD_B200:
			lspeed = LINUX_B200; break;
		case BSD_B300:
			lspeed = LINUX_B300; break;
		case BSD_B600:
			lspeed = LINUX_B600; break;
		case BSD_B1200:
			lspeed = LINUX_B1200; break;
		case BSD_B1800:
			lspeed = LINUX_B1800; break;
		case BSD_B2400:
			lspeed = LINUX_B2400; break;
		case BSD_B4800:
		case BSD_B7200:
			lspeed = LINUX_B4800; break;
		case BSD_B28800:
		case BSD_B14400:
		case BSD_B9600:
			lspeed = LINUX_B9600; break;
		case BSD_B19200:
			lspeed = LINUX_B19200; break;
		case BSD_B38400:
			lspeed = LINUX_B38400; break;
		case BSD_B76800:
		case BSD_B57600:
			lspeed = LINUX_B57600; break;
		case BSD_B115200:
			lspeed = LINUX_B115200; break;
		case BSD_B230400:
			lspeed = LINUX_B230400; break;
		default:
			lspeed = 0;
	}

	*l = lspeed;
}

// Linux provides TIOCGETP only on some architectures
// and even there the support is very limited.
// See http://elixir.free-electrons.com/linux/latest/source/drivers/tty/tty_ioctl.c#L712
// Hence we do the translation ourselves.
static void sgflag_linux_to_bsd(const struct linux_termios* tos, short* sgflags)
{
	*sgflags = 0;

	if (!(tos->c_lflag & LINUX_ICANON))
	{
		if (tos->c_lflag & LINUX_ISIG)
			*sgflags |= 0x2; // cbreak
		else
			*sgflags |= 0x20; // raw
	}

	if (tos->c_lflag & LINUX_ECHO)
		*sgflags |= 0x8; // echo

	if (tos->c_oflag & LINUX_OPOST)
	{
		if (tos->c_oflag & LINUX_ONLCR)
			*sgflags |= 0x10; // crmod
	}
}

static void sgflag_bsd_to_linux(const short sgflags, struct linux_termios* tos)
{
	tos->c_iflag = LINUX_ICRNL | LINUX_IXON;
	tos->c_oflag = 0;
	tos->c_lflag = LINUX_ISIG | LINUX_ICANON;

	if (sgflags & 0x02) // cbreak
	{
		tos->c_iflag = 0;
		tos->c_lflag &= LINUX_ICANON;
	}

	if (sgflags & 0x08) // echo
	{
		tos->c_lflag |= LINUX_ECHO | LINUX_ECHOE | LINUX_ECHOK
			| LINUX_ECHOCTL | LINUX_ECHOKE | LINUX_IEXTEN;
	}

	if (sgflags & 0x10) // crmod
	{
		tos->c_oflag |= LINUX_OPOST | LINUX_ONLCR;
	}

	if (sgflags & 0x20) // raw
	{
		tos->c_iflag = 0;
		tos->c_lflag &= ~(LINUX_ISIG | LINUX_ICANON);
	}

	if (!(tos->c_lflag & LINUX_ICANON))
	{
		tos->c_cc[LINUX_VMIN] = 1;
		tos->c_cc[LINUX_VTIME] = 0;
	}
}

