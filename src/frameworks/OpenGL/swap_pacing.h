#ifndef CGL_SWAP_PACING_H
#define CGL_SWAP_PACING_H

// Software swap interval for display platforms where EGL cannot honor one
// safely. With an interval above 0, Mesa's Wayland platform makes
// eglSwapBuffers wait for the previous frame callback, and the compositor
// never sends one for a surface it is not presenting (hidden, unmapped, other
// workspace), which freezes a caller that swaps on its main thread. The
// context keeps EGL at interval 0 and calls cgl_swap_pace() before each swap.
//
// Header only and free of OpenGL, EGL and CoreFoundation so a host test can
// include it as it is.

#include <errno.h>
#include <stdint.h>
#include <time.h>

// Used when the backend has not told the context a refresh rate.
#define CGL_DEFAULT_REFRESH_MILLIHERTZ 60000u

// No single swap waits longer than this, whatever interval was requested.
#define CGL_MAX_SWAP_WAIT_NS 1000000000ull

struct cgl_swap_pacer {
    uint64_t period_ns; // one refresh period; 0 until set
    uint64_t last_ns;   // when the previous swap was due; 0 before the first
};

static inline uint64_t cgl_period_ns_from_millihertz(uint32_t millihertz) {
    if (millihertz == 0)
        millihertz = CGL_DEFAULT_REFRESH_MILLIHERTZ;
    return 1000000000000ull / millihertz;
}

static inline uint64_t cgl_monotonic_ns(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_nsec;
}

// How long a swap issued at now_ns must wait: until interval refresh periods
// after the previous swap, nothing when that moment has passed.
static inline uint64_t cgl_pacing_wait_ns(const struct cgl_swap_pacer *pacer,
                                          int interval, uint64_t now_ns) {
    if (interval <= 0 || pacer->last_ns == 0)
        return 0;
    uint64_t period = pacer->period_ns ? pacer->period_ns
                                       : cgl_period_ns_from_millihertz(0);
    uint64_t span = (uint64_t)interval > CGL_MAX_SWAP_WAIT_NS / period
        ? CGL_MAX_SWAP_WAIT_NS : period * (uint64_t)interval;
    uint64_t due = pacer->last_ns + span;
    return now_ns < due ? due - now_ns : 0;
}

// Sleeps for the wait above and records the swap. A swap that arrives late
// restarts the schedule from now instead of catching up with a burst; one that
// waited keeps the phase of the schedule.
static inline void cgl_swap_pace(struct cgl_swap_pacer *pacer, int interval) {
    uint64_t now = cgl_monotonic_ns();
    uint64_t wait = cgl_pacing_wait_ns(pacer, interval, now);
    if (wait) {
        struct timespec left = { (time_t)(wait / 1000000000ull),
                                 (long)(wait % 1000000000ull) };
        while (nanosleep(&left, &left) != 0 && errno == EINTR)
            ;
    }
    pacer->last_ns = now + wait;
}

#endif
