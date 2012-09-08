#include "config.h"
#include "getpwent.h"
#include "darwin_errno_codes.h"
#include <pwd.h>
#include <cstring>
#include <errno.h>
#include <cstdlib>
#include <iostream>
#include "errno.h"

static void passwdLinuxToDarwin(const struct passwd* in, __darwin_passwd* out)
{
	memset(out, 0, sizeof(*out));
	out->pw_name = in->pw_name;
	out->pw_passwd = in->pw_passwd;
	out->pw_uid = in->pw_uid;
	out->pw_gid = in->pw_gid;
	out->pw_gecos = in->pw_gecos;
	out->pw_dir = in->pw_dir;
	out->pw_shell = in->pw_shell;
}

static __darwin_passwd* passwdLinuxToDarwin(const struct passwd* in)
{
	static __thread __darwin_passwd pwd;
	passwdLinuxToDarwin(in, &pwd);
	return &pwd;
}

__darwin_passwd* __darwin_getpwent()
{
	return passwdLinuxToDarwin(getpwent());
}

__darwin_passwd* __darwin_getpwnam(const char* login)
{
	return passwdLinuxToDarwin(getpwnam(login));
}

template<typename Func, typename Param> int GenericGet_R(Func f, Param p, struct __darwin_passwd *pwd,
		char *buffer, size_t bufsize, struct __darwin_passwd **result)
{
	static_assert(sizeof(struct __darwin_passwd) >= sizeof(struct passwd), "struct passwd is too small on this platform!");

	int rv = f(p, reinterpret_cast<struct passwd*>(pwd), buffer, bufsize, reinterpret_cast<struct passwd**>(result));
	if (rv != 0)
	{
		return errnoLinuxToDarwin(rv);
	}
	else
	{
		if (*result)
		{
			struct passwd l;
			memcpy(&l, *result, sizeof(l));
			passwdLinuxToDarwin(&l, *result);
		}

		return 0;
	}
}

int __darwin_getpwnam_r(const char *name, struct __darwin_passwd *pwd, char *buffer, size_t bufsize, struct __darwin_passwd **result)
{
	return GenericGet_R(getpwnam_r, name, pwd, buffer, bufsize, result);
}

__darwin_passwd* __darwin_getpwuid(uid_t uid)
{
	return passwdLinuxToDarwin(getpwuid(uid));
}

int __darwin_getpwuid_r(uid_t uid, struct __darwin_passwd *pwd, char *buffer, size_t bufsize, struct __darwin_passwd **result)
{
	return GenericGet_R(getpwuid_r, uid, pwd, buffer, bufsize, result);
}

int __darwin_getpwuuid(/* ... */)
{
	errno = DARWIN_ENOSYS;
	return -1;
}

int __darwin_getpwuuid_r(/* ... */)
{
	errno = DARWIN_ENOSYS;
	return -1;
}

int __darwin_setpassent(int stayopen);


