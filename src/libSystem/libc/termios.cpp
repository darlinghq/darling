#include "termios.h"
#include "errno.h"
#include "common/auto.h"
#include <stdint.h>
#include <cstring>

const static uint8_t g_ccIndices[][2] = {
	{DARWIN_VEOF, VEOF}, {DARWIN_VEOL, VEOL}, {DARWIN_VEOL2, VEOL2}, {DARWIN_VERASE, VERASE},  {DARWIN_VWERASE, VWERASE},
	{DARWIN_VKILL, VKILL}, {DARWIN_VREPRINT, VREPRINT}, {DARWIN_VINTR,VINTR}, {DARWIN_VQUIT, VQUIT}, {DARWIN_VSUSP, VSUSP},
	{DARWIN_VSTART, VSTART}, {DARWIN_VSTOP, VSTOP}, {DARWIN_VLNEXT, VLNEXT}, {DARWIN_VDISCARD, VDISCARD}, {DARWIN_VMIN, VMIN},
	{DARWIN_VTIME, VTIME}
};

const static Darling::MappedFlag g_inputFlags[] = {
	{DARWIN_IGNBRK, IGNBRK}, {DARWIN_BRKINT, BRKINT}, {DARWIN_IGNPAR, IGNPAR}, {DARWIN_PARMRK, PARMRK},
	/*{DARWIN_INPCL, INPCL},*/ {DARWIN_ISTRIP, ISTRIP}, {DARWIN_INLCR, INLCR}, {DARWIN_ICRNL, ICRNL}, {DARWIN_IXON, IXON},
	{DARWIN_IXOFF, IXOFF}, /*{DARWIN_IXANZ, IXANZ},*/ {DARWIN_IMAXBEL, IMAXBEL}, {DARWIN_IUTF8, IUTF8}
};

const static Darling::MappedFlag g_outputFlags[] = {
	{DARWIN_OPOST, OPOST}, {DARWIN_ONLCR, ONLCR}, /*{DARWIN_OXTABS, OXTABS},*/ /*{DARWIN_ONOEOT, ONOEOT} */
};

const static Darling::MappedFlag g_controlFlags[] = {
	/*{DARWIN_XIGNORE, XIGNORE}, */ {DARWIN_CSIZE, CSIZE}, {DARWIN_CS5, CS5}, {DARWIN_CS6, CS6}, {DARWIN_CS7, CS7},
	{DARWIN_CS8, CS8}, {DARWIN_CSTOPB, CSTOPB}, {DARWIN_CREAD, CREAD}, {DARWIN_PARENB, PARENB}, {DARWIN_PARODD, PARODD},
	{DARWIN_HUPCL, HUPCL}, {DARWIN_CLOCAL, CLOCAL}, /*{DARWIN_CCTS_OFLOW, CCTS_OFLOW}, {DARWIN_CRTS_IFLOW, CRTS_IFLOW},
	{DARWIN_CDTR_IFLOW, CDTR_IFLOW}, {DARWIN_CDSR_OFLOW, CDSR_OFLOW}, {DARWIN_CCAR_OFLOW, CCAR_OFLOW}*/
};

const static Darling::MappedFlag g_localFlags[] = {
	{DARWIN_ECHOKE, ECHOKE}, {DARWIN_ECHOE, ECHOE}, {DARWIN_ECHOK, ECHOK}, {DARWIN_ECHO, ECHO}, {DARWIN_ECHONL, ECHONL},
	/* {DARWIN_ECHOPTR, ECHOPTR}, */ {DARWIN_ECHOCTL, ECHOCTL}, {DARWIN_ISIG, ISIG}, {DARWIN_ICANON, ICANON},
	/* {DARWIN_ALTWERASE, ALTWERASE}, */ {DARWIN_IEXTEN, IEXTEN}, {DARWIN_EXTPROC, EXTPROC}, {DARWIN_TOSTOP, TOSTOP},
	{DARWIN_FLUSHO, FLUSHO}, {DARWIN_PENDIN, PENDIN}, {int(DARWIN_NOFLSH), NOFLSH}
};

