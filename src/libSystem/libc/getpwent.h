#ifndef LIBC_GETPWENT_H
#define LIBC_GETPWENT_H
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct __darwin_passwd
{
	const char* pw_name;
	const char* pw_passwd;
	uid_t pw_uid;
	gid_t pw_gid;
	time_t pw_change;
	const char* pw_class;
	const char* pw_gecos;
	const char* pw_dir;
	const char* pw_shell;
	time_t pw_expire;
};

__darwin_passwd* __darwin_getpwent();
__darwin_passwd* __darwin_getpwnam(const char* login);
int __darwin_getpwnam_r(const char *name, struct __darwin_passwd *pwd, char *buffer, size_t bufsize, struct __darwin_passwd **result);
__darwin_passwd* __darwin_getpwuid(uid_t uid);
int __darwin_getpwuid_r(uid_t uid, struct __darwin_passwd *pwd, char *buffer, size_t bufsize, struct __darwin_passwd **result);
int __darwin_getpwuuid(/* ... */);
int __darwin_getpwuuid_r(/* ... */);
int __darwin_setpassent(int stayopen);

// not needed, compatible
// void __darwin_setpwent();
// void __darwin_endpwent();

#ifdef __cplusplus
}
#endif


#endif

