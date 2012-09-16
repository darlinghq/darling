#ifndef LIBC_TERMIOS_H
#define LIBC_TERMIOS_H
#include <termios.h>

extern "C"
{

#define DARWIN_NCCS 20
struct __darwin_termios
{
	long c_iflag; // 4 on Linux, 4/8 on BSD
	long c_oflag;
	long c_cflag;
	long c_lflag;
	cc_t c_line; // 1
	cc_t c_cc[DARWIN_NCCS]; // 32 on Linux, 20 on BSD
	long c_ispeed; // 4 on Linux, 4/8 on BSD
	long c_ospeed;
};

int __darwin_tcgetattr(int fd, struct __darwin_termios *termios_p);
int __darwin_tcsetattr(int fd, int optional_actions, const struct __darwin_termios *termios_p);
void __darwin_cfmakeraw(struct __darwin_termios *termios_p);
long __darwin_cfgetispeed(const struct __darwin_termios *termios_p);
long __darwin_cfgetospeed(const struct __darwin_termios *termios_p);
int __darwin_cfsetispeed(struct __darwin_termios *termios_p, long speed);
int __darwin_cfsetospeed(struct __darwin_termios *termios_p, long speed);
int __darwin_cfsetspeed(struct __darwin_termios *termios_p, long speed);

int __darwin_tcflush(int fd, int queue_selector);
int __darwin_tcflow(int fd, int action);

// cc_t
#define DARWIN_VEOF 0
#define DARWIN_VEOL 1
#define DARWIN_VEOL2 2
#define DARWIN_VERASE 3
#define DARWIN_VWERASE 4
#define DARWIN_VKILL 5
#define DARWIN_VREPRINT 6
#define DARWIN_VINTR 8
#define DARWIN_VQUIT 9
#define DARWIN_VSUSP 10
// #define DARWIN_VDSUSP 11
#define DARWIN_VSTART 12
#define DARWIN_VSTOP 13
#define DARWIN_VLNEXT 14
#define DARWIN_VDISCARD 15
#define DARWIN_VMIN 16
#define DARWIN_VTIME 17
// #define DARWIN_VSTATUS 19
// #define DARWIN_NCCS 20

// input flags
#define DARWIN_IGNBRK 0x1
#define DARWIN_BRKINT 0x2
#define DARWIN_IGNPAR 0x4
#define DARWIN_PARMRK 0x8
#define DARWIN_INPCL 0x10
#define DARWIN_ISTRIP 0x20
#define DARWIN_INLCR 0x80
#define DARWIN_ICRNL 0x100
#define DARWIN_IXON 0x200
#define DARWIN_IXOFF 0x400
#define DARWIN_IXANZ 0x800
#define DARWIN_IMAXBEL 0x2000
#define DARWIN_IUTF8 0x4000

// output flags
#define DARWIN_OPOST 0x1
#define DARWIN_ONLCR 0x2
#define DARWIN_OXTABS 0x4
#define DARWIN_ONOEOT 0x8

// control flags
#define DARWIN_XIGNORE 0x1
#define DARWIN_CSIZE 0x300
#define DARWIN_CS5 0
#define DARWIN_CS6 0x100
#define DARWIN_CS7 0x200
#define DARWIN_CS8 0x300
#define DARWIN_CSTOPB 0x400
#define DARWIN_CREAD 0x800
#define DARWIN_PARENB 0x1000
#define DARWIN_PARODD 0x2000
#define DARWIN_HUPCL 0x4000
#define DARWIN_CLOCAL 0x8000
#define DARWIN_CCTS_OFLOW 0x10000
#define DARWIN_CRTS_IFLOW 0x20000
#define DARWIN_CDTR_IFLOW 0x40000
#define DARWIN_CDSR_OFLOW 0x80000
#define DARWIN_CCAR_OFLOW 0x100000

// local flags
#define DARWIN_ECHOKE 0x1
#define DARWIN_ECHOE 0x2
#define DARWIN_ECHOK 0x4
#define DARWIN_ECHO 0x8
#define DARWIN_ECHONL 0x10
#define DARWIN_ECHOPTR 0x20
#define DARWIN_ECHOCTL 0x40
#define DARWIN_ISIG 0x80
#define DARWIN_ICANON 0x100
#define DARWIN_ALTWERASE 0x200
#define DARWIN_IEXTEN 0x400
#define DARWIN_EXTPROC 0x800
#define DARWIN_TOSTOP 0x400000
#define DARWIN_FLUSHO 0x800000
//#define DARWIN_NOKERNINFO 0x2000000
#define DARWIN_PENDIN 0x20000000
#define DARWIN_NOFLSH 0x80000000

}

#endif

