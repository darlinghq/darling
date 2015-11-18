#ifndef EMU_TERMIOS_H
#define EMU_TERMIOS_H
#include "ioctl.h"

int handle_termios(int fd, unsigned int cmd, void* arg, int* retval);

struct bsd_winsize
{
	unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel;
};

struct bsd_termios
{
	unsigned long c_iflag;
	unsigned long c_oflag;
	unsigned long c_cflag;
	unsigned long c_lflag;
	unsigned char c_cc[20];
	unsigned long c_ispeed;
	unsigned long c_ospeed;
};
struct linux_termios
{
	unsigned int c_iflag;
	unsigned int c_oflag;
	unsigned int c_cflag;
	unsigned int c_lflag;
	unsigned char c_line;
	unsigned char c_cc[32];
	unsigned int c_ispeed;
	unsigned int c_ospeed;
};

// c_cc indices
enum {
	BSD_VEOF, BSD_VEOL, BSD_VEOL2, BSD_VERASE, BSD_VWERASE, BSD_VKILL,
	BSD_VREPRINT, BSD_VINTR, BSD_VQUIT, BSD_VSUSP, BSD_VDSUSP, BSD_VSTART,
	BSD_VSTOP, BSD_VLNEXT, BSD_VDISCARD, BSD_VMIN, BSD_VTIME, BSD_VSTATUS,
};
enum {
	LINUX_VINTR, LINUX_VQUIT, LINUX_VERASE, LINUX_VKILL, LINUX_VEOF,
	LINUX_VTIME, LINUX_VMIN, LINUX_VSWTC, LINUX_VSTART, LINUX_VSTOP,
	LINUX_VSUSP, LINUX_VEOL, LINUX_VREPRINT, LINUX_VDISCARD, LINUX_VWERASE,
	LINUX_VLNEXT, LINUX_VEOL2
};

#define BSD_TIOCMODG        BSD_IOR('t', 3, int)
#define BSD_TIOCMODS        BSD_IOW('t', 4, int)
#define         BSD_TIOCM_LE        0001
#define         BSD_TIOCM_DTR       0002
#define         BSD_TIOCM_RTS       0004
#define         BSD_TIOCM_ST        0010
#define         BSD_TIOCM_SR        0020
#define         BSD_TIOCM_CTS       0040
#define         BSD_TIOCM_CAR       0100
#define         BSD_TIOCM_CD        TIOCM_CAR
#define         BSD_TIOCM_RNG       0200
#define         BSD_TIOCM_RI        TIOCM_RNG
#define         BSD_TIOCM_DSR       0400

#define BSD_TIOCEXCL         BSD_IO('t', 13)
#define BSD_TIOCNXCL         BSD_IO('t', 14)

#define BSD_TIOCFLUSH       BSD_IOW('t', 16, int)

#define BSD_TIOCGETA        BSD_IOR('t', 19, struct bsd_termios)
#define BSD_TIOCSETA        BSD_IOW('t', 20, struct bsd_termios)
#define BSD_TIOCSETAW       BSD_IOW('t', 21, struct bsd_termios)
#define BSD_TIOCSETAF       BSD_IOW('t', 22, struct bsd_termios)
#define BSD_TIOCGETD        BSD_IOR('t', 26, int)
#define BSD_TIOCSETD        BSD_IOW('t', 27, int)
#define BSD_TIOCIXON         BSD_IO('t', 129)
#define BSD_TIOCIXOFF        BSD_IO('t', 128)

#define BSD_TIOCSBRK         BSD_IO('t', 123)
#define BSD_TIOCCBRK         BSD_IO('t', 122)
#define BSD_TIOCSDTR         BSD_IO('t', 121)
#define BSD_TIOCCDTR         BSD_IO('t', 120)
#define BSD_TIOCGPGRP       BSD_IOR('t', 119, int)
#define BSD_TIOCSPGRP       BSD_IOW('t', 118, int)

