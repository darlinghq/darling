#ifndef LIBC_AIO_H
#define LIBC_AIO_H
#include <aio.h>

extern "C"
{

#define DARWIN_LIO_NOWAIT              0x1
#define DARWIN_LIO_WAIT                0x2
#define DARWIN_LIO_NOP                 0x0
#define DARWIN_LIO_READ                0x1
#define DARWIN_LIO_WRITE               0x2
#define DARWIN_AIO_ALLDONE             0x1
#define DARWIN_AIO_CANCELED            0x2
#define DARWIN_AIO_NOTCANCELED         0x4

#ifndef DARWIN_O_SYNC
#define DARWIN_O_SYNC                  0x0080
#define DARWIN_O_DSYNC                 0x400000
#endif

int __darwin_aio_read(struct aiocb *aiocbp);
int __darwin_aio_write(struct aiocb *aiocbp);
int __darwin_aio_fsync(int op, struct aiocb *aiocbp); // flags
int __darwin_aio_error(const struct aiocb *aiocbp);
ssize_t __darwin_aio_return(struct aiocb *aiocbp);
int __darwin_aio_suspend(const struct aiocb * const aiocb_list[], int nitems, const struct timespec *timeout);
int __darwin_aio_cancel(int fd, struct aiocb *aiocbp); // flags
int __darwin_lio_listio(int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp); // flags

}

#endif

