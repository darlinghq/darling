/*	$FreeBSD: src/sys/netinet6/ipsec.h,v 1.4.2.2 2001/07/03 11:01:54 ume Exp $	*/
/*	$KAME: ipsec.h,v 1.44 2001/03/23 08:08:47 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * IPsec controller part.
 */

#ifndef _NETINET6_IPSEC_H_
#define _NETINET6_IPSEC_H_
#include <sys/cdefs.h>
#include <sys/appleapiopts.h>

#include <net/pfkeyv2.h>
#include <uuid/uuid.h>
#ifdef BSD_KERNEL_PRIVATE
#include <netkey/keydb.h>
#include <netinet/ip_var.h>

#include <os/log.h>

/* lock for IPsec stats */
extern lck_grp_t         *sadb_stat_mutex_grp;
extern lck_grp_attr_t    *sadb_stat_mutex_grp_attr;
extern lck_attr_t        *sadb_stat_mutex_attr;
extern lck_mtx_t         *sadb_stat_mutex;


#define IPSEC_STAT_INCREMENT(x) \
	OSIncrementAtomic64((SInt64 *)&x)

struct secpolicyaddrrange {
	struct sockaddr_storage start;  /* Start (low values) of address range */
	struct sockaddr_storage end;    /* End (high values) of address range */
};

/*
 * Security Policy Index
 * Ensure that both address families in the "src" and "dst" are same.
 * When the value of the ul_proto is ICMPv6, the port field in "src"
 * specifies ICMPv6 type, and the port field in "dst" specifies ICMPv6 code.
 */
struct secpolicyindex {
	u_int8_t dir;                   /* direction of packet flow, see below */
	struct sockaddr_storage src;    /* IP src address for SP */
	struct sockaddr_storage dst;    /* IP dst address for SP */
	u_int8_t prefs;                 /* prefix length in bits for src */
	u_int8_t prefd;                 /* prefix length in bits for dst */
	u_int16_t ul_proto;             /* upper layer Protocol */
	ifnet_t internal_if; /* Interface a matching packet is bound to */
	struct secpolicyaddrrange src_range;    /* IP src address range for SP */
	struct secpolicyaddrrange dst_range;    /* IP dst address range for SP */
#ifdef notyet
	uid_t uids;
	uid_t uidd;
	gid_t gids;
	gid_t gidd;
#endif
};

/* Security Policy Data Base */
struct secpolicy {
	LIST_ENTRY(secpolicy) chain;

	int refcnt;                     /* reference count */
	struct secpolicyindex spidx;    /* selector */
	u_int32_t id;                   /* It's unique number on the system. */
	u_int state;                    /* 0: dead, others: alive */
#define IPSEC_SPSTATE_DEAD      0
#define IPSEC_SPSTATE_ALIVE     1

	u_int policy;           /* DISCARD, NONE or IPSEC, see keyv2.h */
	struct ipsecrequest *req;
	/* pointer to the ipsec request tree, */
	/* if policy == IPSEC else this value == NULL.*/

	ifnet_t ipsec_if; /* IPsec interface to use */
	ifnet_t outgoing_if; /* Outgoing interface for encrypted traffic */

	char disabled; /* Set to ignore policy */

	/*
	 * lifetime handler.
	 * the policy can be used without limitiation if both lifetime and
	 * validtime are zero.
	 * "lifetime" is passed by sadb_lifetime.sadb_lifetime_addtime.
	 * "validtime" is passed by sadb_lifetime.sadb_lifetime_usetime.
	 */
	long created;           /* time created the policy */
	long lastused;          /* updated every when kernel sends a packet */
	long lifetime;          /* duration of the lifetime of this policy */
	long validtime;         /* duration this policy is valid without use */
};

/* Request for IPsec */
struct ipsecrequest {
	struct ipsecrequest *next;
	/* pointer to next structure */
	/* If NULL, it means the end of chain. */
	struct secasindex saidx;/* hint for search proper SA */
	                        /* if __ss_len == 0 then no address specified.*/
	u_int level;            /* IPsec level defined below. */

	struct secpolicy *sp;   /* back pointer to SP */
};

/* security policy in PCB */
struct inpcbpolicy {
	struct secpolicy *sp_in;
	struct secpolicy *sp_out;
	int priv;                       /* privileged socket ? */
};

/* SP acquiring list table. */
struct secspacq {
	LIST_ENTRY(secspacq) chain;

	struct secpolicyindex spidx;

