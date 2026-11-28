/*
  This file is part of Darling.

  Copyright (C) 2026 Lubos Dolezel

  Darling is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Darling is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Network Security Services (NSS) public API, documented at
   https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Reference.
   This is a typed forwarding surface; the implementation calls the host's
   libnss3 through elfcalls. */
#ifndef _NSS_H_
#define _NSS_H_

#include <nspr/prtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int SECStatus;
#define SECSuccess 0
#define SECFailure -1
#define SECWouldBlock -2

typedef struct SECItemStr {
    unsigned char *data;
    unsigned int len;
    unsigned char type;
} SECItem;

typedef struct PLArenaPoolStr PLArenaPool;
typedef struct PK11SlotInfoStr PK11SlotInfo;
typedef struct CERTCertDBHandleStr CERTCertDBHandle;
typedef struct CERTCertificateStr CERTCertificate;
typedef struct PRFileDescStr NSS_PRFileDesc;
typedef struct NSSInitContextStr NSSInitContext;
typedef struct NSSInitParametersStr NSSInitParameters;

#define NSS_INIT_READONLY 0x01
#define NSS_INIT_NOCERTDB 0x02
#define NSS_INIT_NOMODDB 0x04
#define NSS_INIT_FORCEOPEN 0x08
#define NSS_INIT_READONLY_DB 0x10

SECStatus NSS_Init(const char *configdir);
SECStatus NSS_InitReadWrite(const char *configdir);
SECStatus NSS_Initialize(const char *configdir, const char *certPrefix,
                         const char *keyPrefix, const char *secmodName,
                         PRUint32 flags);
NSSInitContext *NSS_InitContext(const char *configdir, const char *certPrefix,
                                const char *keyPrefix, const char *secmodName,
                                NSSInitParameters *initParams, PRUint32 flags);
SECStatus NSS_Shutdown(void);
SECStatus NSS_ShutdownContext(NSSInitContext *context);
const char *NSS_GetVersion(void);

SECItem *SECITEM_AllocItem(PLArenaPool *arena, SECItem *item, unsigned int len);
SECStatus SECITEM_CopyItem(PLArenaPool *arena, SECItem *to, const SECItem *from);
void SECITEM_FreeItem(SECItem *item, int freeit);

SECStatus PK11_GenerateRandom(unsigned char *data, int len);
SECStatus PK11_GenerateRandomOnSlot(PK11SlotInfo *slot, unsigned char *data, int len);
PK11SlotInfo *PK11_GetInternalKeySlot(void);
void PK11_FreeSlot(PK11SlotInfo *slot);

CERTCertDBHandle *CERT_GetDefaultCertDB(void);
CERTCertificate *CERT_FindCertByNickname(CERTCertDBHandle *handle, const char *nickname);
void CERT_DestroyCertificate(CERTCertificate *cert);

/* SSL is a separate host library but shares NSS's opaque FD type. */
NSS_PRFileDesc *SSL_ImportFD(NSS_PRFileDesc *model, NSS_PRFileDesc *fd);
SECStatus SSL_SetURL(NSS_PRFileDesc *fd, const char *url);

#ifdef __cplusplus
}
#endif

#endif
