/*
 * Copyright (c) 1983, 1987, 1989
 *    The Regents of the University of California.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 	This product includes software developed by the University of
 * 	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *	@(#)resolv.h	8.1 (Berkeley) 6/2/93
 *	$Id: resolv.h,v 1.1 2006/03/01 19:01:39 majka Exp $
 */

/*
 * Important note regarding the BIND-9 API on Mac OS
 * -------------------------------------------------
 * 
 * Mac OS supports a DNS query routing API (see <dns.h>) which is used by
 * most system services to access DNS.  The BIND-9 APIs described here are
 * a lower-level that does not do query routing or search amongst multiple
 * resolver clients.  The results of DNS queries from this API may differ
 * significantly from the results of queries sent to the <dns.h> API.  We
 * strongly encourage developers to use higher-level APIs where possible.
 */

#ifndef _RESOLV_9_H_
#define	_RESOLV_9_H_

#include <sys/param.h>
#if (!defined(BSD)) || (BSD < 199306)
# include <sys/bitypes.h>
#else
# include <sys/types.h>
#endif
#include <sys/cdefs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <nameser.h>

/*
 * Revision information.  This is the release date in YYYYMMDD format.
 * It can change every day so the right thing to do with it is use it
 * in preprocessor commands such as "#if (__RES > 19931104)".  Do not
 * compare for equality; rather, use it to determine whether your resolver
 * is new enough to contain a certain feature.
 */

#define	__RES	19991006

#define __h_errno_set _res_9_h_errno_set
#define RES_SET_H_ERRNO(r,x) __h_errno_set(r,x)

#define __res_state __res_9_state
struct __res_state; /* forward */

__BEGIN_DECLS
void __h_errno_set(struct __res_state *res, int err);
__END_DECLS

/*
 * Resolver configuration file.
 * Normally not present, but may contain the address of the
 * inital name server(s) to query and the domain search list.
 *
 * Apple Note:  The default DNS resolver client configuration
 * is now stored in a system configuration database, not in
 */

#ifndef _PATH_RESCONF
#define _PATH_RESCONF        "/etc/resolv.conf"
#endif

#define res_goahead res_9_goahead
#define res_nextns res_9_nextns
#define res_modified res_9_modified
#define res_done res_9_done
#define res_error res_9_error
#define res_sendhookact res_9_sendhookact

typedef enum
{
	res_goahead,
	res_nextns,
	res_modified,
	res_done,
	res_error
} res_sendhookact;

typedef res_sendhookact (*res_send_qhook)__P((struct sockaddr * const *ns,
					      const u_char **query,
					      int *querylen,
					      u_char *ans,
					      int anssiz,
					      int *resplen));

typedef res_sendhookact (*res_send_rhook)__P((const struct sockaddr *ns,
					      const u_char *query,
					      int querylen,
					      u_char *ans,
					      int anssiz,
					      int *resplen));

#define res_sym res_9_sym

struct res_sym
{
	int		number;	   /* Identifying number, like T_MX */
	const char *	name;	   /* Its symbolic name, like "MX" */
	const char *	humanname; /* Its fun name, like "mail exchanger" */
};

/*
 * Global defines and variables for resolver stub.
 */
#define	MAXNS			3	/* max # name servers we'll track */
#define	MAXDFLSRCH		3	/* # default domain levels to try */
#define	MAXDNSRCH		6	/* max # domains in search path */
#define	LOCALDOMAINPARTS	2	/* min levels in name that is "local" */

#define	RES_TIMEOUT		5	/* min. seconds between retries */
#define	MAXRESOLVSORT		10	/* number of net to sort on */
#define	RES_MAXNDOTS		15	/* should reflect bit field size */
#define	RES_MAXRETRANS		30	/* only for resolv.conf/RES_OPTIONS */
#define	RES_MAXRETRY		5	/* only for resolv.conf/RES_OPTIONS */
#define	RES_DFLRETRY		2	/* Default #/tries. */
#define	RES_MAXTIME		65535	/* Infinity, in milliseconds. */

