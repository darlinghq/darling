/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Thread-local variables (__thread) keep their value between accesses, start
// from their initializer, have one instance per thread, and are the same object
// whichever function takes their address. Run for both i386 and x86_64.

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

static __thread int t_zero;
static __thread int t_initialized = 42;
static __thread char t_buffer[64];

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

__attribute__((noinline)) static int *zeroAddress(void) { return &t_zero; }
__attribute__((noinline)) static int *initializedAddress(void) { return &t_initialized; }

struct thread_result {
    int zeroAfterSet, initializedAtStart, initializedAfterSet;
    int sameAddress;
    int *zeroSeen;
};

static void *worker(void *argument) {
    struct thread_result *result = argument;
    result->initializedAtStart = t_initialized;
    t_zero = 7;
    t_initialized = 8;
    result->zeroAfterSet = *zeroAddress();
    result->initializedAfterSet = *initializedAddress();
    result->sameAddress = zeroAddress() == zeroAddress() && &t_zero == zeroAddress();
    result->zeroSeen = &t_zero;
    return NULL;
}

int main(void) {
    CHECK(t_zero == 0);
    CHECK(t_initialized == 42);

    t_zero = 5;
    t_initialized = 6;
    t_buffer[3] = 'x';
    CHECK(t_zero == 5);
    CHECK(*zeroAddress() == 5);
    CHECK(zeroAddress() == zeroAddress());
    CHECK(zeroAddress() == &t_zero);
    CHECK(t_initialized == 6 && *initializedAddress() == 6);
    CHECK(t_buffer[3] == 'x');
    for (int i = 0; i < 1000; i++)
        t_zero++;
    CHECK(t_zero == 1005);

    struct thread_result result = { 0 };
    pthread_t thread;
    CHECK(pthread_create(&thread, NULL, worker, &result) == 0);
    CHECK(pthread_join(thread, NULL) == 0);
    CHECK(result.initializedAtStart == 42);
    CHECK(result.zeroAfterSet == 7);
    CHECK(result.initializedAfterSet == 8);
    CHECK(result.sameAddress);
    CHECK(result.zeroSeen != &t_zero);

    CHECK(t_zero == 1005);
    CHECK(t_initialized == 6);

    if (g_failures == 0)
        printf("PASS\n");
    return g_failures != 0;
}
