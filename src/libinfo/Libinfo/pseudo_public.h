/*
	Copyright (c) 2018 Apple Inc. All rights reserved.
	
	This pseduo-header file contains declarations for public symbols that are exported in libsystem_info.dylib but don't have
	a proper header file in the Libinfo project. These symbols are typically declared in header files installed by other
	projects. The purpose of this file is to allow InstallAPI, which parses header files to infer the symbols exported by a
	library, to know about these symbols.
*/


#include <arpa/inet.h>
#include <fstab.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

// Declared in /usr/include/arpa/inet.h
// Note: These are currently implemented as macros based on machine endianness. Include for completeness.

#undef ntohl
uint32_t ntohl(uint32_t x);
#undef htonl
uint32_t htonl(uint32_t x);
#undef ntohs
uint16_t ntohs(uint16_t x);
#undef htons
uint16_t htons(uint16_t x);


// Declared in /usr/include/fstab.h

void			endfsent(void);
struct fstab *	getfsent(void);
struct fstab *	getfsfile(const char *file);
struct fstab *	getfsspec(const char *spec);
int				setfsent(void);


// Declared in /usr/include/net/ethernet.h

struct ether_addr *	ether_aton(const char *s);
int					ether_hostton(const char *name, struct ether_addr *e);
int					ether_line(const char *s, struct ether_addr *e, char *hostname);
char *				ether_ntoa(const struct ether_addr *e);
int					ether_ntohost(char *name, const struct ether_addr *e);


// Declared in /usr/include/net/if.h

void					if_freenameindex(struct if_nameindex *ptr);
char *					if_indextoname(unsigned int ifindex, char *ifname);
struct if_nameindex *	if_nameindex(void);
unsigned int			if_nametoindex(const char *ifname);


// Declared in /usr/include/netinet/in.h

extern const struct in6_addr	in6addr_any;
extern const struct in6_addr	in6addr_linklocal_allnodes;
extern const struct in6_addr	in6addr_linklocal_allrouters;
extern const struct in6_addr	in6addr_linklocal_allv2routers;
extern const struct in6_addr	in6addr_loopback;
extern const struct in6_addr	in6addr_nodelocal_allnodes;

int					inet6_opt_append(void *extbuf, socklen_t extlen, int offset, u_int8_t type, socklen_t len, u_int8_t align, void **databufp);
int					inet6_opt_find(void *extbuf, socklen_t extlen, int offset, u_int8_t type, socklen_t *lenp, void **databufp);
int					inet6_opt_finish(void *extbuf, socklen_t extlen, int offset);
int					inet6_opt_get_val(void *databuf, int offset, void *val, socklen_t vallen);
int					inet6_opt_init(void *extbuf, socklen_t extlen);
int					inet6_opt_next(void *extbuf, socklen_t extlen, int offset, u_int8_t *typep, socklen_t *lenp, void **databufp);
int					inet6_opt_set_val(void *databuf, int offset, void *val, socklen_t vallen);
u_int8_t *			inet6_option_alloc(struct cmsghdr *cmsg, int datalen, int multx, int plusy);
int					inet6_option_append(struct cmsghdr *cmsg, const u_int8_t *typep, int multx, int plusy);
int					inet6_option_find(const struct cmsghdr *cmsg, u_int8_t **tptrp, int type);
int					inet6_option_init(void *bp, struct cmsghdr **cmsgp, int type);
int					inet6_option_next(const struct cmsghdr *cmsg, u_int8_t **tptrp);
int					inet6_option_space(int nbytes);
int					inet6_rth_add(void *bp, const struct in6_addr *addr);
struct in6_addr *	inet6_rth_getaddr(const void *bp, int idx);
void *				inet6_rth_init(void *bp, socklen_t bp_len, int type, int segments);
int					inet6_rth_reverse(const void *in, void *out);
int					inet6_rth_segments(const void *bp);
socklen_t			inet6_rth_space(int type, int segments);
int					inet6_rthdr_add(struct cmsghdr *cmsg, const struct in6_addr *addr, u_int flags);
struct in6_addr *	inet6_rthdr_getaddr(struct cmsghdr *cmsg, int idx);
int					inet6_rthdr_getflags(const struct cmsghdr *cmsg, int idx);
struct cmsghdr *	inet6_rthdr_init(void *bp, int type);
int					inet6_rthdr_lasthop(struct cmsghdr *cmsg, unsigned int flags);
int					inet6_rthdr_segments(const struct cmsghdr *cmsg);
size_t				inet6_rthdr_space(int type, int seg);


// Declared in /usr/include/rpcsvc/yp.h

// bool_t	xdr_keydat(XDR *xdrs, keydat *objp)
int	xdr_keydat(void *xdrs, void *objp);
// bool_t	xdr_valdat(XDR *xdrs, valdat *objp)
int	xdr_valdat(void *xdrs, void *objp);


// Declared in /usr/include/rpcsvc/yppasswd.h

// bool_t	xdr_x_passwd(XDR *xdrs, x_passwd *objp)
int	xdr_x_passwd(void *xdrs, void *objp);
// bool_t	xdr_yppasswd(XDR *xdrs, yppasswd *objp)
int	xdr_yppasswd(void *xdrs, void *objp);


// Declared in /usr/include/unistd.h

int	getdomainname(char *val, int inlen);
int	getgrouplist(const char *name, int basegid, int *groups, int *ngroups);
int	initgroups(const char *name, int basegid);
int	iruserok(unsigned long raddr, int superuser, const char *ruser, const char *luser);
int	iruserok_sa(const void *ra, int rlen, int superuser, const char *ruser, const char *luser);
int	rcmd(char **ahost, int rport, const char *locuser, const char *remuser, const char *cmd, int *fd2p);
int	rcmd_af(char **ahost, int rport, const char *locuser, const char *remuser, const char *cmd, int *fd2p, int af);
int	rresvport(int *port);
int	rresvport_af(int *alport, int family);
int	ruserok(const char *rhost, int superuser, const char *ruser, const char *luser);
int	setdomainname(const char *name, int namelen);


// The following symbols are part of the resolv API that have somehow managed to remain in libinfo. There's no header that
// declares them. Yet, according to https://mariana.itunes.apple.com, some developers are still using some of them, so, for
// the time being, we'll include them here so that they continue to get exported in libsystem_info.dylib.
// Note: The current proper resolv library prefixes its symbol names with "res_9_" (see /usr/include/resolv.h).

extern char	_res[];

int			__dn_skipname(const u_char *comp_dn, const u_char *eom);
u_int32_t	_getlong(const u_char *msgp);
u_int16_t	_getshort(const u_char *msgp);
int			dn_expand(const u_char *msg, const u_char *eomorig, const u_char *comp_dn, char *exp_dn, int length);
int			res_init(void);
int			res_query(const char *name, int class, int type, u_char *answer, int anslen);
int			res_search(const char *name, int class, int type, u_char *answer, int anslen);