#define __res_state_ext __res_9_state_ext
struct __res_state_ext;

#define __res_state __res_9_state
struct __res_state {
	int	retrans;	 	/* retransmition time interval */
	int	retry;			/* number of times to transmit (attempts, not retries) */
#ifdef sun
	u_int	options;		/* option flags - see below. */
#else
	u_long	options;		/* option flags - see below. */
#endif
	int	nscount;		/* number of name servers */
	struct sockaddr_in
		nsaddr_list[MAXNS];	/* address of name server */
#define	nsaddr	nsaddr_list[0]		/* for backward compatibility */
	u_short	id;			/* current message id */
	char	*dnsrch[MAXDNSRCH+1];	/* components of domain to search */
	char	defdname[256];		/* default domain (deprecated) */
#ifdef sun
	u_int	pfcode;			/* RES_PRF_ flags - see below. */
#else
	u_long	pfcode;			/* RES_PRF_ flags - see below. */
#endif
	unsigned ndots:4;		/* threshold for initial abs. query */
	unsigned nsort:4;		/* number of elements in sort_list[] */
	char	unused[3];
	struct {
		struct in_addr	addr;
		u_int32_t	mask;
	} sort_list[MAXRESOLVSORT];
	res_send_qhook qhook;		/* query hook */
	res_send_rhook rhook;		/* response hook */
	int	res_h_errno;		/* last one set for this context */
	int	_vcsock;		/* PRIVATE: for res_send VC i/o */
	u_int	_flags;			/* PRIVATE: see below */
	u_int	_pad;			/* make _u 64 bit aligned */
	union {
		/* On an 32-bit arch this means 512b total. */
		char	pad[72 - 4*sizeof (int) - 2*sizeof (void *)];
		struct {
			u_int16_t		nscount;
			u_int16_t		nstimes[MAXNS];	/* ms. */
			int			nssocks[MAXNS];
			struct __res_state_ext *ext;	/* extention for IPv6 */
		} _ext;
	} _u;
};

#define res_state res_9_state

typedef struct __res_state *res_state;

#define res_sockaddr_union res_9_sockaddr_union

union res_sockaddr_union {
	struct sockaddr_in	sin;
#ifdef IN6ADDR_ANY_INIT
	struct sockaddr_in6	sin6;
#endif
#ifdef ISC_ALIGN64
	int64_t			__align64;	/* 64bit alignment */
#else
	int32_t			__align32;	/* 32bit alignment */
#endif
	char			__space[128];   /* max size */
};

/*
 * Resolver flags (used to be discrete per-module statics ints).
 */
#define	RES_F_VC			0x00000001	/* socket is TCP */
#define	RES_F_CONN		0x00000002	/* socket is connected */
#define	RES_F_EDNS0ERR	0x00000004	/* EDNS0 caused errors */

/* res_findzonecut2() options */
#define	RES_EXHAUSTIVE	0x00000001	/* always do all queries */
#define	RES_IPV4ONLY		0x00000002	/* IPv4 only */
#define	RES_IPV6ONLY		0x00000004	/* IPv6 only */

/*
 * Resolver options (keep these in synch with res_debug.c, please)
 */