	long created;           /* for lifetime */
	int count;              /* for lifetime */
	/* XXX: here is mbuf place holder to be sent ? */
};
#endif /* BSD_KERNEL_PRIVATE */

/* according to IANA assignment, port 0x0000 and proto 0xff are reserved. */
#define IPSEC_PORT_ANY          0
#define IPSEC_ULPROTO_ANY       255
#define IPSEC_PROTO_ANY         255

/* mode of security protocol */
/* NOTE: DON'T use IPSEC_MODE_ANY at SPD.  It's only use in SAD */
#define IPSEC_MODE_ANY          0       /* i.e. wildcard. */
#define IPSEC_MODE_TRANSPORT    1
#define IPSEC_MODE_TUNNEL       2

/*
 * Direction of security policy.
 * NOTE: Since INVALID is used just as flag.
 * The other are used for loop counter too.
 */
#define IPSEC_DIR_ANY           0
#define IPSEC_DIR_INBOUND       1
#define IPSEC_DIR_OUTBOUND      2
#define IPSEC_DIR_MAX           3
#define IPSEC_DIR_INVALID       4

/* Policy level */
/*
 * IPSEC, ENTRUST and BYPASS are allowed for setsockopt() in PCB,
 * DISCARD, IPSEC and NONE are allowed for setkey() in SPD.
 * DISCARD and NONE are allowed for system default.
 */
#define IPSEC_POLICY_DISCARD    0       /* discarding packet */
#define IPSEC_POLICY_NONE       1       /* through IPsec engine */
#define IPSEC_POLICY_IPSEC      2       /* do IPsec */
#define IPSEC_POLICY_ENTRUST    3       /* consulting SPD if present. */
#define IPSEC_POLICY_BYPASS     4       /* only for privileged socket. */
#define IPSEC_POLICY_GENERATE   5       /* same as discard - IKE daemon can override with generated policy */

/* Security protocol level */
#define IPSEC_LEVEL_DEFAULT     0       /* reference to system default */
#define IPSEC_LEVEL_USE         1       /* use SA if present. */
#define IPSEC_LEVEL_REQUIRE     2       /* require SA. */
#define IPSEC_LEVEL_UNIQUE      3       /* unique SA. */

#define IPSEC_MANUAL_REQID_MAX  0x3fff
/*
 * if security policy level == unique, this id
 * indicate to a relative SA for use, else is
 * zero.
 * 1 - 0x3fff are reserved for manual keying.
 * 0 are reserved for above reason.  Others is
 * for kernel use.
 * Note that this id doesn't identify SA
 * by only itself.
 */
#define IPSEC_REPLAYWSIZE  32

/* statistics for ipsec processing */
struct ipsecstat {
	u_quad_t in_success __attribute__ ((aligned(8)));  /* succeeded inbound process */
	u_quad_t in_polvio __attribute__ ((aligned(8)));
	/* security policy violation for inbound process */
	u_quad_t in_nosa __attribute__ ((aligned(8)));      /* inbound SA is unavailable */
	u_quad_t in_inval __attribute__ ((aligned(8)));     /* inbound processing failed due to EINVAL */
	u_quad_t in_nomem __attribute__ ((aligned(8)));     /* inbound processing failed due to ENOBUFS */
	u_quad_t in_badspi __attribute__ ((aligned(8)));    /* failed getting a SPI */
	u_quad_t in_ahreplay __attribute__ ((aligned(8)));  /* AH replay check failed */
	u_quad_t in_espreplay __attribute__ ((aligned(8)));  /* ESP replay check failed */
	u_quad_t in_ahauthsucc __attribute__ ((aligned(8)));  /* AH authentication success */
	u_quad_t in_ahauthfail __attribute__ ((aligned(8)));  /* AH authentication failure */
	u_quad_t in_espauthsucc __attribute__ ((aligned(8)));  /* ESP authentication success */
	u_quad_t in_espauthfail __attribute__ ((aligned(8)));  /* ESP authentication failure */
	u_quad_t in_esphist[256] __attribute__ ((aligned(8)));
	u_quad_t in_ahhist[256] __attribute__ ((aligned(8)));
	u_quad_t in_comphist[256] __attribute__ ((aligned(8)));
	u_quad_t out_success __attribute__ ((aligned(8)));  /* succeeded outbound process */
	u_quad_t out_polvio __attribute__ ((aligned(8)));
	/* security policy violation for outbound process */
	u_quad_t out_nosa __attribute__ ((aligned(8)));     /* outbound SA is unavailable */
	u_quad_t out_inval __attribute__ ((aligned(8)));    /* outbound process failed due to EINVAL */
	u_quad_t out_nomem __attribute__ ((aligned(8)));     /* inbound processing failed due to ENOBUFS */
	u_quad_t out_noroute __attribute__ ((aligned(8)));  /* there is no route */
	u_quad_t out_esphist[256] __attribute__ ((aligned(8)));
	u_quad_t out_ahhist[256] __attribute__ ((aligned(8)));
	u_quad_t out_comphist[256] __attribute__ ((aligned(8)));
};

