// Copyright (C) 2026 Lubos Dolezel
//
// IOKit clients are answered by iokitd whether or not the host has a D-Bus client library: without it iokitd
// simply has no host bus (as when no bus is running), and a client's lookup of the platform expert still
// returns, instead of waiting for a daemon that died at start-up. A watchdog reports a call that never returns.
// Builds for x86_64 and i386 and runs on macOS.
#include <IOKit/IOKitLib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static void watchdog(int sig)
{
    (void)sig;
    static const char msg[] = "FAIL: IOKit call did not return (iokitd not answering)\n";
    write(1, msg, sizeof(msg) - 1);
    _exit(1);
}

int main(void)
{
    signal(SIGALRM, watchdog);
    alarm(40);

    mach_port_t master = MACH_PORT_NULL;
    kern_return_t kr = IOMasterPort(MACH_PORT_NULL, &master);
    printf("IOMasterPort -> %d\n", (int)kr);
    if (kr != KERN_SUCCESS) {
        printf("FAIL: IOMasterPort\n");
        return 1;
    }

    io_service_t platform = IOServiceGetMatchingService(master, IOServiceMatching("IOPlatformExpertDevice"));
    printf("IOPlatformExpertDevice service -> %s\n", platform != IO_OBJECT_NULL ? "found" : "none");

    io_iterator_t iterator = IO_OBJECT_NULL;
    kr = IOServiceGetMatchingServices(master, IOServiceMatching("IOService"), &iterator);
    printf("IOServiceGetMatchingServices -> %d\n", (int)kr);
    if (kr != KERN_SUCCESS) {
        printf("FAIL: IOServiceGetMatchingServices\n");
        return 1;
    }
    IOObjectRelease(iterator);
    if (platform != IO_OBJECT_NULL)
        IOObjectRelease(platform);
    printf("PASS\n");
    return 0;
}