#define RES_INIT			0x00000001	/* address initialized */
#define RES_DEBUG		0x00000002	/* print debug messages */
#define RES_AAONLY		0x00000004	/* authoritative answers only (!IMPL)*/
#define RES_USEVC		0x00000008	/* use virtual circuit */
#define RES_PRIMARY		0x00000010	/* query primary server only (!IMPL) */
#define RES_IGNTC		0x00000020	/* ignore trucation errors */
#define RES_RECURSE		0x00000040	/* recursion desired */
#define RES_DEFNAMES		0x00000080	/* use default domain name */
#define RES_STAYOPEN		0x00000100	/* Keep TCP socket open */
#define RES_DNSRCH		0x00000200	/* search up local domain tree */
#define	RES_INSECURE1	0x00000400	/* type 1 security disabled */
#define	RES_INSECURE2	0x00000800	/* type 2 security disabled */
#define	RES_NOALIASES	0x00001000	/* shuts off HOSTALIASES feature */
#define	RES_USE_INET6	0x00002000	/* use/map IPv6 in gethostbyname() */
#define RES_ROTATE		0x00004000	/* rotate ns list after each query */
#define	RES_NOCHECKNAME	0x00008000	/* do not check names for sanity. */
#define	RES_KEEPTSIG		0x00010000	/* do not strip TSIG records */
#define	RES_BLAST		0x00020000	/* blast all recursive servers */
#define RES_NO_NIBBLE	0x00040000	/* disable IPv6 nibble mode reverse */
#define RES_NO_BITSTRING 0x00080000	/* disable IPv6 bitstring mode reverse */
#define RES_NOTLDQUERY	0x00100000	/* don't unqualified name as a tld */
#define RES_USE_DNSSEC	0x00200000	/* use DNSSEC using OK bit in OPT */
/* KAME extensions: use higher bit to avoid conflict with ISC use */
#define RES_USE_DNAME	0x10000000	/* use DNAME */
#define RES_USE_A6		0x20000000	/* use A6 */
#define RES_USE_EDNS0	0x40000000	/* use EDNS0 if configured */
#define RES_NO_NIBBLE2	0x80000000	/* disable alternate nibble lookup */

#define RES_DEFAULT	(RES_RECURSE | RES_DEFNAMES | RES_DNSRCH | RES_INSECURE1)

/*
 * Resolver "pfcode" values.  Used by dig.
 */
#define RES_PRF_STATS	0x00000001
#define RES_PRF_UPDATE	0x00000002
#define RES_PRF_CLASS   0x00000004
#define RES_PRF_CMD		0x00000008
#define RES_PRF_QUES		0x00000010
#define RES_PRF_ANS		0x00000020
#define RES_PRF_AUTH		0x00000040
#define RES_PRF_ADD		0x00000080
#define RES_PRF_HEAD1	0x00000100
#define RES_PRF_HEAD2	0x00000200
#define RES_PRF_TTLID	0x00000400
#define RES_PRF_HEADX	0x00000800
#define RES_PRF_QUERY	0x00001000
#define RES_PRF_REPLY	0x00002000
#define RES_PRF_INIT		0x00004000
#define RES_PRF_TRUNC	0x00008000
/*			0x00010000	*/

/* Things involving an internal (static) resolver context. */
#ifndef __BIND_NOSTATIC
extern struct __res_state _res;
#endif

#ifndef __BIND_NOSTATIC
#define fp_nquery		res_9_fp_nquery
#define fp_query			res_9_fp_query
#define hostalias		res_9_hostalias_1
#define p_query			res_9_p_query
#define res_close		res_9_close
#define res_init			res_9_init
#define res_isourserver	res_9_isourserver
#define res_mkquery		res_9_mkquery
#define res_query		res_9_query
#define res_querydomain	res_9_querydomain
#define res_search		res_9_search
#define res_send			res_9_send
#define res_sendsigned	res_9_sendsigned

__BEGIN_DECLS
void		fp_nquery __P((const u_char *, int, FILE *));
void		fp_query __P((const u_char *, FILE *));
const char *hostalias __P((const char *));
void		p_query __P((const u_char *));
void		res_close __P((void));
int		res_init __P((void));
int		res_isourserver __P((const struct sockaddr_in *));
int		res_mkquery __P((int, const char *, int, int, const u_char *,
				 int, const u_char *, u_char *, int));
int		res_query __P((const char *, int, int, u_char *, int));
int		res_querydomain __P((const char *, const char *, int, int,
				     u_char *, int));
