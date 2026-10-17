// Host test of swap_pacing.h, which has no OpenGL, EGL or CoreFoundation dependency.
//   cc -std=c11 -Wall -fsanitize=address,undefined -o swap_pacing_test swap_pacing_test.c && ./swap_pacing_test
#include "swap_pacing.h"
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

#define MS 1000000ull

int main(void) {
    // 75 Hz as wl_output reports it: 75000 mHz.
    CHECK(cgl_period_ns_from_millihertz(75000) == 13333333ull);
    // Unknown refresh falls back to 60 Hz.
    CHECK(cgl_period_ns_from_millihertz(0) == 16666666ull);

    struct cgl_swap_pacer p = { cgl_period_ns_from_millihertz(60000), 0 };
    uint64_t period = p.period_ns;

    // No previous swap, or interval 0: never waits.
    CHECK(cgl_pacing_wait_ns(&p, 1, 5000 * MS) == 0);
    p.last_ns = 1000 * MS;
    CHECK(cgl_pacing_wait_ns(&p, 0, 1000 * MS) == 0);
    CHECK(cgl_pacing_wait_ns(&p, -1, 1000 * MS) == 0);

    // Interval N waits N periods from the previous swap, less what has elapsed.
    CHECK(cgl_pacing_wait_ns(&p, 1, 1000 * MS) == period);
    CHECK(cgl_pacing_wait_ns(&p, 1, 1000 * MS + 5 * MS) == period - 5 * MS);
    CHECK(cgl_pacing_wait_ns(&p, 3, 1000 * MS) == 3 * period);

    // A swap that is already late does not wait.
    CHECK(cgl_pacing_wait_ns(&p, 1, 1000 * MS + period) == 0);
    CHECK(cgl_pacing_wait_ns(&p, 1, 1000 * MS + 10 * period) == 0);

    // A huge interval is bounded, and so is a missing period.
    CHECK(cgl_pacing_wait_ns(&p, 1000000, 1000 * MS) == CGL_MAX_SWAP_WAIT_NS);
    CHECK(cgl_pacing_wait_ns(&p, 0x7fffffff, 1000 * MS) == CGL_MAX_SWAP_WAIT_NS);
    struct cgl_swap_pacer unset = { 0, 1000 * MS };
    CHECK(cgl_pacing_wait_ns(&unset, 1, 1000 * MS) == 16666666ull);

    // cgl_swap_pace with the real clock: the first swap is immediate, the next
    // ones are spaced by interval periods and never by much more.
    struct cgl_swap_pacer live = { cgl_period_ns_from_millihertz(100000), 0 }; // 10 ms
    uint64_t t0 = cgl_monotonic_ns();
    cgl_swap_pace(&live, 2);
    CHECK(cgl_monotonic_ns() - t0 < 5 * MS);
    for (int i = 0; i < 5; i++)
        cgl_swap_pace(&live, 2);
    uint64_t elapsed = cgl_monotonic_ns() - t0;
    CHECK(elapsed >= 5 * 20 * MS);        // five waits of two 10 ms periods
    CHECK(elapsed < 5 * 20 * MS + 40 * MS); // scheduler slack

    if (failures == 0)
        printf("PASS swap_pacing\n");
    return failures != 0;
}
