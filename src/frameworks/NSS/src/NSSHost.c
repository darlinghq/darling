/*
  This file is part of Darling.

  Copyright (C) 2026 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
*/

// NSS is implemented on the host's NSS ELF libraries. They are opened and
// their entry points are resolved through elfcalls, the same way OpenCL
// reaches the host's ICD loader. The definitions below carry the documented
// public signatures; the real implementations stay in the host libraries.

#include <nss/nss.h>
#include <pthread.h>
#include <elfcalls.h>

extern struct elf_calls *_elfcalls;

// name, return type, parameter list, argument names, result when the host
// libraries are unavailable
#define NSS_FORWARDED_FUNCTIONS(X) \
    X(NSS_Init, SECStatus, (const char *configdir), (configdir), SECFailure) \
    X(NSS_InitReadWrite, SECStatus, (const char *configdir), (configdir), SECFailure) \
    X(NSS_Initialize, SECStatus, (const char *configdir, const char *certPrefix, \
        const char *keyPrefix, const char *secmodName, PRUint32 flags), \
        (configdir, certPrefix, keyPrefix, secmodName, flags), SECFailure) \
    X(NSS_InitContext, NSSInitContext *, (const char *configdir, const char *certPrefix, \
        const char *keyPrefix, const char *secmodName, NSSInitParameters *initParams, \
        PRUint32 flags), \
        (configdir, certPrefix, keyPrefix, secmodName, initParams, flags), NULL) \
    X(NSS_Shutdown, SECStatus, (void), (), SECFailure) \
    X(NSS_ShutdownContext, SECStatus, (NSSInitContext *context), (context), SECFailure) \
    X(NSS_GetVersion, const char *, (void), (), "NSS unavailable") \
    X(SECITEM_AllocItem, SECItem *, (PLArenaPool *arena, SECItem *item, unsigned int len), \
        (arena, item, len), NULL) \
    X(SECITEM_CopyItem, SECStatus, (PLArenaPool *arena, SECItem *to, const SECItem *from), \
        (arena, to, from), SECFailure) \
    X(SECITEM_FreeItem, void, (SECItem *item, int freeit), (item, freeit), (void)0) \
    X(PK11_GenerateRandom, SECStatus, (unsigned char *data, int len), (data, len), SECFailure) \
    X(PK11_GenerateRandomOnSlot, SECStatus, (PK11SlotInfo *slot, unsigned char *data, int len), \
        (slot, data, len), SECFailure) \
    X(PK11_GetInternalKeySlot, PK11SlotInfo *, (void), (), NULL) \
    X(PK11_FreeSlot, void, (PK11SlotInfo *slot), (slot), (void)0) \
    X(CERT_GetDefaultCertDB, CERTCertDBHandle *, (void), (), NULL) \
    X(CERT_FindCertByNickname, CERTCertificate *, (CERTCertDBHandle *handle, \
        const char *nickname), (handle, nickname), NULL) \
    X(CERT_DestroyCertificate, void, (CERTCertificate *cert), (cert), (void)0) \
    X(SSL_ImportFD, NSS_PRFileDesc *, (NSS_PRFileDesc *model, NSS_PRFileDesc *fd), \
        (model, fd), NULL) \
    X(SSL_SetURL, SECStatus, (NSS_PRFileDesc *fd, const char *url), (fd, url), SECFailure)

#define NSS_DECLARE_FUNCTION(name, ret, params, args, fallback) \
    typedef ret (*nss_host_##name##_fn) params; \
    static nss_host_##name##_fn nss_host_##name;

NSS_FORWARDED_FUNCTIONS(NSS_DECLARE_FUNCTION)

#undef NSS_DECLARE_FUNCTION

static void *nss_host_libraries[5];
static size_t nss_host_library_count;
static pthread_once_t nss_load_once = PTHREAD_ONCE_INIT;

static void nss_load(void)
{
    if (_elfcalls == NULL || _elfcalls->dlopen == NULL || _elfcalls->dlsym == NULL)
        return;

    static const char *const names[] = {
        "libnss3.so", "libnss3.so.1", "libssl3.so", "libssl3.so.1", "libnssutil3.so", NULL
    };
    for (const char *const *name = names; *name != NULL; name++) {
        void *library = _elfcalls->dlopen(*name);
        if (library != NULL && nss_host_library_count < 5)
            nss_host_libraries[nss_host_library_count++] = library;
    }
    if (nss_host_library_count == 0)
        return;

#define NSS_LOAD_FUNCTION(name, ret, params, args, fallback) \
    if (nss_host_##name == NULL) { \
        for (size_t i = 0; i < nss_host_library_count; i++) { \
            nss_host_##name = (nss_host_##name##_fn) \
                    _elfcalls->dlsym(nss_host_libraries[i], #name); \
            if (nss_host_##name != NULL) \
                break; \
        } \
    }

    NSS_FORWARDED_FUNCTIONS(NSS_LOAD_FUNCTION)

#undef NSS_LOAD_FUNCTION
}

static int nss_ready(void)
{
    pthread_once(&nss_load_once, nss_load);
    return nss_host_library_count != 0;
}

#define NSS_DEFINE_FUNCTION(name, ret, params, args, fallback) \
    ret name params \
    { \
        if (!nss_ready() || nss_host_##name == NULL) \
            return (fallback); \
        return nss_host_##name args; \
    }

NSS_FORWARDED_FUNCTIONS(NSS_DEFINE_FUNCTION)

#undef NSS_DEFINE_FUNCTION