#define IPSEC_MAX_WAKE_PKT_LEN  100
struct ipsec_wake_pkt_info {
	u_int8_t wake_pkt[IPSEC_MAX_WAKE_PKT_LEN];
	uuid_string_t wake_uuid;
	u_int32_t wake_pkt_spi;
	u_int32_t wake_pkt_seq;
	u_int16_t wake_pkt_len;
};

#ifdef BSD_KERNEL_PRIVATE
/*
 * Definitions for IPsec & Key sysctl operations.
 */
/*
 * Names for IPsec & Key sysctl objects
 */
#define IPSECCTL_STATS                  1       /* stats */
#define IPSECCTL_DEF_POLICY             2
#define IPSECCTL_DEF_ESP_TRANSLEV       3       /* int; ESP transport mode */
#define IPSECCTL_DEF_ESP_NETLEV         4       /* int; ESP tunnel mode */
#define IPSECCTL_DEF_AH_TRANSLEV        5       /* int; AH transport mode */
#define IPSECCTL_DEF_AH_NETLEV          6       /* int; AH tunnel mode */
#if 0   /* obsolete, do not reuse */
#define IPSECCTL_INBOUND_CALL_IKE       7
#endif
#define IPSECCTL_AH_CLEARTOS            8
#define IPSECCTL_AH_OFFSETMASK          9
#define IPSECCTL_DFBIT                  10
#define IPSECCTL_ECN                    11
#define IPSECCTL_DEBUG                  12
#define IPSECCTL_ESP_RANDPAD            13
#define IPSECCTL_MAXID                  14

#define IPSECCTL_NAMES { \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "def_policy", CTLTYPE_INT }, \
	{ "esp_trans_deflev", CTLTYPE_INT }, \
	{ "esp_net_deflev", CTLTYPE_INT }, \
	{ "ah_trans_deflev", CTLTYPE_INT }, \
	{ "ah_net_deflev", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ "ah_cleartos", CTLTYPE_INT }, \
	{ "ah_offsetmask", CTLTYPE_INT }, \
	{ "dfbit", CTLTYPE_INT }, \
	{ "ecn", CTLTYPE_INT }, \
	{ "debug", CTLTYPE_INT }, \
	{ "esp_randpad", CTLTYPE_INT }, \
}

#define IPSEC6CTL_NAMES { \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "def_policy", CTLTYPE_INT }, \
	{ "esp_trans_deflev", CTLTYPE_INT }, \
	{ "esp_net_deflev", CTLTYPE_INT }, \
	{ "ah_trans_deflev", CTLTYPE_INT }, \
	{ "ah_net_deflev", CTLTYPE_INT }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ 0, 0 }, \
	{ "ecn", CTLTYPE_INT }, \
	{ "debug", CTLTYPE_INT }, \
	{ "esp_randpad", CTLTYPE_INT }, \
}

#if defined(__ARM__)
#define IPSEC_IS_P2ALIGNED(p)        IS_P2ALIGNED(p, sizeof (u_int32_t))
#define IPSEC_GET_P2UNALIGNED_OFS(p) (sizeof(u_int32_t) - (((uintptr_t)(p)) & ((uintptr_t)(sizeof(u_int32_t)) - 1)))
#else
#define IPSEC_IS_P2ALIGNED(p)        1
#define IPSEC_GET_P2UNALIGNED_OFS(p) 0
#endif

struct ipsec_output_state {
	int tunneled;
	struct mbuf *m;
	struct route_in6 ro;
	struct sockaddr *dst;
	u_int outgoing_if;
};

struct ipsec_history {
	int ih_proto;
	u_int32_t ih_spi;
};

extern int ipsec_debug;

extern struct ipsecstat ipsecstat;
extern struct secpolicy ip4_def_policy;
extern int ip4_esp_trans_deflev;
extern int ip4_esp_net_deflev;
extern int ip4_ah_trans_deflev;
extern int ip4_ah_net_deflev;
extern int ip4_ah_cleartos;
extern int ip4_ah_offsetmask;
extern int ip4_ipsec_dfbit;
extern int ip4_ipsec_ecn;
extern int ip4_esp_randpad;

