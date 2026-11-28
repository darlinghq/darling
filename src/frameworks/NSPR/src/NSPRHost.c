/*
  This file is part of Darling.

  Copyright (C) 2026 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

// NSPR is implemented on the host's libnspr4 ELF library. It is opened and its
// entry points are resolved through elfcalls, the same way OpenCL reaches the
// host's ICD loader. Keeping the definitions here gives each entry point the
// documented public signature while the host library keeps its native ABI.

#include <nspr/nspr.h>
#include <pthread.h>
#include <elfcalls.h>

extern struct elf_calls *_elfcalls;

// name, return type, parameter list, argument names, result when the host
// library is unavailable
#define NSPR_FORWARDED_FUNCTIONS(X) \
    X(PR_InitializeNetAddr, PRStatus, (PRIPAddr ipaddress, PRUint16 port, PRNetAddr *addr), \
        (ipaddress, port, addr), PR_FAILURE) \
    X(PR_SetNetAddr, PRStatus, (PRIPAddr what, PRUint16 family, PRUint16 port, PRNetAddr *addr), \
        (what, family, port, addr), PR_FAILURE) \
    X(PR_GetNetAddressByName, PRStatus, (PRNetAddr *addr, const char *hostname), \
        (addr, hostname), PR_FAILURE) \
    X(PR_NetAddrFamily, PRUint16, (const PRNetAddr *addr), (addr), 0) \
    X(PR_OpenTCPSocket, PRFileDesc *, (int family), (family), NULL) \
    X(PR_OpenUDPSocket, PRFileDesc *, (int family), (family), NULL) \
    X(PR_Close, PRStatus, (PRFileDesc *fd), (fd), PR_FAILURE) \
    X(PR_Connect, PRStatus, (PRFileDesc *fd, const void *addr, unsigned int timeout), \
        (fd, addr, timeout), PR_FAILURE) \
    X(PR_Bind, PRStatus, (PRFileDesc *fd, const void *addr), (fd, addr), PR_FAILURE) \
    X(PR_Read, PRInt32, (PRFileDesc *fd, void *buf, PRInt32 amount), (fd, buf, amount), -1) \
    X(PR_Write, PRInt32, (PRFileDesc *fd, const void *buf, PRInt32 amount), (fd, buf, amount), -1)

#define NSPR_DECLARE_FUNCTION(name, ret, params, args, fallback) \
    typedef ret (*nspr_host_##name##_fn) params; \
    static nspr_host_##name##_fn nspr_host_##name;

NSPR_FORWARDED_FUNCTIONS(NSPR_DECLARE_FUNCTION)

#undef NSPR_DECLARE_FUNCTION

static void *nspr_host_library;
static pthread_once_t nspr_load_once = PTHREAD_ONCE_INIT;

static void nspr_load(void)
{
    if (_elfcalls == NULL || _elfcalls->dlopen == NULL || _elfcalls->dlsym == NULL)
        return;

    static const char *const names[] = { "libnspr4.so", "libnspr4.so.0", NULL };
    for (const char *const *name = names; *name != NULL; name++) {
        nspr_host_library = _elfcalls->dlopen(*name);
        if (nspr_host_library != NULL)
            break;
    }
    if (nspr_host_library == NULL)
        return;

#define NSPR_LOAD_FUNCTION(name, ret, params, args, fallback) \
    nspr_host_##name = (nspr_host_##name##_fn) _elfcalls->dlsym(nspr_host_library, #name);

    NSPR_FORWARDED_FUNCTIONS(NSPR_LOAD_FUNCTION)

#undef NSPR_LOAD_FUNCTION
}

static int nspr_ready(void)
{
    pthread_once(&nspr_load_once, nspr_load);
    return nspr_host_library != NULL;
}

#define NSPR_DEFINE_FUNCTION(name, ret, params, args, fallback) \
    ret name params \
    { \
        if (!nspr_ready() || nspr_host_##name == NULL) \
            return (fallback); \
        return nspr_host_##name args; \
    }

NSPR_FORWARDED_FUNCTIONS(NSPR_DEFINE_FUNCTION)

#undef NSPR_DEFINE_FUNCTION
