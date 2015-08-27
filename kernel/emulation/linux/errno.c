#include "errno.h"
#include "duct_errno.h"

static int linux_to_darwin[512];

__attribute__((constructor)) void __init_errno_map(void)
{
	linux_to_darwin[LINUX_EPERM] = EPERM;
	linux_to_darwin[LINUX_ENOENT] = ENOENT;
	linux_to_darwin[LINUX_ESRCH] = ESRCH;
	linux_to_darwin[LINUX_EINTR] = EINTR;
	linux_to_darwin[LINUX_EIO] = EIO;
	linux_to_darwin[LINUX_ENXIO] = ENXIO;
	linux_to_darwin[LINUX_ENOEXEC] = ENOEXEC;
	linux_to_darwin[LINUX_EBADF] = EBADF;
	linux_to_darwin[LINUX_ECHILD] = ECHILD;
	linux_to_darwin[LINUX_EDEADLK] = EDEADLK;
	linux_to_darwin[LINUX_ENOMEM] = ENOMEM;
	linux_to_darwin[LINUX_EACCES] = EACCES;
	linux_to_darwin[LINUX_EFAULT] = EFAULT;
	linux_to_darwin[LINUX_ENOTBLK] = ENOTBLK;
	linux_to_darwin[LINUX_EBUSY] = EBUSY;
	linux_to_darwin[LINUX_EEXIST] = EEXIST;
	linux_to_darwin[LINUX_EXDEV] = EXDEV;
	linux_to_darwin[LINUX_ENODEV] = ENODEV;
	linux_to_darwin[LINUX_ENOTDIR] = ENOTDIR;
	linux_to_darwin[LINUX_EISDIR] = EISDIR;
	linux_to_darwin[LINUX_EINVAL] = EINVAL;
	linux_to_darwin[LINUX_ENFILE] = ENFILE;
	linux_to_darwin[LINUX_EMFILE] = EMFILE;
	linux_to_darwin[LINUX_ENOTTY] = ENOTTY;
	linux_to_darwin[LINUX_ETXTBSY] = ETXTBSY;
	linux_to_darwin[LINUX_EFBIG] = EFBIG;
	linux_to_darwin[LINUX_ENOSPC] = ENOSPC;
	linux_to_darwin[LINUX_ESPIPE] = ESPIPE;
	linux_to_darwin[LINUX_EROFS] = EROFS;
	linux_to_darwin[LINUX_EMLINK] = EMLINK;
	linux_to_darwin[LINUX_EPIPE] = EPIPE;
	linux_to_darwin[LINUX_EDOM] = EDOM;
	linux_to_darwin[LINUX_ERANGE] = ERANGE;
	linux_to_darwin[LINUX_EAGAIN] = EAGAIN;
	linux_to_darwin[LINUX_EINPROGRESS] = EINPROGRESS;
	linux_to_darwin[LINUX_EALREADY] = EALREADY;
	linux_to_darwin[LINUX_ENOTSOCK] = ENOTSOCK;
	linux_to_darwin[LINUX_EDESTADDRREQ] = EDESTADDRREQ;
	linux_to_darwin[LINUX_EMSGSIZE] = EMSGSIZE;
	linux_to_darwin[LINUX_EPROTOTYPE] = EPROTOTYPE;
	linux_to_darwin[LINUX_ENOPROTOOPT] = ENOPROTOOPT;
	linux_to_darwin[LINUX_EPROTONOSUPPORT] = EPROTONOSUPPORT;
	linux_to_darwin[LINUX_ESOCKTNOSUPPORT] = ESOCKTNOSUPPORT;
	linux_to_darwin[LINUX_ENOTSUP] = ENOTSUP;
	linux_to_darwin[LINUX_EPFNOSUPPORT] = EPFNOSUPPORT;
	linux_to_darwin[LINUX_EAFNOSUPPORT] = EAFNOSUPPORT;
	linux_to_darwin[LINUX_EADDRINUSE] = EADDRINUSE;
	linux_to_darwin[LINUX_EADDRNOTAVAIL] = EADDRNOTAVAIL;
	linux_to_darwin[LINUX_ENETDOWN] = ENETDOWN;
	linux_to_darwin[LINUX_ENETUNREACH] = ENETUNREACH;
	linux_to_darwin[LINUX_ENETRESET] = ENETRESET;
	linux_to_darwin[LINUX_ECONNABORTED] = ECONNABORTED;
	linux_to_darwin[LINUX_ECONNRESET] = ECONNRESET;
	linux_to_darwin[LINUX_ENOBUFS] = ENOBUFS;
	linux_to_darwin[LINUX_EISCONN] = EISCONN;
	linux_to_darwin[LINUX_ENOTCONN] = ENOTCONN;
	linux_to_darwin[LINUX_ESHUTDOWN] = ESHUTDOWN;
	linux_to_darwin[LINUX_ETOOMANYREFS] = ETOOMANYREFS;
	linux_to_darwin[LINUX_ETIMEDOUT] = ETIMEDOUT;
	linux_to_darwin[LINUX_ECONNREFUSED] = ECONNREFUSED;
	linux_to_darwin[LINUX_ELOOP] = ELOOP;
	linux_to_darwin[LINUX_ENAMETOOLONG] = ENAMETOOLONG;
	linux_to_darwin[LINUX_EHOSTDOWN] = EHOSTDOWN;
	linux_to_darwin[LINUX_EHOSTUNREACH] = EHOSTUNREACH;
	linux_to_darwin[LINUX_ENOTEMPTY] = ENOTEMPTY;
	linux_to_darwin[LINUX_EUSERS] = EUSERS;
	linux_to_darwin[LINUX_EDQUOT] = EDQUOT;
	linux_to_darwin[LINUX_ESTALE] = ESTALE;
	linux_to_darwin[LINUX_EREMOTE] = EREMOTE;
	linux_to_darwin[LINUX_ENOLCK] = ENOLCK;
	linux_to_darwin[LINUX_ENOSYS] = ENOSYS;
	linux_to_darwin[LINUX_EOVERFLOW] = EOVERFLOW;
	linux_to_darwin[LINUX_ECANCELED] = ECANCELED;
	linux_to_darwin[LINUX_EIDRM] = EIDRM;
	linux_to_darwin[LINUX_ENOMSG] = ENOMSG;
	linux_to_darwin[LINUX_EILSEQ] = EILSEQ;
	linux_to_darwin[LINUX_EBADMSG] = EBADMSG;
	linux_to_darwin[LINUX_EMULTIHOP] = EMULTIHOP;
	linux_to_darwin[LINUX_ENODATA] = ENODATA;
	linux_to_darwin[LINUX_ENOLINK] = ENOLINK;
	linux_to_darwin[LINUX_ENOSR] = ENOSR;
	linux_to_darwin[LINUX_ENOSTR] = ENOSTR;
	linux_to_darwin[LINUX_EPROTO] = EPROTO;
	linux_to_darwin[LINUX_ETIME] = ETIME;
	linux_to_darwin[LINUX_EOPNOTSUPP] = EOPNOTSUPP;
}

int errno_linux_to_bsd(int err)
{
	int v = err;
	if (v < 0)
		v = -v;

	if (linux_to_darwin[v])
	{
		v = linux_to_darwin[v];
		return (err < 0) ? -v : v;
	}

	return err;
}

