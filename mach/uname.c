#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

int main(int argc, char* argv[]) {
  struct utsname buf;
  if (uname(&buf) != 0)
    abort();
  printf("sysname=%s\nnodename=%s\nrelease=%s\nversion=%s\nmachine=%s\n",
         buf.sysname, buf.nodename, buf.release, buf.version, buf.machine);
  printf("sysname=%lu\nnodename=%lu\nrelease=%lu\nversion=%lu\nmachine=%lu\n",
         offsetof(struct utsname, sysname), offsetof(struct utsname, nodename),
         offsetof(struct utsname, release), offsetof(struct utsname, version),
         offsetof(struct utsname, machine));
  return 0;
}
