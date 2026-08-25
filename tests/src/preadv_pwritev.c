#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <errno.h>

void test_pwritev_preadv_basic(void)
{
	char path[] = "/tmp/preadv_pwritev_test.XXXXXX";
	int fd = mkstemp(path);
	assert(fd >= 0);

	// Test 1: Write header at offset 0
	const char* hdr = "HEADER-";
	ssize_t w = pwrite(fd, hdr, strlen(hdr), 0);
	assert(w == (ssize_t)strlen(hdr));

	// Test 2: pwritev at offset 7
	char buf1[] = "Hello, ";
	char buf2[] = "Darling ";
	char buf3[] = "World!\n";
	struct iovec iov_write[3];
	iov_write[0].iov_base = buf1;
	iov_write[0].iov_len = strlen(buf1);
	iov_write[1].iov_base = buf2;
	iov_write[1].iov_len = strlen(buf2);
	iov_write[2].iov_base = buf3;
	iov_write[2].iov_len = strlen(buf3);

	size_t total_write = strlen(buf1) + strlen(buf2) + strlen(buf3);
	ssize_t nw = pwritev(fd, iov_write, 3, strlen(hdr));
	assert(nw == (ssize_t)total_write);

	// Test 3: preadv from offset 7
	char r1[8] = {0};
	char r2[9] = {0};
	char r3[8] = {0};
	struct iovec iov_read[3];
	iov_read[0].iov_base = r1;
	iov_read[0].iov_len = strlen(buf1);
	iov_read[1].iov_base = r2;
	iov_read[1].iov_len = strlen(buf2);
	iov_read[2].iov_base = r3;
	iov_read[2].iov_len = strlen(buf3);

	ssize_t nr = preadv(fd, iov_read, 3, strlen(hdr));
	assert(nr == (ssize_t)total_write);

	assert(strcmp(r1, buf1) == 0);
	assert(strcmp(r2, buf2) == 0);
	assert(strcmp(r3, buf3) == 0);

	// Test 4: direct syscall test (SYS_preadv = 540, SYS_pwritev = 541)
	memset(r1, 0, sizeof(r1));
	memset(r2, 0, sizeof(r2));
	memset(r3, 0, sizeof(r3));
	long sys_nr = syscall(540, fd, iov_read, 3, (off_t)strlen(hdr));
	assert(sys_nr == (long)total_write);
	assert(strcmp(r1, buf1) == 0);
	assert(strcmp(r2, buf2) == 0);
	assert(strcmp(r3, buf3) == 0);

	close(fd);
	unlink(path);
}

int main(void)
{
	test_pwritev_preadv_basic();
	printf("preadv_pwritev tests passed successfully!\n");
	return 0;
}
