#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Test optional-wrapper acquisition, using a host adapter for Mach-O metadata.

The generated acquisition/availability C is used verbatim; the host library must stay loaded when the wrapper is unloaded (no release). Symbol
resolvers and the Mach-O __elfname section are excluded from the host adapter.
The elf_calls hooks count acquisitions and releases, with a barrier forcing
concurrent lazy acquisitions to overlap. This is a host test, not a Darwin ABI
or symbol-resolver test. Run with --wrapgen, --elf, --source-dir and --out.
"""
import argparse
from pathlib import Path
import subprocess

HARNESS = r'''
#define _GNU_SOURCE
#include <elfcalls.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { THREADS = 8, READS = 100 };
static pthread_barrier_t barrier;
static int opened, closed, threaded, missing, bad;
static int (*available)(void);
static void *acquire(const char *name) {
    (void)name;
    __atomic_add_fetch(&opened, 1, __ATOMIC_RELAXED);
    if (__atomic_load_n(&threaded, __ATOMIC_ACQUIRE))
        pthread_barrier_wait(&barrier);
    return missing ? NULL : &opened;
}
static int release_handle(void *handle) {
    if (handle != &opened) abort();
    __atomic_add_fetch(&closed, 1, __ATOMIC_RELAXED);
    return 0;
}
static struct elf_calls hooks = {
    .dlopen = acquire, .dlopen_fatal = acquire,
    .dlclose_fatal = release_handle
};
struct elf_calls *_elfcalls = &hooks;
static void *reader(void *arg) {
    (void)arg;
    for (int i = 0; i < READS; ++i)
        if (available() != !missing)
            __atomic_add_fetch(&bad, 1, __ATOMIC_RELAXED);
    return NULL;
}
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    missing = !strcmp(argv[2], "missing");
    if (pthread_barrier_init(&barrier, NULL, THREADS)) return 2;
    void *module = dlopen(argv[1], RTLD_NOW);
    if (!module) { fprintf(stderr, "%s\n", dlerror()); return 2; }
    available = dlsym(module, "fixture_available");
    if (!available) return 2;
    __atomic_store_n(&threaded, 1, __ATOMIC_RELEASE);
    pthread_t threads[THREADS];
    for (int i = 0; i < THREADS; ++i)
        if (pthread_create(&threads[i], NULL, reader, NULL)) _Exit(2);
    for (int i = 0; i < THREADS; ++i)
        if (pthread_join(threads[i], NULL)) return 2;
    if (dlclose(module)) return 2;
    pthread_barrier_destroy(&barrier);
    printf("WRAPPER_ACQUISITIONS=%d RELEASES=%d BAD_READS=%d MODE=%s\n",
           opened, closed, bad, argv[2]);
    return opened == 1 && closed == 0 && bad == 0 ? 0 : 1;
}
'''

def main():
    p = argparse.ArgumentParser(description=__doc__)
    for key in ['wrapgen','elf','source-dir','out']:
        p.add_argument('--'+key,required=True)
    a = p.parse_args()
    out = Path(a.out).resolve()
    out.mkdir(parents=True,exist_ok=True)
    generated = out/'generated.c'
    subprocess.run([a.wrapgen,a.elf,str(generated),str(out/'vars.h'),'fixture_available'],check=True)
    body = generated.read_text().split('#if defined(__arm64__)',1)[0]
    assert 'dlclose_fatal' not in body, 'wrappers must not unload host libraries'
    (out/'adapter.c').write_text(body+'\nconst char __elfname[] = "fixture";\n')
    (out/'harness.c').write_text(HARNESS)
    inc = str(Path(a.source_dir)/'src/startup/mldr/elfcalls')
    subprocess.run(['clang-19','-shared','-fPIC','-I',inc,str(out/'adapter.c'),'-o',str(out/'wrapper.so')],check=True)
    subprocess.run(['clang-19','-pthread','-rdynamic','-I',inc,str(out/'harness.c'),'-ldl','-o',str(out/'harness')],check=True)
    statuses = []
    for mode in ['present','missing']:
        r = subprocess.run([str(out/'harness'),str(out/'wrapper.so'),mode],capture_output=True,text=True,timeout=20)
        print(r.stdout,end='')
        print('WRAPPER_MODE='+mode+' RC='+str(r.returncode))
        statuses.append(r.returncode)
    return 0 if statuses == [0,0] else 1

if __name__ == '__main__':
    raise SystemExit(main())