#define BSD_TIOCOUTQ        BSD_IOR('t', 115, int)
#define BSD_TIOCSTI         BSD_IOW('t', 114, char)
#define BSD_TIOCNOTTY        BSD_IO('t', 113)
#define BSD_TIOCPKT         BSD_IOW('t', 112, int)
#define         BSD_TIOCPKT_DATA            0x00
#define         BSD_TIOCPKT_FLUSHREAD       0x01
#define         BSD_TIOCPKT_FLUSHWRITE      0x02
#define         BSD_TIOCPKT_STOP            0x04
#define         BSD_TIOCPKT_START           0x08
#define         BSD_TIOCPKT_NOSTOP          0x10
#define         BSD_TIOCPKT_DOSTOP          0x20
#define         BSD_TIOCPKT_IOCTL           0x40
#define BSD_TIOCSTOP         BSD_IO('t', 111)
#define BSD_TIOCSTART        BSD_IO('t', 110)
#define BSD_TIOCMSET        BSD_IOW('t', 109, int)
#define BSD_TIOCMBIS        BSD_IOW('t', 108, int)
#define BSD_TIOCMBIC        BSD_IOW('t', 107, int)
#define BSD_TIOCMGET        BSD_IOR('t', 106, int)
#define BSD_TIOCREMOTE      BSD_IOW('t', 105, int)
#define BSD_TIOCGWINSZ      BSD_IOR('t', 104, struct bsd_winsize)
#define BSD_TIOCSWINSZ      BSD_IOW('t', 103, struct bsd_winsize)
#define BSD_TIOCUCNTL       BSD_IOW('t', 102, int)
#define BSD_TIOCSTAT         BSD_IO('t', 101)
#define         BSD_UIOCCMD(n)      BSD_IO('u', n)
#define BSD_TIOCSCONS       BSD_IO('t', 99)
#define BSD_TIOCCONS        BSD_IOW('t', 98, int)
#define BSD_TIOCSCTTY        BSD_IO('t', 97)
#define BSD_TIOCEXT         BSD_IOW('t', 96, int)
#define BSD_TIOCSIG          BSD_IO('t', 95)
#define BSD_TIOCDRAIN        BSD_IO('t', 94)
#define BSD_TIOCMSDTRWAIT   BSD_IOW('t', 91, int)
#define BSD_TIOCMGDTRWAIT   BSD_IOR('t', 90, int)
#define BSD_TIOCTIMESTAMP   BSD_IOR('t', 89, struct bsd_timeval)
#define BSD_TIOCDCDTIMESTAMP BSD_IOR('t', 88, struct bsd_timeval)
#define BSD_TIOCSDRAINWAIT  BSD_IOW('t', 87, int)
#define BSD_TIOCGDRAINWAIT  BSD_IOR('t', 86, int)
#define BSD_TIOCPTYGRANT    BSD_IO('t', 84)
#define BSD_TIOCPTYGNAME    BSD_IOC(IOC_OUT, 't', 83, 128)
#define BSD_TIOCPTYUNLK     BSD_IO('t', 82)

#define BSD_IGNBRK          0x00000001
#define BSD_BRKINT          0x00000002
#define BSD_IGNPAR          0x00000004
#define BSD_PARMRK          0x00000008
#define BSD_INPCK           0x00000010
#define BSD_ISTRIP          0x00000020
#define BSD_INLCR           0x00000040
#define BSD_IGNCR           0x00000080
#define BSD_ICRNL           0x00000100
#define BSD_IXON            0x00000200
#define BSD_IXOFF           0x00000400
#define BSD_IXANY           0x00000800
#define BSD_IMAXBEL         0x00002000
#define BSD_IUTF8           0x00004000

#define BSD_OPOST           0x00000001
#define BSD_ONLCR           0x00000002
#define BSD_OXTABS          0x00000004
#define BSD_ONOEOT          0x00000008