extern bool ipsec_save_wake_pkt;

#define _ipsec_log(level, fmt, ...) do {                            \
	os_log_type_t type;                                         \
	switch (level) {                                            \
	default:                                                    \
	        type = OS_LOG_TYPE_DEFAULT;                         \
	        break;                                              \
	case LOG_INFO:                                              \
	        type = OS_LOG_TYPE_INFO;                            \
	        break;                                              \
	case LOG_DEBUG:                                             \
	        type = OS_LOG_TYPE_DEBUG;                           \
	        break;                                              \
	case LOG_ERR:                                               \
	        type = OS_LOG_TYPE_ERROR;                           \
	        break;                                              \
	}                                                           \
	os_log_with_type(OS_LOG_DEFAULT, type, fmt, ##__VA_ARGS__); \
} while (0)

#define ipseclog(x)     do { if (ipsec_debug != 0) _ipsec_log x; } while (0)

extern struct secpolicy *ipsec4_getpolicybysock(struct mbuf *, u_int,
    struct socket *, int *);
extern struct secpolicy *ipsec4_getpolicybyaddr(struct mbuf *, u_int, int,
    int *);
extern int ipsec4_getpolicybyinterface(struct mbuf *, u_int, int *,
    struct ip_out_args *, struct secpolicy **);

extern u_int ipsec_get_reqlevel(struct ipsecrequest *);

struct inpcb;
extern int ipsec_init_policy(struct socket *so, struct inpcbpolicy **);
extern int ipsec_copy_policy(struct inpcbpolicy *, struct inpcbpolicy *);
extern u_int ipsec_get_reqlevel(struct ipsecrequest *);

extern int ipsec4_set_policy(struct inpcb *inp, int optname,
    caddr_t request, size_t len, int priv);
extern int ipsec4_delete_pcbpolicy(struct inpcb *);
extern int ipsec4_in_reject_so(struct mbuf *, struct socket *);
extern int ipsec4_in_reject(struct mbuf *, struct inpcb *);

struct secas;
struct tcpcb;
extern int ipsec_chkreplay(u_int32_t, struct secasvar *, u_int8_t);
extern int ipsec_updatereplay(u_int32_t, struct secasvar *, u_int8_t);

extern size_t ipsec4_hdrsiz(struct mbuf *, u_int, struct inpcb *);
extern size_t ipsec_hdrsiz_tcp(struct tcpcb *);
extern size_t ipsec_hdrsiz(struct secpolicy *);

struct ip;
extern const char *ipsec4_logpacketstr(struct ip *, u_int32_t);
extern const char *ipsec_logsastr(struct secasvar *);

extern void ipsec_dumpmbuf(struct mbuf *);

extern int ipsec4_interface_output(struct ipsec_output_state *state, ifnet_t interface);
extern int ipsec4_output(struct ipsec_output_state *, struct secpolicy *, int);
#if INET
extern struct mbuf * ipsec4_splithdr(struct mbuf *);
extern int ipsec4_encapsulate(struct mbuf *, struct secasvar *);
#endif
#if INET6
extern struct mbuf * ipsec6_splithdr(struct mbuf *);
extern int ipsec6_encapsulate(struct mbuf *, struct secasvar *);
#endif
extern int ipsec4_tunnel_validate(struct mbuf *, int, u_int, struct secasvar *, sa_family_t *);
extern struct mbuf *ipsec_copypkt(struct mbuf *);
extern void ipsec_delaux(struct mbuf *);
extern int ipsec_setsocket(struct mbuf *, struct socket *);
extern struct socket *ipsec_getsocket(struct mbuf *);
extern int ipsec_addhist(struct mbuf *, int, u_int32_t);
extern struct ipsec_history *ipsec_gethist(struct mbuf *, int *);
extern void ipsec_clearhist(struct mbuf *);
extern void ipsec_monitor_sleep_wake(void);
extern void ipsec_save_wake_packet(struct mbuf *, u_int32_t, u_int32_t);
#endif /* BSD_KERNEL_PRIVATE */

#ifndef KERNEL
__BEGIN_DECLS
extern caddr_t ipsec_set_policy(char *, int);
extern int ipsec_get_policylen(caddr_t);
extern char *ipsec_dump_policy(caddr_t, char *);

extern const char *ipsec_strerror(void);
__END_DECLS
#endif /* KERNEL */

#endif /* _NETINET6_IPSEC_H_ */