static void termiosDarwinToLinux(const struct __darwin_termios* p, struct termios* n)
{
	memset(n, 0, sizeof(*n));
	cfsetispeed(n, p->c_ispeed);
	cfsetospeed(n, p->c_ospeed);
	//n->c_line = p->c_line;

	memset(n->c_cc, 0, sizeof(n->c_cc));

	for (int i = 0; i < sizeof(g_ccIndices) / sizeof(g_ccIndices[0]); i++)
		n->c_cc[g_ccIndices[i][1]] = p->c_cc[g_ccIndices[i][0]];

	n->c_oflag = Darling::flagsDarwinToNative(g_outputFlags, sizeof(g_outputFlags)/sizeof(g_outputFlags[0]), p->c_oflag);
	n->c_iflag = Darling::flagsDarwinToNative(g_inputFlags, sizeof(g_inputFlags)/sizeof(g_inputFlags[0]), p->c_iflag);
	n->c_cflag = Darling::flagsDarwinToNative(g_controlFlags, sizeof(g_controlFlags)/sizeof(g_controlFlags[0]), p->c_cflag);
	n->c_lflag = Darling::flagsDarwinToNative(g_localFlags, sizeof(g_localFlags)/sizeof(g_localFlags[0]), p->c_lflag);
}

static void termiosLinuxToDarwin(const struct termios* p, struct __darwin_termios* d)
{
	memset(d, 0, sizeof(*d));
	d->c_ispeed = cfgetispeed(p);
	d->c_ospeed = cfgetospeed(p);
	//d->c_line = p->c_line;

	memset(d->c_cc, 0, sizeof(d->c_cc));

	for (int i = 0; i < sizeof(g_ccIndices) / sizeof(g_ccIndices[0]); i++)
		d->c_cc[g_ccIndices[i][0]] = p->c_cc[g_ccIndices[i][1]];

	d->c_oflag = Darling::flagsNativeToDarwin(g_outputFlags, sizeof(g_outputFlags)/sizeof(g_outputFlags[0]), p->c_oflag);
	d->c_iflag = Darling::flagsNativeToDarwin(g_inputFlags, sizeof(g_inputFlags)/sizeof(g_inputFlags[0]), p->c_iflag);
	d->c_cflag = Darling::flagsNativeToDarwin(g_controlFlags, sizeof(g_controlFlags)/sizeof(g_controlFlags[0]), p->c_cflag);
	d->c_lflag = Darling::flagsNativeToDarwin(g_localFlags, sizeof(g_localFlags)/sizeof(g_localFlags[0]), p->c_lflag);
}

int __darwin_tcgetattr(int fd, struct __darwin_termios *termios_p)
{
	struct termios n;
	int rv = tcgetattr(fd, &n);

	if (rv == -1)
		errnoOut();
	else
		termiosLinuxToDarwin(&n, termios_p);
	return rv;
}

int __darwin_tcsetattr(int fd, int optional_actions, const struct __darwin_termios *termios_p)
{
	struct termios n;
	termiosDarwinToLinux(termios_p, &n);

	int rv = tcsetattr(fd, optional_actions, &n);
	if (rv == -1)
		errnoOut();
	return rv;
}

void __darwin_cfmakeraw(struct __darwin_termios *termios_p)
{
	// from man termios(3)
	termios_p->c_iflag &= ~(DARWIN_IGNBRK | DARWIN_BRKINT | DARWIN_PARMRK | DARWIN_ISTRIP | DARWIN_INLCR | /*DARWIN_IGNCR |*/ DARWIN_ICRNL | DARWIN_IXON);
	termios_p->c_oflag &= ~DARWIN_OPOST;
	termios_p->c_lflag &= ~(DARWIN_ECHO | DARWIN_ECHONL | DARWIN_ICANON | DARWIN_ISIG | DARWIN_IEXTEN);
	termios_p->c_cflag &= ~(DARWIN_CSIZE | DARWIN_PARENB);
	termios_p->c_cflag |= DARWIN_CS8;
}

long __darwin_cfgetispeed(const struct __darwin_termios *termios_p)
{
	return termios_p->c_ispeed;
}

long __darwin_cfgetospeed(const struct __darwin_termios *termios_p)
{
	return termios_p->c_ospeed;
}

int __darwin_cfsetispeed(struct __darwin_termios *termios_p, long speed)
{
	termios_p->c_ispeed = speed;
	return 0;
}

int __darwin_cfsetospeed(struct __darwin_termios *termios_p, long speed)
{
	termios_p->c_ospeed = speed;
	return 0;
}

int __darwin_cfsetspeed(struct __darwin_termios *termios_p, long speed)
{
	termios_p->c_ospeed = termios_p->c_ispeed = speed;
	return 0;
}

int __darwin_tcflush(int fd, int queue_selector)
{
	int rv = tcflush(fd, queue_selector-1);
	if (rv == -1)
		errnoOut();
	return rv;
}

int __darwin_tcflow(int fd, int action)
{
	int rv = tcflow(fd, action-1);
	if (rv == -1)
		errnoOut();
	return rv;
}

