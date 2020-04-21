#include <signal.h>
#include <unistd.h>
#include <mach/mach.h>


__attribute__((section("__DATA,__allow_alt_plat"))) uint64_t dummy;

int main()
{
    (void)kill(getpid(), SIGSTOP);
    return 0;
}