#define BSD_CIGNORE         0x00000001
#define BSD_CSIZE           0x00000300
#define     BSD_CS5             0x00000000
#define     BSD_CS6             0x00000100
#define     BSD_CS7             0x00000200
#define     BSD_CS8             0x00000300
#define BSD_CSTOPB          0x00000400
#define BSD_CREAD           0x00000800
#define BSD_PARENB          0x00001000
#define BSD_PARODD          0x00002000
#define BSD_HUPCL           0x00004000
#define BSD_CLOCAL          0x00008000
#define BSD_CCTS_OFLOW      0x00010000
#define BSD_CRTS_IFLOW      0x00020000
#define BSD_CDTR_IFLOW      0x00040000
#define BSD_CDSR_OFLOW      0x00080000
#define BSD_CCAR_OFLOW      0x00100000
#define BSD_MDMBUF          0x00100000


#define BSD_ECHOKE          0x00000001
#define BSD_ECHOE           0x00000002
#define BSD_ECHOK           0x00000004
#define BSD_ECHO            0x00000008
#define BSD_ECHONL          0x00000010
#define BSD_ECHOPRT         0x00000020
#define BSD_ECHOCTL         0x00000040
#define BSD_ISIG            0x00000080
#define BSD_ICANON          0x00000100
#define BSD_ALTWERASE       0x00000200
#define BSD_IEXTEN          0x00000400
#define BSD_EXTPROC         0x00000800
#define BSD_TOSTOP          0x00400000
#define BSD_FLUSHO          0x00800000
#define BSD_NOKERNINFO      0x02000000
#define BSD_PENDIN          0x20000000
#define BSD_NOFLSH          0x80000000

#define BSD_B0      0
#define BSD_B50     50
#define BSD_B75     75
#define BSD_B110    110
#define BSD_B134    134
#define BSD_B150    150
#define BSD_B200    200
#define BSD_B300    300
#define BSD_B600    600
#define BSD_B1200   1200
#define BSD_B1800   1800
#define BSD_B2400   2400
#define BSD_B4800   4800
#define BSD_B9600   9600
#define BSD_B19200  19200
#define BSD_B38400  38400
#define BSD_B7200   7200
#define BSD_B14400  14400
#define BSD_B28800  28800
#define BSD_B57600  57600
#define BSD_B76800  76800
#define BSD_B115200 115200
#define BSD_B230400 230400

#define LINUX_TCGETS          0x5401
#define LINUX_TCSETS          0x5402
#define LINUX_TCSETSW         0x5403
#define LINUX_TCSETSF         0x5404
#define LINUX_TCGETA          0x5405
#define LINUX_TCSETA          0x5406
#define LINUX_TCSETAW         0x5407
#define LINUX_TCSETAF         0x5408
#define LINUX_TCSBRK          0x5409
#define LINUX_TCXONC          0x540A
#define LINUX_TCFLSH          0x540B
#define LINUX_TIOCEXCL        0x540C
#define LINUX_TIOCNXCL        0x540D
#define LINUX_TIOCSCTTY       0x540E
#define LINUX_TIOCGPGRP       0x540F
#define LINUX_TIOCSPGRP       0x5410
#define LINUX_TIOCOUTQ        0x5411
#define LINUX_TIOCSTI         0x5412
#define LINUX_TIOCGWINSZ      0x5413
#define LINUX_TIOCSWINSZ      0x5414
#define LINUX_TIOCMGET        0x5415
#define LINUX_TIOCMBIS        0x5416
#define LINUX_TIOCMBIC        0x5417
#define LINUX_TIOCMSET        0x5418
#define LINUX_TIOCGSOFTCAR    0x5419
#define LINUX_TIOCSSOFTCAR    0x541A
#define LINUX_FIONREAD        0x541B
#define LINUX_TIOCINQ         LINUX_FIONREAD
#define LINUX_TIOCLINUX       0x541C
#define LINUX_TIOCCONS        0x541D
#define LINUX_TIOCGSERIAL     0x541E
#define LINUX_TIOCSSERIAL     0x541F
#define LINUX_TIOCPKT         0x5420
#define LINUX_FIONBIO         0x5421
#define LINUX_TIOCNOTTY       0x5422
#define LINUX_TIOCSETD        0x5423
#define LINUX_TIOCGETD        0x5424
#define LINUX_TIOCSBRK        0x5425
#define LINUX_TIOCCBRK        0x5426

#define LINUX_TIOCSTOP    LINUX_IO('t', 111)
#define LINUX_TIOCSTART   LINUX_IO('t', 110)

