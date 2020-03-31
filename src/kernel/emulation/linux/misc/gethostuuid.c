#include "gethostuuid.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../libc/include/fcntl.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "../unistd/read.h"

static int hex2bin(const char* str, unsigned char* uuid_buf);

long sys_gethostuuid(unsigned char* uuid_buf, void* timeout)
{
	int ret, fd;
	char buf[32];

	fd = sys_open("/Volumes/SystemRoot/etc/machine-id", O_RDONLY, 0);
	if (fd < 0)
		return fd;

	ret = sys_read(fd, buf, sizeof(buf));
	close_internal(fd);

	if (ret < 0)
		return ret;
	if (ret != sizeof(buf))
		return -ENOSYS;

	return hex2bin(buf, uuid_buf);
}

static inline unsigned char hvalue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return 10 + (c - 'A');
	else if (c >= 'a' && c <= 'f')
		return 10 + (c - 'a');
	else
		return 0;
}

static int hex2bin(const char* str, unsigned char* uuid_buf)
{
	int i;

	for (i = 0; i < 16; i++)
	{
		unsigned char v;

		v = hvalue(str[i*2]) << 4;
		v |= hvalue(str[i*2+1]);

		uuid_buf[i] = v;
	}

	return 0;
}

