#ifndef res_9_private_h
#define res_9_private_h
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdint.h>

/*
 * status codes from dns_res_xxx SPIs
 * positive numbers are ns_rcode values.
 */
#define DNS_RES_STATUS_TIMEOUT -1001
#define DNS_RES_STATUS_CANCELLED -1002
#define DNS_RES_STATUS_INVALID_QUERY -1003
#define DNS_RES_STATUS_INVALID_ARGUMENT -1004
#define DNS_RES_STATUS_INVALID_RES_STATE -1005
#define DNS_RES_STATUS_INVALID_REPLY -1006
#define DNS_RES_STATUS_CONNECTION_REFUSED -1007
#define DNS_RES_STATUS_SEND_FAILED -1008
#define DNS_RES_STATUS_CONNECTION_FAILED -1009
#define DNS_RES_STATUS_SYSTEM_ERROR -1010

#define RES_EXT_SUFFIX_LEN 64

typedef struct {
	unsigned	id :16;		/* query identification number */
#if BYTE_ORDER == BIG_ENDIAN
			/* fields in third byte */
	unsigned	qr: 1;		/* response flag */
	unsigned	opcode: 4;	/* purpose of message */
	unsigned	aa: 1;		/* authoritive answer */
	unsigned	tc: 1;		/* truncated message */
	unsigned	rd: 1;		/* recursion desired */
			/* fields in fourth byte */
	unsigned	ra: 1;		/* recursion available */
	unsigned	unused :3;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned	rcode :4;	/* response code */
#endif
#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
			/* fields in third byte */
	unsigned	rd :1;		/* recursion desired */
	unsigned	tc :1;		/* truncated message */
	unsigned	aa :1;		/* authoritive answer */
	unsigned	opcode :4;	/* purpose of message */
	unsigned	qr :1;		/* response flag */
			/* fields in fourth byte */
	unsigned	rcode :4;	/* response code */
	unsigned	unused :3;	/* unused bits (MBZ as of 4.9.3a3) */
	unsigned	ra :1;		/* recursion available */
#endif
			/* remaining bytes */
	unsigned	qdcount :16;	/* number of question entries */
	unsigned	ancount :16;	/* number of answer entries */
	unsigned	nscount :16;	/* number of authority entries */
	unsigned	arcount :16;	/* number of resource entries */
} HEADER;

#ifndef __res_state_ext
#define __res_state_ext __res_9_res_state_ext
#endif

struct __res_state_ext {
	union res_sockaddr_union nsaddrs[MAXNS];
	struct sort_list {
		int af;
		union {
			struct in_addr  ina;
			struct in6_addr in6a;
		} addr, mask;
	} sort_list[MAXRESOLVSORT];
	char nsuffix[64];
	char bsuffix[64];
	char nsuffix2[64];
};

#define get_nsaddr res_9_get_nsaddr
struct sockaddr *get_nsaddr __P((res_state, size_t));

#define res_nsend_2 res_9_nsend_2
int res_nsend_2(res_state, const u_char *, int, u_char *, int, struct sockaddr *, int *);

#define res_ourserver_p res_9_ourserver_p
int res_ourserver_p(const res_state, const struct sockaddr *);

res_state res_state_new();
void res_client_close(res_state res);

/*
 * From lookupd Thread.h.  We use this to signal threads to quit, since pthread_cancel() doesn't work.
 */
#define ThreadStateExitRequested 4

/*
 * notification SPI
 */
extern uint32_t notify_register_plain(const char *name, int *out_token);

extern int res_query_mDNSResponder(res_state statp, const char *name, int class, int type, u_char *answer, int anslen, struct sockaddr *from, uint32_t *fromlen);

int dns_res_once(struct sockaddr *server, struct timeval *timeout, int options, const char *name, int class, int type, u_char *res, int *reslen);

/*
 * Interrupt a thread blocked in select()
 */
void res_interrupt_requests_enable(void);
void res_interrupt_requests_disable(void);
void res_interrupt_request(void *token);
void *res_init_interrupt_token(void);
void res_delete_interrupt_token(void *token);

#endif
