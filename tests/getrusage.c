#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>

int main() {
  struct rusage ru;
  int r = getrusage(RUSAGE_SELF, &ru);
  printf("ret=%d\n", r);
  printf("utime=%d.%d stime=%d.%d\n"
         "maxrss=%ld ixrss=%ld idrss=%ld isrss=%ld\n"
         "minflt=%ld majflt=%ld nswap=%ld\n"
         "inblock=%ld oublock=%ld msgsnd=%ld msgrcv=%ld\n"
         "nsignals=%ld nvcsw=%ld nivcsw=%ld\n"
         ,
         (int)ru.ru_utime.tv_sec, (int)ru.ru_utime.tv_usec,
         (int)ru.ru_stime.tv_sec, (int)ru.ru_stime.tv_usec,
         ru.ru_maxrss, ru.ru_ixrss, ru.ru_idrss, ru.ru_isrss,
         ru.ru_minflt, ru.ru_majflt, ru.ru_nswap,
         ru.ru_inblock, ru.ru_oublock, ru.ru_msgsnd, ru.ru_msgrcv,
         ru.ru_nsignals, ru.ru_nvcsw, ru.ru_nivcsw);
  return 0;
}
