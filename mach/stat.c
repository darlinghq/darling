#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
  struct stat buf;
  int fd = open(argv[0], O_RDONLY);
  int ret = fstat(fd, &buf);
  printf("ret=%d size=%d size_offset=%ld\n",
         ret, buf.st_size, (char*)&buf.st_size - (char*)&buf);
  return 0;
}
