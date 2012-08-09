#include "config.h"
#include "dir.h"
#include "trace.h"
#include "errno.h"
#include "util/trace.h"
#include "common/auto.h"
#include "common/path.h"
#include <cstdlib>

static darwin_dirent* convertDirent(const struct dirent* ent);
static darwin_dirent64* convertDirent64(const struct dirent* ent);

darwin_dirent64* __darwin_readdir64(DIR* dirp)
{
	TRACE1(dirp);
	struct dirent* linux_buf = readdir(dirp);
	if (!linux_buf)
	{
		errnoOut();
		return 0;
	}
	
	//TRACE() << ARG(dirp) << linux_buf->d_name;
	
	return convertDirent64(linux_buf);
}

darwin_dirent* __darwin_readdir(DIR* dirp)
{
	TRACE1(dirp);
	struct dirent* linux_buf = readdir(dirp);
	if (!linux_buf)
	{
		errnoOut();
		return 0;
	}
	
	return convertDirent(linux_buf);
}

darwin_dirent* convertDirent(const struct dirent* linux_buf)
{
	static __thread darwin_dirent mac;
	
	mac.d_ino = linux_buf->d_ino;
	mac.d_reclen = linux_buf->d_reclen;
	
#ifdef _DIRENT_HAVE_D_TYPE
	mac.d_type = linux_buf->d_type;
#else
	mac.d_type = DT_UNKNOWN;
#endif

	mac.d_namlen = strlen(linux_buf->d_name);
	strcpy(mac.d_name, linux_buf->d_name);
	
	return &mac;
}

darwin_dirent64* convertDirent64(const struct dirent* linux_buf);
{
	static __thread darwin_dirent64 mac;
	
	mac.d_ino = linux_buf->d_ino;
	mac.d_reclen = linux_buf->d_reclen;
	
#ifdef _DIRENT_HAVE_D_TYPE
	mac.d_type = linux_buf->d_type;
#else
	mac.d_type = DT_UNKNOWN;
#endif
	mac.d_namlen = strlen(linux_buf->d_name);
#ifdef _DIRENT_HAVE_D_OFF
	mac.d_seekoff = linux_buf->d_off;
#else
	mac.d_seekoff = 0;
#endif

	strcpy(mac.d_name, linux_buf->d_name);
	return &mac;
}

DIR* __darwin_opendir(const char *name)
{
	DIR* rv = opendir(translatePathCI(name));
	if (!rv)
		errnoOut();
	return rv;
}

MAP_FUNCTION1(DIR*, fdopendir, int);
MAP_FUNCTION1(int, closedir, DIR*);
MAP_FUNCTION4(ssize_t, getdirentries, int, char*, int, off_t);

// scandir impl

static __thread int (*orig_filter)(const struct darwin_dirent *) = 0;
static __thread int (*orig_compar)(const struct darwin_dirent *) = 0;
static __thread int (*orig_filter64)(const struct darwin_dirent64 *) = 0;
static __thread int (*orig_compar64)(const struct darwin_dirent64 *) = 0;

static int native_filter(const struct dirent* e)
{
	darwin_dirent* ee = convertDirent(e);
	return orig_filter(ee);
}

static int native_compar(const struct dirent** a, const struct dirent** b)
{
	darwin_dirent xa, xb;
	darwin_dirent *aa, *bb;
	aa = &xa;
	bb = &xb;
	xa = *convertDirent(*a);
	xb = *convertDirent(*b);
	
	return orig_compar(&aa, &bb);
}

static int native_filter64(const struct dirent* e)
{
	darwin_dirent64* ee = convertDirent64(e);
	return orig_filter64(ee);
}

static int native_compar64(const struct dirent** a, const struct dirent** b)
{
	darwin_dirent64 xa, xb;
	darwin_dirent64 *aa, *bb;
	aa = &xa;
	bb = &xb;
	xa = *convertDirent64(*a);
	xb = *convertDirent64(*b);
	
	return orig_compar64(&aa, &bb);
}

int __darwin_scandir(const char *dirp, struct darwin_dirent ***namelist,
              int (*filter)(const struct darwin_dirent *),
              int (*compar)(const struct darwin_dirent **, const struct darwin_dirent **))
{
	struct dirent** nl;
	
	*namelist = 0;
	orig_filter = filter;
	orig_compar = compar;
	
	int (*my_filter)(const struct dirent *) = 0;
	
	if (filter)
		my_filter = native_filter;
	
	int rv = scandir(translatePathCI(dirp), &nl, my_filter, native_compar);
	if (rv == -1)
	{
		errnoOut();
		return -1;
	}
	else
	{
		*namelist = reinterpret_cast<struct darwin_dirent**>(malloc(rv * sizeof(struct darwin_dirent*)));
		for (int i = 0; i < rv; i++)
		{
			(*namelist)[i] = reinterpret_cast<struct darwin_dirent*>(malloc(sizeof(struct darwin_dirent)));
			*(*namelist)[i] = convertDirent(nl[i]);
			free(nl[i]);
		}
		free(nl);
		return rv;
	}
}

int __darwin_scandir64(const char *dirp, struct darwin_dirent64 ***namelist,
              int (*filter)(const struct darwin_dirent64 *),
              int (*compar)(const struct darwin_dirent64 **, const struct darwin_dirent64 **))
{
	struct dirent** nl;
	
	*namelist = 0;
	orig_filter64 = filter;
	orig_compar64 = compar;
	
	int (*my_filter)(const struct dirent *) = 0;
	
	if (filter)
		my_filter = native_filter;
	
	int rv = scandir(translatePathCI(dirp), &nl, my_filter, native_compar);
	if (rv == -1)
	{
		errnoOut();
		return -1;
	}
	else
	{
		*namelist = reinterpret_cast<struct darwin_dirent64**>(malloc(rv * sizeof(struct darwin_dirent64*)));
		for (int i = 0; i < rv; i++)
		{
			(*namelist)[i] = reinterpret_cast<struct darwin_dirent64*>(malloc(sizeof(struct darwin_dirent64)));
			*(*namelist)[i] = convertDirent64(nl[i]);
			free(nl[i]);
		}
		free(nl);
		return rv;
	}
}

template<typename T, typename Func> int darwin_sortconv(const void *a, const void *b, Func f)
{
	const T** aa = reinterpret_cast<const T**>(a);
	const T** bb = reinterpret_cast<const T**>(b);
	dirent na, nb;
	dirent *pna, *pnb;
	
	strcpy(na->d_name, (*aa)->d_name);
	strcpy(nb->d_name, (*bb)->d_name);
	
	return f(&pna, &pnb);
}

int __darwin_alphasort(const void *a, const void *b)
{
	return darwin_sortconv<darwin_dirent>(a, b, alphasort);
}

int __darwin_alphasort64(const void *a, const void *b)
{
	return darwin_sortconv<darwin_dirent64>(a, b, alphasort);
}

int __darwin_versionsort(const void *a, const void *b)
{
	return darwin_sortconv<darwin_dirent>(a, b, versionsort);
}

int __darwin_versionsort64(const void *a, const void *b)
{
	return darwin_sortconv<darwin_dirent64>(a, b, versionsort);
}