/* c_cc characters */
#define LINUX_VINTR 0
#define LINUX_VQUIT 1
#define LINUX_VERASE 2
#define LINUX_VKILL 3
#define LINUX_VEOF 4
#define LINUX_VTIME 5
#define LINUX_VMIN 6
#define LINUX_VSWTC 7
#define LINUX_VSTART 8
#define LINUX_VSTOP 9
#define LINUX_VSUSP 10
#define LINUX_VEOL 11
#define LINUX_VREPRINT 12
#define LINUX_VDISCARD 13
#define LINUX_VWERASE 14
#define LINUX_VLNEXT 15
#define LINUX_VEOL2 16

/* c_iflag bits */
#define LINUX_IGNBRK  0000001
#define LINUX_BRKINT  0000002
#define LINUX_IGNPAR  0000004
#define LINUX_PARMRK  0000010
#define LINUX_INPCK   0000020
#define LINUX_ISTRIP  0000040
#define LINUX_INLCR   0000100
#define LINUX_IGNCR   0000200
#define LINUX_ICRNL   0000400
#define LINUX_IUCLC   0001000
#define LINUX_IXON    0002000
#define LINUX_IXANY   0004000
#define LINUX_IXOFF   0010000
#define LINUX_IMAXBEL 0020000
#define LINUX_IUTF8   0040000

/* c_oflag bits */
#define LINUX_OPOST   0000001
#define LINUX_OLCUC   0000002
#define LINUX_ONLCR   0000004
#define LINUX_OCRNL   0000010
#define LINUX_ONOCR   0000020
#define LINUX_ONLRET  0000040
#define LINUX_OFILL   0000100
#define LINUX_OFDEL   0000200

#define LINUX_VTDLY   0040000
#define   LINUX_VT0   0000000
#define   LINUX_VT1   0040000

#define LINUX_XTABS  0014000

/* c_cflag bit meaning */
#define LINUX_CSIZE   0000060
#define   LINUX_CS5   0000000
#define   LINUX_CS6   0000020
#define   LINUX_CS7   0000040
#define   LINUX_CS8   0000060
#define LINUX_CSTOPB  0000100
#define LINUX_CREAD   0000200
#define LINUX_PARENB  0000400
#define LINUX_PARODD  0001000
#define LINUX_HUPCL   0002000
#define LINUX_CLOCAL  0004000

/* c_lflag bits */
#define LINUX_ISIG    0000001
#define LINUX_ICANON  0000002
#define LINUX_XCASE  0000004
#define LINUX_ECHO    0000010
#define LINUX_ECHOE   0000020
#define LINUX_ECHOK   0000040
#define LINUX_ECHONL  0000100
#define LINUX_NOFLSH  0000200
#define LINUX_TOSTOP  0000400

#define LINUX_ECHOCTL 0001000
#define LINUX_ECHOPRT 0002000
#define LINUX_ECHOKE  0004000
#define LINUX_FLUSHO  0010000
#define LINUX_PENDIN  0040000
#define LINUX_IEXTEN  0100000
#define LINUX_EXTPROC 0200000

/* Linux speed */
#define LINUX_CBAUD  0010017
#define LINUX_B0     0000000
#define LINUX_B50    0000001
#define LINUX_B75    0000002
#define LINUX_B110   0000003
#define LINUX_B134   0000004
#define LINUX_B150   0000005
#define LINUX_B200   0000006
#define LINUX_B300   0000007
#define LINUX_B600   0000010
#define LINUX_B1200  0000011
#define LINUX_B1800  0000012
#define LINUX_B2400  0000013
#define LINUX_B4800  0000014
#define LINUX_B9600  0000015
#define LINUX_B19200 0000016
#define LINUX_B38400 0000017

#define LINUX_CBAUDEX 0010000
#define LINUX_B57600   0010001
#define LINUX_B115200  0010002
#define LINUX_B230400  0010003

#define LINUX_POSIX_VDISABLE ((unsigned char)'\0')
#define BSD_POSIX_VDISABLE ((unsigned char)'\377')

#endif