int		res_search __P((const char *, int, int, u_char *, int));
int		res_send __P((const u_char *, int, u_char *, int));
int		res_sendsigned __P((const u_char *, int, ns_tsig_key *,
				    u_char *, int));
__END_DECLS
#endif

#if !defined(SHARED_LIBBIND) || defined(LIB)
/*
 * If libbind is a shared object (well, DLL anyway)
 * these externs break the linker when resolv.h is 
 * included by a lib client (like named)
 * Make them go away if a client is including this
 *
 */
#define res_sym res_9_sym
#define __p_key_syms __res_9_p_key_syms
#define __p_cert_syms __res_9_p_cert_syms
#define __p_class_syms __res_9_p_class_syms
#define __p_type_syms __res_9_p_type_syms
#define __p_rcode_syms __res_9_p_rcode_syms

extern const struct res_sym __p_key_syms[];
extern const struct res_sym __p_cert_syms[];
extern const struct res_sym __p_class_syms[];
extern const struct res_sym __p_type_syms[];
extern const struct res_sym __p_rcode_syms[];
#endif /* SHARED_LIBBIND */

#define b64_ntop					res_9_b64_ntop
#define b64_pton					res_9_b64_pton
#define dn_comp					res_9_dn_comp
#define dn_count_labels			res_9_dn_count_labels
#define dn_expand				res_9_dn_expand
#define dn_skipname				res_9_dn_skipname
#define fp_resstat				res_9_fp_resstat
#define loc_aton					res_9_loc_aton
#define loc_ntoa					res_9_loc_ntoa
#define p_cdname					res_9_p_cdname
#define p_cdnname				res_9_p_cdnname
#define p_class					res_9_p_class
#define p_fqname					res_9_p_fqname
#define p_fqnname				res_9_p_fqnname
#define p_option					res_9_p_option
#define p_secstodate				res_9_p_secstodate
#define p_section				res_9_p_section
#define p_time					res_9_p_time
#define p_type					res_9_p_type
#define p_rcode					res_9_p_rcode
#define putlong					res_9_putlong
#define putshort					res_9_putshort
#define res_dnok					res_9_dnok
#define res_findzonecut			res_9_findzonecut
#define res_findzonecut2			res_9_findzonecut2
#define res_hnok					res_9_hnok
#define res_hostalias			res_9_hostalias_2
#define res_mailok				res_9_mailok
#define res_nameinquery			res_9_nameinquery
#define res_nclose				res_9_nclose
#define res_ninit				res_9_ninit
#define res_nmkquery				res_9_nmkquery
#define res_pquery				res_9_pquery
#define res_nquery				res_9_nquery
#define res_nquerydomain			res_9_nquerydomain
#define res_nsearch				res_9_nsearch
#define res_nsend				res_9_nsend
#define res_nsendsigned			res_9_nsendsigned
#define res_nisourserver			res_9_nisourserver
#define res_ownok				res_9_ownok
#define res_queriesmatch			res_9_queriesmatch
#define res_randomid				res_9_randomid
#define sym_ntop					res_9_sym_ntop
#define sym_ntos					res_9_sym_ntos
#define sym_ston					res_9_sym_ston
#define res_nopt					res_9_nopt
#define res_ndestroy				res_9_ndestroy
#define res_nametoclass			res_9_nametoclass
#define res_nametotype			res_9_nametotype
#define res_setservers			res_9_setservers
#define res_getservers			res_9_getservers
#define _getshort				res_9_getshort
#define _getlong					res_9_getlong
#define __res_vinit				res_9_vinit

