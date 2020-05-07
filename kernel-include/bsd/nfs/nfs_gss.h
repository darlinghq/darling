/*
 * Copyright (c) 2007-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _NFS_NFS_GSS_H_
#define _NFS_NFS_GSS_H_

#include "gss/gss_krb5_mech.h"
#include <gssd/gssd_mach.h>
#include <sys/param.h>
#include <nfs/nfs_ioctl.h>

#define RPCSEC_GSS                      6
#define RPCSEC_GSS_VERS_1               1

enum rpcsec_gss_proc {
	RPCSEC_GSS_DATA                 = 0,
	RPCSEC_GSS_INIT                 = 1,
	RPCSEC_GSS_CONTINUE_INIT        = 2,
	RPCSEC_GSS_DESTROY              = 3
};

enum rpcsec_gss_service {
	RPCSEC_GSS_SVC_NONE             = 1,    // sec=krb5
	RPCSEC_GSS_SVC_INTEGRITY        = 2,    // sec=krb5i
	RPCSEC_GSS_SVC_PRIVACY          = 3,    // sec=krb5p
};

/* encoded krb5 OID */
extern u_char krb5_mech_oid[11];


/*
 * RFC 2203 and friends don't define maximums for token lengths
 * and context handles. We try to pick reasonable values here.
 *
 * N.B. Kerberos mech tokens can be quite large from the output
 * of a gss_init_sec_context if it includes a large PAC.
 */

#define GSS_MAX_CTX_HANDLE_LEN          256
#define GSS_MAX_TOKEN_LEN               64*1024

/*
 * Put a "reasonble" bound on MIC lengths
 */
#define GSS_MAX_MIC_LEN                 2048

#define GSS_MAXSEQ                      0x80000000      // The biggest sequence number
#define GSS_SVC_MAXCONTEXTS             500000          // Max contexts supported
#define GSS_SVC_SEQWINDOW               256             // Server's sequence window
#define GSS_CLNT_SEQLISTMAX             32              // Max length of req seq num list

#define MAX_SKEYLEN     32
#define MAX_LUCIDLEN    (sizeof (lucid_context) + MAX_SKEYLEN)
#define GSS_MAX_NEG_CACHE_ENTRIES 16
#define GSS_NEG_CACHE_TO 3
#define GSS_PRINT_DELAY  (8 * 3600)     // Wait day before printing the same error message

/*
 * The client's RPCSEC_GSS context information
 */
struct nfs_gss_clnt_ctx {
	lck_mtx_t               *gss_clnt_mtx;
	thread_t                gss_clnt_thread;        // Thread creating context
	TAILQ_ENTRY(nfs_gss_clnt_ctx)   gss_clnt_entries;
	uint32_t                gss_clnt_flags;         // Flag bits - see below
	int32_t                 gss_clnt_refcnt;        // Reference count
	kauth_cred_t            gss_clnt_cred;          // Owner of this context
	uint8_t                 *gss_clnt_principal;    // Principal to use for this credential
	uint32_t                gss_clnt_prinlen;       // Length of principal
	gssd_nametype           gss_clnt_prinnt;        // Name type of principal
	char                    *gss_clnt_display;      // display name of principal
	uint32_t                gss_clnt_proc;          // Current GSS proc for cred
	uint32_t                gss_clnt_seqnum;        // GSS sequence number
	uint32_t                gss_clnt_service;       // Indicates krb5, krb5i or krb5p
	uint8_t                 *gss_clnt_handle;       // Identifies server context
	uint32_t                gss_clnt_handle_len;    // Size of server's ctx handle
	time_t                  gss_clnt_nctime;        // When context was put in the negative cache
	uint32_t                gss_clnt_seqwin;        // Server's seq num window
	uint32_t                *gss_clnt_seqbits;      // Bitmap to track seq numbers in use
	mach_port_t             gss_clnt_mport;         // Mach port for gssd upcall
	uint32_t                gss_clnt_verflen;       // RPC verifier length from server
	uint8_t                 *gss_clnt_verf;         // RPC verifier from server
	uint8_t                 *gss_clnt_svcname;      // Service name e.g. "nfs/big.apple.com"
	uint32_t                gss_clnt_svcnamlen;     // Service name length
	gssd_nametype           gss_clnt_svcnt;         // Service name type
	gssd_cred               gss_clnt_cred_handle;   // Opaque cred handle from gssd
	gssd_ctx                gss_clnt_context;       // Opaque context handle from gssd
	gss_ctx_id_t            gss_clnt_ctx_id;        // Underlying gss context
	uint8_t                 *gss_clnt_token;        // GSS token exchanged via gssd & server
	uint32_t                gss_clnt_tokenlen;      // Length of token
	uint32_t                gss_clnt_gssd_flags;    // Special flag bits to gssd
	uint32_t                gss_clnt_major;         // GSS major result from gssd or server
	uint32_t                gss_clnt_minor;         // GSS minor result from gssd or server
	time_t                  gss_clnt_ptime;         // When last error message was printed
};

/*
 * gss_clnt_flags
 */
