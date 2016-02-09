#ifndef EMU_IOCTL_H
#define EMU_IOCTL_H
#include <stdint.h>

extern long __real_ioctl(int fd, int cmd, void* arg);

enum {
	IOCTL_PASS,			/** ioctl cmd not known */
	IOCTL_HANDLED		/** ioctl cmd now already handled */
};


#define BSD_IOCPARM_MASK    0x1fff
#define BSD_IOCPARM_LEN(x)  (((x) >> 16) & BSD_IOCPARM_MASK)
#define BSD_IOCBASECMD(x)   ((x) & ~(IBSD_OCPARM_MASK << 16))
#define BSD_IOCGROUP(x)     (((x) >> 8) & 0xff)

#define BSD_IOCPARM_MAX     (BSD_IOCPARM_MASK + 1)
#define BSD_IOC_VOID        (uint32_t)0x20000000
#define BSD_IOC_OUT         (uint32_t)0x40000000
#define BSD_IOC_IN          (uint32_t)0x80000000
#define BSD_IOC_INOUT       (BSD_IOC_IN|BSD_IOC_OUT)
#define BSD_IOC_DIRMASK     (uint32_t)0xe0000000

#define BSD_IOC(inout,group,num,len) \
        (int)(inout | ((len & BSD_IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define BSD_IO(g,n)        BSD_IOC(BSD_IOC_VOID,  (g), (n), 0)
#define BSD_IOR(g,n,t)     BSD_IOC(BSD_IOC_OUT,   (g), (n), sizeof(t))
#define BSD_IOW(g,n,t)     BSD_IOC(BSD_IOC_IN,    (g), (n), sizeof(t))
#define BSD_IOWR(g,n,t)    BSD_IOC(BSD_IOC_INOUT, (g), (n), sizeof(t))


#define BSD_FIOCLEX          BSD_IO('f', 1)
#define BSD_FIONCLEX         BSD_IO('f', 2)
#define BSD_FIONREAD        BSD_IOR('f', 127, int)
#define BSD_FIONBIO         BSD_IOW('f', 126, int)
#define BSD_FIOASYNC        BSD_IOW('f', 125, int)
#define BSD_FIOSETOWN       BSD_IOW('f', 124, int)
#define BSD_FIOGETOWN       BSD_IOR('f', 123, int)
#define BSD_FIODTYPE        BSD_IOR('f', 122, int)

#define LINUX_IOC_NONE      0U
#define LINUX_IOC_WRITE     1U
#define LINUX_IOC_READ      2U
#define LINUX_IOC_NRBITS	8
#define LINUX_IOC_TYPEBITS	8
#define LINUX_IOC_SIZEBITS	14
#define LINUX_IOC_DIRBITS	2

#define LINUX_IOC_NRSHIFT    0
#define LINUX_IOC_TYPESHIFT  (LINUX_IOC_NRSHIFT+LINUX_IOC_NRBITS)
#define LINUX_IOC_SIZESHIFT  (LINUX_IOC_TYPESHIFT+LINUX_IOC_TYPEBITS)
#define LINUX_IOC_DIRSHIFT   (LINUX_IOC_SIZESHIFT+LINUX_IOC_SIZEBITS)

#define LINUX_IOC(dir,type,nr,size) \
        (((dir)  << LINUX_IOC_DIRSHIFT) | \
         ((type) << LINUX_IOC_TYPESHIFT) | \
         ((nr)   << LINUX_IOC_NRSHIFT) | \
         ((size) << LINUX_IOC_SIZESHIFT))

#define LINUX_IOC_TYPECHECK(t) (sizeof(t))

/* used to create numbers */
#define LINUX_IO(type,nr)            LINUX_IOC(LINUX_IOC_NONE,(type),(nr),0)
#define LINUX_IOR(type,nr,size)      LINUX_IOC(LINUX_IOC_READ,(type),(nr),(LINUX_IOC_TYPECHECK(size)))
#define LINUX_IOW(type,nr,size)      LINUX_IOC(LINUX_IOC_WRITE,(type),(nr),(LINUX_IOC_TYPECHECK(size)))
#define LINUX_IOWR(type,nr,size)     LINUX_IOC(LINUX_IOC_READ|LINUX_IOC_WRITE,(type),(nr),(LINUX_IOC_TYPECHECK(size)))
#define LINUX_IOR_BAD(type,nr,size)  _IOC(LINUX_IOC_READ,(type),(nr),sizeof(size))
#define LINUX_IOW_BAD(type,nr,size)  _IOC(LINUX_IOC_WRITE,(type),(nr),sizeof(size))
#define LINUX_IOWR_BAD(type,nr,size) _IOC(LINUX_IOC_READ|LINUX_IOC_WRITE,(type),(nr),sizeof(size))


#endif