__BEGIN_DECLS
int		res_hnok __P((const char *));
int		res_ownok __P((const char *));
int		res_mailok __P((const char *));
int		res_dnok __P((const char *));
int		sym_ston __P((const struct res_sym *, const char *, int *));
const char *	sym_ntos __P((const struct res_sym *, int, int *));
const char *sym_ntop __P((const struct res_sym *, int, int *));
int		b64_ntop __P((u_char const *, size_t, char *, size_t));
int		b64_pton __P((char const *, u_char *, size_t));
int		loc_aton __P((const char *ascii, u_char *binary));
const char *	loc_ntoa __P((const u_char *binary, char *ascii));
int		dn_skipname __P((const u_char *, const u_char *));
void		putlong __P((u_int32_t, u_char *));
void		putshort __P((u_int16_t, u_char *));
#ifndef __ultrix__
u_int16_t	_getshort __P((const u_char *src));
u_int32_t	_getlong __P((const u_char *src));
#endif
const char * p_class __P((int));
const char * p_time __P((u_int32_t));
const char * p_type __P((int));
const char * p_rcode __P((int));
const u_char * p_cdnname __P((const u_char *, const u_char *, int, FILE *));
const u_char * p_cdname __P((const u_char *, const u_char *, FILE *));
const u_char * p_fqnname __P((const u_char *cp, const u_char *msg, int, char *, int));
const u_char * p_fqname __P((const u_char *, const u_char *, FILE *));
const char * p_option __P((u_long option));
char * p_secstodate __P((u_long));
int		dn_count_labels __P((const char *));
int		dn_comp __P((const char *, u_char *, int, u_char **, u_char **));
int		dn_expand __P((const u_char *, const u_char *, const u_char *, char *, int));
u_int		res_randomid __P((void));
int		res_nameinquery __P((const char *, int, int, const u_char *, const u_char *));
int		res_queriesmatch __P((const u_char *, const u_char *, const u_char *, const u_char *));
const char * p_section __P((int section, int opcode));

/* Things involving a resolver context. */
int		res_ninit __P((res_state));
int		res_nisourserver __P((const res_state, const struct sockaddr_in *));
void		fp_resstat __P((const res_state, FILE *));
void		res_pquery __P((const res_state, const u_char *, int, FILE *));
const char *res_hostalias __P((const res_state, const char *, char *, size_t));
int		res_nquery __P((res_state, const char *, int, int, u_char *, int));
int		res_nsearch __P((res_state, const char *, int, int, u_char *, int));
int		res_nquerydomain __P((res_state, const char *, const char *, int, int, u_char *, int));
int		res_nmkquery __P((res_state, int, const char *, int, int, const u_char *, int, const u_char *, u_char *, int));
int		res_nsend __P((res_state, const u_char *, int, u_char *, int));
int		res_nsendsigned __P((res_state, const u_char *, int, ns_tsig_key *, u_char *, int));
int		res_findzonecut __P((res_state, const char *, ns_class, int, char *, size_t, struct in_addr *, int));
int		res_findzonecut2 __P((res_state, const char *, ns_class, int, char *, size_t, union res_sockaddr_union *, int));
void		res_nclose __P((res_state));
int		res_nopt __P((res_state, int, u_char *, int, int));
void		res_send_setqhook __P((res_send_qhook hook));
void		res_send_setrhook __P((res_send_rhook hook));
int		__res_vinit __P((res_state, int));
void		res_destroyservicelist __P((void));
const char *	res_servicename __P((u_int16_t port, const char *proto));
const char *	res_protocolname __P((int num));
void		res_destroyprotolist __P((void));
void		res_buildprotolist __P((void));
const char *	res_get_nibblesuffix __P((res_state));
const char *	res_get_nibblesuffix2 __P((res_state));
const char *	res_get_bitstringsuffix __P((res_state));
void		res_ndestroy __P((res_state));
u_int16_t	res_nametoclass __P((const char *buf, int *success));
u_int16_t	res_nametotype __P((const char *buf, int *success));
void		res_setservers __P((res_state, const union res_sockaddr_union *, int));
int		res_getservers __P((res_state, union res_sockaddr_union *, int));
__END_DECLS

#endif /* !_RESOLV_9_H_ */
