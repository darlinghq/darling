#include "aio.h"
#include "errno.h"
#include "common/path.h"
#include "common/auto.h"

MAP_FUNCTION1(int,aio_read,struct aiocb*);
MAP_FUNCTION1(int,aio_write,struct aiocb*);

int __darwin_aio_fsync(int op, struct aiocb *aiocbp)
{
	// TODO
}

int __darwin_aio_error(const struct aiocb *aiocbp)
{
	int rv = aio_error(aiocbp);
	if (rv > 0)
		rv = errnoLinuxToDarwin(rv);
	else if (rv == -1)
		errnoOut();
	return rv;
}

MAP_FUNCTION1(ssize_t,aio_return,struct aiocb*);
MAP_FUNCTION3(int,aio_suspend,const struct aiocb * const*,int,const struct timespec *);

int __darwin_aio_cancel(int fd, struct aiocb *aiocbp)
{
	// TODO
}

int __darwin_lio_listio(int mode, struct aiocb *const aiocb_list[], int nitems, struct sigevent *sevp)
{
	// TODO
}

