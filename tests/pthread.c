#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
  {
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
      abort();
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) != 0) {
      abort();
    }
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE) != 0) {
      abort();
    }
  }

  {
    pthread_rwlockattr_t attr;
    if (pthread_rwlockattr_init(&attr) != 0) {
      abort();
    }
    if (pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE) != 0) {
      abort();
    }
  }

  // TODO(hamaji): PTHREAD_MUTEX_INITIALIZER has a different implementation...
#if 0
  if (pthread_mutex_lock(&g_mutex) != 0) {
    abort();
  }
#endif
}
