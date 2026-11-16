// Copyright (C) 2026 Lubos Dolezel
//
// TickCount (Events.h) counts 1/60 s ticks since startup: monotonic, and about 60 per second.
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    UInt32 a = TickCount();
    usleep(500000);
    UInt32 b = TickCount();
    UInt32 d = b - a;
    printf("ticks in 0.5 s: %u\n", d);
    if (d < 24 || d > 40) { printf("FAIL\n"); return 1; }
    printf("PASS\n");
    return 0;
}
