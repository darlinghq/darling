#undef NDEBUG
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static __thread int initialized = 17;
static __thread unsigned char zeros[64];
static void *worker(void *unused)
{
    (void)unused;
    assert(initialized == 17);
    for (unsigned i = 0; i < sizeof(zeros); ++i) assert(zeros[i] == 0);
    initialized = 29;
    zeros[8] = 71;
    assert(initialized == 29 && zeros[8] == 71);
    return NULL;
}

int main(void)
{
    setbuf(stdout, NULL);
    alarm(10);
    assert(initialized == 17);
    initialized = 43;
    zeros[8] = 91;
    for (unsigned i = 0; i < 2; ++i) {
        pthread_t thread;
        assert(pthread_create(&thread, NULL, worker, NULL) == 0);
        assert(pthread_join(thread, NULL) == 0);
        assert(initialized == 43 && zeros[8] == 91);
    }
    alarm(0);
    puts("PASS guest compiler-generated TLV initialization, repeated access and thread isolation");
    return 0;
}