#define GSS_CTX_COMPLETE        0x00000001      // Context is complete
#define GSS_CTX_INVAL           0x00000002      // Context is invalid
#define GSS_CTX_STICKY          0x00000004      // Context has been set by user
#define GSS_NEEDSEQ             0x00000008      // Need a sequence number
#define GSS_NEEDCTX             0x00000010      // Need the context
#define GSS_CTX_DESTROY         0x00000020      // Context is being destroyed, don't cache

/*
 * The server's RPCSEC_GSS context information
 */
struct nfs_gss_svc_ctx {
	lck_mtx_t               *gss_svc_mtx;
	LIST_ENTRY(nfs_gss_svc_ctx)     gss_svc_entries;
	uint32_t                gss_svc_handle;         // Identifies server context to client
	uint32_t                gss_svc_refcnt;         // Reference count
	uint32_t                gss_svc_proc;           // Current GSS proc from cred
	uid_t                   gss_svc_uid;            // UID of this user
	gid_t                   gss_svc_gids[NGROUPS];  // GIDs of this user
	uint32_t                gss_svc_ngroups;        // Count of gids
	uint64_t                gss_svc_incarnation;    // Delete ctx if we exceed this + ttl value
	uint32_t                gss_svc_seqmax;         // Current max GSS sequence number
	uint32_t                gss_svc_seqwin;         // GSS sequence number window
	uint32_t                *gss_svc_seqbits;       // Bitmap to track seq numbers
	gssd_cred               gss_svc_cred_handle;    // Opaque cred handle from gssd
	gssd_ctx                gss_svc_context;        // Opaque context handle from gssd
	gss_ctx_id_t            gss_svc_ctx_id;         // Underlying gss context
	u_char                  *gss_svc_token;         // GSS token exchanged via gssd & client
	uint32_t                gss_svc_tokenlen;       // Length of token
	uint32_t                gss_svc_major;          // GSS major result from gssd
	uint32_t                gss_svc_minor;          // GSS minor result from gssd
};

#define SVC_CTX_HASHSZ  64
#define SVC_CTX_HASH(handle)    ((handle) % SVC_CTX_HASHSZ)
LIST_HEAD(nfs_gss_svc_ctx_hashhead, nfs_gss_svc_ctx);

/*
 * Macros to manipulate bits in the sequence window
 */
#define win_getbit(bits, bit)      ((bits[(bit) / 32] &   (1 << (bit) % 32)) != 0)
#define win_setbit(bits, bit)   do { bits[(bit) / 32] |=  (1 << (bit) % 32); } while (0)
#define win_resetbit(bits, bit) do { bits[(bit) / 32] &= ~(1 << (bit) % 32); } while (0)

/*
 * Server context stale times
 */
#define GSS_CTX_PEND            5               // seconds
#define GSS_CTX_EXPIRE          (8 * 3600)      // seconds
#define GSS_CTX_TTL_MIN         1               // seconds
#define GSS_TIMER_PERIOD        300             // seconds
#define MSECS_PER_SEC           1000

#define auth_is_kerberized(auth) \
	(auth == RPCAUTH_KRB5 || \
	 auth == RPCAUTH_KRB5I || \
	 auth == RPCAUTH_KRB5P)

__BEGIN_DECLS

void    nfs_gss_init(void);
uid_t   nfs_cred_getasid2uid(kauth_cred_t);
int     nfs_gss_clnt_cred_put(struct nfsreq *, struct nfsm_chain *, mbuf_t);
int     nfs_gss_clnt_verf_get(struct nfsreq *, struct nfsm_chain *,
    uint32_t, uint32_t, uint32_t *);
void    nfs_gss_clnt_rpcdone(struct nfsreq *);
int     nfs_gss_clnt_args_restore(struct nfsreq *);
int     nfs_gss_clnt_ctx_renew(struct nfsreq *);
void    nfs_gss_clnt_ctx_ref(struct nfsreq *, struct nfs_gss_clnt_ctx *);
void    nfs_gss_clnt_ctx_unref(struct nfsreq *);
void    nfs_gss_clnt_ctx_unmount(struct nfsmount *);
int     nfs_gss_clnt_ctx_remove(struct nfsmount *, kauth_cred_t);
int     nfs_gss_clnt_ctx_set_principal(struct nfsmount *, vfs_context_t, uint8_t *, uint32_t, uint32_t);
int     nfs_gss_clnt_ctx_get_principal(struct nfsmount *, vfs_context_t, struct user_nfs_gss_principal *);
int     nfs_gss_svc_cred_get(struct nfsrv_descript *, struct nfsm_chain *);
int     nfs_gss_svc_verf_put(struct nfsrv_descript *, struct nfsm_chain *);
int     nfs_gss_svc_ctx_init(struct nfsrv_descript *, struct nfsrv_sock *, mbuf_t *);
int     nfs_gss_svc_prepare_reply(struct nfsrv_descript *, struct nfsm_chain *);
int     nfs_gss_svc_protect_reply(struct nfsrv_descript *, mbuf_t);
void    nfs_gss_svc_ctx_deref(struct nfs_gss_svc_ctx *);
void    nfs_gss_svc_cleanup(void);

__END_DECLS
#endif /* _NFS_NFS_GSS_H_ */
