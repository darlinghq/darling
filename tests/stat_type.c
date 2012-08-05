#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
  struct stat buf;
  int ret = stat("/tmp", &buf);
  if (!S_ISDIR(buf.st_mode))
    abort();
  return 0;
}
